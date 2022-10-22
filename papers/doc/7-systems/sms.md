# TI SN76489 (e.g. Sega Master System)

a relatively simple sound chip made by Texas Instruments. a derivative of it is used in Sega's Master System, the predecessor to Genesis.

the original iteration of the SN76489 used in the TI-99/4A computers was clocked at 447 KHz, being able to play as low as 13.670 Hz (A -1). consequentially, pitch accuracy for higher notes is compromised.

on the other hand, the chip was clocked at a much higher speed on Master System and Genesis, which makes it rather poor in the bass range.

# effects

- `20xy`: set noise mode.
  - `x` controls whether to inherit frequency from channel 3.
    - 0: use one of 3 preset frequencies (C: A-2; C#: A-3; D: A-4).
    - 1: use frequency of channel 3.
  - `y` controls whether to select noise or thin pulse.
    - 0: thin pulse.
    - 1: noise.
