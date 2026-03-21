#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// Parameter layout — all 30 parameters in DAW automation order
// EQ (9) → Compressor (8) → Delay (7) → Reverb (6)
//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout VocalStripAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // =========================================================================
    // EQ Section — 8 Float + 1 Bool = 9 parameters
    // =========================================================================

    // eq_hp_freq — High-pass filter cutoff (kept for state compatibility, DSP uses fixed 100 Hz)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "eq_hp_freq", 1 },
        "HP Freq",
        juce::NormalisableRange<float>(20.0f, 500.0f, 0.1f, 1.0f),
        100.0f,
        "Hz"
    ));

    // eq_hp_enable — HP filter on/off switch (fixed at 100 Hz)
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "eq_hp_enable", 1 },
        "HP Enable",
        true
    ));

    // eq_low_shelf_gain — Low shelf boost/cut (-20 to +20 dB)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "eq_low_shelf_gain", 1 },
        "Low Shelf Gain",
        juce::NormalisableRange<float>(-20.0f, 20.0f, 0.1f, 1.0f),
        0.0f,
        "dB"
    ));

    // eq_low_shelf_freq — Low shelf frequency (kept for state compatibility, DSP uses fixed 150 Hz)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "eq_low_shelf_freq", 1 },
        "Low Shelf Freq",
        juce::NormalisableRange<float>(60.0f, 400.0f, 0.1f, 1.0f),
        150.0f,
        "Hz"
    ));

    // eq_mid_gain — Mid peak boost/cut (-20 to +20 dB)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "eq_mid_gain", 1 },
        "Mid Gain",
        juce::NormalisableRange<float>(-20.0f, 20.0f, 0.1f, 1.0f),
        0.0f,
        "dB"
    ));

    // eq_mid_freq — Mid peak center frequency (sweepable)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "eq_mid_freq", 1 },
        "Mid Freq",
        juce::NormalisableRange<float>(200.0f, 8000.0f, 1.0f, 1.0f),
        1000.0f,
        "Hz"
    ));

    // eq_mid_q — Mid peak bandwidth (kept for state compatibility, DSP uses fixed Q=1.0)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "eq_mid_q", 1 },
        "Mid Q",
        juce::NormalisableRange<float>(0.3f, 5.0f, 0.01f, 1.0f),
        1.0f,
        ""
    ));

    // eq_high_shelf_gain — High shelf boost/cut (-20 to +20 dB)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "eq_high_shelf_gain", 1 },
        "Hi Shelf Gain",
        juce::NormalisableRange<float>(-20.0f, 20.0f, 0.1f, 1.0f),
        0.0f,
        "dB"
    ));

    // eq_high_shelf_freq — High shelf frequency (kept for state compatibility, DSP uses fixed 10 kHz)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "eq_high_shelf_freq", 1 },
        "Hi Shelf Freq",
        juce::NormalisableRange<float>(2000.0f, 16000.0f, 1.0f, 1.0f),
        10000.0f,
        "Hz"
    ));

    // eq_bypass — Bypass entire EQ section
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "eq_bypass", 1 },
        "EQ Bypass",
        false
    ));

    // =========================================================================
    // Compressor Section — 6 Float + 1 Bool + 1 Choice = 8 parameters
    // =========================================================================

    // comp_mode — Simple (one-knob) or Manual (full control)
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID { "comp_mode", 1 },
        "Comp Mode",
        juce::StringArray { "Simple", "Manual" },
        0  // Default: Simple
    ));

    // comp_amount — One-knob compression amount (Simple mode)
    // Maps: threshold = -6.0 - (amount/100 * 34.0), ratio = 2.0 + (amount/100 * 8.0)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "comp_amount", 1 },
        "Comp Amount",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f, 1.0f),
        50.0f,
        "%"
    ));

    // comp_threshold — Compressor threshold (Manual mode)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "comp_threshold", 1 },
        "Threshold",
        juce::NormalisableRange<float>(-60.0f, 0.0f, 0.1f, 1.0f),
        -18.0f,
        "dB"
    ));

    // comp_ratio — Compression ratio (Manual mode)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "comp_ratio", 1 },
        "Ratio",
        juce::NormalisableRange<float>(1.0f, 20.0f, 0.1f, 1.0f),
        4.0f,
        ":1"
    ));

    // comp_attack — Attack time (Manual mode)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "comp_attack", 1 },
        "Attack",
        juce::NormalisableRange<float>(0.1f, 100.0f, 0.1f, 1.0f),
        10.0f,
        "ms"
    ));

    // comp_release — Release time (Manual mode)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "comp_release", 1 },
        "Release",
        juce::NormalisableRange<float>(10.0f, 1000.0f, 1.0f, 1.0f),
        200.0f,
        "ms"
    ));

    // comp_makeup — Post-compressor makeup gain (always visible)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "comp_makeup", 1 },
        "Makeup Gain",
        juce::NormalisableRange<float>(0.0f, 24.0f, 0.1f, 1.0f),
        0.0f,
        "dB"
    ));

    // comp_bypass — Bypass entire compressor section
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "comp_bypass", 1 },
        "Comp Bypass",
        false
    ));

    // =========================================================================
    // Delay Section — 5 Float + 1 Bool + 2 Choice = 8 parameters (spec: 7)
    // Note: delay_mode + delay_division are both Choice, delay_time is Float
    // =========================================================================

    // delay_mode — Slapback (explicit ms) or Sync (BPM-locked)
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID { "delay_mode", 1 },
        "Delay Mode",
        juce::StringArray { "Slapback", "Sync" },
        0  // Default: Slapback
    ));

    // delay_time — Delay time in milliseconds (Slapback mode, max 250ms)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "delay_time", 1 },
        "Delay Time",
        juce::NormalisableRange<float>(20.0f, 250.0f, 0.1f, 1.0f),
        100.0f,
        "ms"
    ));

    // delay_division — Note division for tempo-sync mode
    // 1/16 = (60000/BPM)/4, 1/8 = (60000/BPM)/2, 1/4 = (60000/BPM), 1/2 = (60000/BPM)*2
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID { "delay_division", 1 },
        "Delay Division",
        juce::StringArray { "1/16", "1/8", "1/4", "1/2" },
        1  // Default: 1/8
    ));

    // delay_feedback — Delay feedback amount (0-90% for stability)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "delay_feedback", 1 },
        "Feedback",
        juce::NormalisableRange<float>(0.0f, 90.0f, 0.1f, 1.0f),
        20.0f,
        "%"
    ));

    // delay_mix — Delay wet/dry mix
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "delay_mix", 1 },
        "Delay Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f, 1.0f),
        30.0f,
        "%"
    ));

    // delay_reverb_send — Amount of delay wet signal fed into reverb input
    // Send tap is pre-D/W mix so delay_mix and delay_reverb_send are independent
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "delay_reverb_send", 1 },
        "Reverb Send",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f, 1.0f),
        20.0f,
        "%"
    ));

    // delay_bypass — Bypass entire delay section (also disables reverb send)
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "delay_bypass", 1 },
        "Delay Bypass",
        false
    ));

    // =========================================================================
    // Reverb Section — 4 Float + 1 Bool + 1 Choice = 6 parameters
    // =========================================================================

    // reverb_type — Plate or Room character preset
    // Plate: roomSize 0.4-0.95, width 1.0; Room: roomSize 0.1-0.60, width 0.7
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID { "reverb_type", 1 },
        "Reverb Type",
        juce::StringArray { "Plate", "Room" },
        0  // Default: Plate
    ));

    // reverb_predelay — Pre-delay before reverb tail onset (stereo)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "reverb_predelay", 1 },
        "Pre-Delay",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f, 1.0f),
        20.0f,
        "ms"
    ));

    // reverb_decay — Reverb tail decay time (maps to roomSize parameter)
    // decay_normalized = (decay_s - 0.3) / (8.0 - 0.3)
    // Plate: roomSize = 0.4 + decay_norm * 0.55; Room: roomSize = 0.1 + decay_norm * 0.50
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "reverb_decay", 1 },
        "Decay",
        juce::NormalisableRange<float>(0.3f, 8.0f, 0.01f, 1.0f),
        1.5f,
        "s"
    ));

    // reverb_damping — High-frequency damping in reverb tail
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "reverb_damping", 1 },
        "Damping",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f, 1.0f),
        50.0f,
        "%"
    ));

    // reverb_mix — Reverb wet/dry mix
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "reverb_mix", 1 },
        "Reverb Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f, 1.0f),
        25.0f,
        "%"
    ));

    // reverb_bypass — Bypass entire reverb section (includes pre-delay)
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "reverb_bypass", 1 },
        "Reverb Bypass",
        false
    ));

    return { params.begin(), params.end() };
}

//==============================================================================
// Constructor
//==============================================================================
VocalStripAudioProcessor::VocalStripAudioProcessor()
    : AudioProcessor(BusesProperties()
                        .withInput("Input",   juce::AudioChannelSet::mono(),   true)
                        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , parameters(*this, nullptr, "Parameters", createParameterLayout())
{
}

VocalStripAudioProcessor::~VocalStripAudioProcessor()
{
}

//==============================================================================
// Bus layout — accept mono-in + stereo-out only
//==============================================================================
bool VocalStripAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // Output must always be stereo
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // Input must be mono (vocal mic source)
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::mono())
        return false;

    return true;
}

//==============================================================================
// Prepare to play — DSP initialization
//==============================================================================
void VocalStripAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    // Mono spec for EQ and compressor (mono signal path before stereo expansion)
    juce::dsp::ProcessSpec monoSpec;
    monoSpec.sampleRate = sampleRate;
    monoSpec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    monoSpec.numChannels = 1;

    // Prepare EQ filters (mono)
    hpFilter.prepare(monoSpec);
    lsFilter.prepare(monoSpec);
    midFilter.prepare(monoSpec);
    hsFilter.prepare(monoSpec);

    hpFilter.reset();
    lsFilter.reset();
    midFilter.reset();
    hsFilter.reset();

    // Prepare compressor (mono)
    compressor.prepare(monoSpec);
    compressor.reset();

    // Reset GR meter to 0 on playback start
    grMeterValue.store(0.0f);

    // =========================================================================
    // Phase DSP-2: Delay + Reverb Send
    // =========================================================================

    // Prepare delay line (mono)
    delayLine.prepare(monoSpec);
    delayLine.reset();

    // Prepare delay dry/wet mixer (mono, linear mixing)
    delayDryWet.prepare(monoSpec);
    delayDryWet.setMixingRule(juce::dsp::DryWetMixingRule::linear);
    delayDryWet.reset();

    // Reset feedback state
    feedbackSample = 0.0f;

    // Preallocate reverb send buffer (mono, max block size)
    reverbSendBuffer.setSize(1, samplesPerBlock);
    reverbSendBuffer.clear();

    // =========================================================================
    // Phase DSP-3: Reverb + Full Chain Integration
    // =========================================================================

    // Stereo spec for reverb (2 channels)
    juce::dsp::ProcessSpec stereoSpec;
    stereoSpec.sampleRate = sampleRate;
    stereoSpec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    stereoSpec.numChannels = 2;

    // Pre-delay: stereo (L+R identical delay instances, mono)
    preDelayL.prepare(monoSpec);
    preDelayL.reset();
    preDelayR.prepare(monoSpec);
    preDelayR.reset();

    // Reverb: stereo (CRITICAL: use prepare(spec) — pattern #17)
    reverb.prepare(stereoSpec);
    reverb.reset();

    // Reverb dry/wet mixer: stereo, linear mixing
    reverbDryWet.prepare(stereoSpec);
    reverbDryWet.setMixingRule(juce::dsp::DryWetMixingRule::linear);
    reverbDryWet.reset();
}

void VocalStripAudioProcessor::releaseResources()
{
    // Release large buffers to save memory when plugin not in use
    reverbSendBuffer.setSize(0, 0);
}

//==============================================================================
// processBlock — EQ + Compressor (Phase DSP-1), mono-to-stereo expansion
//==============================================================================
void VocalStripAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                             juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);

    const int numSamples = buffer.getNumSamples();
    if (numSamples == 0)
        return;

    // =========================================================================
    // Read EQ parameters (atomic, real-time safe)
    // =========================================================================
    const bool eqHpEnable = parameters.getRawParameterValue("eq_hp_enable")->load() > 0.5f;
    const auto eqLsGain  = parameters.getRawParameterValue("eq_low_shelf_gain")->load();
    const auto eqMidGain = parameters.getRawParameterValue("eq_mid_gain")->load();
    const auto eqMidFreq = parameters.getRawParameterValue("eq_mid_freq")->load();
    const auto eqHsGain  = parameters.getRawParameterValue("eq_high_shelf_gain")->load();
    const bool eqBypass  = parameters.getRawParameterValue("eq_bypass")->load() > 0.5f;

    // Fixed EQ frequencies (not user-controllable)
    constexpr float HP_FREQ   = 100.0f;
    constexpr float LS_FREQ   = 150.0f;
    constexpr float MID_Q     = 1.0f;
    constexpr float HS_FREQ   = 10000.0f;

    // =========================================================================
    // Read Compressor parameters (atomic, real-time safe)
    // =========================================================================
    const auto compAmount  = parameters.getRawParameterValue("comp_amount")->load();
    const auto compAttack  = parameters.getRawParameterValue("comp_attack")->load();
    const auto compRelease = parameters.getRawParameterValue("comp_release")->load();
    const bool compBypass  = parameters.getRawParameterValue("comp_bypass")->load() > 0.5f;

    // =========================================================================
    // EQ Processing (mono, channel 0 only)
    // =========================================================================
    if (!eqBypass)
    {
        // Update filter coefficients (per-block, not per-sample)
        // HP filter: fixed at 100 Hz, only applied when enabled
        if (eqHpEnable)
        {
            *hpFilter.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass(
                currentSampleRate, HP_FREQ);
        }

        *lsFilter.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(
            currentSampleRate, LS_FREQ, 0.7071f,
            juce::Decibels::decibelsToGain(eqLsGain));

        *midFilter.coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
            currentSampleRate, eqMidFreq, MID_Q,
            juce::Decibels::decibelsToGain(eqMidGain));

        *hsFilter.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(
            currentSampleRate, HS_FREQ, 0.7071f,
            juce::Decibels::decibelsToGain(eqHsGain));

        // Process mono channel 0 through filters in series
        auto monoBlock = juce::dsp::AudioBlock<float>(buffer).getSingleChannelBlock(0);
        juce::dsp::ProcessContextReplacing<float> monoContext(monoBlock);

        if (eqHpEnable)
            hpFilter.process(monoContext);
        lsFilter.process(monoContext);
        midFilter.process(monoContext);
        hsFilter.process(monoContext);
    }

    // =========================================================================
    // Compressor Processing (mono, channel 0 only)
    // =========================================================================
    if (!compBypass)
    {
        // Amount-based compressor: threshold, ratio and auto-makeup all derived from Amount
        const float amountNorm  = compAmount / 100.0f;
        const float thresholdDB = -6.0f - (amountNorm * 34.0f);    // -6 to -40 dB
        const float ratio       = 2.0f + (amountNorm * 8.0f);      // 2:1 to 10:1
        // Auto gain compensation: ~50% of the steady-state gain reduction
        const float autoMakeupDB = std::abs(thresholdDB) * (1.0f - 1.0f / ratio) * 0.5f;

        compressor.setThreshold(thresholdDB);
        compressor.setRatio(ratio);
        compressor.setAttack(compAttack);
        compressor.setRelease(compRelease);

        // Measure input RMS before compression (mono ch0)
        const float inputRMS = buffer.getRMSLevel(0, 0, numSamples);

        // Process compressor on mono channel 0
        auto monoBlock = juce::dsp::AudioBlock<float>(buffer).getSingleChannelBlock(0);
        juce::dsp::ProcessContextReplacing<float> monoContext(monoBlock);
        compressor.process(monoContext);

        // Measure output RMS after compression (before makeup)
        const float outputRMSPreMakeup = buffer.getRMSLevel(0, 0, numSamples);

        // Apply auto makeup gain
        const float makeupGain = juce::Decibels::decibelsToGain(autoMakeupDB);
        if (makeupGain != 1.0f)
        {
            auto* ch0 = buffer.getWritePointer(0);
            juce::FloatVectorOperations::multiply(ch0, makeupGain, numSamples);
        }

        // Compute gain reduction in dB from RMS ratio (pre-makeup for accurate GR reading)
        float grDB = 0.0f;
        if (inputRMS > 1.0e-6f && outputRMSPreMakeup > 1.0e-6f)
        {
            grDB = 20.0f * std::log10(outputRMSPreMakeup / inputRMS);
            grDB = juce::jlimit(-30.0f, 0.0f, grDB);
        }
        grMeterValue.store(grDB);
    }
    else
    {
        // Compressor bypassed: report 0 dB gain reduction
        grMeterValue.store(0.0f);
    }

    // =========================================================================
    // Mono-to-Stereo Expansion: copy channel 0 to channel 1
    // =========================================================================
    if (buffer.getNumChannels() >= 2)
    {
        buffer.copyFrom(1, 0, buffer, 0, 0, numSamples);
    }

    // =========================================================================
    // Phase DSP-2: Delay Section (mono processing on ch0)
    // =========================================================================

    // Read delay parameters (atomic, real-time safe)
    const auto delayTimeMs  = parameters.getRawParameterValue("delay_time")->load();
    const auto delayFeedback = parameters.getRawParameterValue("delay_feedback")->load();
    const auto delayMix     = parameters.getRawParameterValue("delay_mix")->load();
    const bool delayBypass  = parameters.getRawParameterValue("delay_bypass")->load() > 0.5f;

    // Ensure reverbSendBuffer is large enough for this block (no alloc if already sized)
    if (reverbSendBuffer.getNumSamples() < numSamples)
        reverbSendBuffer.setSize(1, numSamples, false, false, true);

    if (!delayBypass)
    {
        // Slapback mode only — direct ms value, clamped to safe range
        const float delayMs = juce::jlimit(1.0f, 250.0f, delayTimeMs);
        const float delaySamples = delayMs * static_cast<float>(currentSampleRate) / 1000.0f;
        delayLine.setDelay(delaySamples);

        delayDryWet.setWetMixProportion(delayMix / 100.0f);

        auto monoBlock = juce::dsp::AudioBlock<float>(buffer).getSingleChannelBlock(0);
        delayDryWet.pushDrySamples(monoBlock);

        const float feedbackGain = delayFeedback / 100.0f;
        auto* ch0 = buffer.getWritePointer(0);

        for (int n = 0; n < numSamples; ++n)
        {
            const float inputWithFeedback = ch0[n] + feedbackSample;
            delayLine.pushSample(0, inputWithFeedback);
            const float wetSample = delayLine.popSample(0, delaySamples);
            feedbackSample = wetSample * feedbackGain;
            ch0[n] = wetSample;
        }

        auto wetBlock = juce::dsp::AudioBlock<float>(buffer).getSingleChannelBlock(0);
        delayDryWet.mixWetSamples(wetBlock);

        if (buffer.getNumChannels() >= 2)
            buffer.copyFrom(1, 0, buffer, 0, 0, numSamples);
    }

    // Reverb send always 0 — clear buffer regardless of delay bypass state
    reverbSendBuffer.clear(0, 0, numSamples);

    // =========================================================================
    // Phase DSP-3: Reverb Section + Full Chain Integration
    // =========================================================================

    // Read reverb parameters (atomic, real-time safe)
    const int  reverbType     = static_cast<int>(parameters.getRawParameterValue("reverb_type")->load());
    const auto reverbPredelay = parameters.getRawParameterValue("reverb_predelay")->load();
    const auto reverbDecay    = parameters.getRawParameterValue("reverb_decay")->load();
    const auto reverbDamping  = parameters.getRawParameterValue("reverb_damping")->load();
    const auto reverbMix      = parameters.getRawParameterValue("reverb_mix")->load();
    const bool reverbBypass   = parameters.getRawParameterValue("reverb_bypass")->load() > 0.5f;

    if (!reverbBypass)
    {
        // Step 1: Capture dry stereo signal BEFORE reverb processing for DryWetMixer
        auto stereoBlock = juce::dsp::AudioBlock<float>(buffer);
        reverbDryWet.setWetMixProportion(reverbMix / 100.0f);
        reverbDryWet.pushDrySamples(stereoBlock);

        // Step 2: Add delay-to-reverb send signal into both L+R channels
        // reverbSendBuffer is mono (1 channel), add to both stereo channels
        const float* sendData = reverbSendBuffer.getReadPointer(0);
        float* ch0 = buffer.getWritePointer(0);
        float* ch1 = buffer.getWritePointer(1);
        for (int n = 0; n < numSamples; ++n)
        {
            ch0[n] += sendData[n];
            ch1[n] += sendData[n];
        }

        // Step 3: Pre-delay (stereo, independent L+R delay lines, same input)
        const float preDelaySamples = reverbPredelay * static_cast<float>(currentSampleRate) / 1000.0f;
        const float clampedPreDelay = juce::jlimit(0.0f, 19999.0f, preDelaySamples);

        preDelayL.setDelay(clampedPreDelay);
        preDelayR.setDelay(clampedPreDelay);

        for (int n = 0; n < numSamples; ++n)
        {
            preDelayL.pushSample(0, ch0[n]);
            ch0[n] = preDelayL.popSample(0, clampedPreDelay);

            preDelayR.pushSample(0, ch1[n]);
            ch1[n] = preDelayR.popSample(0, clampedPreDelay);
        }

        // Step 4: Set reverb parameters based on type (Plate or Room)
        juce::dsp::Reverb::Parameters reverbParams;
        const float decayNorm = (reverbDecay - 0.3f) / (8.0f - 0.3f);  // 0.0 - 1.0

        if (reverbType == 0)  // Plate
        {
            reverbParams.roomSize = 0.4f + decayNorm * 0.55f;   // 0.40 - 0.95
            reverbParams.width    = 1.0f;                         // Full stereo spread
        }
        else  // Room
        {
            reverbParams.roomSize = 0.1f + decayNorm * 0.50f;   // 0.10 - 0.60
            reverbParams.width    = 0.7f;                         // Slightly narrower
        }
        reverbParams.damping    = reverbDamping / 100.0f;
        reverbParams.wetLevel   = 1.0f;   // Wet/dry handled by DryWetMixer
        reverbParams.dryLevel   = 0.0f;
        reverbParams.freezeMode = false;
        reverb.setParameters(reverbParams);

        // Step 5: Process reverb (CRITICAL pattern #17: process(context) NOT processMono/processStereo)
        juce::dsp::AudioBlock<float> reverbBlock(buffer);
        juce::dsp::ProcessContextReplacing<float> reverbContext(reverbBlock);
        reverb.process(reverbContext);

        // Step 6: Apply reverb dry/wet mix
        auto stereoOutBlock = juce::dsp::AudioBlock<float>(buffer);
        reverbDryWet.mixWetSamples(stereoOutBlock);
    }
    // If reverbBypass: stereo signal from delay section passes through unchanged
}

//==============================================================================
// Editor
//==============================================================================
juce::AudioProcessorEditor* VocalStripAudioProcessor::createEditor()
{
    return new VocalStripAudioProcessorEditor(*this);
}

//==============================================================================
// State management — save/restore all 30 APVTS parameters
//==============================================================================
void VocalStripAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void VocalStripAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState != nullptr && xmlState->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// Factory function
//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VocalStripAudioProcessor();
}
