#include "TimeStretcher.h"

void TimeStretcher::prepare (double sampleRate, int /*maxBlockSize*/)
{
    sampleRate_ = sampleRate;
    grainSizeSamples_ = 2048.0f;
    initHannLUT();
    reset();
}

void TimeStretcher::reset()
{
    buffer_.fill (0.0f);
    writePtr_ = 0;
    readPtrA_ = 0.0;
    readPtrB_ = grainSizeSamples_ * 0.5;
    crossfadePhase_ = 0.0;
}

void TimeStretcher::initHannLUT()
{
    for (int i = 0; i < HANN_LUT_SIZE; ++i)
    {
        float phase = static_cast<float> (i) / static_cast<float> (HANN_LUT_SIZE);
        float angle = 1.5707963f * phase;
        float s = std::sin (angle);
        float c = std::cos (angle);
        hannFadeIn_[i] = s * s;
        hannFadeOut_[i] = c * c;
    }
}

void TimeStretcher::setStretchRatio (float ratio)
{
    stretchRatio_ = (ratio < 0.25f) ? 0.25f : ((ratio > 4.0f) ? 4.0f : ratio);
}

float TimeStretcher::processSample (float input)
{
    // Write to circular buffer at normal speed
    buffer_[writePtr_] = input;
    writePtr_ = (writePtr_ + 1) & BUFFER_MASK;

    if (std::abs (stretchRatio_ - 1.0f) < 0.001f)
        return input; // Bypass when no stretch

    // Read speed = 1/stretchRatio (slower read = stretch)
    double readSpeed = 1.0 / static_cast<double> (stretchRatio_);

    readPtrA_ += readSpeed;
    readPtrB_ += readSpeed;
    if (readPtrA_ >= BUFFER_SIZE) readPtrA_ -= BUFFER_SIZE;
    if (readPtrA_ < 0.0) readPtrA_ += BUFFER_SIZE;
    if (readPtrB_ >= BUFFER_SIZE) readPtrB_ -= BUFFER_SIZE;
    if (readPtrB_ < 0.0) readPtrB_ += BUFFER_SIZE;

    float ratioDiff = std::abs (1.0f / stretchRatio_ - 1.0f);
    if (ratioDiff < 0.0001f) ratioDiff = 0.0001f;

    crossfadePhase_ += static_cast<double> (ratioDiff) / static_cast<double> (grainSizeSamples_);

    if (crossfadePhase_ >= 1.0)
    {
        crossfadePhase_ -= 1.0;
        readPtrB_ = readPtrA_ - static_cast<double> (grainSizeSamples_) * 0.5 * readSpeed;
        if (readPtrB_ < 0.0) readPtrB_ += BUFFER_SIZE;
        std::swap (readPtrA_, readPtrB_);
    }

    float sampleA = hermiteInterp (readPtrA_);
    float sampleB = hermiteInterp (readPtrB_);

    int lutIndex = static_cast<int> (crossfadePhase_ * HANN_LUT_SIZE) & HANN_LUT_MASK;
    return sampleA * hannFadeIn_[lutIndex] + sampleB * hannFadeOut_[lutIndex];
}

float TimeStretcher::hermiteInterp (double pos) const
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
