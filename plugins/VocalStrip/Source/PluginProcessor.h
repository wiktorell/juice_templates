#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>

class VocalStripAudioProcessor : public juce::AudioProcessor
{
public:
    VocalStripAudioProcessor();
    ~VocalStripAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "VocalStrip"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // GR meter value written by audio thread, read by UI timer at ~30 Hz
    // Range: 0.0 (no reduction) to -30.0 dB (maximum reduction)
    std::atomic<float> grMeterValue { 0.0f };

    juce::AudioProcessorValueTreeState parameters;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // =========================================================================
    // DSP Components — Phase DSP-1: EQ + Compressor (mono signal path)
    // =========================================================================

    // Cached sample rate for coefficient recalculation
    double currentSampleRate { 44100.0 };

    // EQ: 4 biquad IIR filters (mono, series)
    juce::dsp::IIR::Filter<float> hpFilter;
    juce::dsp::IIR::Filter<float> lsFilter;
    juce::dsp::IIR::Filter<float> midFilter;
    juce::dsp::IIR::Filter<float> hsFilter;

    // Compressor (mono, VCA-style)
    juce::dsp::Compressor<float> compressor;

    // =========================================================================
    // DSP Components — Phase DSP-2: Delay + Reverb Send (mono signal path)
    // =========================================================================

    // Delay line: mono, linear interpolation, max 200000 samples (~1000ms at 192kHz)
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLine { 200000 };

    // Delay dry/wet mixer (mono, linear mixing rule)
    juce::dsp::DryWetMixer<float> delayDryWet;

    // Feedback sample for delay feedback loop (reset in prepareToPlay)
    float feedbackSample { 0.0f };

    // Reverb send buffer: mono, sized to max block in prepareToPlay
    juce::AudioBuffer<float> reverbSendBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VocalStripAudioProcessor)
};
