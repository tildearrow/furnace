# modification disclaimer

baseline: [Nuked-OPL3-fast](https://github.com/tgies/Nuked-OPL3-fast),
a bit-exact performance-optimized fork of Nuked-OPL3 1.8.

Furnace-specific changes on top:

- `muted` field on `opl3_channel`. see
  [issue #414](https://github.com/tildearrow/furnace/issues/414).
- `OPL3_Resample(chip, samplerate)` to change output rate without
  resetting chip state.
