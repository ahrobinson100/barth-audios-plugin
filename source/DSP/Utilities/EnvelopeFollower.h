#pragma once
#include <cmath>

class EnvelopeFollower
{
public:
    void prepare (double sampleRate)
    {
        sampleRate_ = sampleRate;
        updateCoefficients();
        reset();
    }

    void reset()
    {
        envelope_ = 0.0f;
        prevEnvelope_ = 0.0f;
        triggered_ = false;
    }

    void setAttackMs (float ms)
    {
        attackMs_ = ms;
        updateCoefficients();
    }

    void setReleaseMs (float ms)
    {
        releaseMs_ = ms;
        updateCoefficients();
    }

    void setThresholdDb (float db)
    {
        threshold_ = std::pow (10.0f, db / 20.0f);
    }

    float processSample (float input)
    {
        float absInput = std::abs (input);
        float coeff = (absInput > envelope_) ? attackCoeff_ : releaseCoeff_;
        envelope_ += coeff * (absInput - envelope_);
        if (! (envelope_ < -1.0e-8f || envelope_ > 1.0e-8f)) envelope_ = 0.0f;
        return envelope_;
    }

    bool detectTransient (float input)
    {
        prevEnvelope_ = envelope_;
        processSample (input);

        bool isAboveThreshold = envelope_ > threshold_;
        bool isRising = envelope_ > prevEnvelope_ * 1.5f;

        if (isAboveThreshold && isRising && !triggered_)
        {
            triggered_ = true;
            return true;
        }

        if (envelope_ < threshold_ * 0.5f)
            triggered_ = false;

        return false;
    }

private:
    void updateCoefficients()
    {
        if (sampleRate_ > 0.0)
        {
            attackCoeff_ = 1.0f - std::exp (-1.0f / (static_cast<float> (sampleRate_) * attackMs_ * 0.001f));
            releaseCoeff_ = 1.0f - std::exp (-1.0f / (static_cast<float> (sampleRate_) * releaseMs_ * 0.001f));
        }
    }

    double sampleRate_ = 48000.0;
    float attackMs_ = 1.0f;
    float releaseMs_ = 50.0f;
    float attackCoeff_ = 0.1f;
    float releaseCoeff_ = 0.001f;
    float envelope_ = 0.0f;
    float prevEnvelope_ = 0.0f;
    float threshold_ = 0.1f;
    bool triggered_ = false;
};
