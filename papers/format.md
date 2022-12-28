# the Furnace file format (.fur)

while Furnace works directly with the .dmf format, I had to create a new format to handle future additions to the program.

this document has the goal of detailing the format.

**notice:** GitHub's Markdown formatter may break on this file as it doesn't seem to treat tables correctly.

# information

files may be zlib-compressed, but Furnace accepts uncompressed files as well.

all numbers are little-endian.

the following fields may be found in "size":
- `f` indicates a floating point number.
- `STR` is a UTF-8 zero-terminated string.
- `???` is an array of variable size.
- `S??` is an array of `STR`s.
- `1??` is an array of bytes.
- `2??` is an array of shorts.
- `4??` is an array of ints.

the format has changed several times across versions. a `(>=VER)` indicates this field is only present starting from format version `VER`, and `(<VER)` indicates this field is present only before version `VER`.

furthermore, an `or reserved` indicates this field is always present, but is reserved when the version condition is not met.

the `size of this block` fields represent the size of a block excluding the ID and the aforementioned field.
these fields are 0 in format versions prior to 100 (0.6pre1).

# format versions

the format versions are:

- 133: Furnace 0.6pre3
- 132: Furnace 0.6pre2
- 131: Furnace dev131
- 130: Furnace dev130
- 129: Furnace dev129
- 128: Furnace dev128
- 127: Furnace dev127
- 126: Furnace dev126
- 125: Furnace dev125
- 124: Furnace dev124
- 123: Furnace dev123
- 122: Furnace dev122
- 121: Furnace dev121
- 120: Furnace dev120
- 119: Furnace dev119
- 118: Furnace dev118
- 117: Furnace dev117
- 116: Furnace 0.6pre1.5
- 115: Furnace dev115
- 114: Furnace dev114
- 113: Furnace dev113
- 112: Furnace dev112
- 111: Furnace dev111
- 110: Furnace dev110
- 109: Furnace dev109
- 108: Furnace dev108
- 107: Furnace dev107
- 106: Furnace dev106
- 105: Furnace dev105
- 104: Furnace dev104
- 103: Furnace dev103
- 102: Furnace 0.6pre1 (dev102)
- 101: Furnace 0.6pre1 (dev101)
- 100: Furnace 0.6pre1
- 99: Furnace dev99
- 98: Furnace dev98
- 97: Furnace dev97
- 96: Furnace dev96
- 95: Furnace dev95
- 94: Furnace dev94
- 93: Furnace dev93
- 92: Furnace dev92
- 91: Furnace dev91
- 90: Furnace dev90
- 89: Furnace dev89
- 88: Furnace dev88
- 87: Furnace dev87
- 86: Furnace dev86
- 85: Furnace dev85
- 84: Furnace dev84
- 83: Furnace dev83
- 82: Furnace dev82
- 81: Furnace dev81
- 80: Furnace dev80
- 79: Furnace dev79
- 78: Furnace dev78
- 77: Furnace dev77
- 76: Furnace dev76
- 75: Furnace dev75/April Fools' 0.6pre0
- 74: Furnace dev74
- 73: Furnace dev73
- 72: Furnace dev72
- 71: Furnace dev71
- 70: Furnace dev70
- 69: Furnace dev69
- 68: Furnace dev68
- 67: Furnace dev67
- 66: Furnace dev66
- 65: Furnace dev65
- 64: Furnace dev64
- 63: Furnace dev63
- 62: Furnace dev62
- 61: Furnace dev61
- 60: Furnace dev60
- 59: Furnace dev59
- 58: Furnace dev58
- 57: Furnace dev57

- 54: Furnace 0.5.8
- 53: Furnace 0.5.7
- 52: Furnace 0.5.7pre4
- 51: Furnace 0.5.7pre3
- 50: Furnace 0.5.7pre2
- 49: Furnace 0.5.7pre1
- 48: Furnace 0.5.6
- 47: Furnace 0.5.6pre1
- 46: Furnace 0.5.5
- 45: Furnace 0.5.5pre3
- 44: Furnace 0.5.5pre2
- 43: Furnace 0.5.5pre1
- 42: Furnace 0.5.4
- 41: Furnace 0.5.3
- 40: Furnace 0.5.2
- 39: Furnace 0.5.2pre3
- 38: Furnace 0.5.2pre2
- 37: Furnace 0.5.2pre1
- 36: Furnace 0.5.1
- 35: Furnace 0.5
- 27: Furnace 0.4.6
- 26: Furnace 0.4.6pre1
- 25: Furnace 0.4.5
- 24: Furnace 0.4.4
- 23: Furnace 0.4.3
- 22: Furnace 0.4.2
- 21: Furnace 0.4.1
- 20: Furnace 0.4
- 19: Furnace 0.4pre3
- 18: Furnace 0.4pre2
- 17: Furnace 0.4pre1
- 16: Furnace 0.3.1
- 15: Furnace 0.3
- 14: Furnace 0.2.2
- 13: Furnace 0.2.1
- 12: Furnace 0.2

# header

the header is 32 bytes long.

```
size | description
-----|------------------------------------
 16  | "-Furnace module-" format magic
  2  | format version
  2  | reserved
  4  | song info pointer
  8  | reserved
```

# song info

```
size | description
-----|------------------------------------
  4  | "INFO" block ID
  4  | size of this block
  1  | time base (of first song)
  1  | speed 1 (of first song)
  1  | speed 2 (of first song)
  1  | initial arpeggio time (of first song)
  4f | ticks per second (of first song)
     | - 60 is NTSC
     | - 50 is PAL
  2  | pattern length (of first song)
     | - the limit is 256.
  2  | orders length (of first song)
     | - the limit is 256 (>=80) or 127 (<80).
  1  | highlight A (of first song)
  1  | highlight B (of first song)
  2  | instrument count
     | - the limit is 256.
  2  | wavetable count
     | - the limit is 256.
  2  | sample count
     | - the limit is 256.
  4  | pattern count (global)
 32  | list of sound chips
     | - possible soundchips:
     |   - 0x00: end of list
     |   - 0x01: YMU759 - 17 channels
     |   - 0x02: Genesis - 10 channels (compound!)
     |   - 0x03: SMS (SN76489) - 4 channels
     |   - 0x04: Game Boy - 4 channels
     |   - 0x05: PC Engine - 6 channels
     |   - 0x06: NES - 5 channels
     |   - 0x07: C64 (8580) - 3 channels
     |   - 0x08: Arcade (YM2151+SegaPCM) - 13 channels (compound!)
     |   - 0x09: Neo Geo CD (YM2610) - 13 channels
     |   - 0x42: Genesis extended - 13 channels
     |   - 0x43: SMS (SN76489) + OPLL (YM2413) - 13 channels (compound!)
     |   - 0x46: NES + VRC7 - 11 channels (compound!)
     |   - 0x47: C64 (6581) - 3 channels
     |   - 0x49: Neo Geo CD extended - 16 channels
     |   - 0x80: AY-3-8910 - 3 channels
     |   - 0x81: Amiga - 4 channels
     |   - 0x82: YM2151 alone - 8 channels
     |   - 0x83: YM2612 alone - 6 channels
     |   - 0x84: TIA - 2 channels
     |   - 0x85: VIC-20 - 4 channels
     |   - 0x86: PET - 1 channel
     |   - 0x87: SNES - 8 channels
     |   - 0x88: VRC6 - 3 channels
     |   - 0x89: OPLL (YM2413) - 9 channels
     |   - 0x8a: FDS - 1 channel
     |   - 0x8b: MMC5 - 3 channels
     |   - 0x8c: Namco 163 - 8 channels
     |   - 0x8d: YM2203 - 6 channels
     |   - 0x8e: YM2608 - 16 channels
     |   - 0x8f: OPL (YM3526) - 9 channels
     |   - 0x90: OPL2 (YM3812) - 9 channels
     |   - 0x91: OPL3 (YMF262) - 18 channels
     |   - 0x92: MultiPCM - 28 channels
     |   - 0x93: Intel 8253 (beeper) - 1 channel
     |   - 0x94: POKEY - 4 channels
     |   - 0x95: RF5C68 - 8 channels
     |   - 0x96: WonderSwan - 4 channels
     |   - 0x97: Philips SAA1099 - 6 channels
     |   - 0x98: OPZ (YM2414) - 8 channels
     |   - 0x99: Pokémon Mini - 1 channel
     |   - 0x9a: AY8930 - 3 channels
     |   - 0x9b: SegaPCM - 16 channels
     |   - 0x9c: Virtual Boy - 6 channels
     |   - 0x9d: VRC7 - 6 channels
     |   - 0x9e: YM2610B - 16 channels
     |   - 0x9f: ZX Spectrum (beeper) - 6 channels
     |   - 0xa0: YM2612 extended - 9 channels
     |   - 0xa1: Konami SCC - 5 channels
     |   - 0xa2: OPL drums (YM3526) - 11 channels
     |   - 0xa3: OPL2 drums (YM3812) - 11 channels
     |   - 0xa4: OPL3 drums (YMF262) - 20 channels
     |   - 0xa5: Neo Geo (YM2610) - 14 channels
     |   - 0xa6: Neo Geo extended (YM2610) - 17 channels
     |   - 0xa7: OPLL drums (YM2413) - 11 channels
     |   - 0xa8: Atari Lynx - 4 channels
     |   - 0xa9: SegaPCM (for DefleMask compatibility) - 5 channels
     |   - 0xaa: MSM6295 - 4 channels
     |   - 0xab: MSM6258 - 1 channel
     |   - 0xac: Commander X16 (VERA) - 17 channels
     |   - 0xad: Bubble System WSG - 2 channels
     |   - 0xae: OPL4 (YMF278B) - 42 channels
     |   - 0xaf: OPL4 drums (YMF278B) - 44 channels
     |   - 0xb0: Seta/Allumer X1-010 - 16 channels
     |   - 0xb1: Ensoniq ES5506 - 32 channels
     |   - 0xb2: Yamaha Y8950 - 10 channels
     |   - 0xb3: Yamaha Y8950 drums - 12 channels
     |   - 0xb4: Konami SCC+ - 5 channels
     |   - 0xb5: tildearrow Sound Unit - 8 channels
     |   - 0xb6: YM2203 extended - 9 channels
     |   - 0xb7: YM2608 extended - 19 channels
     |   - 0xb8: YMZ280B - 8 channels
     |   - 0xb9: Namco WSG - 3 channels
     |   - 0xba: Namco 15xx - 8 channels
     |   - 0xbb: Namco CUS30 - 8 channels
     |   - 0xbc: MSM5232 - 8 channels
     |   - 0xbd: YM2612 extra features extended - 11 channels
     |   - 0xbe: YM2612 extra features - 7 channels
     |   - 0xbf: T6W28 - 4 channels
     |   - 0xc0: PCM DAC - 1 channel
     |   - 0xc1: YM2612 CSM - 10 channels
     |   - 0xc2: Neo Geo CSM (YM2610) - 18 channels
     |   - 0xc3: YM2203 CSM - 10 channels
     |   - 0xc4: YM2608 CSM - 20 channels
     |   - 0xc5: YM2610B CSM - 20 channels
     |   - 0xc6: K007232 - 2 channels
     |   - 0xc7: GA20 - 4 channels
     |   - 0xde: YM2610B extended - 19 channels
     |   - 0xe0: QSound - 19 channels
     |   - 0xfc: Pong - 1 channel
     |   - 0xfd: Dummy System - 8 channels
     |   - 0xfe: reserved for development
     |   - 0xff: reserved for development
     | - (compound!) means that the system is composed of two or more chips,
     |   and has to be flattened.
 32  | sound chip volumes
     | - signed char, 64=1.0, 127=~2.0
 32  | sound chip panning
     | - signed char, -128=left, 127=right
 128 | sound chip flag pointers (>=119) or sound chip flags
     | - before 118, these were 32-bit flags.
     | - for conversion details, see the "converting from old flags" section.
 STR | song name
 STR | song author
  4f | A-4 tuning
  1  | limit slides (>=36) or reserved
  1  | linear pitch (>=36) or reserved
     | - 0: non-linear
     | - 1: only pitch change (04xy/E5xx) linear
     | - 2: full linear (>=94)
  1  | loop modality (>=36) or reserved
  1  | proper noise layout (>=42) or reserved
  1  | wave duty is volume (>=42) or reserved
  1  | reset macro on porta (>=45) or reserved
  1  | legacy volume slides (>=45) or reserved
  1  | compatible arpeggio (>=45) or reserved
  1  | note off resets slides (>=45) or reserved
  1  | target resets slides (>=45) or reserved
  1  | arpeggio inhibits portamento (>=47) or reserved
  1  | wack algorithm macro (>=47) or reserved
  1  | broken shortcut slides (>=49) or reserved
  1  | ignore duplicate slides (>=50) or reserved
  1  | stop portamento on note off (>=62) or reserved
  1  | continuous vibrato (>=62) or reserved
  1  | broken DAC mode (>=64) or reserved
  1  | one tick cut (>=65) or reserved
  1  | instrument change allowed during porta (>=66) or reserved
  1  | reset note base on arpeggio effect stop (0000) (>=69) or reserved
 4?? | pointers to instruments
 4?? | pointers to wavetables
 4?? | pointers to samples
 4?? | pointers to patterns
 ??? | orders (of first song)
     | - a table of bytes
     | - size=channels*ordLen
     | - read orders then channels
     | - the maximum value of a cell is FF (>=80) or 7F (<80).
 ??? | effect columns (of first song)
     | - size=channels
 1?? | channel hide status (of first song)
     | - size=channels
 1?? | channel collapse status (of first song)
     | - size=channels
 S?? | channel names (of first song)
     | - a list of channelCount C strings
 S?? | channel short names (of first song)
     | - same as above
 STR | song comment
  4f | master volume, 1.0f=100% (>=59)
     | this is 2.0f for modules before 59
 --- | **extended compatibility flags** (>=70)
  1  | broken speed selection
  1  | no slides on first tick (>=71) or reserved
  1  | next row reset arp pos (>=71) or reserved
  1  | ignore jump at end (>=71) or reserved
  1  | buggy portamento after slide (>=72) or reserved
  1  | new ins affects envelope (Game Boy) (>=72) or reserved
  1  | ExtCh channel state is shared (>=78) or reserved
  1  | ignore DAC mode change outside of intended channel (>=83) or reserved
  1  | E1xy and E2xy also take priority over Slide00 (>=83) or reserved
  1  | new Sega PCM (with macros and proper vol/pan) (>=84) or reserved
  1  | weird f-num/block-based chip pitch slides (>=85) or reserved
  1  | SN duty macro always resets phase (>=86) or reserved
  1  | pitch macro is linear (>=90) or reserved
  1  | pitch slide speed in full linear pitch mode (>=94) or reserved
  1  | old octave boundary behavior (>=97) or reserved
  1  | disable OPN2 DAC volume control (>=98) or reserved
  1  | new volume scaling strategy (>=99) or reserved
  1  | volume macro still applies after end (>=99) or reserved
  1  | broken outVol (>=99) or reserved
  1  | E1xy and E2xy stop on same note (>=100) or reserved
  1  | broken initial position of porta after arp (>=101) or reserved
  1  | SN periods under 8 are treated as 1 (>=108) or reserved
  1  | cut/delay effect policy (>=110) or reserved
  1  | 0B/0D effect treatment (>=113) or reserved
  1  | automatic system name detection (>=115) or reserved
     | - this one isn't a compatibility flag, but it's here for convenience...
  1  | disable sample macro (>=117) or reserved
  1  | broken outVol episode 2 (>=121) or reserved
  1  | old arpeggio strategy (>=130) or reserved
 --- | **virtual tempo data**
  2  | virtual tempo numerator of first song (>=96) or reserved
  2  | virtual tempo denominator of first song (>=96) or reserved
 --- | **additional subsongs** (>=95)
 STR | first subsong name
 STR | first subsong comment
  1  | number of additional subsongs
  3  | reserved
 4?? | pointers to subsong data
 --- | **additional metadata** (>=103)
 STR | system name
 STR | album/category/game name
 STR | song name (Japanese)
 STR | song author (Japanese)
 STR | system name (Japanese)
 STR | album/category/game name (Japanese)
```

# subsong

from version 95 onwards, Furnace supports storing multiple songs on a single file.
the way it's currently done is really weird, but it provides for some backwards compatibility (previous versions will only load the first subsong which is already defined in the `INFO` block).

```
size | description
-----|------------------------------------
  4  | "SONG" block ID
  4  | size of this block
  1  | time base
  1  | speed 1
  1  | speed 2
  1  | initial arpeggio time
  4f | ticks per second
     | - 60 is NTSC
     | - 50 is PAL
  2  | pattern length
     | - the limit is 256.
  2  | orders length
     | - the limit is 256.
  1  | highlight A
  1  | highlight B
  2  | virtual tempo numerator
  2  | virtual tempo denominator
 STR | subsong name
 STR | subsong comment
 ??? | orders
     | - a table of bytes
     | - size=channels*ordLen
     | - read orders then channels
     | - the maximum value of a cell is FF.
 ??? | effect columns
     | - size=channels
 1?? | channel hide status
     | - size=channels
 1?? | channel collapse status
     | - size=channels
 S?? | channel names
     | - a list of channelCount C strings
 S?? | channel short names
     | - same as above
```

# chip flags

```
size | description
-----|------------------------------------
  4  | "FLAG" block ID
  4  | size of this block
 STR | data
```

flags are stored in text (`key=value`) format. for example:

```
clock=4000000
stereo=true
```

# instrument (>=127)

Furnace dev127 and higher use the new instrument format.

```
size | description
-----|------------------------------------
  4  | "INS2" block ID
  4  | size of this block
  2  | format version
  2  | instrument type
 ??? | features...
```

see [newIns.md](newIns.md) for more information.

# old instrument (<127)

notes:

- the entire instrument is stored, regardless of instrument type.
- the macro range varies depending on the instrument type.
- "macro open" indicates whether the macro is collapsed or not in the instrument editor.
  - as of format version 120, bit 1-2 indicates macro mode:
    - 0: sequence (normal)
    - 1: ADSR
    - 2: LFO
  - see sub-section for information on how to interpret parameters.
- FM operator order is:
  - 1/3/2/4 (internal order) for OPN, OPM, OPZ and OPL 4-op
  - 1/2/?/? (? = unused) for OPL 2-op and OPLL
- meaning of extended macros varies depending on instrument type.
- meaning of panning macros varies depending on instrument type:
  - for hard-panned chips (e.g. FM and Game Boy): left panning is 2-bit panning macro (left/right)
  - otherwise both left and right panning macros are used

```
size | description
-----|------------------------------------
  4  | "INST" block ID
  4  | size of this block
  2  | format version (see header)
  1  | instrument type
     | - 0: SN76489/standard
     | - 1: FM (OPN)
     | - 2: Game Boy
     | - 3: C64
     | - 4: Amiga/sample
     | - 5: PC Engine
     | - 6: AY-3-8910
     | - 7: AY8930
     | - 8: TIA
     | - 9: SAA1099
     | - 10: VIC
     | - 11: PET
     | - 12: VRC6
     | - 13: OPLL
     | - 14: OPL
     | - 15: FDS
     | - 16: Virtual Boy
     | - 17: Namco 163
     | - 18: SCC
     | - 19: OPZ
     | - 20: POKEY
     | - 21: PC Speaker
     | - 22: WonderSwan
     | - 23: Lynx
     | - 24: VERA
     | - 25: X1-010
     | - 26: VRC6 (saw)
     | - 27: ES5506
     | - 28: MultiPCM
     | - 29: SNES
     | - 30: Sound Unit
     | - 31: Namco WSG
     | - 32: OPL (drums)
     | - 33: FM (OPM)
     | - 34: NES
     | - 35: MSM6258
     | - 36: MSM6295
     | - 37: ADPCM-A
     | - 38: ADPCM-B
     | - 39: SegaPCM
     | - 40: QSound
     | - 41: YMZ280B
     | - 42: RF5C68
     | - 43: MSM5232
     | - 44: T6W28
  1  | reserved
 STR | instrument name
 --- | **FM instrument data**
  1  | alg (SUS on OPLL)
  1  | feedback
  1  | fms (DC on OPLL)
  1  | ams (DM on OPLL)
  1  | operator count
     | - this is either 2 or 4, and is ignored on non-OPL systems.
     | - always read 4 ops regardless of this value.
  1  | OPLL preset (>=60) or reserved
     | - 0: custom
     | - 1-15: pre-defined patches
     | - 16: drums (compatibility only!)
  2  | reserved
 --- | **FM operator data** × 4
  1  | am
  1  | ar
  1  | dr
  1  | mult
  1  | rr
  1  | sl
  1  | tl
  1  | dt2
  1  | rs
  1  | dt
  1  | d2r
  1  | ssgEnv
     | - bit 4: on (EG-S on OPLL)
     | - bit 0-3: envelope type
  1  | dam (for YMU759 compat; REV on OPZ)
  1  | dvb (for YMU759 compat; FINE on OPZ)
  1  | egt (for YMU759 compat; FixedFreq on OPZ)
  1  | ksl (EGShift on OPZ)
  1  | sus
  1  | vib
  1  | ws
  1  | ksr
  1  | operator enabled (>=114) or reserved
  1  | KVS mode (>=115) or reserved
     | - 0: off
     | - 1: on
     | - 2: auto (depending on alg)
 10  | reserved
 --- | **Game Boy instrument data**
  1  | volume
  1  | direction
  1  | length
  1  | sound length
 --- | **C64 instrument data**
  1  | triangle
  1  | saw
  1  | pulse
  1  | noise
  1  | attack
  1  | decay
  1  | sustain
  1  | release
  2  | duty
  1  | ring mod
  1  | osc sync
  1  | to filter
  1  | init filter
  1  | vol macro is cutoff
  1  | resonance
  1  | low pass
  1  | band pass
  1  | high pass
  1  | channel 3 off
  2  | cutoff
  1  | duty macro is absolute
  1  | filter macro is absolute
 --- | **Amiga instrument data**
  2  | initial sample
  1  | mode (>=82) or reserved
     | - 0: sample
     | - 1: wavetable
  1  | wavetable length (-1) (>=82) or reserved
 12  | reserved
 --- | **standard instrument data**
  4  | volume macro length
  4  | arp macro length
  4  | duty macro length
  4  | wave macro length
  4  | pitch macro length (>=17)
  4  | extra 1 macro length (>=17)
  4  | extra 2 macro length (>=17)
  4  | extra 3 macro length (>=17)
  4  | volume macro loop
  4  | arp macro loop
  4  | duty macro loop
  4  | wave macro loop
  4  | pitch macro loop (>=17)
  4  | extra 1 macro loop (>=17)
  4  | extra 2 macro loop (>=17)
  4  | extra 3 macro loop (>=17)
  1  | arp macro mode (<112) or reserved
     | - treat this value in a special way.
     | - before version 112, this byte indicates whether the arp macro mode is fixed or not.
     | - from that version onwards, the fixed mode is part of the macro values.
     | - to convert a <112 macro mode to a modern one, do the following:
     |   - is the macro mode set to fixed?
     |     - if yes, then:
     |       - set bit 30 of all arp macro values (this is the fixed mode bit)
     |       - does the macro loop?
     |         - if yes, then do nothing else
     |         - if no, then add one to the macro length, and set the last macro value to 0
     |     - if no, then do nothing
  1  | reserved (>=17) or volume macro height (>=15) or reserved
  1  | reserved (>=17) or duty macro height (>=15) or reserved
  1  | reserved (>=17) or wave macro height (>=15) or reserved
 4?? | volume macro
     | - before version 87, if this is the C64 relative cutoff macro, its values were stored offset by 18.
 4?? | arp macro
     | - before version 31, this macro's values were stored offset by 12.
     | - from version 112 onward, bit 30 of a value indicates fixed mode.
 4?? | duty macro
     | - before version 87, if this is the C64 relative duty macro, its values were stored offset by 12.
 4?? | wave macro
 4?? | pitch macro (>=17)
 4?? | extra 1 macro (>=17)
 4?? | extra 2 macro (>=17)
 4?? | extra 3 macro (>=17)
  4  | alg macro length (>=29)
  4  | fb macro length (>=29)
  4  | fms macro length (>=29)
  4  | ams macro length (>=29)
  4  | alg macro loop (>=29)
  4  | fb macro loop (>=29)
  4  | fms macro loop (>=29)
  4  | ams macro loop (>=29)
  1  | volume macro open (>=29)
  1  | arp macro open (>=29)
  1  | duty macro open (>=29)
  1  | wave macro open (>=29)
  1  | pitch macro open (>=29)
  1  | extra 1 macro open (>=29)
  1  | extra 2 macro open (>=29)
  1  | extra 3 macro open (>=29)
  1  | alg macro open (>=29)
  1  | fb macro open (>=29)
  1  | fms macro open (>=29)
  1  | ams macro open (>=29)
 4?? | alg macro (>=29)
 4?? | fb macro (>=29)
 4?? | fms macro (>=29)
 4?? | ams macro (>=29)
 --- | **operator macro headers** × 4 (>=29)
  4  | AM macro length
  4  | AR macro length
  4  | DR macro length
  4  | MULT macro length
  4  | RR macro length
  4  | SL macro length
  4  | TL macro length
  4  | DT2 macro length
  4  | RS macro length
  4  | DT macro length
  4  | D2R macro length
  4  | SSG-EG macro length
  4  | AM macro loop
  4  | AR macro loop
  4  | DR macro loop
  4  | MULT macro loop
  4  | RR macro loop
  4  | SL macro loop
  4  | TL macro loop
  4  | DT2 macro loop
  4  | RS macro loop
  4  | DT macro loop
  4  | D2R macro loop
  4  | SSG-EG macro loop
  1  | AM macro open
  1  | AR macro open
  1  | DR macro open
  1  | MULT macro open
  1  | RR macro open
  1  | SL macro open
  1  | TL macro open
  1  | DT2 macro open
  1  | RS macro open
  1  | DT macro open
  1  | D2R macro open
  1  | SSG-EG macro open
 --- | **operator macros** × 4 (>=29)
 1?? | AM macro
 1?? | AR macro
 1?? | DR macro
 1?? | MULT macro
 1?? | RR macro
 1?? | SL macro
 1?? | TL macro
 1?? | DT2 macro
 1?? | RS macro
 1?? | DT macro
 1?? | D2R macro
 1?? | SSG-EG macro
 --- | **release points** (>=44)
  4  | volume macro release
  4  | arp macro release
  4  | duty macro release
  4  | wave macro release
  4  | pitch macro release
  4  | extra 1 macro release
  4  | extra 2 macro release
  4  | extra 3 macro release
  4  | alg macro release
  4  | fb macro release
  4  | fms macro release
  4  | ams macro release
 --- | **operator release points** × 4 (>=44)
  4  | AM macro release
  4  | AR macro release
  4  | DR macro release
  4  | MULT macro release
  4  | RR macro release
  4  | SL macro release
  4  | TL macro release
  4  | DT2 macro release
  4  | RS macro release
  4  | DT macro release
  4  | D2R macro release
  4  | SSG-EG macro release
 --- | **extended op macro headers** × 4 (>=61)
  4  | DAM macro length
  4  | DVB macro length
  4  | EGT macro length
  4  | KSL macro length
  4  | SUS macro length
  4  | VIB macro length
  4  | WS macro length
  4  | KSR macro length
  4  | DAM macro loop
  4  | DVB macro loop
  4  | EGT macro loop
  4  | KSL macro loop
  4  | SUS macro loop
  4  | VIB macro loop
  4  | WS macro loop
  4  | KSR macro loop
  4  | DAM macro release
  4  | DVB macro release
  4  | EGT macro release
  4  | KSL macro release
  4  | SUS macro release
  4  | VIB macro release
  4  | WS macro release
  4  | KSR macro release
  1  | DAM macro open
  1  | DVB macro open
  1  | EGT macro open
  1  | KSL macro open
  1  | SUS macro open
  1  | VIB macro open
  1  | WS macro open
  1  | KSR macro open
 --- | **extended op macros** × 4 (>=61)
 1?? | DAM macro
 1?? | DVB macro
 1?? | EGT macro
 1?? | KSL macro
 1?? | SUS macro
 1?? | VIB macro
 1?? | WS macro
 1?? | KSR macro
 --- | **OPL drums mode data** (>=63)
  1  | fixed frequency mode
  1  | reserved
  2  | kick frequency
  2  | snare/hi-hat frequency
  2  | tom/top frequency
 --- | **Sample instrument extra data** (>=67)
  1  | use note map
     | - only read the following two data structures if this is true!
 4?? | note frequency × 120
     | - 480 bytes
 2?? | note sample × 120
     | - 240 bytes
 --- | **Namco 163 data** (>=73)
  4  | initial waveform
  1  | wave position
  1  | wave length
  1  | wave mode:
     | - bit 1: update on change
     | - bit 0: load on playback
  1  | reserved
 --- | **even more macros** (>=76)
  4  | left panning macro length
  4  | right panning macro length
  4  | phase reset macro length
  4  | extra 4 macro length
  4  | extra 5 macro length
  4  | extra 6 macro length
  4  | extra 7 macro length
  4  | extra 8 macro length
  4  | left panning macro loop
  4  | right panning macro loop
  4  | phase reset macro loop
  4  | extra 4 macro loop
  4  | extra 5 macro loop
  4  | extra 6 macro loop
  4  | extra 7 macro loop
  4  | extra 8 macro loop
  4  | left panning macro release
  4  | right panning macro release
  4  | phase reset macro release
  4  | extra 4 macro release
  4  | extra 5 macro release
  4  | extra 6 macro release
  4  | extra 7 macro release
  4  | extra 8 macro release
  1  | left panning macro open
  1  | right panning macro open
  1  | phase reset macro open
  1  | extra 4 macro open
  1  | extra 5 macro open
  1  | extra 6 macro open
  1  | extra 7 macro open
  1  | extra 8 macro open
 --- | **even more macro data** (>=76)
 4?? | left panning macro
 4?? | right panning macro
 4?? | phase reset macro
 4?? | extra 4 macro
 4?? | extra 5 macro
 4?? | extra 6 macro
 4?? | extra 7 macro
 4?? | extra 8 macro
 --- | **FDS instrument data** (>=76)
  4  | modulation speed
  4  | modulation depth
  1  | init modulation table with first wave
  3  | reserved
 32  | modulation table
 --- | **OPZ instrument extra data** (>=77)
  1  | fms2
  1  | ams2
 --- | **wavetable synth data** (>=79)
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
 --- | **additional macro mode flags** (>=84)
  1  | volume macro mode
  1  | duty macro mode
  1  | wave macro mode
  1  | pitch macro mode
  1  | extra 1 macro mode
  1  | extra 2 macro mode
  1  | extra 3 macro mode
  1  | alg macro mode
  1  | fb macro mode
  1  | fms macro mode
  1  | ams macro mode
  1  | left panning macro mode
  1  | right panning macro mode
  1  | phase reset macro mode
  1  | extra 4 macro mode
  1  | extra 5 macro mode
  1  | extra 6 macro mode
  1  | extra 7 macro mode
  1  | extra 8 macro mode
 --- | **extra C64 data** (>=89)
  1  | don't test/gate before new note
 --- | **MultiPCM data** (>=93)
  1  | attack rate
  1  | decay 1 rate
  1  | decay level
  1  | decay 2 rate
  1  | release rate
  1  | rate correction
  1  | lfo rate
  1  | vib depth
  1  | am depth
 23  | reserved
 --- | **Sound Unit data** (>=104)
  1  | use sample
  1  | switch roles of phase reset timer and frequency
 --- | **Game Boy envelope sequence** (>=105)
  1  | length
 ??? | hardware sequence data
     | size is length*3:
     | 1 byte: command
     | - 0: set envelope
     | - 1: set sweep
     | - 2: wait
     | - 3: wait for release
     | - 4: loop
     | - 5: loop until release
     | 2 bytes: data
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
 --- | **Game Boy extra flags** (>=106)
  1  | use software envelope
  1  | always init hard env on new note
 --- | **ES5506 data** (>=107)
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
 --- | **SNES data** (>=109)
  1  | use envelope
  1  | gain mode
  1  | gain
  1  | attack
  1  | decay
  1  | sustain
     | - bit 3: sustain mode (>=118)
  1  | release
 --- | **macro speeds/delays** (>=111)
  1  | volume macro speed
  1  | arp macro speed
  1  | duty macro speed
  1  | wave macro speed
  1  | pitch macro speed
  1  | extra 1 macro speed
  1  | extra 2 macro speed
  1  | extra 3 macro speed
  1  | alg macro speed
  1  | fb macro speed
  1  | fms macro speed
  1  | ams macro speed
  1  | left panning macro speed
  1  | right panning macro speed
  1  | phase reset macro speed
  1  | extra 4 macro speed
  1  | extra 5 macro speed
  1  | extra 6 macro speed
  1  | extra 7 macro speed
  1  | extra 8 macro speed
  1  | volume macro delay
  1  | arp macro delay
  1  | duty macro delay
  1  | wave macro delay
  1  | pitch macro delay
  1  | extra 1 macro delay
  1  | extra 2 macro delay
  1  | extra 3 macro delay
  1  | alg macro delay
  1  | fb macro delay
  1  | fms macro delay
  1  | ams macro delay
  1  | left panning macro delay
  1  | right panning macro delay
  1  | phase reset macro delay
  1  | extra 4 macro delay
  1  | extra 5 macro delay
  1  | extra 6 macro delay
  1  | extra 7 macro delay
  1  | extra 8 macro delay
 --- | **operator macro speeds/delay** × 4 (>=111)
  1  | AM macro speed
  1  | AR macro speed
  1  | DR macro speed
  1  | MULT macro speed
  1  | RR macro speed
  1  | SL macro speed
  1  | TL macro speed
  1  | DT2 macro speed
  1  | RS macro speed
  1  | DT macro speed
  1  | D2R macro speed
  1  | SSG-EG macro speed
  1  | DAM macro speed
  1  | DVB macro speed
  1  | EGT macro speed
  1  | KSL macro speed
  1  | SUS macro speed
  1  | VIB macro speed
  1  | WS macro speed
  1  | KSR macro speed
  1  | AM macro delay
  1  | AR macro delay
  1  | DR macro delay
  1  | MULT macro delay
  1  | RR macro delay
  1  | SL macro delay
  1  | TL macro delay
  1  | DT2 macro delay
  1  | RS macro delay
  1  | DT macro delay
  1  | D2R macro delay
  1  | SSG-EG macro delay
  1  | DAM macro delay
  1  | DVB macro delay
  1  | EGT macro delay
  1  | KSL macro delay
  1  | SUS macro delay
  1  | VIB macro delay
  1  | WS macro delay
  1  | KSR macro delay
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

# wavetable

```
size | description
-----|------------------------------------
  4  | "WAVE" block ID
  4  | size of this block
 STR | wavetable name
  4  | wavetable width
  4  | reserved
  4  | wavetable height
 4?? | wavetable data
```

# sample (>=102)

this is the new sample storage format used in Furnace dev102 and higher.

```
size | description
-----|------------------------------------
  4  | "SMP2" block ID
  4  | size of this block
 STR | sample name
  4  | length
  4  | compatibility rate
  4  | C-4 rate
  1  | depth
     | - 0: ZX Spectrum overlay drum (1-bit)
     | - 1: 1-bit NES DPCM (1-bit)
     | - 3: YMZ ADPCM
     | - 4: QSound ADPCM
     | - 5: ADPCM-A
     | - 6: ADPCM-B
     | - 8: 8-bit PCM
     | - 9: BRR (SNES)
     | - 10: VOX
     | - 16: 16-bit PCM
  1  | loop direction (>=123) or reserved
     | - 0: forward
     | - 1: backward
     | - 2: ping-pong
  1  | flags (>=129) or reserved
     | - 0: BRR emphasis
  1  | reserved
  4  | loop start
     | - -1 means no loop
  4  | loop end
     | - -1 means no loop
 16  | sample presence bitfields
     | - for future use.
     | - indicates whether the sample should be present in the memory of a system.
     | - read 4 32-bit numbers (for 4 memory banks per system, e.g. YM2610
     |   does ADPCM-A and ADPCM-B on separate memory banks).
 ??? | sample data
     | - size is length
```

# old sample (<102)

this format is present when saving using previous Furnace versions.

```
size | description
-----|------------------------------------
  4  | "SMPL" block ID
  4  | size of this block
 STR | sample name
  4  | length
  4  | compatibility rate
  2  | volume (<58) or reserved
  2  | pitch (<58) or reserved
  1  | depth
     | - 0: ZX Spectrum overlay drum (1-bit)
     | - 1: 1-bit NES DPCM (1-bit)
     | - 3: YMZ ADPCM
     | - 4: QSound ADPCM
     | - 5: ADPCM-A
     | - 6: ADPCM-B
     | - 8: 8-bit PCM
     | - 9: BRR (SNES)
     | - 10: VOX
     | - 16: 16-bit PCM
  1  | reserved
  2  | C-4 rate (>=32) or reserved
  4  | loop point (>=19) or reserved
     | - -1 means no loop
 ??? | sample data
     | - version<58 size is length*2
     | - version>=58 size is length
```

# pattern

```
size | description
-----|------------------------------------
  4  | "PATR" block ID
  4  | size of this block
  2  | channel
  2  | pattern index
  2  | subsong (>=95) or reserved
  2  | reserved
 ??? | pattern data
     | - size: rows*(4+effectColumns*2)*2
     | - read shorts in this order:
     |   - note
     |     - 0: empty/invalid
     |     - 1: C#
     |     - 2: D
     |     - 3: D#
     |     - 4: E
     |     - 5: F
     |     - 6: F#
     |     - 7: G
     |     - 8: G#
     |     - 9: A
     |     - 10: A#
     |     - 11: B
     |     - 12: C (of next octave)
     |       - this is actually a leftover of the .dmf format.
     |     - 100: note off
     |     - 100: note release
     |     - 100: macro release
     |   - octave
     |     - this is an signed char stored in a short.
     |     - therefore octave value 255 is actually octave -1.
     |     - yep, another leftover of the .dmf format...
     |   - instrument
     |   - volume
     |   - effect and effect data (× effect columns)
     | - for note/octave, if both values are 0 then it means empty.
     | - for instrument, volume, effect and effect data, a value of -1 means empty.
 STR | pattern name (>=51)
```

# the Furnace instrument format (.fui)

the instrument format is pretty similar to the file format, but it also stores wavetables and samples used by the instrument.

```
size | description
-----|------------------------------------
 16  | "-Furnace instr.-" format magic
  2  | format version
  2  | reserved
  4  | pointer to instrument data
  2  | wavetable count
  2  | sample count
  4  | reserved
 4?? | pointers to wavetables
 4?? | pointers to samples
```

instrument data follows.

# the Furnace wavetable format (.fuw)

similar to the instrument format...

```
size | description
-----|------------------------------------
 16  | "-Furnace waveta-" format magic
  2  | format version
  2  | reserved
```

wavetable data follows.

# converting from old flags

prior to format version 119, chip flags were stored as a 32-bit integer.
this section will help you understand the old flag format.

chips which aren't on this list don't have any flags.

## 0x02: Genesis (COMPOUND) and 0x42: Genesis extended (COMPOUND)

- bit 31: ladderEffect (bool)
- bit 0-30: clockSel (int)
  - 0: NTSC
  - 1: PAL
  - 2: 8MHz
  - 3: Firecore (`COLOR_NTSC*12/7`)
  - 4: System 32 (`COLOR_NTSC*9/4`)
  - only 0 and 1 apply to the SN part as well.

## 0x03: SMS (SN76489)

- flags AND 0xff03: clockSel (int)
  - 0x0000: NTSC (becomes 0)
  - 0x0001: PAL (becomes 1)
  - 0x0002: 4MHz (becomes 2)
  - 0x0003: half NTSC (becomes 3)
  - 0x0100: 3MHz (becomes 4)
  - 0x0101: 2MHz (becomes 5)
  - 0x0102: eighth NTSC (becomes 6)
- flags AND 0xcc: chipType (int)
  - 0x00: Sega PSG (becomes 0)
  - 0x04: TI SN76489 (becomes 1)
  - 0x08: SN with Atari-like short noise (becomes 2)
  - 0x0c: Game Gear (becomes 3)
  - 0x40: TI SN76489A (becomes 4)
  - 0x44: TI SN76496 (becomes 5)
  - 0x48: NCR 8496 (becomes 6)
  - 0x4c: Tandy PSSJ 3-voice sound (becomes 7)
  - 0x80: TI SN94624 (becomes 8)
  - 0x84: TI SN76494 (becomes 9)
- bit 4: noPhaseReset (bool)

## 0x04: Game Boy

- bits 0-1: chipType (int)
  - 0: DMG (rev B)
  - 1: CGB (rev C)
  - 2: CGB (rev E)
  - 3: AGB
- bit 3: noAntiClick (bool)

## 0x05: PC Engine

- bit 0: clockSel (int)
  - 0: NTSC
  - 1: pseudo-PAL
- bit 2: chipType (int)
  - 0: HuC6280
  - 1: HuC6280A
- bit 3: noAntiClick (bool)

## 0x06: NES, 0x88: VRC6, 0x8a: FDS and 0x8b: MMC5

- flags: clockSel (int)
  - 0: NTSC (2A03)
  - 1: PAL (2A07)
  - 2: Dendy

## 0x07: C64 (8580) and 0x47: C64 (6581)

- bit 0-3: clockSel (int)
  - 0: NTSC
  - 1: PAL
  - 2: SSI 2001

## 0x08: Arcade (YM2151+SegaPCM; COMPOUND)

- bit 0-7: clockSel (int)
  - 0: NTSC
  - 1: PAL
  - 2: 4MHz
  - this clock only applies to the YM2151.

## 0x09: Neo Geo CD (YM2610), 0xa5: Neo Geo (YM2610), 0xa6: Neo Geo extended (YM2610), 0x49: Neo Geo CD extended, 0x9e: YM2610B and 0xde: YM2610B extended

- bit 0-7: clockSel (int)
  - 0: 8MHz
  - 1: 8.06MHz (Neo Geo AES)

## 0x80: AY-3-8910

- bit 0-3: clockSel (int)
  - 0: NTSC
  - 1: PAL
  - 2: ZX Spectrum 48K (1.75MHz)
  - 3: 2MHz
  - 4: 1.5MHz
  - 5: 1MHz
  - 6: Sunsoft 5B
  - 7: PAL NES
  - 8: Sunsoft 5B on PAL NES
  - 9: 1.10MHz
  - 10: 2^21MHz
  - 11: double NTSC
  - 12: 3.6MHz
  - 13: 1.25MHz
  - 14: 1.536MHz
- bit 4-5: chipType (int)
  - 0: AY-3-8910
  - 1: YM2149(F)
  - 2: Sunsoft 5B
  - 3: AY-3-8914
- bit 6: stereo (bool)
- bit 7: halfClock (bool)
- bit 8-15: stereoSep (int)

## 0x81: Amiga

- bit 0: clockSel (int)
  - 0: NTSC
  - 1: PAL
- bit 1: chipType (int)
  - 0: Amiga 500
  - 1: Amiga 1200
- bit 2: bypassLimits (bool)
- bit 8-14: stereoSep (int)

## 0x82: YM2151 alone

- bit 0-7: clockSel (int)
  - 0: NTSC
  - 1: PAL
  - 2: 4MHz

## 0x83: YM2612 alone, 0xa0: YM2612 extended, 0xbd: YM2612 extra features extended and 0xbe: YM2612 extra features

- bit 31: ladderEffect (bool)
- bit 0-30: clockSel (int)
  - 0: NTSC
  - 1: PAL
  - 2: 8MHz
  - 3: Firecore (`COLOR_NTSC*12/7`)
  - 4: System 32 (`COLOR_NTSC*9/4`)

## 0x84: TIA

- bit 0: clockSel (int)
  - 0: NTSC
  - 1: PAL
- bit 1-2: mixingType (int)
  - 0: mono
  - 1: mono (no distortion)
  - 2: stereo

## 0x85: VIC-20

- bit 0: clockSel (int)
  - 0: NTSC
  - 1: PAL

## 0x87: SNES

- bit 0-6: volScaleL (int)
- bit 8-14: volScaleR (int)

## 0x89: OPLL (YM2413) and 0xa7: OPLL drums (YM2413)

- bit 0-3: clockSel (int)
  - 0: NTSC
  - 1: PAL
  - 2: 4MHz
  - 3: half NTSC
- bit 4-31: patchSet (int)
  - 0: YM2413
  - 1: YMF281
  - 2: YM2423
  - 3: VRC7

## 0x8c: Namco 163

- bit 0-3: clockSel (int)
  - 0: NTSC (2A03)
  - 1: PAL (2A07)
  - 2: Dendy
- bit 4-6: channels (int)
- bit 7: multiplex (bool)

## 0x8d: YM2203 and 0xb6: YM2203 extended

- bit 0-4: clockSel (int)
  - 0: NTSC
  - 1: PAL
  - 2: 4MHz
  - 3: 3MHz
  - 4: 3.99MHz
  - 5: 1.5MHz
- bit 5-6: prescale (int)
  - 0: /6
  - 1: /3
  - 2: /2

## 0x8e: YM2608 and 0xb7: YM2608 extended

- bit 0-4: clockSel (int)
  - 0: 8MHz
  - 1: 7.98MHz
- bit 5-6: prescale (int)
  - 0: /6
  - 1: /3
  - 2: /2

## 0x8f: OPL (YM3526), 0xa2: OPL drums (YM3526), 0x90: OPL2 (YM3812), 0xa3: OPL2 drums (YM3812), 0xb2: Yamaha Y8950 and 0xb3: Yamaha Y8950 drums

- bit 0-7: clockSel (int)
  - 0: NTSC
  - 1: PAL
  - 2: 4MHz
  - 3: 3MHz
  - 4: 3.99MHz
  - 5: 3.5MHz

## 0x91: OPL3 (YMF262) and 0xa4: OPL3 drums (YMF262)

- bit 0-7: clockSel (int)
  - 0: NTSC
  - 1: PAL
  - 2: 14MHz
  - 3: 16MHz
  - 4: 15MHz

## 0x93: Intel 8253 (beeper)

- bit 0-1: speakerType (int)
  - 0: unfiltered
  - 1: cone
  - 2: piezo
  - 3: system

## 0x95: RF5C68

- bit 0-3: clockSel (int)
  - 0: 8MHz
  - 1: 10MHz
  - 2: 12.5MHz
- bit 4-31: chipType (int)
  - 0: RF5C68
  - 1: RF5C164

## 0x97: Philips SAA1099

- flags: clockSel (int)
  - 0: 8MHz
  - 1: NTSC
  - 2: PAL

## 0x98: OPZ (YM2414)

- flags: clockSel (int)
  - 0: NTSC
  - 1: pseudo-PAL
  - 2: 4MHz

## 0x9a: AY8930

- bit 0-3: clockSel (int)
  - 0: NTSC
  - 1: PAL
  - 2: ZX Spectrum 48K (1.75MHz)
  - 3: 2MHz
  - 4: 1.5MHz
  - 5: 1MHz
  - 6: Sunsoft 5B
  - 7: PAL NES
  - 8: Sunsoft 5B on PAL NES
  - 9: 1.10MHz
  - 10: 2^21MHz
  - 11: double NTSC
  - 12: 3.6MHz
- bit 6: stereo (bool)
- bit 7: halfClock (bool)
- bit 8-15: stereoSep (int)

## 0x9d: VRC7

- bit 0-3: clockSel (int)
  - 0: NTSC
  - 1: PAL
  - 2: 4MHz
  - 3: half NTSC

## 0x9f: ZX Spectrum (beeper)

- bit 0-1: clockSel (int)
  - 0: NTSC
  - 1: PAL

## 0xa1: Konami SCC and 0xb4: Konami SCC+

- bit 0-6: clockSel (int)
  - 0: NTSC
  - 1: PAL
  - 2: 1.5MHz
  - 3: 2MHz

## 0xaa: MSM6295

- bit 0-6: clockSel (int)
  - 0: 1MHz
  - 1: 1.056MHz
  - 2: 4MHz
  - 3: 4.224MHz
  - 4: NTSC
  - 5: half NTSC
  - 6: 2/7 NTSC
  - 7: quarter NTSC
  - 8: 2MHz
  - 9: 2.112MHz
  - 10: 875KHz
  - 11: 937.5KHz
  - 12: 1.5MHz
  - 13: 3MHz
  - 14: 1/3 NTSC
- bit 7: rateSel (bool)

## 0xab: MSM6258

- flags: clockSel (int)
  - 0: 4MHz
  - 1: 4.096MHz
  - 2: 8MHz
  - 3: 8.192MHz

## 0xae: OPL4 (YMF278B) and 0xaf: OPL4 drums (YMF278B)

- bit 0-7: clockSel (int)
  - 0: NTSC
  - 1: PAL
  - 2: 33.8688MHz

## 0xb0: Seta/Allumer X1-010

- bit 0-3: clockSel (int)
  - 0: 16MHz
  - 1: 16.67MHz
- bit 4: stereo (bool)

## 0xb5: tildearrow Sound Unit

- bit 0: clockSel (int)
  - 0: NTSC
  - 1: PAL
- bit 2: echo (bool)
- bit 3: swapEcho (bool)
- bit 4: sampleMemSize (int)
  - 0: 8K
  - 1: 64K
- bit 5: pdm (bool)
- bit 8-13: echoDelay (int)
- bit 16-19: echoFeedback (int)
- bit 20-23: echoResolution (int)
- bit 24-31: echoVol (int)

## 0xb8: YMZ280B

- bit 0-7: clockSel (int)
  - 0: 16.9344MHz
  - 1: NTSC
  - 2: PAL
  - 3: 16MHz
  - 4: 16.67MHz
  - 5: 14MHz

## 0xc0: PCM DAC

- bit 0-15: rate (int)
  - add +1 to this value
- bit 16-19: outDepth (int)
- bit 20: stereo (bool)

## 0xe0: QSound

- bit 0-11: echoDelay (int)
- bit 12-19: echoFeedback (int)
