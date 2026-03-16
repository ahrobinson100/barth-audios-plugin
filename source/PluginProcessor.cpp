#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout PluginProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // === Core Section ===
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "pitchL", 1 }, "Pitch L",
        juce::NormalisableRange<float> (-60.0f, 24.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "pitchR", 1 }, "Pitch R",
        juce::NormalisableRange<float> (-60.0f, 24.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { "pitchLink", 1 }, "Link L/R", true));
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

    // === Sequencer Section ===
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { "seqEnabled", 1 }, "Sequencer", false));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "seqStep1", 1 }, "Step 1",
        juce::NormalisableRange<float> (-60.0f, 24.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "seqStep2", 1 }, "Step 2",
        juce::NormalisableRange<float> (-60.0f, 24.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "seqStep3", 1 }, "Step 3",
        juce::NormalisableRange<float> (-60.0f, 24.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "seqStep4", 1 }, "Step 4",
        juce::NormalisableRange<float> (-60.0f, 24.0f, 0.01f), 0.0f));
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
    pitchLParam_ = apvts_.getRawParameterValue ("pitchL");
    pitchRParam_ = apvts_.getRawParameterValue ("pitchR");
    pitchLinkParam_ = apvts_.getRawParameterValue ("pitchLink");
    grainParam_ = apvts_.getRawParameterValue ("grain");
    portamentoParam_ = apvts_.getRawParameterValue ("portamento");
    stretchParam_ = apvts_.getRawParameterValue ("stretch");
    delayParam_ = apvts_.getRawParameterValue ("delay");
    feedbackParam_ = apvts_.getRawParameterValue ("feedback");
    mixParam_ = apvts_.getRawParameterValue ("mix");

    seqEnabledParam_ = apvts_.getRawParameterValue ("seqEnabled");
    seqStep1Param_ = apvts_.getRawParameterValue ("seqStep1");
    seqStep2Param_ = apvts_.getRawParameterValue ("seqStep2");
    seqStep3Param_ = apvts_.getRawParameterValue ("seqStep3");
    seqStep4Param_ = apvts_.getRawParameterValue ("seqStep4");
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
    pitchLSmoothed_.reset (sampleRate, 0.05);
    pitchRSmoothed_.reset (sampleRate, 0.05);
    grainSmoothed_.reset (sampleRate, 0.02);
    stretchSmoothed_.reset (sampleRate, 0.02);
    delaySmoothed_.reset (sampleRate, 0.02);
    feedbackSmoothed_.reset (sampleRate, 0.01);
    mixSmoothed_.reset (sampleRate, 0.01);

    // Report latency (half grain size)
    float grainMs = grainParam_->load();
    int latencySamples = static_cast<int> (grainMs * static_cast<float> (sampleRate) / 1000.0f * 0.5f);
    setLatencySamples (latencySamples);
}

void PluginProcessor::releaseResources()
{
}

bool PluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
}

void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    const int numSamples = buffer.getNumSamples();

    // Read all parameters atomically (no locks)
    const float pitchLSt = pitchLParam_->load();
    const float pitchRSt = pitchLinkParam_->load() > 0.5f ? pitchLSt : pitchRParam_->load();
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

    // Update smoothed targets
    pitchLSmoothed_.setTargetValue (pitchLSt);
    pitchRSmoothed_.setTargetValue (pitchRSt);
    grainSmoothed_.setTargetValue (grainMs);
    stretchSmoothed_.setTargetValue (stretchPct);
    delaySmoothed_.setTargetValue (delayMs);
    feedbackSmoothed_.setTargetValue (feedback);
    mixSmoothed_.setTargetValue (mixPct);

    // Update non-smoothed DSP parameters (per-block, not per-sample)
    pitchShifterL_.setPortamento (portamento);
    pitchShifterR_.setPortamento (portamento);

    sequencer_.setEnabled (seqEnabled);
    sequencer_.setMode (static_cast<StepSequencer::Mode> (seqMode));
    sequencer_.setRateHz (seqRate);
    sequencer_.setHostSync (seqSync);
    sequencer_.setStepPitch (0, seqStep1Param_->load());
    sequencer_.setStepPitch (1, seqStep2Param_->load());
    sequencer_.setStepPitch (2, seqStep3Param_->load());
    sequencer_.setStepPitch (3, seqStep4Param_->load());

    static constexpr float divisionValues[] = { 1.0f, 0.5f, 0.25f, 0.125f, 0.0625f, 0.03125f };
    sequencer_.setDivision (divisionValues[seqDivIdx < 6 ? seqDivIdx : 2]);

    envFollower_.setThresholdDb (envThresholdParam_->load());

    // Effects per-block setup
    switch (fxSelect)
    {
        case 1: // Freq Shift
            freqShifterL_.setShiftHz (fxFreq);
            freqShifterR_.setShiftHz (fxFreq);
            break;
        case 2: // Ring Mod
            ringModL_.setFrequency (fxFreq);
            ringModR_.setFrequency (fxFreq);
            ringModL_.setDepth (fxDepth);
            ringModR_.setDepth (fxDepth);
            break;
        case 3: // Phaser
            phaseShifter_.setRate (fxFreq);
            phaseShifter_.setDepth (fxDepth);
            phaseShifter_.setStereoMode (static_cast<PhaseShifter::StereoMode> (fxStereoMode));
            break;
        default:
            break;
    }

    if (reverbEnabled)
    {
        reverb_.setDecayTime (reverbDecay);
        reverb_.setRoomSize (reverbSize);
    }

    if (distEnabled)
    {
        distortionL_.setDrive (distDrive);
        distortionR_.setDrive (distDrive);
        distortionL_.setBassBoostDb (distBassBoost);
        distortionR_.setBassBoostDb (distBassBoost);
    }

    if (adsrEnabled)
    {
        adsr_.setAttackMs (adsrAttackParam_->load());
        adsr_.setDecayMs (adsrDecayParam_->load());
        adsr_.setSustain (adsrSustainParam_->load() / 100.0f);
        adsr_.setReleaseMs (adsrReleaseParam_->load());
        adsr_.setInverted (adsrInvertParam_->load() > 0.5f);
    }

    loFiL_.setBitDepth (bitDepth);
    loFiR_.setBitDepth (bitDepth);
    loFiL_.setSampleRateDivider (srDiv);
    loFiR_.setSampleRateDivider (srDiv);
    loFiL_.setLPFCutoff (lpfCutoff);
    loFiR_.setLPFCutoff (lpfCutoff);
    antiAliasL_.setCutoff (lpfCutoff);
    antiAliasR_.setCutoff (lpfCutoff);

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

    for (int i = 0; i < numSamples; ++i)
    {
        float dryL = dataL[i];
        float dryR = dataR[i];

        // Get smoothed parameter values
        float curPitchLSt = pitchLSmoothed_.getNextValue();
        float curPitchRSt = pitchRSmoothed_.getNextValue();
        grainSmoothed_.getNextValue();    // advance smoother (block value used above)
        stretchSmoothed_.getNextValue();  // advance smoother (block value used above)
        float curDelay = delaySmoothed_.getNextValue();
        float curFeedback = feedbackSmoothed_.getNextValue();
        float curMix = mixSmoothed_.getNextValue();

        // Sequencer: get pitch offset
        float seqOffset = sequencer_.processSample (bpm, ppqPosition + static_cast<double> (i) / (bpm / 60.0 * getSampleRate()), isPlaying);

        // Envelope follower trigger
        if (envFollow && seqEnabled)
        {
            float monoInput = (dryL + dryR) * 0.5f;
            if (envFollower_.detectTransient (monoInput))
                sequencer_.triggerNextStep();
        }

        // Apply pitch with sequencer offset (std::pow per sample is unavoidable
        // since pitch changes every sample via SmoothedValue + sequencer)
        float totalPitchLSt = curPitchLSt + seqOffset;
        float totalPitchRSt = curPitchRSt + seqOffset;

        pitchShifterL_.setPitchSemitones (totalPitchLSt);
        pitchShifterR_.setPitchSemitones (totalPitchRSt);

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
        apvts_.replaceState (juce::ValueTree::fromXml (*xml));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}
