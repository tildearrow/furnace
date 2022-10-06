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

read command and values (if any).
the list of commands follows.

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
 be | panning // (left, right)
 c0 | pre porta // (inporta, isportaorslide)
 c2 | vibrato // (speed, depth)
 c3 | vibrato range // (range)
 c4 | vibrato shape // (shape)
 c5 | pitch // (pitch)
 c6 | arpeggio // (note1, note2)
 c7 | volume // (vol)
 c8 | vol slide // (amount, onetick)
 c9 | porta // (target, speed)
 ca | legato // (note)
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
 f7 | full command (command and data follows)
 f8 | go to sub-block (offset follows)
 f9 | return from sub-block
 fa | jump (offset follows)
 fb | set tick rate (4 bytes)
 fc | wait (16-bit)
 fd | wait (8-bit)
 fe | wait one tick
 ff | stop
```

