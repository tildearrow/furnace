# Konami SCC

## modified

the emulation core has been modified for optimization.

## Summary

- 5 voice wavetable
  - 8 bit signed, 32 width long for each voice
  - each voice has separated waveform space
  - 4th and 5th voice shares waveform (K051649)
- MegaROM mapper (K051649) or MegaRAM mapper (K052539)

## Source code

- scc.hpp: Base header
  - scc.cpp: Source emulation core

## Description

Konami SCC means "Sound Creative Chip", it's actually MSX MegaROM/RAM Mapper with 5 channel Wavetable sound generator.

It was first appeared at 1987, F-1 Spirit and Nemesis 2/Gradius 2 for MSX. then several MSX cartridges used that until 1990, Metal Gear 2: Solid Snake. Even after MSX is discontinued, it was still used at some low-end arcade and amusement hardwares. and some Third-party MSX utilities still support this due to its market shares.

There's 2 SCC types:

- K051649 (or simply known as SCC)
  This chip is used for MSX MegaROM Mapper, some arcade machines.
  Channel 4 and 5 must be share waveform, other channels has its own waveforms.

- K052539 (also known as SCC+)
  This chip is used for MSX MegaRAM Mapper (Konami Sound Cartridges for Snatcher/SD Snatcher). All channels can be has its own waveforms, and also has backward compatibility mode with K051649.

## Register layout

### K051649

- 4000-bfff MegaROM Mapper

```
Address   Bit       R/W Description
          7654 3210

4000-5fff xxxx xxxx R   Bank page 0
c000-dfff mirror of 4000-5fff

6000-7fff xxxx xxxx R   Bank page 1
e000-ffff mirror of 6000-7fff

8000-9fff xxxx xxxx R   Bank page 2
0000-1fff mirror of 8000-9fff

a000-bfff xxxx xxxx R   Bank page 3
2000-3fff mirror of a000-bfff
```

- 5000-57ff, 7000-77ff, 9000-97ff, b000-b7ff Bank select

```
Address   Bit       R/W Description
          7654 3210

5000      --xx xxxx   W Bank select, Page 0
5001-57ff Mirror of 5000

7000      --xx xxxx   W Bank select, Page 1
7001-77ff Mirror of 7000

9000      --xx xxxx   W Bank select, Page 2
          --11 1111   W SCC Enable
9001-97ff Mirror of 9000

b000      --xx xxxx   W Bank select, Page 3
b001-b7ff Mirror of b000
```

- 9800-9fff SCC register

```
9800-987f Waveform

Address   Bit       R/W Description
          7654 3210

9800-981f xxxx xxxx R/W Channel 0 Waveform (32 byte, 8 bit signed)
9820-983f xxxx xxxx R/W Channel 1 ""
9840-985f xxxx xxxx R/W Channel 2 ""
9860-987f xxxx xxxx R/W Channel 3/4 ""

9880-9889 Pitch

9880      xxxx xxxx   W Channel 0 Pitch LSB
9881      ---- xxxx   W Channel 0 Pitch MSB
9882      xxxx xxxx   W Channel 1 Pitch LSB
9883      ---- xxxx   W Channel 1 Pitch MSB
9884      xxxx xxxx   W Channel 2 Pitch LSB
9885      ---- xxxx   W Channel 2 Pitch MSB
9886      xxxx xxxx   W Channel 3 Pitch LSB
9887      ---- xxxx   W Channel 3 Pitch MSB
9888      xxxx xxxx   W Channel 4 Pitch LSB
9889      ---- xxxx   W Channel 4 Pitch MSB

9888-988e Volume

988a      ---- xxxx   W Channel 0 Volume
988b      ---- xxxx   W Channel 1 Volume
988c      ---- xxxx   W Channel 2 Volume
988d      ---- xxxx   W Channel 3 Volume
988e      ---- xxxx   W Channel 4 Volume

988f      ---x ----   W Channel 4 Output enable/disable flag
          ---- x---   W Channel 3 Output enable/disable flag
          ---- -x--   W Channel 2 Output enable/disable flag
          ---- --x-   W Channel 1 Output enable/disable flag
          ---- ---x   W Channel 0 Output enable/disable flag

9890-989f Mirror of 9880-988f

98a0-98bf xxxx xxxx R   Channel 4 Waveform

98e0      x--- ----   W Waveform rotate flag for channel 4
          -x-- ----   W Waveform rotate flag for all channels
          --x- ----   W Reset waveform position after pitch writes
          ---- --x-   W 8 bit frequency
          ---- --0x   W 4 bit frequency

98e1-98ff Mirror of 98e0

9900-9fff Mirror of 9800-98ff
```

### K052539

- 4000-bfff MegaRAM Mapper

```
Address   Bit       R/W Description
          7654 3210

4000-5fff xxxx xxxx R/W Bank page 0
c000-dfff xxxx xxxx R/W ""

6000-7fff xxxx xxxx R/W Bank page 1
e000-ffff xxxx xxxx R/W ""

8000-9fff xxxx xxxx R/W Bank page 2
0000-1fff xxxx xxxx R/W ""

a000-bfff xxxx xxxx R/W Bank page 3
2000-3fff xxxx xxxx R/W ""
```

- 5000-57ff, 7000-77ff, 9000-97ff, b000-b7ff Bank select

```
Address   Bit       R/W Description
          7654 3210

5000      xxxx xxxx   W Bank select, Page 0
5001-57ff Mirror of 5000

7000      xxxx xxxx   W Bank select, Page 1
7001-77ff Mirror of 7000

9000      xxxx xxxx   W Bank select, Page 2
          --11 1111   W SCC Enable (SCC Compatible mode)
9001-97ff Mirror of 9000

b000      xxxx xxxx   W Bank select, Page 3
          1--- ----   W SCC+ Enable (SCC+ mode)
b001-b7ff Mirror of b000
```

- bffe-bfff Mapper configuration

```
Address   Bit       R/W Description
          7654 3210

bffe      --x- ----   W SCC operation mode
          --0- ----   W SCC Compatible mode
          --1- ----   W SCC+ mode
          ---x ----   W RAM write/Bank select toggle for all Bank pages
          ---0 ----   W Bank select enable
          ---1 ----   W RAM write enable
          ---0 -x--   W RAM write/Bank select toggle for Bank page 2
          ---0 --x-   W RAM write/Bank select toggle for Bank page 1
          ---0 ---x   W RAM write/Bank select toggle for Bank page 0

bfff Mirror of bffe
```

- 9800-9fff SCC Compatible mode register

```
9800-987f Waveform

Address   Bit       R/W Description
          7654 3210

9800-981f xxxx xxxx R/W Channel 0 Waveform (32 byte, 8 bit signed)
9820-983f xxxx xxxx R/W Channel 1 ""
9840-985f xxxx xxxx R/W Channel 2 ""
9860-987f xxxx xxxx R/W Channel 3/4 ""

9880-9889 Pitch

9880      xxxx xxxx   W Channel 0 Pitch LSB
9881      ---- xxxx   W Channel 0 Pitch MSB
9882      xxxx xxxx   W Channel 1 Pitch LSB
9883      ---- xxxx   W Channel 1 Pitch MSB
9884      xxxx xxxx   W Channel 2 Pitch LSB
9885      ---- xxxx   W Channel 2 Pitch MSB
9886      xxxx xxxx   W Channel 3 Pitch LSB
9887      ---- xxxx   W Channel 3 Pitch MSB
9888      xxxx xxxx   W Channel 4 Pitch LSB
9889      ---- xxxx   W Channel 4 Pitch MSB

9888-988e Volume

988a      ---- xxxx   W Channel 0 Volume
988b      ---- xxxx   W Channel 1 Volume
988c      ---- xxxx   W Channel 2 Volume
988d      ---- xxxx   W Channel 3 Volume
988e      ---- xxxx   W Channel 4 Volume

988f      ---x ----   W Channel 4 Output enable/disable flag
          ---- x---   W Channel 3 Output enable/disable flag
          ---- -x--   W Channel 2 Output enable/disable flag
          ---- --x-   W Channel 1 Output enable/disable flag
          ---- ---x   W Channel 0 Output enable/disable flag

9890-989f Mirror of 9880-988f

98a0-98bf xxxx xxxx R   Channel 4 Waveform

98c0      -x-- ----   W Waveform rotate flag for all channels
          --x- ----   W Reset waveform position after pitch writes
          ---- --x-   W 8 bit frequency
          ---- --0x   W 4 bit frequency

98c1-98df Mirror of 98c0

 9900-9fff Mirror of 9800-98ff
```

- b800-bfff SCC+ mode register

```
b800-b89f Waveform

Address   Bit       R/W Description
          7654 3210

b800-b81f xxxx xxxx R/W Channel 0 Waveform (32 byte, 8 bit signed)
b820-b83f xxxx xxxx R/W Channel 1 ""
b840-b85f xxxx xxxx R/W Channel 2 ""
b860-b87f xxxx xxxx R/W Channel 3 ""
b880-b89f xxxx xxxx R/W Channel 3 ""

b8a0-b8a9 Pitch

b8a0      xxxx xxxx   W Channel 0 Pitch LSB
b8a1      ---- xxxx   W Channel 0 Pitch MSB
b8a2      xxxx xxxx   W Channel 1 Pitch LSB
b8a3      ---- xxxx   W Channel 1 Pitch MSB
b8a4      xxxx xxxx   W Channel 2 Pitch LSB
b8a5      ---- xxxx   W Channel 2 Pitch MSB
b8a6      xxxx xxxx   W Channel 3 Pitch LSB
b8a7      ---- xxxx   W Channel 3 Pitch MSB
b8a8      xxxx xxxx   W Channel 4 Pitch LSB
b8a9      ---- xxxx   W Channel 4 Pitch MSB

b8a8-b8ae Volume

b8aa      ---- xxxx   W Channel 0 Volume
b8ab      ---- xxxx   W Channel 1 Volume
b8ac      ---- xxxx   W Channel 2 Volume
b8ad      ---- xxxx   W Channel 3 Volume
b8ae      ---- xxxx   W Channel 4 Volume

b8af      ---x ----   W Channel 4 Output enable/disable flag
          ---- x---   W Channel 3 Output enable/disable flag
          ---- -x--   W Channel 2 Output enable/disable flag
          ---- --x-   W Channel 1 Output enable/disable flag
          ---- ---x   W Channel 0 Output enable/disable flag

b8b0-b8bf Mirror of b8a0-b8af

b8c0      -x-- ----   W Waveform rotate flag for all channels
          --x- ----   W Reset waveform position after pitch writes
          ---- --x-   W 8 bit frequency
          ---- --0x   W 4 bit frequency

b8c1-b8df Mirror of b8c0

b900-bfff Mirror of b800-b8ff
```

## Frequency calculation

```
 if 8 bit frequency then
  Frequency = Input clock / ((bit 0 to 7 of Pitch input) + 1)
 else if 4 bit frequency then
  Frequency = Input clock / ((bit 8 to 11 of Pitch input) + 1)
 else
  Frequency = Input clock / (Pitch input + 1)
```

## Reference

<https://www.msx.org/wiki/MegaROM_Mappers>
<https://www.msx.org/wiki/Konami_051649>
<https://www.msx.org/wiki/Konami_052539>
<http://bifi.msxnet.org/msxnet/tech/scc>
<http://bifi.msxnet.org/msxnet/tech/soundcartridge>
