# Dave

this is the sound chip used in the Enterprise 128 home computer of the '80s, which competed against other home computers in Europe such as the ZX Spectrum and Amstrad CPC.

Dave is very similar to POKEY in many aspects. it has most of the signature Atari sounds and POKEY-style high-pass filter.
it has 4 channels, of which 3 generate LFSR-based sounds and the last one is a noise channel which has five lengths and either runs at a fixed frequency, or steals the frequency of another channel.
these channels have ring modulation and the aforementioned high-pass filter capabilities. the noise one also has a pseudo-low-pass filter.
the pitch and volume resolutions are much greater than that of POKEY, with 4096 pitches and 64 volume levels.
it also has stereo output.
on top of that, there's a DAC mode which may be enabled for each side of the stereo output. this mode overrides sound generation output.

## effects

- `10xx`: **set waveform or noise length.**
  - the following waveforms apply in the first three channels:
    - 0: square
    - 1: bass
    - 2: buzz
    - 3: reed
    - 4: noise
  - if placed in the noise channel, `x` is a value from `0` to `3`.
- `11xx`: **set noise frequency source.**
  - 0: fixed frequency (~62.5KHz)
  - 1: channel 1
  - 2: channel 2
  - 3: channel 3
- `12xx`: **toggle high-pass with the next channel.**
- `13xx`: **toggle ring modulation with the channel that is located two channels ahead of this one.**
  - in the case of channel 1, it modulates with channel 3. channel 2 modulates with channel 4 and so on.
- `14xx`: **toggle "swap counters" mode.**
  - only in noise channel.
  - when enabled, the noise length is even shorter and has no effect.
- `15xx`: **toggle low-pass with channel 2.**
  - only in noise channel.
- `16xx`: **set global clock divider.**
  - 0: divide by 2.
  - 1: divide by 3.

## info

this chip uses the [Dave](../4-instrument/dave.md) instrument editor.

when two channels are joined due to high-pass filter, the channel bar will show `high` on a bracket tying them together.

when two channels are joined due to low-pass filter, the channel bar will show `low` on a bracket tying them together.

when two channels are joined for ring modulation, the channel bar will show `ring` on a bracket tying them together.
