# Game Boy Advance

a portable video game console from Nintendo, succeeding the Game Boy.

it starts with the [Game Boy sound hardware](game-boy.md) and adds two stereo sample audio channels which can be used directly ("DMA", hard-panned, usually used as left/right channels) or used in a software mixing driver (most games do this) in order to have multiple voices.

# effects

- `10xx`: **change wave.**

## Game Boy Advance (MinMod)

this is the software mixing driver available in Furnace. it is written by Natt Akuma.
it features echo and up to 16 voices.

- `10xx`: **change wave.**
- `11xy`: **configure echo.**
  - this effect is kinda odd. here's how to use it:
    - create an empty instrument and put a very high note of it in channel 1.
    - put `110x` in the effect column.
    - set volume column to set feedback.
    - don't use the channel for anything else.
- `12xy`: **toggle invert.**
  - `x` left channel.
  - `y` right channel.

## info

this chip uses the [GBA DMA](../4-instrument/gbadma.md) and [GBA MinMod](../4-instrument/gbaminmod.md) instrument editors.

## chip config

the following options are available in the Chip Manager window:

- **DAC bit depth**: sets the bit depth.

these options are available when using MinMod:

- **Volume scale**: scale volumes to prevent clipping/distortion.
- **Mix buffers**: sets how many mix buffers will be stored in memory. higher values result in longer echo.
- **Channel limit**: sets the number of channels that will be available. higher values use more CPU.
- **Sample rate**: sets the mixing rate. higher values use more CPU.
