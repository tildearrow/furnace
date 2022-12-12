
# Sharp SM8521

## Summary

- 2 voice wavetable
  - 4 bit signed, 32 nibbles long per each voice
- 1 voice noise generator
- D/A direct output

## Source code

- sm8521.hpp: Base header
  - sm8521.cpp: Source emulation core

## TODO

- Verify noise algorithm

## Description

Sharp SM8521 is SoC, with SM85CPU, 4 port 8 bit GPIO, greyscale LCD controller and DMA, Sound core.

It's sound core features 2 voice wavetable and noise generator, or D/A direct output.

It was used in Tiger Game.com.

## Register layout

- Sound only

```
Address Bits      Description
        7654 3210
40      xxxx xxxx Global control
        x--- ---- Sound output enable
        ---- x--- DAC enable
        ---- -x-- SG2 enable
        ---- --x- SG1 enable
        ---- ---x SG0 enable

42      ---x xxxx SG0 Output level
44      ---x xxxx SG1 Output level

46      ---- xxxx SG0 Pitch MSB
47      xxxx xxxx SG0 Pitch LSB

48      ---- xxxx SG1 Pitch MSB
49      xxxx xxxx SG1 Pitch LSB

4A      ---x xxxx SG2 Output level

4C      ---- xxxx SG2 Pitch MSB
4D      xxxx xxxx SG2 Pitch LSB

4E      xxxx xxxx DAC output

60-6F SG0 waveform
        xxxx ---- High nibble
        ---- xxxx Low nibble

70-7F SG1 waveform

```

## Frequency calculation

```
    Frequency: Input clock / (Pitch - 1)
```
