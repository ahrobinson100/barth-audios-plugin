#pragma once

#include "PluginProcessor.h"

class PluginEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit PluginEditor (PluginProcessor&);
    ~PluginEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    PluginProcessor& processorRef;

    // Custom LookAndFeel for dark theme
    struct DarkLookAndFeel : public juce::LookAndFeel_V4
    {
        DarkLookAndFeel();
    };
    DarkLookAndFeel darkLnf_;

    // Helper to create a rotary slider + label pair
    struct KnobWithLabel
    {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;

        void init (const juce::String& paramId, const juce::String& labelText,
                   juce::AudioProcessorValueTreeState& apvts, juce::Component* parent);
    };

    // === Core Section ===
    KnobWithLabel pitchLKnob_, pitchRKnob_, grainKnob_, portamentoKnob_;
    KnobWithLabel stretchKnob_, delayKnob_, feedbackKnob_, mixKnob_;
    juce::ToggleButton pitchLinkButton_ { "Link" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> pitchLinkAttach_;

    // === Sequencer Section ===
    KnobWithLabel seqStep1Knob_, seqStep2Knob_, seqStep3Knob_, seqStep4Knob_;
    KnobWithLabel seqRateKnob_;
    juce::ToggleButton seqEnabledButton_ { "Seq" };
    juce::ToggleButton seqSyncButton_ { "Sync" };
    juce::ToggleButton envFollowButton_ { "Env" };
    juce::ComboBox seqModeBox_, seqDivisionBox_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> seqEnabledAttach_, seqSyncAttach_, envFollowAttach_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> seqModeAttach_, seqDivisionAttach_;

    // Step LED indicators
    int lastSeqStep_ = -1;

    // === Effects Section ===
    KnobWithLabel fxFreqKnob_, fxDepthKnob_;
    juce::ComboBox fxSelectBox_, fxStereoBox_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> fxSelectAttach_, fxStereoAttach_;

    // Reverb
    juce::ToggleButton reverbEnabledButton_ { "Reverb" };
    KnobWithLabel reverbDecayKnob_, reverbSizeKnob_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> reverbEnabledAttach_;

    // Distortion
    juce::ToggleButton distEnabledButton_ { "Dist" };
    KnobWithLabel distDriveKnob_, distBassBoostKnob_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> distEnabledAttach_;

    // === ADSR Section ===
    juce::ToggleButton adsrEnabledButton_ { "ADSR" };
    KnobWithLabel adsrAttackKnob_, adsrDecayKnob_, adsrSustainKnob_, adsrReleaseKnob_;
    juce::ComboBox adsrTriggerBox_, adsrRoutingBox_;
    juce::ToggleButton adsrInvertButton_ { "Inv" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> adsrEnabledAttach_, adsrInvertAttach_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> adsrTriggerAttach_, adsrRoutingAttach_;

    // === Lo-Fi Section ===
    KnobWithLabel bitDepthKnob_, srDivKnob_, lpfCutoffKnob_;
    juce::ComboBox monoModeBox_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> monoModeAttach_;

    // Section labels
    juce::Label coreSectionLabel_, seqSectionLabel_, fxSectionLabel_;
    juce::Label adsrSectionLabel_, lofiSectionLabel_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
