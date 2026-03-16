#pragma once
#include <array>
#include <cmath>

// Single-sideband frequency shifter via Hilbert transform approximation.
// Uses cascaded all-pass filters for ~90° phase split.
class FrequencyShifter
{
public:
    void prepare (double sampleRate);
    void reset();
    void setShiftHz (float hz);

    float processSample (float input);

private:
    // 6-stage all-pass Hilbert transform approximation
    static constexpr int NUM_ALLPASS = 6;

    struct AllPassStage
    {
        float coeff = 0.0f;
        float state0 = 0.0f;
        float state1 = 0.0f;

        float process (float input)
        {
            float output = coeff * (input - state1) + state0;
            state0 = input;
            state1 = output;
            if (! (state0 < -1.0e-8f || state0 > 1.0e-8f)) state0 = 0.0f;
            if (! (state1 < -1.0e-8f || state1 > 1.0e-8f)) state1 = 0.0f;
            return output;
        }

        void reset()
        {
            state0 = 0.0f;
            state1 = 0.0f;
        }
    };

    std::array<AllPassStage, NUM_ALLPASS> hilbertI_{}; // In-phase path
    std::array<AllPassStage, NUM_ALLPASS> hilbertQ_{}; // Quadrature path

    float shiftHz_ = 0.0f;
    double sampleRate_ = 48000.0;

    // Quadrature oscillator (incremental rotation, no per-sample sin/cos)
    float oscCos_ = 1.0f;
    float oscSin_ = 0.0f;
    float oscCosInc_ = 1.0f;
    float oscSinInc_ = 0.0f;
    int oscCounter_ = 0;  // renormalize periodically
};
