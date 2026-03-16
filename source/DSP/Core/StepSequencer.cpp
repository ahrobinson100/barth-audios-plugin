#include "StepSequencer.h"
#include <cmath>

void StepSequencer::prepare (double sampleRate, int /*maxBlockSize*/)
{
    sampleRate_ = sampleRate;
    samplesPerStep_ = sampleRate / rateHz_;
    reset();
}

void StepSequencer::reset()
{
    currentStep_ = 0;
    direction_ = 1;
    phaseSamples_ = 0.0;
    lastBeatPos_ = -1.0;
}

void StepSequencer::setStepPitch (int step, float semitones)
{
    if (step >= 0 && step < 4)
        stepPitch_[static_cast<size_t> (step)] = semitones;
}

void StepSequencer::setMode (Mode mode)
{
    mode_ = mode;
}

void StepSequencer::setRateHz (float hz)
{
    rateHz_ = hz;
    if (sampleRate_ > 0.0)
        samplesPerStep_ = sampleRate_ / static_cast<double> (hz);
}

void StepSequencer::setHostSync (bool enabled)
{
    hostSync_ = enabled;
}

void StepSequencer::setDivision (float noteFraction)
{
    division_ = noteFraction;
}

void StepSequencer::setEnabled (bool enabled)
{
    enabled_ = enabled;
    if (! enabled)
        reset();
}

float StepSequencer::processSample (double bpm, double ppqPosition, bool isPlaying)
{
    if (! enabled_)
        return 0.0f;

    if (hostSync_ && isPlaying && bpm > 0.0)
    {
        // Host sync: advance step at beat divisions
        double beatsPerStep = division_ * 4.0; // division_ is note fraction (0.25 = quarter)
        double stepPos = ppqPosition / beatsPerStep;
        double currentBeat = std::floor (stepPos);

        if (currentBeat != lastBeatPos_ && lastBeatPos_ >= 0.0)
        {
            advanceStep();
        }
        lastBeatPos_ = currentBeat;
    }
    else
    {
        // Free-run timing
        phaseSamples_ += 1.0;
        if (phaseSamples_ >= samplesPerStep_)
        {
            phaseSamples_ -= samplesPerStep_;
            advanceStep();
        }
    }

    return stepPitch_[static_cast<size_t> (currentStep_)];
}

void StepSequencer::triggerNextStep()
{
    if (enabled_)
        advanceStep();
}

void StepSequencer::advanceStep()
{
    switch (mode_)
    {
        case Mode::Forward:
            currentStep_ = (currentStep_ + 1) % 4;
            break;

        case Mode::Backward:
            currentStep_ = (currentStep_ + 3) % 4; // -1 mod 4
            break;

        case Mode::Random:
            currentStep_ = std::rand() % 4;
            break;

        case Mode::PingPong:
            currentStep_ += direction_;
            if (currentStep_ >= 3)
            {
                currentStep_ = 3;
                direction_ = -1;
            }
            else if (currentStep_ <= 0)
            {
                currentStep_ = 0;
                direction_ = 1;
            }
            break;

        case Mode::TwoStep:
            currentStep_ = (currentStep_ + 1) % 2;
            break;

        case Mode::ThreeStep:
            currentStep_ = (currentStep_ + 1) % 3;
            break;

        case Mode::FourStep:
            currentStep_ = (currentStep_ + 1) % 4;
            break;
    }
}
