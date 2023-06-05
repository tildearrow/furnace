# Namco WSG | Namco C15 | Namco C30

a family of wavetable synth sound chips used by Namco in their arcade machines (Pacman and later). waveforms are 4-bit, with 32-byte sample length.

everything starts with Namco WSG, which is a simple 3-channel wavetable with no extra frills. C15 is a much more advanced sound source with 8 channels, and C30 adds stereo output and noise mode.

# effects

- `10xx`: change waveform.
- `11xx`: toggle noise mode (WARNING: only on C30).
