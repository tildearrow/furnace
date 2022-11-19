# possible new Furnace instrument format

the main issue with Furnace instrument files is that they are too big, even if the instrument is nothing more than the FM setup...

the aim of this new format is to greatly reduce the size of a resulting instrument.

# header

```
size | description
-----|------------------------------------
  4  | "FINS" format magic
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

# FM data (FM)

```
size | description
-----|------------------------------------
  1  | operator count
-----|------------------------------------
     | **base data**
     | /7 6 5 4 3 2 1 0|
  1  | |x| ALG |x| FB  |
  1  | |FMS2 |AMS| FMS |
  1  | |AM2|4| LLPatch |
  1  | |KV4|KV3|KV2|KV1|
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
  1  | |e|x|     D2R   |
     |  \- EGT
  1  | |  S L  |  R R  |
  1  | |  DVB  |  SSG  |
  1  | | DAM |   W S   |
```

# macro data (MA)

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
     | - 17: ex7
     | - 18: ex8
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

