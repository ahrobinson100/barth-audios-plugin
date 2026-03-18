#include "helpers/test_helpers.h"
#include <PluginProcessor.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

TEST_CASE ("Full chain: processes without crashing", "[integration]")
{
    PluginProcessor plugin;
    plugin.setPlayConfigDetails (2, 2, 48000.0, 512);
    plugin.prepareToPlay (48000.0, 512);

    auto buffer = TestHelpers::makeStereoSine (440.0f, 48000.0, 48000);
    juce::MidiBuffer midi;

    // Process in blocks
    for (int pos = 0; pos + 512 <= 48000; pos += 512)
    {
        juce::AudioBuffer<float> block (buffer.getArrayOfWritePointers(), 2, pos, 512);
        plugin.processBlock (block, midi);
    }

    // Check no NaN/Inf in output
    for (int ch = 0; ch < 2; ++ch)
    {
        const auto* data = buffer.getReadPointer (ch);
        for (int i = 0; i < 48000; ++i)
        {
            REQUIRE_FALSE (std::isnan (data[i]));
            REQUIRE_FALSE (std::isinf (data[i]));
        }
    }
}

TEST_CASE ("Full chain: state save/restore round-trip", "[integration]")
{
    PluginProcessor plugin;
    plugin.setPlayConfigDetails (2, 2, 48000.0, 512);
    plugin.prepareToPlay (48000.0, 512);

    // Set some parameters
    auto& apvts = plugin.getAPVTS();
    if (auto* p = apvts.getParameter ("prog1PitchA"))
        p->setValueNotifyingHost (p->getNormalisableRange().convertTo0to1 (7.0f));
    if (auto* p = apvts.getParameter ("delay"))
        p->setValueNotifyingHost (p->getNormalisableRange().convertTo0to1 (500.0f));
    if (auto* p = apvts.getParameter ("feedback"))
        p->setValueNotifyingHost (p->getNormalisableRange().convertTo0to1 (50.0f));

    // Save state
    juce::MemoryBlock stateData;
    plugin.getStateInformation (stateData);

    // Create new plugin and restore
    PluginProcessor plugin2;
    plugin2.setPlayConfigDetails (2, 2, 48000.0, 512);
    plugin2.prepareToPlay (48000.0, 512);
    plugin2.setStateInformation (stateData.getData(), static_cast<int> (stateData.getSize()));

    // Verify parameters match
    auto& apvts2 = plugin2.getAPVTS();
    auto* pitchA1 = apvts.getRawParameterValue ("prog1PitchA");
    auto* pitchA2 = apvts2.getRawParameterValue ("prog1PitchA");
    REQUIRE (pitchA1->load() == Catch::Approx (pitchA2->load()).margin (0.1f));

    auto* delay1 = apvts.getRawParameterValue ("delay");
    auto* delay2 = apvts2.getRawParameterValue ("delay");
    REQUIRE (delay1->load() == Catch::Approx (delay2->load()).margin (1.0f));
}

TEST_CASE ("Full chain: rapid parameter automation", "[integration][stress]")
{
    PluginProcessor plugin;
    plugin.setPlayConfigDetails (2, 2, 48000.0, 128);
    plugin.prepareToPlay (48000.0, 128);

    auto& apvts = plugin.getAPVTS();
    juce::MidiBuffer midi;

    // Process 10 seconds of audio with parameter changes every block
    for (int block = 0; block < 3750; ++block) // 48000*10 / 128
    {
        // Change pitch every block
        float pitch = -24.0f + static_cast<float> (block % 48) * 1.0f;
        if (auto* p = apvts.getParameter ("prog1PitchA"))
            p->setValueNotifyingHost (p->getNormalisableRange().convertTo0to1 (pitch));

        auto buffer = TestHelpers::makeStereoSine (440.0f, 48000.0, 128);
        plugin.processBlock (buffer, midi);

        // Check for NaN/Inf
        for (int ch = 0; ch < 2; ++ch)
        {
            const auto* data = buffer.getReadPointer (ch);
            for (int i = 0; i < 128; ++i)
            {
                REQUIRE_FALSE (std::isnan (data[i]));
                REQUIRE_FALSE (std::isinf (data[i]));
            }
        }
    }
}

TEST_CASE ("Full chain: extreme settings stress test", "[integration][stress]")
{
    PluginProcessor plugin;
    plugin.setPlayConfigDetails (2, 2, 48000.0, 512);
    plugin.prepareToPlay (48000.0, 512);

    auto& apvts = plugin.getAPVTS();

    // Set extreme parameters
    auto setParam = [&] (const juce::String& id, float val) {
        if (auto* p = apvts.getParameter (id))
            p->setValueNotifyingHost (p->getNormalisableRange().convertTo0to1 (val));
    };

    setParam ("prog1PitchA", 24.0f);   // Max pitch up
    setParam ("delay", 5000.0f);       // Max delay
    setParam ("feedback", 95.0f); // Max feedback
    setParam ("mix", 100.0f);     // Full wet

    juce::MidiBuffer midi;

    // Process 5 seconds
    for (int block = 0; block < 469; ++block)
    {
        auto buffer = TestHelpers::makeStereoSine (440.0f, 48000.0, 512);
        plugin.processBlock (buffer, midi);

        for (int ch = 0; ch < 2; ++ch)
        {
            const auto* data = buffer.getReadPointer (ch);
            for (int i = 0; i < 512; ++i)
            {
                REQUIRE_FALSE (std::isnan (data[i]));
                REQUIRE_FALSE (std::isinf (data[i]));
                REQUIRE (std::abs (data[i]) < 2.0f); // Soft clipper should keep bounded
            }
        }
    }
}

TEST_CASE ("Plugin instance name", "[instance]")
{
    PluginProcessor testPlugin;
    CHECK (testPlugin.getName() == juce::String ("Barth Audios Pitch Transposer"));
}

TEST_CASE ("Full chain: all effects enabled simultaneously", "[integration]")
{
    PluginProcessor plugin;
    plugin.setPlayConfigDetails (2, 2, 48000.0, 512);
    plugin.prepareToPlay (48000.0, 512);

    auto& apvts = plugin.getAPVTS();
    auto setParam = [&] (const juce::String& id, float val) {
        if (auto* p = apvts.getParameter (id))
            p->setValueNotifyingHost (p->getNormalisableRange().convertTo0to1 (val));
    };

    // Enable everything
    setParam ("prog1PitchA", 5.0f);
    setParam ("fxSelect", 3.0f); // Phaser
    setParam ("fxFreq", 2.0f);
    setParam ("fxDepth", 50.0f);
    setParam ("reverbEnabled", 1.0f);
    setParam ("reverbDecay", 2.0f);
    setParam ("distEnabled", 1.0f);
    setParam ("distDrive", 50.0f);
    setParam ("adsrEnabled", 1.0f);
    setParam ("bitDepth", 8.0f);
    setParam ("srDiv", 2.0f);
    setParam ("mix", 100.0f);

    juce::MidiBuffer midi;
    for (int block = 0; block < 94; ++block) // ~1 sec
    {
        auto buffer = TestHelpers::makeStereoSine (440.0f, 48000.0, 512);
        plugin.processBlock (buffer, midi);

        for (int ch = 0; ch < 2; ++ch)
        {
            const auto* data = buffer.getReadPointer (ch);
            for (int i = 0; i < 512; ++i)
            {
                REQUIRE_FALSE (std::isnan (data[i]));
                REQUIRE_FALSE (std::isinf (data[i]));
            }
        }
    }
}

TEST_CASE ("Full chain: silence with all effects on", "[integration]")
{
    PluginProcessor plugin;
    plugin.setPlayConfigDetails (2, 2, 48000.0, 512);
    plugin.prepareToPlay (48000.0, 512);

    auto& apvts = plugin.getAPVTS();
    auto setParam = [&] (const juce::String& id, float val) {
        if (auto* p = apvts.getParameter (id))
            p->setValueNotifyingHost (p->getNormalisableRange().convertTo0to1 (val));
    };

    setParam ("reverbEnabled", 1.0f);
    setParam ("distEnabled", 1.0f);
    setParam ("distDrive", 50.0f);
    setParam ("mix", 100.0f);

    juce::MidiBuffer midi;
    float maxOut = 0.0f;

    for (int block = 0; block < 94; ++block)
    {
        juce::AudioBuffer<float> buffer (2, 512);
        buffer.clear();
        plugin.processBlock (buffer, midi);

        for (int ch = 0; ch < 2; ++ch)
        {
            const auto* data = buffer.getReadPointer (ch);
            for (int i = 0; i < 512; ++i)
                maxOut = std::max (maxOut, std::abs (data[i]));
        }
    }
    REQUIRE (maxOut < 0.01f);
}

TEST_CASE ("Full chain: various sample rates", "[integration]")
{
    for (double sr : { 44100.0, 96000.0, 192000.0 })
    {
        PluginProcessor plugin;
        plugin.setPlayConfigDetails (2, 2, sr, 512);
        plugin.prepareToPlay (sr, 512);

        juce::MidiBuffer midi;
        int numSamples = static_cast<int> (sr); // 1 second

        for (int pos = 0; pos + 512 <= numSamples; pos += 512)
        {
            auto buffer = TestHelpers::makeStereoSine (440.0f, sr, 512);
            plugin.processBlock (buffer, midi);

            for (int ch = 0; ch < 2; ++ch)
            {
                const auto* data = buffer.getReadPointer (ch);
                for (int i = 0; i < 512; ++i)
                {
                    REQUIRE_FALSE (std::isnan (data[i]));
                    REQUIRE_FALSE (std::isinf (data[i]));
                }
            }
        }
    }
}

TEST_CASE ("Full chain: comprehensive state round-trip", "[integration]")
{
    PluginProcessor plugin;
    plugin.setPlayConfigDetails (2, 2, 48000.0, 512);
    plugin.prepareToPlay (48000.0, 512);

    auto& apvts = plugin.getAPVTS();
    auto setParam = [&] (const juce::String& id, float val) {
        if (auto* p = apvts.getParameter (id))
            p->setValueNotifyingHost (p->getNormalisableRange().convertTo0to1 (val));
    };

    // Set many parameters
    setParam ("prog1PitchA", 7.0f);
    setParam ("prog1PitchB", -5.0f);
    setParam ("grain", 60.0f);
    setParam ("stretch", 150.0f);
    setParam ("delay", 500.0f);
    setParam ("feedback", 50.0f);
    setParam ("mix", 80.0f);
    setParam ("reverbEnabled", 1.0f);
    setParam ("reverbDecay", 3.0f);
    setParam ("distEnabled", 1.0f);
    setParam ("distDrive", 30.0f);
    setParam ("bitDepth", 12.0f);

    // Save
    juce::MemoryBlock stateData;
    plugin.getStateInformation (stateData);

    // Restore to new instance
    PluginProcessor plugin2;
    plugin2.setPlayConfigDetails (2, 2, 48000.0, 512);
    plugin2.prepareToPlay (48000.0, 512);
    plugin2.setStateInformation (stateData.getData(), static_cast<int> (stateData.getSize()));

    auto& apvts2 = plugin2.getAPVTS();

    // Verify all set parameters
    auto checkParam = [&] (const juce::String& id, float margin = 1.0f) {
        auto* v1 = apvts.getRawParameterValue (id);
        auto* v2 = apvts2.getRawParameterValue (id);
        REQUIRE (v1->load() == Catch::Approx (v2->load()).margin (margin));
    };

    checkParam ("prog1PitchA");
    checkParam ("prog1PitchB");
    checkParam ("grain");
    checkParam ("stretch");
    checkParam ("delay", 5.0f);
    checkParam ("feedback");
    checkParam ("mix");
    checkParam ("reverbDecay");
    checkParam ("distDrive");
    checkParam ("bitDepth");
}

TEST_CASE ("Full chain: parameter bounds iteration", "[integration][stress]")
{
    PluginProcessor plugin;
    plugin.setPlayConfigDetails (2, 2, 48000.0, 512);
    plugin.prepareToPlay (48000.0, 512);

    auto& apvts = plugin.getAPVTS();
    juce::MidiBuffer midi;

    // Iterate every parameter to min then max, process a block each time
    for (auto* param : apvts.processor.getParameters())
    {
        auto* rparam = dynamic_cast<juce::RangedAudioParameter*> (param);
        if (rparam == nullptr) continue;

        rparam->setValueNotifyingHost (0.0f); // min
        auto buffer = TestHelpers::makeStereoSine (440.0f, 48000.0, 512);
        plugin.processBlock (buffer, midi);

        for (int ch = 0; ch < 2; ++ch)
            for (int i = 0; i < 512; ++i)
            {
                REQUIRE_FALSE (std::isnan (buffer.getSample (ch, i)));
                REQUIRE_FALSE (std::isinf (buffer.getSample (ch, i)));
            }

        rparam->setValueNotifyingHost (1.0f); // max
        auto buffer2 = TestHelpers::makeStereoSine (440.0f, 48000.0, 512);
        plugin.processBlock (buffer2, midi);

        for (int ch = 0; ch < 2; ++ch)
            for (int i = 0; i < 512; ++i)
            {
                REQUIRE_FALSE (std::isnan (buffer2.getSample (ch, i)));
                REQUIRE_FALSE (std::isinf (buffer2.getSample (ch, i)));
            }
    }
}
