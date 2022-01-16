# PC Engine/TurboGrafx-16

a console from NEC that attempted to enter the fierce battle between Nintendo and Sega, but because its capabilities are a mix of third and fourth generation, it failed to last long.

it has 6 wavetable channels and the last two ones also double as noise channels.
furthermore, it has some PCM!

# effects

- `10xx`: change wave.
- `11xx`: toggle noise mode. only available in the last two channels.
- `12xx`: setup LFO. the following values are accepted:
  - `00`: LFO disabled.
  - `01`: LFO enabled, shift 0.
  - `02`: LFO enabled, shift 4.
  - `03`: LFO enabled, shift 8.
  - when LFO is enabled, channel 2 is muted and its output is passed to channel 1's frequency.
- `13xx`: set LFO speed.
- `17xx`: toggle PCM mode.
