# Nanao GA20

## Summary

- 4 voice PCM
  - 8 bit unsigned, with end marker
  - total accessible memory: 1 Mbyte

## Source code

- ga20.hpp: Base header
  - ga20.cpp: Source emulation core

## TODO

- undocumented/unverified registers

## Description

It's a 4 voice PCM chip that was used on Irem's latest hardwares, and succeeding their previous single voice sample players.

It's accessing is seems like 8 bit, but Irem always paired with 16 bit V35 CPU.

## Register layout

```
    Address Bits      R/W Description
            7654 3210
    0x0-0x7 Voice 0
    0x0     xxxx xxxx   W Start address bit 4-11
    0x1     xxxx xxxx   W Start address bit 12-19
    0x2     xxxx xxxx   W End address bit 4-11
    0x3     xxxx xxxx   W End address bit 12-19
    0x4     xxxx xxxx   W Pitch
    0x5     xxxx xxxx   W Volume
    0x6     ---- --x-   W Keyon/off
    0x7     ---- ---x R   Busy

    0x8-0xF Voice 1
    0x10-0x17 Voice 2
    0x18-0x1F Voice 3
```

## Frequency calculation

```
    Frequency: Input clock / 4 * (256 - Pitch)
```
