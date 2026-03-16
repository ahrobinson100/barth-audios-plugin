#pragma once
#include <vector>
#include <cmath>

// Pitch-independent time stretcher using overlap-add with two read pointers.
// Stretch ratio: 0.25 (4x faster) to 4.0 (4x slower). 1.0 = no change.
class TimeStretcher
{
public:
    static constexpr int BUFFER_SIZE = 131072; // 2^17
    static constexpr int BUFFER_MASK = BUFFER_SIZE - 1;
    static constexpr int HANN_LUT_SIZE = 4096;
    static constexpr int HANN_LUT_MASK = HANN_LUT_SIZE - 1;

    void prepare (double sampleRate, int maxBlockSize);
    void reset();

    void setStretchRatio (float ratio); // 0.25..4.0, 1.0 = no change

    float processSample (float input);

private:
    float hermiteInterp (double pos) const;
    void initHannLUT();

    std::vector<float> buffer_;
    int writePtr_ = 0;
    double readPtrA_ = 0.0;
    double readPtrB_ = 0.0;
    double crossfadePhase_ = 0.0;
    float grainSizeSamples_ = 2048.0f;
    float stretchRatio_ = 1.0f;
    double sampleRate_ = 48000.0;

    std::array<float, HANN_LUT_SIZE> hannFadeIn_{};
    std::array<float, HANN_LUT_SIZE> hannFadeOut_{};
};
