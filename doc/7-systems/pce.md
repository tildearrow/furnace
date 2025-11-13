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

## info

this chip uses the [PC Engine](../4-instrument/pce.md) instrument editor.

## channel status

the following icons are displayed when channel status is enabled in the pattern view:

- noise mode (channels 5 and 6 only):
  - ![noise mode off](status-PCE-noise-off.png) off
  - ![noise mode on](status-PCE-noise-on.png) on

## chip config

the following options are available in the Chip Manager window:

- **Pseudo-PAL**: run the chip on a PAL clock. such a configuration has (probably) never existed, despite a planned official PAL version of the PC Engine.
- **Disable anti-click**: waveform switching requires a phase reset, which may cause clicks. Furnace uses a wave-position predicting algorithm to minimize these clicks. enable this option to turn off the feature.
- **Chip revision**: sets the chip revision. HuC6280A has less pops.
