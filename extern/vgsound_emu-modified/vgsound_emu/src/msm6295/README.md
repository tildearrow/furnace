# OKI MSM6295

## Summary

- 4 voice Dialogic ADPCM
  - total accessible memory: 256 KByte
- Clock divider via SS pin

## Source code

- msm6295.hpp: Base header
  - msm6295.cpp: Source emulation core

### Dependencies

- vox.hpp: Dialogic ADPCM decoder header
  - vox.cpp: Dialogic ADPCM decoder source

## Description

It is 4 channel ADPCM playback chip from OKI semiconductor. It was becomes de facto standard for ADPCM playback in arcade machine, due to cost performance.

The chip itself is pretty barebone: there is no "register" in chip. It can't control volume and pitch in currently playing channels, only stopable them. And volume is must be determined at playback start command.

## Command format

 Playback command (2 byte):

```
Byte Bit      Description
     76543210
0    1xxxxxxx Phrase select (Header stored in ROM)
1    x000---- Play channel 4
     0x00---- Play channel 3
     00x0---- Play channel 2
     000x---- Play channel 1
     ----xxxx Volume
     ----0000 0.0dB
     ----0001 -3.2dB
     ----0010 -6.0dB
     ----0011 -9.2dB
     ----0100 -12.0dB
     ----0101 -14.5dB
     ----0110 -18.0dB
     ----0111 -20.5dB
     ----1000 -24.0dB
```

 Suspend command (1 byte):

```
Byte Bit      Description
     76543210
0    0x------ Suspend channel 4
     0-x----- Suspend channel 3
     0--x---- Suspend channel 2
     0---x--- Suspend channel 1
```

## Frequency calculation

```
 if (SS) then
  Frequency = Input clock / 165
 else then
  Frequency = Input clock / 132
```
