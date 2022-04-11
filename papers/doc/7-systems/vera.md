# VERA

this is a video and sound generator chip used in the Commander X16, a modern 8-bit computer created by The 8-Bit Guy.
it has 16 channels of pulse/triangle/saw/noise and one stereo PCM channel.

currently Furnace does not support the PCM channel's stereo mode, though (except for panning).

# effects

- `20xx`: set waveform. the following values are accepted:
  - 0: pulse
  - 1: saw
  - 2: triangle
  - 3: noise
- `22xx`: set duty cycle. `xx` may go from 0 to 3F.
