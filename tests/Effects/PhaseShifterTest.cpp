#include "../helpers/test_helpers.h"
#include <DSP/Effects/PhaseShifter.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

TEST_CASE ("PhaseShifter: produces output", "[phaser]")
{
    PhaseShifter ps;
    ps.prepare (48000.0);
    ps.setRate (1.0f);
    ps.setDepth (0.5f);
    ps.setStereoMode (PhaseShifter::StereoMode::Stereo);

    auto input = TestHelpers::makeSine (440.0f, 48000.0, 48000);
    juce::AudioBuffer<float> output (1, 48000);
    const auto* in = input.getReadPointer (0);
    auto* out = output.getWritePointer (0);

    for (int i = 0; i < 48000; ++i)
        out[i] = ps.processSample (in[i], 0);

    REQUIRE_FALSE (TestHelpers::hasNanOrInf (output));
    REQUIRE (TestHelpers::measureRMS (output) > 0.1f);
}

TEST_CASE ("PhaseShifter: stereo mode produces different L/R", "[phaser]")
{
    PhaseShifter ps;
    ps.prepare (48000.0);
    ps.setRate (2.0f);
    ps.setDepth (0.8f);
    ps.setStereoMode (PhaseShifter::StereoMode::Stereo);

    float diffSum = 0.0f;
    for (int i = 0; i < 48000; ++i)
    {
        float in = std::sin (static_cast<float> (i) * 0.1f);
        float outL = ps.processSample (in, 0);
        float outR = ps.processSample (in, 1);
        diffSum += std::abs (outL - outR);
    }

    // There should be noticeable difference between L and R
    REQUIRE (diffSum > 10.0f);
}

TEST_CASE ("PhaseShifter: mono mode produces identical L/R", "[phaser]")
{
    PhaseShifter ps;
    ps.prepare (48000.0);
    ps.setRate (2.0f);
    ps.setDepth (0.8f);
    ps.setStereoMode (PhaseShifter::StereoMode::Mono);

    float diffSum = 0.0f;
    for (int i = 0; i < 48000; ++i)
    {
        float in = std::sin (static_cast<float> (i) * 0.1f);
        float outL = ps.processSample (in, 0);
        float outR = ps.processSample (in, 1);
        diffSum += std::abs (outL - outR);
    }

    REQUIRE (diffSum < 1.0f);
}
