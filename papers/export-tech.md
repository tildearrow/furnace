# ROM export technical details

## instrument data

TODO

## macro data

read length, loop and then release (1 byte).
if it is a 2-byte macro, read a dummy byte.

then read data.

## binary command stream

Furnace Command Stream, split version.

```
size | description
-----|------------------------------------
  4  | "FCS\0" format magic
  4  | channel count
 4?? | pointers to channel data
 1?? | preset delays
     | - 16 values
 1?? | speed dial commands
     | - 16 values
 ??? | channel data
```

program follows.

### commands/instructions

```
hex | description
----|------------------------------------
 00 | note on: C-(-5)
 01 | note on: C#(-5)
 02 | note on: D-(-5)
 .. | ...
 b1 | note on: A-9
 b2 | note on: A#9
 b3 | note on: B-9
 b4 | note on: null
----|------------------------------------
 b5 | note off
 b6 | note off env
 b7 | env release
 b8 | instrument // (ins, force)
 c0 | pre porta // (inporta, isportaorslide)
 c2 | vibrato // (speed, depth)
 c3 | vibrato range // (range)
 c4 | vibrato shape // (shape)
 c5 | pitch // (pitch)
 c6 | arpeggio // (note1, note2)
 c7 | volume // (vol)
 c8 | vol slide // (amount)
 c9 | porta // (target, speed)
 ca | legato // (note)
 cb | volume slide with target // (amount, target)
 cc | tremolo // (speed/depth)
 cd | panbrello // (speed/depth)
 ce | pan slide // (speed)
 cf | panning // (left, right)
----|------------------------------------
 d0 | speed dial command 0
 d1 | speed dial command 1
 .. | ...
 df | speed dial command 15
----|------------------------------------
 e0 | preset delay 0
 e1 | preset delay 1
 .. | ...
 ef | preset delay 15
----|------------------------------------
 f0 | UNUSED - placeholder used during optimization passes (3-byte nonce follows)
 f1 | no operation
 f2 | UNUSED
 f3 | loop (negative offset and count follow... both are 8-bit)
 f4 | UNUSED - call symbol (32-bit index follows; only used internally)
 f5 | call sub-block (32-bit address follows)
 f6 | UNUSED
 f7 | full command (command and data follows)
 f8 | call sub-block (16-bit address follows)
 f9 | return from sub-block
 fa | jump (address follows)
 fb | set tick rate (4 bytes)
 fc | wait (16-bit)
 fd | wait (8-bit)
 fe | wait one tick
 ff | stop
```

## full commands

```

```