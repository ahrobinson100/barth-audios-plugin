#include "../helpers/test_helpers.h"
#include <DSP/Utilities/LoFiProcessor.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

TEST_CASE ("LoFiProcessor: 16-bit no divider passes through", "[lofi]")
{
    LoFiProcessor lofi;
    lofi.prepare (48000.0);
    lofi.setBitDepth (16);
    lofi.setSampleRateDivider (1);
    lofi.setLPFCutoff (20000.0f);

    float out = lofi.processSample (0.5f);
    REQUIRE (out == Catch::Approx (0.5f).margin (0.01f));
}

TEST_CASE ("LoFiProcessor: bit depth reduction quantizes", "[lofi]")
{
    LoFiProcessor lofi;
    lofi.prepare (48000.0);
    lofi.setBitDepth (4); // 16 levels
    lofi.setSampleRateDivider (1);
    lofi.setLPFCutoff (20000.0f);

    // With only 16 levels, output should be quantized
    float out1 = lofi.processSample (0.1f);
    float out2 = lofi.processSample (0.11f);
    float out3 = lofi.processSample (0.12f);

    // Some of these should map to the same quantized value
    // At 4 bits: step size = 1/15 ≈ 0.067, so 0.1 and 0.11 likely same bin
    REQUIRE (out1 == Catch::Approx (out2).margin (0.001f));
}

TEST_CASE ("LoFiProcessor: SR divider reduces effective sample rate", "[lofi]")
{
    LoFiProcessor lofi;
    lofi.prepare (48000.0);
    lofi.setBitDepth (16);
    lofi.setSampleRateDivider (4);
    lofi.setLPFCutoff (20000.0f);

    // Process a ramp - SR divider should produce staircase
    float prev = lofi.processSample (0.0f);
    int sameCount = 0;

    for (int i = 1; i < 100; ++i)
    {
        float in = static_cast<float> (i) / 100.0f;
        float out = lofi.processSample (in);
        if (std::abs (out - prev) < 0.001f) // LPF smooths, but should still see hold
            ++sameCount;
        prev = out;
    }

    // With divider 4, about 75% of samples should be held
    REQUIRE (sameCount > 30);
}

TEST_CASE ("LoFiProcessor: no NaN/Inf", "[lofi][safety]")
{
    LoFiProcessor lofi;
    lofi.prepare (48000.0);
    lofi.setBitDepth (4);
    lofi.setSampleRateDivider (16);
    lofi.setLPFCutoff (200.0f);

    for (int i = 0; i < 48000; ++i)
    {
        float out = lofi.processSample (std::sin (static_cast<float> (i) * 0.05f));
        REQUIRE_FALSE (std::isnan (out));
        REQUIRE_FALSE (std::isinf (out));
    }
}
