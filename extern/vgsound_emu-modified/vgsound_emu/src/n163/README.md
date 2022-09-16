# Namco 163

## Summary

- 1 to 8 voice wavetable
  - 4 bit unsigned
  - each waveforms are can be placed to anywhere in internal RAM, and its size is can be variable.
- activated voice count can be changed any time, multiplexed output

## Source code

- n163.hpp: Base header
  - n163.cpp: Source emulation core

## Description

This chip is one of NES mapper with sound expansion, This one is by Namco.

It has 1 to 8 wavetable channels, All channel registers and waveforms are stored to internal RAM. 4 bit Waveforms are freely allocatable, and its length is variables; its can be stores many short waveforms or few long waveforms in RAM.

But waveforms are needs to squash, reallocate to avoid conflict with channel register area, each channel register size is 8 bytes per channels.

Sound output is time division multiplexed, it's can be captured only single channels output at once. in reason, More activated channels are less sound quality.

## Sound register layout

```
Address Bit      Description
        7654 3210

78-7f Channel 0
78      xxxx xxxx Channel 0 Pitch input bit 0-7
79      xxxx xxxx Channel 0 Accumulator bit 0-7
7a      xxxx xxxx Channel 0 Pitch input bit 8-15
7b      xxxx xxxx Channel 0 Accumulator bit 8-15
7c      xxxx xx-- Channel 0 Waveform length, 256 - (x * 4)
        ---- --xx Channel 0 Pitch input bit 16-17
7d      xxxx xxxx Channel 0 Accumulator bit 16-23
7e      xxxx xxxx Channel 0 Waveform base offset
        xxxx xxx- RAM byte (0 to 127)
        ---- ---x RAM nibble
        ---- ---0 Low nibble
        ---- ---1 High nibble
7f      ---- xxxx Channel 0 Volume

7f Number of active channels
7f      -xxx ---- Number of active channels
        -000 ---- Channel 0 activated
        -001 ---- Channel 1 activated
        -010 ---- Channel 2 activated
        ...
        -110 ---- Channel 6 activated
        -111 ---- Channel 7 activated

70-77 Channel 1 (Optional if activated)
68-6f Channel 2 (Optional if activated)
...
48-4f Channel 6 (Optional if activated)
40-47 Channel 7 (Optional if activated)
```

Rest of RAM area are for 4 bit Waveform and/or scratchpad.

## Waveform format

Each waveform byte has 2 nibbles packed, fetches LSB first, MSB next.

```
   ---- xxxx 4 bit waveform, LSB
   xxxx ---- Same as above, MSB
```

Waveform address: Waveform base offset + Bit 16 to 23 of Accumulator, 1 LSB of result is nibble select, 7 MSB of result is Byte address in RAM.

## Frequency calculation

```
 Frequency: Pitch input * ((Input clock * 15 * Number of activated voices) / 65536)
```

## Technical notice

This core only outputs raw output from chip (or accumulated output, see below); any kind of off-chip stuff needs to implemented outside core.

There's to way for reduce N163 noises: reduce channel limit and demultiplex:

- Channel limit is runtime changeable and it makes some usable effects.
- Demultiplex is used for "non-ear destroyable" emulators, but less hardware accurate. (when LPF and RF filter is not considered) This core is support both, You can choose output behavior
