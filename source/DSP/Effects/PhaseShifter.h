#pragma once
#include <array>
#include <cmath>

class PhaseShifter
{
public:
    enum class StereoMode { Mono, Stereo, Tandem };

    void prepare (double sampleRate);
    void reset();

    void setRate (float hz);     // LFO rate
    void setDepth (float depth); // 0..1
    void setStereoMode (StereoMode mode);

    // Process a single channel. channel: 0=L, 1=R
    float processSample (float input, int channel);

private:
    static constexpr int NUM_STAGES = 6;

    struct AllPassStage
    {
        float state = 0.0f;

        float process (float input, float coeff)
        {
            float output = coeff * input + state;
            state = input - coeff * output;
            return output;
        }

        void reset() { state = 0.0f; }
    };

    // Two channels, each with NUM_STAGES all-pass filters
    std::array<std::array<AllPassStage, NUM_STAGES>, 2> stages_{};

    double sampleRate_ = 48000.0;
    double lfoPhase_ = 0.0;
    double lfoPhaseInc_ = 0.0;
    float depth_ = 0.5f;
    StereoMode stereoMode_ = StereoMode::Stereo;

    // Notch frequency range (Hz)
    float minFreq_ = 200.0f;
    float maxFreq_ = 4000.0f;
};
