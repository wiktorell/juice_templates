#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_extra/juce_gui_extra.h>

class VocalStripAudioProcessor;

class VocalStripAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit VocalStripAudioProcessorEditor (VocalStripAudioProcessor& p);
    ~VocalStripAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    std::optional<juce::WebBrowserComponent::Resource>
    getResource (const juce::String& url);

    VocalStripAudioProcessor& audioProcessor;

    // ========================================================================
    // 1 RELAYS FIRST (pattern #11 — no dependencies)
    // ========================================================================

    // EQ Section
    std::unique_ptr<juce::WebSliderRelay>        eqHpFreqRelay;
    std::unique_ptr<juce::WebSliderRelay>        eqLowShelfGainRelay;
    std::unique_ptr<juce::WebSliderRelay>        eqLowShelfFreqRelay;
    std::unique_ptr<juce::WebSliderRelay>        eqMidGainRelay;
    std::unique_ptr<juce::WebSliderRelay>        eqMidFreqRelay;
    std::unique_ptr<juce::WebSliderRelay>        eqMidQRelay;
    std::unique_ptr<juce::WebSliderRelay>        eqHighShelfGainRelay;
    std::unique_ptr<juce::WebSliderRelay>        eqHighShelfFreqRelay;
    std::unique_ptr<juce::WebToggleButtonRelay>  eqBypassRelay;

    // Compressor Section
    std::unique_ptr<juce::WebComboBoxRelay>      compModeRelay;
    std::unique_ptr<juce::WebSliderRelay>        compAmountRelay;
    std::unique_ptr<juce::WebSliderRelay>        compThresholdRelay;
    std::unique_ptr<juce::WebSliderRelay>        compRatioRelay;
    std::unique_ptr<juce::WebSliderRelay>        compAttackRelay;
    std::unique_ptr<juce::WebSliderRelay>        compReleaseRelay;
    std::unique_ptr<juce::WebSliderRelay>        compMakeupRelay;
    std::unique_ptr<juce::WebToggleButtonRelay>  compBypassRelay;

    // Delay Section
    std::unique_ptr<juce::WebComboBoxRelay>      delayModeRelay;
    std::unique_ptr<juce::WebSliderRelay>        delayTimeRelay;
    std::unique_ptr<juce::WebComboBoxRelay>      delayDivisionRelay;
    std::unique_ptr<juce::WebSliderRelay>        delayFeedbackRelay;
    std::unique_ptr<juce::WebSliderRelay>        delayMixRelay;
    std::unique_ptr<juce::WebSliderRelay>        delayReverbSendRelay;
    std::unique_ptr<juce::WebToggleButtonRelay>  delayBypassRelay;

    // Reverb Section
    std::unique_ptr<juce::WebComboBoxRelay>      reverbTypeRelay;
    std::unique_ptr<juce::WebSliderRelay>        reverbPredelayRelay;
    std::unique_ptr<juce::WebSliderRelay>        reverbDecayRelay;
    std::unique_ptr<juce::WebSliderRelay>        reverbDampingRelay;
    std::unique_ptr<juce::WebSliderRelay>        reverbMixRelay;
    std::unique_ptr<juce::WebToggleButtonRelay>  reverbBypassRelay;

    // ========================================================================
    // 2 WEBVIEW SECOND (pattern #11 — depends on relays)
    // ========================================================================
    std::unique_ptr<juce::WebBrowserComponent> webView;

    // ========================================================================
    // 3 ATTACHMENTS LAST (pattern #11 — depend on relays AND webView)
    // ========================================================================

    // EQ attachments
    std::unique_ptr<juce::WebSliderParameterAttachment>       eqHpFreqAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment>       eqLowShelfGainAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment>       eqLowShelfFreqAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment>       eqMidGainAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment>       eqMidFreqAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment>       eqMidQAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment>       eqHighShelfGainAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment>       eqHighShelfFreqAttachment;
    std::unique_ptr<juce::WebToggleButtonParameterAttachment> eqBypassAttachment;

    // Compressor attachments
    std::unique_ptr<juce::WebComboBoxParameterAttachment>     compModeAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment>       compAmountAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment>       compThresholdAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment>       compRatioAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment>       compAttackAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment>       compReleaseAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment>       compMakeupAttachment;
    std::unique_ptr<juce::WebToggleButtonParameterAttachment> compBypassAttachment;

    // Delay attachments
    std::unique_ptr<juce::WebComboBoxParameterAttachment>     delayModeAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment>       delayTimeAttachment;
    std::unique_ptr<juce::WebComboBoxParameterAttachment>     delayDivisionAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment>       delayFeedbackAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment>       delayMixAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment>       delayReverbSendAttachment;
    std::unique_ptr<juce::WebToggleButtonParameterAttachment> delayBypassAttachment;

    // Reverb attachments
    std::unique_ptr<juce::WebComboBoxParameterAttachment>     reverbTypeAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment>       reverbPredelayAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment>       reverbDecayAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment>       reverbDampingAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment>       reverbMixAttachment;
    std::unique_ptr<juce::WebToggleButtonParameterAttachment> reverbBypassAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VocalStripAudioProcessorEditor)
};
