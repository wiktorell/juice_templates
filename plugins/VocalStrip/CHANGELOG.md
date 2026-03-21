# VocalStrip Changelog

## [1.2.0] — 2026-03-21

### Changed
- Compressor simplified to 3 knobs: Amount, Attack, Release — mode switch removed
- Amount knob controls threshold (-6→-40 dB), ratio (2:1→10:1) and auto gain compensation simultaneously
- Auto-makeup calculated as `|threshold| × (1 - 1/ratio) × 0.5` — gain compensates automatically as Amount increases
- Compressor knobs positioned at same height as EQ Mid row (top:105px)
- Removed: Simple/Manual mode switch, Threshold, Ratio, Makeup knobs from UI
- Retained: comp_mode, comp_threshold, comp_ratio, comp_makeup APVTS parameters for state compatibility

## [1.1.0] — 2026-03-21

### Added
- HP filter on/off switch (`eq_hp_enable`) — fixed at 100 Hz, default enabled
- HP LED button in EQ section UI (bottom row, right side)

### Changed
- EQ layout reordered: High (top) → Mid (middle) → Low + HP (bottom)
- Low shelf frequency fixed at 150 Hz (knob removed from UI; parameter retained for state compatibility)
- Mid EQ Q fixed at 1.0 (Q knob removed from UI; parameter retained for state compatibility)
- High shelf frequency fixed at 10 kHz (knob removed from UI; parameter retained for state compatibility)
- HP filter frequency fixed at 100 Hz (freq knob removed from UI; parameter retained for state compatibility)
- All EQ gain ranges extended: -12/+12 dB → **-20/+20 dB** (Low, Mid, High)

### Notes
- No breaking changes: all APVTS parameter IDs preserved, state files load correctly
- Mid Q defaults to 1.0 in DSP regardless of stored Q value

## [1.0.0] — 2026-03-21

### Added
- Initial release: EQ (HP + Low Shelf + Mid Peak + High Shelf), Compressor (Simple/Manual modes),
  Delay (Slapback/Sync), Reverb (Plate/Room) with GR meter
- WebView UI with brass/vintage aesthetic
- GR meter needle driven at 30 Hz from audio thread
