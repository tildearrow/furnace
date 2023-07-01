# SNES instrument editor

these tabs are unique to the editor for SNES instruments.



# SNES

**Use envelope** enables the ADSR volume envelope. if it's on:

- **A**: attack rate.
- **D**: decay rate.
- **S**: sustain level.
- **D2**: decay rate during sustain.
- **R**: release rate.
- **Sustain/release mode**:
  - **Direct**: note release acts as note cut.
  - **Effective (linear decrease)**: after release, volume lowers by subtractions of 1/64 steps.
  - **Effective (exponential decrease)**: after release, volume decays exponentially. see [gain chart](../7-systems/snes.md).
  - **Delayed (write R on release)**: after release, waits until A and D have completed before starting exponential decrease.

if envelope is off, select gain mode as described below.



# Macros

- **Volume**: volume.
- **Arpeggio**: pitch in half-steps.
- **Noise Freq**: preset frequency of noise generator.
- **Waveform**: waveform.
- **Panning (left)**: output level of left channel.
- **Panning (right)**: output level of right channel.
- **Pitch**: fine pitch.
- **Special**: bitmap of flags.
  - invert left: inverts output of left channel.
  - invert right: inverts output of right channel.
  - pitch mod: modulates pitch using previous channel's output.
  - echo: enables echo.
  - noise: enables noise generator.
- **Gain**: sets mode and value of gain.
  - 0 - 127: direct gain from 0 to 127
  - 128 - 159: linear gain from -0 to -31
  - 160 - 191: exponential gain from -0 to -31
  - 192 - 223: linear gain from +0 to +31
  - 224 - 255: exponential gain from +0 to +31
