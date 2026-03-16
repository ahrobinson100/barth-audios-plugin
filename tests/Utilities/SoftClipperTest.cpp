#include "../helpers/test_helpers.h"
#include <DSP/Utilities/SoftClipper.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

TEST_CASE ("SoftClipper: below threshold passthrough", "[softclip]")
{
    SoftClipper clipper;
    clipper.setThreshold (0.95f);

    REQUIRE (clipper.processSample (0.5f) == Catch::Approx (0.5f));
    REQUIRE (clipper.processSample (0.0f) == Catch::Approx (0.0f));
    REQUIRE (clipper.processSample (-0.5f) == Catch::Approx (-0.5f));
    REQUIRE (clipper.processSample (0.94f) == Catch::Approx (0.94f));
}

TEST_CASE ("SoftClipper: above threshold saturated", "[softclip]")
{
    SoftClipper clipper;
    clipper.setThreshold (0.95f);

    float out = clipper.processSample (2.0f);
    REQUIRE (out > 0.0f);
    REQUIRE (out <= 0.95f);

    float out2 = clipper.processSample (10.0f);
    REQUIRE (out2 <= 0.95f);
}

TEST_CASE ("SoftClipper: symmetric positive/negative", "[softclip]")
{
    SoftClipper clipper;
    clipper.setThreshold (0.95f);

    float pos = clipper.processSample (2.0f);
    float neg = clipper.processSample (-2.0f);
    REQUIRE (pos == Catch::Approx (-neg).margin (0.0001f));
}

TEST_CASE ("SoftClipper: no NaN/Inf", "[softclip][safety]")
{
    SoftClipper clipper;
    clipper.setThreshold (0.95f);

    float extremes[] = { 0.0f, 1.0f, -1.0f, 100.0f, -100.0f, 1e6f, -1e6f };
    for (float val : extremes)
    {
        float out = clipper.processSample (val);
        REQUIRE_FALSE (std::isnan (out));
        REQUIRE_FALSE (std::isinf (out));
    }
}
