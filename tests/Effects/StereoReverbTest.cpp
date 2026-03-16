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

TEST_CASE ("StereoReverb: silence in produces silence out", "[reverb][safety]")
{
    StereoReverb rev;
    rev.prepare (48000.0);
    rev.setDecayTime (2.0f);
    rev.setRoomSize (0.5f);

    float maxOut = 0.0f;
    for (int i = 0; i < 48000; ++i)
    {
        float outL, outR;
        rev.processSample (0.0f, 0.0f, outL, outR);
        maxOut = std::max (maxOut, std::max (std::abs (outL), std::abs (outR)));
    }
    REQUIRE (maxOut < 0.0001f);
}

TEST_CASE ("StereoReverb: damping reduces high frequencies", "[reverb]")
{
    // Compare bright vs damped tail
    auto processImpulse = [] (float damping) {
        StereoReverb rev;
        rev.prepare (48000.0);
        rev.setDecayTime (2.0f);
        rev.setRoomSize (0.5f);
        rev.setDamping (damping);

        float outL, outR;
        rev.processSample (1.0f, 1.0f, outL, outR); // Impulse

        // Collect late tail (after 0.5s) into a buffer for RMS measurement
        for (int i = 0; i < 24000; ++i)
            rev.processSample (0.0f, 0.0f, outL, outR);

        float rmsSum = 0.0f;
        int count = 0;
        for (int i = 0; i < 24000; ++i)
        {
            rev.processSample (0.0f, 0.0f, outL, outR);
            rmsSum += outL * outL + outR * outR;
            ++count;
        }
        return std::sqrt (rmsSum / static_cast<float> (count * 2));
    };

    float brightRms = processImpulse (1.0f);  // damping=1.0 = no filtering = bright
    float dampedRms = processImpulse (0.1f);  // damping=0.1 = heavy filtering = damped

    // Damped tail should be quieter (energy absorbed by damping)
    REQUIRE (dampedRms < brightRms);
}

TEST_CASE ("StereoReverb: reset clears state", "[reverb]")
{
    StereoReverb rev;
    rev.prepare (48000.0);
    rev.setDecayTime (5.0f);
    rev.setRoomSize (0.5f);

    // Feed impulse
    float outL, outR;
    rev.processSample (1.0f, 1.0f, outL, outR);
    for (int i = 0; i < 4800; ++i)
        rev.processSample (0.0f, 0.0f, outL, outR);

    rev.reset();

    // After reset, should output silence
    float maxOut = 0.0f;
    for (int i = 0; i < 4800; ++i)
    {
        rev.processSample (0.0f, 0.0f, outL, outR);
        maxOut = std::max (maxOut, std::max (std::abs (outL), std::abs (outR)));
    }
    REQUIRE (maxOut < 0.001f);
}
