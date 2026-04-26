# effect list

some of the effect numbers are taken from ProTracker / FastTracker 2.

however, effects are continuous (unless specified), which means you only need to type it once and then stop it with an effect value of `00` or no effect value at all.

## volume

- `0Axy`: **Volume slide.**
  - if `x` is 0 then this slides volume down by `y` each tick.
  - if `y` is 0 then this slides volume up by `x` each tick.
- `FAxy`: **Fast volume slide.** same as `0Axy` above but 4× faster.
- `F3xx`: **Fine volume slide up.** same as `0Ax0` but 64× slower.
- `F4xx`: **Fine volume slide down.** same as `0A0x` but 64× slower.
- `F8xx`: **Single tick volume up.** adds `x` to volume.
- `F9xx`: **Single tick volume down.** subtracts `x` from volume.
  - ---
- `D3xx`: **Volume portamento.** slides the volume to the one specified in the volume column. `x` is the slide speed.
  - a volume _must_ be present with this effect for it to work.
- `D4xx`: **Volume portamento (fast).** like `D3xx` but 4× faster.
  - ---
- `07xy`: **Tremolo.** changes volume to be "wavy" with a sine LFO. `x` is the speed. `y` is the depth.
  - tremolo is downward only.
  - maximum tremolo depth is -60 volume steps.
  - ---
- `DCxx`: **Delayed mute.** sets channel volume to 0 after `xx` ticks.

## pitch

- `E5xx`: **Set pitch.** `00` is -1 semitone, `80` is base pitch, `FF` is nearly +1 semitone.
- `01xx`: **Pitch slide up.**
- `02xx`: **Pitch slide down.**
- `F1xx`: **Single tick pitch up.**
- `F2xx`: **Single tick pitch down.**
  - ---
- `03xx`: **Portamento.** slides the currently playing note's pitch toward the new note. `x` is the slide speed.
  - a note _must_ be present with this effect for it to work.
  - the effect stops automatically when it reaches the new note.
- `E1xy`: **Note slide up.** `x` is the speed, while `y` is how many semitones to slide up.
- `E2xy`: **Note slide down.** `x` is the speed, while `y` is how many semitones to slide down.
  - ---
- `EAxx`: **Toggle legato.** while on, new notes instantly change the pitch of the currently playing sound instead of starting it over.
- `E6xy`: **Quick legato (compatibility).** transposes note by `y` semitones after `x` ticks.
  - if `x` is between 0 and 7, it transposes up.
  - if `x` is between 8 and F, it transposes down.
- `E8xy`: **Quick legato up**. transposes note up by `y` semitones after `x` ticks.
- `E9xy`: **Quick legato down**. transposes note down by `y` semitones after `x` ticks.
- `00xy`: **Arpeggio.** this effect produces a rapid cycle between the current note, the note plus `x` semitones and the note plus `y` semitones.
  - as an example, start with a chord of C-3, G-3, and D#4. the G-3 and D#4 are 7 and 15 semitones higher than the root note, so the corresponding effect is `007F`.
- `E0xx`: **Set arpeggio speed.** this sets the number of ticks between arpeggio values. default is 1.
  - ---
- `04xy`: **Vibrato.** makes the pitch oscillate. `x` is the speed, while `y` is the depth.
  - maximum vibrato depth is ±1 semitone.
- `E3xx`: **Set vibrato shape.** `xx` may be one of the following:
  - `00`: sine (default)
  - `01`: sine (upper portion only)
  - `02`: sine (lower portion only)
  - `03`: triangle
  - `04`: ramp up
  - `05`: ramp down
  - `06`: square
  - `07`: random
  - `08`: square (up)
  - `09`: square (down)
  - `0a`: half sine (up)
  - `0b`: half sine (down)
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
  - ---
- `83xy`: **Panning slide.**
  - if `y` is 0 then this pans to the left by `x` each tick.
  - if `x` is 0 then this pans to the right by `y` each tick.
  - be noted that panning macros override this effect.
- `84xy`: **Panbrello.** makes panning oscillate. `x` is the speed, while `y` is the depth.
  - be noted that panning macros override this effect.

## time

- `09xx`: **Set speed/groove.** if no grooves are defined, this sets speed. if alternating speeds are active, this sets the first speed.
- `0Fxx`: **Set speed 2.** during alternating speeds or a groove, this sets the second speed.
  - ---
- `Cxxx`: **Set tick rate.** changes tick rate to `xxx` Hz (ticks per second).
  - `xxx` may be from `000` to `3FF`.
- `F0xx`: **Set BPM.** changes tick rate according to beats per minute. range is `01` to `FF`.
  - ---
- `FDxx`: **Set virtual tempo numerator.** sets the virtual tempo's numerator to the effect value.
- `FExx`: **Set virtual tempo denominator.** sets the virtual tempo's denominator to the effect value.
  - ---
- `0Bxx`: **Jump to order.** `x` is the order to play after the current row.
  - this marks the end of a loop with order `x` as the loop start.
- `0Dxx`: **Jump to next pattern.** skips the current row and remainder of current order. `x` is the row at which to start playing the next pattern.
  - this can be used to shorten the current order as though it had a different pattern length.
- `FFxx`: **Stop song.** stops playback and ends the song. `x` is ignored.

## note

- `0Cxx`: **Retrigger.** repeats current note every `xx` ticks.
  - this effect is not continuous; it must be entered on every row.
- `ECxx`: **Note cut.** triggers note off after `xx` ticks. this triggers key off in FM/hardware envelope chips, or cuts the note otherwise.
- `EDxx`: **Note delay.** delays note by `x` ticks.
- `FCxx`: **Note release.** releases current note after `xx` ticks. this releases macros and triggers key off in FM/hardware envelope chips.
- `E7xx`: **Macro release.** releases macros after `xx` ticks. this does not trigger key off.

## sample offset

these effects make the current playing sample on the channel jump to a specific position.
only some chips support this effect.

sample offset is a 24-bit (3 byte) number.

- `90xx`: **Set sample offset (first byte).**
- `91xx`: **Set sample offset (second byte).**
- `92xx`: **Set sample offset (third byte).**

you may use these effects simultaneously in a row.

if you do not set a byte, its most recent value will be used.

in previous versions of Furnace a `9xxx` effect existed which set the sample position to `$xxx00` (`xxx` was effectively multiplied by 256). this maps to `920x 91xx` in current Furnace.

## other

- `EExx`: **Send external command.**
  - this effect is currently incomplete.
- `F5xx`: **Disable macro.**
- `F6xx`: **Enable macro.**
- `F7xx`: **Restart macro.**
  - see macro table at the end of this document for possible values.

additionally, [each chip has its own effects](../7-systems/README.md).

## macro table

ID   | macro           |  ID  | op 1   |  ID  | op 2   |  ID  | op 3   |  ID  | op 4   |
:---:|-----------------|:----:|--------|:----:|--------|:----:|--------|:----:|--------|
`00` | Volume          | `20` | AM     | `40` | AM     | `60` | AM     | `80` | AM     |
`01` | Arpeggio        | `21` | AR     | `41` | AR     | `61` | AR     | `81` | AR     |
`02` | Duty/Noise      | `22` | DR     | `42` | DR     | `62` | DR     | `82` | DR     |
`03` | Waveform        | `23` | MULT   | `43` | MULT   | `63` | MULT   | `83` | MULT   |
`04` | Pitch           | `24` | RR     | `44` | RR     | `64` | RR     | `84` | RR     |
`05` | extra 1         | `25` | SL     | `45` | SL     | `65` | SL     | `85` | SL     |
`06` | extra 2         | `26` | TL     | `46` | TL     | `66` | TL     | `86` | TL     |
`07` | extra 3         | `27` | DT2    | `47` | DT2    | `67` | DT2    | `87` | DT2    |
`08` | extra A (ALG)   | `28` | RS     | `48` | RS     | `68` | RS     | `88` | RS     |
`09` | extra B (FB)    | `29` | DT     | `49` | DT     | `69` | DT     | `89` | DT     |
`0A` | extra C (FMS)   | `2A` | D2R    | `4A` | D2R    | `6A` | D2R    | `8A` | D2R    |
`0B` | extra D (AMS)   | `2B` | SSG-EG | `4B` | SSG-EG | `6B` | SSG-EG | `8B` | SSG-EG |
`0C` | Panning (left)  | `2C` | DAM    | `4C` | DAM    | `6C` | DAM    | `8C` | DAM    |
`0D` | Panning (right) | `2D` | DVB    | `4D` | DVB    | `6D` | DVB    | `8D` | DVB    |
`0E` | Phase Reset     | `2E` | EGT    | `4E` | EGT    | `6E` | EGT    | `8E` | EGT    |
`0F` | extra 4         | `2F` | KSL    | `4F` | KSL    | `6F` | KSL    | `8F` | KSL    |
`10` | extra 5         | `30` | SUS    | `50` | SUS    | `70` | SUS    | `90` | SUS    |
`11` | extra 6         | `31` | VIB    | `51` | VIB    | `71` | VIB    | `91` | VIB    |
`12` | extra 7         | `32` | WS     | `52` | WS     | `72` | WS     | `92` | WS     |
`13` | extra 8         | `33` | KSR    | `53` | KSR    | `73` | KSR    | `93` | KSR    |

the interpretation of duty, wave, and extra macros depends on chip/instrument type:

ex | FM         | OPLL          | OPM        | OPZ         |
:-:|------------|---------------|------------|-------------|
 D | Noise Freq |               | Noise Freq |             |
 W |            | Patch         | LFO Shape  | LFO Shape   |
 1 |            |               | AM Depth   | AM Depth    |
 2 |            |               | PM Depth   | PM Depth    |
 3 | LFO Speed  |               | LFO Speed  | LFO Speed   |
 A | Algorithm  | Sustain       | Algorithm  | Algorithm   |
 B | Feedback   | Feedback      | Feedback   | Feedback    |
 C | LFO > Freq | OP2 Half Sine | LFO > Freq | LFO > Freq  |
 D | LFO > Amp  | OP1 Half Sine | LFO > Amp  | LFO > Amp   |
 4 | OpMask     |               | OpMask     |             |
 5 |            |               |            | AM Depth 2  |
 6 |            |               |            | PM Depth 2  |
 7 |            |               |            | LFO Speed 2 |
 8 |            |               |            | LFO Shape 2 |

ex | ES5506            | MSM6258       | QSound        | SNES       |
:-:|-------------------|---------------|---------------|------------|
 D | Filter Mode       | Freq Divider  | Echo Level    | Noise Freq |
 W |                   |               |               | Waveform   |
 1 | Filter K1         | Clock Divider | Echo Feedback | Special    |
 2 | Filter K2         |               | Echo Length   | Gain       |
 3 | Envelope length   |               |               |            |
 A | Control           |               |               |            |
 B | Outputs           |               |               |            |
 C |                   |               |               |            |
 D |                   |               |               |            |
 4 | Left Volume Ramp  |               |               |            |
 5 | Right Volume Ramp |               |               |            |
 6 | Filter K1 Ramp    |               |               |            |
 7 | Filter K2 Ramp    |               |               |            |
 8 |                   |               |               |            |

ex | FDS       | Namco 163   | X1-010        |
:-:|-----------|-------------|---------------|
 D |           | Wave Pos    |               |
 W | Waveform  | Waveform    | Waveform      |
 1 | Mod Depth | Wave Length | Envelope Mode |
 2 | Mod Speed |             | Envelope      |
 3 |           |             | AutoEnv Num   |
 A |           |             | AutoEnv Den   |
 B |           |             |               |
 C |           |             |               |
 D |           |             |               |
 4 |           |             |               |
 5 |           |             |               |
 6 |           |             |               |
 7 |           |             |               |
 8 |           |             |               |

ex | AY-3-8910    | AY8930         | Lynx      | MSM5232      | SAA1099  |
:-:|--------------|----------------|-----------|--------------|----------|
 D | Noise Freq   | Noise Freq     | Duty/Int  | Group Ctrl   |          |
 W | Waveform     | Waveform       |           |              | Waveform |
 1 |              | Duty           | Load LFSR | Group Attack | Envelope |
 2 | Envelope     | Envelope       |           | Group Decay  |          |
 3 | AutoEnv Num  | AutoEnv Num    |           | Noise        |          |
 A | AutoEnv Den  | AutoEnv Den    |           |              |          |
 B |              | Noise AND Mask |           |              |          |
 C |              | Noise OR Mask  |           |              |          |
 D |              |                |           |              |          |
 4 | Force Period | Force Period   |           |              |          |
 5 | Env Period   | Env Period     |           |              |          |
 6 |              |                |           |              |          |
 7 |              |                |           |              |          |
 8 |              |                |           |              |          |

ex | C64           | SID2          | SID3               | Sound Unit        |
:-:|---------------|---------------|--------------------|-------------------|
 D | Duty          | Duty          | Duty               | Duty              |
 W | Waveform      | Waveform      | Waveform           | Waveform          |
 1 | Filter Mode   | Filter Mode   | Special            | Cutoff            |
 2 | Resonance     | Resonance     | Attack             | Resonance         |
 3 | Filter Toggle | Filter Toggle | Decay              | Control           |
 A | Cutoff        | Cutoff        | Special Wave       |                   |
 B |               |               | Phase Mod Source   |                   |
 C |               | Noise Mode    | Ring Mod Source    |                   |
 D |               | Wave Mix      | Hard Sync Source   |                   |
 4 | Special       | Special       | Sustain            | Phase Reset Timer |
 5 | Attack        | Attack        | Sustain Rate       |                   |
 6 | Decay         | Decay         | Release            |                   |
 7 | Sustain       | Sustain       | Noise LFSR bits    |                   |
 8 | Release       | Release       | Wave Mix           |                   |

SID3 instruments also use some of the FM operator macros in the main macros list:

  ex   | SID3              |
:-----:|-------------------|
OP1 AM | Key On/Off        |
OP2 AM | Noise Phase Reset |
OP3 AM | Envelope Reset    |
OP4 AM |                   |
OP1 AR |                   |
OP2 AR | 1-bit Noise       |
OP3 AR | Channel inversion |
OP4 AR | Feedback          |

SID3 instruments use FM operator macros for filters:

 ex  | SID3                      |
:---:|---------------------------|
 D2R | Cutoff                    |
 DAM | Resonance                 |
 DR  | Filter Toggle             |
 DT2 | Distortion Level          |
 DT  | Output Volume             |
 DVB | Channel Input Connection  |
 EGT | Channel Output Connection |
 KSL | Connection Matrix Row     |
 KSR | Filter Mode               |
