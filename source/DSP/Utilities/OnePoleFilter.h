#pragma once
#include <cmath>

class OnePoleFilter
{
public:
    void prepare (double sampleRate)
    {
        sampleRate_ = sampleRate;
        setCutoff (cutoffHz_);
        reset();
    }

    void reset()
    {
        state_ = 0.0f;
    }

    void setCutoff (float hz)
    {
        cutoffHz_ = hz;
        if (sampleRate_ > 0.0)
        {
            float w = 6.2831853f * hz / static_cast<float> (sampleRate_);
            coefficient_ = w / (1.0f + w);
        }
    }

    float processLowPass (float input)
    {
        state_ += coefficient_ * (input - state_);
        return state_;
    }

    float processHighPass (float input)
    {
        return input - processLowPass (input);
    }

private:
    double sampleRate_ = 48000.0;
    float cutoffHz_ = 20000.0f;
    float coefficient_ = 1.0f;
    float state_ = 0.0f;
};
