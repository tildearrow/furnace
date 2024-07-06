# TI SN76489 (e.g. Sega Master System)

a relatively simple sound chip made by Texas Instruments. a derivative of it is used in Sega's Master System, the predecessor to Genesis. it has three square wave channels and one noise channel... not really.

nominal mode of SN76489 has 3 square wave channels, with noise channel having only 3 preset frequencies to use (absurdly low, very low, low). to use more pitches, one can enable a mode which "steals" the frequency from square wave channel 3. by doing that, SN76489 becomes effectively a 3 channel sound chip. in addition, periodic noise mode can be enabled, with same caveats.

the original iteration of the SN76489 used in the TI-99/4A computer, the SN94624, could only produce tones as low as 100Hz, and was clocked at 447 KHz. all later versions (such as the one in the Master System and Genesis) had a clock divider but ran on a faster clock... except for the SN76494, which can play notes as low as 13670 Hz (A -1). as a result, its pitch accuracy for higher notes is compromised.

## SN7 versions

SN7 was extremely popular due to low cost. therefore, it was cloned and copied to no end, often with minor differences between each other. Furnace supports several of these:
- SN94624, can only produce tones as low as 100Hz, and is clocked at 447 KHz.
- SN76494, which can play notes as low as 13.670 Hz (A -1). it has a different noise feedback and invert masks.
- SN76489, identical to SN94624, just without a clock divider.
- SN76489A, identical to 76494, just with a /8 clock divider.
- SN76496, literally identical to former. why is it even here?
- SN76496 with a Atari-like short noise. the chip of many legend and rumours which may be a result of inaccurate emulation.
- Sega Master System VDP version has a different, characteristic noise LFSR.
- Game Gear SN7, identical to the above, but with stereo.
- NCR8496, different noise invert mask.
- PSSJ3, literally identical to the former. it just swaps "high" and "low" signals in the output, which results in no audible difference.

## effects

- `20xy`: **set noise mode.**
  - `x` controls whether to inherit frequency from channel 3.
    - `0`: use one of 3 preset frequencies (C: A-2; C#: A-3; D: A-4).
    - `1`: use frequency of channel 3.
  - `y` controls whether to select noise or thin pulse.
    - `0`: thin pulse.
    - `1`: noise.


## info

this chip uses the [SN76489/Sega PSG](../4-instrument/psg.md) instrument editor.

## chip config

the following options are available in the Chip Manager window:

- **Clock rate**: sets the rate at which the chip will run.
- **Chip type**: changes the chip type. see above for more details.
- **Disable noise period change phase reset**: when enabled, the noise channel won't be reset every time its frequency changes. very useful.
- **Disable easy period to note mapping on upper octaves**: Furnace maps the notes in the upper octaves to periods, for easier noise tuning. this option allows you to disable this feature.
