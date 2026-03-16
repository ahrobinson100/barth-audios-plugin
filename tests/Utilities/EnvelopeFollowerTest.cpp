#include "../helpers/test_helpers.h"
#include <DSP/Utilities/EnvelopeFollower.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

TEST_CASE ("EnvelopeFollower: tracks amplitude", "[envfollower]")
{
    EnvelopeFollower env;
    env.prepare (48000.0);
    env.setAttackMs (1.0f);
    env.setReleaseMs (50.0f);

    // Feed loud signal
    for (int i = 0; i < 4800; ++i)
        env.processSample (0.8f * std::sin (static_cast<float> (i) * 0.1f));

    // Envelope should be tracking near the amplitude
    float result = env.processSample (0.8f);
    REQUIRE (result > 0.3f);
}

TEST_CASE ("EnvelopeFollower: detectTransient fires on impulse", "[envfollower]")
{
    EnvelopeFollower env;
    env.prepare (48000.0);
    env.setAttackMs (0.5f);
    env.setReleaseMs (50.0f);
    env.setThresholdDb (-40.0f);

    // Feed silence, then sudden impulse
    for (int i = 0; i < 1000; ++i)
        env.detectTransient (0.0f);

    // Transient should trigger on sudden loud signal
    bool triggered = false;
    for (int i = 0; i < 100; ++i)
    {
        if (env.detectTransient (0.9f))
            triggered = true;
    }
    REQUIRE (triggered);
}

TEST_CASE ("EnvelopeFollower: no transient on sustained signal", "[envfollower]")
{
    EnvelopeFollower env;
    env.prepare (48000.0);
    env.setAttackMs (1.0f);
    env.setReleaseMs (50.0f);
    env.setThresholdDb (-20.0f);

    // Build up envelope with sustained signal
    for (int i = 0; i < 4800; ++i)
        env.detectTransient (0.5f);

    // Continuing the same level should not fire transients
    int transientCount = 0;
    for (int i = 0; i < 4800; ++i)
    {
        if (env.detectTransient (0.5f))
            ++transientCount;
    }
    REQUIRE (transientCount == 0);
}

TEST_CASE ("EnvelopeFollower: threshold setting", "[envfollower]")
{
    EnvelopeFollower env;
    env.prepare (48000.0);
    env.setAttackMs (0.5f);
    env.setReleaseMs (50.0f);
    env.setThresholdDb (0.0f); // threshold = 1.0, very high

    // Feed moderate signal - should not trigger at threshold 0dB
    for (int i = 0; i < 1000; ++i)
        env.detectTransient (0.0f);

    bool triggered = false;
    for (int i = 0; i < 100; ++i)
    {
        if (env.detectTransient (0.5f))
            triggered = true;
    }
    REQUIRE_FALSE (triggered);
}

TEST_CASE ("EnvelopeFollower: reset clears state", "[envfollower]")
{
    EnvelopeFollower env;
    env.prepare (48000.0);

    for (int i = 0; i < 1000; ++i)
        env.processSample (0.8f);

    env.reset();

    float out = env.processSample (0.0f);
    REQUIRE (out == Catch::Approx (0.0f).margin (0.001f));
}
