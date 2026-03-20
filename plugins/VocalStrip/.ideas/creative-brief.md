# VocalStrip - Creative Brief

## Overview

**Type:** Audio Effect (Vocal Channel Strip)
**Core Concept:** En komplett signalkedja för sångpålägg — EQ → Kompressor → Delay → Reverb — designad för snabb och enkel inspelning med professionellt resultat.
**Status:** 💡 Ideated
**Created:** 2026-03-20

## Vision

VocalStrip är en allt-i-ett kanalremsa optimerad för sånginspelning. Tanken är att musiker och producenter snabbt ska kunna forma en bra vokalklang utan att behöva stapla flera plugins. Signalen flödar i klassisk ordning: EQ rensar och formar, kompressorn kontrollerar dynamiken transparent, delayen ger djup och dimension, och reverbet placerar rösten i ett akustiskt rum.

Pluginet inspireras av det enkla arbetsflödet hos klassiska analoga kanalremsor som SSL 4000 och UAD Topline Vocal Suite — kontrollerbart men inte överväldigande. Varje sektion kan bypassa individuellt, vilket gör det enkelt att jämföra med/utan och hitta rätt balans under inspelningssessionen.

Nyckelinsikt från forskning: de bästa vokalpluginsen prioriterar snabbhet framför maximal kontroll. VocalStrip erbjuder därför ett "Amount"-läge på kompressorn (inspirerat av Voxformers one-knob-approach och Toplines Transparent→Aggressive-skala) vid sidan av full manuell kontroll. En send-slider från delay till reverb möjliggör att delay-ekot matas in i reverbet för djupare, mer sammanhängande rumskänsla — ett designval hämtat från UAD Topline.

## Signal Flow

```
[Mono Input] → EQ → Compressor → Delay → Reverb → [Stereo Output]
```

## Parameters

### EQ (Semi-parametric, 3-band)

| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| HP Filter | 20–500 Hz | 80 Hz | High-pass filter för att rensa lågfrekvent brus |
| Low Shelf Gain | -12 to +12 dB | 0 dB | Boost/cut av låga frekvenser |
| Low Shelf Freq | 60–400 Hz | 200 Hz | Frekvens för low shelf |
| Mid Gain | -12 to +12 dB | 0 dB | Boost/cut av midfrekvenser |
| Mid Freq | 200–8000 Hz | 1000 Hz | Sweepable mittband |
| Mid Q | 0.3–5.0 | 1.0 | Bandbredd för mittbandet |
| High Shelf Gain | -12 to +12 dB | 0 dB | Boost/cut av höga frekvenser |
| High Shelf Freq | 2000–16000 Hz | 8000 Hz | Frekvens för high shelf |
| EQ Bypass | On/Off | Off | Bypassa EQ-sektionen |

### Compressor (VCA-stil, transparent)

| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| Comp Mode | Simple / Manual | Simple | Simple = one-knob Amount, Manual = full kontroll |
| Amount | 0–100% | 50% | One-knob kompression (Simple-läge) — styr threshold/ratio automatiskt |
| Threshold | -60 to 0 dB | -18 dB | Nivå vid vilken kompression startar (Manual-läge) |
| Ratio | 1:1 to 20:1 | 4:1 | Kompressionsstyrka (Manual-läge) |
| Attack | 0.1–100 ms | 10 ms | Tid innan kompressorn reagerar (Manual-läge) |
| Release | 10–1000 ms | 200 ms | Tid innan kompressorn släpper — 200ms ger mjuk känsla |
| Makeup Gain | 0–24 dB | 0 dB | Kompensera för förlorad nivå |
| Comp Bypass | On/Off | Off | Bypassa kompressorsektionen |

### Delay

| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| Delay Mode | Slapback / Tempo-sync | Slapback | Välj mellan slapback och tempo-synkad delay |
| Delay Time (Slapback) | 20–200 ms | 100 ms | Delaytid i ms (slapback-läge) |
| Delay Division (Sync) | 1/16, 1/8, 1/4, 1/2 | 1/8 | Notdivision (tempo-sync-läge) |
| Feedback | 0–90% | 20% | Mängd upprepningar |
| Delay Mix | 0–100% | 30% | Wet/dry-balans för delayen |
| Delay→Reverb Send | 0–100% | 20% | Hur mycket av delay-signalen matas in i reverbet |
| Delay Bypass | On/Off | Off | Bypassa delay-sektionen |

### Reverb

| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| Reverb Type | Plate / Room | Plate | Välj karaktär på reverbet |
| Pre-Delay | 0–100 ms | 20 ms | Fördröjning innan reverbet startar |
| Decay | 0.3–8.0 s | 1.5 s | Hur länge klangen klingar ut |
| Damping | 0–100% | 50% | Dämpar höga frekvenser i svansen |
| Reverb Mix | 0–100% | 25% | Wet/dry-balans för reverbet |
| Reverb Bypass | On/Off | Off | Bypassa reverb-sektionen |

## UI Concept

**Layout:** Horisontell kanalremsa med fyra tydliga sektioner — EQ, Comp, Delay, Reverb — i signalflödets ordning (vänster till höger). Varje sektion har en tydlig bypass-knapp/LED.
**Visual Style:** Ren och professionell, inspirerad av klassiska studiomixrar. Mörk bakgrund med tydlig sektionsindelning.
**Key Elements:**
- Individuell bypass per sektion (lysande LED-indikator)
- GR-meter (gain reduction) på kompressorn
- Simple/Manual-toggle på kompressorn — enkel knapp döljer/visar avancerade parametrar
- Delay→Reverb send-slider synlig mellan de två sektionerna
- Signal flow visually implied left-to-right
- Preset-väljare i toppen

## Use Cases

- Snabb inspelning av sångpålägg med omedelbar monitorlyssning
- Demo-produktioner där man vill ha ett färdigt sound direkt
- Live-inspelning där ingen efterproduktion planeras
- Nybörjare som vill slippa stacka EQ + kompressor + delay + reverb separat

## Inspirations

- SSL 4000 Channel Strip (VCA-kompressor, semi-parametrisk EQ, EQ/comp-ordning)
- UAD Topline Vocal Suite (signal flow left-to-right, delay→reverb send, simplified controls)
- Voxengo Voxformer (one-knob kompressor, snabbhetsfilosofi)
- UAD Neve 1073 (EQ-karaktär)
- Lexicon 480L (reverb-känsla, plate och room)
- Abbey Road Plates (klassisk vokalkvalitet)

## Technical Notes

- Mono-in / Stereo-out signalformat (mikrofon → stereo DAW-kanal)
- Delay och reverb sprider ut signalen till stereo; EQ och comp arbetar i mono
- VCA-kompressor: standard lookahead-fri implementation med snabb attack/release; Simple-läge beräknar threshold/ratio automatiskt från Amount-värdet
- Semi-parametrisk EQ: biquad-filter per band (high-pass Butterworth + shelf/peak-filter); SSL-inspirerade standardvärden (boost 4 kHz för närvaro, high shelf 10 kHz för luft)
- Delay: separata delay-linjer per läge (slapback = single delay, sync = BPM-locked via AudioPlayHead)
- Delay→Reverb send: delay-output mixas in i reverb-input med separat nivå
- Reverb: algoritmisk (Schroeder/Moorer-variant eller Freeverb-baserad) med Plate/Room presets
- APVTS för alla parametrar med DAW-automation och preset-hantering
- Startpresets: "Pop Vocal", "Rock Vocal", "Intimate" — justerar alla sektioner som paket

## Next Steps

- [ ] Create UI mockup (`/dream VocalStrip` → option 3)
- [ ] Start implementation (`/implement VocalStrip`)
