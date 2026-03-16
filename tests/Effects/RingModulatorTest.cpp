#include "../helpers/test_helpers.h"
#include <DSP/Effects/RingModulator.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

TEST_CASE ("RingModulator: zero depth passes through", "[ringmod]")
{
    RingModulator rm;
    rm.prepare (48000.0);
    rm.setFrequency (200.0f);
    rm.setDepth (0.0f);

    float in = 0.5f;
    float out = rm.processSample (in);
    REQUIRE (out == Catch::Approx (0.5f).margin (0.01f));
}

TEST_CASE ("RingModulator: full depth produces ring modulation", "[ringmod]")
{
    RingModulator rm;
    rm.prepare (48000.0);
    rm.setFrequency (200.0f);
    rm.setDepth (1.0f);

    // Sine at 440Hz × sine at 200Hz should produce sidebands at 240Hz and 640Hz
    auto input = TestHelpers::makeSine (440.0f, 48000.0, 48000);
    juce::AudioBuffer<float> output (1, 48000);
    const auto* in = input.getReadPointer (0);
    auto* out = output.getWritePointer (0);

    for (int i = 0; i < 48000; ++i)
        out[i] = rm.processSample (in[i]);

    REQUIRE_FALSE (TestHelpers::hasNanOrInf (output));

    // The RMS should be reasonable
    float rms = TestHelpers::measureRMS (output);
    REQUIRE (rms > 0.1f);
    REQUIRE (rms < 1.0f);
}

TEST_CASE ("RingModulator: no NaN/Inf", "[ringmod][safety]")
{
    RingModulator rm;
    rm.prepare (48000.0);
    rm.setFrequency (5000.0f);
    rm.setDepth (1.0f);

    for (int i = 0; i < 48000; ++i)
    {
        float out = rm.processSample (std::sin (static_cast<float> (i) * 0.1f));
        REQUIRE_FALSE (std::isnan (out));
        REQUIRE_FALSE (std::isinf (out));
    }
}
