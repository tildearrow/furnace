# new Furnace instrument format

the main issue with Furnace instrument files is that they are too big, even if the instrument is nothing more than the FM setup...

the aim of this new format is to greatly reduce the size of a resulting instrument.

# information

this format is "featural", meaning that only used parameters are stored (depending on instrument types).
this is the biggest improvement over the previous format, which stored everything including unused parameters.

features which are not recognized by Furnace will be ignored.

instruments are not compressed using zlib, unlike Furnace songs.

all numbers are little-endian.

the following fields may be found in "size":
- `f` indicates a floating point number.
- `STR` is a UTF-8 zero-terminated string.
- `???` is an array of variable size.
- `S??` is an array of `STR`s.
- `1??` is an array of bytes.
- `2??` is an array of shorts.
- `4??` is an array of ints.

the format may change across versions. a `(>=VER)` indicates this field is only present starting from format version `VER`, and `(<VER)` indicates this field is present only before version `VER`.

furthermore, an `or reserved` indicates this field is always present, but is reserved when the version condition is not met.

the `size of this block` fields represent the size of a block excluding the ID and the aforementioned field.

# header

.fui files use the following header:

```
size | description
-----|------------------------------------
  4  | "FINS" format magic
  2  | format version
  2  | instrument type
 ??? | features...
```

instruments in a .fur file use the following header instead:

```
size | description
-----|------------------------------------
  4  | "INS2" block ID
  4  | size of this block
  2  | format version
  2  | instrument type
 ??? | features...
```

a feature uses the following format:

```
size | description
-----|------------------------------------
  2  | feature code
  2  | length of block
 ??? | data...
```

the following instrument types are available:

- 0: SN76489
- 1: FM (OPN)
- 2: Game Boy
- 3: C64
- 4: Amiga/sample
- 5: PC Engine
- 6: AY-3-8910
- 7: AY8930
- 8: TIA
- 9: SAA1099
- 10: VIC
- 11: PET
- 12: VRC6
- 13: OPLL
- 14: OPL
- 15: FDS
- 16: Virtual Boy
- 17: Namco 163
- 18: SCC
- 19: OPZ
- 20: POKEY
- 21: PC Speaker
- 22: WonderSwan
- 23: Lynx
- 24: VERA
- 25: X1-010
- 26: VRC6 (saw)
- 27: ES5506
- 28: MultiPCM
- 29: SNES
- 30: Sound Unit
- 31: Namco WSG
- 32: OPL (drums)
- 33: FM (OPM)
- 34: NES
- 35: MSM6258
- 36: MSM6295
- 37: ADPCM-A
- 38: ADPCM-B
- 39: SegaPCM
- 40: QSound
- 41: YMZ280B
- 42: RF5C68
- 43: MSM5232
- 44: T6W28
- 45: K007232
- 46: GA20
- 47: Pokémon Mini

the following feature codes are recognized:

- `NA`: instrument name
- `FM`: FM ins data
- `MA`: macro data
- `64`: C64 ins data
- `GB`: Game Boy ins data
- `SM`: sample ins data
- `O1`: operator 1 macros
- `O2`: operator 2 macros
- `O3`: operator 3 macros
- `O4`: operator 4 macros
- `LD`: OPL drums mode data
- `SN`: SNES ins data
- `N1`: Namco 163 ins data
- `FD`: FDS/Virtual Boy ins data
- `WS`: wavetable synth data
- `SL`: list of samples
- `WL`: list of wavetables
- `MP`: MultiPCM ins data
- `SU`: Sound Unit ins data
- `ES`: ES5506 ins data
- `X1`: X1-010 ins data
- `EN`: end of features
  - if you find this feature code, stop reading the instrument.
  - it will usually appear only when there sample/wave lists.
  - instruments in a .fur shall end with this feature code.

# instrument name (NA)

```
size | description
-----|------------------------------------
 STR | instrument name
```

# FM data (FM)

- FM operator order is:
  - 1/3/2/4 (internal order) for OPN, OPM, OPZ and OPL 4-op
  - 1/2/?/? (? = unused) for OPL 2-op and OPLL

```
size | description
-----|------------------------------------
  1  | flags
     | - bit 4-7: op enabled
     |   - op order from 4 to 7: 0, 2, 1, 3
     |   - 2-op instruments: 0, 1, x, x
     | - bit 0-3: op count
-----|------------------------------------
     | **base data**
     | /7 6 5 4 3 2 1 0|
  1  | |x| ALG |x| FB  |
  1  | |FMS2 |AMS| FMS |
  1  | |AM2|4| LLPatch |
-----|------------------------------------
     | **operator data × opCount**
     | /7 6 5 4 3 2 1 0|
  1  | |r| D T |  MULT |
     |  \- KSR
  1  | |s|     T L     |
     |  \- SUS
  1  | |R S|v|   A R   |
     |      \- VIB
  1  | |A|KSL|   D R   |
     |  \- AM
  1  | |e|KVS|   D2R   |
     |  \- EGT
  1  | |  S L  |  R R  |
  1  | |  DVB  |  SSG  |
  1  | | DAM |DT2| W S |
```

# macro data (MA)

notes:

- the macro range varies depending on the instrument type.
- "macro open" indicates whether the macro is collapsed or not in the instrument editor.
- meaning of extended macros varies depending on instrument type.
- meaning of panning macros varies depending on instrument type:
  - for hard-panned chips (e.g. FM and Game Boy): left panning is 2-bit panning macro (left/right)
  - otherwise both left and right panning macros are used

```
size | description
-----|------------------------------------
  2  | length of macro header
 ??? | data...
```

each macro is represented like this:

```
size | description
-----|------------------------------------
  1  | macro code
     | - 0: vol
     | - 1: arp
     | - 2: duty
     | - 3: wave
     | - 4: pitch
     | - 5: ex1
     | - 6: ex2
     | - 7: ex3
     | - 8: alg
     | - 9: fb
     | - 10: fms
     | - 11: ams
     | - 12: panL
     | - 13: panR
     | - 14: phaseReset
     | - 15: ex4
     | - 16: ex5
     | - 17: ex6
     | - 18: ex7
     | - 19: ex8
     | - 255: stop reading and move on
  1  | macro length
  1  | macro loop
  1  | macro release
  1  | macro mode
  1  | macro open/type/word size
     | - bit 6-7: word size
     |   - 0: 8-bit unsigned
     |   - 1: 8-bit signed
     |   - 2: 16-bit signed
     |   - 3: 32-bit signed
     | - bit 1-2: type
     |   - 0: normal
     |   - 1: ADSR
     |   - 2: LFO
     | - bit 0: open
  1  | macro delay
  1  | macro speed
 ??? | macro data
     | - length: macro length × word sizs
```

## interpreting macro mode values

- sequence (normal): I think this is obvious...
- ADSR:
  - `val[0]`: bottom
  - `val[1]`: top
  - `val[2]`: attack
  - `val[3]`: hold time
  - `val[4]`: decay
  - `val[5]`: sustain level
  - `val[6]`: sustain hold time
  - `val[7]`: decay 2
  - `val[8]`: release
- LFO:
  - `val[11]`: speed
  - `val[12]`: waveform
    - 0: triangle
    - 1: saw
    - 2: pulse
  - `val[13]`: phase
  - `val[14]`: loop
  - `val[15]`: global (not sure how will I implement this)

# C64 data (64)

```
size | description
-----|------------------------------------
  1  | flags 1
     | - bit 7: dutyIsAbs
     | - bit 6: initFilter
     | - bit 5: volIsCutoff
     | - bit 4: toFilter
     | - bit 3: noise on
     | - bit 2: pulse on
     | - bit 1: saw on
     | - bit 0: triangle on
  1  | flags 2
     | - bit 7: oscSync
     | - bit 6: ringMod
     | - bit 5: noTest
     | - bit 4: filterIsAbs
     | - bit 3: ch3off
     | - bit 2: band pass
     | - bit 1: high pass
     | - bit 0: low pass
  1  | attack/decay
     | - bit 4-7: attack
     | - bit 0-3: decay
  1  | sustain release
     | - bit 4-7: sustain
     | - bit 0-3: release
  2  | duty
  2  | cutoff/resonance
     | - bit 12-15: resonance
     | - bit 0-10: cutoff
```

# Game Boy data (GB)

```
size | description
-----|------------------------------------
  1  | envelope params
     | - bit 5-7: length
     | - bit 4: direction
     | - bit 0-3: volume
  1  | sound length
     | - 64 is infinity
  1  | flags
     | - bit 1: always init envelope
     | - bit 0: software envelope (zombie mode)
  1  | hardware sequence length
 ??? | hardware sequence...
     | - length: 3*hwSeqLen
```

a value in the hardware sequence has the following format:

```
size | description
-----|------------------------------------
  1  | command
     | - 0: set envelope
     | - 1: set sweep
     | - 2: wait
     | - 3: wait for release
     | - 4: loop
     | - 5: loop until release
  2  | data
     | - for set envelope:
     |   - 1 byte: parameter
     |     - bit 4-7: volume
     |     - bit 3: direction
     |     - bit 0-2: length
     |   - 1 byte: sound length
     | - for set sweep:
     |   - 1 byte: parameter
     |     - bit 4-6: length
     |     - bit 3: direction
     |     - bit 0-2: shift
     |   - 1 byte: nothing
     | - for wait:
     |   - 1 byte: length (in ticks)
     |   - 1 byte: nothing
     | - for wait for release:
     |   - 2 bytes: nothing
     | - for loop/loop until release:
     |   - 2 bytes: position
```

# sample ins data (SM)

```
size | description
-----|------------------------------------
  2  | initial sample
  1  | flags
     | - bit 2: use wave
     | - bit 1: use sample
     | - bit 0: use sample map
  1  | waveform length
 4?? | sample map... (120 entries)
     | - only read if sample map is enabled
```

the sample map format:

```
size | description
-----|------------------------------------
  2  | note to play
  2  | sample to play
```

# operator macro data (O1, O2, O3 and O4)

similar to macro data, but using these macro codes:

- 0: AM
- 1: AR
- 2: DR
- 3: MULT
- 4: RR
- 5: SL
- 6: TL
- 7: DT2
- 8: RS
- 9: DT
- 10: D2R
- 11: SSG-EG
- 12: DAM
- 13: DVB
- 14: EGT
- 15: KSL
- 16: SUS
- 17: VIB
- 18: WS
- 19: KSR

# OPL drums mode data (LD)

```
size | description
-----|------------------------------------
  1  | fixed frequency mode
  2  | kick freq
  2  | snare/hat freq
  2  | tom/top freq
```

# SNES data (SN)

```
size | description
-----|------------------------------------
  1  | attack/decay
     | - bit 4-6: decay
     | - bit 0-3: attack
  1  | sustain/release
     | - bit 5-7: sustain
     | - bit 0-4: release
  1  | flags
     | - bit 4: envelope on
     | - bit 3: make sustain effective (<131)
     | - bit 0-2: gain mode
     |   - 0: direct
     |   - 4: dec
     |   - 5: exp
     |   - 6: inc
     |   - 7: bent
  1  | gain
  1  | decay 2/sustain mode (>=131)
     | - bit 5-6: sustain mode
     |   - 0: direct
     |   - 1: sustain (release with dec)
     |   - 2: sustain (release with exp)
     |   - 3: sustain (release with rel)
     | - bit 0-4: decay 2
```

# Namco 163 data (N1)

```
size | description
-----|------------------------------------
  4  | waveform
  1  | wave pos
  1  | wave len
  1  | wave mode
```

# FDS/Virtual Boy data (FD)

```
size | description
-----|------------------------------------
  4  | mod speed
  4  | mod depth
  1  | init mod table with first wave
 1?? | modulation table (32 entries)
```

# wavetable synth data (WS)

```
size | description
-----|------------------------------------
  4  | first wave
  4  | second wave
  1  | rate divider
  1  | effect
     | - bit 7: single or dual effect
  1  | enabled
  1  | global
  1  | speed (+1)
  1  | parameter 1
  1  | parameter 2
  1  | parameter 3
  1  | parameter 4
```

# list of samples (SL)

```
size | description
-----|------------------------------------
  1  | number of samples
 1?? | sample indexes...
 4?? | pointers to samples...
     | - these use the Furnace sample format.
```

# list of wavetables (WL)

```
size | description
-----|------------------------------------
  1  | number of wavetables
 1?? | wavetable indexes...
 4?? | pointers to wavetables...
     | - these use the Furnace wavetable format.
```

# MultiPCM data (MP)

```
size | description
-----|------------------------------------
  1  | attack rate
  1  | decay 1 rate
  1  | decay level
  1  | decay 2 rate
  1  | release rate
  1  | rate correction
  1  | LFO rate
  1  | vibrato depth
  1  | AM depth
```

# Sound Unit data (SU)

```
size | description
-----|------------------------------------
  1  | switch roles of phase reset timer and frequency
```

# ES5506 data (ES)

```
size | description
-----|------------------------------------
  1  | filter mode
     | - 0: HPK2_HPK2
     | - 1: HPK2_LPK1
     | - 2: LPK2_LPK2
     | - 3: LPK2_LPK1
  2  | K1
  2  | K2
  2  | envelope count
  1  | left volume ramp
  1  | right volume ramp
  1  | K1 ramp
  1  | K2 ramp
  1  | K1 slow
  1  | K2 slow
```

# X1-010 data (X1)

```
size | description
-----|------------------------------------
  4  | bank slot
```
