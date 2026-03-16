#pragma once
#include "OnePoleFilter.h"
#include <cmath>

class LoFiProcessor
{
public:
    void prepare (double sampleRate)
    {
        sampleRate_ = sampleRate;
        lpf_.prepare (sampleRate);
        reset();
    }

    void reset()
    {
        holdSample_ = 0.0f;
        holdCounter_ = 0;
        lpf_.reset();
    }

    void setBitDepth (int bits)
    {
        bitDepth_ = bits;
        float levels = static_cast<float> (1 << bits);
        quantScale_ = levels - 1.0f;
        invQuantScale_ = 1.0f / quantScale_;
    }

    void setSampleRateDivider (int divider)
    {
        srDivider_ = divider;
    }

    void setLPFCutoff (float hz)
    {
        lpf_.setCutoff (hz);
    }

    float processSample (float input)
    {
        // Sample rate divider (sample-and-hold)
        if (++holdCounter_ >= srDivider_)
        {
            holdCounter_ = 0;
            holdSample_ = input;
        }

        float sample = holdSample_;

        // Bit depth reduction
        if (bitDepth_ < 16)
        {
            sample = std::round (sample * quantScale_) * invQuantScale_;
        }

        // Anti-alias LPF
        sample = lpf_.processLowPass (sample);

        return sample;
    }

private:
    double sampleRate_ = 48000.0;
    int bitDepth_ = 16;
    int srDivider_ = 1;
    float quantScale_ = 65535.0f;
    float invQuantScale_ = 1.0f / 65535.0f;
    float holdSample_ = 0.0f;
    int holdCounter_ = 0;
    OnePoleFilter lpf_;
};
