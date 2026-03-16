#include "DelayLine.h"

void DelayLine::prepare (double sampleRate, int /*maxBlockSize*/)
{
    sampleRate_ = sampleRate;
    dcBlocker_.prepare (sampleRate);
    feedbackClipper_.setThreshold (0.95f);
    reset();
}

void DelayLine::reset()
{
    buffer_.fill (0.0f);
    writePtr_ = 0;
    feedbackSample_ = 0.0f;
    dcBlocker_.reset();
}

void DelayLine::setDelayMs (float ms)
{
    delaySamples_ = ms * static_cast<float> (sampleRate_) / 1000.0f;
    if (delaySamples_ >= static_cast<float> (MAX_DELAY_SAMPLES - 1))
        delaySamples_ = static_cast<float> (MAX_DELAY_SAMPLES - 2);
    if (delaySamples_ < 0.0f)
        delaySamples_ = 0.0f;
}

void DelayLine::setDelaySamples (float samples)
{
    delaySamples_ = samples;
    if (delaySamples_ >= static_cast<float> (MAX_DELAY_SAMPLES - 1))
        delaySamples_ = static_cast<float> (MAX_DELAY_SAMPLES - 2);
    if (delaySamples_ < 0.0f)
        delaySamples_ = 0.0f;
}

void DelayLine::setFeedback (float fb)
{
    feedback_ = (fb > 0.95f) ? 0.95f : fb;
    if (feedback_ < 0.0f) feedback_ = 0.0f;
}

float DelayLine::processSample (float input)
{
    // Write input + feedback to buffer
    float writeVal = input + feedbackSample_;
    buffer_[writePtr_] = writeVal;
    writePtr_ = (writePtr_ + 1) & BUFFER_MASK;

    // Read from delay position
    if (delaySamples_ < 1.0f)
        return input; // No delay, pass through

    double readPos = static_cast<double> (writePtr_) - static_cast<double> (delaySamples_);
    if (readPos < 0.0)
        readPos += MAX_DELAY_SAMPLES;

    float delayed = hermiteInterp (readPos);

    // Feedback path: DC block + soft clip
    feedbackSample_ = dcBlocker_.processSample (delayed) * feedback_;
    feedbackSample_ = feedbackClipper_.processSample (feedbackSample_);

    return delayed;
}

float DelayLine::hermiteInterp (double pos) const
{
    int idx = static_cast<int> (pos);
    float f = static_cast<float> (pos - idx);
    float y0 = buffer_[(idx - 1) & BUFFER_MASK];
    float y1 = buffer_[idx & BUFFER_MASK];
    float y2 = buffer_[(idx + 1) & BUFFER_MASK];
    float y3 = buffer_[(idx + 2) & BUFFER_MASK];
    float c1 = 0.5f * (y2 - y0);
    float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
    float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);
    return ((c3 * f + c2) * f + c1) * f + y1;
}
