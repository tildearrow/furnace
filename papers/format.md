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

# format versions

the format versions are:

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
  4  | reserved
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
     |   - 0x8d: OPN (YM2203) - 6 channels
     |   - 0x8e: PC-98 (YM2608) - 16 channels
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
     |   - 0xb6: OPN extended - 9 channels
     |   - 0xb7: PC-98 extended - 19 channels
     |   - 0xb8: YMZ280B - 8 channels
     |   - 0xde: YM2610B extended - 19 channels
     |   - 0xe0: QSound - 19 channels
     |   - 0xfd: Dummy System - 8 channels
     |   - 0xfe: reserved for development
     |   - 0xff: reserved for development
     | - (compound!) means that the system is composed of two or more chips,
     |   and has to be flattened.
 32  | sound chip volumes
     | - signed char, 64=1.0, 127=~2.0
 32  | sound chip panning
     | - signed char, -128=left, 127=right
 128 | sound chip parameters
 STR | song name
 STR | song author
  4f | A-4 tuning
  1  | limit slides (>=36) or reserved
  1  | linear pitch (>=36) or reserved
     | - 0: non-linaer
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
  1  | E1xx and E2xx also take priority over Slide00 (>=83) or reserved
  1  | new Sega PCM (with macros and proper vol/pan) (>=84) or reserved
  1  | weird f-num/block-based chip pitch slides (>=85) or reserved
  1  | SN duty macro always resets phase (>=86) or reserved
  1  | pitch macro is linear (>=90) or reserved
  1  | pitch slide speed in full linear pitch mode (>=94) or reserved
 14  | reserved
 --- | **virtual tempo data**
  2  | virtual tempo numerator of first song (>=96) or reserved
  2  | virtual tempo denominator of first song (>=96) or reserved
 --- | **additional subsongs** (>=95)
 STR | first subsong name
 STR | first subsong comment
  1  | number of additional subsongs
  3  | reserved
 4?? | pointers to subsong data
```

# subsong

from version 95 onwards, Furnace supports storing multiple songs on a single file.
the way it's currently done is really weird, but it provides for some backwards compatibility (previous versions will only load the first subsong which is already defined in the `INFO` block).

```
size | description
-----|------------------------------------
  4  | "SONG" block ID
  4  | reserved
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

# instrument

notes:

- the entire instrument is stored, regardless of instrument type.
- the macro range varies depending on the instrument type.
- "macro open" indicates whether the macro is collapsed or not in the instrument editor.
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
  4  | reserved
  2  | format version (see header)
  1  | instrument type
     | - 0: standard
     | - 1: FM (OPM/OPN)
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
 12  | reserved
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
  1  | arp macro mode
  1  | reserved (>=17) or volume macro height (>=15) or reserved
  1  | reserved (>=17) or duty macro height (>=15) or reserved
  1  | reserved (>=17) or wave macro height (>=15) or reserved
 4?? | volume macro
     | - before version 87, if this is the C64 relative cutoff macro, its values were stored offset by 18.
 4?? | arp macro
     | - before version 31, this macro's values were stored offset by 12.
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
```

# wavetable

```
size | description
-----|------------------------------------
  4  | "WAVE" block ID
  4  | reserved
 STR | wavetable name
  4  | wavetable size
  4  | wavetable min
  4  | wavetable max
 4?? | wavetable data
```

# sample

```
size | description
-----|------------------------------------
  4  | "SMPL" block ID
  4  | reserved
 STR | sample name
  4  | length
  4  | rate
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
  4  | reserved
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
