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

sample length, loop start, and loop end are 32-bit word aligned, meaning they must be divisible evenly by the alignments shown in the table below:

| sample type | sample length max | loop start max | alignment |
|:-----------:|------------------:|---------------:|----------:|
| 16-bit PCM  |           8388606 |         131070 | 2 samples |
| 8-bit PCM   |          16777212 |         262140 | 4 samples |
| IMA ADPCM   |          33554424 |         524280 | 8 samples |

## chip config

the following options are available in the Chip Manager window:

- **Model**: set the amount of memory available for samples.
