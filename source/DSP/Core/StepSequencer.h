#pragma once
#include <array>
#include <cstdlib>

class StepSequencer
{
public:
    enum class Mode { Forward, Backward, Random, PingPong, TwoStep, ThreeStep, FourStep };

    void prepare (double sampleRate, int maxBlockSize);
    void reset();

    void setStepPitch (int step, float semitones); // 0-3
    void setMode (Mode mode);
    void setRateHz (float hz);             // Free-run speed
    void setHostSync (bool enabled);
    void setDivision (float noteFraction); // 0.25 = 1/4 note
    void setEnabled (bool enabled);

    // Call per-sample in processBlock. Returns pitch offset in semitones.
    float processSample (double bpm, double ppqPosition, bool isPlaying);

    // Trigger next step externally (envelope follower)
    void triggerNextStep();

    int getCurrentStep() const { return currentStep_; }
    bool isEnabled() const { return enabled_; }

private:
    void advanceStep();

    std::array<float, 4> stepPitch_ = { 0.0f, 0.0f, 0.0f, 0.0f };
    Mode mode_ = Mode::Forward;
    int currentStep_ = 0;
    int direction_ = 1; // For ping-pong
    bool enabled_ = false;

    // Free-run timing
    float rateHz_ = 2.0f;
    double phaseSamples_ = 0.0;
    double samplesPerStep_ = 24000.0; // at 48kHz, 2Hz
    double sampleRate_ = 48000.0;

    // Host sync
    bool hostSync_ = false;
    float division_ = 0.25f; // 1/4 note
    double lastBeatPos_ = -1.0;
};
