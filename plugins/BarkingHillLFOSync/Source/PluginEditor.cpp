#include "PluginEditor.h"
#include "BinaryData.h"

//==============================================================================
BarkingHillLFOSyncAudioProcessorEditor::BarkingHillLFOSyncAudioProcessorEditor (
    BarkingHillLFOSyncAudioProcessor& p)
    : AudioProcessorEditor (&p),
      audioProcessor (p)
{
    // ══════════════════════════════════════════════════════════════════════════
    // CONSTRUCTION ORDER: Relays -> WebView -> Attachments
    // Mirrors the declaration order in the header.
    // Reference: juce8-critical-patterns.md §11
    // ══════════════════════════════════════════════════════════════════════════

    // ── 1. Create Relays ──────────────────────────────────────────────────────
    // Relay IDs must exactly match APVTS parameter IDs (case-sensitive).

    rateRelay      = std::make_unique<juce::WebSliderRelay>       ("rate");
    depthRelay     = std::make_unique<juce::WebSliderRelay>       ("depth");
    waveformRelay  = std::make_unique<juce::WebSliderRelay>       ("waveform");
    syncRelay      = std::make_unique<juce::WebToggleButtonRelay> ("sync");
    retriggerRelay = std::make_unique<juce::WebToggleButtonRelay> ("retrigger");
    ccNumberRelay  = std::make_unique<juce::WebSliderRelay>       ("cc_number");

    // ── 2. Create WebView — all relay options must be registered ──────────────
    // Resource provider is wired here; assets are served from BinaryData (Stage 3).
    // In Stage 1 the resource provider returns nullopt (404) for all requests;
    // the webView will display a blank/default state — that is expected.

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
    );

    // ── 3. Create Attachments ─────────────────────────────────────────────────
    // JUCE 8: always 3-parameter form (parameter, relay, undoManager).
    // Pass nullptr for undoManager. Missing nullptr causes silent failure (pattern #12).

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


    // ── 4. Add WebView to component hierarchy ─────────────────────────────────
    addAndMakeVisible (*webView);
    webView->goToURL (juce::WebBrowserComponent::getResourceProviderRoot());

    // ── 5. Set plugin window size (700 x 390 per v7 mockup spec) ─────────────
    setSize (700, 390);
    setResizable (false, false);
}

BarkingHillLFOSyncAudioProcessorEditor::~BarkingHillLFOSyncAudioProcessorEditor()
{
    // Destruction happens automatically in REVERSE declaration order:
    //   Attachments first (ccNumberAttachment -> ... -> rateAttachment)
    //   then webView
    //   then Relays (assignRelay -> ... -> rateRelay)
    // Attachments are destroyed before webView — this is the required order.
}

//==============================================================================
void BarkingHillLFOSyncAudioProcessorEditor::paint (juce::Graphics& g)
{
    // WebView fills the entire component — dark brown backdrop visible during load.
    g.fillAll (juce::Colour (0xff1a0a00));
}

void BarkingHillLFOSyncAudioProcessorEditor::resized()
{
    // WebView fills the full plugin window (700 x 390).
    if (webView != nullptr)
        webView->setBounds (getLocalBounds());
}

//==============================================================================
/**
 * getResource()
 *
 * Serves HTML/JS assets from BinaryData to the WebView.
 *
 * Stage 1: Returns nullopt for all URLs (BinaryData targets added in Stage 3).
 * Stage 3 (gui-agent) will populate this method with explicit URL mappings:
 *
 *   "/" or "/index.html"              -> BinaryData::index_html
 *   "/js/juce/index.js"               -> BinaryData::index_js
 *   "/js/juce/check_native_interop.js" -> BinaryData::check_native_interop_js
 *
 * Explicit URL mapping is used (not a generic loop) — see pattern #8.
 * check_native_interop.js MUST be served — omitting it causes frozen UI (pattern #13).
 *
 * MIME types:
 *   .html -> "text/html"
 *   .js   -> "text/javascript"  (WebKit prefers text/javascript over application/javascript)
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
