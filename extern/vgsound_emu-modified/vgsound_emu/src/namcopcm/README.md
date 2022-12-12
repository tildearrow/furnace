# Namco C140/C219

## Summary

- 24 (C140) / 16 (C219) voice PCM
  - 12 (C140) / 8 (C219) bit signed or 8 bit non-linear
  - total accessible memory: 64 Kbyte per bank, max 256 bank
- Internal Timer (C140)

## Source code

- namcopcm.hpp: Base header
  - namcopcm.cpp: Source emulation core

## TODO

- undocumented/unverified registers

## Description

Starting at Namco System 2, Namco starts to using PCM sound hardware; C140 was used at System 2/21 and derivatives. It has 24 voice PCM sound, a many voices at arcade hardware in 1987.

C219 was used at Namco NA1/2, it was using work RAM of Main CPU and MCU, instead dedicated PCM sample ROM. Register format is almost identical to C140, but some differences and additional feature: so it's not completely compatible.

## Register layout

8 or 16 bit accessing is available.

### C140

```
    Address Bits                 R/W Description
            fedc ba98 7654 3210
    0x0-0xf Voice 0
    0x0     xxxx xxxx ---- ----  R/W Right volume
            ---- ---- xxxx xxxx  R/W Left volume
    0x2     xxxx xxxx xxxx xxxx  R/W Frequency
    0x4     xxxx xxxx ---- ----  R/W Bank
            ---- ---- x--- ----  R/W Keyon/off
            ---- ---- -x-- ----  R   Busy?
            ---- ---- -x-- ----    W Unknown writes
            ---- ---- ---x ----  R/W Loop flags
            ---- ---- ---- x---  R/W Use compressed sample
    0x6     xxxx xxxx xxxx xxxx  R/W Start address
    0x8     xxxx xxxx xxxx xxxx  R/W Loop address
    0xA     xxxx xxxx xxxx xxxx  R/W End address
    
    0x10-0x1F Voice 1
    0x20-0x2F Voice 2
    ...
    0x170-0x17F Voice 23
    
    0x1F8   xxxx xxxx ---- ----  R/W Timer period
    0x1FA   xxxx xxxx ---- ----    W Reload timer
    0x1FE   ---- ---x ---- ----    W Timer enable
```

### C219

```
    Address Bits                 R/W Description
            fedc ba98 7654 3210
    0x0-0xf Voice 0
    0x0     xxxx xxxx ---- ----  R/W Right volume
            ---- ---- xxxx xxxx  R/W Left volume
    0x2     xxxx xxxx xxxx xxxx  R/W Frequency
    0x4     xxxx xxxx ---- ----  R/W Unknown
            ---- ---- x--- ----  R/W Keyon/off
            ---- ---- -x-- ----  R   Busy?
            ---- ---- -x-- ----    W Invert output
            ---- ---- ---x ----  R/W Loop flags
            ---- ---- ---- x---  R/W Invert left output
            ---- ---- ---- -x--  R/W Noise flag
            ---- ---- ---- ---x  R/W Use compressed sample
    0x6     xxxx xxxx xxxx xxxx  R/W Start address
    0x8     xxxx xxxx xxxx xxxx  R/W Loop address
    0xA     xxxx xxxx xxxx xxxx  R/W End address
    
    0x10-0x1F Voice 1
    0x20-0x2F Voice 2
    ...
    0xF0-0xFF Voice 15
    
    0x1F0-0x1F7 Bank register
    0x1F0   ---- ---- xxxx xxxx  R/W Bank for voice 4-7
    0x1F2   ---- ---- xxxx xxxx  R/W Bank for voice 8-11
    0x1F4   ---- ---- xxxx xxxx  R/W Bank for voice 12-15
    0x1F6   ---- ---- xxxx xxxx  R/W Bank for voice 0-3
    
    0x1F8-0x1FF Mirrored from 0x1f0-0x1f7
```

## Frequency calculation

```
    Frequency value * (Sample rate * 2)
```
