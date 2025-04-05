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
----|------------------------------------
    | **Namco 163 commands**
 71 | wave position
 72 | wave length
 73 | UNUSED
 74 | UNUSED
 75 | wave load position
 76 | wave load length
 77 | UNUSED
 78 | channel limit
 79 | global wave load
 7a | global wave load position
 7b | UNUSED
 7c | UNUSED
----|------------------------------------
    | **Sound Unit commands**
 7d | sweep period low
 7e | sweep period high
 7f | sweep bound
 80 | sweep enable
 81 | sync period low
 82 | sync period high
----|------------------------------------
 83 | ADPCM-A volume
----|------------------------------------
    | **SNES commands**
 84 | echo
 85 | pitch mod
 86 | invert
 87 | gain mode
 88 | gain
 89 | echo enable
 8a | echo delay
 8b | echo vol left
 8c | echo vol right
 8d | echo feedback
 8e | echo filter
----|------------------------------------
    | **NES commands (continued)**
 8f | envelope mode
 90 | length counter
 91 | count mode (?)
----|------------------------------------
    | **macro control**
 92 | macro off
 93 | macro on
----|------------------------------------
 94 | surround panning
----|------------------------------------
    | **FM commands (continued)**
 95 | AM depth 2
 96 | PM depth 2
----|------------------------------------
    | **ES5506 commands**
 97 | filter mode
 98 | filter K1
 99 | filter K2
 9a | filter K1 slide
 9b | filter K2 slide
 9c | envelope count
 9d | envelope left vol ramp
 9e | envelope right vol ramp
 9f | envelope K1 ramp
 a0 | envelope K2 ramp
 a1 | pause
----|------------------------------------
 a2 | arpeggio speed
----|------------------------------------
    | **SNES commands (continued)**
 a3 | global vol left
 a4 | global vol right
----|------------------------------------
 a5 | NES linear counter length
----|------------------------------------
 a6 | external command
----|------------------------------------
    | **C64 commands (continued)**
 a7 | attack/decay
 a8 | sustain/release
----|------------------------------------
    | **ESFM commands**
 a9 | operator panning
 aa | output level
 ab | modulation input
 ac | envelope delay
----|------------------------------------
 ad | restart macro
----|------------------------------------
    | **PowerNoise commands**
 ae | load counter
 af | I/O write
----|------------------------------------
    | **Dave commands**
 b0 | high pass
 b1 | ring mod
 b2 | swap counters
 b3 | low pass
 b4 | clock divider
----|------------------------------------
 b5 | MinMod echo setup
----|------------------------------------
    | **Bifurcator commands**
 b6 | state load
 b7 | set parameter
----|------------------------------------
 b8 | FDS auto mod
----|------------------------------------
 b9 | FM operator mask
----|------------------------------------
    | **MultiPCM commands**
 ba | mix FM
 bb | mix PCM
 bc | LFO
 bd | VIB
 be | AM
 bf | AR
 c0 | D1R
 c1 | DL
 c2 | D2R
 c3 | RC
 c4 | RR
 c5 | damp
 c6 | pseudo-reverb
 c7 | LFO reset
 c8 | level direct
----|------------------------------------
    | **SID3 commands**
 c9 | special wave
 ca | ring mod source
 cb | hard sync source
 cc | phase mod source
 cd | wave mix
 ce | LFSR feedback bits
 cf | 1-bit noise
 d0 | filter distortion
 d1 | filter output volume
 d2 | channel invert
 d3 | filter connection
 d4 | filter matrix
 d5 | filter enable
----|------------------------------------
    | **slide commands**
 d6 | pulse width slide
 d7 | cutoff slide
----|------------------------------------
    | **SID3 commands (continued)**
 d8 | phase reset
 d9 | noise phase reset
 da | envelope reset
 db | cutoff scaling
 dc | resonance scaling
----|------------------------------------
 dd | WonderSwan speaker volume
```
