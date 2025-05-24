# Watara Supervision

a failed competitor of Game Boy, straight from Taiwan. released in 1992, it had a tilting screen, $50 price tag, very fast 4 MHz 6502-like CPU, framebuffer graphics and sound capabilities similar to Game Boy.

these are 2 pulse wave channels (same as on GB), barely working PCM channel and noise channel, also not unlike Game Boy. no hardware envelopes or zombie mode, though.

## effects

- `12xx`: **set duty cycle/noise mode.**
  - range is `0` to `3` for pulse and `0` to `1` for noise.

## info

this chip uses the [Watara Supervision](../4-instrument/watarasv.md) and [Generic Sample](../4-instrument/sample.md) instrument editors.

### sample info

sample channel is a 4-bit DMA channel with 4 frequencies assigned to sample octaves (C2, C3, C4, C5; C, C#, D, D# in Furnace respectively). max sample size is 4 kilobytes (8192 samples).

## chip config

the following options are available in the Chip Manager window:

- **Swap noise duty cycles**: enabled by default. when enabled, short noise is on odd-indexed duty cycles, like on Game Boy, rather than even.
- **Stereo pulse waves**: disabled by default. when enabled, it forces pulse channel 1 to the right output channel and second pulse to the left output channel.
