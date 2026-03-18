#pragma once
#include <array>
#include <vector>
#include <cmath>
#include <cstring>
#include "../Utilities/LoFiProcessor.h"

// Pitch shifter using variable-rate delay line with signal-aware splice
// detection. Based on the same principle as the Eventide H949 ALG-3
// and Fischer/Barth Audios (1978): a single read pointer runs at a different
// rate to the write pointer, and when it runs out of room it is repositioned
// at a signal-aligned location with a brief crossfade.
//
// Most of the time only ONE pointer contributes to output (no comb filtering).
// Splices are brief and signal-aligned, making them inaudible.
//
// Four splice modes available for comparison:
//   Fischer         — THE REAL ONE: detect fundamental, jump by integer periods,
//                     splice at zero crossing (Studio Magazin 1981 description)
//   Autocorrelation — pitch-period-aligned jumps (H949 ALG-3 style)
//   ZeroCrossing    — splice at zero crossings only (no pitch detection)
//   Correlation     — cross-correlate old and new positions to find best offset
class PitchShifter
{
public:
    static constexpr int BUFFER_SIZE = 65536; // 2^16
    static constexpr int BUFFER_MASK = BUFFER_SIZE - 1;

    enum class SpliceMode
    {
        Fischer,          // Fundamental detection + zero-crossing splice (the real algorithm)
        Autocorrelation,  // Pitch-period detection, snap jump to integer periods
        ZeroCrossing,     // Wait for zero crossing near splice, short crossfade
        Correlation       // Cross-correlate outgoing/incoming segments for best offset
    };

    PitchShifter() = default;

    void prepare (double sampleRate, int maxBlockSize);
    void reset();

    void setPitchSemitones (float semitones);
    void setPitchRatio (float ratio);
    void setGrainSizeMs (float ms);
    void setPortamento (float speed); // 0..1
    void setSpliceMode (SpliceMode mode) { spliceMode_ = mode; }
    void setVintageCharacter (bool enabled);

    float processSample (float input);
    float getLatencySamples() const { return grainSizeSamples_ * 0.5f; }

private:
    float hermiteInterp (double pos) const;
    int detectPitchPeriod() const;
    int detectPitchPeriodZeroCrossing() const;
    double findZeroCrossingNear (double pos, int searchRadius) const;
    double findBestCorrelationOffset (double oldPos, double idealNewPos, int searchRadius) const;

    std::vector<float> buffer_;
    int writePtr_ = 0;

    // Two delay taps — primary is active, secondary is used during crossfade
    struct Tap
    {
        double readPtr = 0.0;
    };

    Tap primary_{};
    Tap secondary_{};
    bool crossfading_ = false;
    int crossfadePos_ = 0;
    int crossfadeLen_ = 240; // ~5ms at 48kHz

    float currentRatio_ = 1.0f;
    float targetRatio_ = 1.0f;
    float portaCoeff_ = 1.0f;
    float lastSemitones_ = 0.0f;

    SpliceMode spliceMode_ = SpliceMode::Fischer;

    // grainSizeSamples_ controls the "safe zone" depth — how far
    // the read pointer trails the write pointer
    float grainSizeSamples_ = 1920.0f; // 40ms at 48kHz
    double sampleRate_ = 48000.0;

    // Vintage character mode — emulates 1978 hardware limitations
    bool vintageMode_ = false;
    float vintageGrainSizeSamples_ = 0.0f;  // ~20ms, set in prepare()
    int vintageCrossfadeLen_ = 0;            // ~2.5ms, set in prepare()
    int activeCrossfadeLen_ = 240;           // tracks which crossfade length is active during a fade
    LoFiProcessor vintageLoFi_;              // 12-bit quantization + 15kHz LPF
};
