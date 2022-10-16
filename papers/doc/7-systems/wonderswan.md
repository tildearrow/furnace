# WonderSwan

a handheld console released only in Japan by Bandai, designed by the same people behind Game Boy and Virtual Boy.
for this reason it has lots of similar elements from those two systems in the sound department.

it has 4 wavetable channels. some of them have additional capabilities:
- the second channel could play samples
- the third one has hardware sweep
- the fourth one also does noise

# effects

- `10xx`: change wave.
- `11xx`: setup noise mode (channel 4 only).
  - 0: disable.
  - 1-8: enable and set tap preset.
- `12xx`: setup sweep period (channel 3 only).
  - 0: disable.
  - 1-32: enable and set period.
- `13xx`: setup sweep amount (channel 3 only).
- `17xx`: toggle PCM mode (channel 2 only).
