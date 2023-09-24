# effect list

some of the effect numbers are taken from ProTracker / FastTracker 2.

however, effects are continuous, which means you only need to type it once and then stop it with an effect value of `00` or no effect value at all.

## volume

- `0Axy`: **Volume slide.**
  - if `x` is 0 then this slides volume down by `y` each tick.
  - if `y` is 0 then this slides volume up by `x` each tick.
- `FAxy`: **Fast volume slide.** same as `0Axy` above but 4× faster.
- `F3xx`: **Fine volume slide up.** same as `0Ax0` but 64× slower.
- `F4xx`: **Fine volume slide down.** same as `0A0x` but 64× slower.
- `F8xx`: **Single tick volume slide up.** adds `x` to volume on first tick only.
- `F9xx`: **Single tick volume slide down.** subtracts `x` from volume on first tick only.
  - ---
- `07xy`: **Tremolo.** changes volume to be "wavy" with a sine LFO. `x` is the speed. `y` is the depth.
  - tremolo is downward only.
  - maximum tremolo depth is -60 volume steps.

## pitch

- `E5xx`: **Set pitch.** `00` is -1 semitone, `80` is base pitch, `FF` is nearly +1 semitone.
- `01xx`: **Pitch slide up.**
- `02xx`: **Pitch slide down.**
- `F1xx`: **Single tick pitch slide up.**
- `F2xx`: **Single tick pitch slide down.**
  - ---
- `03xx`: **Portamento.** slides the currently playing note's pitch toward the new note. `x` is the slide speed.
  - a note _must_ be present with this effect for it to work.
  - the effect stops automatically when it reaches the new note.
- `E1xy`: **Note slide up.** `x` is the speed, while `y` is how many semitones to slide up.
- `E2xy`: **Note slide down.** `x` is the speed, while `y` is how many semitones to slide down.
  - ---
- `EAxx`: **Toggle legato.** while on, new notes instantly change the pitch of the currently playing sound instead of starting it over.
- `00xy`: **Arpeggio.** after using this effect the channel will rapidly switch between semitone values of `note`, `note + x` and `note + y`.
- `E0xx`: **Set arpeggio speed.** this sets the number of ticks between arpeggio values. default is 1.
  - ---
- `04xy`: **Vibrato.** changes pitch to be "wavy" with a sine LFO. `x` is the speed, while `y` is the depth.
  - maximum vibrato depth is ±1 semitone.
- `E3xx`: **Set vibrato direction.** `xx` may be one of the following:
  - `00`: up and down. default.
  - `01`: up only.
  - `02`: down only.
- `E4xx`: **Set vibrato range** in 1/16th of a semitone. 

## panning

not all chips support these effects.

- `08xy`: **Set panning.** changes stereo volumes independently. `x` is the left channel and `y` is the right one.
- `88xy`: **Set rear panning.** changes rear channel volumes independently. `x` is the rear left channel and `y` is the rear right one.
- `81xx`: **Set volume of left channel** (from `00` to `FF`).
- `82xx`: **Set volume of right channel** (from `00` to `FF`).
- `89xx`: **Set volume of rear left channel** (from `00` to `FF`).
- `8Axx`: **Set volume of rear right channel** (from `00` to `FF`).
  - ---
- `80xx`: **Set panning (linear).** this effect behaves more like other trackers:
  - `00` is left.
  - `80` is center.
  - `FF` is right.

## time

- `09xx`: **Set speed/groove.** if no grooves are defined, this sets speed. if alternating speeds are active, this sets the first speed.
- `0Fxx`: **Set speed 2.** during alternating speeds or a groove, this sets the second speed.
  - ---
- `Cxxx`: **Set tick rate.** changes tick rate to `xxx` Hz (ticks per second).
  - `xxx` may be from `000` to `3FF`.
- `F0xx`: **Set BPM.** changes tick rate according to beats per minute. range is `01` to `FF`.
  - ---
- `0Bxx`: **Jump to order.** `x` is the order to play after the current row.
  - this marks the end of a loop with order `x` as the loop start.
- `0Dxx`: **Jump to next pattern.** skips the current row and remainder of current order. `x` is the row at which to start playing the next pattern.
  - this can be used to shorten the current order as though it had a different pattern length.
- `FFxx`: **Stop song.** stops playback and ends the song. `x` is ignored.

## note

- `0Cxx`: **Retrigger.** repeats current note every `xx` ticks.
  - this effect is not continuous; it must be entered on every row.
- `ECxx`: **Note cut.** ends current note after `xx` ticks. for FM instruments, it's equivalent to a "key off".
- `EDxx`: **Note delay.** delays note by `x` ticks.

## other

- `9xxx`: **Set sample position.** jumps current sample to position `xxx * 0x100`.
  - not all chips support this effect.
- `EBxx`: **Set LEGACY sample mode bank.** selects sample bank. used only for compatibility.
  - does not apply on Amiga.
- `EExx`: **Send external command.**
  - this effect is currently incomplete.
- `F5xx`: **Disable macro.**
- `F6xx`: **Enable macro.**
  - see macro table at the end of this document for possible values.

additionally, [each chip has its own effects](../7-systems/README.md).

## macro table

 ID  | macro
-----|-----------------------------
`00` | volume
`01` | arpeggio
`02` | duty/noise
`03` | waveform
`04` | pitch
`05` | extra 1
`06` | extra 2
`07` | extra 3
`08` | extra A (ALG)
`09` | extra B (FM)
`0A` | extra C (FMS)
`0B` | extra D (AMS)
`0C` | panning left
`0D` | panning right
`0E` | phase reset
`0F` | extra 4
`10` | extra 5
`11` | extra 6
`12` | extra 7
`13` | extra 8
|    | **operator 1 macros**
`20` | AM
`21` | AR
`22` | DR
`23` | MULT
`24` | RR
`25` | SL
`26` | TL
`27` | DT2
`28` | RS
`29` | DT
`2A` | D2R
`2B` | SSG-EG
`2C` | DAM
`2D` | DVB
`2E` | EGT
`2F` | KSL
`30` | SUS
`31` | VIB
`32` | WS
`33` | KSR
`40` | **operator 2 macros**
`60` | **operator 3 macros**
`80` | **operator 4 macros**

the interpretation of duty, wave and extra macros depends on chip/instrument type:

ex | FM     | OPM       | OPZ       | OPLL  | AY-3-8910  | AY8930     | Lynx     | C64        |
---|--------|-----------|-----------|-------|------------|------------|----------|------------|
 D | NoiseF | NoiseFreq |           |       | NoiseFreq  | NoiseFreq  | Duty/Int | Duty       |
 W |        | LFO Shape | LFO Shape | Patch | Waveform   | Waveform   |          | Waveform   |
 1 |        | AMD       | AMD       |       |            | Duty       |          | FilterMode |
 2 |        | PMD       | PMD       |       | Envelope   | Envelope   |          | Resonance  |
 3 | LFOSpd | LFO Speed | LFO Speed |       | AutoEnvNum | AutoEnvNum |          | Special    |
 A | ALG    | ALG       | ALG       |       | AutoEnvDen | AutoEnvDen |          |            |
 B | FB     | FB        | FB        |       |            | Noise AND  |          |            |
 C | FMS    | FMS       | FMS       |       |            | Noise OR   |          |            |
 D | AMS    | AMS       | AMS       |       |            |            |          |            |
 4 | OpMask | OpMask    |           |       |            |            |          | Test/Gate  |
 5 |        |           | AMD2      |       |            |            |          |            |
 6 |        |           | PMD2      |       |            |            |          |            |
 7 |        |           | LFO2Speed |       |            |            |          |            |
 8 |        |           | LFO2Shape |       |            |            |          |            |

ex | SAA1099  | X1-010     | Namco 163  | FDS       | Sound Unit | ES5506    | MSM6258  |
---|----------|------------|------------|-----------|------------|-----------|----------|
 D |          |            | Wave Pos   |           | Duty       | Filt Mode | FreqDiv  |
 W | Waveform | Waveform   | Waveform   | Waveform  | Waveform   |           |          |
 1 | Envelope | EnvMode    | WaveLen    | Mod Depth | Cutoff     | Filter K1 | ClockDiv |
 2 |          | Envelope   | WaveUpdate | Mod Speed | Resonance  | Filter K2 |          |
 3 |          | AutoEnvNum | WaveLoad W |           | Control    | Env Count |          |
 A |          | AutoEnvDen | WaveLoad P |           |            | Control   |          |
 B |          |            | WaveLoad L |           |            |           |          |
 C |          |            | WaveLoad T |           |            |           |          |
 D |          |            |            |           |            |           |          |
 4 |          |            |            |           | PResetTime | EnvRampL  |          |
 5 |          |            |            |           |            | EnvRampR  |          |
 6 |          |            |            |           |            | EnvRampK1 |          |
 7 |          |            |            |           |            | EnvRampK2 |          |
 8 |          |            |            |           |            | Env Mode  |          |

ex | QSound       | SNES      | MSM5232   |
---|--------------|-----------|-----------|
 D | Echo Level   | NoiseFreq | GroupCtrl |
 W |              | Waveform  |           |
 1 | EchoFeedback | Special   | GroupAtk  |
 2 | Echo Length  | Gain      | GroupDec  |
 3 |              |           | Noise     |
 A |              |           |           |
 B |              |           |           |
 C |              |           |           |
 D |              |           |           |
 4 |              |           |           |
 5 |              |           |           |
 6 |              |           |           |
 7 |              |           |           |
 8 |              |           |           |
