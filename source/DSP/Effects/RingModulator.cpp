#include "RingModulator.h"

void RingModulator::prepare (double sampleRate)
{
    sampleRate_ = sampleRate;
    reset();
}

void RingModulator::reset()
{
    phase_ = 0.0;
}

void RingModulator::setFrequency (float hz)
{
    phaseInc_ = 2.0 * 3.14159265358979323846 * static_cast<double> (hz) / sampleRate_;
}

void RingModulator::setDepth (float depth)
{
    depth_ = (depth < 0.0f) ? 0.0f : ((depth > 1.0f) ? 1.0f : depth);
}

float RingModulator::processSample (float input)
{
    float carrier = static_cast<float> (std::sin (phase_));
    phase_ += phaseInc_;
    if (phase_ > 6.283185307) phase_ -= 6.283185307;

    // Blend between dry and ring-modulated based on depth
    float modulated = input * carrier;
    return input * (1.0f - depth_) + modulated * depth_;
}
