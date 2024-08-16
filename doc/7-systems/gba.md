# Game Boy Advance

a portable video game console from Nintendo, succeeding the Game Boy.

it adds two stereo sample audio channels which can be used directly ("DMA", left/right) or used in a software mixing driver (most games do this) in order to have multiple voices.

# effects

- `10xx`: **change wave.**

## Game Boy Advance (MinMod)

this is the software mixing driver available in Furnace. it is written by Natt Akuma.
it features echo and up to 16 voices.

- `10xx`: **change wave.**
- `11xy`: **configure echo.**
  - this effect is kinda odd. this is how it works:

> How do you echo on GBA
>
> Create an empty instrment and put a very high note of it in channel 1 then do 110x in effect column and set volume column to set feedback and do nothing else on it

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
