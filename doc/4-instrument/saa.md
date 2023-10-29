# Philips SAA1099 instrument editor

the SAA1099 instrument editor consists of these macros:

- **Volume**: volume sequence.
- **Arpeggio**: pitch sequence.
- **Duty/Noise**: noise generator frequency. the following values are available:
  - `0`: high
  - `1`: mid
  - `2`: low
  - `3`: use frequency of channel 1 or 4 (depending on where the instrument plays).
- **Waveform**: selector between tone and noise.
- **Panning (left)**: output level for left channel.
- **Panning (right)**: output level for right channel.
- **Pitch**: fine pitch.
- **Envelope**: envelope generator settings:
  - **enable**: enables the envelope generator.
  - **N/A**: has no effect.
  - **fixed**: toggles whether to use a fixed, slow frequency or lock to the frequency of channel 2 or 5 (depending on where the instrument plays).
  - **resolution**: increases the envelope generator pitch resolution.
  - **direction**: inverts the envelope.
  - **cut**: cuts the envelope (producing saw wave out of tri wave)
  - **loop**: toggles whether envelope is one-shot or constantly looping.
  - **mirror**: sets whether the right output will mirror the left one.
  - the envelope only has effect in channels 3 and 6.
