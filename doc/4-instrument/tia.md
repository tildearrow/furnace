# Atari TIA instrument editor

the TIA instrument editor consists of these macros:

- **Volume**: volume sequence.
- **Arpeggio**: pitch sequence.
  - note: fixed mode works differently. it sets the frequency directly rather than the note, so it only goes from 0 to 31.
- **Waveform**: selects waveform to be used:
  - `0`: nothing
  - `1`: buzzy
  - `2`: low buzzy
  - `3`: flangy
  - `4`: square
  - `5`: square
  - `6`: pure buzzy
  - `7`: reedy
  - `8`: noise
  - `9`: reedy
  - `A`: pure buzzy
  - `B`: nothing
  - `C`: low square
  - `D`: low square
  - `E`: low pure buzzy
  - `F`: low reedy
- **Pitch**: "fine" pitch. fine in quotes as TIA doesn't have true pitch control at all.
