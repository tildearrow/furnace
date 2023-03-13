# Amiga verification export format

"ROM" export format exclusively for verifying whether the Amiga emulation in Furnace is correct.

do not assume this is the actual ROM export! it is nothing more than a register dump kind of thing...

# process

enable the setting in Furnace to unlock the export option by adding this to furnace.cfg:

```
iCannotWait=true
```

go to file > export Amiga validation data...

put sample.bin and seq.bin in this directory.

compile with vasm:

```
vasmm68k_mot -Fhunkexe -kick1hunks player.s
```

run a.out on Amiga. it should play the exported song.

# sequence format

## 00-0F: global

- 00: do nothing
- 01: next tick
- 02 xx: wait
- 03 xxxx: wait
- 06 xxxx: write to DMACON
- 0a xxxx: write to INTENA
- 0e xxxx: write to ADKCON

## 10-1F: per-channel (10, 20, 30, 40)

- 10 xxxxxx yyyy zzzzzz wwww: set loc/len
  - x: loc
  - y: len
  - z: loc after interrupt
  - w: len after interrupt
- 12 xxxx yy: initialize wavetable (xxxx: pos; yy: length)
- 16 xxxx: set period
- 18 xx: set volume
- 1a xxxx: set data
