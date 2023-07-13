# Standard instrument editor

The instrument editor for NES and PSG (SMS, MSX, and such) consists of these macros:

- **Volume**: volume.
- **Arpeggio**: pitch in half-steps.
- **Duty**: duty cycle and noise mode.
  - NES noise modes:
    - `0`: long noise.
    - `1`: short noise.
  - PSG noise modes:
    - `0`: short noise, preset frequencies.
    - `1`: long noise, preset frequencies.
    - `2`: short noise, use channel 3 for frequency.
    - `3`: long noise, use channel 3 for frequency.
- **Panning**: output for left and right channels.
- **Pitch**: fine pitch.
- **Phase Reset**: trigger restart of waveform.