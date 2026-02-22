#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

//==============================================================================
/**
 * BarkingHillLFOSyncAudioProcessor
 *
 * LFO Sync — tempo-syncable LFO modulation utility.
 *
 * Plugin type:    Effect (IS_SYNTH FALSE), stereo audio passthrough
 * MIDI:           NEEDS_MIDI_INPUT TRUE, NEEDS_MIDI_OUTPUT TRUE
 * Parameters:     7 (rate, depth, waveform, sync, retrigger, cc_number, assign)
 *                 + 1 system parameter (mod_output, runtime-only)
 *
 * Stage 2: DSP Complete (all phases)
 * - Phase 3.1: LFO oscillator (5 waveforms), MIDI CC output, mod_output automation
 * - Phase 3.2: MIDI clock sync (AudioPlayHead, 13 subdivisions), phase retrigger
 * - Phase 3.3: Edge case hardening, state persistence verified
 */
class BarkingHillLFOSyncAudioProcessor : public juce::AudioProcessor
{
public:
    //==========================================================================
    BarkingHillLFOSyncAudioProcessor();
    ~BarkingHillLFOSyncAudioProcessor() override;

    //==========================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==========================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    //==========================================================================
    const juce::String getName() const override { return "LFO Sync"; }

    // MIDI I/O enabled (retrigger input + CC output)
    bool acceptsMidi() const override  { return true; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    //==========================================================================
    int getNumPrograms() override    { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    //==========================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==========================================================================
    // APVTS — public so editor can access parameters for relay attachment
    juce::AudioProcessorValueTreeState parameters;

private:
    //==========================================================================
    // Parameter layout factory — called in initialiser list
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // LFO State (Phase 3.1)
    // Phase accumulator: [0.0, 1.0), advanced each processBlock in FREE mode
    float lfoPhase = 0.0f;

    // Sample rate stored in prepareToPlay for phase increment calculation
    double currentSampleRate = 44100.0;

    // mod_output system parameter pointer (runtime-only, not in layout save/load)
    // Accessed via parameters.getParameter("mod_output") in processBlock
    // setValueNotifyingHost() called each processBlock from audio thread (valid per JUCE docs)

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BarkingHillLFOSyncAudioProcessor)
};
