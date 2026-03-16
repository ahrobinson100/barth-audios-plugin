#pragma once
#include <cmath>

class ADSREnvelope
{
public:
    enum class TriggerSource { Transient, Keyboard, Sequencer };
    enum class State { Idle, Attack, Decay, Sustain, Release };

    void prepare (double sampleRate);
    void reset();

    void setAttackMs (float ms);
    void setDecayMs (float ms);
    void setSustain (float level);   // 0..1
    void setReleaseMs (float ms);
    void setInverted (bool inverted);
    void updateCoefficients();

    void trigger();
    void release();

    float processSample();

    State getState() const { return state_; }

private:

    double sampleRate_ = 48000.0;

    float attackMs_ = 10.0f;
    float decayMs_ = 100.0f;
    float sustainLevel_ = 0.7f;
    float releaseMs_ = 200.0f;
    bool inverted_ = false;

    float attackCoeff_ = 0.0f;
    float decayCoeff_ = 0.0f;
    float releaseCoeff_ = 0.0f;

    State state_ = State::Idle;
    float envelope_ = 0.0f;
};
