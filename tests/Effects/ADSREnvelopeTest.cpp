#include "../helpers/test_helpers.h"
#include <DSP/Effects/ADSREnvelope.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

TEST_CASE ("ADSREnvelope: idle state outputs zero", "[adsr]")
{
    ADSREnvelope adsr;
    adsr.prepare (48000.0);

    float val = adsr.processSample();
    REQUIRE (val == 0.0f);
    REQUIRE (adsr.getState() == ADSREnvelope::State::Idle);
}

TEST_CASE ("ADSREnvelope: trigger starts attack", "[adsr]")
{
    ADSREnvelope adsr;
    adsr.prepare (48000.0);
    adsr.setAttackMs (10.0f);
    adsr.setDecayMs (100.0f);
    adsr.setSustain (0.7f);
    adsr.setReleaseMs (200.0f);
    adsr.updateCoefficients();

    adsr.trigger();
    REQUIRE (adsr.getState() == ADSREnvelope::State::Attack);

    // After attack time, should reach near 1.0
    for (int i = 0; i < 960; ++i) // ~20ms at 48kHz
        adsr.processSample();

    float val = adsr.processSample();
    REQUIRE (val > 0.9f);
}

TEST_CASE ("ADSREnvelope: reaches sustain level", "[adsr]")
{
    ADSREnvelope adsr;
    adsr.prepare (48000.0);
    adsr.setAttackMs (5.0f);
    adsr.setDecayMs (50.0f);
    adsr.setSustain (0.5f);
    adsr.setReleaseMs (100.0f);
    adsr.updateCoefficients();

    adsr.trigger();

    // Run enough for attack + decay
    for (int i = 0; i < 48000; ++i)
        adsr.processSample();

    // Should be at sustain level
    float val = adsr.processSample();
    REQUIRE (val == Catch::Approx (0.5f).margin (0.05f));
    REQUIRE (adsr.getState() == ADSREnvelope::State::Sustain);
}

TEST_CASE ("ADSREnvelope: release goes to zero", "[adsr]")
{
    ADSREnvelope adsr;
    adsr.prepare (48000.0);
    adsr.setAttackMs (1.0f);
    adsr.setDecayMs (10.0f);
    adsr.setSustain (0.7f);
    adsr.setReleaseMs (50.0f);
    adsr.updateCoefficients();

    adsr.trigger();
    for (int i = 0; i < 24000; ++i) // Get to sustain
        adsr.processSample();

    adsr.release();
    for (int i = 0; i < 24000; ++i) // Wait for release
        adsr.processSample();

    float val = adsr.processSample();
    REQUIRE (val < 0.01f);
    REQUIRE (adsr.getState() == ADSREnvelope::State::Idle);
}

TEST_CASE ("ADSREnvelope: inverted mode", "[adsr]")
{
    ADSREnvelope adsr;
    adsr.prepare (48000.0);
    adsr.setAttackMs (5.0f);
    adsr.setDecayMs (50.0f);
    adsr.setSustain (0.5f);
    adsr.setReleaseMs (100.0f);
    adsr.updateCoefficients();
    adsr.setInverted (true);

    // Idle: inverted should output 1.0
    float val = adsr.processSample();
    REQUIRE (val == Catch::Approx (1.0f));

    adsr.trigger();
    // During attack (rising to 1.0), inverted falls toward 0.0
    for (int i = 0; i < 960; ++i)
        adsr.processSample();

    val = adsr.processSample();
    REQUIRE (val < 0.2f); // Inverted: near 0 at peak
}

TEST_CASE ("ADSREnvelope: no NaN/Inf", "[adsr][safety]")
{
    ADSREnvelope adsr;
    adsr.prepare (48000.0);
    adsr.setAttackMs (1.0f);
    adsr.setDecayMs (1.0f);
    adsr.setSustain (0.0f);
    adsr.setReleaseMs (1.0f);
    adsr.updateCoefficients();

    // Rapid trigger/release
    for (int i = 0; i < 48000; ++i)
    {
        if (i % 100 == 0) adsr.trigger();
        if (i % 100 == 50) adsr.release();
        float val = adsr.processSample();
        REQUIRE_FALSE (std::isnan (val));
        REQUIRE_FALSE (std::isinf (val));
        REQUIRE (val >= 0.0f);
        REQUIRE (val <= 1.1f); // Slight overshoot in attack is OK
    }
}

TEST_CASE ("ADSREnvelope: re-trigger during release", "[adsr]")
{
    ADSREnvelope adsr;
    adsr.prepare (48000.0);
    adsr.setAttackMs (10.0f);
    adsr.setDecayMs (50.0f);
    adsr.setSustain (0.5f);
    adsr.setReleaseMs (500.0f);
    adsr.updateCoefficients();

    // Get to sustain
    adsr.trigger();
    for (int i = 0; i < 24000; ++i)
        adsr.processSample();

    // Start release
    adsr.release();
    for (int i = 0; i < 4800; ++i)
        adsr.processSample();

    float levelBeforeRetrigger = adsr.processSample();
    REQUIRE (levelBeforeRetrigger < 0.5f); // Should be partway through release

    // Re-trigger: should start new attack from current level
    adsr.trigger();
    REQUIRE (adsr.getState() == ADSREnvelope::State::Attack);

    // After enough time, should reach peak again
    for (int i = 0; i < 960; ++i) // ~20ms
        adsr.processSample();

    float val = adsr.processSample();
    REQUIRE (val > 0.8f);
}

TEST_CASE ("ADSREnvelope: zero-length attack", "[adsr]")
{
    ADSREnvelope adsr;
    adsr.prepare (48000.0);
    adsr.setAttackMs (1.0f); // Minimum attack
    adsr.setDecayMs (50.0f);
    adsr.setSustain (0.5f);
    adsr.setReleaseMs (100.0f);
    adsr.updateCoefficients();

    adsr.trigger();

    // With very fast attack, should reach near peak quickly
    for (int i = 0; i < 480; ++i) // 10ms
        adsr.processSample();

    float val = adsr.processSample();
    REQUIRE (val > 0.8f);
}

TEST_CASE ("ADSREnvelope: reset clears state", "[adsr]")
{
    ADSREnvelope adsr;
    adsr.prepare (48000.0);
    adsr.setAttackMs (10.0f);
    adsr.setDecayMs (50.0f);
    adsr.setSustain (0.5f);
    adsr.setReleaseMs (100.0f);
    adsr.updateCoefficients();

    adsr.trigger();
    for (int i = 0; i < 4800; ++i)
        adsr.processSample();

    adsr.reset();

    REQUIRE (adsr.getState() == ADSREnvelope::State::Idle);
    float val = adsr.processSample();
    REQUIRE (val == 0.0f);
}
