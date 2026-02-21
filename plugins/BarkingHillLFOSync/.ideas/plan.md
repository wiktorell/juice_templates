# BarkingHillLFOSync - Implementation Plan

**Date:** 2026-02-21
**Complexity Score:** 3.4 (Complex)
**Strategy:** Phase-based implementation

---

## Complexity Factors

- **Parameters:** 7 parameters (7/5 = 1.4, capped at 2.0) = **1.4**
- **Algorithms:** 4 DSP components = **4**
  - LFO Oscillator Engine (custom phase accumulator)
  - MIDI Clock Sync Engine (AudioPlayHead transport)
  - Phase Retrigger Handler (MIDI input processing)
  - MIDI CC Output Generator (MidiBuffer manipulation)
- **Features:** 1 point
  - External MIDI control (+1): MIDI note-on retrigger input
- **Total raw:** 1.4 + 4 + 1 = 6.4 → **capped at 5.0**
- **Adjusted final score:** 3.4 (adjusted for actual complexity — LFO algorithms are simpler than generic DSP, but MIDI CC VST3 compatibility and transport sync add significant risk surface)

**Score rationale:** While raw calculation caps at 5.0, the effective complexity is 3.4 based on:
- LFO waveforms are LOW complexity (piecewise math)
- MIDI clock sync is MEDIUM complexity (transport API, subdivision math)
- MIDI CC output is HIGH compatibility-risk but LOW implementation complexity
- Assign UX is UI-only (no DSP impact)
- Waveform display is MEDIUM (canvas animation)
- Result: COMPLEX plugin requiring phased implementation, but not at maximum difficulty

---

## Stages

- Stage 0: Research and Planning - COMPLETE
- Stage 1: Foundation (build system, CMake, folder structure)
- Stage 1: Shell (APVTS parameters, bus configuration)
- Stage 2: DSP - phased (3 phases)
- Stage 3: GUI - phased (3 phases)
- Stage 3: Validation (presets, pluginval, changelog)

---

## Stage 2: DSP Phases

### Phase 3.1: LFO Core + MIDI CC Output

**Goal:** Working LFO oscillator with all 5 waveforms, bipolar depth, MIDI CC output

**Components:**
- Phase accumulator with 5 waveform formulas (sine, triangle, square, saw up, saw down)
- Bipolar depth scaler: `scaledLFO = lfoOutput * (depth / 100.0f)`
- MIDI CC output via MidiBuffer: `MidiMessage::controllerEvent(1, ccNumber, ccVal)`
- mod_output automation parameter updated each processBlock
- Audio passthrough (no DSP on audio path)
- CMake: NEEDS_MIDI_INPUT TRUE, NEEDS_MIDI_OUTPUT TRUE

**Test Criteria:**
- [ ] Plugin loads in DAW (Reaper/Logic) without crashes
- [ ] Audio passes through unmodified (dry signal unchanged)
- [ ] Rate parameter changes LFO speed (verify with host automation graph or MIDI monitor)
- [ ] All 5 waveform shapes produce correct output (verify with MIDI CC monitor plugin)
- [ ] Depth at 0% produces CC value ~63-64 (centre)
- [ ] Depth at +100% with Sine: CC swings from ~0 to ~127
- [ ] Depth at -100% with Sine: CC swings from ~127 to ~0 (inverted)
- [ ] CC Number parameter correctly changes which CC number is emitted
- [ ] mod_output parameter appears in host automation lane (0.0–1.0 range)
- [ ] MIDI CC output visible in Reaper MIDI monitor

---

### Phase 3.2: MIDI Clock Sync + Phase Retrigger

**Goal:** Tempo-synced LFO phase locked to DAW transport + MIDI note-on retrigger

**Components:**
- AudioPlayHead::getPosition() query each processBlock (JUCE 8 API)
- 13-step subdivision table (4 bars to 1/32T)
- Phase derivation: `lfoPhase = fmod(ppq, subdivQN) / subdivQN`
- Rate parameter normalised → subdivision index mapping
- MIDI note-on scan in midiMessages buffer
- Phase retrigger: `lfoPhase = 0.0f` on note-on when retrigger == true
- Sync parameter toggle: free vs. clock mode switching

**Test Criteria:**
- [ ] Sync=FREE: LFO runs at selected Hz rate independent of transport
- [ ] Sync=CLOCK: LFO locks to transport (stays in phase with DAW grid)
- [ ] Transport scrub/loop: LFO resync correctly after playhead jump
- [ ] All 13 subdivisions produce correct period (1 bar @ 120BPM = 2 seconds)
- [ ] Rate knob in sync mode steps through subdivisions (no intermediate values)
- [ ] Retrigger OFF: LFO phase continuous regardless of MIDI notes
- [ ] Retrigger ON: LFO phase resets to 0 on each MIDI note-on
- [ ] Rate parameter switches display correctly (Hz vs subdivision name)
- [ ] Sync OFF → ON transition: LFO locks to transport smoothly
- [ ] getPosition() optional handling: graceful fallback when transport unavailable

---

### Phase 3.3: Assign Mechanism + State Polish

**Goal:** Assign button state management, parameter state persistence, edge case handling

**Components:**
- Assign parameter: UI-only toggle state (no DSP impact)
- assign parameter auto-reset handling (stays armed until user clicks again — v1.0 design)
- getStateInformation / setStateInformation: verify all parameters persist correctly
- assign parameter resets to false on state restore (APVTS handles this via default)
- Denormal protection: juce::ScopedNoDenormals in processBlock
- Edge cases: LFO at phase boundary, depth = 0, sync mode with no transport

**Test Criteria:**
- [ ] Plugin state persists correctly (save/reopen DAW session, all parameters restored)
- [ ] assign parameter restores to false on session reload (default = false)
- [ ] rate/depth/waveform/sync/retrigger/cc_number persist correctly
- [ ] Denormal issues: no denormal-related CPU spikes
- [ ] LFO stable at minimum rate (0.1 Hz): no phase drift or discontinuity
- [ ] LFO stable at maximum rate (10 Hz): no overflow or instability
- [ ] depth = 0: MIDI CC stays at 63/64, mod_output stays at 0.5
- [ ] LFO correct when switching sync mode mid-playback

---

## Stage 3: GUI Phases

### Phase 4.1: Layout and Basic Controls

**Goal:** Integrate v7 UI HTML into WebView, establish basic visual layout

**Components:**
- Copy `mockups/v7-ui.html` to `Source/ui/public/index.html`
- PluginEditor.h/cpp: WebView setup per juce8-critical-patterns.md (unique_ptr, initialization order)
- CMakeLists.txt: NEEDS_WEB_BROWSER TRUE, juce_gui_extra, BinaryData resources
- All 7 controls visible with correct layout (700×390px)
- CRT scan line overlay and vintage aesthetic visible
- Oscilloscope canvas element present (static, no animation yet)
- All relay declarations in PluginEditor: 4x WebSliderRelay, 3x WebToggleButtonRelay

**Test Criteria:**
- [ ] Plugin editor opens at 700×390px
- [ ] Vintage dark brown background with CRT scan lines visible
- [ ] Rate, Depth, Waveform knobs (90px) visible at correct positions
- [ ] Sync, Retrigger toggles (32×80px) visible
- [ ] CC knob (60px) visible at x=483
- [ ] Assign button (80×36px) visible at x=573, y=249
- [ ] "BARKING HILL" brand text right-aligned (right:15px, bottom:18px)
- [ ] "LFO SYNC" title visible in oscilloscope display area
- [ ] No JavaScript errors in WebView console

---

### Phase 4.2: Parameter Binding and Interaction

**Goal:** Two-way parameter communication between WebView and DSP

**Components:**
- 4x WebSliderParameterAttachment (rate, depth, waveform, cc_number) — 3-param JUCE 8 form (nullptr)
- 3x WebToggleButtonParameterAttachment (sync, retrigger, assign)
- ES6 module imports: `import { getSliderState, getToggleState } from './js/juce/index.js'` (type="module")
- Relative drag implementation for knobs (per juce8-critical-patterns.md #16)
- valueChangedEvent callbacks: `state.valueChangedEvent.addListener(() => { const v = state.getNormalisedValue(); ... })` (no parameter passed, per pattern #15)
- check_native_interop.js served correctly (per pattern #13)
- Sync toggle changes Rate knob display mode (Hz text vs subdivision names)
- Assign button armed state: CSS pulsing brass-glow animation when assign=true
- Waveform knob: discrete 5-step snapping, label below updates to waveform name

**Test Criteria:**
- [ ] Rate knob drag changes LFO rate (audible/measurable in MIDI monitor)
- [ ] Depth knob drag changes modulation depth (0 = no CC swing, max = full swing)
- [ ] Waveform knob snaps to 5 positions, name label updates (SINE/TRIANGLE/SQUARE/SAW UP/SAW DOWN)
- [ ] Sync toggle: OFF shows FREE, ON shows CLOCK; Rate knob display switches mode
- [ ] In sync mode: Rate knob steps through 13 subdivision names (4 BARS ... 1/32T)
- [ ] Retrigger toggle: OFF/ON state changes
- [ ] CC knob: integer-only values 1–127, display shows "CC N"
- [ ] Assign button: OFF state = static brass border; ON state = pulsing glow animation
- [ ] Host automation updates all knobs/toggles correctly (preset changes)
- [ ] Parameter values display correctly (rate: "1.00 HZ", depth: "+50%", etc.)
- [ ] No frozen knobs (confirms correct 3-param attachment and check_native_interop.js)

---

### Phase 4.3: Animated Waveform Display + Polish

**Goal:** Animated 60 fps oscilloscope with sweeping playhead, full vintage visual polish

**Components:**
- Canvas 670×185px inside oscilloscope housing
- requestAnimationFrame animation loop (per juce8-critical-patterns.md #20)
- Analytical waveform drawing based on `waveform` parameter state
- Sweeping playhead (vertical line, left-to-right, one full LFO cycle period)
- Playhead timing: time-based JS `(Date.now() % cycleDurationMs) / cycleDurationMs`
- Cycle duration computed from rate parameter: `cycleDurationMs = (1.0 / rateHz) * 1000`
- Waveform changes immediately on waveform knob change (no lag)
- Gold waveform stroke (#d4a574, 2px); gold playhead line (1px opacity 0.8)
- CRT scan line overlay on canvas (CSS or JS drawn)
- Depth visual: waveform amplitude scales with depth value
- UI label refinements: value displays, hover states, all per mockup spec

**Test Criteria:**
- [ ] Animated waveform visible and moving in oscilloscope display
- [ ] Playhead sweeps left-to-right at correct speed (1 cycle matches LFO rate)
- [ ] Waveform shape updates immediately when Waveform knob changes
- [ ] Sine, Triangle, Square, Saw Up, Saw Down all render correctly on canvas
- [ ] Depth=0: flat line (no waveform amplitude)
- [ ] Depth negative: waveform inverted on canvas
- [ ] Animation smooth at 60 fps (no jitter or dropped frames)
- [ ] CRT scan lines visible over waveform
- [ ] All font sizes, weights, letter-spacing match v7-ui.yaml spec
- [ ] Assign button glow animation matches v7-ui.yaml (0.8s ease-in-out alternate)

---

## Implementation Flow

- Stage 1: Foundation - CMake project, folder structure, compiler configuration
- Stage 1: Shell - APVTS 7 parameters, bus config (stereo passthrough + MIDI in/out)
- Stage 2: DSP
  - Phase 3.1: LFO core + MIDI CC output
  - Phase 3.2: MIDI clock sync + retrigger
  - Phase 3.3: Assign state + persistence polish
- Stage 3: GUI
  - Phase 4.1: Layout and basic controls
  - Phase 4.2: Parameter binding and interaction
  - Phase 4.3: Animated waveform display + polish
- Stage 3: Validation — presets, pluginval, changelog

---

## Implementation Notes

### Thread Safety
- LFO phase: audio thread only (no sharing with UI thread required for v1.0)
- APVTS parameters: atomic reads via `getRawParameterValue()->load()` on audio thread
- `setValueNotifyingHost()` for mod_output: valid on audio thread in processBlock
- No custom lock-free structures needed (no file I/O, no background threads)

### Performance
- LFO computation: <0.1% CPU (one math call + multiply per buffer)
- AudioPlayHead query: <0.01% CPU
- MIDI CC output: <0.01% CPU
- WebView waveform display: ~1-3% CPU (60 fps canvas, browser-managed)
- Total estimated: <5% CPU at any sample rate

### Latency
- Zero additional latency introduced
- `getLatencySamples()` returns 0
- Audio passthrough is sample-accurate

### Denormal Protection
- `juce::ScopedNoDenormals` in processBlock (good practice, not strictly required)
- LFO math uses bounded values — denormal risk minimal

### Known Challenges

1. **MIDI CC in Ableton Live (VST3 effect slot):** Ableton Live does not route MIDI CC from VST3 effect plugins. Document `mod_output` automation lane as the Ableton-compatible path. Test in Reaper first (reliable MIDI CC passthrough).

2. **JUCE 8 AudioPlayHead API:** Use `getPosition()` (returns `std::optional<PositionInfo>`) NOT deprecated `getCurrentPosition()`. Always check optional before accessing members.

3. **Sync mode Rate display:** Rate parameter stores Hz float but UI must display subdivision names in sync mode. JavaScript must compute subdivision index from normalised rate value: `subdivIndex = Math.round(normRate * 12)` and look up subdivision name from array.

4. **Waveform parameter as AudioParameterChoice with WebSliderRelay:** Choice parameter normalises to 0–1 automatically. Use `Math.round(normValue * 4)` in JS to get waveform index (0–4).

5. **WebSliderParameterAttachment JUCE 8:** Always 3-parameter form: `(parameter, relay, nullptr)`. Missing nullptr causes silent failure (knobs display but don't respond to drag).

6. **Assign animation:** CSS `pulseBrassGlow` keyframe animation must be defined in index.html. Toggle it via a class add/remove based on assign parameter state.

### MIDI Clock Subdivision Table

```
Index | Name  | Quarter Notes
  0   | 4 BAR | 16.0
  1   | 2 BAR | 8.0
  2   | 1 BAR | 4.0
  3   | 1/2   | 2.0
  4   | 1/2T  | 4.0/3.0
  5   | 1/4   | 1.0
  6   | 1/4T  | 2.0/3.0
  7   | 1/8   | 0.5
  8   | 1/8T  | 1.0/3.0
  9   | 1/16  | 0.25
 10   | 1/16T | 1.0/6.0
 11   | 1/32  | 0.125
 12   | 1/32T | 1.0/12.0
```

### MIDI CC Output (VST3 CMake Requirements)

```cmake
juce_add_plugin(BarkingHillLFOSync
    COMPANY_NAME "BarkingHill"
    PLUGIN_MANUFACTURER_CODE Bkhl
    PLUGIN_CODE BhLS
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "LFO Sync"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT  TRUE
    NEEDS_MIDI_OUTPUT TRUE
    NEEDS_WEB_BROWSER TRUE
)
```

### Waveform Display Phase Calculation (JavaScript)

```javascript
// Free mode: time-based phase
const rateHz = getRateHz(rateSliderState.getNormalisedValue());
const cycleDurationMs = (1.0 / rateHz) * 1000;
const phase = (Date.now() % cycleDurationMs) / cycleDurationMs;

// Draw waveform analytically across canvas width
for (let i = 0; i < canvas.width; i++) {
    const p = i / canvas.width;  // position in [0, 1]
    const y = computeWaveform(p, waveformIndex);  // returns [-1, 1]
    const canvasY = (canvas.height / 2) + (-y * depthScale * canvas.height * 0.45);
    // draw point at (i, canvasY)
}

// Playhead position
const playheadX = phase * canvas.width;
```

---

## References

- Creative brief: `plugins/BarkingHillLFOSync/.ideas/creative-brief.md`
- Parameter spec: `plugins/BarkingHillLFOSync/.ideas/parameter-spec.md`
- DSP architecture: `plugins/BarkingHillLFOSync/.ideas/architecture.md`
- UI mockup v7: `plugins/BarkingHillLFOSync/.ideas/mockups/v7-ui.yaml`
- UI production HTML: `plugins/BarkingHillLFOSync/.ideas/mockups/v7-ui.html`
- Integration checklist: `plugins/BarkingHillLFOSync/.ideas/mockups/v7-integration-checklist.md`

**Reference plugins:**
- AngelGrain — MIDI input handling pattern (retrigger on note-on)
- FlutterVerb — MIDI CC output pattern via MidiBuffer
- GainKnob — WebView relay/attachment reference implementation (canonical pattern)
- TapeAge — WebView parameter binding, requestAnimationFrame usage
