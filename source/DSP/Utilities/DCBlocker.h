#pragma once

class DCBlocker
{
public:
    void prepare (double sampleRate)
    {
        // First-order HPF at ~5Hz: R = 1 - (2*pi*5/sr)
        coefficient_ = 1.0f - (6.2831853f * 5.0f / static_cast<float> (sampleRate));
        reset();
    }

    void reset()
    {
        xPrev_ = 0.0f;
        yPrev_ = 0.0f;
    }

    float processSample (float input)
    {
        float output = input - xPrev_ + coefficient_ * yPrev_;
        xPrev_ = input;
        yPrev_ = output;
        return output;
    }

private:
    float coefficient_ = 0.995f;
    float xPrev_ = 0.0f;
    float yPrev_ = 0.0f;
};
