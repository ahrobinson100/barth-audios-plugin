#include "StereoReverb.h"

void StereoReverb::prepare (double sampleRate)
{
    sampleRate_ = sampleRate;

    // Scale delay lengths to sample rate
    double ratio = sampleRate / 48000.0;
    for (int i = 0; i < NUM_LINES; ++i)
    {
        int scaled = static_cast<int> (DELAY_LENGTHS[i] * ratio * (0.8 + roomSize_ * 0.4));
        if (scaled >= MAX_DELAY) scaled = MAX_DELAY - 1;
        if (scaled < 1) scaled = 1;
        lines_[static_cast<size_t> (i)].length = scaled;
    }

    reset();
}

void StereoReverb::reset()
{
    for (auto& line : lines_)
        line.reset();
    modPhase_ = 0.0;
}

void StereoReverb::setDecayTime (float seconds)
{
    // Calculate feedback gain from desired decay time
    // RT60 = -60dB / (20 * log10(g)) * avgDelay / sampleRate
    float avgDelay = 0.0f;
    for (auto& line : lines_)
        avgDelay += static_cast<float> (line.length);
    avgDelay /= static_cast<float> (NUM_LINES);

    if (seconds < 0.1f) seconds = 0.1f;
    if (seconds > 10.0f) seconds = 10.0f;

    // g = 10^(-3 * avgDelay / (sampleRate * decayTime))
    feedbackGain_ = std::pow (10.0f, -3.0f * avgDelay / (static_cast<float> (sampleRate_) * seconds));
    if (feedbackGain_ > 0.98f) feedbackGain_ = 0.98f;
}

void StereoReverb::setRoomSize (float size)
{
    roomSize_ = (size < 0.0f) ? 0.0f : ((size > 1.0f) ? 1.0f : size);

    double ratio = sampleRate_ / 48000.0;
    for (int i = 0; i < NUM_LINES; ++i)
    {
        int scaled = static_cast<int> (DELAY_LENGTHS[i] * ratio * (0.8 + roomSize_ * 0.4));
        if (scaled >= MAX_DELAY) scaled = MAX_DELAY - 1;
        if (scaled < 1) scaled = 1;
        lines_[static_cast<size_t> (i)].length = scaled;
    }
}

void StereoReverb::setDamping (float damping)
{
    damping_ = (damping < 0.0f) ? 0.0f : ((damping > 1.0f) ? 1.0f : damping);
}

void StereoReverb::processSample (float inputL, float inputR, float& outputL, float& outputR)
{
    // Read from delay lines
    std::array<float, NUM_LINES> delayed;
    for (int i = 0; i < NUM_LINES; ++i)
        delayed[static_cast<size_t> (i)] = lines_[static_cast<size_t> (i)].read();

    // Apply damping (one-pole lowpass in feedback path)
    for (int i = 0; i < NUM_LINES; ++i)
    {
        auto& line = lines_[static_cast<size_t> (i)];
        delayed[static_cast<size_t> (i)] = line.dampState + damping_ * (delayed[static_cast<size_t> (i)] - line.dampState);
        line.dampState = delayed[static_cast<size_t> (i)];
        if (! (line.dampState < -1.0e-8f || line.dampState > 1.0e-8f)) line.dampState = 0.0f;
    }

    // Hadamard mixing matrix (4x4, normalized)
    // H = 0.5 * [[1,1,1,1],[1,-1,1,-1],[1,1,-1,-1],[1,-1,-1,1]]
    float a = delayed[0], b = delayed[1], c = delayed[2], d = delayed[3];
    std::array<float, NUM_LINES> mixed;
    mixed[0] = 0.5f * (a + b + c + d);
    mixed[1] = 0.5f * (a - b + c - d);
    mixed[2] = 0.5f * (a + b - c - d);
    mixed[3] = 0.5f * (a - b - c + d);

    // Write back with input injection and feedback
    float inputMono = (inputL + inputR) * 0.5f;
    for (int i = 0; i < NUM_LINES; ++i)
    {
        float writeVal = inputMono + mixed[static_cast<size_t> (i)] * feedbackGain_;
        // Soft-limit feedback to prevent runaway
        if (std::abs (writeVal) > 0.95f)
            writeVal = 0.95f * std::tanh (writeVal / 0.95f);
        lines_[static_cast<size_t> (i)].write (writeVal);
    }

    // Stereo output: mix odd/even delay lines for L/R
    outputL = (delayed[0] + delayed[2]) * 0.5f;
    outputR = (delayed[1] + delayed[3]) * 0.5f;

    // Modulation phase reserved for future use
}
