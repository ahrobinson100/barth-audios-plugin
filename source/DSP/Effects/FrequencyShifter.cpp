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
    oscCos_ = 1.0f;
    oscSin_ = 0.0f;
    oscCounter_ = 0;
}

void FrequencyShifter::setShiftHz (float hz)
{
    shiftHz_ = hz;
    double phaseInc = 2.0 * 3.14159265358979323846 * static_cast<double> (hz) / sampleRate_;
    oscCosInc_ = static_cast<float> (std::cos (phaseInc));
    oscSinInc_ = static_cast<float> (std::sin (phaseInc));
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

    // Upper sideband: I*cos - Q*sin
    float output = inPhase * oscCos_ - quadPhase * oscSin_;

    // Advance quadrature oscillator via rotation
    float newCos = oscCos_ * oscCosInc_ - oscSin_ * oscSinInc_;
    float newSin = oscSin_ * oscCosInc_ + oscCos_ * oscSinInc_;
    oscCos_ = newCos;
    oscSin_ = newSin;

    // Renormalize every 512 samples to prevent drift
    if (++oscCounter_ >= 512)
    {
        oscCounter_ = 0;
        float mag = oscCos_ * oscCos_ + oscSin_ * oscSin_;
        if (mag > 0.0f)
        {
            float invMag = 1.0f / std::sqrt (mag);
            oscCos_ *= invMag;
            oscSin_ *= invMag;
        }
    }

    return output;
}
