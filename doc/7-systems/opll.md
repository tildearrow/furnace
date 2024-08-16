# Yamaha YM2413/OPLL

the YM2413, otherwise known as OPLL, is a cost-reduced FM synthesis sound chip, based on the Yamaha YM3812 (OPL2).

OPLL also spawned a few derivative chips, the best known of these is:
- the famous Konami VRC7. used in the Japan-only video game Lagrange Point, it was **another** cost reduction on top of the OPLL! this time just 6 channels...
- Yamaha YM2423, same chip as YM2413, just a different patch set...
- Yamaha YMF281, ditto.....

## technical specifications

the YM2413 is equipped with the following features:

- 9 channels of 2 operator FM synthesis
- a drum/percussion mode, replacing the last 3 voices with 5 rhythm channels, with drum mode tones hard-defined in the chip itself, like FM instruments. only pitch might be altered.
  - drum mode works like following: FM channel 7 is for Kick Drum, which is a normal FM channel but routed through mixer twice for 2× volume, like all drum sounds. FM channel 8 splits to Snare, Drum, and Hi-Hat. Snare Drum is the carrier and it works with a special 1 bit noise generator combined with a square wave, all possible by overriding phase-generator with some different synthesis method. Hi-Hat is the modulator and it works with the noise generator and also the special synthesis. channel 9 splits to Top-Cymbal and Tom-Tom, Top-Cymbal is the carrier and only has the special synthesis, while Tom-Tom is basically a 1op wave. 
  - special synthesis mentioned already is: 5 square waves are gathered from 4×, 64× and 128× the pitch of channel 8 and 16× and 64× the pitch of channel 9 and they go through a process where 2 HH bits OR'd together, then 1 HH and 1 TC bit OR'd, then the two TC bits OR'd together, and those 3 results get XOR'd.
- **1 user-definable patch (this patch can be changed throughout the course of the song)**
- **15 pre-defined patches which can all be used at the same time**
- support for ADSR on both the modulator and the carrier
- sine and half-sine based FM synthesis
- 9 octave note control
- 4096 different frequencies for channels
- 16 unique volume levels (NOTE: volume 0 is NOT silent.)
- modulator and carrier key scaling
- built-in hardware vibrato support

## effects

- `10xx`: **change patch.**
- `11xx`: **set feedback of channel.**
- `12xx`: **set operator 1 level.**
- `13xx`: **set operator 2 level.**
- `16xy`: **set multiplier of operator.**
  - `x` is the operator, either 1 or 2.
  - `y` is the new MULT value..
- `18xx`: **toggle drums mode.**
  - `0` disables it and `1` enables it.
  - only in drums mode.
- `19xx`: **set attack of all operators.**
- `1Axx`: **set attack of operator 1.**
- `1Bxx`: **set attack of operator 2.**
- `50xy`: **set AM of operator.**
  - `x` is the operator, either 1 or 2. a value of `0` means "all operators".
  - `y` determines whether AM is on.
- `51xy`: **set SL of operator.**
  - `x` is the operator, either 1 or 2. a value of `0` means "all operators".
  - `y` is the value.
- `52xy`: **set RR of operator.**
  - `x` is the operator, either 1 or 2. a value of `0` means "all operators".
  - `y` is the value.
- `53xy`: **set VIB of operator.**
  - `x` is the operator, either 1 or 2. a value of `0` means "all operators".
  - `y` determines whether VIB is on.
- `54xy`: **set KSL of operator.**
  - `x` is the operator, either 1 or 2. a value of `0` means "all operators".
  - `y` is the value.
- `55xy`: **set EGT of operator.**
  - `x` is the operator, either 1 or 2. a value of `0` means "all operators".
  - `y` determines whether EGT is on.
- `56xx`: **set DR of all operators.**
- `57xx`: **set DR of operator 1.**
- `58xx`: **set DR of operator 2.**
- `5Bxy`: **set KSR of operator.**
  - `x` is the operator, either 1 or 2. a value of `0` means "all operators".
  - `y` determines whether KSR is on.

## info

this chip uses the [FM (OPLL)](../4-instrument/fm-opll.md) instrument editor.

## chip config

the following options are available in the Chip Manager window:

- **Clock rate**: sets the rate at which the chip will run.
- **Patch set**: changes the chip model, providing different built-in sounds.

the following options are visible in drums mode:

- **Ignore top/hi-hat frequency changes**: in drums mode, makes the top/hi-hat channels not write frequency since they share it with snare and tom.
- **Apply fixed frequency to all drums at once**: sets the frequency of all drums to that of a fixed frequency OPLL drums instrument when one note with it is reached.
