# Commodore Amiga

a computer with a desktop OS, lifelike graphics and 4 channels of PCM sound in 1985? no way!

in this very computer music trackers were born...

imported MOD files use this chip, and will set A-4 tuning to 436.

# effects

- `10xx`: **toggle low-pass filter.** `0` turns it off and `1` turns it on.
- `11xx`: **toggle amplitude modulation with the next channel.**
  - does not work on the last channel.
- `12xx`: **toggle period (frequency) modulation with the next channel.**
  - does not work on the last channel.
- `13xx`: **change wave.**
  - only works when "Mode" is set to "Wavetable" in the instrument.

# info

this chip uses the [Generic Sample](../4-instrument/amiga.md) instrument editor.

- the maximum rate for sample playback is technically 31469Hz but anything higher than 28867Hz will sound glitchy on hardware.
- sample lengths and loop will be set to an even number.
- samples can't be longer than 131070.