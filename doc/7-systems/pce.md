# PC Engine/TurboGrafx-16

a console from NEC that, depending on a region:
- attempted to enter the fierce battle between Nintendo and Sega, but because its capabilities are a mix of third and fourth generation, it failed to last long (US and Europe), or
- was Nintendo's most fearsome rival, completely defeating Sega Mega Drive and defending itself against Super Famicom (Japan).

it has 6 wavetable channels and the last two ones also double as noise channels.
furthermore, it has some PCM and LFO!

## effects

- `10xx`: **change wave.**
- `11xx`: **toggle noise mode.** only available in the last two channels.
- `12xx`: **setup LFO.** the following values are accepted:
  - `00`: LFO disabled.
  - `01`: LFO enabled, shift 0.
  - `02`: LFO enabled, shift 4.
  - `03`: LFO enabled, shift 8.
  - when LFO is enabled, channel 2 is muted and its output is passed to channel 1's frequency.
- `13xx`: **set LFO speed.**
- `17xx`: **toggle LEGACY sample mode.**
  - **this effect exists only for compatibility reasons! its use is NOT recommented. use Sample type instruments instead.**

## info

this chip uses the [PC Engine](../4-instrument/pce.md) instrument editor.
