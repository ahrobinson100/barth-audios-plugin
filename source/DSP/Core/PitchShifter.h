#pragma once
#include <vector>
#include <cmath>
#include <cstring>

class PitchShifter
{
public:
    static constexpr int BUFFER_SIZE = 65536; // 2^16, ~1.36s at 48kHz
    static constexpr int BUFFER_MASK = BUFFER_SIZE - 1;
    static constexpr int HANN_LUT_SIZE = 4096;
    static constexpr int HANN_LUT_MASK = HANN_LUT_SIZE - 1;

    PitchShifter();

    void prepare (double sampleRate, int maxBlockSize);
    void reset();

    void setPitchSemitones (float semitones);
    void setPitchRatio (float ratio);
    void setGrainSizeMs (float ms);
    void setPortamento (float speed); // 0..1

    float processSample (float input);
    float getLatencySamples() const { return grainSizeSamples_ * 0.5f; }

private:
    float hermiteInterp (double pos) const;
    void initHannLUT();

    std::vector<float> buffer_;
    int writePtr_ = 0;

    double readPtrA_ = 0.0;
    double readPtrB_ = 0.0;
    double crossfadePhase_ = 0.0;

    float currentRatio_ = 1.0f;
    float targetRatio_ = 1.0f;
    float portaCoeff_ = 1.0f; // 1.0 = instant

    float grainSizeSamples_ = 1920.0f; // 40ms at 48kHz
    double sampleRate_ = 48000.0;

    std::array<float, HANN_LUT_SIZE> hannFadeIn_{};
    std::array<float, HANN_LUT_SIZE> hannFadeOut_{};
};
