# Atari Lynx/MIKEY

The Atari Lynx is a 16 bit handheld console developed by (obviously) Atari Corporation, and initially released in September of 1989, with the worldwide release being in 1990.

The Atari Lynx's custom sound chip and CPU (MIKEY) is a 6502-based 8 bit CPU running at 16MHz, however this information is generally not useful in the context of Furnace.

## Sound capabilities

 - The MIKEY has 4 channels of square wave-based sound, which can be modulated with different frequencies (×0, ×1, ×2, ×3, ×4, ×5, ×7, ×10, and ×11) to create wavetable-like results.
 - The MIKEY also has hard stereo panning capabilities via the `08xx` effect command.
 - The MIKEY has four 8-bit DACs (Digital to Analog Converter) — one for each voice — that essentially mean you can play samples on the MIKEY (at the cost of CPU time and memory).
 - The MIKEY also has a variety of pitches to choose from, and they go from 32Hz to "above the range of human hearing", according to Atari.
