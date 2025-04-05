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
hex | description
----|------------------------------------
 1c | sample mode
 1d | sample freq
 1e | legacy sample bank
 1f | sample position
 20 | sample direction
----|------------------------------------
    | **FM commands**
 21 | hard reset
 22 | LFO speed
 23 | LFO waveform
 24 | TL
 25 | AM
 26 | AR
 27 | DR
 28 | SL
 29 | D2R
 2a | RR
 2b | DT
 2c | DT2
 2d | RS
 2e | KSR
 2f | VIB
 30 | SUS
 31 | WS
 32 | SSG-EG
 33 | REV
 34 | EG-Shift
 35 | FB
 36 | MULT
 37 | FINE
 38 | fixed frequency
 39 | ExtCh
 3a | AM depth
 3b | PM depth
 3c | LFO2 speed
 3d | LFO2 wave
----|------------------------------------
    | **PSG commands**
 3e | noise freq
 3f | noise mode/duty/whatever
 40 | waveform
----|------------------------------------
    | **Game Boy commands**
 41 | sweep time
 42 | sweep direction
----|------------------------------------
    | **PC Engine commands**
 43 | LFO mode
 44 | LFO speed
----|------------------------------------
    | **NES commands**
 45 | sweep
 46 | set DMC
----|------------------------------------
    | **C64 commands**
 47 | coarse cutoff
 48 | resonance
 49 | filter mode
 4a | reset time
 4b | reset mask
 4c | filter reset
 4d | duty reset
 4e | extended
 4f | duty
 50 | cutoff
----|------------------------------------
    | **AY commands**
 51 | set envelope
 52 | envelope freq low
 53 | envelope freq high
 54 | envelope slide
 55 | noise AND mask
 56 | noise OR mask
 57 | auto envelope
 58 | I/O port write
 59 | AutoPWM
----|------------------------------------
    | **FDS commands**
 5a | mod depth
 5b | mod speed high
 5c | mod speed low
 5d | mod position
 5e | mod waveform
----|------------------------------------
    | **SAA1099 commands**
 5f | envelope
----|------------------------------------
    | **Amiga commands**
 60 | toggle filter
 61 | AM
 62 | period modulation
----|------------------------------------
    | **Lynx commands**
 63 | load LFSR
----|------------------------------------
    | **QSound commands**
 64 | echo feedback
 65 | echo delay
 66 | echo level
 67 | surround
----|------------------------------------
    | **X1-010 commands**
 68 | envelope shape
 69 | envelope enable
 6a | envelope mode
 6b | envelope period
 6c | envelope slide
 6d | auto envelope
 6e | sample bank slot
----|------------------------------------
    | **WonderSwan commands**
 6f | sweep time
 70 | sweep amount
```