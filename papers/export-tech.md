# ROM export technical details

## instrument data

TODO

## macro data

read length, loop and then release (1 byte).
if it is a 2-byte macro, read a dummy byte.

then read data.

## binary command stream

this is the Furnace Command Stream specification.
all numbers are little-endian unless the "big-endian mode" flag is set.

```
size | description
-----|------------------------------------
  4  | "FCS\0" format magic
  2  | channel count
  1  | flags
     | - bit 1: big-endian mode
     | - bit 0: pointer size (off: short; on: long)
  1  | reserved
 1?? | preset delays
     | - 16 values
 1?? | speed dial commands
     | - 16 values
 ??? | pointers to channel data
     | - pointers are short (2-byte) or long (4-byte), set in flags
 1?? | maximum stack size per channel
     | - length: channel count
 ??? | channel data
```

program follows.

### commands/instructions

command parameters (including their types) are indicated within parentheses. for example, (cs) means "read a char (8-bit) and a short (16-bit)".

the types are:

- `c`: signed char
- `s`: signed short (16-bit)
- `i`: signed int (32-bit)
- `b`: unsigned char (byte)
- `S`: unsigned short
- `U`: unsigned int
- `X`: custom (described under the command name)


the command list follows.

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
 b8 | instrument (b)
 c0 | pre porta (X)
    | - bit 7: inPorta
    | - bit 6: isPortaOrSlide
 c1 | arpeggio speed (b)
 c2 | vibrato (X)
    | - bit 4-7: speed
    | - bit 0-3: depth
 c3 | vibrato range (b)
 c4 | vibrato shape (b)
 c5 | pitch (c)
 c6 | arpeggio (X)
    | - bit 4-7: note 1
    | - bit 0-3: note 2
 c7 | volume (b)
 c8 | vol slide (s)
 c9 | porta (bb) // target, speed
 ca | legato (b) // note... $ff is null note
 cb | volume slide with target (sS) // amount, target
 cc | tremolo (X)
    | - bit 4-7: speed
    | - bit 0-3: depth
 cd | panbrello (X)
    | - bit 4-7: speed
    | - bit 0-3: depth
 ce | pan slide (c)
 cf | panning (bb) // left, right
----|------------------------------------
 d0 | UNUSED - placeholder used during optimization passes (3-byte nonce follows)
 d1 | no operation
 d2 | UNUSED
 d3 | UNUSED
 d4 | UNUSED - call symbol (32-bit index follows; only used internally)
 d5 | call sub-block (32-bit address follows)
 d6 | note off + wait one tick
 d7 | full command (command and data follows)
 d8 | call sub-block (16-bit address follows)
 d9 | return from sub-block
 da | jump (address follows)
 db | set tick rate (4 bytes)
 dc | wait (16-bit)
 dd | wait (8-bit)
 de | wait one tick
 df | stop
----|------------------------------------
 e0 | speed dial command 0
 e1 | speed dial command 1
 .. | ...
 ef | speed dial command 15
----|------------------------------------
 f0 | preset delay 0
 f1 | preset delay 1
 .. | ...
 ff | preset delay 15
```

## full commands

```
hex | description
----|------------------------------------
 1c | sample mode (b)
 1d | sample freq (b)
 1e | legacy sample bank (b)
 1f | sample position (U)
 20 | sample direction (b)
----|------------------------------------
    | **FM commands**
 21 | hard reset (b)
 22 | LFO speed (b)
 23 | LFO waveform (b)
 24 | TL (bb)
 25 | AM (bb)
 26 | AR (bb)
 27 | DR (bb)
 28 | SL (bb)
 29 | D2R (bb)
 2a | RR (bb)
 2b | DT (bb)
 2c | DT2 (bb)
 2d | RS (bb)
 2e | KSR (bb)
 2f | VIB (bb)
 30 | SUS (bb)
 31 | WS (bb)
 32 | SSG-EG (bb)
 33 | REV (bb)
 34 | EG-Shift (bb)
 35 | FB (b)
 36 | MULT (bb)
 37 | FINE (bb)
 38 | fixed frequency (XX)
    | - byte 0: frequency low
    | - byte 1:
    |   - bit 4-5: operator
    |   - bit 0-2: frequency high
 39 | ExtCh (b)
 3a | AM depth (b)
 3b | PM depth (b)
 3c | LFO2 speed (b)
 3d | LFO2 wave (b)
----|------------------------------------
    | **PSG commands**
 3e | noise freq (b)
 3f | noise mode/duty/whatever (b)
 40 | waveform (b)
----|------------------------------------
    | **Game Boy commands**
 41 | sweep time (b)
 42 | sweep direction (b)
----|------------------------------------
    | **PC Engine commands**
 43 | LFO mode (b)
 44 | LFO speed (b)
----|------------------------------------
    | **NES commands**
 45 | sweep (X)
    | - bit 3: on
    | - rest of bits: sweep time/amount
 46 | set DMC (b)
----|------------------------------------
    | **C64 commands**
 47 | coarse cutoff (b)
 48 | resonance (b)
 49 | filter mode (b)
 4a | reset time (b)
 4b | reset mask (b)
 4c | filter reset (b)
 4d | duty reset (b)
 4e | extended (b)
 4f | duty (S)
 50 | cutoff (S)
----|------------------------------------
    | **AY commands**
 51 | set envelope (b)
 52 | envelope freq low (b)
 53 | envelope freq high (b)
 54 | envelope slide (b)
 55 | noise AND mask (b)
 56 | noise OR mask (b)
 57 | auto envelope (b)
 58 | I/O port write (bb)
 59 | AutoPWM (bb)
----|------------------------------------
    | **FDS commands**
 5a | mod depth (b)
 5b | mod speed high (b)
 5c | mod speed low (b)
 5d | mod position (b)
 5e | mod waveform (b)
----|------------------------------------
    | **SAA1099 commands**
 5f | envelope (b)
----|------------------------------------
    | **Amiga commands**
 60 | toggle filter (b)
 61 | AM (b)
 62 | period modulation (b)
----|------------------------------------
    | **Lynx commands**
 63 | load LFSR (S)
----|------------------------------------
    | **QSound commands**
 64 | echo feedback (b)
 65 | echo delay (S)
 66 | echo level (b)
 67 | surround (b)
----|------------------------------------
    | **X1-010 commands**
 68 | envelope shape (b)
 69 | UNUSED - envelope enable (b)
 6a | envelope mode (b)
 6b | envelope period (b)
 6c | envelope slide (b)
 6d | auto envelope (b)
 6e | sample bank slot (b)
----|------------------------------------
    | **WonderSwan commands**
 6f | sweep time (b)
 70 | sweep amount (b)
----|------------------------------------
    | **Namco 163 commands**
 71 | wave position (b)
 72 | wave length (b)
 73 | UNUSED (b)
 74 | UNUSED (b)
 75 | wave load position (b)
 76 | wave load length (b)
 77 | UNUSED (b)
 78 | channel limit (b)
 79 | global wave load (b)
 7a | global wave load position (b)
 7b | UNUSED (b)
 7c | UNUSED (b)
----|------------------------------------
    | **Sound Unit commands**
 7d | sweep period low (bb)
 7e | sweep period high (bb)
 7f | sweep bound (bb)
 80 | sweep enable (bb)
 81 | sync period low (b)
 82 | sync period high (b)
----|------------------------------------
 83 | ADPCM-A volume (b)
----|------------------------------------
    | **SNES commands**
 84 | echo (b)
 85 | pitch mod (b)
 86 | invert (b)
 87 | gain mode (b)
 88 | gain (b)
 89 | echo enable (b)
 8a | echo delay (b)
 8b | echo vol left (c)
 8c | echo vol right (c)
 8d | echo feedback (c)
 8e | echo filter (bc)
----|------------------------------------
    | **NES commands (continued)**
 8f | envelope mode (b)
 90 | length counter (b)
 91 | frame counter rate/count mode (b)
----|------------------------------------
    | **macro control**
 92 | macro off (b)
 93 | macro on (b)
----|------------------------------------
 94 | surround panning (bb)
----|------------------------------------
    | **FM commands (continued)**
 95 | AM depth 2 (b)
 96 | PM depth 2 (b)
----|------------------------------------
    | **ES5506 commands**
 97 | filter mode (b)
 98 | filter K1 (SS)
 99 | filter K2 (SS)
 9a | filter K1 slide (bb)
 9b | filter K2 slide (bb)
 9c | envelope count (S)
 9d | envelope left vol ramp (b)
 9e | envelope right vol ramp (b)
 9f | envelope K1 ramp (bb)
 a0 | envelope K2 ramp (bb)
 a1 | pause (b)
----|------------------------------------
    | **SNES commands (continued)**
 a3 | global vol left (c)
 a4 | global vol right (c)
----|------------------------------------
 a5 | NES linear counter length (b)
----|------------------------------------
 a6 | external command (b)
----|------------------------------------
    | **C64 commands (continued)**
 a7 | attack/decay (X)
    | - bit 4-7: attack
    | - bit 0-3: decay
 a8 | sustain/release (X)
    | - bit 4-7: sustain
    | - bit 0-3: release
----|------------------------------------
    | **ESFM commands**
 a9 | operator panning (bb)
 aa | output level (bb)
 ab | modulation input (bb)
 ac | envelope delay (bb)
----|------------------------------------
 ad | restart macro (b)
----|------------------------------------
    | **PowerNoise commands**
 ae | load counter (bb)
 af | I/O write (bb)
----|------------------------------------
    | **Dave commands**
 b0 | high pass (b)
 b1 | ring mod (b)
 b2 | swap counters (b)
 b3 | low pass (b)
 b4 | clock divider (b)
----|------------------------------------
 b5 | MinMod echo setup (b)
----|------------------------------------
    | **Bifurcator commands**
 b6 | state load (bb)
 b7 | set parameter (bb)
----|------------------------------------
 b8 | FDS auto mod (b)
----|------------------------------------
 b9 | FM operator mask (b)
----|------------------------------------
    | **MultiPCM commands**
 ba | mix FM (b)
 bb | mix PCM (b)
 bc | LFO (b)
 bd | VIB (b)
 be | AM (b)
 bf | AR (b)
 c0 | D1R (b)
 c1 | DL (b)
 c2 | D2R (b)
 c3 | RC (b)
 c4 | RR (b)
 c5 | damp (b)
 c6 | pseudo-reverb (b)
 c7 | LFO reset (b)
 c8 | level direct (b)
----|------------------------------------
    | **SID3 commands**
 c9 | special wave (b)
 ca | ring mod source (b)
 cb | hard sync source (b)
 cc | phase mod source (b)
 cd | wave mix (b)
 ce | LFSR feedback bits (bb)
 cf | 1-bit noise (b)
 d0 | filter distortion (bb)
 d1 | filter output volume (bb)
 d2 | channel invert (b)
 d3 | filter connection (b)
 d4 | filter matrix (b)
 d5 | filter enable (b)
----|------------------------------------
    | **slide commands**
 d6 | pulse width slide (bc)
 d7 | cutoff slide (bc)
----|------------------------------------
    | **SID3 commands (continued)**
 d8 | phase reset (b)
 d9 | noise phase reset (b)
 da | envelope reset (b)
 db | cutoff scaling (b)
 dc | resonance scaling (b)
----|------------------------------------
 dd | WonderSwan speaker volume (b)
```
