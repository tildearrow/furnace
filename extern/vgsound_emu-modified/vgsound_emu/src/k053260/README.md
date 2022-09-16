# Konami K053260

## Summary

- 4 voice DPCM or PCM
  - 8 bit signed PCM or 4 bit DPCM (unique type)
  - total accessible memory: 2 MByte, 64 KByte per sample
- CPU to CPU Communication
- CPU can be accessible k053260 memory through voice register
- 2 Stereo sound input, 1 Stereo output (YM3012 DAC compatible)

## Source code

- k053260.hpp: Base header
  - k053260.cpp: Source emulation core

## Description

It's one of Konami's custom PCM playback chip with CPU to CPU communication feature, and built in timer.

It's architecture is successed from K007232, but it features various enhancements:

it's expanded to 4 channels, Supports more memory space, 4 bit DPCM, Built in volume and stereo panning support, and Dual chip configurations.

There's 2 stereo inputs and single stereo output, Both format is YM3012 compatible.

## Register layout

```
Address Bits      R/W Description
        7654 3210

00...03 Communication Registers

00      xxxx xxxx R   Answer from host CPU LSB
01      xxxx xxxx R   Answer from host CPU MSB

02      xxxx xxxx   W Reply to host CPU LSB
03      xxxx xxxx   W Reply to host CPU MSB

08...0f Voice 0 Register

08      xxxx xxxx   W Pitch bit 0-7
09      ---- xxxx   W Pitch bit 8-11

0a      xxxx xxxx   W Source length bit 0-7 (byte wide)
0b      xxxx xxxx   W Source length bit 8-15 (byte wide)

0c      xxxx xxxx   W Start address/ROM readback base bit 0-7
0d      xxxx xxxx   W Start address/ROM readback base bit 8-15
0e      ---x xxxx   W Start address/ROM readback base bit 16-20

0f      -xxx xxxx   W Volume

10...17 Voice 1 Register
18...1f Voice 2 Register
20...27 Voice 3 Register

28      ---- x---   W Voice 3 Key on/off trigger
        ---- -x--   W Voice 2 Key on/off trigger
        ---- --x-   W Voice 1 Key on/off trigger
        ---- ---x   W Voice 0 Key on/off trigger

29      ---- x--- R   Voice 3 busy
        ---- -x-- R   Voice 2 busy
        ---- --x- R   Voice 1 busy
        ---- ---x R   Voice 0 busy

2a      x--- ----   W Voice 3 source format
        0--- ----     8 bit signed PCM
        1--- ----     4 bit ADPCM
        -x-- ----   W Voice 2 source format
        --x- ----   W Voice 1 source format
        ---x ----   W Voice 0 source format

        ---- x---   W Voice 3 Loop enable
        ---- -x--   W Voice 2 Loop enable
        ---- --x-   W Voice 1 Loop enable
        ---- ---x   W Voice 0 Loop enable

2c      --xx x---   W Voice 1 Pan angle in degrees*1
        --00 0---     Mute
        --00 1---     0 degrees
        --01 0---     24 degrees
        --01 1---     35 degrees
        --10 0---     45 degrees
        --10 1---     55 degrees
        --11 0---     66 degrees
        --11 1---     90 degrees
        ---- -xxx   W Voice 0 Pan angle in degrees*1

2d      --xx x---   W Voice 3 Pan angle in degrees*1
        ---- -xxx   W Voice 2 Pan angle in degrees*1

2e      xxxx xxxx R   ROM readback (use Voice 0 register)

2f      ---- x---   W AUX2 input enable
        ---- -x--   W AUX1 input enable
        ---- --x-   W Sound enable
        ---- ---x   W ROM readbank enable

*1 Actually fomula unknown, Use floating point type until explained that.
```

## Frequency calculation

```
 Frequency: Input clock / (4096 - Pitch)
```
