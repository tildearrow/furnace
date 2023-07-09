# Game Boy

the Nintendo Game Boy is one of the most successful portable game systems ever made.

with stereo sound, two pulse channels, a wave channel and a noise channel, it packed some serious punch.

# effects

- `10xx`: **change wave.**
- `11xx`: **set noise length.**
  - `0`: long
  - `1`: short
- `12xx`: **set duty cycle.**
  - `0`: 12.5%
  - `1`: 25%
  - `2`: 50%
  - `3`: 75%
- `13xy`: **setup sweep.** pulse channels only.
  - `x` is the time.
  - `y` is the shift.
  - set to `0` to disable it.
- `14xx`: **set sweep direction.** `0` is up and `1` is down.

# links

- [Gameboy sound hardware](https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware) - detailed technical information

- [GameBoy Sound Table](http://www.devrs.com/gb/files/sndtab.html) - note frequency table