#pragma once
#include <array>
#include <cmath>
#include <vector>

// Feedback Delay Network reverb with 4 delay lines and Hadamard mixing
class StereoReverb
{
public:
    void prepare (double sampleRate);
    void reset();

    void setDecayTime (float seconds);
    void setRoomSize (float size); // 0..1
    void setDamping (float damping); // 0..1

    // Process stereo: input/output are left, right pair
    void processSample (float inputL, float inputR, float& outputL, float& outputR);

private:
    static constexpr int NUM_LINES = 4;
    // Prime-number delay lengths for maximum density
    static constexpr int DELAY_LENGTHS[NUM_LINES] = { 1087, 1283, 1511, 1777 };
    static constexpr int MAX_DELAY = 8192;
    static constexpr int DELAY_MASK = MAX_DELAY - 1;

    struct DelayLineFDN
    {
        std::vector<float> buffer;
        int writePtr = 0;
        int length = 1087;
        float dampState = 0.0f;

        void reset()
        {
            buffer.assign (MAX_DELAY, 0.0f);
            writePtr = 0;
            dampState = 0.0f;
        }

        void write (float sample)
        {
            buffer[static_cast<size_t> (writePtr)] = sample;
            writePtr = (writePtr + 1) & DELAY_MASK;
        }

        float read() const
        {
            int readPtr = (writePtr - length) & DELAY_MASK;
            return buffer[static_cast<size_t> (readPtr)];
        }
    };

    std::array<DelayLineFDN, NUM_LINES> lines_{};
    float feedbackGain_ = 0.5f;
    float damping_ = 0.3f;
    double sampleRate_ = 48000.0;
    float roomSize_ = 0.5f;
};
