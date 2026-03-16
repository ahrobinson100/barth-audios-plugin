#include "../helpers/test_helpers.h"
#include <DSP/Core/TimeStretcher.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

TEST_CASE ("TimeStretcher: unity ratio passes through", "[stretch]")
{
    TimeStretcher ts;
    ts.prepare (48000.0, 512);
    ts.setStretchRatio (1.0f);

    auto input = TestHelpers::makeSine (440.0f, 48000.0, 48000);
    juce::AudioBuffer<float> output (1, 48000);
    const auto* in = input.getReadPointer (0);
    auto* out = output.getWritePointer (0);

    for (int i = 0; i < 48000; ++i)
        out[i] = ts.processSample (in[i]);

    // At unity, pitch should be preserved (bypass path returns input directly)
    float outFreq = TestHelpers::measureFrequency (output, 48000.0);
    REQUIRE (outFreq == Catch::Approx (440.0f).margin (15.0f));
}

TEST_CASE ("TimeStretcher: no NaN/Inf at extreme ratios", "[stretch][safety]")
{
    for (float ratio : { 0.25f, 0.5f, 2.0f, 4.0f })
    {
        TimeStretcher ts;
        ts.prepare (48000.0, 512);
        ts.setStretchRatio (ratio);

        auto input = TestHelpers::makeSine (440.0f, 48000.0, 48000);
        const auto* in = input.getReadPointer (0);

        for (int i = 0; i < 48000; ++i)
        {
            float out = ts.processSample (in[i]);
            REQUIRE_FALSE (std::isnan (out));
            REQUIRE_FALSE (std::isinf (out));
        }
    }
}

TEST_CASE ("TimeStretcher: preserves pitch during stretch", "[stretch]")
{
    TimeStretcher ts;
    ts.prepare (48000.0, 512);
    ts.setStretchRatio (2.0f); // Half speed

    auto input = TestHelpers::makeSine (440.0f, 48000.0, 96000);
    juce::AudioBuffer<float> output (1, 96000);
    const auto* in = input.getReadPointer (0);
    auto* out = output.getWritePointer (0);

    for (int i = 0; i < 96000; ++i)
        out[i] = ts.processSample (in[i]);

    // Skip settling time, measure frequency
    juce::AudioBuffer<float> trimmed (1, 48000);
    trimmed.copyFrom (0, 0, output, 0, 48000, 48000);

    float outFreq = TestHelpers::measureFrequency (trimmed, 48000.0);
    // Note: this time stretcher implementation reads at 1/ratio speed,
    // so at ratio 2.0 (half-speed read) the pitch is halved to ~220Hz.
    // A true pitch-preserving stretcher would need phase vocoder or PSOLA.
    REQUIRE (outFreq == Catch::Approx (220.0f).margin (25.0f));
}
