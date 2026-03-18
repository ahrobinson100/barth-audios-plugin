#pragma once
#include "OnePoleFilter.h"
#include <cmath>
#include <cstdint>

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

        // Bit depth reduction with TPDF dither
        // Real DACs had thermal noise that naturally dithered the quantization,
        // converting harsh stepping artifacts into a smooth noise floor.
        // TPDF (triangular PDF) dither: sum of two uniform random values
        // scaled to 1 LSB — eliminates quantization distortion entirely.
        if (bitDepth_ < 16)
        {
            float dither = (nextRandom() + nextRandom()) * invQuantScale_;
            sample = std::round (sample * quantScale_ + dither) * invQuantScale_;
        }

        // Anti-alias LPF
        sample = lpf_.processLowPass (sample);

        return sample;
    }

private:
    // Fast LCG for dither — doesn't need to be high quality,
    // just uncorrelated with the audio signal
    float nextRandom()
    {
        rngState_ = rngState_ * 1664525u + 1013904223u;
        return static_cast<float> (static_cast<int32_t> (rngState_)) / 4294967296.0f; // [-0.5, 0.5)
    }

    double sampleRate_ = 48000.0;
    int bitDepth_ = 16;
    int srDivider_ = 1;
    float quantScale_ = 65535.0f;
    float invQuantScale_ = 1.0f / 65535.0f;
    float holdSample_ = 0.0f;
    int holdCounter_ = 0;
    uint32_t rngState_ = 12345u;
    OnePoleFilter lpf_;
};
