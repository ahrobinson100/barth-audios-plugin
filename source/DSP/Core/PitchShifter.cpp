#include "PitchShifter.h"
#include <algorithm>

void PitchShifter::prepare (double sampleRate, int /*maxBlockSize*/)
{
    sampleRate_ = sampleRate;
    grainSizeSamples_ = 40.0f * static_cast<float> (sampleRate) / 1000.0f;
    crossfadeLen_ = static_cast<int> (5.0 * sampleRate / 1000.0); // 5ms
    vintageGrainSizeSamples_ = 20.0f * static_cast<float> (sampleRate) / 1000.0f;
    vintageCrossfadeLen_ = static_cast<int> (2.5 * sampleRate / 1000.0);
    activeCrossfadeLen_ = crossfadeLen_;
    vintageLoFi_.prepare (sampleRate);
    vintageLoFi_.setBitDepth (12);
    vintageLoFi_.setSampleRateDivider (1);
    vintageLoFi_.setLPFCutoff (12000.0f);
    reset();
}

void PitchShifter::reset()
{
    buffer_.assign (BUFFER_SIZE, 0.0f);
    writePtr_ = 0;
    currentRatio_ = targetRatio_;

    // Position primary tap at ideal trailing distance
    double offset = static_cast<double> (grainSizeSamples_);
    primary_.readPtr = static_cast<double> (writePtr_) - offset;
    while (primary_.readPtr < 0.0)
        primary_.readPtr += BUFFER_SIZE;

    secondary_.readPtr = 0.0;
    crossfading_ = false;
    crossfadePos_ = 0;
    vintageLoFi_.reset();
    activeCrossfadeLen_ = crossfadeLen_;
}

void PitchShifter::setPitchSemitones (float semitones)
{
    if (semitones == lastSemitones_)
        return;
    lastSemitones_ = semitones;
    setPitchRatio (std::exp2f (semitones / 12.0f));
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
    if (speed <= 0.0f)
        portaCoeff_ = 1.0f;
    else
        portaCoeff_ = 1.0f - speed * 0.9999f;
}

void PitchShifter::setVintageCharacter (bool enabled)
{
    vintageMode_ = enabled;
}

// ─── Splice Mode: Autocorrelation ───────────────────────────────────────────
// Detect the dominant pitch period using normalised autocorrelation.
// Returns period in samples, or 0 if no clear pitch.
int PitchShifter::detectPitchPeriod() const
{
    int minLag = std::max (2, static_cast<int> (sampleRate_ / 1000.0)); // 1000 Hz
    int maxLag = static_cast<int> (sampleRate_ / 50.0);                 // 50 Hz
    int analysisLen = maxLag;

    float bestCorr = 0.3f;
    int bestLag = 0;

    for (int lag = minLag; lag <= maxLag; ++lag)
    {
        float sum = 0.0f, energy1 = 0.0f, energy2 = 0.0f;
        for (int n = 0; n < analysisLen; ++n)
        {
            float s1 = buffer_[(writePtr_ - analysisLen + n) & BUFFER_MASK];
            float s2 = buffer_[(writePtr_ - analysisLen + n - lag) & BUFFER_MASK];
            sum += s1 * s2;
            energy1 += s1 * s1;
            energy2 += s2 * s2;
        }
        float denom = std::sqrt (energy1 * energy2) + 1.0e-10f;
        float corr = sum / denom;
        if (corr > bestCorr)
        {
            bestCorr = corr;
            bestLag = lag;
        }
    }

    return bestLag;
}

// ─── Vintage: TTL zero-crossing pitch detector ─────────────────────────────
// Crude pitch detector that finds the last two positive-going zero crossings
// and returns the distance between them. No correlation, no confidence
// threshold — just raw interval measurement. Gets fooled by harmonics
// (returns half-period for signals with strong harmonics). This emulates
// the crude TTL comparator circuit in the 1978 hardware.
int PitchShifter::detectPitchPeriodZeroCrossing() const
{
    int minPeriod = std::max (2, static_cast<int> (sampleRate_ / 1000.0)); // 1000 Hz
    int maxPeriod = static_cast<int> (sampleRate_ / 50.0);                 // 50 Hz
    int searchLen = maxPeriod * 3; // look back far enough to find two crossings

    int crossing1 = -1; // most recent positive-going zero crossing
    int crossing2 = -1; // second most recent

    for (int offset = 1; offset < searchLen; ++offset)
    {
        int i = (writePtr_ - offset) & BUFFER_MASK;
        int iPrev = (writePtr_ - offset - 1) & BUFFER_MASK;

        // Positive-going zero crossing: previous < 0, current >= 0
        if (buffer_[iPrev] < 0.0f && buffer_[i] >= 0.0f)
        {
            if (crossing1 < 0)
                crossing1 = offset;
            else
            {
                crossing2 = offset;
                break;
            }
        }
    }

    if (crossing1 < 0 || crossing2 < 0)
        return 0;

    int period = crossing2 - crossing1;

    // Range check: reject if outside 50Hz–1000Hz
    if (period < minPeriod || period > maxPeriod)
        return 0;

    return period;
}

// ─── Splice Mode: Zero-Crossing ─────────────────────────────────────────────
// Find the nearest zero crossing to a given buffer position.
// Searches outward from pos up to searchRadius samples in each direction.
// Returns the position of the zero crossing, or the original pos if none found.
// Achievable with a single comparator in TTL hardware.
double PitchShifter::findZeroCrossingNear (double pos, int searchRadius) const
{
    int centre = static_cast<int> (pos) & BUFFER_MASK;

    for (int offset = 0; offset <= searchRadius; ++offset)
    {
        // Search forward
        int i = (centre + offset) & BUFFER_MASK;
        int iPrev = (i - 1) & BUFFER_MASK;
        float s0 = buffer_[iPrev];
        float s1 = buffer_[i];
        if ((s0 >= 0.0f && s1 < 0.0f) || (s0 < 0.0f && s1 >= 0.0f))
        {
            // Linear interpolation to find exact crossing
            float frac = s0 / (s0 - s1);
            return static_cast<double> ((centre + offset - 1) & BUFFER_MASK) + frac;
        }

        if (offset == 0)
            continue;

        // Search backward
        i = (centre - offset) & BUFFER_MASK;
        iPrev = (i - 1) & BUFFER_MASK;
        s0 = buffer_[iPrev];
        s1 = buffer_[i];
        if ((s0 >= 0.0f && s1 < 0.0f) || (s0 < 0.0f && s1 >= 0.0f))
        {
            float frac = s0 / (s0 - s1);
            return static_cast<double> ((centre - offset - 1) & BUFFER_MASK) + frac;
        }
    }

    return pos; // no crossing found, use original position
}

// ─── Splice Mode: Correlation ───────────────────────────────────────────────
// Cross-correlate a short window around the old read position with candidate
// positions near the ideal new position. Returns the offset that gives the
// best match — so the crossfade blends two segments that look alike.
// This is closer to what the Publison DHM89's "Deglitcher Board" did.
double PitchShifter::findBestCorrelationOffset (double oldPos, double idealNewPos, int searchRadius) const
{
    int windowLen = crossfadeLen_ * 2; // compare ~10ms of audio
    int oldStart = static_cast<int> (oldPos) & BUFFER_MASK;
    int idealStart = static_cast<int> (idealNewPos) & BUFFER_MASK;

    float bestCorr = -1.0f;
    int bestOffset = 0;

    for (int offset = -searchRadius; offset <= searchRadius; ++offset)
    {
        int candidateStart = (idealStart + offset) & BUFFER_MASK;

        float sum = 0.0f, e1 = 0.0f, e2 = 0.0f;
        for (int n = 0; n < windowLen; ++n)
        {
            float a = buffer_[(oldStart + n) & BUFFER_MASK];
            float b = buffer_[(candidateStart + n) & BUFFER_MASK];
            sum += a * b;
            e1 += a * a;
            e2 += b * b;
        }
        float denom = std::sqrt (e1 * e2) + 1.0e-10f;
        float corr = sum / denom;

        if (corr > bestCorr)
        {
            bestCorr = corr;
            bestOffset = offset;
        }
    }

    double result = idealNewPos + bestOffset;
    while (result < 0.0) result += BUFFER_SIZE;
    while (result >= BUFFER_SIZE) result -= BUFFER_SIZE;
    return result;
}

// ─── Main processing ────────────────────────────────────────────────────────
float PitchShifter::processSample (float input)
{
    // Write to circular buffer
    buffer_[writePtr_] = input;
    writePtr_ = (writePtr_ + 1) & BUFFER_MASK;

    // Smooth pitch ratio (portamento)
    currentRatio_ += portaCoeff_ * (targetRatio_ - currentRatio_);

    // --- Read from primary tap ---
    float output = hermiteInterp (primary_.readPtr);

    // --- Crossfade blend if splicing ---
    if (crossfading_)
    {
        float secondaryOut = hermiteInterp (secondary_.readPtr);

        // Constant-power crossfade (sine/cosine window)
        // sin²(x) + cos²(x) = 1 maintains energy throughout the blend,
        // compensating for phase cancellation between misaligned segments.
        float t = static_cast<float> (crossfadePos_) / static_cast<float> (activeCrossfadeLen_);
        float fadeIn = std::sin (1.5707963f * t);   // PI/2 * t
        float fadeOut = std::cos (1.5707963f * t);
        output = secondaryOut * fadeOut + output * fadeIn;

        // Advance secondary read pointer at same rate
        secondary_.readPtr += static_cast<double> (currentRatio_);
        if (secondary_.readPtr >= BUFFER_SIZE) secondary_.readPtr -= BUFFER_SIZE;
        if (secondary_.readPtr < 0.0) secondary_.readPtr += BUFFER_SIZE;

        crossfadePos_++;
        if (crossfadePos_ >= activeCrossfadeLen_)
            crossfading_ = false;
    }

    // --- Advance primary read pointer ---
    primary_.readPtr += static_cast<double> (currentRatio_);
    if (primary_.readPtr >= BUFFER_SIZE) primary_.readPtr -= BUFFER_SIZE;
    if (primary_.readPtr < 0.0) primary_.readPtr += BUFFER_SIZE;

    // --- Check if a splice is needed ---
    if (! crossfading_ && std::abs (currentRatio_ - 1.0f) > 0.001f)
    {
        double gap = static_cast<double> (writePtr_) - primary_.readPtr;
        if (gap < 0.0) gap += BUFFER_SIZE;
        if (gap >= BUFFER_SIZE) gap -= BUFFER_SIZE;

        float effectiveGrain = vintageMode_ ? vintageGrainSizeSamples_ : grainSizeSamples_;
        int effectiveCrossfade = vintageMode_ ? vintageCrossfadeLen_ : crossfadeLen_;

        double idealGap = static_cast<double> (effectiveGrain);
        double minGap = static_cast<double> (effectiveCrossfade) * 3.0;
        double maxGap = idealGap * 2.0;

        if (gap < minGap || gap > maxGap)
        {
            // Save current position as secondary (fading out)
            secondary_ = primary_;

            // Target: reposition primary to ideal trailing distance
            double newReadPtr = static_cast<double> (writePtr_) - idealGap;

            switch (spliceMode_)
            {
                case SpliceMode::Fischer:
                {
                    // THE REAL FISCHER ALGORITHM (Studio Magazin 1981):
                    // 1. Detect fundamental wave (Grundwelle) of the signal
                    // 2. Size the jump to integer pitch periods
                    // 3. Execute splice at a zero crossing (Nulldurchgang)
                    int period = detectPitchPeriod();
                    if (period > 0)
                    {
                        double T = static_cast<double> (period);
                        double diff = newReadPtr - secondary_.readPtr;
                        if (diff < -BUFFER_SIZE / 2) diff += BUFFER_SIZE;
                        if (diff > BUFFER_SIZE / 2) diff -= BUFFER_SIZE;

                        double periods = std::round (diff / T);
                        if (periods == 0.0)
                            periods = (diff > 0.0) ? 1.0 : -1.0;

                        newReadPtr = secondary_.readPtr + periods * T;

                        // Snap to nearest zero crossing — compensates for
                        // pitch detection rounding errors.
                        int searchRadius = static_cast<int> (sampleRate_ * 0.002);
                        newReadPtr = findZeroCrossingNear (newReadPtr, searchRadius);
                    }
                    else
                    {
                        // Pitch detection failed (polyphonic/noise/transient):
                        // fall back to cross-correlation alignment so the
                        // crossfade blends two similar-looking segments.
                        int searchRadius = static_cast<int> (sampleRate_ * 0.005);
                        newReadPtr = findBestCorrelationOffset (secondary_.readPtr, newReadPtr, searchRadius);
                    }
                    break;
                }

                case SpliceMode::Autocorrelation:
                {
                    // Snap jump distance to integer pitch periods
                    int period = detectPitchPeriod();
                    if (period > 0)
                    {
                        double T = static_cast<double> (period);
                        double diff = newReadPtr - secondary_.readPtr;
                        if (diff < -BUFFER_SIZE / 2) diff += BUFFER_SIZE;
                        if (diff > BUFFER_SIZE / 2) diff -= BUFFER_SIZE;

                        double periods = std::round (diff / T);
                        if (periods == 0.0)
                            periods = (diff > 0.0) ? 1.0 : -1.0;

                        newReadPtr = secondary_.readPtr + periods * T;
                    }
                    else
                    {
                        // Fall back to correlation when pitch detection fails
                        int searchRadius = static_cast<int> (sampleRate_ * 0.005);
                        newReadPtr = findBestCorrelationOffset (secondary_.readPtr, newReadPtr, searchRadius);
                    }
                    break;
                }

                case SpliceMode::ZeroCrossing:
                {
                    // Snap primary to nearest zero crossing.
                    // Don't move secondary — same reasoning as Fischer.
                    int searchRadius = static_cast<int> (sampleRate_ * 0.002);
                    newReadPtr = findZeroCrossingNear (newReadPtr, searchRadius);
                    break;
                }

                case SpliceMode::Correlation:
                {
                    // Find the offset near idealNewPos that best matches the
                    // waveform at the old position — so the crossfade blends
                    // two similar-looking segments. Search ~5ms around ideal.
                    int searchRadius = static_cast<int> (sampleRate_ * 0.005);
                    newReadPtr = findBestCorrelationOffset (secondary_.readPtr, newReadPtr, searchRadius);
                    break;
                }
            }

            // Wrap into buffer range
            while (newReadPtr < 0.0) newReadPtr += BUFFER_SIZE;
            while (newReadPtr >= BUFFER_SIZE) newReadPtr -= BUFFER_SIZE;

            primary_.readPtr = newReadPtr;
            crossfading_ = true;
            crossfadePos_ = 0;
            activeCrossfadeLen_ = effectiveCrossfade;
        }
    }

    if (vintageMode_)
        output = vintageLoFi_.processSample (output);

    return output;
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
