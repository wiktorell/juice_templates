#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include "PluginProcessor.h"

//==============================================================================
/**
 * BarkingHillLFOSyncAudioProcessorEditor
 *
 * WebView-based plugin editor for BarkingHillLFOSync.
 * Stage 1: Stub implementation — WebView relays declared, placeholder paint().
 *          Full WebView integration added by gui-agent in Stage 3.
 *
 * CRITICAL MEMBER DECLARATION ORDER (pattern #11):
 * Members are destroyed in REVERSE order of declaration.
 * Required order: Relays -> WebView -> Attachments
 *
 *   Relays      — no dependencies, constructed first
 *   WebView     — depends on relays being registered at construction time
 *   Attachments — depend on relays AND webView; destroyed first at runtime
 *
 * Breaking this order causes undefined behaviour in release builds.
 * Reference: troubleshooting/patterns/juce8-critical-patterns.md §11
 */
class BarkingHillLFOSyncAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit BarkingHillLFOSyncAudioProcessorEditor (BarkingHillLFOSyncAudioProcessor&);
    ~BarkingHillLFOSyncAudioProcessorEditor() override;

    //==========================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    //==========================================================================
    // Resource provider — serves BinaryData assets to the WebView (Stage 3)
    std::optional<juce::WebBrowserComponent::Resource>
    getResource (const juce::String& url);

    //==========================================================================
    // Reference to processor (must outlive editor)
    BarkingHillLFOSyncAudioProcessor& audioProcessor;

    //==========================================================================
    // MEMBER DECLARATION ORDER: Relays -> WebView -> Attachments (pattern #11)

    // 1. RELAYS FIRST — no dependencies, safe to construct first
    //    IDs must exactly match APVTS parameter IDs (case-sensitive)
    std::unique_ptr<juce::WebSliderRelay>        rateRelay;
    std::unique_ptr<juce::WebSliderRelay>        depthRelay;
    std::unique_ptr<juce::WebSliderRelay>        waveformRelay;   // Choice as stepped slider
    std::unique_ptr<juce::WebToggleButtonRelay>  syncRelay;
    std::unique_ptr<juce::WebToggleButtonRelay>  retriggerRelay;
    std::unique_ptr<juce::WebSliderRelay>        ccNumberRelay;   // Integer-stepped slider

    // 2. WEBVIEW SECOND — constructed after all relays are registered
    std::unique_ptr<juce::WebBrowserComponent>   webView;

    // 3. ATTACHMENTS LAST — destroyed first (reverse order)
    //    JUCE 8: WebSliderParameterAttachment requires 3 args: (param, relay, nullptr)
    //    Reference: juce8-critical-patterns.md §12
    std::unique_ptr<juce::WebSliderParameterAttachment>        rateAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment>        depthAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment>        waveformAttachment;
    std::unique_ptr<juce::WebToggleButtonParameterAttachment>  syncAttachment;
    std::unique_ptr<juce::WebToggleButtonParameterAttachment>  retriggerAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment>        ccNumberAttachment;

    //==========================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BarkingHillLFOSyncAudioProcessorEditor)
};
