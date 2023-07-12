# NES

the console from Nintendo that plays Super Mario Bros. and helped revive the agonizing video game market in the US during mid-80s.

also known as Famicom. it is a five-channel sound generator: first two channels play pulse wave with three different duty cycles, third is a fixed-volume triangle channel, fourth is a noise channel (can work in both pseudo-random and periodic modes) and fifth is a (D)PCM sample channel.

# effects

- `11xx`: **write to delta modulation counter.** range is `00` to `7F`.
  - this may be used to attenuate the triangle and noise channels; at `7F`, they will be at about 57% volume.
  - will not work if a sample is playing.
- `12xx`: **set duty cycle or noise mode of channel.**
  - may be `0` to `3` for the pulse channels:
    - `0`: 12.5%
    - `1`: 25%
    - `2`: 50%
    - `3`: 75%
  - may be `0` or `1` for the noise channel:
    - `0`: long (15-bit LFSR, 32767-step)
    - `1`: short (9-bit LFSR, 93-step)
- `13xy`: **setup sweep up.**
  - `x` is the time.
  - `y` is the shift.
  - set to `0` to disable it.
- `14xy`: **setup sweep down.**
  - `x` is the time.
  - `y` is the shift.
  - set to `0` to disable it.
- `15xx`: **set envelope mode.**
  - `0`: envelope + length counter. volume represents envelope duration.
  - `1`: length counter. volume represents output volume.
  - `2`: looping envelope. volume represents envelope duration.
  - `3`: constant volume. default value. volume represents output volume.
  - Pulse and noise channels only.
  - you may need to apply a phase reset (using the macro) to make the envelope effective.
- `16xx`: **set length counter.**
  - see table below for possible values.
  - this will trigger phase reset.
- `17xx`: **set frame counter mode.**
  - `0`: 4-step.
    - NTSC: 120Hz sweeps and lengths; 240Hz envelope.
    - PAL: 100Hz sweeps and lengths; 200Hz envelope.
    - Dendy: 118.9Hz sweeps and lengths; 237.8Hz envelope.
  - `1`: 5-step.
    - NTSC: 96Hz sweeps and lengths; 192Hz envelope.
    - PAL: 80Hz sweeps and lengths; 160Hz envelope.
    - Dendy: 95.1Hz sweeps and lengths; 190.2Hz envelope.
- `18xx`: **set PCM channel mode.**
  - `00`: PCM (software).
  - `01`: DPCM (hardware).
  - when in DPCM mode, samples will sound muffled (due to its nature), availables pitches are limited, and loop point is ignored.
- `19xx`: **set triangle linear counter.**
  - `00` to `7F` set the counter.
  - `80` and higher halt it.
- `20xx`: **set DPCM frequency.**
  - only works in DPCM mode.
  - see table below for possible values.

# tables

## short noise frequencies (NTSC)

note  | arpeggio | fundamental | MIDI note | pitch
:---- | -------: | ----------: | --------: | :----------
`C-0` |       @0 |      4.7 Hz |     -9.47 | `d_1` + 53¢ 
`C#0` |       @1 |      9.5 Hz |      2.53 | `D-0` + 53¢
`D-0` |       @2 |     18.9 Hz |     14.55 | `D-1` + 55¢
`D#0` |       @3 |     25.3 Hz |     19.53 | `G-1` + 53¢
`E-0` |       @4 |     37.9 Hz |     26.55 | `D-2` + 55¢
`F-0` |       @5 |     50.6 Hz |     31.57 | `G-2` + 57¢
`F#0` |       @6 |     75.8 Hz |     38.55 | `D-3` + 55¢
`G-0` |       @7 |     95.3 Hz |     42.51 | `F#3` + 51¢
`G#0` |       @8 |    120.3 Hz |     46.55 | `A#3` + 55¢
`A-0` |       @9 |    150.4 Hz |     50.41 | `D-4` + 41¢
`A#0` |      @10 |    200.5 Hz |     55.39 | `G-4` + 39¢
`B-0` |      @11 |    300.7 Hz |     62.41 | `D-5` + 41¢
`C-1` |      @12 |    601.4 Hz |     74.41 | `D-6` + 41¢
`C#1` |      @13 |   1202.8 Hz |     86.41 | `D-7` + 41¢
`D-1` |      @14 |   2405.6 Hz |     98.41 | `D-8` + 41¢
`D#1` |      @15 |   4811.2 Hz |    110.41 | `D-9` + 41¢

reference: [NESdev](https://www.nesdev.org/wiki/APU_Noise)

## length counter table

<!-- organized by input value... of little use -->
<!--
value | raw | NTSC  | PAL   | Dendy | NTSC 5-step | PAL 5-step | Dendy 5-step
-----:|----:|------:|------:|------:|------------:|-----------:|-------------:
 `00` |  10 | 83ms  | 100ms | 84ms  | 104ms       | 125ms      | 105ms
 `01` | 254 | 2.1s  | 2.5s  | 2.1s  | 2.6s        | 3.2s       | 2.7s
 `02` |  20 | 166ms | 200ms | 168ms | 208ms       | 250ms      | 210ms
 `03` |   2 | 17ms  | 20ms  | 17ms  | 21ms        | 25ms       | 21ms
 `04` |  40 | 333ms | 400ms | 336ms | 417ms       | 500ms      | 421ms
 `05` |   4 | 33ms  | 40ms  | 34ms  | 42ms        | 50ms       | 42ms
 `06` |  80 | 667ms | 800ms | 673ms | 833ms       | 1.0s       | 841ms
 `07` |   6 | 50ms  | 60ms  | 50ms  | 63ms        | 75ms       | 63ms
 `08` | 160 | 1.3s  | 1.6s  | 1.3s  | 1.7s        | 2.0s       | 1.7s
 `09` |   8 | 67ms  | 80ms  | 67ms  | 83ms        | 100ms      | 84ms
 `0A` |  60 | 500ms | 600ms | 505ms | 625ms       | 750ms      | 631ms
 `0B` |  10 | 83ms  | 100ms | 84ms  | 104ms       | 125ms      | 105ms
 `0C` |  14 | 117ms | 140ms | 118ms | 146ms       | 175ms      | 147ms
 `0D` |  12 | 100ms | 120ms | 101ms | 125ms       | 150ms      | 126ms
 `0E` |  26 | 217ms | 260ms | 219ms | 271ms       | 325ms      | 273ms
 `0F` |  14 | 117ms | 140ms | 118ms | 145ms       | 175ms      | 147ms
 `10` |  12 | 100ms | 120ms | 101ms | 125ms       | 150ms      | 126ms
 `11` |  16 | 133ms | 160ms | 135ms | 167ms       | 200ms      | 168ms
 `12` |  24 | 200ms | 240ms | 202ms | 250ms       | 300ms      | 252ms
 `13` |  18 | 150ms | 180ms | 151ms | 188ms       | 225ms      | 189ms
 `14` |  48 | 400ms | 480ms | 404ms | 500ms       | 600ms      | 505ms
 `15` |  20 | 167ms | 200ms | 168ms | 208ms       | 250ms      | 210ms
 `16` |  96 | 800ms | 960ms | 807ms | 1.0s        | 1.2s       | 1.0s
 `17` |  22 | 183ms | 220ms | 185ms | 229ms       | 275ms      | 231ms
 `18` | 192 | 1.6s  | 1.9s  | 1.6s  | 2.0s        | 2.4s       | 2.0s
 `19` |  24 | 200ms | 240ms | 202ms | 250ms       | 300ms      | 252ms
 `1A` |  72 | 600ms | 720ms | 606ms | 750ms       | 900ms      | 757ms
 `1B` |  26 | 217ms | 260ms | 219ms | 271ms       | 325ms      | 273ms
 `1C` |  16 | 133ms | 160ms | 135ms | 167ms       | 200ms      | 168ms
 `1D` |  28 | 233ms | 280ms | 235ms | 292ms       | 350ms      | 294ms
 `1E` |  32 | 267ms | 320ms | 269ms | 333ms       | 400ms      | 336ms
 `1F` |  30 | 250ms | 300ms | 252ms | 313ms       | 375ms      | 315ms
-->

<!-- organized by resulting times... more useful! -->
value | raw | NTSC  | PAL   | Dendy | NTSC 5-step | PAL 5-step | Dendy 5-step
-----:|----:|------:|------:|------:|------------:|-----------:|-------------:
 `03` |   2 | 17ms  | 20ms  | 17ms  | 21ms        | 25ms       | 21ms
 `05` |   4 | 33ms  | 40ms  | 34ms  | 42ms        | 50ms       | 42ms
 `07` |   6 | 50ms  | 60ms  | 50ms  | 63ms        | 75ms       | 63ms
 `09` |   8 | 67ms  | 80ms  | 67ms  | 83ms        | 100ms      | 84ms
 `00` |  10 | 83ms  | 100ms | 84ms  | 104ms       | 125ms      | 105ms
 `0B` |  10 | 83ms  | 100ms | 84ms  | 104ms       | 125ms      | 105ms
 `0D` |  12 | 100ms | 120ms | 101ms | 125ms       | 150ms      | 126ms
 `10` |  12 | 100ms | 120ms | 101ms | 125ms       | 150ms      | 126ms
 `0C` |  14 | 117ms | 140ms | 118ms | 146ms       | 175ms      | 147ms
 `0F` |  14 | 117ms | 140ms | 118ms | 145ms       | 175ms      | 147ms
 `1C` |  16 | 133ms | 160ms | 135ms | 167ms       | 200ms      | 168ms
 `11` |  16 | 133ms | 160ms | 135ms | 167ms       | 200ms      | 168ms
 `13` |  18 | 150ms | 180ms | 151ms | 188ms       | 225ms      | 189ms
 `02` |  20 | 166ms | 200ms | 168ms | 208ms       | 250ms      | 210ms
 `15` |  20 | 167ms | 200ms | 168ms | 208ms       | 250ms      | 210ms
 `17` |  22 | 183ms | 220ms | 185ms | 229ms       | 275ms      | 231ms
 `12` |  24 | 200ms | 240ms | 202ms | 250ms       | 300ms      | 252ms
 `19` |  24 | 200ms | 240ms | 202ms | 250ms       | 300ms      | 252ms
 `0E` |  26 | 217ms | 260ms | 219ms | 271ms       | 325ms      | 273ms
 `1B` |  26 | 217ms | 260ms | 219ms | 271ms       | 325ms      | 273ms
 `1D` |  28 | 233ms | 280ms | 235ms | 292ms       | 350ms      | 294ms
 `1F` |  30 | 250ms | 300ms | 252ms | 313ms       | 375ms      | 315ms
 `1E` |  32 | 267ms | 320ms | 269ms | 333ms       | 400ms      | 336ms
 `04` |  40 | 333ms | 400ms | 336ms | 417ms       | 500ms      | 421ms
 `14` |  48 | 400ms | 480ms | 404ms | 500ms       | 600ms      | 505ms
 `0A` |  60 | 500ms | 600ms | 505ms | 625ms       | 750ms      | 631ms
 `1A` |  72 | 600ms | 720ms | 606ms | 750ms       | 900ms      | 757ms
 `06` |  80 | 667ms | 800ms | 673ms | 833ms       | 1.0s       | 841ms
 `16` |  96 | 800ms | 960ms | 807ms | 1.0s        | 1.2s       | 1.0s
 `08` | 160 | 1.3s  | 1.6s  | 1.3s  | 1.7s        | 2.0s       | 1.7s
 `18` | 192 | 1.6s  | 1.9s  | 1.6s  | 2.0s        | 2.4s       | 2.0s
 `01` | 254 | 2.1s  | 2.5s  | 2.1s  | 2.6s        | 3.2s       | 2.7s

reference: [NESdev](https://www.nesdev.org/wiki/APU_Length_Counter)

## DPCM frequency table

value | NTSC      | PAL
------|----------:|----------:
 `00` | 4181.7Hz  | 4177.4Hz
 `01` | 4709.9Hz  | 4696.6Hz
 `02` | 5264.0Hz  | 5261.4Hz
 `03` | 5593.0Hz  | 5579.2Hz
 `04` | 6257.9Hz  | 6023.9Hz
 `05` | 7046.3Hz  | 7044.9Hz
 `06` | 7919.3Hz  | 7917.2Hz
 `07` | 8363.4Hz  | 8397.0Hz
 `08` | 9419.9Hz  | 9446.6Hz
 `09` | 11186.1Hz | 11233.8Hz
 `0A` | 12604.0Hz | 12595.5Hz
 `0B` | 13982.6Hz | 14089.9Hz
 `0C` | 16884.6Hz | 16965.4Hz
 `0D` | 21306.8Hz | 21315.5Hz
 `0E` | 24858.0Hz | 25191.0Hz
 `0F` | 33143.9Hz | 33252.1Hz
