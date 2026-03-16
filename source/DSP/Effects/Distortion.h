#pragma once
#include <cmath>

class Distortion
{
public:
    void prepare (double sampleRate);
    void reset();

    void setDrive (float drive);       // 0..1
    void setBassBoostDb (float db);    // -12..+12

    float processSample (float input);

private:
    double sampleRate_ = 48000.0;
    float drive_ = 0.0f;
    float bassBoostGain_ = 1.0f;

    // Bass boost: one-pole low shelf
    float bassState_ = 0.0f;
    float bassCoeff_ = 0.0f;
    float bassMix_ = 0.0f;

    void updateBassCoeffs();
};
