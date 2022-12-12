
# General instruments PSG and variants

## Summary

- 3 voice
  - square wave AND/OR noise AND/OR envelope output
  - shared noise generator
  - envelope generator (shared or independent between voice)
- up to 2 8 bit IO port

## Source code

- aypsg.hpp: Base header
  - aypsg.cpp: Source emulation core

## TODO

- Verify volume, AY8930 expanded mode register behaviors

## Description

Original AY-3-8910 was released from General instruments.

It's start from AY-3-8910, It was widespreaded at most of machines and variated at GI itself or Microchip, Yamaha, etc...

Shared feature of these hardwares is 3 voices of Square/Envelope/Noise mixer; Envelope and White noise generator is shared on all voices.

Some cut-down variation has single or no IO port compared to Original's 2 IO port.

Yamaha variations (YM2149, etc) are finer envelope resolution and different volume method.

It was enhanced at AY8930 EPSG, features independent envelope generator per voices, Extended square/noise frequency value, Enhanced noise generator with AND/OR mask.

YM2149 and AY8930 has clock divider, for allowing high clock rate.

### Comparision Table

## Register format

```
Register Bit      Description
         76543210
0        xxxxxxxx Square 0 Fine tune
1        ----xxxx Square 0 Coarse tune

2        xxxxxxxx Square 1 Fine tune
3        ----xxxx Square 1 Coarse tune

4        xxxxxxxx Square 2 Fine tune
5        ----xxxx Square 2 Coarse tune

6        ---xxxxx Noise period

7        x------- IO port B Input/output select
         -x------ IO port A Input/output select
         --210--- Noise enable/disable at voice
         -----210 Square enable/disable at voice

8        ---x---- Square 0 volume mode
         ----xxxx Square 0 direct volume

9        ---x---- Square 1 volume mode
         ----xxxx Square 1 direct volume

10       ---x---- Square 2 volume mode
         ----xxxx Square 2 direct volume

11       xxxxxxxx Envelope Fine tune
12       xxxxxxxx Envelope Coarse tune

13       ----xxxx Envelope Shape
         ----x--- Envelope Continue
         -----x-- Envelope Attack
         ------x- Envelope Alternate
         -------x Envelope Hold

14       xxxxxxxx IO port A
15       xxxxxxxx IO port B
```

### AY-3-8914 Register format

Extended envelope volume range, Different mapping

```
Register Bit      Description
         76543210
0        xxxxxxxx Square 0 Fine tune
1        xxxxxxxx Square 1 Fine tune
2        xxxxxxxx Square 2 Fine tune
3        xxxxxxxx Envelope Fine tune

4        ----xxxx Square 0 Coarse tune
5        ----xxxx Square 1 Coarse tune
6        ----xxxx Square 2 Coarse tune
7        xxxxxxxx Envelope Coarse tune

8        x------- IO port B Input/output select
         -x------ IO port A Input/output select
         --210--- Noise enable/disable at voice
         -----210 Square enable/disable at voice

9        ---xxxxx Noise period

10       ----xxxx Envelope Shape
         ----x--- Envelope Continue
         -----x-- Envelope Attack
         ------x- Envelope Alternate
         -------x Envelope Hold

11       --xx---- Square 0 volume mode/Envelope volume
         ----xxxx Square 0 direct volume

12       --xx---- Square 1 volume mode/Envelope volume
         ----xxxx Square 1 direct volume

13       --xx---- Square 2 volume mode/Envelope volume
         ----xxxx Square 2 direct volume

14       xxxxxxxx IO port A
15       xxxxxxxx IO port B
```

### AY8930 Expanded mode Register format

Expanded mode has 2 bank of register, and used for its exclusive features.
also Coarse tune and noise frequency range is extended, Envelope is independent per each voices.

```
Register Bank Bit      Description
              76543210
0        0    xxxxxxxx Square 0 Fine tune
1        0    xxxxxxxx Square 0 Coarse tune

2        0    xxxxxxxx Square 1 Fine tune
3        0    xxxxxxxx Square 1 Coarse tune

4        0    xxxxxxxx Square 2 Fine tune
5        0    xxxxxxxx Square 2 Coarse tune

6        0    xxxxxxxx Noise period

7        0    x------- IO port B Input/output select
              -x------ IO port A Input/output select
              --210--- Noise enable/disable at voice
              -----210 Square enable/disable at voice

8        0    ---x---- Square 0 volume mode
              ----xxxx Square 0 direct volume

9        0    ---x---- Square 1 volume mode
              ----xxxx Square 1 direct volume

10       0    ---x---- Square 2 volume mode
              ----xxxx Square 2 direct volume

11       0    xxxxxxxx Square 0 Envelope Fine tune
12       0    xxxxxxxx Square 0 Envelope Coarse tune

13       -    101----- Expanded mode
              ---x---- Register bank
              ----xxxx Square 0 Envelope Shape
              ----x--- Envelope Continue
              -----x-- Envelope Attack
              ------x- Envelope Alternate
              -------x Envelope Hold

14       0    xxxxxxxx IO port A
15       0    xxxxxxxx IO port B

0        1    xxxxxxxx Square 1 Envelope Fine tune
1        1    xxxxxxxx Square 1 Envelope Coarse tune

2        1    xxxxxxxx Square 2 Envelope Fine tune
3        1    xxxxxxxx Square 2 Envelope Coarse tune

4        1    ----xxxx Square 1 Envelope Shape
5        1    ----xxxx Square 2 Envelope Shape

6        1    ----xxxx Square 0 Duty cycle
              ----0000 3.125 %
              ----0001 6.25 %
              ----0010 12.5 %
              ----0011 25 %
              ----0100 50 %
              ----0101 75 %
              ----0110 87.5 %
              ----0111 93.75 %
              ----1--- 96.875 %
7        1    ----xxxx Square 1 Duty cycle
8        1    ----xxxx Square 2 Duty cycle

9        1    xxxxxxxx Noise AND mask
10       1    xxxxxxxx Noise OR mask
15       1    xxxxxxxx Test
```

## Frequency calculation

```
  Square frequency = Input clock / (16 or 32 if divided) / (Pitch input)
  Noise frequency = Input clock / (32 or 64 if divided) / (Pitch input)
  Envelope frequency = Input clock / (16 or 32 if divided) / (Pitch input)
```
