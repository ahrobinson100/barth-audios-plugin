#include "Distortion.h"

void Distortion::prepare (double sampleRate)
{
    sampleRate_ = sampleRate;
    updateBassCoeffs();
    reset();
}

void Distortion::reset()
{
    bassState_ = 0.0f;
}

void Distortion::setDrive (float drive)
{
    drive_ = (drive < 0.0f) ? 0.0f : ((drive > 1.0f) ? 1.0f : drive);
}

void Distortion::setBassBoostDb (float db)
{
    if (db < -12.0f) db = -12.0f;
    if (db > 12.0f) db = 12.0f;
    bassBoostGain_ = std::pow (10.0f, db / 20.0f);
    updateBassCoeffs();
}

void Distortion::updateBassCoeffs()
{
    // Low shelf at ~200Hz
    float freq = 200.0f;
    float w = 6.2831853f * freq / static_cast<float> (sampleRate_);
    bassCoeff_ = w / (1.0f + w);
    bassMix_ = bassBoostGain_ - 1.0f; // Extra gain for low frequencies
}

float Distortion::processSample (float input)
{
    if (drive_ < 0.001f && std::abs (bassMix_) < 0.001f)
        return input;

    float sample = input;

    // Bass boost (parallel low shelf)
    bassState_ += bassCoeff_ * (sample - bassState_);
    if (! (bassState_ < -1.0e-8f || bassState_ > 1.0e-8f)) bassState_ = 0.0f;
    sample += bassState_ * bassMix_;

    // Waveshaper: blend between clean and tanh saturation based on drive
    if (drive_ > 0.001f)
    {
        float preGain = 1.0f + drive_ * 10.0f; // Up to 11x gain into saturator
        float shaped = std::tanh (sample * preGain);
        // Compensate output level
        float postGain = 1.0f / (1.0f + drive_ * 2.0f);
        sample = shaped * postGain;
    }

    return sample;
}
