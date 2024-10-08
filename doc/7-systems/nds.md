# Nintendo DS

this portable video game console succeeded the Game Boy Advance.
it has 16 channels of sampled sound, supporting 8-bit PCM, 16-bit PCM and IMA ADPCM.

additionally, the last 8 channels may be put in "PSG mode", featuring 6 channels of pulse with 8 duty cycles and 2 noise channels.

# effects

- `12xy`: **set duty cycle.**
  - `0` to `7`.
  - only works in PSG channels.
- `1Fxx`: **set global volume.**

## info

this chip uses the [Nintendo DS](../4-instrument/nds.md) instrument editor.

## chip config

the following options are available in the Chip Manager window:

- **Model**: set the amount of memory available for samples.
