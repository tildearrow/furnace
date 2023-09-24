# NEC PC Engine instrument editor

the PCE instrument editor contains three tabs: Sample, Wavetable and Macros.

## Sample

for sample settings, see [the Sample instrument editor](sample.md).

the only differences are the lack of an "Use wavetable" option, and the presence of a "Use sample" one.

## Wavetable

this allows you to enable and configure the Furnace wavetable synthesizer. see [this page](wavesynth.md) for more information.

note: on PC Engine, using the wave synth may result in clicking and/or phase resets. by default Furnace attempts to mitigate this problem though.

## Macros

- **Volume**: volume sequence.
- **Arpeggio**: pitch sequence.
- **Noise**: enable noise mode.
  - only on channels 5 and 6.
- **Waveform**: wavetable sequence.
- **Panning (left)**: output level for left channel.
- **Panning (right)**: output level for right channel.
- **Pitch**: fine pitch.
- **Phase Reset**: trigger restart of waveform.
