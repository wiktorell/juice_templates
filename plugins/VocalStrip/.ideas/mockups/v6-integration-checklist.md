# Stage 3 (GUI) Integration Checklist - VocalStrip v6

**Plugin:** VocalStrip
**Mockup Version:** v6
**Generated:** 2026-03-20
**Parameters:** 30 total (18 sliders, 4 combos, 4 toggles / bypass LEDs)

---

## 1. Create UI Directory Structure

```bash
mkdir -p plugins/VocalStrip/Source/ui/public/js/juce
```

Expected layout after this step:

```
plugins/VocalStrip/Source/ui/public/
  index.html
  js/
    juce/
      index.js
      check_native_interop.js
```

---

## 2. Copy UI Files

- [ ] Copy `v6-ui.html` to `Source/ui/public/index.html`
- [ ] Copy JUCE frontend library to `Source/ui/public/js/juce/index.js`
      (source: `<JUCE_ROOT>/modules/juce_gui_extra/native/javascript/index.js`)
- [ ] Copy `check_native_interop.js` to `Source/ui/public/js/juce/check_native_interop.js`
      (source: same JUCE module directory — required, pattern #13)

---

## 3. Update PluginEditor.h

- [ ] Replace content with `v6-PluginEditor-TEMPLATE.h`
- [ ] Rename class to `VocalStripAudioProcessorEditor` (already done in template)
- [ ] Verify member declaration order: **relays -> webView -> attachments**
      Count: 30 relays declared BEFORE webView, 30 attachments declared AFTER webView
- [ ] Confirm `#include <juce_gui_extra/juce_gui_extra.h>` is present

---

## 4. Update PluginEditor.cpp

- [ ] Replace content with `v6-PluginEditor-TEMPLATE.cpp`
- [ ] Verify initialization order in constructor initializer list matches declaration order
- [ ] Confirm relay count = 30 (matches parameter count)
- [ ] Confirm attachment count = 30 (matches relay count)
- [ ] Confirm all parameter IDs in `getParameter("id")` calls match APVTS IDs exactly

**Parameter ID reference (must match PluginProcessor.cpp APVTS definitions):**

| Parameter ID         | Type    | Relay Type              |
|----------------------|---------|-------------------------|
| eq_hp_freq           | Float   | WebSliderRelay          |
| eq_low_shelf_gain    | Float   | WebSliderRelay          |
| eq_low_shelf_freq    | Float   | WebSliderRelay          |
| eq_mid_gain          | Float   | WebSliderRelay          |
| eq_mid_freq          | Float   | WebSliderRelay          |
| eq_mid_q             | Float   | WebSliderRelay          |
| eq_high_shelf_gain   | Float   | WebSliderRelay          |
| eq_high_shelf_freq   | Float   | WebSliderRelay          |
| eq_bypass            | Bool    | WebToggleButtonRelay    |
| comp_mode            | Choice  | WebComboBoxRelay        |
| comp_amount          | Float   | WebSliderRelay          |
| comp_threshold       | Float   | WebSliderRelay          |
| comp_ratio           | Float   | WebSliderRelay          |
| comp_attack          | Float   | WebSliderRelay          |
| comp_release         | Float   | WebSliderRelay          |
| comp_makeup          | Float   | WebSliderRelay          |
| comp_bypass          | Bool    | WebToggleButtonRelay    |
| delay_mode           | Choice  | WebComboBoxRelay        |
| delay_time           | Float   | WebSliderRelay          |
| delay_division       | Choice  | WebComboBoxRelay        |
| delay_feedback       | Float   | WebSliderRelay          |
| delay_mix            | Float   | WebSliderRelay          |
| delay_reverb_send    | Float   | WebSliderRelay          |
| delay_bypass         | Bool    | WebToggleButtonRelay    |
| reverb_type          | Choice  | WebComboBoxRelay        |
| reverb_predelay      | Float   | WebSliderRelay          |
| reverb_decay         | Float   | WebSliderRelay          |
| reverb_damping       | Float   | WebSliderRelay          |
| reverb_mix           | Float   | WebSliderRelay          |
| reverb_bypass        | Bool    | WebToggleButtonRelay    |

---

## 5. Update CMakeLists.txt

- [ ] Add `NEEDS_WEB_BROWSER TRUE` to `juce_add_plugin()` call (pattern #9)
- [ ] Add `juce_add_binary_data(VocalStrip_UIResources ...)` block from snippet
      (includes index.html, index.js, check_native_interop.js)
- [ ] Add `VocalStrip_UIResources` and `juce::juce_gui_extra` to `target_link_libraries()`
- [ ] Add `JUCE_WEB_BROWSER=1` and `JUCE_USE_CURL=0` to `target_compile_definitions()`
- [ ] Verify `juce_generate_juce_header(VocalStrip)` is present after `target_link_libraries()`

Reference: `v6-CMakeLists-SNIPPET.txt`

---

## 6. Build — Debug Mode

```bash
./scripts/build-and-install.sh VocalStrip
```

- [ ] Build completes without errors
- [ ] No warnings about missing BinaryData symbols
- [ ] Standalone opens and shows WebView (not blank white)
- [ ] Open developer console: right-click inside plugin -> Inspect (macOS WebView)
- [ ] Console shows no JavaScript errors
- [ ] `window.__JUCE__` object exists in console

---

## 7. Verify WebView Configuration

- [ ] No viewport units in CSS: search `index.html` for `100vh`, `100vw` -> should be absent
- [ ] `html, body { height: 100%; }` present in CSS
- [ ] `user-select: none` present (native application feel)
- [ ] Context menu disabled: `document.addEventListener("contextmenu", ...)` present
- [ ] Resource provider logs no "unhandled URL" messages (check Xcode console)

---

## 8. Build — Release Mode

```bash
./scripts/build-and-install.sh VocalStrip --config Release
```

- [ ] Release build succeeds
- [ ] Load plugin in DAW (VST3 and AU)
- [ ] Reload plugin 10 times — no crashes (tests member order correctness)
- [ ] Plugin shows correctly in DAW plugin list

---

## 9. Test Parameter Binding — All 30 Parameters

### EQ Section
- [ ] HP knob syncs UI <-> APVTS (20–500 Hz)
- [ ] Low Shelf Gain knob syncs UI <-> APVTS (-12 to +12 dB)
- [ ] Low Shelf Freq knob syncs UI <-> APVTS (60–400 Hz)
- [ ] Mid Gain knob syncs UI <-> APVTS (-12 to +12 dB)
- [ ] Mid Freq knob syncs UI <-> APVTS (200–8000 Hz)
- [ ] Mid Q knob syncs UI <-> APVTS (0.3–5.0)
- [ ] Hi Shelf Gain knob syncs UI <-> APVTS (-12 to +12 dB)
- [ ] Hi Shelf Freq knob syncs UI <-> APVTS (2000–16000 Hz)
- [ ] EQ Bypass LED toggles correctly

### Compressor Section
- [ ] Comp Mode pill switch syncs UI <-> APVTS (Simple/Manual)
- [ ] Amount knob visible in Simple mode, syncs UI <-> APVTS (0–100%)
- [ ] Threshold knob visible in Manual mode, syncs UI <-> APVTS (-60 to 0 dB)
- [ ] Ratio knob visible in Manual mode, syncs UI <-> APVTS (1–20)
- [ ] Attack knob visible in Manual mode, syncs UI <-> APVTS (0.1–100 ms)
- [ ] Release knob visible in Manual mode, syncs UI <-> APVTS (10–1000 ms)
- [ ] Makeup knob syncs UI <-> APVTS (0–24 dB)
- [ ] Comp Bypass LED toggles correctly

### Delay Section
- [ ] Delay Mode pill switch syncs UI <-> APVTS (Slapback/Sync)
- [ ] Time knob visible in Slapback mode, syncs UI <-> APVTS (20–200 ms)
- [ ] Division buttons visible in Sync mode, sync UI <-> APVTS (1/16, 1/8, 1/4, 1/2)
- [ ] Feedback knob syncs UI <-> APVTS (0–90%)
- [ ] Mix knob syncs UI <-> APVTS (0–100%)
- [ ] Reverb Send slider syncs UI <-> APVTS (0–100%)
- [ ] Delay Bypass LED toggles correctly

### Reverb Section
- [ ] Reverb Type pill switch syncs UI <-> APVTS (Plate/Room)
- [ ] Pre-Dly knob syncs UI <-> APVTS (0–100 ms)
- [ ] Decay knob syncs UI <-> APVTS (0.3–8.0 s)
- [ ] Damping knob syncs UI <-> APVTS (0–100%)
- [ ] Mix knob syncs UI <-> APVTS (0–100%)
- [ ] Reverb Bypass LED toggles correctly

---

## 10. Test Automation and Presets

- [ ] DAW automation writes and reads back all parameters
- [ ] DAW automation lane updates UI visuals in real time
- [ ] Save preset -> reload preset -> all knobs restore to saved positions
- [ ] All 30 parameters persist after plugin reload (close/reopen plugin window)

---

## 11. Conditional UI Behaviour

- [ ] Comp mode SIMPLE: shows large Amount knob, hides Threshold/Ratio/Attack/Release
- [ ] Comp mode MANUAL: shows Threshold/Ratio/Attack/Release, hides Amount knob
- [ ] Delay mode SLAPBACK: shows Time knob, hides division buttons
- [ ] Delay mode SYNC: shows division buttons, hides Time knob
- [ ] Feedback and Mix knobs always visible regardless of delay mode

---

## Known Issues / Notes

- GR meter needle is driven by decorative animation loop in v6-ui.html.
  For production, wire it to a JUCE native function callback that sends
  real gain reduction values from the DSP compressor.
  See pattern #20 (requestAnimationFrame) in juce8-critical-patterns.md.

- Preset selector in top bar is decorative (no JUCE preset integration yet).
  Wire to APVTS preset management or a custom preset manager in Stage 3.

- comp_mode in YAML has `default: 1` (MANUAL). Confirm APVTS default matches.

- delay_mode in YAML has `default: 0` (SLAPBACK). Confirm APVTS default matches.

- reverb_type in YAML has `default: 0` (PLATE). Confirm APVTS default matches.

- All four Choice parameters need `juce::AudioParameterChoice` in APVTS with
  matching option lists:
    comp_mode:     ["Simple", "Manual"]
    delay_mode:    ["Slapback", "Sync"]
    delay_division:["1/16", "1/8", "1/4", "1/2"]
    reverb_type:   ["Plate", "Room"]

---

## Reference Files

| File | Purpose |
|------|---------|
| `v6-ui.html`                    | Production HTML — copy to Source/ui/public/index.html |
| `v6-PluginEditor-TEMPLATE.h`    | C++ header boilerplate |
| `v6-PluginEditor-TEMPLATE.cpp`  | C++ implementation with parameter bindings |
| `v6-CMakeLists-SNIPPET.txt`     | CMake WebView configuration |
| `v6-ui.yaml`                    | Finalized design specification |
| `v6-ui-test.html`               | Browser-testable mockup (reference only) |
| `../parameter-spec.md`          | Immutable parameter contract |
