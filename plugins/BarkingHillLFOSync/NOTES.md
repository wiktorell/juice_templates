# BarkingHillLFOSync Notes

## Status
- **Current Status:** 📦 Installed
- **Version:** 2.0.0
- **Type:** MIDI FX (LFO Modulator)

## Lifecycle Timeline

- **2026-02-21 (Ideation):** Creative brief created. Core concept: tempo-syncable LFO modulator.
- **2026-02-21 (UI Mockup):** v7 finalized. 700×390px, vintage hardware aesthetic (brass/gold, CRT scan lines).
- **2026-02-22 (Stage 0):** Architecture + plan documented. Complexity: 3.4.
- **2026-02-22 (Stage 1):** Foundation complete. APVTS 6+1 parameters, WebView editor, CMakeLists.
- **2026-02-22 (Stage 2):** DSP complete. LFO oscillator (5 waveforms), MIDI clock sync, retrigger, bipolar depth.
- **2026-02-22 (Stage 3):** GUI complete. v7 HTML deployed, all parameter bindings, animated oscilloscope.
- **2026-02-22 (v1.0.0):** Installed as audio effect plugin.
- **2026-02-28 (v1.1.0):** Added MIDI CC Learn button (replaced non-functional Assign button).
- **2026-02-28 (v1.2.0):** Removed Learn button — MIDI CC routing does not work for audio effects in Logic.
- **2026-02-28 (v2.0.0):** Rebuilt as MIDI FX plugin (IS_MIDI_EFFECT TRUE). Logic MIDI FX slot. Confirmed working with Alchemy via CC modulation matrix.

## Confirmed Working Use Case

**Logic Pro + Alchemy (or any instrument with MIDI CC modulation):**
1. Add BarkingHillLFOSync to the **MIDI FX slot** of a Software Instrument track
2. Set CC number in LFO Sync (default CC 1)
3. In Alchemy → Advanced → Modulation → add source: MIDI CC [same number] → destination: any parameter
4. LFO drives the parameter in real-time via MIDI CC

## Known Limitations

- Does **not** modulate audio effect plugin parameters in Logic (Smart Controls do not respond to plugin-generated MIDI CC)
- Works on **Software Instrument tracks only** (MIDI FX slot) — not audio tracks
- Works with instruments that support MIDI CC modulation routing (Alchemy, ES2, Kontakt, most synths)
- In Ableton Live / Reaper: MIDI CC routing is more flexible — may work in additional contexts

## Parameters

| Parameter | ID | Range | Default | Description |
|-----------|-----|-------|---------|-------------|
| Rate | `rate` | 0.1–10 Hz | 1 Hz | LFO speed (Hz in free mode, subdivision index in sync mode) |
| Depth | `depth` | −100–+100% | 50% | Modulation depth, bipolar |
| Waveform | `waveform` | 0–4 | 0 (Sine) | Sine, Triangle, Square, Saw Up, Saw Down |
| Sync | `sync` | bool | false | Free mode / MIDI clock sync |
| Retrigger | `retrigger` | bool | false | Reset phase on MIDI note-on |
| CC Number | `cc_number` | 1–127 | 1 | MIDI CC output number |
| Mod Output | `mod_output` | 0–1 | 0.5 | System param — LFO value as host automation lane |

## Installation Locations

- VST3: `~/Library/Audio/Plug-Ins/VST3/LFO Sync.vst3`
- AU: `~/Library/Audio/Plug-Ins/Components/LFO Sync.component`
