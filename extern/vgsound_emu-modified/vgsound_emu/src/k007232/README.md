# Konami K007232

## Summary

- 2 voice PCM
  - 7 bit unsigned, with end marker
  - total accessible memory: 128 Kbyte per bank
    - Per-voice bankswitchable via E clock
- External 8 bit I/O (usually for volume)

## Source code

- k007232.hpp: Base header
  - k007232.cpp: Source emulation core

## Description

It's Konami's one of custom PCM sound chip, Used at their arcade hardware at mid-80s to early-90s.

It has 2 channel of PCM, these are has its own output pins...just 7 LSB of currently fetched data.

PCM Sample format is unique, 1 MSB is end marker and 7 LSB is actually output. (unsigned format)

The chip itself is DACless, so Sound output and mixing control needs external logics and sound DAC.

## Register layout

```
 Address Bits      R/W Description
         7654 3210

 0x0     xxxx xxxx   W Channel 0 Pitch bit 0-7
 0x1     --x- ----   W Channel 0 4 bit Frequency mode
         ---x ----   W Channel 0 8 bit Frequency mode
         ---- xxxx   W Channel 0 Pitch bit 8-11

 0x2     xxxx xxxx   W Channel 0 Start address bit 0-7
 0x3     xxxx xxxx   W Channel 0 Start address bit 8-15
 0x4     ---- ---x   W Channel 0 Start address bit 16

 0x5               R/W Channel 0 Key on trigger (R/W)

 0x6     xxxx xxxx   W Channel 1 Pitch bit 0-7
 0x7     --x- ----   W Channel 1 4 bit Frequency mode
         ---x ----   W Channel 1 8 bit Frequency mode
         ---- xxxx   W Channel 1 Pitch bit 8-11

 0x8     xxxx xxxx   W Channel 1 Start address bit 0-7
 0x9     xxxx xxxx   W Channel 1 Start address bit 8-15
 0xa     ---- ---x   W Channel 1 Start address bit 16

 0xb               R/W Channel 1 Key on trigger (R/W)

 0xc     xxxx xxxx   W External port write (w/SLEV pin, Usually for volume control)

 0xd     ---- --x-   W Channel 1 Loop enable
         ---- ---x   W Channel 0 Loop enable
```

## Frequency calculation

(Guesswork in 8/4 bit Frequency mode)

```
 if 8 bit Frequency mode then
  Frequency: Input clock / 4 * (256 - (Pitch bit 0 - 7))
 else if 4 bit Frequency mode then
  Frequency: Input clock / 4 * (16 - (Pitch bit 8 - 11))
 else
  Frequency: Input clock / 4 * (4096 - (Pitch bit 0 - 11))
```

## Reference

<https://github.com/furrtek/VGChips/tree/master/Konami/007232>
