# tildearrow Sound Unit instrument editor

this instrument editor has two tabs.

## Sound Unit

for sample settings, see [the Sample instrument editor](sample.md).

the differences are:
- the lack of an "Use wavetable" option
- the presence of a "Use sample" one
- the presence of a "**Switch roles of frequency and phase reset timer**" option. when enabled, this writes frequency to the phase reset timer register rather than the frequency register
  - this may be used to create sync-like effects.

### hardware sequence

Furnace provides a sequencer for volume/envelope/cutoff sweeps.

the sequence consists of a list of "commands".

the `+` button adds a new command, which may be one of the following:

- **Volume Sweep**: configure a volume sweep.
  - Period: number of chip samples between sweep ticks. higher values represent slower sweeps.
  - Amount: how many volume units to add/subtract on each sweep tick.
  - Bound: sets the minimum/maximum volume.
    - volume values between 128-255 are invalid and may result in glitches.
    - if Loop is enabled, this is ignored.
  - Up/Down: sets the sweep's direction.
  - Loop: the sweep will repeat if this is enabled.
  - Flip: change the sweep's direction upon reaching floor/ceiling.
    - Loop must be enabled for this to work.
  - it is recommended not to use a volume macro as it may conflict with volume sweep.
- **Frequency Sweep**: configure a frequency sweep.
  - Period: number of chip samples between sweep ticks. higher values represent slower sweeps.
  - Amount: how many frequency units to add/subtract on each sweep tick.
  - Bound: sets the high byte of maximum/minimum frequency.
    - be careful with up sweeps! if the bound is set too high, the sweep may overflow.
  - Up/Down: sets the sweep's direction.
- **Cutoff Sweep**: configure a cutoff sweep.
  - Period: number of chip samples between sweep ticks. higher values represent slower sweeps.
  - Amount: how many cutoff units to add/subtract on each sweep tick.
  - Bound: sets the high byte of maximum/minimum cutoff.
    - values above 64 are impractical.
  - Up/Down: sets the sweep's direction.

- **Wait**: waits a specific number of ticks.
- **Wait for Release**: waits until the note is released with `===` or `REL`.
- **Loop**: goes to a previous position in the sequence.
- **Loop until Release**: same as Loop, but doesn't have effect after releasing the note.

each command in the sequence is represented in three columns:

- **Tick**: the tick this command will execute, followed by position in the sequence.
- **Command**: the command and its parameters.
- **Move/Remove**: allows you to move the command, or remove it.

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
