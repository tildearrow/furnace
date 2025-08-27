# VERA

this is a video and sound generator chip used in the Commander X16, a modern 8-bit computer created by the 8-Bit Guy. it has 16 channels of pulse/triangle/saw/noise and one stereo PCM channel.

currently Furnace does not support the PCM channel's stereo mode, though (except for panning).

depending on the computer's configuration, the VERA may appear alongside one [Yamaha OPM](ym2151.md) or two [Yamaha OPL3](opl.md) chips.


## effects

- `20xx`: **set waveform.**
  - `0`: pulse
  - `1`: saw
  - `2`: triangle
  - `3`: noise
- `22xx`: **set duty cycle.**
  - range is `0` to `3F`.
- `EExx`: **ZSM synchronization event.**
  - `xx` is the event payload. this has no effect in how the music is played in Furnace, but the ZSMKit library for the Commander X16 interprets these events inside ZSM files and optionally triggers a callback routine. this can be used, for instance, to cause game code to respond to beats or at certain points in the music.

## info

this chip uses the [VERA](../4-instrument/vera.md) and [Generic Sample](../4-instrument/sample.md) instrument editors.

## chip config

the following options are available in the Chip Manager window:

- **Chip revision**: sets which version of the chip's firmware to use.
  - **Initial release**: all earlier versions.
  - **V 47.0.2 (9-bit volume)**: introduces a slightly different volume table.
  - **V 48.0.1 (Tri/Saw PW XOR)**: adds the ability to XOR the triangle or sawtooth waveforms with a pulse wave.
  - **X16 Emu R49 (Noise freq fix)**: fixes an emulation bug that previously doubled the noise frequency. default.
