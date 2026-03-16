#include "../helpers/test_helpers.h"
#include <DSP/Utilities/DCBlocker.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

TEST_CASE ("DCBlocker: removes DC offset", "[dcblocker]")
{
    DCBlocker dc;
    dc.prepare (48000.0);

    // Feed constant DC offset (0.5) for 1 second
    for (int i = 0; i < 48000; ++i)
        dc.processSample (0.5f);

    // After settling, output of a pure DC signal should be near zero
    float out = dc.processSample (0.5f);
    REQUIRE (std::abs (out) < 0.01f);
}

TEST_CASE ("DCBlocker: passes AC signal", "[dcblocker]")
{
    DCBlocker dc;
    dc.prepare (48000.0);

    auto sine = TestHelpers::makeSine (440.0f, 48000.0, 48000);
    const auto* in = sine.getReadPointer (0);

    // Process the full sine wave
    juce::AudioBuffer<float> output (1, 48000);
    auto* out = output.getWritePointer (0);
    for (int i = 0; i < 48000; ++i)
        out[i] = dc.processSample (in[i]);

    // RMS should be substantial (AC passes through)
    float rms = TestHelpers::measureRMS (output);
    REQUIRE (rms > 0.5f);
}

TEST_CASE ("DCBlocker: no NaN/Inf at extreme input", "[dcblocker][safety]")
{
    DCBlocker dc;
    dc.prepare (48000.0);

    float extremes[] = { 1e6f, -1e6f, 1e-30f, -1e-30f, 0.0f };
    for (float val : extremes)
    {
        float out = dc.processSample (val);
        REQUIRE_FALSE (std::isnan (out));
        REQUIRE_FALSE (std::isinf (out));
    }
}

TEST_CASE ("DCBlocker: reset clears state", "[dcblocker]")
{
    DCBlocker dc;
    dc.prepare (48000.0);

    // Process some signal
    for (int i = 0; i < 1000; ++i)
        dc.processSample (0.8f);

    dc.reset();

    // After reset, feeding silence should produce silence
    for (int i = 0; i < 100; ++i)
    {
        float out = dc.processSample (0.0f);
        REQUIRE (std::abs (out) < 0.001f);
    }
}
