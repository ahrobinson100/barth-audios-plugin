#pragma once
#include <cmath>

class RingModulator
{
public:
    void prepare (double sampleRate);
    void reset();
    void setFrequency (float hz);
    void setDepth (float depth); // 0..1

    float processSample (float input);

private:
    double sampleRate_ = 48000.0;
    double phase_ = 0.0;
    double phaseInc_ = 0.0;
    float depth_ = 1.0f;
};
