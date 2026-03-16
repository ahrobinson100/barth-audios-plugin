#include "PluginEditor.h"

//==============================================================================
// DarkLookAndFeel
//==============================================================================
PluginEditor::DarkLookAndFeel::DarkLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, juce::Colour (0xff1a1a2e));
    setColour (juce::Slider::thumbColourId, juce::Colour (0xffe94560));
    setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xffe94560));
    setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff333366));
    setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colour (0xff333366));
    setColour (juce::Label::textColourId, juce::Colour (0xffccccdd));
    setColour (juce::ToggleButton::textColourId, juce::Colour (0xffccccdd));
    setColour (juce::ToggleButton::tickColourId, juce::Colour (0xffe94560));
    setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff222244));
    setColour (juce::ComboBox::textColourId, juce::Colours::white);
    setColour (juce::ComboBox::outlineColourId, juce::Colour (0xff333366));
    setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff1a1a2e));
    setColour (juce::PopupMenu::textColourId, juce::Colours::white);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xffe94560));
}

//==============================================================================
// KnobWithLabel
//==============================================================================
void PluginEditor::KnobWithLabel::init (const juce::String& paramId, const juce::String& labelText,
                                         juce::AudioProcessorValueTreeState& apvts, juce::Component* parent)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 56, 14);
    parent->addAndMakeVisible (slider);

    label.setText (labelText, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setFont (juce::Font (11.0f));
    parent->addAndMakeVisible (label);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, paramId, slider);
}

//==============================================================================
// PluginEditor
//==============================================================================
PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    setLookAndFeel (&darkLnf_);

    auto& apvts = processorRef.getAPVTS();

    // Section labels
    auto initSectionLabel = [this] (juce::Label& lbl, const juce::String& text) {
        lbl.setText (text, juce::dontSendNotification);
        lbl.setFont (juce::Font (13.0f, juce::Font::bold));
        lbl.setColour (juce::Label::textColourId, juce::Colour (0xffe94560));
        addAndMakeVisible (lbl);
    };
    initSectionLabel (coreSectionLabel_, "CORE");
    initSectionLabel (seqSectionLabel_, "SEQUENCER");
    initSectionLabel (fxSectionLabel_, "EFFECTS");
    initSectionLabel (adsrSectionLabel_, "ADSR");
    initSectionLabel (lofiSectionLabel_, "LO-FI");

    // === Core ===
    pitchLKnob_.init ("pitchL", "Pitch L", apvts, this);
    pitchRKnob_.init ("pitchR", "Pitch R", apvts, this);
    grainKnob_.init ("grain", "Grain", apvts, this);
    portamentoKnob_.init ("portamento", "Porta", apvts, this);
    stretchKnob_.init ("stretch", "Stretch", apvts, this);
    delayKnob_.init ("delay", "Delay", apvts, this);
    feedbackKnob_.init ("feedback", "Fdbk", apvts, this);
    mixKnob_.init ("mix", "Mix", apvts, this);

    addAndMakeVisible (pitchLinkButton_);
    pitchLinkAttach_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, "pitchLink", pitchLinkButton_);

    // === Sequencer ===
    seqStep1Knob_.init ("seqStep1", "S1", apvts, this);
    seqStep2Knob_.init ("seqStep2", "S2", apvts, this);
    seqStep3Knob_.init ("seqStep3", "S3", apvts, this);
    seqStep4Knob_.init ("seqStep4", "S4", apvts, this);
    seqRateKnob_.init ("seqRate", "Rate", apvts, this);

    addAndMakeVisible (seqEnabledButton_);
    addAndMakeVisible (seqSyncButton_);
    addAndMakeVisible (envFollowButton_);
    seqEnabledAttach_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, "seqEnabled", seqEnabledButton_);
    seqSyncAttach_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, "seqSync", seqSyncButton_);
    envFollowAttach_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, "envFollow", envFollowButton_);

    seqModeBox_.addItemList ({ "Fwd", "Bwd", "Rnd", "P-P", "2-St", "3-St", "4-St" }, 1);
    addAndMakeVisible (seqModeBox_);
    seqModeAttach_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, "seqMode", seqModeBox_);

    seqDivisionBox_.addItemList ({ "1/1", "1/2", "1/4", "1/8", "1/16", "1/32" }, 1);
    addAndMakeVisible (seqDivisionBox_);
    seqDivisionAttach_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, "seqDivision", seqDivisionBox_);

    // === Effects ===
    fxFreqKnob_.init ("fxFreq", "Freq", apvts, this);
    fxDepthKnob_.init ("fxDepth", "Depth", apvts, this);

    fxSelectBox_.addItemList ({ "Off", "FShift", "RMod", "Phase" }, 1);
    addAndMakeVisible (fxSelectBox_);
    fxSelectAttach_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, "fxSelect", fxSelectBox_);

    fxStereoBox_.addItemList ({ "Mono", "Stereo", "Tandem" }, 1);
    addAndMakeVisible (fxStereoBox_);
    fxStereoAttach_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, "fxStereo", fxStereoBox_);

    // Reverb
    addAndMakeVisible (reverbEnabledButton_);
    reverbEnabledAttach_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, "reverbEnabled", reverbEnabledButton_);
    reverbDecayKnob_.init ("reverbDecay", "Decay", apvts, this);
    reverbSizeKnob_.init ("reverbSize", "Size", apvts, this);

    // Distortion
    addAndMakeVisible (distEnabledButton_);
    distEnabledAttach_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, "distEnabled", distEnabledButton_);
    distDriveKnob_.init ("distDrive", "Drive", apvts, this);
    distBassBoostKnob_.init ("distBassBoost", "Bass", apvts, this);

    // === ADSR ===
    addAndMakeVisible (adsrEnabledButton_);
    adsrEnabledAttach_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, "adsrEnabled", adsrEnabledButton_);
    adsrAttackKnob_.init ("adsrAttack", "Atk", apvts, this);
    adsrDecayKnob_.init ("adsrDecay", "Dec", apvts, this);
    adsrSustainKnob_.init ("adsrSustain", "Sus", apvts, this);
    adsrReleaseKnob_.init ("adsrRelease", "Rel", apvts, this);

    adsrTriggerBox_.addItemList ({ "Trans", "Key", "Seq" }, 1);
    addAndMakeVisible (adsrTriggerBox_);
    adsrTriggerAttach_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, "adsrTrigger", adsrTriggerBox_);

    adsrRoutingBox_.addItemList ({ "Pre-Rev", "Post-Rev" }, 1);
    addAndMakeVisible (adsrRoutingBox_);
    adsrRoutingAttach_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, "adsrRouting", adsrRoutingBox_);

    addAndMakeVisible (adsrInvertButton_);
    adsrInvertAttach_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, "adsrInvert", adsrInvertButton_);

    // === Lo-Fi ===
    bitDepthKnob_.init ("bitDepth", "Bits", apvts, this);
    srDivKnob_.init ("srDiv", "SRDiv", apvts, this);
    lpfCutoffKnob_.init ("lpfCutoff", "LPF", apvts, this);

    monoModeBox_.addItemList ({ "Stereo", "Mono", "Diff" }, 1);
    addAndMakeVisible (monoModeBox_);
    monoModeAttach_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, "monoMode", monoModeBox_);

    setSize (620, 720);
    startTimerHz (15); // Update step LED at 15fps
}

PluginEditor::~PluginEditor()
{
    setLookAndFeel (nullptr);
}

void PluginEditor::timerCallback()
{
    int step = processorRef.getCurrentSeqStep();
    if (step != lastSeqStep_)
    {
        lastSeqStep_ = step;
        repaint(); // Trigger LED repaint
    }
}

void PluginEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff1a1a2e));

    // Title
    g.setColour (juce::Colours::white);
    g.setFont (juce::Font (16.0f, juce::Font::bold));
    g.drawText ("BARTH AUDIOS", getLocalBounds().removeFromTop (28), juce::Justification::centred);
    g.setFont (juce::Font (10.0f));
    g.setColour (juce::Colour (0xff888899));
    g.drawText ("Pitch Transposer & Delay", 0, 22, getWidth(), 14, juce::Justification::centred);

    // Section dividers
    auto drawDivider = [&] (int y) {
        g.setColour (juce::Colour (0xff333366));
        g.drawHorizontalLine (y, 10.0f, static_cast<float> (getWidth() - 10));
    };
    drawDivider (38);
    drawDivider (200);
    drawDivider (340);
    drawDivider (480);
    drawDivider (590);
    drawDivider (660);

    // Sequencer step LEDs
    int seqY = 210;
    for (int s = 0; s < 4; ++s)
    {
        int ledX = 22 + s * 62;
        g.setColour (s == lastSeqStep_ ? juce::Colour (0xffe94560) : juce::Colour (0xff333344));
        g.fillEllipse (static_cast<float> (ledX), static_cast<float> (seqY), 8.0f, 8.0f);
    }
}

void PluginEditor::resized()
{
    const int knobW = 62;
    const int knobH = 62;
    const int labelH = 14;
    const int row1Y = 42;
    const int startX = 10;

    auto placeKnob = [] (KnobWithLabel& k, int x, int y, int w = 62, int h = 62) {
        k.slider.setBounds (x, y, w, h);
        k.label.setBounds (x, y + h, w, 14);
    };

    // === Section Labels ===
    coreSectionLabel_.setBounds (startX, row1Y, 60, 16);
    seqSectionLabel_.setBounds (startX, 204, 90, 16);
    fxSectionLabel_.setBounds (startX, 344, 70, 16);
    adsrSectionLabel_.setBounds (startX, 484, 60, 16);
    lofiSectionLabel_.setBounds (startX, 594, 60, 16);

    // === Core Section (row1Y + 18 to ~200) ===
    int coreY = row1Y + 18;
    placeKnob (pitchLKnob_, startX, coreY);
    placeKnob (pitchRKnob_, startX + knobW, coreY);
    pitchLinkButton_.setBounds (startX + knobW * 2, coreY + 20, 50, 20);
    placeKnob (grainKnob_, startX + knobW * 2 + 52, coreY);
    placeKnob (portamentoKnob_, startX + knobW * 3 + 52, coreY);

    int coreRow2Y = coreY + knobH + labelH + 4;
    placeKnob (stretchKnob_, startX, coreRow2Y);
    placeKnob (delayKnob_, startX + knobW, coreRow2Y);
    placeKnob (feedbackKnob_, startX + knobW * 2, coreRow2Y);
    placeKnob (mixKnob_, startX + knobW * 3, coreRow2Y);

    // === Sequencer Section (204 to ~340) ===
    int seqY = 222;
    placeKnob (seqStep1Knob_, startX + 14, seqY, 56, 56);
    placeKnob (seqStep2Knob_, startX + 14 + 62, seqY, 56, 56);
    placeKnob (seqStep3Knob_, startX + 14 + 124, seqY, 56, 56);
    placeKnob (seqStep4Knob_, startX + 14 + 186, seqY, 56, 56);
    placeKnob (seqRateKnob_, startX + 14 + 260, seqY, 56, 56);

    int seqBtnY = seqY + 76;
    seqEnabledButton_.setBounds (startX, seqBtnY, 50, 20);
    seqSyncButton_.setBounds (startX + 52, seqBtnY, 50, 20);
    envFollowButton_.setBounds (startX + 104, seqBtnY, 50, 20);
    seqModeBox_.setBounds (startX + 160, seqBtnY, 80, 20);
    seqDivisionBox_.setBounds (startX + 244, seqBtnY, 70, 20);

    // === Effects Section (344 to ~480) ===
    int fxY = 362;
    fxSelectBox_.setBounds (startX, fxY, 90, 22);
    placeKnob (fxFreqKnob_, startX + 96, fxY - 4);
    placeKnob (fxDepthKnob_, startX + 96 + knobW, fxY - 4);
    fxStereoBox_.setBounds (startX + 96 + knobW * 2 + 4, fxY, 80, 22);

    int fxRow2Y = fxY + knobH + 14;
    reverbEnabledButton_.setBounds (startX, fxRow2Y, 70, 20);
    placeKnob (reverbDecayKnob_, startX + 72, fxRow2Y - 4, 56, 56);
    placeKnob (reverbSizeKnob_, startX + 72 + 60, fxRow2Y - 4, 56, 56);

    distEnabledButton_.setBounds (startX + 200, fxRow2Y, 56, 20);
    placeKnob (distDriveKnob_, startX + 258, fxRow2Y - 4, 56, 56);
    placeKnob (distBassBoostKnob_, startX + 258 + 60, fxRow2Y - 4, 56, 56);

    // === ADSR Section (484 to ~590) ===
    int adsrY = 502;
    adsrEnabledButton_.setBounds (startX, adsrY, 56, 20);
    placeKnob (adsrAttackKnob_, startX + 58, adsrY - 4, 56, 56);
    placeKnob (adsrDecayKnob_, startX + 58 + 60, adsrY - 4, 56, 56);
    placeKnob (adsrSustainKnob_, startX + 58 + 120, adsrY - 4, 56, 56);
    placeKnob (adsrReleaseKnob_, startX + 58 + 180, adsrY - 4, 56, 56);

    int adsrBtnY = adsrY + 60;
    adsrTriggerBox_.setBounds (startX, adsrBtnY, 70, 20);
    adsrRoutingBox_.setBounds (startX + 74, adsrBtnY, 80, 20);
    adsrInvertButton_.setBounds (startX + 158, adsrBtnY, 50, 20);

    // === Lo-Fi Section (594 to ~660) ===
    int lofiY = 610;
    placeKnob (bitDepthKnob_, startX, lofiY, 56, 56);
    placeKnob (srDivKnob_, startX + 60, lofiY, 56, 56);
    placeKnob (lpfCutoffKnob_, startX + 120, lofiY, 56, 56);
    monoModeBox_.setBounds (startX + 186, lofiY + 16, 80, 22);
}
