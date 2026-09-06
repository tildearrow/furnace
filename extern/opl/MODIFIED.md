# modification disclaimer

baseline: [Nuked-OPL3-fast at f44bacb](https://github.com/tgies/Nuked-OPL3-fast/tree/f44bacb),
a bit-exact performance-optimized fork of Nuked-OPL3 1.8 (fork version
1.8-fast.3).

Furnace-specific changes on top:

- `muted` field on `opl3_channel`. see
  [issue #414](https://github.com/tildearrow/furnace/issues/414).
- `OPL3_Resample(chip, samplerate)` to change output rate without resetting
  chip state.
- `OPL_COMPAT_OLD_EG=1` compile definition to preserve the envelope behavior
  of Furnace's previously vendored core. Furnace's 2023 quad-output update
  already included immediate 4-op routing, so `OPL_COMPAT_DEFERRED_4OP_ALG`
  remains disabled.
