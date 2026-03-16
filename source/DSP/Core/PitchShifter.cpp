#include "PitchShifter.h"

PitchShifter::PitchShifter()
{
    initHannLUT();
}

void PitchShifter::initHannLUT()
{
    for (int i = 0; i < HANN_LUT_SIZE; ++i)
    {
        float phase = static_cast<float> (i) / static_cast<float> (HANN_LUT_SIZE);
        float angle = 1.5707963f * phase; // pi/2 * phase
        float s = std::sin (angle);
        float c = std::cos (angle);
        hannFadeIn_[i] = s * s;
        hannFadeOut_[i] = c * c;
    }
}

void PitchShifter::prepare (double sampleRate, int /*maxBlockSize*/)
{
    sampleRate_ = sampleRate;
    grainSizeSamples_ = 40.0f * static_cast<float> (sampleRate) / 1000.0f;
    reset();
}

void PitchShifter::reset()
{
    buffer_.fill (0.0f);
    writePtr_ = 0;
    readPtrA_ = 0.0;
    readPtrB_ = grainSizeSamples_ * 0.5;
    crossfadePhase_ = 0.0;
    currentRatio_ = targetRatio_;
}

void PitchShifter::setPitchSemitones (float semitones)
{
    setPitchRatio (std::pow (2.0f, semitones / 12.0f));
}

void PitchShifter::setPitchRatio (float ratio)
{
    targetRatio_ = ratio;
}

void PitchShifter::setGrainSizeMs (float ms)
{
    grainSizeSamples_ = ms * static_cast<float> (sampleRate_) / 1000.0f;
    if (grainSizeSamples_ < 64.0f)
        grainSizeSamples_ = 64.0f;
}

void PitchShifter::setPortamento (float speed)
{
    // speed 0..1 maps to coefficient: 1.0 (instant) to 0.0001 (very slow)
    if (speed <= 0.0f)
        portaCoeff_ = 1.0f;
    else
        portaCoeff_ = 1.0f - speed * 0.9999f;
}

float PitchShifter::processSample (float input)
{
    // Write to circular buffer
    buffer_[writePtr_] = input;
    writePtr_ = (writePtr_ + 1) & BUFFER_MASK;

    // Smooth pitch ratio (portamento)
    currentRatio_ += portaCoeff_ * (targetRatio_ - currentRatio_);

    // Advance both read pointers at variable speed
    readPtrA_ += static_cast<double> (currentRatio_);
    readPtrB_ += static_cast<double> (currentRatio_);

    // Wrap read pointers
    if (readPtrA_ >= BUFFER_SIZE) readPtrA_ -= BUFFER_SIZE;
    if (readPtrA_ < 0.0) readPtrA_ += BUFFER_SIZE;
    if (readPtrB_ >= BUFFER_SIZE) readPtrB_ -= BUFFER_SIZE;
    if (readPtrB_ < 0.0) readPtrB_ += BUFFER_SIZE;

    // Advance crossfade phase based on pitch deviation from unity
    float ratioDiff = std::abs (currentRatio_ - 1.0f);
    if (ratioDiff < 0.0001f)
        ratioDiff = 0.0001f; // Prevent stalling at unity

    crossfadePhase_ += static_cast<double> (ratioDiff) / static_cast<double> (grainSizeSamples_);

    // Reset crossfade cycle — reposition the trailing pointer
    if (crossfadePhase_ >= 1.0)
    {
        crossfadePhase_ -= 1.0;

        // Position pointer B at half a grain behind pointer A
        readPtrB_ = readPtrA_ - static_cast<double> (grainSizeSamples_) * 0.5 * static_cast<double> (currentRatio_);
        if (readPtrB_ < 0.0)
            readPtrB_ += BUFFER_SIZE;

        // Swap pointers so the fresh one becomes primary
        std::swap (readPtrA_, readPtrB_);
    }

    // Read with Hermite interpolation
    float sampleA = hermiteInterp (readPtrA_);
    float sampleB = hermiteInterp (readPtrB_);

    // Crossfade with pre-computed Hann window LUT
    int lutIndex = static_cast<int> (crossfadePhase_ * HANN_LUT_SIZE) & HANN_LUT_MASK;
    float gainA = hannFadeIn_[lutIndex];
    float gainB = hannFadeOut_[lutIndex];

    return sampleA * gainA + sampleB * gainB;
}

float PitchShifter::hermiteInterp (double pos) const
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
