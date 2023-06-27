# Super Nintendo Entertainment System (SNES) / Super Famicom

the successor to NES to compete with Genesis, packing superior graphics and sample-based audio.

its Sony-developed audio system features a DSP chip, SPC700 CPU, and 64KB of dedicated SRAM used by both.
this whole system itself is pretty much a separate computer that the main CPU needs to upload its program and samples to.

Furnace communicates with the DSP directly and provides a full 64KB of memory. this memory might be reduced excessively on ROM export to make up for playback engine and pattern data. you can go to window > statistics to see how much memory your samples are using.

some notable features of the DSP are:
- pitch modulation, meaning that you can use 2 channels to make a basic FM synth without eating up too much memory.
- a built in noise generator, useful for hi-hats, cymbals, rides, effects, among other things.
- per-channel echo, which unfortunately eats up a lot of memory but can be used to save channels in songs.
- an 8-tap FIR filter for the echo, which is basically a procedural low-pass filter that you can edit however you want.
- sample loop, but the loop points have to be multiples of 16.
- left/right channel invert for surround sound.
- ADSR and gain envelope modes.
- 7-bit volume per channel.
- sample interpolation, which is basically a low-pass filter that gets affected by the pitch of the channel.

Furnace also allows the SNES to use wavetables (and the wavetable synthesizer) in order to create more 'animated' sounds, using less memory than regular samples. this however is not a hardware feature, and might be difficult to implement on real hardware.

# effects

- `10xx`: **set waveform.**
- `11xx`: **toggle noise mode.**
- `12xx`: **toggle echo on this channel.**
- `13xx`: **toggle pitch modulation.** frequency modulation by the previous channel's output. no effect on channel 1.
- `14xy`: **toggle inverting the left or right channels.** `x` is left, `y` is right.
- `15xx`: **set envelope mode.** see gain chart below for `1` through `5`.
  - `0`: ADSR mode.
  - `1`: gain (direct). volume holds at one level.
  - `2`: linear decrement. volume lowers by subtractions of 1/64.
  - `3`: exponential decrement. volume lowers by multiplications of 255/256.
  - `4`: linear increment. volume rises by additions of 1/64.
  - `5`: bent line (inverse log) increment. volume rises by additions of 1/64 until 3/4, then additions of 1/256.
- `16xx`: **set gain.** `00` to `7F` if direct, `00` to `1F` otherwise.
- `18xx`: **enable echo buffer.**
- `19xx`: **set echo delay.** range is `0` to `F`.
- `1Axx`: **set left echo channel volume.**\
  `1Bxx`: **set right echo channel volume.**\
  `1Cxx`: **set echo feedback.**
  - all of these are signed numbers.
  - `00` to `7F` for 0 to 127.
  - `80` to `FF` for -128 to -1.
    - setting these to -128 is not recommended as it may cause echo output to overflow and therefore click.
- `1Dxx`: **set noise generator frequency.** range is `00` to `1F`. see noise frequencies chart below.
- `1Exx`: **set left dry / global volume.**\
  `1Fxx`: **set right dry / global volume.**
  - these do not affect echo.
- `20xx`: **set attack.** range is `0` to `F`.\
  `21xx`: **set decay.** range is `0` to `7`.\
  `22xx`: **set sustain.** range is `0` to `7`.\
  `23xx`: **set release.** range is `00` to `1F`.
  - these four are only used in ADSR envelope mode. see ADSR chart below.
- `30xx`: **set echo filter coefficient 0.**\
  `31xx`: **set echo filter coefficient 1.**\
  `32xx`: **set echo filter coefficient 2.**\
  `33xx`: **set echo filter coefficient 3.**\
  `34xx`: **set echo filter coefficient 4.**\
  `35xx`: **set echo filter coefficient 5.**\
  `36xx`: **set echo filter coefficient 6.**\
  `37xx`: **set echo filter coefficient 7.**
  - all of these are signed numbers.
  - `00` to `7F` for 0 to 127.
  - `80` to `FF` for -128 to -1.
  - _Note:_ Be sure the sum of all coefficients is between -128 and 127. sums outside that may result in overflow and therefore clicking.
  - see [SnesLab](https://sneslab.net/wiki/FIR_Filter) for a full explanation and examples.

# tables

## ADSR

| attack | 0→1 time | decay | 1→S time | sustain | ratio | release | S→0 time
| -----: | -------: | ----: | -------: | ------: | :---: | ------: | -------:
|   `00` | 4.1 sec  |  `00` | 1.2 sec  |    `00` |  1/8  |    `00` |        ∞
|   `01` | 2.5 sec  |  `01` | 740 msec |    `01` |  2/8  |    `01` |   38 sec
|   `02` | 1.5 sec  |  `02` | 440 msec |    `02` |  3/8  |    `02` |   28 sec
|   `03` | 1.0 sec  |  `03` | 290 msec |    `03` |  4/8  |    `03` |   24 sec
|   `04` | 640 msec |  `04` | 180 msec |    `04` |  5/8  |    `04` |   19 sec
|   `05` | 380 msec |  `05` | 110 msec |    `05` |  6/8  |    `05` |   14 sec
|   `06` | 260 msec |  `06` |  74 msec |    `06` |  7/8  |    `06` |   12 sec
|   `07` | 160 msec |  `07` |  37 msec |    `07` |   1   |    `07` |  9.4 sec
|   `08` |  96 msec |       |          |         |       |    `08` |  7.1 sec
|   `09` |  64 msec |       |          |         |       |    `09` |  5.9 sec
|   `0A` |  40 msec |       |          |         |       |    `0A` |  4.7 sec
|   `0B` |  24 msec |       |          |         |       |    `0B` |  3.5 sec
|   `0C` |  16 msec |       |          |         |       |    `0C` |  2.9 sec
|   `0D` |  10 msec |       |          |         |       |    `0D` |  2.4 sec
|   `0E` |   6 msec |       |          |         |       |    `0E` |  1.8 sec
|   `0F` |   0 msec |       |          |         |       |    `0F` |  1.5 sec
|        |          |       |          |         |       |    `10` |  1.2 sec
|        |          |       |          |         |       |    `11` | 880 msec
|        |          |       |          |         |       |    `12` | 740 msec
|        |          |       |          |         |       |    `13` | 590 msec
|        |          |       |          |         |       |    `14` | 440 msec
|        |          |       |          |         |       |    `15` | 370 msec
|        |          |       |          |         |       |    `16` | 290 msec
|        |          |       |          |         |       |    `17` | 220 msec
|        |          |       |          |         |       |    `18` | 180 msec
|        |          |       |          |         |       |    `19` | 150 msec
|        |          |       |          |         |       |    `1A` | 110 msec
|        |          |       |          |         |       |    `1B` |  92 msec
|        |          |       |          |         |       |    `1C` |  74 msec
|        |          |       |          |         |       |    `1D` |  55 msec
|        |          |       |          |         |       |    `1E` |  37 msec
|        |          |       |          |         |       |    `1F` |  18 msec

reference: [Super Famicom Development Wiki](https://wiki.superfamicom.org/spc700-reference#dsp-voice-register:-adsr-1097)

## gain

value | linear inc. | bent line inc. | linear dec. | exponent dec.
----: | ----------: | -------------: | ----------: | ------------:
 `00` |           ∞ |              ∞ |           ∞ |             ∞
 `01` |     4.1 sec |        7.2 sec |     4.1 sec |        38 sec
 `02` |     3.1 sec |        5.4 sec |     3.1 sec |        28 sec
 `03` |     2.6 sec |        4.6 sec |     2.6 sec |        24 sec
 `04` |     2.0 sec |        3.5 sec |     2.0 sec |        19 sec
 `05` |     1.5 sec |        2.6 sec |     1.5 sec |        14 sec
 `06` |     1.3 sec |        2.3 sec |     1.3 sec |        12 sec
 `07` |     1.0 sec |        1.8 sec |     1.0 sec |       9.4 sec
 `08` |    770 msec |        1.3 sec |    770 msec |       7.1 sec
 `09` |    640 msec |        1.1 sec |    640 msec |       5.9 sec
 `0A` |    510 msec |       900 msec |    510 msec |       4.7 sec
 `0B` |    380 msec |       670 msec |    380 msec |       3.5 sec
 `0C` |    320 msec |       560 msec |    320 msec |       2.9 sec
 `0D` |    260 msec |       450 msec |    260 msec |       2.4 sec
 `0E` |    190 msec |       340 msec |    190 msec |       1.8 sec
 `0F` |    160 msec |       280 msec |    160 msec |       1.5 sec
 `10` |    130 msec |       220 msec |    130 msec |       1.2 sec
 `11` |     96 msec |       170 msec |     96 msec |      880 msec
 `12` |     80 msec |       140 msec |     80 msec |      740 msec
 `13` |     64 msec |       110 msec |     64 msec |      590 msec
 `14` |     48 msec |        84 msec |     48 msec |      440 msec
 `15` |     40 msec |        70 msec |     40 msec |      370 msec
 `16` |     32 msec |        56 msec |     32 msec |      290 msec
 `17` |     24 msec |        42 msec |     24 msec |      220 msec
 `18` |     20 msec |        35 msec |     20 msec |      180 msec
 `19` |     16 msec |        28 msec |     16 msec |      150 msec
 `1A` |     12 msec |        21 msec |     12 msec |      110 msec
 `1B` |     10 msec |        18 msec |     10 msec |       92 msec
 `1C` |      8 msec |        14 msec |      8 msec |       74 msec
 `1D` |      6 msec |        11 msec |      6 msec |       55 msec
 `1E` |      4 msec |         7 msec |      4 msec |       37 msec
 `1F` |      2 msec |       3.5 msec |      2 msec |       18 msec

reference: [Super Famicom Development Wiki](https://wiki.superfamicom.org/spc700-reference#dsp-voice-register:-gain-1156)

## noise frequencies

value |  freq. | value |    freq.
----: | -----: | ----: | -------:
`00`  |   0 Hz | `10`  |   500 Hz
`01`  |  16 Hz | `11`  |   667 Hz
`02`  |  21 Hz | `12`  |   800 Hz
`03`  |  25 Hz | `13`  |  1.0 KHz
`04`  |  31 Hz | `14`  |  1.3 KHz
`05`  |  42 Hz | `15`  |  1.6 KHz
`06`  |  50 Hz | `16`  |  2.0 KHz
`07`  |  63 Hz | `17`  |  2.7 KHz
`08`  |  83 Hz | `18`  |  3.2 KHz
`09`  | 100 Hz | `19`  |  4.0 KHz
`0A`  | 125 Hz | `1A`  |  5.3 KHz
`0B`  | 167 Hz | `1B`  |  6.4 KHz
`0C`  | 200 Hz | `1C`  |  8.0 KHz
`0D`  | 250 Hz | `1D`  | 10.7 KHz
`0E`  | 333 Hz | `1E`  |   16 KHz
`0F`  | 400 Hz | `1F`  |   32 KHz

reference: [Super Famicom Development Wiki](https://wiki.superfamicom.org/spc700-reference#dsp-register:-flg-1318)



# resources

- [SNES-format BRR samples](https://www.smwcentral.net/?p=section&s=brrsamples) at SMW Central
