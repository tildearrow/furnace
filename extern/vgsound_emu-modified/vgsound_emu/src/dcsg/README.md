
# Texas instruments digital complex sound generators and variants

## Summary

- 3 voice square wave
- 1 voice noise generators

## Source code

- dcsg.hpp: Base header
  - dcsg.cpp: Source emulation core

## TODO

- Verify volume and register behaviors

## Description

This hardware has simple tone generators: 3 square wave and 1 noise generator.

Some variations have additional features: internal clock divider, external audio input, or stereo output.

Beginning to TI-99/4, This hardware family was used in a lot of machines, and Some variants were manufactured by some chipmakers: Texas Instruments, Yamaha (for Sega ASIC), Toshiba (for Neo Geo Pocket ASIC), NCR (used in Tandy 1000 series), etc...

The chip has decoding command method instead of register accessing, Its format is shares on most variants.

### Comparision Table

| Chip | Internal clock divider | Negative output | Audio input | Stereo | Individual noise frequency |
|:-:|:-:|:-:|:-:|:-:|:-:|
| SN94624 | No | Yes | No | No | No |
| SN76489 | Yes | Yes | No | No | No |
| SN76489A | Yes | No | No | No | No |
| SN76494/A | No | No | Yes | No | No |
| SN76496/A | Yes | No | Yes | No | No |
| NCR 8496 | Yes | Yes | Yes | No | No |
| Tandy PSSJ-3 | Yes | No | No | No | No |
| Sega VDP PSG | Yes | Yes | No | Game Gear only | No |
| Neo Geo Pocket PSG | Yes | Unknown | No | Yes (additional volume register) | Yes |

## Command format

These commands are shares on most of the variants, excepting Neo Geo Pocket PSG.

 Frequency command (2 byte):

```
Byte Bit      Description
     76543210
0    1xx0---- Voice select
     -00----- Square 0
     -01----- Square 1
     -10----- Square 2
     ----xxxx Frequency bit 0 to 3
1    0-xxxxxx Frequency bit 4 to 9
```

 Volume command (1 byte):

```
Byte Bit      Description
     76543210
0    1xx1---- Voice select
     -00----- Square 0
     -01----- Square 1
     -10----- Square 2
     -11----- Noise
     ----xxxx Volume
     ----0000 0.0dB
     -------1 -2.0dB step
     ------1- -4.0dB step
     -----1-- -8.0dB step
     ----1--- -16.0dB step
     ----1111 Off
```

 Noise control command (1 byte):

```
Byte Bit      Description
     76543210
0    1110---- Noise control
     -----x-- Noise mode
     -----0-- Periodic noise
     -----1-- White noise
     ------xx Frequency
     ------00 /512
     ------01 /1024
     ------10 /2048
     ------11 Use square 2 frequency
```

### Command format (Neo Geo Pocket PSG)

Neo Geo Pocket PSG has additional register for control noise frequency and stereo volume, so command format is slightly different compares to above.

 Frequency command (2 byte):

```
Byte Bit       Description
     A76543210
0    x1xx0---- Voice select
     1-00----- Square 0
     1-01----- Square 1
     1-10----- Square 2
     0-10----- Noise
     -----xxxx Frequency bit 0 to 3
1    -0-xxxxxx Frequency bit 4 to 9
```

 Volume command (1 byte):

```
Byte Bit       Description
     A76543210
0    x1--1---- Output select
     0-------- Right output
     1-------- Left output
     --xx----- Voice select
     --00----- Square 0
     --01----- Square 1
     --10----- Square 2
     --11----- Noise
     ----xxxx Volume
     ----0000 0.0dB
     -------1 -2.0dB step
     ------1- -4.0dB step
     -----1-- -8.0dB step
     ----1--- -16.0dB step
     ----1111 Off
```

 Noise control command (1 byte):

```
Byte Bit       Description
     A76543210
0    0110---- Noise control
     -----x-- Noise mode
     -----0-- Periodic noise
     -----1-- White noise
     ------xx Frequency
     ------00 /512
     ------01 /1024
     ------10 /2048
     ------11 Determined on Frequency command
```

### Stereo control register (Game Gear PSG)

Sega game gear has additional register for stereo output.

```
Byte Bit      Description
     76543210
0    xxxx---- Left output enable
     ---1---- Square 0
     --1----- Square 1
     -1------ Square 2
     1------- Noise
     ----xxxx Right output enable
     -------1 Square 0
     ------1- Square 1
     -----1-- Square 2
     ----1--- Noise
```

## Frequency calculation

```
  Square frequency = Input clock / 4 (or 32 dependent on hardware) / (Pitch input)
  Noise frequency = Input clock / 4 (or 32 dependent on hardware) / 512 or 1024 or 2048 or Square 2 frequency (or own frequency on Neo Geo Pocket PSG)
```
