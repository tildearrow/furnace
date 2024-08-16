# POKEY

a sound and input chip developed by Atari for their 8-bit computers (Atari 400, 800, XL/XE and so on). 4 channels of signature Atari sounds.

## effects

- `10xx`: **set waveform.**
  - 0: harsh noise (poly5+17)
  - 1: square buzz (poly5)
  - 2: weird noise (poly4+5)
  - 3: square buzz (poly5)
  - 4: soft noise (poly17)
  - 5: square
  - 6: bass (poly4)
  - 7: buzz (poly4)
- `11xx`: **set AUDCTL.** `xx` is a bitmask.
  - bit 7: 9-bit poly mode. shortens noise.
  - bit 6: high channel 1 clock (~1.79MHz on NTSC).
    - overrides 15KHz mode.
  - bit 5: high channel 3 clock (~1.79MHz on NTSC).
    - overrides 15KHz mode.
  - bit 4: join channels 1 and 2 for a wide period range.
    - use with conjunction with bit 6.
    - channel 2 becomes inaccessible when this is on.
  - bit 3: join channels 3 and 4 for a wide period range.
    - use with conjunction with bit 5.
    - channel 4 becomes inaccessible when this is on.
  - bit 2: high-pass filter (channels 1 and 3).
    - filtered output on channel 1 (I suggest you to set channel 3 volume to 0).
    - use for PWM effects (not automatic!).
  - bit 1: high-pass filter (channels 2 and 4).
    - filtered output on channel 2 (I suggest you to set channel 4 volume to 0).
    - use for PWM effects (not automatic!).
  - bit 0: 15KHz mode.
- `12xx`: **toggle two-tone mode.**
  - when enabled, channel 2 modulates channel 1. I don't know how, but it does.
  - only on ASAP core.

## info

this chip uses the [POKEY](../4-instrument/pokey.md) instrument editor.

when two channels are joined for filtered output, the channel bar will show `filter` on a bracket tying them together.

when two channels are joined for wide period range, the channel bar will show `16-bit` on a bracket tying them together.

## chip config

the following options are available in the Chip Manager window:

- **PAL**: run the chip at PAL clock (1.77MHz) instead of NTSC (1.79MHz).
