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
    : AudioProcessor (BusesProperties())   // MIDI FX — no audio buses
    , parameters (*this, nullptr, "Parameters", createParameterLayout())
{
}

BarkingHillLFOSyncAudioProcessor::~BarkingHillLFOSyncAudioProcessor()
{
}

//==============================================================================
void BarkingHillLFOSyncAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (samplesPerBlock);

    // Store sample rate for LFO phase increment calculation
    currentSampleRate = sampleRate;

    // Reset LFO phase to beginning of cycle
    lfoPhase = 0.0f;
}

void BarkingHillLFOSyncAudioProcessor::releaseResources()
{
    // Cleanup for DSP components (Stage 2)
}

bool BarkingHillLFOSyncAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // MIDI FX — no audio buses
    juce::ignoreUnused (layouts);
    return true;
}

// ── Phase 3.2: 13-step subdivision table for MIDI clock sync ─────────────────
// Maps subdivision index [0, 12] to duration in quarter-note units.
// Used to derive LFO phase from DAW ppqPosition when sync=true.
static const double kSubdivisionQN[13] = {
    16.0,           // 0:  4 BAR
    8.0,            // 1:  2 BAR
    4.0,            // 2:  1 BAR
    2.0,            // 3:  1/2
    4.0 / 3.0,      // 4:  1/2T (triplet half)
    1.0,            // 5:  1/4
    2.0 / 3.0,      // 6:  1/4T (triplet quarter)
    0.5,            // 7:  1/8
    1.0 / 3.0,      // 8:  1/8T
    0.25,           // 9:  1/16
    1.0 / 6.0,      // 10: 1/16T
    0.125,          // 11: 1/32
    1.0 / 12.0      // 12: 1/32T
};

//==============================================================================
void BarkingHillLFOSyncAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                                      juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Guard: ensure sampleRate is valid before computing phase increment.
    // Prevents division by zero if host calls processBlock before prepareToPlay.
    if (currentSampleRate <= 0.0)
        return;

    // ── Phase 3.1 + 3.2: LFO Core + MIDI CC Output + Sync + Retrigger ────────
    // Audio buffer is NOT modified — BarkingHillLFOSync is a modulation utility.

    // Step 1: Read parameters (atomic loads — real-time safe)
    const float rateHz      = parameters.getRawParameterValue ("rate")->load();
    const float depthPct    = parameters.getRawParameterValue ("depth")->load();
    const int   waveformIdx = static_cast<int> (parameters.getRawParameterValue ("waveform")->load() + 0.5f);
    const int   ccNumber    = static_cast<int> (parameters.getRawParameterValue ("cc_number")->load());
    const bool  syncEnabled = parameters.getRawParameterValue ("sync")->load() >= 0.5f;
    const bool  retrigEnabled = parameters.getRawParameterValue ("retrigger")->load() >= 0.5f;

    const int   numSamples = buffer.getNumSamples();

    // Step 2: [NEW Phase 3.2] AudioPlayHead sync query
    // When sync=true and DAW transport is playing, derive lfoPhase from ppqPosition.
    // This keeps the LFO locked to the transport after scrubs and loop jumps.
    bool phaseAdvanced = false;

    if (syncEnabled)
    {
        if (auto* playHead = getPlayHead())
        {
            if (auto pos = playHead->getPosition())
            {
                const double bpm       = pos->getBpm().hasValue()         ? *pos->getBpm()         : 120.0;
                const double ppq       = pos->getPpqPosition().hasValue() ? *pos->getPpqPosition() : 0.0;
                const bool   isPlaying = pos->getIsPlaying();

                if (isPlaying && bpm > 0.0)
                {
                    // Map rate Hz [0.1, 10.0] → normalised [0, 1] → subdivision index [0, 12]
                    const float normRate  = (rateHz - 0.1f) / 9.9f;
                    const int   subdivIdx = juce::jlimit (0, 12, static_cast<int> (std::round (normRate * 12.0f)));

                    const double subdivQN = kSubdivisionQN[subdivIdx];

                    // Derive normalised phase [0, 1) from ppqPosition.
                    // fmod() keeps phase correct after transport scrub and loop jumps.
                    const double rawPhase = std::fmod (ppq, subdivQN) / subdivQN;
                    lfoPhase = static_cast<float> (rawPhase);

                    // Phase has been set from transport — skip free-mode advance below.
                    phaseAdvanced = true;
                }
                // If not playing: freeze phase at current value (phaseAdvanced stays false,
                // but we also skip free advance to hold position during transport stop).
                else if (!isPlaying)
                {
                    phaseAdvanced = true;  // Freeze: don't advance while transport is stopped
                }
            }
        }
    }

    // Step 3: [NEW Phase 3.2] MIDI note-on retrigger scan
    // Runs AFTER sync phase derivation so retrigger overrides sync on note-on.
    if (retrigEnabled)
    {
        for (const auto metadata : midiMessages)
        {
            const auto msg = metadata.getMessage();
            if (msg.isNoteOn())
            {
                lfoPhase = 0.0f;
                break;  // Only retrigger on first note-on in buffer
            }
        }
    }

    // Step 4: Free-mode phase advance (only when sync has NOT locked the phase)
    // phaseInc represents how many full cycles occur over the block duration.
    if (!phaseAdvanced)
    {
        const float phaseInc = rateHz / static_cast<float> (currentSampleRate) * static_cast<float> (numSamples);
        lfoPhase += phaseInc;
    }

    // Step 5: Wrap phase into [0.0, 1.0)
    // Guard negative phase (can occur with pre-roll ppqPosition in sync mode)
    if (lfoPhase < 0.0f)
        lfoPhase += 1.0f - std::floor (lfoPhase);
    if (lfoPhase >= 1.0f)
        lfoPhase -= std::floor (lfoPhase);

    // Step 6: Compute LFO output from waveform formula (phase p in [0.0, 1.0))
    float lfoOutput = 0.0f;
    switch (waveformIdx)
    {
        case 0: // Sine
            lfoOutput = std::sin (lfoPhase * juce::MathConstants<float>::twoPi);
            break;
        case 1: // Triangle
            lfoOutput = (lfoPhase < 0.5f) ? (4.0f * lfoPhase - 1.0f)
                                           : (3.0f - 4.0f * lfoPhase);
            break;
        case 2: // Square
            lfoOutput = (lfoPhase < 0.5f) ? 1.0f : -1.0f;
            break;
        case 3: // Saw Up
            lfoOutput = 2.0f * lfoPhase - 1.0f;
            break;
        case 4: // Saw Down
            lfoOutput = 1.0f - 2.0f * lfoPhase;
            break;
        default:
            lfoOutput = 0.0f;
            break;
    }

    // Step 7: Apply bipolar depth scaling
    // depthPct range: [-100, +100] → scale factor [-1.0, +1.0]
    // scaledLFO range: [-1.0, +1.0]
    const float scaledLFO = lfoOutput * (depthPct / 100.0f);

    // Step 8: Generate MIDI CC output (one CC event per buffer at offset 0)
    // Map scaledLFO [-1, +1] → MIDI CC [0, 127]
    const int ccVal = juce::jlimit (0, 127, static_cast<int> ((scaledLFO + 1.0f) * 63.5f));
    midiMessages.addEvent (juce::MidiMessage::controllerEvent (1, ccNumber, ccVal), 0);

    // Step 9: Update mod_output automation parameter (normalised [0, 1])
    // 0.5 = LFO at centre (scaledLFO = 0.0), 0.0 = minimum, 1.0 = maximum
    const float normalisedLFO = (scaledLFO + 1.0f) * 0.5f;
    auto* modOutParam = parameters.getParameter ("mod_output");
    modOutParam->setValueNotifyingHost (normalisedLFO);

    // Step 10: Audio passthrough — buffer is left completely unmodified.
    // BarkingHillLFOSync is a modulation utility; audio passes through unchanged.
    juce::ignoreUnused (buffer);
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
    // mod_output will be overwritten every processBlock regardless of saved value.
}

//==============================================================================
// Plugin factory function — required entry point for all JUCE plugins
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BarkingHillLFOSyncAudioProcessor();
}
