# Super Nintendo Entertainment System (SNES) / Super Famicom

the successor to NES to compete with Genesis, packing superior graphics and sample-based audio.

its Sony-developed audio system features a DSP chip, SPC700 CPU, and 64KB of dedicated SRAM used by both.
this whole system itself is pretty much a separate computer that the main CPU needs to upload its program and samples to.

Furnace communicates with the DSP directly and provides a full 64KB of memory. this memory might be reduced excessively on ROM export to make up for playback engine and pattern data. you can go to window > statistics to see how much memory your samples are using.

some notable features of the DSP are:
- pitch modulation.
- a built in noise generator, useful for hi-hats, cymbals, rides, effects, among other things.
- per-channel echo, which unfortunately eats up a lot of memory but can be used to save channels in songs.
- an 8-tap FIR filter for the echo, which is basically a procedural low-pass filter that you can edit however you want.
- sample loop, but the loop points have to be multiples of 16.
- left/right channel invert for surround sound.
- ADSR and gain envelope modes.
- 7-bit volume per channel.
- sample interpolation, which is basically a low-pass filter that gets affected by the pitch of the channel.

Furnace also allows the SNES to use wavetables (and the wavetable synthesizer) in order to create more 'animated' sounds, using less memory than regular samples.

## effects

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
- `1Axx`: **set left echo channel volume.**
- `1Bxx`: **set right echo channel volume.**
- `1Cxx`: **set echo feedback.**
  - all of these are signed numbers.
  - `00` to `7F` for 0 to 127.
  - `80` to `FF` for -128 to -1.
  - setting these to -128 is not recommended as it may cause echo output to overflow and therefore click.
- `1Dxx`: **set noise generator frequency.** range is `00` to `1F`. see noise frequencies chart below.
- `1Exx`: **set left dry / global volume.**
- `1Fxx`: **set right dry / global volume.**
  - these do not affect echo.
- `20xx`: **set attack.** range is `0` to `F`.
- `21xx`: **set decay.** range is `0` to `7`.
- `22xx`: **set sustain.** range is `0` to `7`.
- `23xx`: **set release.** range is `00` to `1F`.
  - these four are only used in ADSR envelope mode. see ADSR chart below.
- `3xyy`: **set echo filter coefficient.**
  - `x` is the coefficient from 0 to 7.
  - `yy` is the value (signed number).
    - `00` to `7F` for 0 to 127.
    - `80` to `FF` for -128 to -1.
  - note: be sure the sum of all coefficients is between -128 and 127. sums outside that may result in overflow and therefore clicking.
  - see SnesLab for [echo filter explanations and examples](https://sneslab.net/wiki/FIR_Filter#Uses).

## info

this chip uses the [SNES](../4-instrument/snes.md) instrument editor.

when two channels are joined for pitch modulation, the channel bar will show `mod` on a bracket tying them together.

when using sample offset commands, be sure to open each involved sample in the sample editor, look to the "Info" section at the top-left, and check the "no BRR filters" box. this prevents sound glitches, at the cost of lowering the sample quality to 4-bit.

## channel status

the following icons are displayed when channel status is enabled in the pattern view:

- envelope/gain status:
  - ![direct gain, envelope off](status-SNES-gain-direct.png) direct gain
  - ![linear gain decrease](status-SNES-gain-dec-lin.png) linear gain decrease
  - ![logarithmic gain decrease](status-SNES-gain-dec-log.png) logarithmic gain decrease
  - ![linear gain increase](status-SNES-gain-inc-lin.png) linear gain increase
  - ![bent-line gain increase](status-SNES-gain-inc-bent.png) bent-line gain increase
  - ![envelope attack](status-SNES-env-A.png) envelope attack
  - ![envelope decay](status-SNES-env-D.png) envelope decay
  - ![envelope sustain](status-SNES-env-S.png) envelope sustain
  - ![envelope release](status-SNES-env-R.png) envelope release

## chip config

the following options are available in the Chip Manager window:

- **Volume scale**: scale volumes to prevent clipping/distortion.
- **Enable echo**: enables the echo buffer.
- **Initial echo state**: selects which channels will have echo applied.
- **Delay**: sets echo length.
- **Feedback**: sets how much of the echo output will be fed back into the buffer.
- **Echo volume**: sets echo volume.
- **Echo filter**: adjusts echo filter.
- **Dec/Hex**: toggles decimal or hexadecimal mode for the filter settings text entry box to the right.
  - SnesLab provides [echo filter explanations and examples](https://sneslab.net/wiki/FIR_Filter#Uses). their example filter strings can be pasted directly into the filter settings text entry box if set to Hex mode.
- **Disable Gaussian interpolation**: removes sample interpolation, resulting in crisper but aliased sound. not accurate to hardware.
- **Anti-click**: reduces clicks in the output by using hardware envelopes to smooth them out.
  - make sure your samples start on center, or else you will still hear clicks.

## ADSR

| attack | 0→1 time | decay | 1→S time | sustain | ratio | release | S→0 time
| -----: | -------: | ----: | -------: | ------: | :---: | ------: | -------:
|   `00` |    4.1s  |  `00` |    1.2s  |    `00` |  1/8  |    `00` |        ∞
|   `01` |    2.5s  |  `01` |    740ms |    `01` |  2/8  |    `01` |      38s
|   `02` |    1.5s  |  `02` |    440ms |    `02` |  3/8  |    `02` |      28s
|   `03` |    1.0s  |  `03` |    290ms |    `03` |  4/8  |    `03` |      24s
|   `04` |    640ms |  `04` |    180ms |    `04` |  5/8  |    `04` |      19s
|   `05` |    380ms |  `05` |    110ms |    `05` |  6/8  |    `05` |      14s
|   `06` |    260ms |  `06` |     74ms |    `06` |  7/8  |    `06` |      12s
|   `07` |    160ms |  `07` |     37ms |    `07` |   1   |    `07` |     9.4s
|   `08` |     96ms |       |          |         |       |    `08` |     7.1s
|   `09` |     64ms |       |          |         |       |    `09` |     5.9s
|   `0A` |     40ms |       |          |         |       |    `0A` |     4.7s
|   `0B` |     24ms |       |          |         |       |    `0B` |     3.5s
|   `0C` |     16ms |       |          |         |       |    `0C` |     2.9s
|   `0D` |     10ms |       |          |         |       |    `0D` |     2.4s
|   `0E` |      6ms |       |          |         |       |    `0E` |     1.8s
|   `0F` |      0ms |       |          |         |       |    `0F` |     1.5s
|        |          |       |          |         |       |    `10` |     1.2s
|        |          |       |          |         |       |    `11` |    880ms
|        |          |       |          |         |       |    `12` |    740ms
|        |          |       |          |         |       |    `13` |    590ms
|        |          |       |          |         |       |    `14` |    440ms
|        |          |       |          |         |       |    `15` |    370ms
|        |          |       |          |         |       |    `16` |    290ms
|        |          |       |          |         |       |    `17` |    220ms
|        |          |       |          |         |       |    `18` |    180ms
|        |          |       |          |         |       |    `19` |    150ms
|        |          |       |          |         |       |    `1A` |    110ms
|        |          |       |          |         |       |    `1B` |     92ms
|        |          |       |          |         |       |    `1C` |     74ms
|        |          |       |          |         |       |    `1D` |     55ms
|        |          |       |          |         |       |    `1E` |     37ms
|        |          |       |          |         |       |    `1F` |     18ms

reference: [Super Famicom Development Wiki](https://wiki.superfamicom.org/spc700-reference#dsp-voice-register:-adsr-1097)

## gain

value | linear inc. | bent line inc. | linear dec. | exponent dec.
----: | ----------: | -------------: | ----------: | ------------:
 `00` |           ∞ |              ∞ |           ∞ |             ∞
 `01` |        4.1s |           7.2s |        4.1s |           38s
 `02` |        3.1s |           5.4s |        3.1s |           28s
 `03` |        2.6s |           4.6s |        2.6s |           24s
 `04` |        2.0s |           3.5s |        2.0s |           19s
 `05` |        1.5s |           2.6s |        1.5s |           14s
 `06` |        1.3s |           2.3s |        1.3s |           12s
 `07` |        1.0s |           1.8s |        1.0s |          9.4s
 `08` |       770ms |           1.3s |       770ms |          7.1s
 `09` |       640ms |           1.1s |       640ms |          5.9s
 `0A` |       510ms |          900ms |       510ms |          4.7s
 `0B` |       380ms |          670ms |       380ms |          3.5s
 `0C` |       320ms |          560ms |       320ms |          2.9s
 `0D` |       260ms |          450ms |       260ms |          2.4s
 `0E` |       190ms |          340ms |       190ms |          1.8s
 `0F` |       160ms |          280ms |       160ms |          1.5s
 `10` |       130ms |          220ms |       130ms |          1.2s
 `11` |        96ms |          170ms |        96ms |         880ms
 `12` |        80ms |          140ms |        80ms |         740ms
 `13` |        64ms |          110ms |        64ms |         590ms
 `14` |        48ms |           84ms |        48ms |         440ms
 `15` |        40ms |           70ms |        40ms |         370ms
 `16` |        32ms |           56ms |        32ms |         290ms
 `17` |        24ms |           42ms |        24ms |         220ms
 `18` |        20ms |           35ms |        20ms |         180ms
 `19` |        16ms |           28ms |        16ms |         150ms
 `1A` |        12ms |           21ms |        12ms |         110ms
 `1B` |        10ms |           18ms |        10ms |          92ms
 `1C` |         8ms |           14ms |         8ms |          74ms
 `1D` |         6ms |           11ms |         6ms |          55ms
 `1E` |         4ms |            7ms |         4ms |          37ms
 `1F` |         2ms |          3.5ms |         2ms |          18ms

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

## resources

- [SNES-format BRR samples](https://www.smwcentral.net/?p=stion&s=brrsamples) at SMW Central
