# Amiga verification export format

"ROM" export format exclusively for verifying whether the Amiga emulation in Furnace is correct.

do not assume this is the actual ROM export! it is nothing more than a register dump kind of thing...

# process

enable the setting in Furnace to unlock the export option by adding this to furnace.cfg:

```
iCannotWait=1
```

go to file > export Amiga validation data...

put sample.bin, seq.bin and wave.bin in this directory.

type `make`. you need vasm (68000 with Mot syntax) in order for it to work.
alternatively, type:

```
vasmm68k_mot -Fhunkexe -kick1hunks -nosym -o player player.s
```

run `player` on Amiga. it should play the exported song.

# sequence format

## 00-0F: per-channel (00, 10, 20, 30)

- 00 xxxxxx yyyy: set loc/len
  - x: loc
  - y: len
- 01 xxxxxx yyyy: initialize wavetable (xxxx: pos; yy: length)
- 06 xxxx: set period
- 08 xx: set volume
- 0a xxxx: set data

## F0-FF: global

- f0: do nothing
- f1: next tick
- f2 xx: wait
- f3 xxxx: wait
- f6 xxxx: write to DMACON
- fe xxxx: write to ADKCON
- ff: end of song
