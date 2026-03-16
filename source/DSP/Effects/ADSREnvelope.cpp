#include "ADSREnvelope.h"

void ADSREnvelope::prepare (double sampleRate)
{
    sampleRate_ = sampleRate;
    updateCoefficients();
    reset();
}

void ADSREnvelope::reset()
{
    state_ = State::Idle;
    envelope_ = 0.0f;
}

void ADSREnvelope::setAttackMs (float ms)
{
    attackMs_ = (ms < 1.0f) ? 1.0f : ms;
}

void ADSREnvelope::setDecayMs (float ms)
{
    decayMs_ = (ms < 1.0f) ? 1.0f : ms;
}

void ADSREnvelope::setSustain (float level)
{
    sustainLevel_ = (level < 0.0f) ? 0.0f : ((level > 1.0f) ? 1.0f : level);
}

void ADSREnvelope::setReleaseMs (float ms)
{
    releaseMs_ = (ms < 1.0f) ? 1.0f : ms;
}

void ADSREnvelope::setInverted (bool inverted)
{
    inverted_ = inverted;
}

void ADSREnvelope::updateCoefficients()
{
    float sr = static_cast<float> (sampleRate_);
    // Exponential coefficients: reach target in ~5 time constants
    attackCoeff_ = 1.0f - std::exp (-1000.0f / (sr * attackMs_));
    decayCoeff_ = 1.0f - std::exp (-1000.0f / (sr * decayMs_));
    releaseCoeff_ = 1.0f - std::exp (-1000.0f / (sr * releaseMs_));
}

void ADSREnvelope::trigger()
{
    state_ = State::Attack;
}

void ADSREnvelope::release()
{
    if (state_ != State::Idle)
        state_ = State::Release;
}

float ADSREnvelope::processSample()
{
    switch (state_)
    {
        case State::Idle:
            envelope_ = 0.0f;
            break;

        case State::Attack:
            envelope_ += attackCoeff_ * (1.1f - envelope_); // Overshoot target slightly for snappy attack
            if (envelope_ >= 1.0f)
            {
                envelope_ = 1.0f;
                state_ = State::Decay;
            }
            break;

        case State::Decay:
            envelope_ += decayCoeff_ * (sustainLevel_ - envelope_);
            if (std::abs (envelope_ - sustainLevel_) < 0.001f)
            {
                envelope_ = sustainLevel_;
                state_ = State::Sustain;
            }
            break;

        case State::Sustain:
            envelope_ = sustainLevel_;
            break;

        case State::Release:
            envelope_ += releaseCoeff_ * (0.0f - envelope_);
            if (envelope_ < 0.001f)
            {
                envelope_ = 0.0f;
                state_ = State::Idle;
            }
            break;
    }

    return inverted_ ? (1.0f - envelope_) : envelope_;
}
