#include "../helpers/test_helpers.h"
#include <DSP/Core/DelayLine.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

TEST_CASE ("DelayLine: exact delay time", "[delay]")
{
    DelayLine dl;
    dl.prepare (48000.0, 512);
    dl.setDelayMs (10.0f); // 480 samples at 48kHz
    dl.setFeedback (0.0f);

    // Send impulse
    float out = dl.processSample (1.0f);
    // The first sample should be input (no delay output initially)

    // Process 479 more silence samples
    for (int i = 0; i < 479; ++i)
        out = dl.processSample (0.0f);

    // At sample 480, we should get the delayed impulse
    out = dl.processSample (0.0f);
    REQUIRE (std::abs (out) > 0.5f); // Impulse should appear near here
}

TEST_CASE ("DelayLine: feedback doesn't explode", "[delay][safety]")
{
    DelayLine dl;
    dl.prepare (48000.0, 512);
    dl.setDelayMs (100.0f);
    dl.setFeedback (0.95f); // Max feedback

    // Feed impulse then silence for 10 seconds
    dl.processSample (1.0f);
    for (int i = 0; i < 480000; ++i)
    {
        float out = dl.processSample (0.0f);
        REQUIRE (std::abs (out) <= 1.0f);
        REQUIRE_FALSE (std::isnan (out));
        REQUIRE_FALSE (std::isinf (out));
    }
}

TEST_CASE ("DelayLine: wraparound correctness", "[delay]")
{
    DelayLine dl;
    dl.prepare (48000.0, 512);
    dl.setDelayMs (100.0f);
    dl.setFeedback (0.0f);

    // Process enough samples to wrap the buffer several times
    for (int i = 0; i < 500000; ++i)
    {
        float in = (i % 48000 == 0) ? 1.0f : 0.0f; // Impulse every second
        float out = dl.processSample (in);
        REQUIRE_FALSE (std::isnan (out));
        REQUIRE_FALSE (std::isinf (out));
    }
}

TEST_CASE ("DelayLine: zero delay passes through", "[delay]")
{
    DelayLine dl;
    dl.prepare (48000.0, 512);
    dl.setDelayMs (0.0f);
    dl.setFeedback (0.0f);

    float out = dl.processSample (0.7f);
    REQUIRE (out == Catch::Approx (0.7f).margin (0.01f));
}

TEST_CASE ("DelayLine: DC blocker prevents DC accumulation", "[delay][safety]")
{
    DelayLine dl;
    dl.prepare (48000.0, 512);
    dl.setDelayMs (10.0f);
    dl.setFeedback (0.9f);

    // Feed DC offset
    for (int i = 0; i < 48000; ++i)
        dl.processSample (0.5f);

    // Now feed silence and check output converges to zero
    float lastOut = 0.0f;
    for (int i = 0; i < 48000; ++i)
        lastOut = dl.processSample (0.0f);

    REQUIRE (std::abs (lastOut) < 0.1f);
}

TEST_CASE ("DelayLine: handles all buffer sizes", "[delay][compat]")
{
    for (int blockSize : { 1, 64, 512, 2048, 8192 })
    {
        DelayLine dl;
        dl.prepare (48000.0, blockSize);
        dl.setDelayMs (50.0f);
        dl.setFeedback (0.5f);

        for (int i = 0; i < blockSize * 10; ++i)
        {
            float in = std::sin (static_cast<float> (i) * 0.1f);
            float out = dl.processSample (in);
            REQUIRE_FALSE (std::isnan (out));
            REQUIRE_FALSE (std::isinf (out));
        }
    }
}

TEST_CASE ("DelayLine: handles all sample rates", "[delay][compat]")
{
    for (double sr : { 44100.0, 48000.0, 96000.0, 192000.0 })
    {
        DelayLine dl;
        dl.prepare (sr, 512);
        dl.setDelayMs (10.0f);
        dl.setFeedback (0.0f);

        // Send impulse and verify it comes back at approximately the right time
        dl.processSample (1.0f);
        int delaySamples = static_cast<int> (10.0f * static_cast<float> (sr) / 1000.0f);
        for (int i = 0; i < delaySamples - 1; ++i)
            dl.processSample (0.0f);

        float out = dl.processSample (0.0f);
        REQUIRE (std::abs (out) > 0.3f);
    }
}

TEST_CASE ("DelayLine: silence in produces silence out", "[delay][safety]")
{
    DelayLine dl;
    dl.prepare (48000.0, 512);
    dl.setDelayMs (100.0f);
    dl.setFeedback (0.5f);

    float maxOut = 0.0f;
    for (int i = 0; i < 48000; ++i)
    {
        float out = dl.processSample (0.0f);
        maxOut = std::max (maxOut, std::abs (out));
    }
    REQUIRE (maxOut < 0.0001f);
}

TEST_CASE ("DelayLine: reset clears state", "[delay]")
{
    DelayLine dl;
    dl.prepare (48000.0, 512);
    dl.setDelayMs (10.0f);
    dl.setFeedback (0.9f);

    // Process signal to fill buffer
    for (int i = 0; i < 4800; ++i)
        dl.processSample (std::sin (static_cast<float> (i) * 0.1f));

    dl.reset();

    // After reset, silence in should give silence out
    float maxOut = 0.0f;
    for (int i = 0; i < 4800; ++i)
    {
        float out = dl.processSample (0.0f);
        maxOut = std::max (maxOut, std::abs (out));
    }
    REQUIRE (maxOut < 0.001f);
}
