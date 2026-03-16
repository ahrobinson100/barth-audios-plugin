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

    // Each call returns current step pitch, then advances for next call
    float val0 = seq.processSample (120.0, 0.0, false);
    REQUIRE (val0 == Catch::Approx (0.0f));   // Step 0
    REQUIRE (seq.getCurrentStep() == 1);       // Advanced to 1

    float val1 = seq.processSample (120.0, 0.0, false);
    REQUIRE (val1 == Catch::Approx (7.0f));   // Step 1
    REQUIRE (seq.getCurrentStep() == 2);       // Advanced to 2

    float val2 = seq.processSample (120.0, 0.0, false);
    REQUIRE (val2 == Catch::Approx (12.0f));  // Step 2
    REQUIRE (seq.getCurrentStep() == 3);       // Advanced to 3

    float val3 = seq.processSample (120.0, 0.0, false);
    REQUIRE (val3 == Catch::Approx (5.0f));   // Step 3
    REQUIRE (seq.getCurrentStep() == 0);       // Wraps back to 0
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

TEST_CASE ("StepSequencer: random mode visits all steps", "[seq]")
{
    StepSequencer seq;
    seq.prepare (48000.0, 512);
    seq.setEnabled (true);
    seq.setMode (StepSequencer::Mode::Random);
    seq.setRateHz (48000.0f);

    std::array<bool, 4> visited = { false, false, false, false };

    for (int i = 0; i < 1000; ++i)
    {
        seq.processSample (120.0, 0.0, false);
        int step = seq.getCurrentStep();
        REQUIRE (step >= 0);
        REQUIRE (step < 4);
        visited[static_cast<size_t> (step)] = true;
    }

    for (int i = 0; i < 4; ++i)
        REQUIRE (visited[static_cast<size_t> (i)]);
}

TEST_CASE ("StepSequencer: two-step mode", "[seq]")
{
    StepSequencer seq;
    seq.prepare (48000.0, 512);
    seq.setEnabled (true);
    seq.setMode (StepSequencer::Mode::TwoStep);
    seq.setRateHz (48000.0f);

    REQUIRE (seq.getCurrentStep() == 0);
    seq.processSample (120.0, 0.0, false);
    REQUIRE (seq.getCurrentStep() == 1);
    seq.processSample (120.0, 0.0, false);
    REQUIRE (seq.getCurrentStep() == 0);
    seq.processSample (120.0, 0.0, false);
    REQUIRE (seq.getCurrentStep() == 1);
}

TEST_CASE ("StepSequencer: three-step mode", "[seq]")
{
    StepSequencer seq;
    seq.prepare (48000.0, 512);
    seq.setEnabled (true);
    seq.setMode (StepSequencer::Mode::ThreeStep);
    seq.setRateHz (48000.0f);

    REQUIRE (seq.getCurrentStep() == 0);
    seq.processSample (120.0, 0.0, false);
    REQUIRE (seq.getCurrentStep() == 1);
    seq.processSample (120.0, 0.0, false);
    REQUIRE (seq.getCurrentStep() == 2);
    seq.processSample (120.0, 0.0, false);
    REQUIRE (seq.getCurrentStep() == 0);
}

TEST_CASE ("StepSequencer: four-step mode", "[seq]")
{
    StepSequencer seq;
    seq.prepare (48000.0, 512);
    seq.setEnabled (true);
    seq.setMode (StepSequencer::Mode::FourStep);
    seq.setRateHz (48000.0f);

    REQUIRE (seq.getCurrentStep() == 0);
    seq.processSample (120.0, 0.0, false);
    REQUIRE (seq.getCurrentStep() == 1);
    seq.processSample (120.0, 0.0, false);
    REQUIRE (seq.getCurrentStep() == 2);
    seq.processSample (120.0, 0.0, false);
    REQUIRE (seq.getCurrentStep() == 3);
    seq.processSample (120.0, 0.0, false);
    REQUIRE (seq.getCurrentStep() == 0);
}

TEST_CASE ("StepSequencer: host sync timing", "[seq]")
{
    StepSequencer seq;
    seq.prepare (48000.0, 512);
    seq.setEnabled (true);
    seq.setMode (StepSequencer::Mode::Forward);
    seq.setHostSync (true);
    seq.setDivision (0.25f); // 1/4 note
    seq.setStepPitch (0, 0.0f);
    seq.setStepPitch (1, 5.0f);

    // At 120 BPM, 1/4 note = 0.5 sec = 24000 samples
    // beatsPerStep = 0.25 * 4 = 1.0
    double bpm = 120.0;
    double ppq = 0.0;
    double ppqInc = bpm / (60.0 * 48000.0);

    REQUIRE (seq.getCurrentStep() == 0);

    // Advance past 1 beat
    for (int i = 0; i < 24001; ++i)
    {
        seq.processSample (bpm, ppq, true);
        ppq += ppqInc;
    }

    // Should have advanced at least once
    REQUIRE (seq.getCurrentStep() >= 1);
}
