#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
/**
 * createParameterLayout()
 *
 * Implements all 7 user-facing parameters + 1 system parameter (mod_output).
 * Called in AudioProcessor initialiser list to populate the APVTS.
 *
 * Parameter spec source: plugins/BarkingHillLFOSync/.ideas/parameter-spec.md
 * JUCE 8 format: juce::ParameterID { "id", 1 } (version number required)
 */
juce::AudioProcessorValueTreeState::ParameterLayout
BarkingHillLFOSyncAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // ── rate ─────────────────────────────────────────────────────────────────
    // Float: 0.1 – 10.0 Hz, default 1.0 Hz, linear
    // In sync mode the normalised 0–1 value selects from 13 clock subdivisions.
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "rate", 1 },
        "Rate",
        juce::NormalisableRange<float> (0.1f, 10.0f, 0.0f, 1.0f),
        1.0f,
        juce::AudioParameterFloatAttributes{}.withLabel ("Hz")
    ));

    // ── depth ────────────────────────────────────────────────────────────────
    // Float: -100.0 – +100.0 %, default 50.0 %, bipolar (centre = 0 %)
    // Negative depth inverts the LFO output direction.
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "depth", 1 },
        "Depth",
        juce::NormalisableRange<float> (-100.0f, 100.0f, 0.0f, 1.0f),
        50.0f,
        juce::AudioParameterFloatAttributes{}.withLabel ("%")
    ));

    // ── waveform ──────────────────────────────────────────────────────────────
    // Choice: 5 waveform shapes, default index 0 (Sine)
    // Uses WebSliderRelay (stepped rotary knob in UI, not ComboBox).
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "waveform", 1 },
        "Waveform",
        juce::StringArray { "Sine", "Triangle", "Square", "Saw Up", "Saw Down" },
        0   // Default: Sine
    ));

    // ── sync ─────────────────────────────────────────────────────────────────
    // Bool: false = FREE (default), true = MIDI CLOCK
    // Switches rate interpretation between Hz and clock subdivision.
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { "sync", 1 },
        "Sync",
        false
    ));

    // ── retrigger ─────────────────────────────────────────────────────────────
    // Bool: false = OFF (default), true = ON
    // When ON, LFO phase resets to 0 on each incoming MIDI note-on.
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { "retrigger", 1 },
        "Retrigger",
        false
    ));

    // ── cc_number ─────────────────────────────────────────────────────────────
    // Float used as integer: 1 – 127, step 1, default 1 (Mod Wheel)
    // Selects the MIDI CC number for LFO output messages.
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "cc_number", 1 },
        "CC Number",
        juce::NormalisableRange<float> (1.0f, 127.0f, 1.0f, 1.0f),
        1.0f
    ));

    // ── assign ────────────────────────────────────────────────────────────────
    // Bool: false = DISARMED (default), true = ARMED
    // UI-only state — arms workflow guide animation; no DSP impact.
    // Resets to false on state restore (default behaviour via APVTS).
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { "assign", 1 },
        "Assign",
        false
    ));

    // ── mod_output (system parameter) ─────────────────────────────────────────
    // Float: 0.0 – 1.0 (normalised LFO output, 0.5 = centre for bipolar)
    // Exposes LFO value as automatable host automation parameter.
    // Updated each processBlock via setValueNotifyingHost() (audio thread, valid).
    // NOT persisted: default 0.5 (centre) on every load.
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "mod_output", 1 },
        "Mod Output",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.0f, 1.0f),
        0.5f
    ));

    return { params.begin(), params.end() };
}

//==============================================================================
BarkingHillLFOSyncAudioProcessor::BarkingHillLFOSyncAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
    , parameters (*this, nullptr, "Parameters", createParameterLayout())
{
}

BarkingHillLFOSyncAudioProcessor::~BarkingHillLFOSyncAudioProcessor()
{
}

//==============================================================================
void BarkingHillLFOSyncAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Initialization for DSP components (Stage 2):
    // - Store sampleRate for LFO phase increment calculation
    // - Reset lfoPhase = 0.0f
    // - Reset transport sync state
    juce::ignoreUnused (sampleRate, samplesPerBlock);
}

void BarkingHillLFOSyncAudioProcessor::releaseResources()
{
    // Cleanup for DSP components (Stage 2)
}

bool BarkingHillLFOSyncAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // Require stereo in + stereo out (audio passthrough utility plugin)
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

//==============================================================================
void BarkingHillLFOSyncAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                                      juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Stage 1: Pass-through audio (plugin is a modulation utility, audio unchanged)
    // DSP implementation (LFO, MIDI CC output, mod_output update) added in Stage 2.

    // Stage 2 will implement:
    //   1. AudioPlayHead position query (sync mode)
    //   2. Phase retrigger on MIDI note-on (retrigger parameter)
    //   3. LFO phase advance + waveform computation
    //   4. Bipolar depth scaling: scaledLFO = lfoOutput * (depth / 100.0f)
    //   5. MIDI CC output: MidiMessage::controllerEvent(1, ccNumber, ccVal)
    //   6. mod_output update: modOutputParam->setValueNotifyingHost(normalisedValue)
    //   7. Audio passthrough (no modification of buffer samples)

    // Real-time parameter reads (pattern for Stage 2 reference):
    //   auto* ratePtr      = parameters.getRawParameterValue ("rate");
    //   auto* depthPtr     = parameters.getRawParameterValue ("depth");
    //   auto* waveformPtr  = parameters.getRawParameterValue ("waveform");
    //   auto* syncPtr      = parameters.getRawParameterValue ("sync");
    //   auto* retrigPtr    = parameters.getRawParameterValue ("retrigger");
    //   auto* ccNumPtr     = parameters.getRawParameterValue ("cc_number");
    //   auto* modOutPtr    = parameters.getRawParameterValue ("mod_output");
    //
    //   float rateHz      = ratePtr->load();      // atomic read (real-time safe)
    //   float depthPct    = depthPtr->load();
    //   int   waveformIdx = static_cast<int> (waveformPtr->load() + 0.5f);
    //   bool  syncEnabled = syncPtr->load() >= 0.5f;
    //   bool  retrigEnabled = retrigPtr->load() >= 0.5f;
    //   int   ccNumber    = static_cast<int> (ccNumPtr->load());

    juce::ignoreUnused (midiMessages);
}

//==============================================================================
juce::AudioProcessorEditor* BarkingHillLFOSyncAudioProcessor::createEditor()
{
    return new BarkingHillLFOSyncAudioProcessorEditor (*this);
}

//==============================================================================
void BarkingHillLFOSyncAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Save APVTS state as XML binary
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void BarkingHillLFOSyncAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Restore APVTS state from XML binary
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState != nullptr && xmlState->hasTagName (parameters.state.getType()))
        parameters.replaceState (juce::ValueTree::fromXml (*xmlState));

    // Note: mod_output will be restored to saved value, but it is overwritten
    // every processBlock by setValueNotifyingHost(). Effectively runtime-only.
    // assign parameter restores to default (false) via APVTS default mechanism.
}

//==============================================================================
// Plugin factory function — required entry point for all JUCE plugins
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BarkingHillLFOSyncAudioProcessor();
}
