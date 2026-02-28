# BarkingHillLFOSync Changelog

## [2.0.0] - 2026-02-28

### Changed — Architecture: Audio Effect → MIDI FX
- Plugin is now a MIDI FX (`IS_MIDI_EFFECT TRUE`, `isMidiEffect()` returns true).
  In Logic Pro it appears in the **MIDI FX slot** (above the instrument plugin)
  instead of the audio effects chain.
- Audio bus configuration removed (no stereo in/out). LFO CC output and mod_output
  automation parameter are unaffected — all DSP logic is unchanged.
- Workflow in Logic: add to MIDI FX slot → LFO outputs CC through MIDI chain →
  Smart Controls respond to CC → map to any plugin parameter on the track.
- `isBusesLayoutSupported()` updated to accept empty bus layout.
- Constructor updated: `BusesProperties()` with no audio buses.

## [1.2.0] - 2026-02-28

### Changed
- Removed LEARN button and all MIDI CC learn logic. The MIDI CC approach
  does not work for audio effect plugins in Logic Pro — MIDI is not routed
  to audio effects in the channel strip. UI simplified to 6 controls:
  Rate, Depth, Waveform, Sync, Retrigger, CC Number.
- `learn` APVTS parameter removed. Sessions saved with v1.1.0 load cleanly
  (JUCE ignores unknown parameter keys on restore).

## [1.1.0] - 2026-02-28

### Added
- **MIDI CC Learn** — new LEARN button replaces the non-functional Assign button.
  Press LEARN → plugin arms (button pulses brass glow, label shows LISTENING) →
  move any CC on the incoming MIDI path → cc_number updates automatically →
  plugin disarms. Click again to cancel learn mode.

### Changed
- `assign` APVTS parameter removed; replaced by `learn` (AudioParameterBool).
  All session state including cc_number persists as before. Sessions saved with
  v1.0.0 will load fine — JUCE APVTS silently ignores the unknown `assign` key
  and `learn` defaults to false (WAITING).
- Plugin is always active when the host's bypass is enabled — no internal on/off
  needed; host bypass is the single control point.

### Technical
- `processBlock`: CC learn scan added at Step 3.5, after retrigger scan and
  before outgoing CC event. Only scans incoming MIDI from host (Step 8 outgoing
  CC not yet in buffer). First `isController()` message captures CC number via
  `setValueNotifyingHost()` on both `cc_number` and `learn` params (audio-thread
  valid — same pattern as `mod_output`).
- `PluginEditor`: `assignRelay`/`assignAttachment` renamed to `learnRelay`/
  `learnAttachment`. WebView options updated. Declaration order (Relay → WebView
  → Attachment) preserved per pattern #11.
- HTML/JS: Button ID `learnBtn`, label `learnLabel`. States: WAITING / LISTENING.
  JUCE relay listener auto-updates button when C++ deactivates learn after CC capture.

## [1.0.0] - 2026-02-22

### Initial release
- Tempo-syncable LFO modulator with animated oscilloscope display
- 5 waveforms: Sine, Triangle, Square, Saw Up, Saw Down
- MIDI clock sync (13 subdivisions: 4 BAR → 1/32T)
- Phase retrigger on MIDI note-on
- Bipolar depth (−100% → +100%)
- MIDI CC output (CC 1–127, channel 1)
- mod_output host automation parameter
- Vintage Hardware aesthetic (700×390px, brass/gold on dark brown, CRT scan lines)
