# Stage 3 (GUI) Integration Checklist — BarkingHillLFOSync v7

**Plugin:** BarkingHillLFOSync
**Mockup Version:** v7
**Window Size:** 700 x 390 px
**Generated:** 2026-02-21
**Parameters:** 7 (rate, depth, waveform, sync, retrigger, cc_number, assign)

---

## Pre-Integration Verification

- [ ] Stage 1 (Foundation) complete — project builds clean
- [ ] Stage 2 (Shell) complete — APVTS has all 7 parameters
- [ ] Verify APVTS parameter IDs match exactly (case-sensitive):
  - `rate` (Float, 0.1–10.0)
  - `depth` (Float, -100.0–100.0)
  - `waveform` (Choice/Float, 0–4)
  - `sync` (Bool)
  - `retrigger` (Bool)
  - `cc_number` (Float, 1–127, step 1)
  - `assign` (Bool)

---

## 1. Set Up Directory Structure

```
Source/
└── ui/
    └── public/
        ├── index.html
        └── js/
            └── juce/
                ├── index.js
                └── check_native_interop.js
```

- [ ] Create `Source/ui/public/` directory
- [ ] Create `Source/ui/public/js/juce/` directory

---

## 2. Copy UI Files

- [ ] Copy `v7-ui.html` to `Source/ui/public/index.html`
  - Verify: no `plugin-frame` wrapper div (that is test-only)
  - Verify: no `.debug-monitor` element
  - Verify: no `.version-tag` element
  - Verify: no mock `window.Juce` backend
  - Verify: no `console.log` debug statements
- [ ] Copy JUCE frontend library to `Source/ui/public/js/juce/index.js`
  - Source: JUCE installation at `modules/juce_gui_extra/native/javascript/index.js`
  - Or copy from a known working plugin (e.g., GainKnob example)
- [ ] Copy `check_native_interop.js` to `Source/ui/public/js/juce/check_native_interop.js`
  - Source: same JUCE installation or working plugin
  - REQUIRED — missing this file causes frozen UI with no error

---

## 3. Update CMakeLists.txt

Reference: `v7-CMakeLists.txt`

- [ ] Add `NEEDS_WEB_BROWSER TRUE` to `juce_add_plugin()` call
  - Critical for VST3: without it the plugin builds but does not appear in DAWs
- [ ] Add `juce_add_binary_data(BarkingHillLFOSync_UIResources ...)` after `juce_add_plugin()`
  - Include: `index.html`, `index.js`, `check_native_interop.js`
- [ ] Add to `target_link_libraries()`:
  - `juce::juce_gui_extra`
  - `BarkingHillLFOSync_UIResources`
- [ ] Add `juce_generate_juce_header(BarkingHillLFOSync)` after `target_link_libraries()`
- [ ] Add to `target_compile_definitions()`:
  - `JUCE_WEB_BROWSER=1`
  - `JUCE_USE_CURL=0`

---

## 4. Replace PluginEditor.h

Reference: `v7-PluginEditor.h`

- [ ] Replace `Source/PluginEditor.h` with content from `v7-PluginEditor.h`
- [ ] Verify member declaration order (critical — see JUCE 8 patterns §11):
  ```
  1. rateRelay, depthRelay, waveformRelay, syncRelay, retriggerRelay, ccNumberRelay, assignRelay
  2. webView
  3. rateAttachment, depthAttachment, waveformAttachment, syncAttachment,
     retriggerAttachment, ccNumberAttachment, assignAttachment
  ```
- [ ] Verify relay types match parameter types:

  | Parameter   | Relay Type              | Attachment Type                       |
  |-------------|------------------------|---------------------------------------|
  | rate        | WebSliderRelay         | WebSliderParameterAttachment          |
  | depth       | WebSliderRelay         | WebSliderParameterAttachment          |
  | waveform    | WebSliderRelay         | WebSliderParameterAttachment          |
  | sync        | WebToggleButtonRelay   | WebToggleButtonParameterAttachment    |
  | retrigger   | WebToggleButtonRelay   | WebToggleButtonParameterAttachment    |
  | cc_number   | WebSliderRelay         | WebSliderParameterAttachment          |
  | assign      | WebToggleButtonRelay   | WebToggleButtonParameterAttachment    |

---

## 5. Replace PluginEditor.cpp

Reference: `v7-PluginEditor.cpp`

- [ ] Replace `Source/PluginEditor.cpp` with content from `v7-PluginEditor.cpp`
- [ ] Verify construction order in constructor body matches header declaration order:
  1. Create all 7 relays
  2. Create webView with `.withOptionsFrom(*relay)` for each relay
  3. Create all 7 attachments
  4. `addAndMakeVisible(*webView)` and `goToURL(...)`
  5. `setSize(700, 390)` and `setResizable(false, false)`
- [ ] Verify each attachment uses 3-argument constructor (JUCE 8, patterns §12):
  ```cpp
  rateAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
      *audioProcessor.parameters.getParameter("rate"), *rateRelay, nullptr);
  ```
- [ ] Verify `getResource()` serves all 3 files with correct MIME types:
  - `/` and `/index.html` → `"text/html"`
  - `/js/juce/index.js` → `"text/javascript"`
  - `/js/juce/check_native_interop.js` → `"text/javascript"`

---

## 6. Build — Debug Configuration

- [ ] Configure CMake: `cmake -B build -DCMAKE_BUILD_TYPE=Debug`
- [ ] Build: `cmake --build build --target BarkingHillLFOSync_Standalone`
- [ ] Build succeeds with zero errors
- [ ] Build succeeds with zero new warnings

---

## 7. Test — Standalone Debug

- [ ] Standalone app launches without crash
- [ ] Plugin window is 700 x 390 px
- [ ] WebView loads (not a blank white rectangle)
- [ ] Open DevTools (right-click → Inspect, or Cmd+Option+I on macOS)
- [ ] Console shows no JavaScript errors
- [ ] `window.__JUCE__` object exists in DevTools console
- [ ] `window.barkingHillLFOSync` namespace exists
- [ ] Waveform canvas animates at 60fps

---

## 8. Test — Parameter Binding (Debug)

For each of the 7 parameters:

- [ ] **rate** — drag knob up/down, DAW automation lane updates
- [ ] **depth** — drag knob, value display shows bipolar % (e.g., +50%, -30%)
- [ ] **waveform** — drag knob, snaps to 5 positions (SINE/TRIANGLE/SQUARE/SAW UP/SAW DOWN)
- [ ] **sync** — click toggle, switches between FREE and CLOCK labels
- [ ] **retrigger** — click toggle, switches between OFF and ON labels
- [ ] **cc_number** — drag small knob, integer CC number updates (1–127)
- [ ] **assign** — click button, pulse animation starts/stops; label reads ARMED/DISARMED

**Bidirectional sync check:**
- [ ] Move knob → APVTS parameter updates (check in DAW automation lane)
- [ ] Set automation → UI visual updates (indicator rotates to correct position)
- [ ] Save preset → reload → all 7 values restored correctly

---

## 9. Build — Release Configuration

- [ ] Build: `cmake --build build --target BarkingHillLFOSync_VST3 BarkingHillLFOSync_AU --config Release`
- [ ] Build succeeds with zero errors
- [ ] Install to system: run project build-and-install script

---

## 10. Test — Release Build Stability

The member order crash only manifests in release builds. Run this sequence 10 times:

- [ ] Load plugin in DAW
- [ ] Change a parameter value
- [ ] Close plugin editor window
- [ ] Reopen plugin editor window
- [ ] DAW does NOT freeze or crash

If DAW freezes on step 5, the member declaration order in `PluginEditor.h` is wrong.
Re-verify relays are declared BEFORE webView, and attachments AFTER webView.

---

## 11. WebView-Specific Validation

- [ ] No viewport units in CSS (`100vh`, `100vw`, `100dvh` — these cause layout issues)
  - v7-ui.html uses `height: 100%` with `html, body { height: 100% }` — correct
- [ ] `user-select: none` is present in CSS — correct in v7-ui.html
- [ ] Context menu is disabled (right-click shows nothing)
- [ ] Resource provider returns correct MIME types (no 404 errors in DevTools Network tab)
- [ ] No mixed-content warnings in console

---

## 12. Known Gotchas

**Knobs frozen but UI loads:**
- Check that `check_native_interop.js` is in BinaryData AND served by `getResource()`
- Check that `type="module"` is on both script tags in index.html
- Check that attachment uses 3-argument constructor (nullptr as third arg)

**UI blank/white:**
- Check `NEEDS_WEB_BROWSER TRUE` in `juce_add_plugin()`
- Check `JUCE_WEB_BROWSER=1` in `target_compile_definitions()`
- Check `juce::juce_gui_extra` is linked

**DAW freeze on plugin reload (release only):**
- Member declaration order wrong — relays must come before webView, attachments must come last
- This ONLY manifests in release builds, not debug

**Parameter values not persisting:**
- Verify APVTS parameter IDs in PluginProcessor match relay IDs exactly (case-sensitive)
- IDs: `rate`, `depth`, `waveform`, `sync`, `retrigger`, `cc_number`, `assign`

**Waveform parameter is type Choice but uses WebSliderRelay:**
- This is intentional: AudioParameterChoice is normalized to 0–1 by JUCE
- WebSliderRelay correctly handles this when the UI maps 0–1 to discrete steps

---

## Reference Files

| File | Purpose |
|------|---------|
| `v7-ui.html` | Production HTML (copy to `Source/ui/public/index.html`) |
| `v7-PluginEditor.h` | C++ header with correct member order |
| `v7-PluginEditor.cpp` | C++ implementation with all 7 parameter bindings |
| `v7-CMakeLists.txt` | CMake configuration snippet |
| `v7-integration-checklist.md` | This file |
| `../.ideas/parameter-spec.md` | Locked parameter specification |

---

## Critical Patterns Reference

- Member declaration order: `juce8-critical-patterns.md` §11
- 3-argument attachment constructor: `juce8-critical-patterns.md` §12
- `check_native_interop.js` required: `juce8-critical-patterns.md` §13
- `NEEDS_WEB_BROWSER TRUE`: `juce8-critical-patterns.md` §9
- `juce_generate_juce_header()`: `juce8-critical-patterns.md` §1
- Explicit resource URL mapping: `juce8-critical-patterns.md` §8
- Relative knob drag pattern: `juce8-critical-patterns.md` §16
- ES6 module loading: `juce8-critical-patterns.md` §21
- Boolean parameters use `getToggleState`: `juce8-critical-patterns.md` §19
