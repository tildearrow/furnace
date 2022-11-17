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
- `FD`: FDS ins data
- `WS`: wavetable synth data
- `SL`: list of samples
- `WL`: list of wavetables
- `MP`: MultiPCM ins data
- `SU`: Sound Unit ins data
- `ES`: ES5506 ins data

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
