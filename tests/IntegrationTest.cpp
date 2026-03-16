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
    if (auto* p = apvts.getParameter ("pitchL"))
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
    auto* pitchL1 = apvts.getRawParameterValue ("pitchL");
    auto* pitchL2 = apvts2.getRawParameterValue ("pitchL");
    REQUIRE (pitchL1->load() == Catch::Approx (pitchL2->load()).margin (0.1f));

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
        if (auto* p = apvts.getParameter ("pitchL"))
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

    setParam ("pitchL", 24.0f);   // Max pitch up
    setParam ("delay", 5000.0f);  // Max delay
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
