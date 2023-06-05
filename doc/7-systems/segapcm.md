# SegaPCM

16 channels of PCM? no way!

yep, that's right! 16 channels of PCM!

a chip used in the Sega OutRun/X/Y arcade boards. eventually the MultiPCM surpassed it with 28 channels, and later they joined the software mixing gang.

# 5-channel SegaPCM

Furnace also has a five channel version of this chip, but it only exists for DefleMask compatibility reasons (which doesn't expose the other channels for rather arbitrary reasons).

# effects

- `20xx`: set PCM frequency.
  - `xx` is a 256th fraction of 31250Hz.
  - this effect exists for mostly DefleMask compatibility - it is otherwise recommended to use Sample type instruments.
