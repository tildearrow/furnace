# Atari Lynx/MIKEY

the Atari Lynx is a 16 bit handheld console developed by (obviously) Atari Corporation, and initially released in September of 1989, with the worldwide release being in 1990.

while it was an incredible handheld for the time (and a lot more powerful than a Game Boy), it unfortunately meant nothing in the end due to the Lynx being a market failure, and ending up as one of the things that contributed to the downfall of Atari.

although the Lynx is still getting (rather impressive) homebrew developed for it, that does not mean the Lynx is a popular system at all.

the Atari Lynx has a 6502-based CPU with a sound part (this chip is known as MIKEY). it has the following sound capabilities:
- 4 channels of LFSR-based sound, which can be modulated with different frequencies (×0, ×1, ×2, ×3, ×4, ×5, ×7, ×10, and ×11) to create square waves and wavetable-like results.
  - likewise, when a lot of the modulators are activated, this can provide a "pseudo-white noise"-like effect, which can be useful for drums and sound effects.
- hard stereo panning capabilities via the `08xx` effect command.
- four 8-bit DACs (Digital to Analog Converter), one for each voice. this allows for sample playback (at the cost of CPU time and memory).
- a variety of pitches to choose from, and they go from 32Hz to "above the range of human hearing", according to Atari.

# effects

- `3xxx`: Load LFSR (0 to FFF).
  - this is a bitmask.
  - for it to work, duty macro in instrument editor must be set to some value. without it LFSR will not be fed with any bits.
