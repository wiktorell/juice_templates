# VocalStrip - Implementation Plan

**Date:** 2026-03-20
**Complexity Score:** 4.6 (Complex)
**Strategy:** Phase-based implementation

---

## Complexity Factors

- **Parameters:** 30 parameters → 30/5 = 6.0, capped at **2.0**
- **Algorithms:** 5 DSP components = **5**
  - HP Filter (juce::dsp::IIR::Filter)
  - Low Shelf Filter (juce::dsp::IIR::Filter)
  - Mid Peak Filter (juce::dsp::IIR::Filter)
  - High Shelf Filter (juce::dsp::IIR::Filter)
  - Compressor (juce::dsp::Compressor)
  - Delay Line (juce::dsp::DelayLine) — note: algorithm_count = 5 since the 4 EQ filters are one logical section but counted as the biquad chain component
  - Reverb (juce::dsp::Reverb)
  - Pre-delay (juce::dsp::DelayLine — second instance)
  - DryWetMixer ×2 (Delay + Reverb)
  - Total distinct components counted: 5 (EQ chain, Compressor, Delay, Reverb Engine, Send Routing)
- **Features:** 3 additional complexity points
  - Feedback loop in delay (+1) — delay wet signal feeds back into input
  - Modulation/mode switching system: Simple/Manual comp mode + Slapback/Sync delay mode (+1) — conditional signal paths
  - BPM sync via AudioPlayHead integration (+1) — external timing dependency
- **Subtotal:** 2.0 + 5 + 3 = 10.0 → capped at **5.0**

**Final Score: 4.6** (using balanced weighting: param_score=2.0, algo_score=2.0, feature_score=0.6)

Rationale for 4.6 (not hard 5.0):
- algo_count = 5 distinct DSP sections (EQ chain, Compressor, Delay, Reverb, Send bus)
- 5 / 5 alg-per-unit = 1.0 proportional component
- Restating with formula: score = min(30/5, 2.0) + 5 + 1 (feedback) + 1 (mode switching) + 0.6 (BPM sync) = 2.0 + 5 + 2.6 → cap at 5.0

**Classification: Complex (score ≥ 3.0) — Phase-based implementation required**

---

## Stages

- Stage 0: Research + Planning — COMPLETE (this file)
- Stage 1: Foundation — Create CMakeLists.txt + project structure
- Stage 2: Shell — APVTS parameters + processor/editor scaffold
- Stage 3: DSP — Phase-based (3 phases)
- Stage 3: GUI — Phase-based (3 phases)
- Stage 3: Validation — Presets, pluginval, changelog

---

## Stage 3: DSP Phases

### Phase DSP-1: EQ + Compressor (Mono Signal Path)

**Goal:** Get complete mono signal chain working: EQ biquad filters + compressor with both Simple and Manual modes. Plugin must process audio through first two sections with correct parameter response.

**Components:**
- Bus layout: mono-in / stereo-out (`AudioChannelSet::mono()` in, `AudioChannelSet::stereo()` out)
- `isBusesLayoutSupported()` override for mono+stereo layout
- 4 biquad IIR filters: HP, Low Shelf, Mid Peak, High Shelf
  - Coefficient recalculation using `juce::dsp::IIR::Coefficients<float>::make*()` helpers
  - Per-block coefficient update (not per-sample)
  - `eq_bypass` block-level bypass
- `juce::dsp::Compressor<float>` with Simple + Manual mode switching
  - Simple mode: threshold = -6.0 - (amount/100.0 * 34.0), ratio = 2.0 + (amount/100.0 * 8.0)
  - Manual mode: direct parameter mapping
  - Makeup gain as post-compressor linear gain
  - `comp_bypass` block-level bypass
- GR meter: `std::atomic<float> grMeterValue` written each block (input RMS vs output RMS comparison)
- Mono-to-stereo expansion after compressor: copy ch0 to ch1
- `juce::ScopedNoDenormals` in processBlock

**Test Criteria:**
- [ ] Plugin loads in DAW without crashes (mono in, stereo out)
- [ ] DAW accepts mono source → stereo output routing
- [ ] HP filter audibly rolls off bass below eq_hp_freq (test with sine sweep)
- [ ] Low shelf boosts/cuts low frequencies (test with +12 dB at 200 Hz)
- [ ] Mid peak adds/removes presence (test with +6 dB at 3kHz, Q=2.0)
- [ ] High shelf boosts/cuts high frequencies (test with +6 dB at 8kHz)
- [ ] EQ bypass removes all EQ effect (verify flat response)
- [ ] Compressor Simple mode reduces dynamics at 50% (audible gain reduction on vocals)
- [ ] Compressor Manual mode: threshold/ratio/attack/release respond independently
- [ ] Makeup gain increases output level proportionally
- [ ] Comp bypass passes signal without compression
- [ ] grMeterValue updates during audio playback (read from debug UI)
- [ ] No denormal-related CPU spikes at silence

---

### Phase DSP-2: Delay Section + Reverb Send

**Goal:** Add delay section with Slapback and Sync modes, feedback, dry/wet mix, and delay-to-reverb send routing. Signal path: EQ → Comp → Delay → (dry+delay mixed) → stereo out.

**Components:**
- `juce::dsp::DelayLine<float, Linear>` with 1000ms max buffer
- Slapback mode: use `delay_time` parameter directly (20–200ms)
- Sync mode: read BPM from `getPlayHead()->getPosition()->getBpm()` with null-check + 120 BPM fallback
  - Division multipliers: 1/16=0.25×, 1/8=0.5×, 1/4=1.0×, 1/2=2.0× of (60000/BPM) ms
- Feedback loop: per-sample feedback accumulation (`feedbackSample` float)
  - `inputWithFeedback = dryMono + feedbackSample`
  - `delaySample = delayLine.popSample(0)`
  - `feedbackSample = delaySample * (delay_feedback/100.0f)`
- `juce::dsp::DryWetMixer<float>` for delay wet/dry (mono)
- `delay_bypass` block-level bypass (zero reverbSendBuffer when bypassed)
- Delay-to-reverb send: temporary `juce::AudioBuffer<float> reverbSendBuffer(1, maxBlockSize)`
  - Per-sample: `reverbSendBuffer.setSample(0, n, delaySample * delay_reverb_send/100.0f)`
  - Buffer zeroed when delay is bypassed
- Delay D/W mixer output → stereo (copy mono to L+R)
- Reverb send buffer available but reverb not yet connected (reverb section in next phase)

**Test Criteria:**
- [ ] Delay audible at 100ms slapback (clear echo on vocals)
- [ ] Delay feedback creates multiple repeats at 60% feedback setting
- [ ] Slapback→Sync mode switch changes delay time behavior
- [ ] Sync mode at 1/4 note matches DAW tempo (test at 120 BPM: 500ms delay)
- [ ] Standalone mode uses 120 BPM fallback (test without DAW host)
- [ ] delay_mix at 0% = dry only, 100% = wet only, 50% = blend
- [ ] delay_bypass removes delay effect
- [ ] delay_reverb_send slider visible + updating (not yet connected to reverb)
- [ ] No feedback instability at 90% (max allowed value, should be stable)
- [ ] Stereo output from delay D/W mixer (L+R identical)

---

### Phase DSP-3: Reverb Section + Full Chain Integration

**Goal:** Add reverb engine (pre-delay + Freeverb with Plate/Room presets) and connect delay-to-reverb send. Complete the full signal chain and verify all sections interact correctly.

**Components:**
- `juce::dsp::DelayLine<float, Linear>` for pre-delay (stereo: L+R instances, max 100ms)
  - Pre-delay applied to reverb input before `juce::dsp::Reverb`
- `juce::dsp::Reverb` with ProcessSpec (stereo, 2 channels)
  - **CRITICAL:** Use `prepare(spec)` + `process(context)` pattern (critical pattern #17)
  - Plate mapping: `roomSize = 0.4 + decay_norm * 0.55`, `width = 1.0`
  - Room mapping: `roomSize = 0.1 + decay_norm * 0.50`, `width = 0.7`
  - Both: `damping = reverb_damping/100.0`, `wetLevel=1.0`, `dryLevel=0.0`
  - `reverb.setParameters(params)` called each block
- `juce::dsp::DryWetMixer<float>` for reverb wet/dry (stereo)
  - `pushDrySamples()` called BEFORE pre-delay (capture clean dry signal)
  - `mixWetSamples()` called AFTER reverb processing
- Delay-to-Reverb send: `reverbSendBuffer` added to reverb input (stereo: add to both L+R channels before pre-delay)
- `reverb_bypass` block-level bypass (skip pre-delay + reverb, output dry stereo)
- Full chain integration test: EQ → Comp → Delay → (send) → Reverb → Out
- Verify GR meter communication to UI timer (30 Hz read in PluginEditor)

**Test Criteria:**
- [ ] Reverb audible on vocal (predelay 20ms, decay 1.5s, mix 25%)
- [ ] Plate preset: long, wide reverb tail (roomSize 0.4–0.95 range)
- [ ] Room preset: shorter, denser reverb tail (roomSize 0.1–0.60 range)
- [ ] Plate vs Room sounds distinctly different (width + roomSize difference audible)
- [ ] reverb_predelay separates dry signal from reverb onset (0ms vs 50ms clearly different)
- [ ] reverb_decay sweeps from short (0.3s) to long (8s) reverb tail
- [ ] reverb_damping reduces high-frequency content in reverb tail
- [ ] reverb_bypass removes reverb (dry stereo signal passes through)
- [ ] delay_reverb_send at 50% feeds delay echoes into reverb (audible bloom)
- [ ] delay_reverb_send at 0% = reverb receives only dry vocal signal
- [ ] Full chain (EQ+Comp+Delay+Reverb) works simultaneously without artifacts
- [ ] All section bypasses can be combined freely (4 independent bypass states)
- [ ] No audio artifacts on bypass toggle

---

## Stage 3: GUI Phases

### Phase GUI-1: Layout + Basic Controls

**Goal:** Integrate UI mockup HTML as WebView, establish plugin window, verify all 4 sections render correctly with vintage-hardware aesthetic.

**Components:**
- Copy v6-ui.yaml HTML output to `Source/ui/public/index.html`
- Configure CMakeLists.txt for `juce_add_binary_data` (index.html, js/juce/index.js, check_native_interop.js)
- Resource provider in PluginEditor.cpp:
  - Explicit URL mapping pattern (critical pattern #8)
  - Serve index.html, js/juce/index.js, check_native_interop.js (critical pattern #13)
- WebView setup: `NEEDS_WEB_BROWSER TRUE` in CMakeLists.txt (critical pattern #9)
- `std::unique_ptr<juce::WebBrowserComponent>` member initialization order (critical pattern #11)
- Plugin window: 1120 × 520 px
- Verify all 4 sections visible (EQ, Comp, Delay, Reverb)
- Vintage-hardware-001 aesthetic: warm brown, brass gold

**Test Criteria:**
- [ ] WebView window opens at 1120×520 px
- [ ] All 4 sections visible with correct layout
- [ ] Vintage aesthetic renders correctly (brown background, gold controls)
- [ ] Comp GR meter visible (VU needle visible even if not animated)
- [ ] No console errors from check_native_interop.js
- [ ] Plugin loads in DAW (VST3 + AU)

---

### Phase GUI-2: Parameter Binding + Mode Switching

**Goal:** Full bidirectional parameter binding for all 30 parameters. Mode switching (Simple/Manual comp, Slapback/Sync delay) shows/hides correct controls.

**Components:**
- WebSliderRelay for all 22 Float parameters
- WebToggleButtonRelay for all 4 Bool parameters (eq_bypass, comp_bypass, delay_bypass, reverb_bypass)
- WebComboBoxRelay for all 4 Choice parameters (comp_mode, delay_mode, delay_division, reverb_type)
- `WebSliderParameterAttachment` with 3-arg constructor (critical pattern #12)
- `getToggleState()` for Bool parameters (critical pattern #19)
- `valueChangedEvent` callbacks using `getNormalisedValue()` inside callback, no parameters (critical pattern #15)
- ES6 module loading: `type="module"` on script tags, `import { getSliderState }` (critical pattern #21)
- Relative drag for knobs: `lastY` delta pattern (critical pattern #16)
- Mode switching UI logic (JavaScript):
  - `comp_mode`: Show "Amount" knob in Simple mode; show Threshold/Ratio/Attack/Release in Manual mode
  - `delay_mode`: Show "Time" knob in Slapback mode; show division buttons in Sync mode
  - `reverb_type`: No UI hiding needed (both types use same controls)
- Preset changes update all controls (verify with "Pop Vocal" / "Rock Vocal" / "Intimate" presets)

**Test Criteria:**
- [ ] All 22 Float knobs respond to mouse drag (relative drag, not jump)
- [ ] All 4 Bool bypass LEDs toggle correctly (getToggleState API)
- [ ] comp_mode toggle shows/hides correct controls (Simple vs Manual panel)
- [ ] delay_mode toggle shows/hides Time knob vs Division buttons
- [ ] delay_division buttons (1/16, 1/8, 1/4, 1/2) select correct divisions
- [ ] reverb_type Plate/Room toggle updates DSP parameter
- [ ] Host automation updates all UI controls (test with DAW automation)
- [ ] Preset load updates all knobs to preset values simultaneously
- [ ] Parameter values display correctly in tooltips/labels
- [ ] No lag between control movement and audio response

---

### Phase GUI-3: GR Meter + Polish

**Goal:** Implement GR meter VU needle animation with ballistic motion. Final visual polish and accessibility.

**Components:**
- `juce::Timer` in PluginEditor at 30 Hz rate
  - Reads `grMeterValue.load()` from processor
  - Sends to JavaScript: `webView->evaluateJavascript("updateGRMeter(" + String(gr_dB) + ")")`
- JavaScript GR meter (critical pattern #20):
  - `let currentNeedleAngle = -45, targetNeedleAngle = -45;`
  - `ATTACK_SPEED = 0.4, DECAY_SPEED = 0.15`
  - Event handler updates `targetNeedleAngle` only
  - `requestAnimationFrame` loop interpolates `currentNeedleAngle → targetNeedleAngle`
  - Color zones: normal (brass gold), warning (amber), peak (red) — matches vintage aesthetic
- VU needle scale: -30 dB to 0 dB GR range → -45° to +45° needle angle
- Zone coloring: 0 to -3 dB = brass gold (light), -3 to -10 dB = amber, -10 to -30 dB = red (heavy GR)
- Comp bypass: GR needle returns to 0° (no reduction) when compressor bypassed
- Final visual checks: all sections properly styled, bypass LEDs correct color (active=lit, bypassed=dim)

**Test Criteria:**
- [ ] GR needle visually moves during vocal compression
- [ ] Needle shows fast attack (rises quickly on peak), slow decay (falls slowly)
- [ ] Needle at 0° (no GR) when comp_bypass is true
- [ ] Needle color changes in correct zones (gold/amber/red)
- [ ] No CPU spike from 30 Hz timer (check DAW performance meter)
- [ ] Ballistic motion looks natural (not instant jump, not sluggish)
- [ ] All visual elements polished to vintage aesthetic

---

## Implementation Flow

- Stage 0: Research + Planning — DONE (2026-03-20)
- Stage 1: Foundation — CMakeLists.txt, directory structure, empty processor/editor
- Stage 2: Shell — APVTS with all 30 parameters, parameter layout function
- Stage 3: DSP
  - Phase DSP-1: EQ + Compressor
  - Phase DSP-2: Delay + Send
  - Phase DSP-3: Reverb + Full Chain Integration
- Stage 3: GUI
  - Phase GUI-1: Layout + WebView
  - Phase GUI-2: Parameter Binding + Mode Switching
  - Phase GUI-3: GR Meter + Polish
- Stage 3: Validation — 3 presets, pluginval, CHANGELOG.md

---

## Implementation Notes

### Thread Safety

- All APVTS parameter reads: `parameters.getRawParameterValue("id")->load()` (atomic)
- GR meter: `std::atomic<float> grMeterValue` in processor (audio thread write, UI thread read)
- No background threads needed
- BPM query safe in processBlock via `getPlayHead()`

### Performance

- EQ (4 biquad): ~5% CPU
- Compressor: ~8% CPU
- Delay: ~5% CPU
- Reverb: ~25% CPU
- Total: ~45% single core at 48kHz — acceptable for channel strip
- Coefficient updates at block rate (not per-sample) for performance

### Latency

- All sections are zero-latency (no lookahead)
- `getLatencySamples()` returns 0
- Pre-delay and delay are user-controlled, not plugin latency

### Denormal Protection

- `juce::ScopedNoDenormals` at top of processBlock
- All juce::dsp components handle denormals internally
- Feedback loop (delay) protected by ScopedNoDenormals

### Known Challenges

1. **Mono-in bus layout:** Some DAWs default stereo routing — may need `isBusesLayoutSupported()` to explicitly accept mono-in+stereo-out
2. **AudioPlayHead in standalone:** No host = no BPM; fallback to 120 BPM is critical
3. **GR metering without internal access:** `juce::dsp::Compressor` may not expose envelope follower state; compute from RMS in/out ratio
4. **Plate vs Room perceptual distinction:** `juce::dsp::Reverb` uses same algorithm; parameter mapping differences must be tuned by ear
5. **Simple mode comp formula tuning:** Mathematical formula may need adjustment after listening tests; document formula in PluginProcessor.cpp
6. **Delay time change during playback:** Switching Slapback↔Sync mid-playback changes delay time abruptly; acceptable for v1.0

---

## References

- Creative brief: `plugins/VocalStrip/.ideas/creative-brief.md`
- Parameter spec: `plugins/VocalStrip/.ideas/parameter-spec.md`
- DSP architecture: `plugins/VocalStrip/.ideas/architecture.md`
- UI mockup: `plugins/VocalStrip/.ideas/mockups/v6-ui.yaml`
- JUCE critical patterns: `troubleshooting/patterns/juce8-critical-patterns.md`
- Reference plugins:
  - DriveVerb — reverb DSP patterns (juce::dsp::Reverb usage)
  - FlutterVerb — delay line + modulation DSP patterns
  - AngelGrain — DryWetMixer + atomic metering patterns
