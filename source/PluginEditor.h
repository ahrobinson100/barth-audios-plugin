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

    // Custom LookAndFeel with Valhalla-style arc knobs
    struct DarkLookAndFeel : public juce::LookAndFeel_V4
    {
        DarkLookAndFeel();
        void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                               float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                               juce::Slider& slider) override;
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

    // === Transpose Section ===
    KnobWithLabel progKnobs_[4][2];  // [program 0-3][0=voiceA, 1=voiceB]
    juce::TextButton progButton1_ { "1" }, progButton2_ { "2" },
                     progButton3_ { "3" }, progButton4_ { "4" };
    juce::TextButton* progButtons_[4] = { &progButton1_, &progButton2_, &progButton3_, &progButton4_ };
    juce::ComboBox activeProgramBox_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> activeProgramAttach_;
    int lastActiveProgram_ = -1;

    KnobWithLabel grainKnob_, portamentoKnob_;
    juce::ToggleButton vintageCharButton_ { "Vintage" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> vintageCharAttach_;

    // === Delay Section ===
    KnobWithLabel stretchKnob_, delayKnob_, feedbackKnob_, mixKnob_;

    // === Sequencer Section ===
    KnobWithLabel seqRateKnob_;
    juce::ToggleButton seqEnabledButton_ { "Seq" };
    juce::ToggleButton seqSyncButton_ { "Sync" };
    juce::ToggleButton envFollowButton_ { "Env" };
    juce::ComboBox seqModeBox_, seqDivisionBox_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> seqEnabledAttach_, seqSyncAttach_, envFollowAttach_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> seqModeAttach_, seqDivisionAttach_;

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
    juce::Label transposeSectionLabel_, delaySectionLabel_, seqSectionLabel_, fxSectionLabel_;
    juce::Label adsrSectionLabel_, lofiSectionLabel_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
