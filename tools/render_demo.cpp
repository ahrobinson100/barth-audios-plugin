// Barth Audios — Offline Demo Renderer
// Reads a WAV file, processes through the DSP chain with a named preset,
// and writes the result as a new WAV file.
//
// Usage: ./RenderDemo input.wav output.wav --preset shimmer
//
// No JUCE dependency — uses dr_wav for WAV I/O and the DSP classes directly.

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

// DSP includes (all JUCE-free)
#include "DSP/Utilities/OnePoleFilter.h"
#include "DSP/Core/PitchShifter.h"
#include "DSP/Core/TimeStretcher.h"
#include "DSP/Core/DelayLine.h"
#include "DSP/Effects/FrequencyShifter.h"
#include "DSP/Effects/RingModulator.h"
#include "DSP/Effects/PhaseShifter.h"
#include "DSP/Effects/Distortion.h"
#include "DSP/Effects/StereoReverb.h"
#include "DSP/Utilities/LoFiProcessor.h"
#include "DSP/Utilities/SoftClipper.h"
#include "DSP/SignalRouter.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <string>
#include <vector>
#include <algorithm>

// ============================================================================
// Preset definition
// ============================================================================
struct Preset
{
    const char* name;
    float pitchA;             // Voice A pitch in semitones
    float pitchB;             // Voice B pitch in semitones
    float delayMs;            // delay time
    float feedbackPct;        // 0..1
    float mix;                // wet/dry 0..1

    // Effect selection: 0=off, 1=freqShift, 2=ringMod, 3=phaser
    int fxSelect;
    float fxFreq;
    float fxDepth;            // 0..1

    bool reverbEnabled;
    float reverbDecay;        // seconds
    float reverbSize;         // 0..1

    bool distEnabled;
    float distDrive;          // 0..1
    float distBassBoost;      // dB

    // Lo-fi
    int bitDepth;             // 4..16 (16 = off)
    int srDivider;            // 1 = off
    float lpfCutoff;          // Hz

    bool vintage;             // vintage character mode
};

static const Preset PRESETS[] = {
    // name            pitchA pitchB delay  fb    mix   fx  fxFreq  fxDep  rev   revDec revSz  dist  dDrv  dBas  bits srD  lpf       vint
    { "octave-up",     12.0f, 12.0f, 0.0f,  0.0f, 1.0f, 0, 0.0f,   0.0f,  false, 0.0f, 0.0f,  false, 0.0f, 0.0f, 16, 1, 20000.0f, false },
    { "octave-down",  -12.0f,-12.0f, 0.0f,  0.0f, 1.0f, 0, 0.0f,   0.0f,  false, 0.0f, 0.0f,  false, 0.0f, 0.0f, 16, 1, 20000.0f, false },
    { "fifth-up",       7.0f,  7.0f, 0.0f,  0.0f, 1.0f, 0, 0.0f,   0.0f,  false, 0.0f, 0.0f,  false, 0.0f, 0.0f, 16, 1, 20000.0f, false },
    { "shimmer",       12.0f, 12.0f, 400.0f,0.50f,1.0f, 0, 0.0f,   0.0f,  true,  3.0f, 0.6f,  false, 0.0f, 0.0f, 16, 1, 20000.0f, false },
    { "dark-ambient",  -5.0f, -5.0f, 800.0f,0.70f,1.0f, 0, 0.0f,   0.0f,  true,  4.0f, 0.7f,  true,  0.4f, 3.0f, 16, 1, 20000.0f, false },
    { "glitch",         7.0f,  7.0f, 200.0f,0.80f,1.0f, 0, 0.0f,   0.0f,  false, 0.0f, 0.0f,  false, 0.0f, 0.0f,  8, 4,  6000.0f, false },
    { "robot",          0.0f,  0.0f, 0.0f,  0.0f, 1.0f, 2, 200.0f,  1.0f,  false, 0.0f, 0.0f,  false, 0.0f, 0.0f, 16, 1, 20000.0f, false },
    { "phaser-wash",    2.0f,  2.0f, 300.0f,0.40f,1.0f, 3, 0.3f,    0.8f,  false, 0.0f, 0.0f,  false, 0.0f, 0.0f, 16, 1, 20000.0f, false },
    // Stereo presets (different Voice A and Voice B)
    { "octave-split",  12.0f,-12.0f, 0.0f,  0.0f, 1.0f, 0, 0.0f,   0.0f,  false, 0.0f, 0.0f,  false, 0.0f, 0.0f, 16, 1, 20000.0f, false },
    { "fifth-harmony",  7.0f,  0.0f, 0.0f,  0.0f, 1.0f, 0, 0.0f,   0.0f,  false, 0.0f, 0.0f,  false, 0.0f, 0.0f, 16, 1, 20000.0f, false },
};

static constexpr int NUM_PRESETS = sizeof (PRESETS) / sizeof (PRESETS[0]);

static const Preset* findPreset (const char* name)
{
    for (int i = 0; i < NUM_PRESETS; ++i)
        if (std::strcmp (PRESETS[i].name, name) == 0)
            return &PRESETS[i];
    return nullptr;
}

// ============================================================================
// Main
// ============================================================================
int main (int argc, char* argv[])
{
    // Parse args
    if (argc < 4)
    {
        std::fprintf (stderr, "Barth Audios Demo Renderer\n\n");
        std::fprintf (stderr, "Usage: %s input.wav output.wav --preset <name> [--splice <mode>] [--vintage]\n\n", argv[0]);
        std::fprintf (stderr, "Presets:\n");
        for (int i = 0; i < NUM_PRESETS; ++i)
            std::fprintf (stderr, "  %s\n", PRESETS[i].name);
        std::fprintf (stderr, "\nSplice modes (default: fischer):\n");
        std::fprintf (stderr, "  fischer          Fundamental detection + zero-crossing (the real algorithm)\n");
        std::fprintf (stderr, "  autocorrelation  Pitch-period-aligned jumps (H949 style)\n");
        std::fprintf (stderr, "  zerocross        Zero-crossing detection (TTL-plausible)\n");
        std::fprintf (stderr, "  correlation      Cross-correlate segments (Publison style)\n");
        std::fprintf (stderr, "\nOptions:\n");
        std::fprintf (stderr, "  --vintage        Enable vintage character (crude pitch detect, 20ms grain, 12-bit/15kHz)\n");
        return 1;
    }

    const char* inputPath = argv[1];
    const char* outputPath = argv[2];
    const char* presetName = nullptr;
    const char* spliceModeName = nullptr;
    bool vintageFlag = false;

    for (int i = 3; i < argc; ++i)
    {
        if (std::strcmp (argv[i], "--preset") == 0 && i + 1 < argc)
            presetName = argv[++i];
        else if (std::strcmp (argv[i], "--splice") == 0 && i + 1 < argc)
            spliceModeName = argv[++i];
        else if (std::strcmp (argv[i], "--vintage") == 0)
            vintageFlag = true;
    }

    if (! presetName)
    {
        std::fprintf (stderr, "Error: --preset <name> required\n");
        return 1;
    }

    const Preset* preset = findPreset (presetName);
    if (! preset)
    {
        std::fprintf (stderr, "Error: unknown preset '%s'\n", presetName);
        return 1;
    }

    // Read input WAV
    drwav wav;
    if (! drwav_init_file (&wav, inputPath, nullptr))
    {
        std::fprintf (stderr, "Error: cannot open '%s'\n", inputPath);
        return 1;
    }

    const unsigned int sampleRate = wav.sampleRate;
    const unsigned int channels = wav.channels;
    const drwav_uint64 totalFrames = wav.totalPCMFrameCount;

    std::fprintf (stderr, "Input: %s (%u Hz, %u ch, %llu frames)\n",
                  inputPath, sampleRate, channels, (unsigned long long) totalFrames);
    std::fprintf (stderr, "Preset: %s (pitchA=%.1f, pitchB=%.1f)\n", preset->name, preset->pitchA, preset->pitchB);

    // Read all samples as float interleaved
    std::vector<float> inputSamples (totalFrames * channels);
    drwav_read_pcm_frames_f32 (&wav, totalFrames, inputSamples.data());
    drwav_uninit (&wav);

    // De-interleave to mono/stereo
    std::vector<float> inL (totalFrames);
    std::vector<float> inR (totalFrames);

    if (channels == 1)
    {
        for (drwav_uint64 i = 0; i < totalFrames; ++i)
            inL[i] = inR[i] = inputSamples[i];
    }
    else
    {
        for (drwav_uint64 i = 0; i < totalFrames; ++i)
        {
            inL[i] = inputSamples[i * channels];
            inR[i] = inputSamples[i * channels + 1];
        }
    }

    // Add tail for delay/reverb (5 seconds of silence)
    const auto tailFrames = static_cast<drwav_uint64> (sampleRate * 5);
    const auto outputFrames = totalFrames + tailFrames;
    inL.resize (outputFrames, 0.0f);
    inR.resize (outputFrames, 0.0f);

    // Prepare DSP chain
    const double sr = static_cast<double> (sampleRate);
    const int blockSize = 512;

    OnePoleFilter antiAliasL, antiAliasR;
    PitchShifter pitchL, pitchR;
    TimeStretcher stretchL, stretchR;
    DelayLine delayL, delayR;
    FrequencyShifter freqShiftL, freqShiftR;
    RingModulator ringModL, ringModR;
    PhaseShifter phaser;
    Distortion distL, distR;
    StereoReverb reverb;
    LoFiProcessor loFiL, loFiR;
    SoftClipper clipperL, clipperR;
    SignalRouter router;

    // Prepare all components
    antiAliasL.prepare (sr);
    antiAliasR.prepare (sr);
    pitchL.prepare (sr, blockSize);
    pitchR.prepare (sr, blockSize);
    stretchL.prepare (sr, blockSize);
    stretchR.prepare (sr, blockSize);
    delayL.prepare (sr, blockSize);
    delayR.prepare (sr, blockSize);
    freqShiftL.prepare (sr);
    freqShiftR.prepare (sr);
    ringModL.prepare (sr);
    ringModR.prepare (sr);
    phaser.prepare (sr);
    distL.prepare (sr);
    distR.prepare (sr);
    reverb.prepare (sr);
    loFiL.prepare (sr);
    loFiR.prepare (sr);
    clipperL.setThreshold (0.95f);
    clipperR.setThreshold (0.95f);

    // Configure from preset — Voice A on left, Voice B on right
    pitchL.setPitchSemitones (preset->pitchA);
    pitchR.setPitchSemitones (preset->pitchB);
    pitchL.setGrainSizeMs (80.0f);
    pitchR.setGrainSizeMs (80.0f);
    pitchL.setPortamento (0.0f); // instant for offline
    pitchR.setPortamento (0.0f);

    // Splice mode selection (default is Fischer — the real algorithm)
    if (spliceModeName)
    {
        PitchShifter::SpliceMode mode = PitchShifter::SpliceMode::Fischer;
        if (std::strcmp (spliceModeName, "autocorrelation") == 0)
            mode = PitchShifter::SpliceMode::Autocorrelation;
        else if (std::strcmp (spliceModeName, "zerocross") == 0)
            mode = PitchShifter::SpliceMode::ZeroCrossing;
        else if (std::strcmp (spliceModeName, "correlation") == 0)
            mode = PitchShifter::SpliceMode::Correlation;
        else if (std::strcmp (spliceModeName, "fischer") != 0)
            std::fprintf (stderr, "Warning: unknown splice mode '%s', using fischer\n", spliceModeName);
        pitchL.setSpliceMode (mode);
        pitchR.setSpliceMode (mode);
        std::fprintf (stderr, "Splice mode: %s\n", spliceModeName);
    }

    // Vintage character mode
    bool useVintage = preset->vintage || vintageFlag;
    pitchL.setVintageCharacter (useVintage);
    pitchR.setVintageCharacter (useVintage);
    if (useVintage)
        std::fprintf (stderr, "Vintage character: ON\n");

    stretchL.setStretchRatio (1.0f); // no time stretching for demos
    stretchR.setStretchRatio (1.0f);

    delayL.setDelayMs (preset->delayMs);
    delayR.setDelayMs (preset->delayMs);
    delayL.setFeedback (preset->feedbackPct);
    delayR.setFeedback (preset->feedbackPct);

    // Anti-alias filter cutoff
    antiAliasL.setCutoff (preset->lpfCutoff);
    antiAliasR.setCutoff (preset->lpfCutoff);

    // Effects
    switch (preset->fxSelect)
    {
        case 1:
            freqShiftL.setShiftHz (preset->fxFreq);
            freqShiftR.setShiftHz (preset->fxFreq);
            break;
        case 2:
            ringModL.setFrequency (preset->fxFreq);
            ringModR.setFrequency (preset->fxFreq);
            ringModL.setDepth (preset->fxDepth);
            ringModR.setDepth (preset->fxDepth);
            break;
        case 3:
            phaser.setRate (preset->fxFreq);
            phaser.setDepth (preset->fxDepth);
            break;
        default:
            break;
    }

    if (preset->reverbEnabled)
    {
        reverb.setDecayTime (preset->reverbDecay);
        reverb.setRoomSize (preset->reverbSize);
    }

    if (preset->distEnabled)
    {
        distL.setDrive (preset->distDrive);
        distR.setDrive (preset->distDrive);
        distL.setBassBoostDb (preset->distBassBoost);
        distR.setBassBoostDb (preset->distBassBoost);
    }

    loFiL.setBitDepth (preset->bitDepth);
    loFiR.setBitDepth (preset->bitDepth);
    loFiL.setSampleRateDivider (preset->srDivider);
    loFiR.setSampleRateDivider (preset->srDivider);
    loFiL.setLPFCutoff (preset->lpfCutoff);
    loFiR.setLPFCutoff (preset->lpfCutoff);

    router.setMix (preset->mix);
    router.setOutputMode (SignalRouter::OutputMode::Stereo);

    // Process sample-by-sample (matches PluginProcessor::processBlock signal flow)
    std::vector<float> outL (outputFrames);
    std::vector<float> outR (outputFrames);

    for (drwav_uint64 i = 0; i < outputFrames; ++i)
    {
        float dryL = inL[i];
        float dryR = inR[i];

        // 1. Anti-alias LPF
        float filtL = antiAliasL.processLowPass (dryL);
        float filtR = antiAliasR.processLowPass (dryR);

        // 2. Pitch shift
        float pitchedL = pitchL.processSample (filtL);
        float pitchedR = pitchR.processSample (filtR);

        // 3. Time stretch
        float stretchedL = stretchL.processSample (pitchedL);
        float stretchedR = stretchR.processSample (pitchedR);

        // 4. Delay
        float delayedL = delayL.processSample (stretchedL);
        float delayedR = delayR.processSample (stretchedR);

        float wetL = delayedL;
        float wetR = delayedR;

        // 5. Effects
        switch (preset->fxSelect)
        {
            case 1:
                wetL = freqShiftL.processSample (wetL);
                wetR = freqShiftR.processSample (wetR);
                break;
            case 2:
                wetL = ringModL.processSample (wetL);
                wetR = ringModR.processSample (wetR);
                break;
            case 3:
                wetL = phaser.processSample (wetL, 0);
                wetR = phaser.processSample (wetR, 1);
                break;
            default:
                break;
        }

        // 6. Distortion
        if (preset->distEnabled)
        {
            wetL = distL.processSample (wetL);
            wetR = distR.processSample (wetR);
        }

        // 7. Reverb
        if (preset->reverbEnabled)
        {
            float revL, revR;
            reverb.processSample (wetL, wetR, revL, revR);
            wetL = revL;
            wetR = revR;
        }

        // 8. Lo-Fi
        wetL = loFiL.processSample (wetL);
        wetR = loFiR.processSample (wetR);

        // 9. Output soft clipper
        wetL = clipperL.processSample (wetL);
        wetR = clipperR.processSample (wetR);

        // 10. Signal routing (wet/dry mix)
        float finalL, finalR;
        router.processStereo (dryL, dryR, wetL, wetR, finalL, finalR);

        outL[i] = finalL;
        outR[i] = finalR;
    }

    // Trim trailing silence (find last sample above -60dB)
    const float silenceThreshold = 0.001f; // ~-60dB
    drwav_uint64 trimEnd = outputFrames;
    while (trimEnd > totalFrames)
    {
        if (std::abs (outL[trimEnd - 1]) > silenceThreshold ||
            std::abs (outR[trimEnd - 1]) > silenceThreshold)
            break;
        --trimEnd;
    }
    // Add a small fade-out (50ms)
    const auto fadeLen = static_cast<drwav_uint64> (sampleRate * 0.05);
    if (trimEnd + fadeLen < outputFrames)
        trimEnd += fadeLen;
    else
        trimEnd = outputFrames;

    // Apply fade-out to last 50ms
    if (trimEnd > fadeLen)
    {
        for (drwav_uint64 i = 0; i < fadeLen; ++i)
        {
            float gain = 1.0f - static_cast<float> (i) / static_cast<float> (fadeLen);
            auto idx = trimEnd - fadeLen + i;
            outL[idx] *= gain;
            outR[idx] *= gain;
        }
    }

    // Check for NaN/Inf
    int badSamples = 0;
    for (drwav_uint64 i = 0; i < trimEnd; ++i)
    {
        if (std::isnan (outL[i]) || std::isinf (outL[i])) { outL[i] = 0.0f; ++badSamples; }
        if (std::isnan (outR[i]) || std::isinf (outR[i])) { outR[i] = 0.0f; ++badSamples; }
    }
    if (badSamples > 0)
        std::fprintf (stderr, "Warning: %d NaN/Inf samples replaced with zero\n", badSamples);

    // Interleave for output
    std::vector<float> outputInterleaved (trimEnd * 2);
    for (drwav_uint64 i = 0; i < trimEnd; ++i)
    {
        outputInterleaved[i * 2] = outL[i];
        outputInterleaved[i * 2 + 1] = outR[i];
    }

    // Write output WAV (always stereo, same sample rate as input)
    drwav_data_format format;
    format.container = drwav_container_riff;
    format.format = DR_WAVE_FORMAT_IEEE_FLOAT;
    format.channels = 2;
    format.sampleRate = sampleRate;
    format.bitsPerSample = 32;

    drwav wavOut;
    if (! drwav_init_file_write (&wavOut, outputPath, &format, nullptr))
    {
        std::fprintf (stderr, "Error: cannot create '%s'\n", outputPath);
        return 1;
    }

    drwav_write_pcm_frames (&wavOut, trimEnd, outputInterleaved.data());
    drwav_uninit (&wavOut);

    // Peak level
    float peak = 0.0f;
    for (drwav_uint64 i = 0; i < trimEnd; ++i)
    {
        peak = std::max (peak, std::abs (outL[i]));
        peak = std::max (peak, std::abs (outR[i]));
    }
    float peakDb = (peak > 0.0f) ? 20.0f * std::log10 (peak) : -120.0f;

    std::fprintf (stderr, "Output: %s (%llu frames, peak %.1f dBFS)\n",
                  outputPath, (unsigned long long) trimEnd, peakDb);

    return 0;
}
