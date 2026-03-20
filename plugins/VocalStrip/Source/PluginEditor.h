#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

class VocalStripAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit VocalStripAudioProcessorEditor(VocalStripAudioProcessor&);
    ~VocalStripAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    VocalStripAudioProcessor& processorRef;

    // WebView UI members declared here, initialized by gui-agent in Stage 3 (GUI).
    // Order: Relays → WebView → Attachments (critical pattern #11)
    // All 22 Float relays, 4 Bool relays, 4 Choice relays + WebBrowserComponent
    // + WebSliderParameterAttachment / WebToggleButtonRelay / WebComboBoxRelay attachments
    // will be added by gui-agent using std::unique_ptr members in correct order.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VocalStripAudioProcessorEditor)
};
