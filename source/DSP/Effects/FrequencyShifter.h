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
    double oscPhase_ = 0.0;
    double oscPhaseInc_ = 0.0;
};
