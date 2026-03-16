#include "../helpers/test_helpers.h"
#include <DSP/Effects/FrequencyShifter.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

TEST_CASE ("FrequencyShifter: zero shift passes signal through", "[freqshift]")
{
    FrequencyShifter fs;
    fs.prepare (48000.0);
    fs.setShiftHz (0.0f);

    auto input = TestHelpers::makeSine (440.0f, 48000.0, 48000);
    juce::AudioBuffer<float> output (1, 48000);
    const auto* in = input.getReadPointer (0);
    auto* out = output.getWritePointer (0);

    for (int i = 0; i < 48000; ++i)
        out[i] = fs.processSample (in[i]);

    // Skip settling, measure frequency
    juce::AudioBuffer<float> trimmed (1, 24000);
    trimmed.copyFrom (0, 0, output, 0, 24000, 24000);
    float outFreq = TestHelpers::measureFrequency (trimmed, 48000.0);
    REQUIRE (outFreq == Catch::Approx (440.0f).margin (15.0f));
}

TEST_CASE ("FrequencyShifter: shifts frequency upward", "[freqshift]")
{
    FrequencyShifter fs;
    fs.prepare (48000.0);
    fs.setShiftHz (100.0f);

    auto input = TestHelpers::makeSine (440.0f, 48000.0, 96000);
    juce::AudioBuffer<float> output (1, 96000);
    const auto* in = input.getReadPointer (0);
    auto* out = output.getWritePointer (0);

    for (int i = 0; i < 96000; ++i)
        out[i] = fs.processSample (in[i]);

    // Skip settling
    juce::AudioBuffer<float> trimmed (1, 48000);
    trimmed.copyFrom (0, 0, output, 0, 48000, 48000);
    float outFreq = TestHelpers::measureFrequency (trimmed, 48000.0);
    // Should be ~540 Hz (440 + 100)
    REQUIRE (outFreq == Catch::Approx (540.0f).margin (30.0f));
}

TEST_CASE ("FrequencyShifter: no NaN/Inf", "[freqshift][safety]")
{
    FrequencyShifter fs;
    fs.prepare (48000.0);
    fs.setShiftHz (1000.0f);

    for (int i = 0; i < 48000; ++i)
    {
        float in = std::sin (static_cast<float> (i) * 0.1f);
        float out = fs.processSample (in);
        REQUIRE_FALSE (std::isnan (out));
        REQUIRE_FALSE (std::isinf (out));
    }
}

TEST_CASE ("FrequencyShifter: silence in produces silence out", "[freqshift][safety]")
{
    FrequencyShifter fs;
    fs.prepare (48000.0);
    fs.setShiftHz (100.0f);

    float maxOut = 0.0f;
    for (int i = 0; i < 48000; ++i)
    {
        float out = fs.processSample (0.0f);
        maxOut = std::max (maxOut, std::abs (out));
    }
    REQUIRE (maxOut < 0.001f);
}

TEST_CASE ("FrequencyShifter: reset clears state", "[freqshift]")
{
    FrequencyShifter fs;
    fs.prepare (48000.0);
    fs.setShiftHz (100.0f);

    // Process some signal
    for (int i = 0; i < 4800; ++i)
        fs.processSample (std::sin (static_cast<float> (i) * 0.1f));

    fs.reset();

    // After reset, silence in should produce silence out
    float maxOut = 0.0f;
    for (int i = 0; i < 4800; ++i)
    {
        float out = fs.processSample (0.0f);
        maxOut = std::max (maxOut, std::abs (out));
    }
    REQUIRE (maxOut < 0.001f);
}
