# MOS 8364 R4 "Paula"

custom-made for the commodore amiga, a computer with a desktop OS, lifelike graphics and 4 channels of PCM sound... in 1985!

it's for this chip that music trackers were born...

imported MOD files use this chip, and will set A-4 tuning to 436.

this chip accepts [Generic Sample](../4-instrument/sample.md) instruments.


# effects

- `10xx`: **toggle low-pass filter.** `0` turns it off and `1` turns it on.
- `11xx`: **toggle amplitude modulation with the next channel.**
  - does not work on the last channel.
- `12xx`: **toggle period (frequency) modulation with the next channel.**
  - does not work on the last channel.
- `13xx`: **change wave.**
  - only works when "Mode" is set to "Wavetable" in the instrument.
