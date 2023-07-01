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
|   `00` |   4.1 s  |  `00` |   1.2 s  |    `00` |  1/8  |    `00` |        ∞
|   `01` |   2.5 s  |  `01` |   740 ms |    `01` |  2/8  |    `01` |     38 s
|   `02` |   1.5 s  |  `02` |   440 ms |    `02` |  3/8  |    `02` |     28 s
|   `03` |   1.0 s  |  `03` |   290 ms |    `03` |  4/8  |    `03` |     24 s
|   `04` |   640 ms |  `04` |   180 ms |    `04` |  5/8  |    `04` |     19 s
|   `05` |   380 ms |  `05` |   110 ms |    `05` |  6/8  |    `05` |     14 s
|   `06` |   260 ms |  `06` |    74 ms |    `06` |  7/8  |    `06` |     12 s
|   `07` |   160 ms |  `07` |    37 ms |    `07` |   1   |    `07` |    9.4 s
|   `08` |    96 ms |       |          |         |       |    `08` |    7.1 s
|   `09` |    64 ms |       |          |         |       |    `09` |    5.9 s
|   `0A` |    40 ms |       |          |         |       |    `0A` |    4.7 s
|   `0B` |    24 ms |       |          |         |       |    `0B` |    3.5 s
|   `0C` |    16 ms |       |          |         |       |    `0C` |    2.9 s
|   `0D` |    10 ms |       |          |         |       |    `0D` |    2.4 s
|   `0E` |     6 ms |       |          |         |       |    `0E` |    1.8 s
|   `0F` |     0 ms |       |          |         |       |    `0F` |    1.5 s
|        |          |       |          |         |       |    `10` |    1.2 s
|        |          |       |          |         |       |    `11` |   880 ms
|        |          |       |          |         |       |    `12` |   740 ms
|        |          |       |          |         |       |    `13` |   590 ms
|        |          |       |          |         |       |    `14` |   440 ms
|        |          |       |          |         |       |    `15` |   370 ms
|        |          |       |          |         |       |    `16` |   290 ms
|        |          |       |          |         |       |    `17` |   220 ms
|        |          |       |          |         |       |    `18` |   180 ms
|        |          |       |          |         |       |    `19` |   150 ms
|        |          |       |          |         |       |    `1A` |   110 ms
|        |          |       |          |         |       |    `1B` |    92 ms
|        |          |       |          |         |       |    `1C` |    74 ms
|        |          |       |          |         |       |    `1D` |    55 ms
|        |          |       |          |         |       |    `1E` |    37 ms
|        |          |       |          |         |       |    `1F` |    18 ms

reference: [Super Famicom Development Wiki](https://wiki.superfamicom.org/spc700-reference#dsp-voice-register:-adsr-1097)

## gain

value | linear inc. | bent line inc. | linear dec. | exponent dec.
----: | ----------: | -------------: | ----------: | ------------:
 `00` |           ∞ |              ∞ |           ∞ |             ∞
 `01` |       4.1 s |          7.2 s |       4.1 s |          38 s
 `02` |       3.1 s |          5.4 s |       3.1 s |          28 s
 `03` |       2.6 s |          4.6 s |       2.6 s |          24 s
 `04` |       2.0 s |          3.5 s |       2.0 s |          19 s
 `05` |       1.5 s |          2.6 s |       1.5 s |          14 s
 `06` |       1.3 s |          2.3 s |       1.3 s |          12 s
 `07` |       1.0 s |          1.8 s |       1.0 s |         9.4 s
 `08` |      770 ms |          1.3 s |      770 ms |         7.1 s
 `09` |      640 ms |          1.1 s |      640 ms |         5.9 s
 `0A` |      510 ms |         900 ms |      510 ms |         4.7 s
 `0B` |      380 ms |         670 ms |      380 ms |         3.5 s
 `0C` |      320 ms |         560 ms |      320 ms |         2.9 s
 `0D` |      260 ms |         450 ms |      260 ms |         2.4 s
 `0E` |      190 ms |         340 ms |      190 ms |         1.8 s
 `0F` |      160 ms |         280 ms |      160 ms |         1.5 s
 `10` |      130 ms |         220 ms |      130 ms |         1.2 s
 `11` |       96 ms |         170 ms |       96 ms |        880 ms
 `12` |       80 ms |         140 ms |       80 ms |        740 ms
 `13` |       64 ms |         110 ms |       64 ms |        590 ms
 `14` |       48 ms |          84 ms |       48 ms |        440 ms
 `15` |       40 ms |          70 ms |       40 ms |        370 ms
 `16` |       32 ms |          56 ms |       32 ms |        290 ms
 `17` |       24 ms |          42 ms |       24 ms |        220 ms
 `18` |       20 ms |          35 ms |       20 ms |        180 ms
 `19` |       16 ms |          28 ms |       16 ms |        150 ms
 `1A` |       12 ms |          21 ms |       12 ms |        110 ms
 `1B` |       10 ms |          18 ms |       10 ms |         92 ms
 `1C` |        8 ms |          14 ms |        8 ms |         74 ms
 `1D` |        6 ms |          11 ms |        6 ms |         55 ms
 `1E` |        4 ms |           7 ms |        4 ms |         37 ms
 `1F` |        2 ms |         3.5 ms |        2 ms |         18 ms

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

- [SNES-format BRR samples](https://www.smwcentral.net/?p=stion&s=brrsamples) at SMW Central
