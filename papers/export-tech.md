# ROM export technical details

## instrument data

### C64

```
size | description
-----|----------------------------------
  1  | waveform/flags
     | - bit 7: noise
     | - bit 6: pulse
     | - bit 5: saw
     | - bit 4: triangle
     | - bit 3: test
     | - bit 2: sync
     | - bit 1: ring
     | - bit 0: gate (always on)
  1  | filter flags
     | - bit 7: don't test before new note
     | - bit 6: enable filter
     | - bit 5: init filter
     | - bit 3: ch3off
     | - bit 2: high pass
     | - bit 1: band pass
     | - bit 0: low pass
  1  | other flags
     | - bit 7: reset duty on new note
     | - bit 6: absolute cutoff macro
     | - bit 5: absolute duty macro
-----|----------------------------------
     | **macro pointers**
  2  | vol
  2  | arp
  2  | duty
  2  | wave
  2  | pitch
  2  | cutoff
  2  | resonance
  2  | filter mode
  2  | filter toggle
  2  | special
  2  | attack
  2  | decay
  2  | sustain
  2  | release
```

### SNES

```
size | description
-----|----------------------------------
  1  | ADSR/gain
  1  | decay 2/release mode
     | - bit 3-7: decay 2
     | - bit 0-2: mode
     |   - 0: direct
     |   - 1: effective linear
     |   - 2: effective exp
     |   - 3: delayed
-----|----------------------------------
     | **sample data**
  1  | type
     | - 0: sample
     | - 1: sample map
     | - 2: wavetable
  2  | value
     | - sample: sample index
     | - sample map: pointer to sample map
     | - wavetable: wave width
-----|----------------------------------
     | **macro pointers**
  2  | vol
  2  | arp
  2  | noise freq
  2  | wave
  2  | pan left
  2  | pan right
  2  | pitch
  2  | special
-----|----------------------------------
     | **wave synth data (only in wavetable mode)**
  1  | enable synth
     | - bit 6: global
     | - bit 0: enable
  1  | synth effect
  2  | wave 1
  2  | wave 2
  1  | update rate
  1  | speed
  1  | amount
  1  | power
```

### Game Boy

```
size | description
-----|----------------------------------
  1  | volume
  1  | length/dir
  1  | sound length
  1  | flags
     | - bit 7: software env
     | - bit 6: init env on every note
     | - bit 5: double wave length
-----|----------------------------------
     | **macro pointers**
  2  | vol
  2  | arp
  2  | duty/noise
  2  | wave
  2  | pan
  2  | pitch
  2  | phase reset
-----|----------------------------------
     | **wave synth data**
  1  | enable synth
     | - bit 6: global
     | - bit 0: enable
  1  | synth effect
  2  | wave 1
  2  | wave 2
  1  | update rate
  1  | speed
  1  | amount
  1  | power
-----|----------------------------------
     | **hardware sequence**
 ??? | data...
```

## compiled macro data

```
size | description
-----|-------------------------------------------------
  1  | flags/macro data type
     | - bit 6: release mode
     |   - active when enabled; passive otherwise
     | - bit 0-5: macro data type
     |   - 0: 8-bit unsigned
     |   - 1: 8-bit signed
     |   - 2: 16-bit unsigned
     |   - 3: 16-bit signed
     |   - 4: arp macro
     |   - 5: 4-bit unsigned (top first, bottom second)
     |   - 6: ADSR macro (16-bit)
     |   - 7: ADSR macro (8-bit)
     |   - 8: LFO macro (16-bit)
     |   - 9: LFO macro (8-bit)
     |   - 10: ADSR macro (24-bit)
     |   - 11: LFO macro (24-bit)
     |     - these two are there just in case. you do not have to implement them.
  1  | step length
  1  | delay
 ??? | macro data...
```

interpret macro data as follows.

for 4/8/16-bit macros and arp macros:

```
size | description
-----|----------------------
  1  | length
  1  | loop point
  1  | release point
 ??? | values...
```

arp macros are special:
- read one byte. this will be the next (signed) value, unless it is $7F or $80.
- if it is $80, fixed mode is on for this value.
- if it is $7F, the value is 16-bit. read two bytes and treat that as the (signed) value.

for 16-bit ADSR macros:

```
size | description
-----|---------------------
  2  | minimum value
  2  | maximum value
  2  | sustain level
  1  | hold time
  1  | sustain time
  3  | attack rate
  3  | decay rate
  3  | sustain decay
  3  | release rate
```

for 8-bit ADSR macros:

```
size | description
-----|---------------------
  1  | minimum value
  1  | maximum value
  1  | sustain level
  1  | hold time
  1  | sustain time
  2  | attack rate
  2  | decay rate
  2  | sustain decay
  2  | release rate
```

for 16-bit LFO macros:

```
size | description
-----|---------------------
  2  | minimum value
  2  | maximum value
  3  | initial accumulator value
  3  | speed
  1  | flags
     | - bit 2: initial direction (set when down)
     | - bit 0-1: shape
     |   - 0: triangle
     |   - 1: saw (down to up)
     |   - 2: pulse
     |   - 3: saw (up to down)
```

for 8-bit LFO macros:

```
size | description
-----|---------------------
  1  | minimum value
  1  | maximum value
  2  | initial accumulator value
  2  | speed
  1  | flags
     | - bit 7: initial direction (set when down)
     | - bit 0-1: shape
     |   - 0: triangle
     |   - 1: saw (down to up)
     |   - 2: pulse
     |   - 3: saw (up to down)
```

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
 1?? | preset instruments
     | - 6 values
 1?? | preset volumes
     | - 6 values
 1?? | speed dial commands
     | - 4 values
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
 e0 | preset instrument 0
 e1 | preset instrument 1
 e2 | preset instrument 2
 e3 | preset instrument 3
 e4 | preset instrument 4
 e5 | preset instrument 5
 e6 | preset volume 0
 e7 | preset volume 1
 e8 | preset volume 2
 e9 | preset volume 3
 ea | preset volume 4
 eb | preset volume 5
 ec | speed dial command 0
 ed | speed dial command 1
 ee | speed dial command 2
 ef | speed dial command 3
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
 71 | wave position (bb)
 72 | wave length (bb)
 73 | UNUSED (b)
 74 | UNUSED (b)
 75 | UNUSED (b)
 76 | UNUSED (b)
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
----|------------------------------------
    | **FM commands (continued)**
 de | ALG (b)
 df | FMS (b)
 e0 | AMS (b)
 e1 | FMS2 (b)
 e2 | AMS2 (b)
```
