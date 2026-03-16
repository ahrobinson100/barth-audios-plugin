#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "DSP/Core/PitchShifter.h"
#include "DSP/Core/DelayLine.h"
#include "DSP/Core/TimeStretcher.h"
#include "DSP/Core/StepSequencer.h"
#include "DSP/Effects/FrequencyShifter.h"
#include "DSP/Effects/RingModulator.h"
#include "DSP/Effects/PhaseShifter.h"
#include "DSP/Effects/StereoReverb.h"
#include "DSP/Effects/Distortion.h"
#include "DSP/Effects/ADSREnvelope.h"
#include "DSP/Utilities/DCBlocker.h"
#include "DSP/Utilities/SoftClipper.h"
#include "DSP/Utilities/OnePoleFilter.h"
#include "DSP/Utilities/EnvelopeFollower.h"
#include "DSP/Utilities/LoFiProcessor.h"
#include "DSP/SignalRouter.h"

class PluginProcessor : public juce::AudioProcessor
{
public:
    PluginProcessor();
    ~PluginProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts_; }

    // For GUI: current sequencer step
    int getCurrentSeqStep() const { return sequencer_.getCurrentStep(); }

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::AudioProcessorValueTreeState apvts_;

    // DSP per-channel
    PitchShifter pitchShifterL_, pitchShifterR_;
    TimeStretcher timeStretcherL_, timeStretcherR_;
    DelayLine delayLineL_, delayLineR_;

    // Effects (shared or stereo)
    FrequencyShifter freqShifterL_, freqShifterR_;
    RingModulator ringModL_, ringModR_;
    PhaseShifter phaseShifter_;
    StereoReverb reverb_;
    Distortion distortionL_, distortionR_;
    ADSREnvelope adsr_;

    // Utilities
    OnePoleFilter antiAliasL_, antiAliasR_;
    LoFiProcessor loFiL_, loFiR_;
    SoftClipper outputClipperL_, outputClipperR_;
    EnvelopeFollower envFollower_;
    SignalRouter router_;

    // Sequencer
    StepSequencer sequencer_;

    // Smoothed parameters
    juce::SmoothedValue<float> pitchLSmoothed_, pitchRSmoothed_;
    juce::SmoothedValue<float> grainSmoothed_;
    juce::SmoothedValue<float> stretchSmoothed_;
    juce::SmoothedValue<float> delaySmoothed_;
    juce::SmoothedValue<float> feedbackSmoothed_;
    juce::SmoothedValue<float> mixSmoothed_;

    // Effects parameter smoothing (per-block)
    juce::SmoothedValue<float> fxFreqSmoothed_, fxDepthSmoothed_;
    juce::SmoothedValue<float> distDriveSmoothed_, distBassBoostSmoothed_;
    juce::SmoothedValue<float> lpfCutoffSmoothed_;
    juce::SmoothedValue<float> reverbDecaySmoothed_, reverbSizeSmoothed_;

    // Latency tracking
    int lastReportedLatency_ = 0;

    // Atomic parameter pointers (set in constructor)
    std::atomic<float>* pitchLParam_ = nullptr;
    std::atomic<float>* pitchRParam_ = nullptr;
    std::atomic<float>* pitchLinkParam_ = nullptr;
    std::atomic<float>* grainParam_ = nullptr;
    std::atomic<float>* portamentoParam_ = nullptr;
    std::atomic<float>* stretchParam_ = nullptr;
    std::atomic<float>* delayParam_ = nullptr;
    std::atomic<float>* feedbackParam_ = nullptr;
    std::atomic<float>* mixParam_ = nullptr;

    std::atomic<float>* seqEnabledParam_ = nullptr;
    std::atomic<float>* seqStep1Param_ = nullptr;
    std::atomic<float>* seqStep2Param_ = nullptr;
    std::atomic<float>* seqStep3Param_ = nullptr;
    std::atomic<float>* seqStep4Param_ = nullptr;
    std::atomic<float>* seqModeParam_ = nullptr;
    std::atomic<float>* seqRateParam_ = nullptr;
    std::atomic<float>* seqSyncParam_ = nullptr;
    std::atomic<float>* seqDivisionParam_ = nullptr;
    std::atomic<float>* envFollowParam_ = nullptr;
    std::atomic<float>* envThresholdParam_ = nullptr;

    std::atomic<float>* fxSelectParam_ = nullptr;
    std::atomic<float>* fxFreqParam_ = nullptr;
    std::atomic<float>* fxDepthParam_ = nullptr;
    std::atomic<float>* fxStereoParam_ = nullptr;

    std::atomic<float>* reverbEnabledParam_ = nullptr;
    std::atomic<float>* reverbDecayParam_ = nullptr;
    std::atomic<float>* reverbSizeParam_ = nullptr;

    std::atomic<float>* distEnabledParam_ = nullptr;
    std::atomic<float>* distDriveParam_ = nullptr;
    std::atomic<float>* distBassBoostParam_ = nullptr;

    std::atomic<float>* adsrEnabledParam_ = nullptr;
    std::atomic<float>* adsrAttackParam_ = nullptr;
    std::atomic<float>* adsrDecayParam_ = nullptr;
    std::atomic<float>* adsrSustainParam_ = nullptr;
    std::atomic<float>* adsrReleaseParam_ = nullptr;
    std::atomic<float>* adsrTriggerParam_ = nullptr;
    std::atomic<float>* adsrRoutingParam_ = nullptr;
    std::atomic<float>* adsrInvertParam_ = nullptr;

    std::atomic<float>* bitDepthParam_ = nullptr;
    std::atomic<float>* srDivParam_ = nullptr;
    std::atomic<float>* lpfCutoffParam_ = nullptr;
    std::atomic<float>* monoModeParam_ = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};
