#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout PluginProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // === Transpose Section: 4 programs x 2 voices ===
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "prog1PitchA", 1 }, "Prog 1 Voice A",
        juce::NormalisableRange<float> (-60.0f, 24.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "prog1PitchB", 1 }, "Prog 1 Voice B",
        juce::NormalisableRange<float> (-60.0f, 24.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "prog2PitchA", 1 }, "Prog 2 Voice A",
        juce::NormalisableRange<float> (-60.0f, 24.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "prog2PitchB", 1 }, "Prog 2 Voice B",
        juce::NormalisableRange<float> (-60.0f, 24.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "prog3PitchA", 1 }, "Prog 3 Voice A",
        juce::NormalisableRange<float> (-60.0f, 24.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "prog3PitchB", 1 }, "Prog 3 Voice B",
        juce::NormalisableRange<float> (-60.0f, 24.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "prog4PitchA", 1 }, "Prog 4 Voice A",
        juce::NormalisableRange<float> (-60.0f, 24.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "prog4PitchB", 1 }, "Prog 4 Voice B",
        juce::NormalisableRange<float> (-60.0f, 24.0f, 0.01f), 0.0f));

    // Program selector (when sequencer is off)
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "activeProgram", 1 }, "Active Program",
        juce::StringArray { "1", "2", "3", "4" }, 0));

    // === Core Section ===
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "grain", 1 }, "Grain Size",
        juce::NormalisableRange<float> (5.0f, 80.0f, 0.1f), 40.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "portamento", 1 }, "Portamento",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "stretch", 1 }, "Time Stretch",
        juce::NormalisableRange<float> (25.0f, 400.0f, 0.1f), 100.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "delay", 1 }, "Delay Time",
        juce::NormalisableRange<float> (0.0f, 5000.0f, 0.1f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "feedback", 1 }, "Feedback",
        juce::NormalisableRange<float> (0.0f, 95.0f, 0.1f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "mix", 1 }, "Mix",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 50.0f));
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { "vintageChar", 1 }, "Vintage Character", false));

    // === Sequencer Section ===
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { "seqEnabled", 1 }, "Sequencer", false));
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "seqMode", 1 }, "Seq Mode",
        juce::StringArray { "Forward", "Backward", "Random", "Ping-Pong", "2-Step", "3-Step", "4-Step" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "seqRate", 1 }, "Seq Rate",
        juce::NormalisableRange<float> (0.1f, 40.0f, 0.01f, 0.3f), 2.0f));
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { "seqSync", 1 }, "Seq Sync", false));
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "seqDivision", 1 }, "Seq Division",
        juce::StringArray { "1/1", "1/2", "1/4", "1/8", "1/16", "1/32" }, 2));
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { "envFollow", 1 }, "Env Follower", false));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "envThreshold", 1 }, "Env Threshold",
        juce::NormalisableRange<float> (-60.0f, 0.0f, 0.1f), -20.0f));

    // === Effects Section ===
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "fxSelect", 1 }, "Effect Type",
        juce::StringArray { "Off", "Freq Shift", "Ring Mod", "Phaser" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "fxFreq", 1 }, "Effect Freq",
        juce::NormalisableRange<float> (0.1f, 5000.0f, 0.01f, 0.3f), 100.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "fxDepth", 1 }, "Effect Depth",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 50.0f));
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "fxStereo", 1 }, "Stereo Mode",
        juce::StringArray { "Mono", "Stereo", "Tandem" }, 1));

    // Reverb
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { "reverbEnabled", 1 }, "Reverb", false));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "reverbDecay", 1 }, "Reverb Decay",
        juce::NormalisableRange<float> (0.1f, 10.0f, 0.01f), 1.5f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "reverbSize", 1 }, "Room Size",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 50.0f));

    // Distortion
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { "distEnabled", 1 }, "Distortion", false));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "distDrive", 1 }, "Drive",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "distBassBoost", 1 }, "Bass Boost",
        juce::NormalisableRange<float> (-12.0f, 12.0f, 0.1f), 0.0f));

    // === ADSR Section ===
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { "adsrEnabled", 1 }, "ADSR", false));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "adsrAttack", 1 }, "Attack",
        juce::NormalisableRange<float> (1.0f, 5000.0f, 0.1f, 0.3f), 10.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "adsrDecay", 1 }, "Decay",
        juce::NormalisableRange<float> (1.0f, 5000.0f, 0.1f, 0.3f), 100.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "adsrSustain", 1 }, "Sustain",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 70.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "adsrRelease", 1 }, "Release",
        juce::NormalisableRange<float> (1.0f, 10000.0f, 0.1f, 0.3f), 200.0f));
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "adsrTrigger", 1 }, "Trigger Source",
        juce::StringArray { "Transient", "Keyboard", "Sequencer" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "adsrRouting", 1 }, "Routing",
        juce::StringArray { "Pre-Reverb", "Post-Reverb" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { "adsrInvert", 1 }, "Invert", false));

    // === Lo-Fi + Routing Section ===
    params.push_back (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { "bitDepth", 1 }, "Bit Depth", 4, 16, 16));
    params.push_back (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { "srDiv", 1 }, "SR Divider", 1, 16, 1));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "lpfCutoff", 1 }, "LPF",
        juce::NormalisableRange<float> (200.0f, 20000.0f, 1.0f, 0.3f), 20000.0f));
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "monoMode", 1 }, "Output Mode",
        juce::StringArray { "Stereo", "Mono", "Diff" }, 0));

    return { params.begin(), params.end() };
}

//==============================================================================
PluginProcessor::PluginProcessor()
    : AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
#endif
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
                          ),
      apvts_ (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    // Cache atomic parameter pointers (safe: pointers are stable for plugin lifetime)
    progPitchAParams_[0] = apvts_.getRawParameterValue ("prog1PitchA");
    progPitchBParams_[0] = apvts_.getRawParameterValue ("prog1PitchB");
    progPitchAParams_[1] = apvts_.getRawParameterValue ("prog2PitchA");
    progPitchBParams_[1] = apvts_.getRawParameterValue ("prog2PitchB");
    progPitchAParams_[2] = apvts_.getRawParameterValue ("prog3PitchA");
    progPitchBParams_[2] = apvts_.getRawParameterValue ("prog3PitchB");
    progPitchAParams_[3] = apvts_.getRawParameterValue ("prog4PitchA");
    progPitchBParams_[3] = apvts_.getRawParameterValue ("prog4PitchB");
    activeProgramParam_ = apvts_.getRawParameterValue ("activeProgram");

    grainParam_ = apvts_.getRawParameterValue ("grain");
    portamentoParam_ = apvts_.getRawParameterValue ("portamento");
    stretchParam_ = apvts_.getRawParameterValue ("stretch");
    delayParam_ = apvts_.getRawParameterValue ("delay");
    feedbackParam_ = apvts_.getRawParameterValue ("feedback");
    mixParam_ = apvts_.getRawParameterValue ("mix");

    seqEnabledParam_ = apvts_.getRawParameterValue ("seqEnabled");
    seqModeParam_ = apvts_.getRawParameterValue ("seqMode");
    seqRateParam_ = apvts_.getRawParameterValue ("seqRate");
    seqSyncParam_ = apvts_.getRawParameterValue ("seqSync");
    seqDivisionParam_ = apvts_.getRawParameterValue ("seqDivision");
    envFollowParam_ = apvts_.getRawParameterValue ("envFollow");
    envThresholdParam_ = apvts_.getRawParameterValue ("envThreshold");

    fxSelectParam_ = apvts_.getRawParameterValue ("fxSelect");
    fxFreqParam_ = apvts_.getRawParameterValue ("fxFreq");
    fxDepthParam_ = apvts_.getRawParameterValue ("fxDepth");
    fxStereoParam_ = apvts_.getRawParameterValue ("fxStereo");

    reverbEnabledParam_ = apvts_.getRawParameterValue ("reverbEnabled");
    reverbDecayParam_ = apvts_.getRawParameterValue ("reverbDecay");
    reverbSizeParam_ = apvts_.getRawParameterValue ("reverbSize");

    distEnabledParam_ = apvts_.getRawParameterValue ("distEnabled");
    distDriveParam_ = apvts_.getRawParameterValue ("distDrive");
    distBassBoostParam_ = apvts_.getRawParameterValue ("distBassBoost");

    adsrEnabledParam_ = apvts_.getRawParameterValue ("adsrEnabled");
    adsrAttackParam_ = apvts_.getRawParameterValue ("adsrAttack");
    adsrDecayParam_ = apvts_.getRawParameterValue ("adsrDecay");
    adsrSustainParam_ = apvts_.getRawParameterValue ("adsrSustain");
    adsrReleaseParam_ = apvts_.getRawParameterValue ("adsrRelease");
    adsrTriggerParam_ = apvts_.getRawParameterValue ("adsrTrigger");
    adsrRoutingParam_ = apvts_.getRawParameterValue ("adsrRouting");
    adsrInvertParam_ = apvts_.getRawParameterValue ("adsrInvert");

    bitDepthParam_ = apvts_.getRawParameterValue ("bitDepth");
    srDivParam_ = apvts_.getRawParameterValue ("srDiv");
    lpfCutoffParam_ = apvts_.getRawParameterValue ("lpfCutoff");
    monoModeParam_ = apvts_.getRawParameterValue ("monoMode");
    vintageCharParam_ = apvts_.getRawParameterValue ("vintageChar");
}

PluginProcessor::~PluginProcessor()
{
}

//==============================================================================
const juce::String PluginProcessor::getName() const { return JucePlugin_Name; }
bool PluginProcessor::acceptsMidi() const { return false; }
bool PluginProcessor::producesMidi() const { return false; }
bool PluginProcessor::isMidiEffect() const { return false; }

double PluginProcessor::getTailLengthSeconds() const
{
    // Account for delay + reverb tail
    return 10.0;
}

int PluginProcessor::getNumPrograms() { return 1; }
int PluginProcessor::getCurrentProgram() { return 0; }
void PluginProcessor::setCurrentProgram (int index) { juce::ignoreUnused (index); }
const juce::String PluginProcessor::getProgramName (int index) { juce::ignoreUnused (index); return {}; }
void PluginProcessor::changeProgramName (int index, const juce::String& newName) { juce::ignoreUnused (index, newName); }

//==============================================================================
void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Core DSP
    pitchShifterL_.prepare (sampleRate, samplesPerBlock);
    pitchShifterR_.prepare (sampleRate, samplesPerBlock);
    timeStretcherL_.prepare (sampleRate, samplesPerBlock);
    timeStretcherR_.prepare (sampleRate, samplesPerBlock);
    delayLineL_.prepare (sampleRate, samplesPerBlock);
    delayLineR_.prepare (sampleRate, samplesPerBlock);

    // Effects
    freqShifterL_.prepare (sampleRate);
    freqShifterR_.prepare (sampleRate);
    ringModL_.prepare (sampleRate);
    ringModR_.prepare (sampleRate);
    phaseShifter_.prepare (sampleRate);
    reverb_.prepare (sampleRate);
    distortionL_.prepare (sampleRate);
    distortionR_.prepare (sampleRate);
    adsr_.prepare (sampleRate);

    // Utilities
    antiAliasL_.prepare (sampleRate);
    antiAliasR_.prepare (sampleRate);
    loFiL_.prepare (sampleRate);
    loFiR_.prepare (sampleRate);
    envFollower_.prepare (sampleRate);

    outputClipperL_.setThreshold (0.95f);
    outputClipperR_.setThreshold (0.95f);

    // Sequencer
    sequencer_.prepare (sampleRate, samplesPerBlock);

    // Smoothed values
    pitchASmoothed_.reset (sampleRate, 0.05);
    pitchBSmoothed_.reset (sampleRate, 0.05);
    grainSmoothed_.reset (sampleRate, 0.02);
    stretchSmoothed_.reset (sampleRate, 0.02);
    delaySmoothed_.reset (sampleRate, 0.02);
    feedbackSmoothed_.reset (sampleRate, 0.01);
    mixSmoothed_.reset (sampleRate, 0.01);

    // Effects smoothing (per-block rate: longer ramp, ~50ms)
    fxFreqSmoothed_.reset (sampleRate, 0.05);
    fxDepthSmoothed_.reset (sampleRate, 0.05);
    distDriveSmoothed_.reset (sampleRate, 0.05);
    distBassBoostSmoothed_.reset (sampleRate, 0.05);
    lpfCutoffSmoothed_.reset (sampleRate, 0.05);
    reverbDecaySmoothed_.reset (sampleRate, 0.05);
    reverbSizeSmoothed_.reset (sampleRate, 0.05);

    // Report latency (half grain size)
    float grainMs = grainParam_->load();
    int latencySamples = static_cast<int> (grainMs * static_cast<float> (sampleRate) / 1000.0f * 0.5f);
    setLatencySamples (latencySamples);
    lastReportedLatency_ = latencySamples;
}

void PluginProcessor::releaseResources()
{
}

bool PluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& outSet = layouts.getMainOutputChannelSet();
    if (outSet != juce::AudioChannelSet::mono()
        && outSet != juce::AudioChannelSet::stereo())
        return false;

#if ! JucePlugin_IsSynth
    const auto& inSet = layouts.getMainInputChannelSet();
    // Allow mono-in -> stereo-out as well as matching layouts
    if (inSet != outSet
        && ! (inSet == juce::AudioChannelSet::mono() && outSet == juce::AudioChannelSet::stereo()))
        return false;
#endif

    return true;
}

int PluginProcessor::getActiveProgram() const
{
    if (seqEnabledParam_->load() > 0.5f)
        return sequencer_.getCurrentStep();
    return juce::jlimit (0, 3, static_cast<int> (activeProgramParam_->load()));
}

void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Mono-in -> stereo-out: duplicate input channel
    if (totalNumInputChannels == 1 && totalNumOutputChannels == 2)
        buffer.copyFrom (1, 0, buffer, 0, 0, buffer.getNumSamples());

    const int numSamples = buffer.getNumSamples();

    // Read all 8 program pitches into stack arrays
    float progPitchA[4], progPitchB[4];
    for (int p = 0; p < 4; ++p)
    {
        progPitchA[p] = progPitchAParams_[p]->load();
        progPitchB[p] = progPitchBParams_[p]->load();
    }
    const int manualProg = static_cast<int> (activeProgramParam_->load());

    // Read other parameters atomically
    const float grainMs = grainParam_->load();
    const float portamento = portamentoParam_->load() / 100.0f;
    const float stretchPct = stretchParam_->load() / 100.0f;
    const float delayMs = delayParam_->load();
    const float feedback = feedbackParam_->load() / 100.0f;
    const float mixPct = mixParam_->load() / 100.0f;

    const bool seqEnabled = seqEnabledParam_->load() > 0.5f;
    const int seqMode = static_cast<int> (seqModeParam_->load());
    const float seqRate = seqRateParam_->load();
    const bool seqSync = seqSyncParam_->load() > 0.5f;
    const int seqDivIdx = static_cast<int> (seqDivisionParam_->load());
    const bool envFollow = envFollowParam_->load() > 0.5f;

    const int fxSelect = static_cast<int> (fxSelectParam_->load());
    const float fxFreq = fxFreqParam_->load();
    const float fxDepth = fxDepthParam_->load() / 100.0f;
    const int fxStereoMode = static_cast<int> (fxStereoParam_->load());

    const bool reverbEnabled = reverbEnabledParam_->load() > 0.5f;
    const float reverbDecay = reverbDecayParam_->load();
    const float reverbSize = reverbSizeParam_->load() / 100.0f;

    const bool distEnabled = distEnabledParam_->load() > 0.5f;
    const float distDrive = distDriveParam_->load() / 100.0f;
    const float distBassBoost = distBassBoostParam_->load();

    const bool adsrEnabled = adsrEnabledParam_->load() > 0.5f;
    const int adsrRouting = static_cast<int> (adsrRoutingParam_->load());

    const int bitDepth = static_cast<int> (bitDepthParam_->load());
    const int srDiv = static_cast<int> (srDivParam_->load());
    const float lpfCutoff = lpfCutoffParam_->load();
    const int monoMode = static_cast<int> (monoModeParam_->load());

    // Set initial smooth targets from active program
    int activeIdx = seqEnabled ? sequencer_.getCurrentStep() : manualProg;
    activeIdx = juce::jlimit (0, 3, activeIdx);
    pitchASmoothed_.setTargetValue (progPitchA[activeIdx]);
    pitchBSmoothed_.setTargetValue (progPitchB[activeIdx]);

    // Update other smoothed targets
    grainSmoothed_.setTargetValue (grainMs);
    stretchSmoothed_.setTargetValue (stretchPct);
    delaySmoothed_.setTargetValue (delayMs);
    feedbackSmoothed_.setTargetValue (feedback);
    mixSmoothed_.setTargetValue (mixPct);

    // Update non-smoothed DSP parameters (per-block, not per-sample)
    const bool vintageChar = vintageCharParam_->load() > 0.5f;
    pitchShifterL_.setVintageCharacter (vintageChar);
    pitchShifterR_.setVintageCharacter (vintageChar);
    pitchShifterL_.setPortamento (portamento);
    pitchShifterR_.setPortamento (portamento);

    sequencer_.setEnabled (seqEnabled);
    sequencer_.setMode (static_cast<StepSequencer::Mode> (seqMode));
    sequencer_.setRateHz (seqRate);
    sequencer_.setHostSync (seqSync);

    static constexpr float divisionValues[] = { 1.0f, 0.5f, 0.25f, 0.125f, 0.0625f, 0.03125f };
    sequencer_.setDivision (divisionValues[seqDivIdx < 6 ? seqDivIdx : 2]);

    envFollower_.setThresholdDb (envThresholdParam_->load());

    // Update effects smoothed targets
    fxFreqSmoothed_.setTargetValue (fxFreq);
    fxDepthSmoothed_.setTargetValue (fxDepth);
    distDriveSmoothed_.setTargetValue (distDrive);
    distBassBoostSmoothed_.setTargetValue (distBassBoost);
    lpfCutoffSmoothed_.setTargetValue (lpfCutoff);
    reverbDecaySmoothed_.setTargetValue (reverbDecay);
    reverbSizeSmoothed_.setTargetValue (reverbSize);

    // Read per-block smoothed values (one getNextValue per block is sufficient)
    const float smoothedFxFreq = fxFreqSmoothed_.getNextValue();
    const float smoothedFxDepth = fxDepthSmoothed_.getNextValue();
    const float smoothedDistDrive = distDriveSmoothed_.getNextValue();
    const float smoothedDistBassBoost = distBassBoostSmoothed_.getNextValue();
    const float smoothedLpfCutoff = lpfCutoffSmoothed_.getNextValue();
    const float smoothedReverbDecay = reverbDecaySmoothed_.getNextValue();
    const float smoothedReverbSize = reverbSizeSmoothed_.getNextValue();

    // Effects per-block setup (using smoothed values)
    switch (fxSelect)
    {
        case 1: // Freq Shift
            freqShifterL_.setShiftHz (smoothedFxFreq);
            freqShifterR_.setShiftHz (smoothedFxFreq);
            break;
        case 2: // Ring Mod
            ringModL_.setFrequency (smoothedFxFreq);
            ringModR_.setFrequency (smoothedFxFreq);
            ringModL_.setDepth (smoothedFxDepth);
            ringModR_.setDepth (smoothedFxDepth);
            break;
        case 3: // Phaser
            phaseShifter_.setRate (smoothedFxFreq);
            phaseShifter_.setDepth (smoothedFxDepth);
            phaseShifter_.setStereoMode (static_cast<PhaseShifter::StereoMode> (fxStereoMode));
            break;
        default:
            break;
    }

    if (reverbEnabled)
    {
        reverb_.setDecayTime (smoothedReverbDecay);
        reverb_.setRoomSize (smoothedReverbSize);
    }

    if (distEnabled)
    {
        distortionL_.setDrive (smoothedDistDrive);
        distortionR_.setDrive (smoothedDistDrive);
        distortionL_.setBassBoostDb (smoothedDistBassBoost);
        distortionR_.setBassBoostDb (smoothedDistBassBoost);
    }

    if (adsrEnabled)
    {
        adsr_.setAttackMs (adsrAttackParam_->load());
        adsr_.setDecayMs (adsrDecayParam_->load());
        adsr_.setSustain (adsrSustainParam_->load() / 100.0f);
        adsr_.setReleaseMs (adsrReleaseParam_->load());
        adsr_.setInverted (adsrInvertParam_->load() > 0.5f);
        adsr_.updateCoefficients();
    }

    loFiL_.setBitDepth (bitDepth);
    loFiR_.setBitDepth (bitDepth);
    loFiL_.setSampleRateDivider (srDiv);
    loFiR_.setSampleRateDivider (srDiv);
    loFiL_.setLPFCutoff (smoothedLpfCutoff);
    loFiR_.setLPFCutoff (smoothedLpfCutoff);
    antiAliasL_.setCutoff (smoothedLpfCutoff);
    antiAliasR_.setCutoff (smoothedLpfCutoff);

    router_.setOutputMode (static_cast<SignalRouter::OutputMode> (monoMode));

    // Get host playhead info for sequencer sync
    double bpm = 120.0;
    double ppqPosition = 0.0;
    bool isPlaying = false;

    if (auto* playHead = getPlayHead())
    {
        if (auto posInfo = playHead->getPosition())
        {
            if (posInfo->getBpm().hasValue())
                bpm = *posInfo->getBpm();
            if (posInfo->getPpqPosition().hasValue())
                ppqPosition = *posInfo->getPpqPosition();
            isPlaying = posInfo->getIsPlaying();
        }
    }

    // Set grain size and stretch ratio once per block (no need for per-sample updates)
    float blockGrain = grainSmoothed_.getCurrentValue();
    pitchShifterL_.setGrainSizeMs (blockGrain);
    pitchShifterR_.setGrainSizeMs (blockGrain);

    float blockStretch = stretchSmoothed_.getCurrentValue();
    timeStretcherL_.setStretchRatio (blockStretch);
    timeStretcherR_.setStretchRatio (blockStretch);

    // Process sample-by-sample
    auto* dataL = buffer.getWritePointer (0);
    auto* dataR = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : dataL;

    // Cache sample rate and precompute PPQ divisor (avoid virtual call per sample)
    const double sr = getSampleRate();
    const double ppqPerSample = (bpm > 0.0 && sr > 0.0) ? bpm / (60.0 * sr) : 0.0;

    for (int i = 0; i < numSamples; ++i)
    {
        float dryL = dataL[i];
        float dryR = dataR[i];

        // Sequencer: advance timing (void, just timing)
        sequencer_.processSample (bpm, ppqPosition + static_cast<double> (i) * ppqPerSample, isPlaying);

        // Envelope follower trigger
        if (envFollow && seqEnabled)
        {
            float monoInput = (dryL + dryR) * 0.5f;
            if (envFollower_.detectTransient (monoInput))
                sequencer_.triggerNextStep();
        }

        // Re-check active program (sequencer may have advanced mid-block)
        if (seqEnabled)
        {
            int newIdx = juce::jlimit (0, 3, sequencer_.getCurrentStep());
            pitchASmoothed_.setTargetValue (progPitchA[newIdx]);
            pitchBSmoothed_.setTargetValue (progPitchB[newIdx]);
        }

        // Get smoothed pitch values
        float curPitchA = pitchASmoothed_.getNextValue();
        float curPitchB = pitchBSmoothed_.getNextValue();
        float curDelay = delaySmoothed_.getNextValue();
        float curFeedback = feedbackSmoothed_.getNextValue();
        float curMix = mixSmoothed_.getNextValue();

        // Apply pitch directly from program (no sequencer offset)
        pitchShifterL_.setPitchSemitones (curPitchA);
        pitchShifterR_.setPitchSemitones (curPitchB);

        // Anti-alias LPF
        float filtL = antiAliasL_.processLowPass (dryL);
        float filtR = antiAliasR_.processLowPass (dryR);

        // Pitch shift
        float pitchedL = pitchShifterL_.processSample (filtL);
        float pitchedR = pitchShifterR_.processSample (filtR);

        // Time stretch (ratio set per-block above)
        float stretchedL = timeStretcherL_.processSample (pitchedL);
        float stretchedR = timeStretcherR_.processSample (pitchedR);

        // Delay
        delayLineL_.setDelayMs (curDelay);
        delayLineR_.setDelayMs (curDelay);
        delayLineL_.setFeedback (curFeedback);
        delayLineR_.setFeedback (curFeedback);
        float delayedL = delayLineL_.processSample (stretchedL);
        float delayedR = delayLineR_.processSample (stretchedR);

        float wetL = delayedL;
        float wetR = delayedR;

        // ADSR (pre-reverb routing)
        if (adsrEnabled && adsrRouting == 0)
        {
            float env = adsr_.processSample();
            wetL *= env;
            wetR *= env;
        }

        // Effects
        switch (fxSelect)
        {
            case 1:
                wetL = freqShifterL_.processSample (wetL);
                wetR = freqShifterR_.processSample (wetR);
                break;
            case 2:
                wetL = ringModL_.processSample (wetL);
                wetR = ringModR_.processSample (wetR);
                break;
            case 3:
                wetL = phaseShifter_.processSample (wetL, 0);
                wetR = phaseShifter_.processSample (wetR, 1);
                break;
            default:
                break;
        }

        // Distortion
        if (distEnabled)
        {
            wetL = distortionL_.processSample (wetL);
            wetR = distortionR_.processSample (wetR);
        }

        // Reverb
        if (reverbEnabled)
        {
            float revL, revR;
            reverb_.processSample (wetL, wetR, revL, revR);
            wetL = revL;
            wetR = revR;
        }

        // ADSR (post-reverb routing)
        if (adsrEnabled && adsrRouting == 1)
        {
            float env = adsr_.processSample();
            wetL *= env;
            wetR *= env;
        }

        // Lo-Fi
        wetL = loFiL_.processSample (wetL);
        wetR = loFiR_.processSample (wetR);

        // Output soft clipper
        wetL = outputClipperL_.processSample (wetL);
        wetR = outputClipperR_.processSample (wetR);

        // Signal routing (wet/dry mix + mono/diff)
        float outL, outR;
        router_.setMix (curMix);
        router_.processStereo (dryL, dryR, wetL, wetR, outL, outR);

        dataL[i] = outL;
        if (buffer.getNumChannels() > 1)
            dataR[i] = outR;
    }

    // Advance grain/stretch smoothers by block size (values used per-block, not per-sample)
    grainSmoothed_.skip (numSamples);
    stretchSmoothed_.skip (numSamples);

    // Update latency if grain size changed significantly (> 1ms difference)
    int newLatency = static_cast<int> (grainSmoothed_.getCurrentValue() * static_cast<float> (sr) / 1000.0f * 0.5f);
    if (std::abs (newLatency - lastReportedLatency_) > static_cast<int> (sr * 0.001))
    {
        setLatencySamples (newLatency);
        lastReportedLatency_ = newLatency;
    }
}

//==============================================================================
bool PluginProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor (*this);
}

//==============================================================================
void PluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts_.copyState();
    auto xml = state.createXml();
    xml->setAttribute ("version", 1);
    copyXmlToBinary (*xml, destData);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary (data, sizeInBytes);
    if (xml && xml->hasTagName (apvts_.state.getType()))
    {
        int version = xml->getIntAttribute ("version", 1);
        juce::ignoreUnused (version);
        apvts_.replaceState (juce::ValueTree::fromXml (*xml));
    }
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}
