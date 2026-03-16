#include "../helpers/test_helpers.h"
#include <DSP/Core/StepSequencer.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

TEST_CASE ("StepSequencer: forward mode cycles correctly", "[seq]")
{
    StepSequencer seq;
    seq.prepare (48000.0, 512);
    seq.setEnabled (true);
    seq.setMode (StepSequencer::Mode::Forward);
    seq.setStepPitch (0, 0.0f);
    seq.setStepPitch (1, 7.0f);
    seq.setStepPitch (2, 12.0f);
    seq.setStepPitch (3, 5.0f);
    seq.setRateHz (48000.0f); // 1 sample per step for easy testing

    float val0 = seq.processSample (120.0, 0.0, false);
    REQUIRE (val0 == Catch::Approx (0.0f));

    // Process 1 sample to advance
    seq.processSample (120.0, 0.0, false);
    REQUIRE (seq.getCurrentStep() == 1);

    seq.processSample (120.0, 0.0, false);
    REQUIRE (seq.getCurrentStep() == 2);

    seq.processSample (120.0, 0.0, false);
    REQUIRE (seq.getCurrentStep() == 3);

    seq.processSample (120.0, 0.0, false);
    REQUIRE (seq.getCurrentStep() == 0); // Wraps back
}

TEST_CASE ("StepSequencer: backward mode", "[seq]")
{
    StepSequencer seq;
    seq.prepare (48000.0, 512);
    seq.setEnabled (true);
    seq.setMode (StepSequencer::Mode::Backward);
    seq.setRateHz (48000.0f);

    // Start at 0, go backward: 0 → 3 → 2 → 1 → 0
    REQUIRE (seq.getCurrentStep() == 0);

    seq.processSample (120.0, 0.0, false); // Advance
    REQUIRE (seq.getCurrentStep() == 3);

    seq.processSample (120.0, 0.0, false);
    REQUIRE (seq.getCurrentStep() == 2);

    seq.processSample (120.0, 0.0, false);
    REQUIRE (seq.getCurrentStep() == 1);

    seq.processSample (120.0, 0.0, false);
    REQUIRE (seq.getCurrentStep() == 0);
}

TEST_CASE ("StepSequencer: ping-pong mode", "[seq]")
{
    StepSequencer seq;
    seq.prepare (48000.0, 512);
    seq.setEnabled (true);
    seq.setMode (StepSequencer::Mode::PingPong);
    seq.setRateHz (48000.0f);

    // 0→1→2→3→2→1→0→1→...
    REQUIRE (seq.getCurrentStep() == 0);
    seq.processSample (120.0, 0.0, false);
    REQUIRE (seq.getCurrentStep() == 1);
    seq.processSample (120.0, 0.0, false);
    REQUIRE (seq.getCurrentStep() == 2);
    seq.processSample (120.0, 0.0, false);
    REQUIRE (seq.getCurrentStep() == 3);
    seq.processSample (120.0, 0.0, false);
    REQUIRE (seq.getCurrentStep() == 2);
    seq.processSample (120.0, 0.0, false);
    REQUIRE (seq.getCurrentStep() == 1);
    seq.processSample (120.0, 0.0, false);
    REQUIRE (seq.getCurrentStep() == 0);
}

TEST_CASE ("StepSequencer: disabled returns 0", "[seq]")
{
    StepSequencer seq;
    seq.prepare (48000.0, 512);
    seq.setEnabled (false);
    seq.setStepPitch (0, 12.0f);

    float val = seq.processSample (120.0, 0.0, false);
    REQUIRE (val == 0.0f);
}

TEST_CASE ("StepSequencer: returns correct pitch values", "[seq]")
{
    StepSequencer seq;
    seq.prepare (48000.0, 512);
    seq.setEnabled (true);
    seq.setMode (StepSequencer::Mode::Forward);
    seq.setStepPitch (0, -5.0f);
    seq.setStepPitch (1, 7.0f);
    seq.setStepPitch (2, 12.0f);
    seq.setStepPitch (3, -12.0f);
    seq.setRateHz (48000.0f);

    float v0 = seq.processSample (120.0, 0.0, false);
    REQUIRE (v0 == Catch::Approx (-5.0f));

    float v1 = seq.processSample (120.0, 0.0, false);
    REQUIRE (v1 == Catch::Approx (7.0f));

    float v2 = seq.processSample (120.0, 0.0, false);
    REQUIRE (v2 == Catch::Approx (12.0f));

    float v3 = seq.processSample (120.0, 0.0, false);
    REQUIRE (v3 == Catch::Approx (-12.0f));
}

TEST_CASE ("StepSequencer: external trigger advances step", "[seq]")
{
    StepSequencer seq;
    seq.prepare (48000.0, 512);
    seq.setEnabled (true);
    seq.setMode (StepSequencer::Mode::Forward);
    seq.setRateHz (0.1f); // Very slow rate (won't advance naturally)

    REQUIRE (seq.getCurrentStep() == 0);
    seq.triggerNextStep();
    REQUIRE (seq.getCurrentStep() == 1);
    seq.triggerNextStep();
    REQUIRE (seq.getCurrentStep() == 2);
}
