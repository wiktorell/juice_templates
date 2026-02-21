#include "PluginEditor.h"

//==============================================================================
BarkingHillLFOSyncAudioProcessorEditor::BarkingHillLFOSyncAudioProcessorEditor (
    BarkingHillLFOSyncAudioProcessor& p)
    : AudioProcessorEditor (&p),
      audioProcessor (p)
{
    // ══════════════════════════════════════════════════════════════════
    // CONSTRUCTION ORDER: Relays → WebView → Attachments
    // Must mirror the declaration order in the header.
    // ══════════════════════════════════════════════════════════════════

    // ── 1. Create Relays ────────────────────────────────────────────
    // Relay IDs must exactly match the APVTS parameter IDs (case-sensitive).

    rateRelay      = std::make_unique<juce::WebSliderRelay>       ("rate");
    depthRelay     = std::make_unique<juce::WebSliderRelay>       ("depth");
    waveformRelay  = std::make_unique<juce::WebSliderRelay>       ("waveform");
    syncRelay      = std::make_unique<juce::WebToggleButtonRelay> ("sync");
    retriggerRelay = std::make_unique<juce::WebToggleButtonRelay> ("retrigger");
    ccNumberRelay  = std::make_unique<juce::WebSliderRelay>       ("cc_number");
    assignRelay    = std::make_unique<juce::WebToggleButtonRelay> ("assign");

    // ── 2. Create WebView — all relay options must be registered ────
    webView = std::make_unique<juce::WebBrowserComponent>(
        juce::WebBrowserComponent::Options{}
            .withNativeIntegrationEnabled()
            .withResourceProvider ([this] (const juce::String& url)
                                   { return getResource (url); },
                                   juce::WebBrowserComponent::getResourceProviderRoot())
            .withOptionsFrom (*rateRelay)
            .withOptionsFrom (*depthRelay)
            .withOptionsFrom (*waveformRelay)
            .withOptionsFrom (*syncRelay)
            .withOptionsFrom (*retriggerRelay)
            .withOptionsFrom (*ccNumberRelay)
            .withOptionsFrom (*assignRelay)
    );

    // ── 3. Create Attachments ──────────────────────────────────────
    // JUCE 8: WebSliderParameterAttachment requires 3 arguments
    //         (parameter, relay, undoManager — pass nullptr).
    // Reference: juce8-critical-patterns.md §12

    rateAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
        *audioProcessor.parameters.getParameter ("rate"),
        *rateRelay,
        nullptr
    );

    depthAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
        *audioProcessor.parameters.getParameter ("depth"),
        *depthRelay,
        nullptr
    );

    waveformAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
        *audioProcessor.parameters.getParameter ("waveform"),
        *waveformRelay,
        nullptr
    );

    syncAttachment = std::make_unique<juce::WebToggleButtonParameterAttachment>(
        *audioProcessor.parameters.getParameter ("sync"),
        *syncRelay,
        nullptr
    );

    retriggerAttachment = std::make_unique<juce::WebToggleButtonParameterAttachment>(
        *audioProcessor.parameters.getParameter ("retrigger"),
        *retriggerRelay,
        nullptr
    );

    ccNumberAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
        *audioProcessor.parameters.getParameter ("cc_number"),
        *ccNumberRelay,
        nullptr
    );

    assignAttachment = std::make_unique<juce::WebToggleButtonParameterAttachment>(
        *audioProcessor.parameters.getParameter ("assign"),
        *assignRelay,
        nullptr
    );

    // ── 4. Add WebView to component hierarchy ──────────────────────
    addAndMakeVisible (*webView);
    webView->goToURL (juce::WebBrowserComponent::getResourceProviderRoot());

    // ── 5. Set plugin window size (from YAML: 700 x 390) ──────────
    setSize (700, 390);
    setResizable (false, false);
}

BarkingHillLFOSyncAudioProcessorEditor::~BarkingHillLFOSyncAudioProcessorEditor()
{
    // Destruction happens automatically in reverse declaration order:
    //   assignAttachment → ccNumberAttachment → retriggerAttachment →
    //   syncAttachment → waveformAttachment → depthAttachment → rateAttachment
    //   → webView
    //   → assignRelay → ccNumberRelay → retriggerRelay → syncRelay →
    //   waveformRelay → depthRelay → rateRelay
    //
    // Attachments are destroyed before webView — this is correct.
}

//==============================================================================
void BarkingHillLFOSyncAudioProcessorEditor::paint (juce::Graphics& g)
{
    // WebView fills the entire component — no additional painting required.
    g.fillAll (juce::Colour (0xff1a0a00));
}

void BarkingHillLFOSyncAudioProcessorEditor::resized()
{
    // WebView fills the full plugin window.
    if (webView != nullptr)
        webView->setBounds (getLocalBounds());
}

//==============================================================================
/**
 * Resource provider — serves HTML/JS assets from BinaryData to the WebView.
 *
 * BinaryData naming convention:
 *   Source path                        → BinaryData symbol
 *   Source/ui/public/index.html        → BinaryData::index_html
 *   Source/ui/public/js/juce/index.js  → BinaryData::index_js
 *   Source/ui/public/js/juce/
 *     check_native_interop.js          → BinaryData::check_native_interop_js
 *
 * Explicit URL mapping is used (not a generic loop) because BinaryData
 * flattens path separators to underscores, making generic matching fragile.
 * Reference: juce8-critical-patterns.md §8
 *
 * MIME types:
 *   .html  → "text/html"
 *   .js    → "text/javascript"   (NOT "application/javascript" — WebKit prefers text/javascript)
 *   .css   → "text/css"
 *
 * check_native_interop.js MUST be served — omitting it causes frozen UI.
 * Reference: juce8-critical-patterns.md §13
 */
std::optional<juce::WebBrowserComponent::Resource>
BarkingHillLFOSyncAudioProcessorEditor::getResource (const juce::String& url)
{
    auto makeVector = [] (const char* data, int size) -> std::vector<std::byte>
    {
        return std::vector<std::byte> (
            reinterpret_cast<const std::byte*> (data),
            reinterpret_cast<const std::byte*> (data) + size
        );
    };

    // ── index.html ────────────────────────────────────────────────
    if (url == "/" || url == "/index.html")
    {
        return juce::WebBrowserComponent::Resource {
            makeVector (BinaryData::index_html, BinaryData::index_htmlSize),
            juce::String ("text/html")
        };
    }

    // ── JUCE bridge: index.js ─────────────────────────────────────
    if (url == "/js/juce/index.js")
    {
        return juce::WebBrowserComponent::Resource {
            makeVector (BinaryData::index_js, BinaryData::index_jsSize),
            juce::String ("text/javascript")
        };
    }

    // ── JUCE bridge: check_native_interop.js (REQUIRED) ──────────
    if (url == "/js/juce/check_native_interop.js")
    {
        return juce::WebBrowserComponent::Resource {
            makeVector (BinaryData::check_native_interop_js,
                        BinaryData::check_native_interop_jsSize),
            juce::String ("text/javascript")
        };
    }

    // ── 404 — resource not found ──────────────────────────────────
    return std::nullopt;
}
