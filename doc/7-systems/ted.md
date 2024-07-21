# MOS Technology TED

also called 7360/8360, TED stands for Text Editing Device. it's both a video and audio chip of Commodore budget computers, like Plus/4 and 16. 

its audio portion is pretty barren - only 2 channels. one can output square wave and other may be either square or noise.
pitch range is limited as well, akin to that of SN76489, and volume control is global.

## effects

none so far.

## info

this chip uses the [TED](../4-instrument/ted.md) instrument editor.

## chip config

the following options are available in the Chip Manager window:

- **Clock rate**: sets the rate at which the chip will run.
- **Global parameter priority**: change the priority of macros which control global parameters, such as volume.
  - Left to right: process channels from 1 to 3. the last one to run macros will take effect.
  - Last used channel: process channels from oldest to newest (since last note). the one which had the latest note on will take a effect.
