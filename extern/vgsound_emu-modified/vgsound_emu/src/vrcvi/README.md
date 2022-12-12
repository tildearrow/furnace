
# Konami VRC VI

## Summary

- 2 voice pulse wave
  - 8 level duty or volume only mode
- 1 voice sawtooth wave
- Internal mapper and timer

## Source code

- vrcvi.hpp: Base header
  - vrcvi.cpp: Source emulation core

## Description

It's one of NES mapper with built-in sound chip, and also one of 2 Konami VRCs with this feature. (rest one has OPLL derivatives.)

It's also DACless like other sound chip and mapper-with-sound manufactured by konami, the Chips 6 bit digital sound output is needs converted to analog sound output when you it want to make some sounds, or send to sound mixer.

Its are used for Akumajou Densetsu (Japan release of Castlevania III), Madara, Esper Dream 2.

The chip is installed in 351951 PCB and 351949A PCB.

351951 PCB is used exclusivly for Akumajou Densetsu, Small board has VRC VI, PRG and CHR ROM.

- It's configuration also calls VRC6a, iNES mapper 024.

351949A PCB is for Last 2 titles with VRC VI, Bigger board has VRC VI, PRG and CHR ROM, and Battery Backed 8K x 8 bit SRAM.

- Additionally, It's PRG A0 and A1 bit to VRC VI input is swapped, compare to above.
- It's configuration also calls VRC6b, iNES mapper 026.

The chip itself has 053328, 053329, 053330 Revision, but Its difference between revision is unknown.

Like other mappers for NES, It has internal timer - Its timer can be sync with scanline like other Konami mapper in this era.

## Register layout

- Sound and Timer only; 351951 PCB case, 351949A swaps xxx1 and xxx2

```
Address Bits      Description
        7654 3210

9000-9002 Pulse 1

9000    x--- ---- Pulse 1 Duty ignore
        -xxx ---- Pulse 1 Duty cycle
        ---- xxxx Pulse 1 Volume
9001    xxxx xxxx Pulse 1 Pitch bit 0-7
9002    x--- ---- Pulse 1 Enable
        ---- xxxx Pulse 1 Pitch bit 8-11

9003 Sound control

9003    ---- -x-- 4 bit Frequency mode
        ---- -0x- 8 bit Frequency mode
        ---- ---x Halt

a000-a002 Pulse 2

a000    x--- ---- Pulse 2 Duty ignore
        -xxx ---- Pulse 2 Duty cycle
        ---- xxxx Pulse 2 Volume
a001    xxxx xxxx Pulse 2 Pitch bit 0-7
a002    x--- ---- Pulse 2 Enable
        ---- xxxx Pulse 2 Pitch bit 8-11

b000-b002 Sawtooth

b000    --xx xxxx Sawtooth Accumulate Rate
b001    xxxx xxxx Sawtooth Pitch bit 0-7
b002    x--- ---- Sawtooth Enable
        ---- xxxx Sawtooth Pitch bit 8-11

f000-f002 IRQ Timer

f000    xxxx xxxx IRQ Timer latch
f001    ---- -0-- Sync with scanline
        ---- --x- Enable timer
        ---- ---x Enable timer after IRQ Acknowledge
f002    ---- ---- IRQ Acknowledge
```

## Frequency calculation

```
 if 4 bit Frequency Mode then
  Frequency: Input clock / (bit 8 to 11 of Pitch + 1)
 end else if 8 bit Frequency Mode then
  Frequency: Input clock / (bit 4 to 11 of Pitch + 1)
 end else then
  Frequency: Input clock / (Pitch + 1)
 end
```
