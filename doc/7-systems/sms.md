# TI SN76489 (e.g. sega Master System)

a relatively simple sound chip made by Texas Instruments. a derivative of it is used in Sega's Master System, the predecessor to Genesis.

the original iteration of the SN76489 used in the TI-99/4A computer, the SN94624, could only produce tones as low as 100Hz, and was clocked at 447 KHz. all later versions (such as the one in the Master System and Genesis) had a clock divider but ran on a faster clock... except for the SN76494, which can play notes as low as 13.670 Hz (A -1). consequently, its pitch accuracy for higher notes is compromised.

# effects

- `20xy`: **set noise mode.**
  - `x` controls whether to inherit frequency from channel 3.
    - `0`: use one of 3 preset frequencies (C: A-2; C#: A-3; D: A-4).
    - `1`: use frequency of channel 3.
  - `y` controls whether to select noise or thin pulse.
    - `0`: thin pulse.
    - `1`: noise.

# chip config
## SN7 versions
SN7 was extremely popular due to low cost. Therefore, it was cloned and copied to no end, often with minor differences between each other. Furnace supports several of these:
- SN94624, can only produce tones as low as 100Hz, and is clocked at 447 KHz.
- SN76494, which can play notes as low as 13.670 Hz (A -1). It has a different noise feedback and invert masks.
- SN76489, identical to SN94624, just without a clock divider
- SN76489A, identical to 76494, just with a /8 clock divider
- SN76496, literally identical to former. Why is it even here?
- SN76496 with a Atari-like short noise. The chip of many legend and rumours, might be a result of inaccurate emulation.
- Sega Master System VDP version has a different, characteristic noise LFSR.
- Game Gear SN7, identical to the above, but with stereo
- NCR8496, different noise invert masks
- PSSJ3, literally identical to the former, it just swaps "high" and "low" signals in the output, which results in no audible difference

TODO: all these checkboxes
