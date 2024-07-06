# Namco WSG / Namco C15 / Namco C30

a family of wavetable synth sound chips used by Namco in their arcade machines (Pac-Man and later). waveforms are 4-bit, with 32-byte sample length.

everything starts with Namco WSG, which is a simple 3-channel wavetable with no extra frills. C15 is a much more advanced sound source with 8 channels, and C30 adds stereo output and noise mode.

## effects

- `10xx`: **change waveform.**
- `11xx`: **toggle noise mode.** _warning:_ only on C30.

## info

this chip uses the [Namco WSG](../4-instrument/wsg.md) instrument editor.

## chip config

the following options are available in the Chip Manager window:

- **Waveform storage mode**: selects whether RAM or ROM is connected to the chip.
  - RAM: connects RAM for unlimited waves.
  - ROM: connects ROM. authentic to hardware, but only the first 8 waves are loaded.

Namco C30 does not have the aforementioned option as it always uses RAM for waveforms. instead, it has this option:

- **Compatible noise frequencies**: for compatibility with old Furnace.
