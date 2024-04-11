# tildearrow Sound Unit instrument editor

this instrument editor has two tabs.

## Sound Unit

for sample settings, see [the Sample instrument editor](sample.md).

the differences are:
- the lack of an "Use wavetable" option
- the presence of a "Use sample" one
- the presence of a "**Switch roles of frequency and phase reset timer**" option. when enabled, this writes frequency to the phase reset timer register rather than the frequency register
  - this may be used to create sync-like effects.

## Macros

- **Volume**: volume sequence.
- **Arpeggio**: pitch sequence.
- **Duty/Noise**: waveform duty cycle sequence.
- **Waveform**: select waveform.
  - `0`: pulse wave
  - `1`: sawtooth
  - `2`: sine wave
  - `3`: triangle wave
  - `4`: noise
  - `5`: periodic noise
  - `6`: XOR sine
  - `7`: XOR triangle
- **Panning**: stereo panning sequence.
- **Pitch**: fine pitch.
- **Phase Reset**: trigger restart of waveform.
- **Cutoff**: set filter cutoff.
- **Resonance**: set filter resonance.
  - values that are too high may distort the output!
- **Control**: filter parameter/ring mod sequence.
  - **band pass**: a band-pass filter. cutoff determines which part of the sound is heard (from bass to treble).
  - **high pass**: a high-pass filter. higher cutoff values result in a less "bassy" sound.
  - **low pass**: a low-pass filter. the lower the cutoff, the darker the sound.
  - **ring mod**: enable ring modulation with previous channel.
    - note: square wave goes from 0 to volume, so in that case it acts more like amplitude modulation.
- **Phase Reset Timer**: sets the phase reset timer.
  - if the "Switch roles of frequency and phase reset timer" option in the Sound Unit tab is enabled, this macro controls the frequency register instead.
