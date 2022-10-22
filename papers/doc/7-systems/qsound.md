# Capcom QSound (DL-1425)

this chip was used in Capcom's CP System Dash, CP System II and ZN arcade PCBs.

it supports 16 PCM channels and uses the patented (now expired) QSound stereo expansion algorithm, as the name implies.

because the chip lacks sample interpolation, it is recommended that you try to play samples at around 24038 Hz to avoid aliasing. this is especially important for e.g. cymbals.

the QSound chip also has a small echo buffer, somewhat similar to the SNES, although with a very basic (and non-adjustable) filter. it is however possible to adjust the feedback and length of the echo buffer (the initial values can be set in the "configure chip" option in the file menu or the chip manager).

there are also 3 ADPCM channels, however they cannot be used in Furnace yet. they have been reserved in case this feature is added later. ADPCM samples are limited to 8012 Hz.

# effects

- `10xx`: set echo feedback level.
  - this effect will apply to all channels.
- `11xx`: set echo level.
- `12xx`: toggle QSound algorithm (on by default).
- `3xxx`: set the length of the echo delay buffer.
