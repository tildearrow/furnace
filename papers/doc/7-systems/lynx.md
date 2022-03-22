# Atari Lynx/MIKEY

The Atari Lynx is a 16 bit handheld console developed by (obviously) Atari Corporation, and initially released in September of 1989, with the worldwide release being in 1990.

The Lynx, while being an incredible handheld for the time (and a lot more powerful than a Game Boy), unfortunately meant nothing in the end due to the Lynx being a market failure, and ending up as one of the things that contributed to the downfall of Atari.

Although the Lynx is still getting (rather impressive) homebrew developed for it, it does not mean that the Lynx is a popular system at all.

The Atari Lynx's custom sound chip and CPU (MIKEY) is a 6502-based 8 bit CPU running at 16MHz, however this information is generally not useful in the context of Furnace.

## Sound capabilities

 - The MIKEY has 4 channels of square wave-based sound, which can be modulated with different frequencies (×0, ×1, ×2, ×3, ×4, ×5, ×7, ×10, and ×11) to create wavetable-like results.
 - Likewise, when a lot of the modulators are activated, this can provide a "pseudo-white noise"-like effect, whoch can be useful for drums and sound effects.
 - The MIKEY also has hard stereo panning capabilities via the `08xx` effect command.
 - The MIKEY has four 8-bit DACs (Digital to Analog Converter) — one for each voice — that essentially mean you can play samples on the MIKEY (at the cost of CPU time and memory).
 - The MIKEY also has a variety of pitches to choose from, and they go from 32Hz to "above the range of human hearing", according to Atari.

## Effect commands
 - `3xxx`: Load LFSR (0 to FFF).
  - this is a bitmask.
  - for it to work, duty macro in instrument editor must be set to some value, without it LFSR will not be fed with any bits.
