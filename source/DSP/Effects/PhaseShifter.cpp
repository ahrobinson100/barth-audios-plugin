#include "PhaseShifter.h"

void PhaseShifter::prepare (double sampleRate)
{
    sampleRate_ = sampleRate;
    logMinFreq_ = std::log (minFreq_);
    logMaxFreq_ = std::log (maxFreq_);
    logFreqRange_ = logMaxFreq_ - logMinFreq_;
    reset();
}

void PhaseShifter::reset()
{
    for (auto& ch : stages_)
        for (auto& stage : ch)
            stage.reset();
    lfoPhase_ = 0.0;
}

void PhaseShifter::setRate (float hz)
{
    lfoPhaseInc_ = 2.0 * 3.14159265358979323846 * static_cast<double> (hz) / sampleRate_;
}

void PhaseShifter::setDepth (float depth)
{
    depth_ = (depth < 0.0f) ? 0.0f : ((depth > 1.0f) ? 1.0f : depth);
}

void PhaseShifter::setStereoMode (StereoMode mode)
{
    stereoMode_ = mode;
}

float PhaseShifter::processSample (float input, int channel)
{
    // Calculate LFO value with optional stereo phase offset
    double phaseOffset = 0.0;
    if (stereoMode_ == StereoMode::Stereo && channel == 1)
        phaseOffset = 3.14159265358979323846; // 180° offset for R channel

    float lfo = static_cast<float> (std::sin (lfoPhase_ + phaseOffset));
    float lfoNorm = (lfo + 1.0f) * 0.5f; // 0..1

    // Map LFO to frequency range (exponential mapping, logs precomputed in prepare())
    float freq = std::exp (logMinFreq_ + lfoNorm * depth_ * logFreqRange_);

    // Convert frequency to all-pass coefficient
    float w = 3.14159265f * freq / static_cast<float> (sampleRate_);
    float coeff = (1.0f - w) / (1.0f + w);

    // Process through cascaded all-pass filters
    size_t ch = static_cast<size_t> (channel & 1);
    float output = input;

    int numPasses = (stereoMode_ == StereoMode::Tandem) ? 2 : 1;
    for (int pass = 0; pass < numPasses; ++pass)
    {
        for (auto& stage : stages_[ch])
            output = stage.process (output, coeff);
    }

    // Mix all-pass output with dry (creates notches)
    float wet = output;
    output = input + wet * depth_;

    // Only advance LFO on channel 0 to keep channels in sync
    if (channel == 0)
    {
        lfoPhase_ += lfoPhaseInc_;
        if (lfoPhase_ > 6.283185307) lfoPhase_ -= 6.283185307;
    }

    return output;
}
