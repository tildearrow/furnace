# Sega PSG instrument editor

the instrument editor for Sega PSG (SMS, and other TI SN76489 derivatives) consists of these macros:

- **Volume**: volume sequence.
- **Arpeggio**: pitch sequence.
- **Duty**: noise mode.
  - `0`: short noise; preset frequencies.
  - `1`: long noise; preset frequencies.
  - `2`: short noise; use channel 3 for frequency.
  - `3`: long noise; use channel 3 for frequency.
- **Panning**: output for left and right channels.
  - only on Game Gear!
- **Pitch**: fine pitch.
- **Phase Reset**: trigger restart of **noise only**.
