#include "../helpers/test_helpers.h"
#include <DSP/Utilities/OnePoleFilter.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

TEST_CASE ("OnePoleFilter: lowpass attenuates high frequencies", "[onepole]")
{
    OnePoleFilter lpf;
    lpf.prepare (48000.0);
    lpf.setCutoff (1000.0f);

    // Process 5kHz sine through 1kHz lowpass
    auto sine = TestHelpers::makeSine (5000.0f, 48000.0, 48000);
    const auto* in = sine.getReadPointer (0);

    juce::AudioBuffer<float> output (1, 48000);
    auto* out = output.getWritePointer (0);
    for (int i = 0; i < 48000; ++i)
        out[i] = lpf.processLowPass (in[i]);

    float inputRms = TestHelpers::measureRMS (sine);
    float outputRms = TestHelpers::measureRMS (output);

    // 5kHz should be attenuated significantly by 1kHz LPF
    REQUIRE (outputRms < inputRms * 0.5f);
}

TEST_CASE ("OnePoleFilter: highpass attenuates low frequencies", "[onepole]")
{
    OnePoleFilter hpf;
    hpf.prepare (48000.0);
    hpf.setCutoff (5000.0f);

    // Process 100Hz sine through 5kHz highpass
    auto sine = TestHelpers::makeSine (100.0f, 48000.0, 48000);
    const auto* in = sine.getReadPointer (0);

    juce::AudioBuffer<float> output (1, 48000);
    auto* out = output.getWritePointer (0);
    for (int i = 0; i < 48000; ++i)
        out[i] = hpf.processHighPass (in[i]);

    float inputRms = TestHelpers::measureRMS (sine);
    float outputRms = TestHelpers::measureRMS (output);

    REQUIRE (outputRms < inputRms * 0.5f);
}

TEST_CASE ("OnePoleFilter: high cutoff passes signal", "[onepole]")
{
    OnePoleFilter lpf;
    lpf.prepare (48000.0);
    lpf.setCutoff (20000.0f);

    // At cutoff near Nyquist, most signal should pass through
    // Settle the filter first
    for (int i = 0; i < 200; ++i)
        lpf.processLowPass (0.5f);

    float out = lpf.processLowPass (0.5f);
    REQUIRE (out == Catch::Approx (0.5f).margin (0.05f));
}

TEST_CASE ("OnePoleFilter: reset clears state", "[onepole]")
{
    OnePoleFilter lpf;
    lpf.prepare (48000.0);
    lpf.setCutoff (1000.0f);

    // Build up state
    for (int i = 0; i < 1000; ++i)
        lpf.processLowPass (1.0f);

    lpf.reset();

    // After reset, processing silence should give near-silence
    float out = lpf.processLowPass (0.0f);
    REQUIRE (std::abs (out) < 0.001f);
}
