#include "FrequencyShifter.h"

void FrequencyShifter::prepare (double sampleRate)
{
    sampleRate_ = sampleRate;

    // Hilbert transform all-pass coefficients (designed for wideband 90° split)
    // These coefficients give good phase accuracy from ~20Hz to ~20kHz
    static const float coeffsI[NUM_ALLPASS] = {
        0.6923878f, 0.9360654322959f, 0.9882295226860f,
        0.9987488452737f, 0.9998494033061f, 0.9999899264795f
    };
    static const float coeffsQ[NUM_ALLPASS] = {
        0.4021921162426f, 0.8561710882420f, 0.9722909545651f,
        0.9952884791278f, 0.9995192111950f, 0.9999518138725f
    };

    for (int i = 0; i < NUM_ALLPASS; ++i)
    {
        hilbertI_[static_cast<size_t> (i)].coeff = coeffsI[i];
        hilbertQ_[static_cast<size_t> (i)].coeff = coeffsQ[i];
    }

    reset();
}

void FrequencyShifter::reset()
{
    for (auto& stage : hilbertI_) stage.reset();
    for (auto& stage : hilbertQ_) stage.reset();
    oscPhase_ = 0.0;
}

void FrequencyShifter::setShiftHz (float hz)
{
    shiftHz_ = hz;
    oscPhaseInc_ = 2.0 * 3.14159265358979323846 * static_cast<double> (hz) / sampleRate_;
}

float FrequencyShifter::processSample (float input)
{
    // Run through Hilbert transform (I and Q paths)
    float inPhase = input;
    float quadPhase = input;

    for (auto& stage : hilbertI_)
        inPhase = stage.process (inPhase);
    for (auto& stage : hilbertQ_)
        quadPhase = stage.process (quadPhase);

    // Modulate with complex oscillator for single-sideband shift
    float cosOsc = static_cast<float> (std::cos (oscPhase_));
    float sinOsc = static_cast<float> (std::sin (oscPhase_));

    // Upper sideband: I*cos - Q*sin
    float output = inPhase * cosOsc - quadPhase * sinOsc;

    // Advance oscillator
    oscPhase_ += oscPhaseInc_;
    if (oscPhase_ > 6.283185307) oscPhase_ -= 6.283185307;
    if (oscPhase_ < 0.0) oscPhase_ += 6.283185307;

    return output;
}
