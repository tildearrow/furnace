# SegaPCM

16 channels of PCM? no way!

yep, that's right! 16 channels of PCM!

a chip used in the Sega OutRun/X/Y arcade boards. eventually the MultiPCM surpassed it with 28 channels, and later they joined the software mixing gang.

## 5-channel SegaPCM

Furnace previously had a five channel version of this chip, but it only existed for DefleMask compatibility reasons. when opening older Furnace files and all DefleMask files, the channel count can be corrected by opening the chip config window and clicking the "click here to fix it." button beneath "irregular channel count detected!"

## effects

- `20xx`: **set PCM frequency.**
  - `xx` is a 256th fraction of 31250Hz.
  - this effect exists mostly for DefleMask compatibility; it is otherwise recommended to use Sample type instruments.

## info

this chip uses the [SegaPCM](../4-instrument/segapcm.md) instrument editor.

maximum sample length is 65280 samples.
