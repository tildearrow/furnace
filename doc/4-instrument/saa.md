# Philips SAA1099 instrument editor

the SAA1099 instrument editor consists of these macros:

- **Volume**: volume sequence.
- **Arpeggio**: pitch sequence.
- **Duty/Noise**: noise generator frequency. the following values are available:
  - `0`: high
  - `1`: mid
  - `2`: low
  - `3`: use frequency of channel 1 or 4 (depending on where the instrument plays).
  - keep in mind that the value affects an entire group! (first or last 3 channels)
- **Waveform**: selector between tone and noise.
- **Panning (left)**: output level for left channel.
- **Panning (right)**: output level for right channel.
- **Pitch**: fine pitch.
- **Envelope**: envelope generator settings:
  - **enable**: enables the envelope generator.
  - **timer strobe**: toggles whether the software timer will strobe the envelope activation bit.
    - this is useful to produce PWM effects.
  - **use timer**: toggles whether to use a software timer or lock to the frequency of channel 2 or 5 (depending on where the instrument plays).
    - in the timer mode, Furnace will periodically knock the envelope register to cycle envelopes.
  - **resolution**: increases the envelope generator pitch resolution.
  - **direction**: inverts the envelope.
  - **cut**: cuts the envelope (producing saw wave out of tri wave)
  - **loop**: toggles whether envelope is one-shot or constantly looping.
  - **mirror**: sets whether the right output will mirror the left one.
  - the envelope only has effect in channels 3 and 6.
  - the envelope pitch depends on the mode:
    - +3 octaves in timer strobe mode.
    - ±0 octaves in timer mode.
    - -3 octaves in channel 2/5 mode.
    - if strobing:
      - add 1 octave for shapes `000`-`011`.
      - if mirror is on, one output channel will be an octave higher.
    - if not strobing:
      - add 1 octave if the low resolution bit is on.
      - subtract 1 octave if the shape is a triangle wave.
