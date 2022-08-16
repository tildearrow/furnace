# ZX Spectrum Speaker

Rather than having a dedicated sound synthesizer, early ZX Spectrum models had one piezo beeper, controlled by Z80 CPU and ULA chip. It's capabilities should be on par with an IBM PC speaker... right?

Not really - very soon talented programmers found out ways to output much more than one square wave channel. A lot of ZX beeper routines do exist, but as of 0.6pre1 Furnace supports only one - Follin-like engine with 6 channels of narrow pulse wave and click drums.

# effects

- `12xx`: set pulse width.
- `17xx`: trigger overlay drum.
  - `xx` is the sample number.
  - overlay drums are 1-bit and always play at 55930Hz (NTSC) or 55420Hz (PAL).
  - the maximum length is 2048!