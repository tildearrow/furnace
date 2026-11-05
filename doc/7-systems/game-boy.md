# Game Boy

the Game Boy is one of the most successful portable game systems ever made.

it has stereo sound, two pulse channels, a wave channel and a noise channel.

## effects

- `10xx`: **change wave.**
- `11xx`: **set noise length.**
  - `0`: long
  - `1`: short
- `12xx`: **set duty cycle.**
  - `0`: 12.5%
  - `1`: 25%
  - `2`: 50%
  - `3`: 75%
- `13xy`: **setup sweep.** pulse 1 only.
  - `x` is the time.
  - `y` is the shift.
  - set to `0` to disable it.
- `14xx`: **set sweep direction.** `0` is up and `1` is down.

## info

this chip uses the [Game Boy](../4-instrument/game-boy.md) instrument editor.

### "zombie mode"

the game boy's envelope hardware has a bug that can be triggered to cause the volume to step up or down only on software command. Furnace uses the same method to trigger it as [LSDJ](https://www.littlesounddj.com/). without this technique, changing the volume directly on a note (such as volume macros do) would cause a phase reset each time, causing significant popping sounds. unfortunately, only a few emulators emulate the "zombie mode" bug properly.

## chip config

the following options are available in the Chip Manager window:

- **Disable anti-click**: waveform switching requires a phase reset, which may cause clicks. Furnace uses a wave-position predicting algorithm to minimize these clicks. enable this option to disable it.
- **Chip revision**: sets the chip model to use. most of these lack audible difference, but Game Boy Advance fixed the wave channel's inversion.
- **Wave channel orientation**: allows you to set how is wave data written.
  - in Game Boy:
    - Exact data: wave data is written as-is. it will appear inverted in the output.
    - Exact output: wave data is inverted so it appears correctly in the output.
  - in Game Boy Advance:
    - Normal: wave data is written as-is.
    - Inverted: guess!
- **Pretty please**: only for compatibility with Synchronize.dmf. do not use.

## links

- [Gameboy sound hardware](https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware) - detailed technical information

- [GameBoy Sound Table](http://www.devrs.com/gb/files/sndtab.html) - note frequency table
