
# Seta/Allumer X1-010

## Summary

- 16 voice wavetable or PCM
  - 8 bit signed for both wavetable and PCM
  - 128 width long waveform
  - wavetable playback must be paired with envelope
  - envelope shape is 4 bit stereo, 128 width long waveform
  - waveform and envelope shape stored at each half area on RAM space
  - total accessible memory for PCM: 1 MByte

## Source code

- x1_010.hpp: Base header
  - x1_010.cpp: Source emulation core

## Description

the chip has 16 voices, all voices can be switchable to Wavetable or PCM sample playback mode. It has also 2 output channels, but no known hardware using this feature for stereo sound.

Wavetable needs to paired with envelope, it's always enabled and similar as AY PSG's one but its shape is stored at RAM.

PCM volume is stored by each register.

Both volume is 4bit per output.

Everything except PCM sample is stored at paired 8 bit RAM.

## RAM layout

common case: Address bit 12 is swapped when RAM is shared with CPU

### Voice registers (0000...007f)

0000...0007 Voice #0 Register

```
Address Bits      Description
        7654 3210
0       x--- ---- Frequency divider*
        ---- -x-- Envelope one-shot mode
        ---- --x- Sound format
        ---- --0- PCM
        ---- --1- Wavetable
        ---- ---x Keyon/off
PCM case:
1       xxxx xxxx Volume (Each nibble is for each output)

2       xxxx xxxx Frequency

4       xxxx xxxx Start address / 4096

5       xxxx xxxx 0x100 - (End address / 4096)
Wavetable case:
1       ---x xxxx Wavetable data select

2       xxxx xxxx Frequency LSB
3       xxxx xxxx "" MSB

4       xxxx xxxx Envelope period

5       ---x xxxx Envelope shape select (!= 0 : Reserved for Voice registers)
```

0008...000f Voice #1 Register
...
0078...007f Voice #15 Register

### Envelope shape data (0080...0fff)

Same format as volume; Each nibble is for each output

0080...00ff Envelope shape #1 data
0100...017f Envelope shape #2 data
...
0f80...0fff Envelope shape #31 data

### Waveform data (1000...1fff)

1000...107f Waveform #0 data
1080...10ff Waveform #1 data
...
1f80...1fff Waveform #31 data

## Frequency calculation

```
Wavetable, Divider Clear: Frequency value * (Input clock / 524288)
Wavetable, Divider Set:   Frequency value * (Input clock / 1048576)
PCM, Divider Clear:       Frequency value * (Input clock / 8192)
PCM, Divider Set:         Frequency value * (Input clock / 16384)
Envelope:                 Envelope period * (Input clock / 524288) - Frequency divider not affected?
```

Frequency divider is higher precision or just right shift? needs verification.
