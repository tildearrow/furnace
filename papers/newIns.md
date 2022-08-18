# possible new Furnace instrument format

the main issue with Furnace instrument files is that they are too big, even if the instrument is nothing more than the FM setup...

the aim of this new format is to greatly reduce the size of a resulting instrument.

```
size | description
-----|------------------------------------
  6  | "FURINS" format magic
  2  | format version
  1  | instrument type
 ??? | feature bits
  4  | instrument length (if wave/sample bits are on)
```

the "feature bits" field is a variable length bitfield. bit 7 in a byte indicates "read one more byte".

the feature bits are:

- 0: has wavetables
- 1: has samples
- 2: has name
- 3: FM data
- 4: FM data size (1: 2-op, 0: 4-op)
- 5: FM data includes OPL/OPZ data
  - if off, only read an op until ssgEnv.
  - if on, read everything else.
- 6: Game Boy data
- 7: (continue in next byte)
- 8: C64 data
- 9: Amiga data
- 10: standard data (macros)
- 11: operator macros
- 12: release points
- 13: op release points
- 14: extended op macros
- 15: (continue in next byte)
- 16: OPL drums mode data
- 17: Amiga sample map data
- 18: Namco 163 data
- 19: extra macros
- 20: FDS data
- 21: OPZ data
- 22: wavetable synth data
- 23: (continue in next byte)
- 24: additional macro modes
- 25: extra C64 data
- 26: MultiPCM data
