#pragma once
#include "../Utilities/DCBlocker.h"
#include "../Utilities/SoftClipper.h"
#include <array>
#include <cmath>

class DelayLine
{
public:
    static constexpr int MAX_DELAY_SAMPLES = 262144; // 2^18, ~5.46s at 48kHz
    static constexpr int BUFFER_MASK = MAX_DELAY_SAMPLES - 1;

    void prepare (double sampleRate, int maxBlockSize);
    void reset();

    void setDelayMs (float ms);
    void setFeedback (float fb); // 0..0.95
    void setDelaySamples (float samples);

    float processSample (float input);
    float getDelaySamples() const { return delaySamples_; }

private:
    float hermiteInterp (double pos) const;

    std::array<float, MAX_DELAY_SAMPLES> buffer_{};
    int writePtr_ = 0;
    float delaySamples_ = 0.0f;
    float feedback_ = 0.0f;
    double sampleRate_ = 48000.0;

    DCBlocker dcBlocker_;
    SoftClipper feedbackClipper_;
    float feedbackSample_ = 0.0f;
};
