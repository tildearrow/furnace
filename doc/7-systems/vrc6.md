# Konami VRC6

the most popular expansion chip to the Famicom's sound system.

the chip has 2 pulse wave channels and one sawtooth channel.
volume register is 4 bit for pulse wave and 6 bit for sawtooth, but sawtooth output is corrupted when volume register value is too high. because this register is actually an 8 bit accumulator, its output may wrap around.

for that reason, the sawtooth channel has its own instrument type. setting volume macro and/or pattern editor volume setting too high (above 42/2A) may distort the waveform.

pulse wave duty cycle is 8-level. it can be ignored and it has potential for DAC at this case: volume register in this mode is DAC output and it can be PCM playback through this mode.
Furnace supports this routine for PCM playback, but it consumes a lot of CPU time in real hardware (even if conjunction with VRC6's integrated IRQ timer).

## effects

these effects only are effective in the pulse channels.

- `12xx`: **set duty cycle.** range is `0` to `7`.
- `17xx`: **toggle LEGACY sample mode.**
  - **this effect exists only for compatibility reasons! its use is NOT recommented. use Sample type instruments instead.**

## info

this chip uses the [VRC6](../4-instrument/vrc6.md) and [VRC6 (saw)](../4-instrument/vrc6.md) instrument editors.

## chip config

the following options are available in the Chip Manager window:

- **Clock rate**: sets the rate at which the chip will run.
