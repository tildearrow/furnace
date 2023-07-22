# VERA

this is a video and sound generator chip used in the Commander X16, a modern 8-bit computer created by The 8-Bit Guy.
it has 16 channels of pulse/triangle/saw/noise and one stereo PCM channel.

currently Furnace does not support the PCM channel's stereo mode, though (except for panning).

# effects

- `20xx`: **set waveform.**
  - `0`: pulse
  - `1`: saw
  - `2`: triangle
  - `3`: noise
- `22xx`: **set duty cycle.** range is `0` to `3F`.
- `EExx`: **ZSM synchronization event.**
  - Where `xx` is the event payload. This has no effect in how the music is played in Furnace, but the ZSMKit library for the Commander X16 interprets these events inside ZSM files and optionally triggers a callback routine. This can be used, for instance, to cause game code to respond to beats or at certain points in the music.
