#include "../helpers/test_helpers.h"
#include <DSP/Core/PitchShifter.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

TEST_CASE ("PitchShifter: unity pitch ratio produces unmodified signal", "[pitch]")
{
    const double sr = 48000.0;
    auto input = TestHelpers::makeSine (440.0f, sr, 48000);
    auto output = TestHelpers::processWithPitchShifter (input, 1.0f, sr, 40.0f, 4800);

    float outFreq = TestHelpers::measureFrequency (output, sr);
    REQUIRE (outFreq == Catch::Approx (440.0f).margin (5.0f));
}

TEST_CASE ("PitchShifter: octave up doubles frequency", "[pitch]")
{
    const double sr = 48000.0;
    auto input = TestHelpers::makeSine (440.0f, sr, 48000);
    auto output = TestHelpers::processWithPitchShifter (input, 2.0f, sr, 40.0f, 4800);

    float outFreq = TestHelpers::measureFrequency (output, sr);
    REQUIRE (outFreq == Catch::Approx (880.0f).margin (15.0f));
}

TEST_CASE ("PitchShifter: octave down halves frequency", "[pitch]")
{
    const double sr = 48000.0;
    auto input = TestHelpers::makeSine (440.0f, sr, 48000);
    auto output = TestHelpers::processWithPitchShifter (input, 0.5f, sr, 40.0f, 4800);

    float outFreq = TestHelpers::measureFrequency (output, sr);
    REQUIRE (outFreq == Catch::Approx (220.0f).margin (10.0f));
}

TEST_CASE ("PitchShifter: no glitches at crossfade boundaries", "[pitch][safety]")
{
    const double sr = 48000.0;
    auto input = TestHelpers::makeSine (440.0f, sr, 96000);
    auto output = TestHelpers::processWithPitchShifter (input, 1.5f, sr, 40.0f, 4800);

    int glitches = TestHelpers::detectGlitches (output, 0.3f);
    REQUIRE (glitches == 0);
}

TEST_CASE ("PitchShifter: silence in → silence out", "[pitch][safety]")
{
    const double sr = 48000.0;
    auto input = TestHelpers::makeSilence (48000);
    auto output = TestHelpers::processWithPitchShifter (input, 1.5f, sr);

    float rms = TestHelpers::measureRMS (output);
    REQUIRE (rms < 0.0001f);

    float dc = TestHelpers::measureDCOffset (output);
    REQUIRE (std::abs (dc) < 0.0001f);
}

TEST_CASE ("PitchShifter: no NaN/Inf at extreme ratios", "[pitch][safety]")
{
    const double sr = 48000.0;
    auto input = TestHelpers::makeSine (440.0f, sr, 48000);

    for (float ratio : { 0.25f, 0.5f, 2.0f, 4.0f })
    {
        auto output = TestHelpers::processWithPitchShifter (input, ratio, sr);
        REQUIRE_FALSE (TestHelpers::hasNanOrInf (output));
    }
}

TEST_CASE ("PitchShifter: output level stays within bounds", "[pitch][safety]")
{
    const double sr = 48000.0;
    auto input = TestHelpers::makeSine (440.0f, sr, 48000);
    auto output = TestHelpers::processWithPitchShifter (input, 2.0f, sr);

    float peak = TestHelpers::measurePeak (output);
    REQUIRE (peak <= 1.5f); // Allow some overshoot from crossfade, but not runaway
}

TEST_CASE ("PitchShifter: handles all buffer sizes", "[pitch][compat]")
{
    for (int blockSize : { 1, 32, 64, 128, 256, 512, 1024, 2048, 8192 })
    {
        PitchShifter ps;
        ps.prepare (48000.0, blockSize);
        ps.setPitchRatio (1.5f);

        // Process a few blocks
        for (int b = 0; b < 10; ++b)
        {
            for (int i = 0; i < blockSize; ++i)
            {
                float val = std::sin (static_cast<float> (b * blockSize + i) * 0.1f);
                float out = ps.processSample (val);
                REQUIRE_FALSE (std::isnan (out));
                REQUIRE_FALSE (std::isinf (out));
            }
        }
    }
}

TEST_CASE ("PitchShifter: handles all sample rates", "[pitch][compat]")
{
    for (double sr : { 22050.0, 44100.0, 48000.0, 88200.0, 96000.0, 192000.0 })
    {
        auto input = TestHelpers::makeSine (440.0f, sr, static_cast<int> (sr));
        auto output = TestHelpers::processWithPitchShifter (input, 2.0f, sr, 40.0f, static_cast<int> (sr * 0.1));

        REQUIRE_FALSE (TestHelpers::hasNanOrInf (output));
        float outFreq = TestHelpers::measureFrequency (output, sr);
        REQUIRE (outFreq == Catch::Approx (880.0f).margin (25.0f));
    }
}

TEST_CASE ("PitchShifter: semitone mode snaps correctly", "[pitch]")
{
    const double sr = 48000.0;
    auto input = TestHelpers::makeSine (440.0f, sr, 48000);

    // +7 semitones = perfect fifth = ratio 1.4983 → freq ~659 Hz
    auto output = TestHelpers::processWithPitchShifterSt (input, 7.0f, sr, 40.0f, 4800);
    float outFreq = TestHelpers::measureFrequency (output, sr);
    float expectedFreq = 440.0f * std::pow (2.0f, 7.0f / 12.0f); // ~659.3
    REQUIRE (outFreq == Catch::Approx (expectedFreq).margin (15.0f));
}
