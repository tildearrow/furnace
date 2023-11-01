# WonderSwan

a handheld console released only in Japan by Bandai, designed by the same people behind Game Boy and Virtual Boy.
for this reason it has lots of similar elements from those two systems in the sound department.

it has 4 wavetable channels. some of them have additional capabilities:
- the second channel could play samples
- the third one has hardware sweep
- the fourth one also does noise

## effects

- `10xx`: **change wave**.
- `11xx`: **setup noise mode.** channel 4 only.
  - 0: disable.
  - 1-8: enable and set length.
- `12xx`: **setup sweep period.** channel 3 only.
  - 0: disable.
  - 1-32: enable and set period.
- `13xx`: **setup sweep amount.** channel 3 only.
  - `00` to `7F` for 0 to 127.
  - `80` to `FF` for -128 to -1.

## info

this chip uses the [WonderSwan](../4-instrument/wonderswan.md) instrument editor.

## channel status

when enabled, channel status will show an additional icon representing the mode of the currently playing note:

- ![PCM mode off](status-Swan-PCM-off.png) PCM mode off
- ![PCM mode on](status-Swan-PCM-on.png) PCM mode on
- ![sweep mode off](status-Swan-sweep-off.png) sweep mode off
- ![sweep mode on](status-Swan-sweep-on.png) sweep mode on
- ![noise mode off](status-Swan-noise-off.png) noise mode off
- ![noise mode on](status-Swan-noise-on.png) noise mode on
