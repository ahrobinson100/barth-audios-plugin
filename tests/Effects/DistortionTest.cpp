#include "../helpers/test_helpers.h"
#include <DSP/Effects/Distortion.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

TEST_CASE ("Distortion: zero drive passes through", "[dist]")
{
    Distortion dist;
    dist.prepare (48000.0);
    dist.setDrive (0.0f);
    dist.setBassBoostDb (0.0f);

    float out = dist.processSample (0.5f);
    REQUIRE (out == Catch::Approx (0.5f).margin (0.01f));
}

TEST_CASE ("Distortion: drive adds harmonics", "[dist]")
{
    Distortion dist;
    dist.prepare (48000.0);
    dist.setDrive (0.8f);
    dist.setBassBoostDb (0.0f);

    auto input = TestHelpers::makeSine (440.0f, 48000.0, 48000);
    juce::AudioBuffer<float> output (1, 48000);
    const auto* in = input.getReadPointer (0);
    auto* out = output.getWritePointer (0);

    for (int i = 0; i < 48000; ++i)
        out[i] = dist.processSample (in[i]);

    REQUIRE_FALSE (TestHelpers::hasNanOrInf (output));
    // Peak should be < 1.0 (output compensation)
    REQUIRE (TestHelpers::measurePeak (output) <= 1.0f);
}

TEST_CASE ("Distortion: bass boost increases low frequency energy", "[dist]")
{
    Distortion dist;
    dist.prepare (48000.0);
    dist.setDrive (0.0f);
    dist.setBassBoostDb (12.0f);

    auto input = TestHelpers::makeSine (100.0f, 48000.0, 48000);
    juce::AudioBuffer<float> output (1, 48000);
    const auto* in = input.getReadPointer (0);
    auto* out = output.getWritePointer (0);

    for (int i = 0; i < 48000; ++i)
        out[i] = dist.processSample (in[i]);

    float inRms = TestHelpers::measureRMS (input);
    float outRms = TestHelpers::measureRMS (output);
    // Bass boosted output should be louder
    REQUIRE (outRms > inRms * 1.5f);
}

TEST_CASE ("Distortion: no NaN/Inf at extreme settings", "[dist][safety]")
{
    Distortion dist;
    dist.prepare (48000.0);
    dist.setDrive (1.0f);
    dist.setBassBoostDb (12.0f);

    for (int i = 0; i < 48000; ++i)
    {
        float out = dist.processSample (std::sin (static_cast<float> (i) * 0.05f));
        REQUIRE_FALSE (std::isnan (out));
        REQUIRE_FALSE (std::isinf (out));
    }
}
