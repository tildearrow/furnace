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
- `F8xx`: single tick volume slide up.
- `F9xx`: single tick volume slide down.
- `FAxy`: fast volume slide (4x faster than `0Axy`).
  - if `x` is 0 then this is a slide down.
  - if `y` is 0 then this is a slide up.
- `FFxx`: end of song/stop playback.

additionally each chip has its own effects. [click here for more details](../7-systems/README.md).
