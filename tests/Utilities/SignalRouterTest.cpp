#include "../helpers/test_helpers.h"
#include <DSP/SignalRouter.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

TEST_CASE ("SignalRouter: full dry mix", "[router]")
{
    SignalRouter router;
    router.setMix (0.0f);
    router.setOutputMode (SignalRouter::OutputMode::Stereo);

    float outL, outR;
    router.processStereo (1.0f, 0.5f, 0.0f, 0.0f, outL, outR);
    REQUIRE (outL == Catch::Approx (1.0f));
    REQUIRE (outR == Catch::Approx (0.5f));
}

TEST_CASE ("SignalRouter: half mix", "[router]")
{
    SignalRouter router;
    router.setMix (0.5f);
    router.setOutputMode (SignalRouter::OutputMode::Stereo);

    float outL, outR;
    router.processStereo (1.0f, 1.0f, 0.0f, 0.0f, outL, outR);
    REQUIRE (outL == Catch::Approx (0.5f));
    REQUIRE (outR == Catch::Approx (0.5f));
}

TEST_CASE ("SignalRouter: full wet mix", "[router]")
{
    SignalRouter router;
    router.setMix (1.0f);
    router.setOutputMode (SignalRouter::OutputMode::Stereo);

    float outL, outR;
    router.processStereo (0.0f, 0.0f, 0.8f, 0.6f, outL, outR);
    REQUIRE (outL == Catch::Approx (0.8f));
    REQUIRE (outR == Catch::Approx (0.6f));
}

TEST_CASE ("SignalRouter: mono summing", "[router]")
{
    SignalRouter router;
    router.setMix (1.0f);
    router.setOutputMode (SignalRouter::OutputMode::Mono);

    float outL, outR;
    router.processStereo (0.0f, 0.0f, 0.8f, 0.4f, outL, outR);
    float expected = (0.8f + 0.4f) * 0.5f;
    REQUIRE (outL == Catch::Approx (expected));
    REQUIRE (outR == Catch::Approx (expected));
    REQUIRE (outL == outR);
}

TEST_CASE ("SignalRouter: diff mode", "[router]")
{
    SignalRouter router;
    router.setMix (1.0f);
    router.setOutputMode (SignalRouter::OutputMode::Diff);

    float outL, outR;
    router.processStereo (0.0f, 0.0f, 0.8f, 0.2f, outL, outR);
    REQUIRE (outL == Catch::Approx ((0.8f - 0.2f) * 0.5f));
    REQUIRE (outR == Catch::Approx ((0.2f - 0.8f) * 0.5f));
    REQUIRE (outL == Catch::Approx (-outR));
}

TEST_CASE ("SignalRouter: mix clamping", "[router]")
{
    SignalRouter router;
    router.setOutputMode (SignalRouter::OutputMode::Stereo);

    // Out-of-range values should be clamped
    router.setMix (-1.0f);
    float outL, outR;
    router.processStereo (1.0f, 1.0f, 0.0f, 0.0f, outL, outR);
    REQUIRE (outL == Catch::Approx (1.0f)); // full dry at mix=0

    router.setMix (5.0f);
    router.processStereo (0.0f, 0.0f, 1.0f, 1.0f, outL, outR);
    REQUIRE (outL == Catch::Approx (1.0f)); // full wet at mix=1
}
