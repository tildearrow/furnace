# ZX Spectrum beeper

rather than having a dedicated sound synthesizer, early ZX Spectrum models had one piezo beeper, controlled by Z80 CPU and ULA chip. its capabilities should be on par with an IBM PC speaker... right?

not really - very soon talented programmers found out ways to output much more than one square wave channel. a lot of ZX beeper routines do exist, but Furnace only supports two engines:

- a Follin/SFX-like engine with 6 channels of narrow pulse wave and click drums.
- QuadTone: PWM-driven engine with 4ch of pulse wave with freely variable duty cycles and 1-bit PCM drums.

## effects

- **`12xx`**: set pulse width.
- **`17xx`**: trigger overlay drum.
  - `xx` is the sample number.
  - overlay drums are 1-bit and always play at 55930Hz (NTSC) or 55420Hz (PAL).
  - the maximum length is 2048!

## info

this chip uses the [Beeper](../4-instrument/beeper.md) or [Pokémon Mini/QuadTone](../4-instrument/pokemini.md) instrument editor.

## chip config

the following options are available in the Chip Manager window:

- **PAL**: run at 3.54MHz instead of 3.58MHz.

QuadTone provides this additional option:

- **Disable hissing**: disables TDM hissing.
