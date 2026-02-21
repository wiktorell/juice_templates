#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include "PluginProcessor.h"

//==============================================================================
/**
 * BarkingHillLFOSyncAudioProcessorEditor
 *
 * WebView-based plugin editor for BarkingHillLFOSync.
 * Generated from v7 finalized mockup — 2026-02-21.
 *
 * ⚠️  CRITICAL MEMBER DECLARATION ORDER ⚠️
 * Members are destroyed in REVERSE order of declaration.
 * The WebView calls evaluateJavascript() during destruction of attachments,
 * so attachments MUST be destroyed BEFORE the WebView component.
 *
 * Required order:
 *   1. Relays   (no dependencies)
 *   2. WebView  (depends on relays being registered at construction time)
 *   3. Attachments (depend on relays AND webView; destroyed first at runtime)
 *
 * Breaking this order causes undefined behaviour in release builds (DAW freeze
 * on plugin reload). This is the #1 WebView integration mistake.
 *
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
    // Resource provider — serves BinaryData to the WebView
    std::optional<juce::WebBrowserComponent::Resource>
    getResource (const juce::String& url);

    //==========================================================================
    // Reference to processor (must outlive editor)
    BarkingHillLFOSyncAudioProcessor& audioProcessor;

    //==========================================================================
    // ══════════════════════════════════════════════════════════════════
    // MEMBER DECLARATION ORDER: Relays → WebView → Attachments
    // ══════════════════════════════════════════════════════════════════

    // 1️⃣  RELAYS FIRST — no dependencies, safe to construct first
    std::unique_ptr<juce::WebSliderRelay>        rateRelay;
    std::unique_ptr<juce::WebSliderRelay>        depthRelay;
    std::unique_ptr<juce::WebSliderRelay>        waveformRelay;   // Choice as stepped slider
    std::unique_ptr<juce::WebToggleButtonRelay>  syncRelay;
    std::unique_ptr<juce::WebToggleButtonRelay>  retriggerRelay;
    std::unique_ptr<juce::WebSliderRelay>        ccNumberRelay;   // Int as stepped slider
    std::unique_ptr<juce::WebToggleButtonRelay>  assignRelay;

    // 2️⃣  WEBVIEW SECOND — constructed after all relays are registered
    std::unique_ptr<juce::WebBrowserComponent>   webView;

    // 3️⃣  ATTACHMENTS LAST — destroyed first (reverse order), can safely
    //      call webView->evaluateJavascript() during destruction
    std::unique_ptr<juce::WebSliderParameterAttachment>        rateAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment>        depthAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment>        waveformAttachment;
    std::unique_ptr<juce::WebToggleButtonParameterAttachment>  syncAttachment;
    std::unique_ptr<juce::WebToggleButtonParameterAttachment>  retriggerAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment>        ccNumberAttachment;
    std::unique_ptr<juce::WebToggleButtonParameterAttachment>  assignAttachment;

    //==========================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BarkingHillLFOSyncAudioProcessorEditor)
};
