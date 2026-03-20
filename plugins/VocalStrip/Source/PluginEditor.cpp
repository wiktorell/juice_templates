#include "PluginEditor.h"

VocalStripAudioProcessorEditor::VocalStripAudioProcessorEditor(VocalStripAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    // Window size: 1120 x 520 px — horizontal channel strip layout
    // (4 sections x 280px: EQ | Comp | Delay | Reverb)
    // This matches the v6 UI mockup dimensions.
    setSize(1120, 520);
}

VocalStripAudioProcessorEditor::~VocalStripAudioProcessorEditor()
{
}

void VocalStripAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Vintage-hardware-001 placeholder — warm brown background
    g.fillAll(juce::Colour(0xff2a1f14));

    // Plugin title
    g.setColour(juce::Colour(0xffd4a574));  // Brass gold
    g.setFont(juce::Font(28.0f, juce::Font::bold));
    g.drawFittedText("VocalStrip", getLocalBounds().removeFromTop(60), juce::Justification::centred, 1);

    // Stage indicator
    g.setFont(juce::Font(14.0f));
    g.setColour(juce::Colour(0xff8a7060));
    g.drawFittedText("Stage 1 — Foundation + Shell",
                     getLocalBounds().reduced(20).removeFromTop(90).removeFromBottom(30),
                     juce::Justification::centred, 1);

    // Section labels — EQ | Comp | Delay | Reverb
    const int sectionWidth = getWidth() / 4;
    const juce::String sections[] = { "EQ", "Comp", "Delay", "Reverb" };
    const juce::String paramCounts[] = { "9 params", "8 params", "7 params", "6 params" };

    for (int i = 0; i < 4; ++i)
    {
        juce::Rectangle<int> sectionArea(i * sectionWidth, 120, sectionWidth, getHeight() - 160);

        // Section divider
        g.setColour(juce::Colour(0xff3d2e1e));
        g.fillRect(sectionArea.reduced(4));

        // Section border
        g.setColour(juce::Colour(0xff6b5a42));
        g.drawRect(sectionArea.reduced(4), 1);

        // Section name
        g.setColour(juce::Colour(0xffd4a574));
        g.setFont(juce::Font(16.0f, juce::Font::bold));
        g.drawFittedText(sections[i],
                         sectionArea.reduced(8).removeFromTop(40),
                         juce::Justification::centred, 1);

        // Parameter count
        g.setColour(juce::Colour(0xff8a7060));
        g.setFont(juce::Font(12.0f));
        g.drawFittedText(paramCounts[i],
                         sectionArea.reduced(8).removeFromBottom(30),
                         juce::Justification::centred, 1);
    }

    // Parameter summary at bottom
    g.setColour(juce::Colour(0xff6b5a42));
    g.setFont(juce::Font(12.0f));
    g.drawFittedText("30 parameters implemented — 22 Float, 4 Bool, 4 Choice — WebView UI added in Stage 3",
                     getLocalBounds().removeFromBottom(30),
                     juce::Justification::centred, 1);
}

void VocalStripAudioProcessorEditor::resized()
{
    // Layout implemented by gui-agent in Stage 3 (GUI).
    // WebView will be set to fill the entire editor bounds.
}
