# TI SN76489 (e.g. Sega Master System)

the SN76489 is a relatively simple sound chip made by Texas Instruments. a derivative of it is used in the Sega Master System and Sega Genesis. it has three square wave channels and one noise channel.

the original iteration of the SN76489 used in the TI-99/4A computer, the SN94624, runs at a clock speed of 447 kHz and can only produce square waves as low as approximately 110 Hz, or note A2. later versions of the chip (such as the one in the Master System and Genesis) add an internal divide-by-8 stage and generally run at higher system clock rates to achieve a matching or similar frequency range.

## SN7 versions

SN7 was extremely popular due to low cost. therefore, it was cloned and copied to no end, often with minor differences between each version. Furnace supports several of these:
- **SN94624**. as described above. noise presets are tuned to A#. each volume step attenuates the volume by 2dB.
- **SN76494**. uses different noise feedback and invert masks. noise presets are tuned to approximately A. each volume step attenuates the volume by 2.25dB.
- **SN76489**. identical to SN94624, with a /8 clock divider.
- **SN76489 with Atari-like short noise**. the chip of many legends and rumours which may be a result of inaccurate emulation. only works with the MAME emulation core.
- **SN76489A**. identical to SN76494, with a /8 clock divider.
- **SN76496**. identical to SN76489A.
- **Sega VDP/Master System**. SN76489A with a different, characteristic noise LFSR.
- **Game Gear**. identical to the above but with hard-panned stereo.
- **NCR8496**. as above with a different noise invert mask.
- **Tandy PSSJ 3-voice sound**. identical to NCR8496 but swaps "high" and "low" signals in the output, which results in no audible difference.

## effects

- `20xy`: **set noise mode.**
  - `x` controls whether to inherit frequency from channel 3.
    - `0`: use one of 3 preset frequencies (see info below).
    - `1`: use frequency of channel 3.
  - `y` controls whether to select noise or thin pulse.
    - `0`: thin pulse.
    - `1`: noise.

## info

this chip uses the [SN76489/Sega PSG](../4-instrument/psg.md) instrument editor.

the noise channel's default mode uses only 3 preset frequencies. to use the full range of pitches, one can enable a mode in which the noise channel matches the frequency of square wave channel 3. in addition, periodic noise mode can be enabled to create a "thin pulse" tone that sounds four octaves lower than the square wave channels. for convenience in entering notes, a tracked note of C corresponds to a played note of A or A# in octave 2, C# is octave 3, and D is octave 4.

noise mode macro values:
- **0**: same as effect `2000`. thin pulse, 3 preset frequencies.
- **1**: same as effect `2001`. noise, 3 preset frequencies.
- **2**: same as effect `2010`. thin pulse, frequency shared with channel 3.
- **3**: same as effect `2011`. noise, frequency shared with channel 3.

## chip config

the following options are available in the Chip Manager window:

- **Clock rate**: sets the rate at which the chip will run.
- **Chip type**: changes the chip type. see "SN7 versions" above for more details.
- **Disable noise period change phase reset**: when checked, the noise channel won't be reset every time its frequency changes. very useful.
- **Disable easy period to note mapping on upper octaves**: Furnace maps notes in octaves 7 and up directly to periods to make it easier to reach the highest pitches with normal note entry. this can be especially useful for noise. check this option to disable this feature.
