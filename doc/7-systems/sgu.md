# SGU-1

a brand new sound chip that cherry-picks the best features of OPL/ESFM, OPM, SID, POKEY, and Paula, combining 4-operator FM synthesis with per-channel subtractive filtering and hardware sweeps.

it has the following capabilities:

- 9 channels of 4-operator FM synthesis
- stereo sound
- 8 waveforms per operator (sine, triangle, sawtooth, pulse, noise, periodic noise, reserved, sample)
- per-operator waveform parameter (WPAR) for wave shaping
- flexible ESFM-style operator routing (per-operator output and modulation input levels)
- per-operator hard sync and ring modulation
- OPN-style ADSR envelope with sustain rate control (AR/DR/SL/SR/RR) and 5-bit attack/decay/sustain rates
- per-channel resonant filter (low pass, band pass, high pass, ring modulation)
- 128 pulse widths for pulse waveform
- volume, frequency and cutoff sweep units (per-channel)
- phase reset timer (per-channel)
- 64KB PCM sample memory
- hardware sequencer (same as Sound Unit)

## effects

__note:__ unlike Sound Unit, waveforms are selected per-operator in the instrument editor, not per-channel. the `10xx` effect does not apply to SGU-1.

__note:__ FM operator parameters (attack rate, decay rate, total level, multiplier, etc.) are controlled through macros in the instrument editor, not through pattern effects.

- `12xx`: __set pulse width.__ range is `0` to `7F`.
- `13xx`: __set resonance of filter.__ range is `0` to `FF`.
- `14xx`: __set filter mode and ring mod.__
  - bit 0: ring mod with next channel
  - bit 1: low pass
  - bit 2: high pass
  - bit 3: band pass
- `15xx`: __set frequency sweep period low byte.__
- `16xx`: __set frequency sweep period high byte.__
- `17xx`: __set volume sweep period low byte.__
- `18xx`: __set volume sweep period high byte.__
- `19xx`: __set cutoff sweep period low byte.__
- `1Axx`: __set cutoff sweep period high byte.__
- `1Bxx`: __set frequency sweep boundary.__
- `1Cxx`: __set volume sweep boundary.__
- `1Dxx`: __set cutoff sweep boundary.__
- `1Exx`: __set phase reset period low byte.__
- `1Fxx`: __set phase reset period high byte.__
- `20xx`: __toggle frequency sweep.__
  - bit 0-6: speed
  - bit 7: up direction
- `21xx`: __toggle volume sweep.__
  - bit 0-4: speed
  - bit 5: up direction
  - bit 6: loop
  - bit 7: alternate
- `22xx`: __toggle cutoff sweep.__
  - bit 0-6: speed
  - bit 7: up direction
- `23xx`: __pulse width slide up.__
- `24xx`: __pulse width slide down.__
- `25xx`: __filter cutoff slide up.__
- `26xx`: __filter cutoff slide down.__
- `4xxx`: __set cutoff.__ range is `0` to `FFF`.

## info

this chip uses the [SGU-1](../4-instrument/sgu.md) instrument editor.
