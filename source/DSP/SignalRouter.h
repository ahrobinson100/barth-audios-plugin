#pragma once

class SignalRouter
{
public:
    enum class OutputMode { Stereo, Mono, Diff };

    void setMix (float mix) { mix_ = (mix < 0.0f) ? 0.0f : ((mix > 1.0f) ? 1.0f : mix); }
    void setOutputMode (OutputMode mode) { outputMode_ = mode; }

    void processStereo (float dryL, float dryR, float wetL, float wetR,
                        float& outL, float& outR) const
    {
        // Wet/dry mix
        float mixedL = dryL * (1.0f - mix_) + wetL * mix_;
        float mixedR = dryR * (1.0f - mix_) + wetR * mix_;

        switch (outputMode_)
        {
            case OutputMode::Stereo:
                outL = mixedL;
                outR = mixedR;
                break;

            case OutputMode::Mono:
                outL = outR = (mixedL + mixedR) * 0.5f;
                break;

            case OutputMode::Diff:
                outL = (mixedL - mixedR) * 0.5f;
                outR = (mixedR - mixedL) * 0.5f;
                break;
        }
    }

private:
    float mix_ = 0.5f;
    OutputMode outputMode_ = OutputMode::Stereo;
};
