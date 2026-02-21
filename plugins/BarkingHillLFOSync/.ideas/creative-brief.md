# BarkingHillLFOSync - Creative Brief

## Overview

**Type:** Utility (Modulation)
**Core Concept:** A tempo-syncable LFO plugin that modulates other plugin parameters via MIDI CC or host automation, with an animated waveform display and an easy assign workflow.
**Status:** 💡 Ideated
**Created:** 2026-02-21

## Vision

BarkingHillLFOSync is a dedicated LFO modulation utility that sits in your DAW's signal chain and drives parameter automation on any other plugin. Rather than digging into the host's automation lanes, the user presses an Assign button, clicks a target parameter on another plugin, and the LFO immediately starts driving it. Modulation is output as MIDI CC (user-configurable, CC 1–127) and/or host automation, giving compatibility across all major hosts.

The plugin prioritises simplicity of use: three knobs (Rate, Depth, Waveform), two toggles (Sync and Retrigger), a CC number selector, and an animated real-time waveform display. When synced to MIDI clock, the Rate knob becomes a subdivision selector ranging from 4 bars down to 1/32 including triplets and dotted values.

## Parameters

| Parameter | Range / Options | Default | Description |
|-----------|----------------|---------|-------------|
| Rate | Free: 0.1–10 Hz / Sync: 4 bars → 1/32 (incl. triplets + dotted) | 1 Hz / 1 bar | LFO speed; behaves as Hz in free mode, subdivision selector in sync mode |
| Depth | −100% to +100% (bipolar) | 50% | Modulation depth; positive drives the parameter up, negative drives it down |
| Waveform | Sine, Triangle, Square, Saw Up, Saw Down | Sine | Selects the LFO shape via a dedicated knob/selector |
| Sync | Free / MIDI Clock | Free | Switches Rate knob between free-running Hz and MIDI clock subdivisions |
| Retrigger | Off / On | Off | When On, LFO phase resets to 0 on each incoming MIDI note-on |
| CC Number | 1–127 | 1 (Mod Wheel) | MIDI CC number the plugin outputs its LFO signal on |
| Assign | Button (momentary) | — | Arms the plugin to capture a target parameter from another plugin |

### Clock Subdivisions (Sync mode)

4 bars, 2 bars, 1 bar, 1/2, 1/2T, 1/4, 1/4T, 1/8, 1/8T, 1/16, 1/16T, 1/32, 1/32T
(T = triplet; dotted variants to be considered in Stage 0 research)

## UI Concept

**Finalized mockup:** v7 (`plugins/BarkingHillLFOSync/.ideas/mockups/v7-ui.yaml`)
**Window size:** 700 × 390px
**Aesthetic:** Vintage Hardware — warm brass/gold on dark brown, CRT scan lines, skeuomorphic knobs

**Layout:** Two zones — full-width oscilloscope display on top, controls row on bottom
**Visual Style:** Warm vintage analog hardware with animated oscilloscope centrepiece

**Zone 1 — Waveform Display (full width, y=15, 670×185px):**
- Deep inset oscilloscope housing (#1a0a00 background, 3px #3a2a1a border)
- CRT horizontal scan lines overlay (0.04 opacity)
- Animated gold waveform line (#d4a574, 2px stroke) — shape changes with Waveform knob
- Vertical gold playhead sweeps left-to-right through one full LFO cycle at 60fps
- Plugin title "LFO SYNC" top-left in gold, 11px, weight 300, letter-spacing 0.4em

**Zone 2 — Controls row (y=222, left to right):**
- Rate knob 90×90px @ x=20 — shows Hz or subdivision name when synced
- Depth knob 90×90px @ x=135 — bipolar, 12 o'clock = 0%
- Waveform knob 90×90px @ x=255 — stepped, 5 shapes
- Sync toggle 32×80px @ x=375 — FREE / CLOCK
- Retrigger toggle 32×80px @ x=430 — OFF / ON
- CC knob 60×60px @ x=483 — stepped 1–127
- Assign button 80×36px @ x=573, y=249 — pulses brass glow when armed

**Brand:** "BARKING HILL" in gold, 9px uppercase, right-aligned to plugin right edge (right: 15px, bottom: 18px)

## Use Cases

- Automate a filter cutoff on a synth in sync with the groove without drawing automation lanes
- Create subtle vibrato or tremolo on any third-party plugin that exposes parameters
- Drive rhythmic gating effects by sending a square LFO to a gain parameter
- Slow-evolving pad textures using a very slow free-running sine on a reverb mix parameter
- Live performance modulation — arm Assign, pick a target, and perform in real time

## Inspirations

- Xfer LFO Tool — the gold standard for DAW-based LFO utilities (Ableton only)
- Cableguys MidiShaper — MIDI CC-based modulation
- Native Instruments' internal LFO modulators in Reaktor/Komplete
- Classic hardware LFOs (Juno-106, JP-8000)

## Technical Notes

- **Routing:** Two parallel output paths:
  1. **MIDI CC output** — LFO value mapped to CC 1–127 (user-selectable), output on MIDI channel 1 by default. Host must route MIDI output to target plugin's MIDI input.
  2. **Host automation output** — Plugin exposes a "Modulation Output" automation parameter that external automation clips or the host can read and route to a target parameter.
- **Assign button mechanism:** In VST3, direct plugin-to-plugin parameter control requires host cooperation. Stage 0 research should investigate: VST3 parameter linking APIs, host-specific solutions (e.g., Ableton's MIDI routing, Logic's MIDI FX chain), and whether an "assign" UX is achievable or must be documented as a host-setup workflow.
- **Waveform rendering:** LFO waveform calculated per-block; display updated on timer callback (~60 fps).
- **Bipolar depth:** LFO output range [-1, +1] scaled by depth. A depth of +100% means full parameter range swing above centre; −100% inverts the direction.
- **Subdivision selector:** When in Sync mode, Rate knob becomes a stepped selector. JUCE AudioParameterChoice works well here.

## Next Steps

- [x] Create UI mockup — v7 finalized
- [ ] Start implementation (`/implement BarkingHillLFOSync`)
