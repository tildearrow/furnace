# SID2

a fictional chip created by LTVA. the idea is to fix SID flaws and add more features, but not too much.

unlike SID, it has per-channel volume control, better ADSR envelope which doesn't have bugs, more waveform mixing modes and the ability to play tonal noise waves.

filter cutoff and resonance ranges were extended, as well as the frequency - finally the chip can hit B-7 note with default clock speed!

## effects

- `10xx`: **change wave.** lower 4 bits specify the wave:
  - `bit 0`: triangle
  - `bit 1`: saw
  - `bit 2`: pulse
  - `bit 3`: noise
- `11xx`: **set resonance.** `xx` may be a value between `00` and `FF`.
- `12xx`: **set filter mode.** the following values are accepted:
  - `00`: filter off
  - `01`: low pass
  - `02`: band pass
  - `03`: low+band pass
  - `04`: high pass
  - `05`: band reject/stop/notch
  - `06`: high+band pass
  - `07`: all pass
- `13xx`: **disable envelope reset for this channel.**
- `14xy`: **reset cutoff**:
  - if `x` is not 0: on new note
  - if `y` is not 0: now
  - this effect is not necessary if the instrument's cutoff macro is absolute.
- `15xy`: **reset duty cycle**:
  - if `x` is not 0: on new note
  - if `y` is not 0: now
  - this effect is not necessary if the instrument's duty macro is absolute.
- `16xy`: **change additional parameters.**
  - `x` may be one of the following:
    - `0`: attack (`y` from `0` to `F`)
    - `1`: decay (`y` from `0` to `F`)
    - `2`: sustain (`y` from `0` to `F`)
    - `3`: release (`y` from `0` to `F`)
    - `4`: ring modulation (`y` is `0` or `1`)
    - `5`: oscillator sync (`y` is `0` or `1`)
    - `6`: filter mode (`y` is `0` to `7`)
    - `7`: waveform mix mode (`y` is `0` to `3`)
    - `8`: noise mode (`y` is `0` to `3`)
    - `9`: phase reset (`y` is a discarded parameter and does not matter)
    - `A`: envelope key on/key off (`y` is `0` (trigger envelope release) or `1` (restart envelope again))
    - `B`: filter on/off (`y` is `0` (disable filter) or `1` (enable filter))
- `3xxx`: **set duty cycle.** `xxx` range is `000` to `FFF`.
- `4xxx`: **set cutoff.** `xxx` range is `000` to `FFF`.

## info

this chip uses the [SID2](../4-instrument/sid2.md) instrument editor.
