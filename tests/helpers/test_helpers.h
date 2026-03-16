#pragma once
#include <PluginProcessor.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <cmath>

namespace TestHelpers
{
    // Generate mono sine wave buffer
    inline juce::AudioBuffer<float> makeSine (float freq, double sampleRate, int numSamples)
    {
        juce::AudioBuffer<float> buffer (1, numSamples);
        auto* data = buffer.getWritePointer (0);
        double phase = 0.0;
        double phaseInc = 2.0 * 3.14159265358979323846 * static_cast<double> (freq) / sampleRate;

        for (int i = 0; i < numSamples; ++i)
        {
            data[i] = static_cast<float> (std::sin (phase));
            phase += phaseInc;
        }

        return buffer;
    }

    // Generate stereo sine wave buffer
    inline juce::AudioBuffer<float> makeStereoSine (float freq, double sampleRate, int numSamples)
    {
        juce::AudioBuffer<float> buffer (2, numSamples);
        auto* dataL = buffer.getWritePointer (0);
        auto* dataR = buffer.getWritePointer (1);
        double phase = 0.0;
        double phaseInc = 2.0 * 3.14159265358979323846 * static_cast<double> (freq) / sampleRate;

        for (int i = 0; i < numSamples; ++i)
        {
            float val = static_cast<float> (std::sin (phase));
            dataL[i] = val;
            dataR[i] = val;
            phase += phaseInc;
        }

        return buffer;
    }

    // Generate impulse
    inline juce::AudioBuffer<float> makeImpulse (int numSamples)
    {
        juce::AudioBuffer<float> buffer (1, numSamples);
        buffer.clear();
        buffer.setSample (0, 0, 1.0f);
        return buffer;
    }

    // Generate silence
    inline juce::AudioBuffer<float> makeSilence (int numSamples)
    {
        juce::AudioBuffer<float> buffer (1, numSamples);
        buffer.clear();
        return buffer;
    }

    // Measure dominant frequency via FFT
    inline float measureFrequency (const juce::AudioBuffer<float>& buffer, double sampleRate)
    {
        const int fftOrder = 12; // 4096 points
        const int fftSize = 1 << fftOrder;
        juce::dsp::FFT fft (fftOrder);

        std::vector<float> fftData (static_cast<size_t> (fftSize * 2), 0.0f);
        const auto* data = buffer.getReadPointer (0);
        int samplesToUse = std::min (buffer.getNumSamples(), fftSize);

        // Copy and apply Hann window
        for (int i = 0; i < samplesToUse; ++i)
        {
            float window = 0.5f * (1.0f - std::cos (2.0f * 3.14159265f * static_cast<float> (i) / static_cast<float> (samplesToUse)));
            fftData[static_cast<size_t> (i)] = data[i] * window;
        }

        fft.performFrequencyOnlyForwardTransform (fftData.data());

        // Find peak bin (skip DC)
        int peakBin = 1;
        float peakVal = 0.0f;
        for (int i = 1; i < fftSize / 2; ++i)
        {
            if (fftData[static_cast<size_t> (i)] > peakVal)
            {
                peakVal = fftData[static_cast<size_t> (i)];
                peakBin = i;
            }
        }

        return static_cast<float> (peakBin) * static_cast<float> (sampleRate) / static_cast<float> (fftSize);
    }

    // Detect clicks/discontinuities
    inline int detectGlitches (const juce::AudioBuffer<float>& buffer, float threshold = 0.5f)
    {
        const auto* data = buffer.getReadPointer (0);
        int count = 0;

        for (int i = 1; i < buffer.getNumSamples(); ++i)
        {
            if (std::abs (data[i] - data[i - 1]) > threshold)
                ++count;
        }

        return count;
    }

    // Measure RMS level
    inline float measureRMS (const juce::AudioBuffer<float>& buffer)
    {
        return buffer.getRMSLevel (0, 0, buffer.getNumSamples());
    }

    // Measure peak level
    inline float measurePeak (const juce::AudioBuffer<float>& buffer)
    {
        return buffer.getMagnitude (0, 0, buffer.getNumSamples());
    }

    // Check for NaN or Inf
    inline bool hasNanOrInf (const juce::AudioBuffer<float>& buffer)
    {
        const auto* data = buffer.getReadPointer (0);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            if (std::isnan (data[i]) || std::isinf (data[i]))
                return true;
        }
        return false;
    }

    // Measure DC offset
    inline float measureDCOffset (const juce::AudioBuffer<float>& buffer)
    {
        const auto* data = buffer.getReadPointer (0);
        double sum = 0.0;
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            sum += static_cast<double> (data[i]);
        return static_cast<float> (sum / buffer.getNumSamples());
    }

    // Helper: run mono buffer through PitchShifter
    inline juce::AudioBuffer<float> processWithPitchShifter (
        const juce::AudioBuffer<float>& input, float pitchRatio,
        double sampleRate, float grainMs = 40.0f, int skipSamples = 0)
    {
        PitchShifter ps;
        ps.prepare (sampleRate, 512);
        ps.setPitchRatio (pitchRatio);
        ps.setGrainSizeMs (grainMs);

        const int numSamples = input.getNumSamples();
        juce::AudioBuffer<float> output (1, numSamples);
        const auto* in = input.getReadPointer (0);
        auto* out = output.getWritePointer (0);

        for (int i = 0; i < numSamples; ++i)
            out[i] = ps.processSample (in[i]);

        // If skipping settling samples, return trimmed buffer
        if (skipSamples > 0 && skipSamples < numSamples)
        {
            int remaining = numSamples - skipSamples;
            juce::AudioBuffer<float> trimmed (1, remaining);
            trimmed.copyFrom (0, 0, output, 0, skipSamples, remaining);
            return trimmed;
        }

        return output;
    }

    // Process PitchShifter by semitones
    inline juce::AudioBuffer<float> processWithPitchShifterSt (
        const juce::AudioBuffer<float>& input, float semitones,
        double sampleRate, float grainMs = 40.0f, int skipSamples = 0)
    {
        float ratio = std::pow (2.0f, semitones / 12.0f);
        return processWithPitchShifter (input, ratio, sampleRate, grainMs, skipSamples);
    }
}

// Keep the original runWithinPluginEditor for pamplejuce compat
[[maybe_unused]] static void runWithinPluginEditor (const std::function<void (PluginProcessor& plugin)>& testCode)
{
    PluginProcessor plugin;
    const auto editor = plugin.createEditorIfNeeded();
    testCode (plugin);
    plugin.editorBeingDeleted (editor);
    delete editor;
}
