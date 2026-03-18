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
    setColour (juce::TextButton::buttonColourId, juce::Colour (0xff222244));
    setColour (juce::TextButton::textColourOffId, juce::Colour (0xffccccdd));
}

void PluginEditor::DarkLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                                                       float sliderPos, float startAngle, float endAngle,
                                                       juce::Slider& /*slider*/)
{
    float radius = static_cast<float> (juce::jmin (w, h)) * 0.4f;
    float cx = static_cast<float> (x) + static_cast<float> (w) * 0.5f;
    float cy = static_cast<float> (y) + static_cast<float> (h) * 0.5f;
    float angle = startAngle + sliderPos * (endAngle - startAngle);

    // Background circle
    g.setColour (juce::Colour (0xff222244));
    g.fillEllipse (cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);

    // Value arc
    float arcRadius = radius * 0.85f;
    juce::Path arc;
    arc.addCentredArc (cx, cy, arcRadius, arcRadius, 0.0f, startAngle, angle, true);
    g.setColour (juce::Colour (0xffe94560));
    g.strokePath (arc, juce::PathStrokeType (2.5f));

    // Dot indicator at current position
    float dotR = 3.0f;
    float dotX = cx + arcRadius * std::cos (angle - juce::MathConstants<float>::halfPi);
    float dotY = cy + arcRadius * std::sin (angle - juce::MathConstants<float>::halfPi);
    g.fillEllipse (dotX - dotR, dotY - dotR, dotR * 2.0f, dotR * 2.0f);
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
    initSectionLabel (transposeSectionLabel_, "TRANSPOSE");
    initSectionLabel (delaySectionLabel_, "DELAY");
    initSectionLabel (seqSectionLabel_, "SEQUENCER");
    initSectionLabel (fxSectionLabel_, "EFFECTS");
    initSectionLabel (adsrSectionLabel_, "ENVELOPE");
    initSectionLabel (lofiSectionLabel_, "LO-FI");

    // === Transpose: 8 program knobs ===
    const char* pitchIds[4][2] = {
        {"prog1PitchA", "prog1PitchB"}, {"prog2PitchA", "prog2PitchB"},
        {"prog3PitchA", "prog3PitchB"}, {"prog4PitchA", "prog4PitchB"}
    };
    const char* pitchLabels[4][2] = {
        {"1A", "1B"}, {"2A", "2B"}, {"3A", "3B"}, {"4A", "4B"}
    };
    for (int prog = 0; prog < 4; ++prog)
        for (int v = 0; v < 2; ++v)
            progKnobs_[prog][v].init (pitchIds[prog][v], pitchLabels[prog][v], apvts, this);

    // Program buttons
    for (int i = 0; i < 4; ++i)
    {
        addAndMakeVisible (progButtons_[i]);
        progButtons_[i].onClick = [this, i] { activeProgramBox_.setSelectedItemIndex (i); };
    }

    // Hidden ComboBox for APVTS attachment
    activeProgramBox_.addItemList ({"1", "2", "3", "4"}, 1);
    activeProgramBox_.setVisible (false);
    addChildComponent (activeProgramBox_);
    activeProgramAttach_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, "activeProgram", activeProgramBox_);

    grainKnob_.init ("grain", "Grain", apvts, this);
    portamentoKnob_.init ("portamento", "Porta", apvts, this);

    addAndMakeVisible (vintageCharButton_);
    vintageCharAttach_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, "vintageChar", vintageCharButton_);

    // === Delay ===
    stretchKnob_.init ("stretch", "Stretch", apvts, this);
    delayKnob_.init ("delay", "Delay", apvts, this);
    feedbackKnob_.init ("feedback", "Fdbk", apvts, this);
    mixKnob_.init ("mix", "Mix", apvts, this);

    // === Sequencer ===
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

    setSize (620, 680);
    startTimerHz (15);
}

PluginEditor::~PluginEditor()
{
    setLookAndFeel (nullptr);
}

void PluginEditor::timerCallback()
{
    int activeProg = processorRef.getActiveProgram();
    if (activeProg != lastActiveProgram_)
    {
        lastActiveProgram_ = activeProg;
        for (int i = 0; i < 4; ++i)
            progButtons_[i].setColour (juce::TextButton::buttonColourId,
                i == activeProg ? juce::Colour (0xffe94560) : juce::Colour (0xff222244));
        repaint();
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
    drawDivider (210);
    drawDivider (300);
    drawDivider (370);
    drawDivider (480);
    drawDivider (560);
    drawDivider (630);
}

void PluginEditor::resized()
{
    const int knobW = 62;
    const int knobH = 62;
    const int startX = 10;

    auto placeKnob = [] (KnobWithLabel& k, int x, int y, int w = 62, int h = 62) {
        k.slider.setBounds (x, y, w, h);
        k.label.setBounds (x, y + h, w, 14);
    };

    // === Section Labels ===
    transposeSectionLabel_.setBounds (startX, 42, 90, 16);
    delaySectionLabel_.setBounds (startX, 214, 60, 16);
    seqSectionLabel_.setBounds (startX, 304, 90, 16);
    fxSectionLabel_.setBounds (startX, 374, 70, 16);
    adsrSectionLabel_.setBounds (startX, 484, 80, 16);
    lofiSectionLabel_.setBounds (startX, 564, 60, 16);

    // === Transpose Section (38 to ~210) ===
    // 4 program columns, each with Voice A knob (top) + Voice B knob (bottom) + button
    int transY = 60;
    int progColW = 72;  // width per program column
    int progKnobSize = 56;

    for (int prog = 0; prog < 4; ++prog)
    {
        int colX = startX + prog * progColW;
        placeKnob (progKnobs_[prog][0], colX, transY, progKnobSize, progKnobSize);        // Voice A
        placeKnob (progKnobs_[prog][1], colX, transY + progKnobSize + 16, progKnobSize, progKnobSize);  // Voice B
        progButtons_[prog].setBounds (colX + 10, transY + (progKnobSize + 16) * 2 - 2, 36, 20);
    }

    // Grain, Porta, Vintage to the right of program columns
    int rightColX = startX + 4 * progColW + 10;
    placeKnob (grainKnob_, rightColX, transY, progKnobSize, progKnobSize);
    placeKnob (portamentoKnob_, rightColX, transY + progKnobSize + 16, progKnobSize, progKnobSize);
    vintageCharButton_.setBounds (rightColX, transY + (progKnobSize + 16) * 2 - 2, 64, 20);

    // === Delay Section (210 to ~300) ===
    int delayY = 232;
    placeKnob (stretchKnob_, startX, delayY);
    placeKnob (delayKnob_, startX + knobW, delayY);
    placeKnob (feedbackKnob_, startX + knobW * 2, delayY);
    placeKnob (mixKnob_, startX + knobW * 3, delayY);

    // === Sequencer Section (300 to ~370) ===
    int seqY = 322;
    placeKnob (seqRateKnob_, startX, seqY, 56, 56);

    int seqBtnX = startX + 62;
    seqEnabledButton_.setBounds (seqBtnX, seqY + 4, 50, 20);
    seqSyncButton_.setBounds (seqBtnX + 50, seqY + 4, 50, 20);
    envFollowButton_.setBounds (seqBtnX + 100, seqY + 4, 50, 20);
    seqModeBox_.setBounds (seqBtnX, seqY + 28, 80, 20);
    seqDivisionBox_.setBounds (seqBtnX + 84, seqY + 28, 70, 20);

    // === Effects Section (370 to ~480) ===
    int fxY = 392;
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

    // === ADSR Section (480 to ~560) ===
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

    // === Lo-Fi Section (560 to ~630) ===
    int lofiY = 578;
    placeKnob (bitDepthKnob_, startX, lofiY, 56, 56);
    placeKnob (srDivKnob_, startX + 60, lofiY, 56, 56);
    placeKnob (lpfCutoffKnob_, startX + 120, lofiY, 56, 56);
    monoModeBox_.setBounds (startX + 186, lofiY + 16, 80, 22);
}
