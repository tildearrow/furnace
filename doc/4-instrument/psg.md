# PSG instrument editor

The instrument editor for PSG (SMS, MSX, and other TI SN76489 derivatives) consists of these macros:

- **Volume**: volume.
- **Arpeggio**: pitch in half-steps.
- **Duty**: noise mode.
  - `0`: short noise, preset frequencies.
  - `1`: long noise, preset frequencies.
  - `2`: short noise, use channel 3 for frequency.
  - `3`: long noise, use channel 3 for frequency.
- **Panning**: output for left and right channels.
- **Pitch**: fine pitch.
- **Phase Reset**: trigger restart of waveform.