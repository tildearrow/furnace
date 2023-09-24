# SNES instrument editor

these four tabs are unique to the editor for SNES instruments.

## Sample

for sample settings, see [the Sample instrument editor](sample.md).

## SNES

**Use envelope** enables the ADSR volume envelope. if it is on:

- **A**: attack rate.
- **D**: decay rate.
- **S**: sustain level.
- **D2**: decay rate during sustain.
  - only appears when Sustain/release mode is Effective or Delayed.
- **R**: release rate.
- **Sustain/release mode**:
  - **Direct**: note release acts as note cut.
  - **Effective (linear decrease)**: after release, volume lowers by subtractions of 1/64 steps.
  - **Effective (exponential decrease)**: after release, volume decays exponentially. see [gain chart](../7-systems/snes.md).
  - **Delayed (write R on release)**: after release, waits until A and D have completed before starting release.

if envelope is off:
- **Gain Mode**: selects gain mode.
  - **Direct**: direct gain from 0 to 127.
  - **Decrease (linear)**: linear gain from -0 to -31.
  - **Decrease (logarithmic)**: exponential gain from -0 to -31.
    - note: using decrease modes will not produce any sound unless a Gain macro is set. the first tick must be the initial gain, and the second tick must be the decrease gain value. gain values are as described in the Macros section below.
  - **Increase (linear)**: linear gain from +0 to +31.
  - **Increase (bent line)**: inverse exponential gain from +0 to +31.
- **Gain**: value of gain.

## Wavetable

this allows you to enable and configure the Furnace wavetable synthesizer. see [this page](wavesynth.md) for more information.

only active when Use wavetable is enabled in the Sample tab.

## Macros

- **Volume**: volume sequence.
- **Arpeggio**: pitch sequence.
- **Noise Freq**: frequency of noise generator.
  - note: global!
- **Waveform**: waveform.
  - only effective when Use wavetable is enabled.
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
  - 0 to 127: direct gain from 0 to 127.
  - 128 to 159: linear gain from -0 to -31 (decrease linear).
  - 160 to 191: exponential gain from -0 to -31 (decrease exponential).
  - 192 to 223: linear gain from +0 to +31 (increase linear).
  - 224 to 255: exponential gain from +0 to +31 (increase bent line).
