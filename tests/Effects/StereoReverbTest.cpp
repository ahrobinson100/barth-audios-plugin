#include "../helpers/test_helpers.h"
#include <DSP/Effects/StereoReverb.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

TEST_CASE ("StereoReverb: impulse produces decaying tail", "[reverb]")
{
    StereoReverb rev;
    rev.prepare (48000.0);
    rev.setDecayTime (1.5f);
    rev.setRoomSize (0.5f);

    // Feed impulse
    float outL, outR;
    rev.processSample (1.0f, 1.0f, outL, outR);

    // Process silence for 2 seconds, check for decay
    float peakFirst = 0.0f;
    float peakLast = 0.0f;

    for (int i = 0; i < 96000; ++i)
    {
        rev.processSample (0.0f, 0.0f, outL, outR);
        float level = std::abs (outL) + std::abs (outR);

        if (i < 24000)
            peakFirst = std::max (peakFirst, level);
        if (i > 72000)
            peakLast = std::max (peakLast, level);
    }

    // Later output should be quieter than earlier
    REQUIRE (peakLast < peakFirst);
}

TEST_CASE ("StereoReverb: no runaway at max settings", "[reverb][safety]")
{
    StereoReverb rev;
    rev.prepare (48000.0);
    rev.setDecayTime (10.0f);
    rev.setRoomSize (1.0f);

    // Feed continuous signal
    for (int i = 0; i < 480000; ++i)
    {
        float in = (i < 48000) ? std::sin (static_cast<float> (i) * 0.1f) : 0.0f;
        float outL, outR;
        rev.processSample (in, in, outL, outR);
        REQUIRE (std::abs (outL) < 2.0f);
        REQUIRE (std::abs (outR) < 2.0f);
        REQUIRE_FALSE (std::isnan (outL));
        REQUIRE_FALSE (std::isnan (outR));
    }
}

TEST_CASE ("StereoReverb: stereo output", "[reverb]")
{
    StereoReverb rev;
    rev.prepare (48000.0);
    rev.setDecayTime (1.0f);
    rev.setRoomSize (0.5f);

    // Feed impulse to L only
    float outL, outR;
    rev.processSample (1.0f, 0.0f, outL, outR);

    // After some time, both L and R should have energy (diffusion)
    float sumL = 0.0f, sumR = 0.0f;
    for (int i = 0; i < 24000; ++i)
    {
        rev.processSample (0.0f, 0.0f, outL, outR);
        sumL += std::abs (outL);
        sumR += std::abs (outR);
    }

    REQUIRE (sumL > 0.01f);
    REQUIRE (sumR > 0.01f);
}
