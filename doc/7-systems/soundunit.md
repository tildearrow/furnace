# tildearrow Sound Unit

a fantasy sound chip, used in the specs2 fantasy computer designed by tildearrow.

it has the following capabilities:
- 8 channels of either waveform or sample
- stereo sound
- 8 waveforms (pulse, saw, sine, triangle, noise, periodic noise, XOR sine and XOR triangle)
- 128 widths for the pulse wave
- per-channel resonant filter
- ring modulation
- volume, frequency and cutoff sweep units (per-channel)
- phase reset timer (per-channel)

## effects

- `10xx`: **set waveform.**
  - `0`: pulse wave
  - `1`: sawtooth
  - `2`: sine wave
  - `3`: triangle wave
  - `4`: noise
  - `5`: periodic noise
  - `6`: XOR sine
  - `7`: XOR triangle
- `12xx`: **set pulse width.** range is `0` to `7F`.
- `13xx`: **set resonance of filter.** range is `0` to `FF`.
  - despite what the internal effects list says (`0` to `F`), you can use a resonance value from `0` to `FF` (255).
- `14xx`: **set filter mode and ringmod.**
  - bit 0: ring mod
  - bit 1: low pass
  - bit 2: high pass
  - bit 3: band pass
- `15xx`: **set frequency sweep period low byte.**
- `16xx`: **set frequency sweep period high byte.**
- `17xx`: **set volume sweep period low byte.**
- `18xx`: **set volume sweep period high byte.**
- `19xx`: **set cutoff sweep period low byte.**
- `1Axx`: **set cutoff sweep period high byte.**
- `1Bxx`: **set frequency sweep boundary.**
- `1Cxx`: **set volume sweep boundary.**
- `1Dxx`: **set cutoff sweep boundary.**
- `1Exx`: **set phase reset period low byte.**
- `1Fxx`: **set phase reset period high byte.**
- `20xx`: **toggle frequency sweep.**
  - bit 0-6: speed
  - bit 7: up direction
- `21xx`: **toggle volume sweep.**
  - bit 0-4: speed
  - bit 5: up direction
  - bit 6: loop
  - bit 7: alternate
- `22xx`: **toggle cutoff sweep.**
  - bit 0-6: speed
  - bit 7: up direction
- `4xxx`: **set cutoff.** range is `0` to `FFF`.

## info

this chip uses the [Sound Unit](../4-instrument/su.md) instrument editor.

## chip config

the following options are available in the Chip Manager window:

- **CPU rate**: sets the rate at which the chip will run.
- **Sample memory**: sets the amount of memory available for samples.
- **DAC resolution**: sets output resolution.
- **Enable echo**: guess.
- **Swap echo channels**: puts left into right and vice-versa.
- **Echo delay**: set echo time.
- **Echo resolution**: set echo resolution. sacrifices quality for time.
- **Echo feedback**: guess.
- **Echo volume**: yep, guess again.
