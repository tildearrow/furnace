# effect list

most of the effect numbers are from ProTracker/FastTracker 2.
however, effects are continuous, which means you only need to type it once and then stop it with an effect value of `00`.

- `00xy`: arpeggio. after using this effect the channel will rapidly switch between `note`, `note+x` and `note+y`.
- `01xx`: slide up.
- `02xx`: slide down.
- `03xx`: note portamento.
  - a note must be present for this effect to work.
- `04xy`: vibrato. `x` is the speed, while `y` is the depth.
  - maximum vibrato depth is ±1 semitone.
- `07xy`: tremolo. `x` is the speed, while `y` is the depth.
  - maximum tremolo depth is -60 volume steps.
- `08xy`: set panning. `x` is the left channel and `y` is the right one.
  - not all chips support this effect.
- `80xx`: set panning (linear). this effect behaves more like other trackers:
  - `00` is left.
  - `80` is center.
  - `FF` is right.
  - not all chips support this effect.
- `81xx`: set volume of left channel (from `00` to `FF`).
  - not all chips support this effect.
- `82xx`: set volume of right channel (from `00` to `FF`).
  - not all chips support this effect.
- `09xx`: set speed 1.
- `0Axy`: volume slide.
  - if `x` is 0 then this is a slide down.
  - if `y` is 0 then this is a slide up.
- `0Bxx`: jump to pattern.
- `0Cxx`: retrigger note every `xx` ticks.
  - this effect is not continuous.
- `0Dxx`: jump to next pattern.
- `0Fxx`: set speed 2.

- `9xxx`: set sample position to `xxx`\*0x100.
  - not all chips support this effect.

- `Cxxx`: change song Hz.
  - `xxx` may be from `000` to `3ff`.

- `E0xx`: set arpeggio tick.
  - this sets the number of ticks between arpeggio values.
- `E1xy`: note slide up. `x` is the speed, while `y` is how many semitones to slide up.
- `E2xy`: note slide down. `x` is the speed, while `y` is how many semitones to slide down.
- `E3xx`: set vibrato direction. `xx` may be one of the following:
  - `00`: up and down.
  - `01`: up only.
  - `02`: down only.
- `E4xx`: set vibrato range in 1/16th of a semitone.
- `E5xx`: set pitch. `80` is 0 cents.
  - range is ±1 semitone.
- `EAxx`: toggle legato.
- `EBxx`: set sample bank.
  - does not apply on Amiga.
- `ECxx`: note off after `xx` ticks.
- `EDxx`: delay note by `xx` ticks.
- `EExx`: send external command.
  - this effect is currently incomplete.
- `EFxx`: add or subtract global pitch.
  - this effect is rather weird. use with caution.
  - `80` is center.
- `F0xx`: change song Hz by BPM value.
- `F1xx`: single tick slide up.
- `F2xx`: single tick slide down.
- `F3xx`: fine volume slide up (64x slower than `0Axy`).
- `F4xx`: fine volume slide down (64x slower than `0Axy`).
- `F5xx`: disable macro.
  - see macro table at the end of this document for possible values.
- `F6xx`: enable macro.
- `F8xx`: single tick volume slide up.
- `F9xx`: single tick volume slide down.
- `FAxy`: fast volume slide (4x faster than `0Axy`).
  - if `x` is 0 then this is a slide down.
  - if `y` is 0 then this is a slide up.
- `FFxx`: end of song/stop playback.

additionally each chip has its own effects. [click here for more details](../7-systems/README.md).

## macro table

ID | macro
---|-----------------------------
00 | volume
01 | arpeggio
02 | duty/noise
03 | waveform
04 | pitch
05 | extra 1
06 | extra 2
07 | extra 3
08 | extra A (ALG)
09 | extra B (FM)
0A | extra C (FMS)
0B | extra D (AMS)
0C | panning left
0D | panning right
0E | phase reset
0F | extra 4
10 | extra 5
11 | extra 6
12 | extra 7
13 | extra 8
---|-----------------------------
20 | **operator 1 macros** - AM
21 | AR
22 | DR
23 | MULT
24 | RR
25 | SL
26 | TL
27 | DT2
28 | RS
29 | DT
2A | D2R
2B | SSG-EG
2C | DAM
2D | DVB
2E | EGT
2F | KSL
30 | SUS
31 | VIB
32 | WS
33 | KSR
---|-----------------------------
40 | operator 2 macros
60 | operator 2 macros
80 | operator 2 macros

the interpretation of duty, wave and extra macros depends on chip/instrument type:

ex | FM     | OPM       | OPZ       | OPLL  | AY-3-8910  | AY8930     | Lynx     | C64        | SAA1099  | X1-010     | Namco 163  | FDS       | Sound Unit | ES5506    | MSM6258  | QSound       | SNES      | MSM5232   |
---|--------|-----------|-----------|-------|------------|------------|----------|------------|----------|------------|------------|-----------|------------|-----------|----------|--------------|-----------|-----------|
 D | NoiseF | NoiseFreq |           |       | NoiseFreq  | NoiseFreq  | Duty/Int | Duty       |          |            | Wave Pos   |           | Duty       | Filt Mode | FreqDiv  | Echo Level   | NoiseFreq | GroupCtrl |
 W |        | LFO Shape | LFO Shape | Patch | Waveform   | Waveform   |          | Waveform   | Waveform | Waveform   | Waveform   | Waveform  | Waveform   |           |          |              | Waveform  |           |
 1 |        | AMD       | AMD       |       |            | Duty       |          | FilterMode | Envelope | EnvMode    | WaveLen    | Mod Depth | Cutoff     | Filter K1 | ClockDiv | EchoFeedback | Special   | GroupAtk  |
 2 |        | PMD       | PMD       |       | Envelope   | Envelope   |          | Resonance  |          | Envelope   | WaveUpdate | Mod Speed | Resonance  | Filter K2 |          | Echo Length  | Gain      | GroupDec  |
 3 |        | LFO Speed | LFO Speed |       | AutoEnvNum | AutoEnvNum |          | Special    |          | AutoEnvNum | WaveLoad W |           | Control    | Env Count |          |              |           | Noise     |
 A | ALG    | ALG       | ALG       |       | AutoEnvDen | AutoEnvDen |          |            |          | AutoEnvDen | WaveLoad P |           |            | Control   |          |              |           |           |
 B | FB     | FB        | FB        |       |            | Noise AND  |          |            |          |            | WaveLoad L |           |            |           |          |              |           |           |
 C | FMS    | FMS       | FMS       |       |            | Noise OR   |          |            |          |            | WaveLoad T |           |            |           |          |              |           |           |
 D | AMS    | AMS       | AMS       |       |            |            |          |            |          |            |            |           |            |           |          |              |           |           |
 4 | OpMask | OpMask    |           |       |            |            |          | Test/Gate  |          |            |            |           | PResetTime | EnvRampL  |          |              |           |           |
 5 |        |           | AMD2      |       |            |            |          |            |          |            |            |           |            | EnvRampR  |          |              |           |           |
 6 |        |           | PMD2      |       |            |            |          |            |          |            |            |           |            | EnvRampK1 |          |              |           |           |
 7 |        |           | LFO2Speed |       |            |            |          |            |          |            |            |           |            | EnvRampK2 |          |              |           |           |
 8 |        |           | LFO2Shape |       |            |            |          |            |          |            |            |           |            | Env Mode  |          |              |           |           |