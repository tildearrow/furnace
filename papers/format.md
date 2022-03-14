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
  1  | time base
  1  | speed 1
  1  | speed 2
  1  | initial arpeggio time
  4f | ticks per second
     | - 60 is NTSC
     | - 50 is PAL
  2  | pattern length
  2  | orders length
  1  | highlight A
  1  | highlight B
  2  | instrument count
  2  | wavetable count
  2  | sample count
  4  | pattern count
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
     |   - bit 6 enables alternate mode:
     |     - 0x42: Genesis extended - 13 channels
     |     - 0x43: SMS (SN76489) + OPLL (YM2413) - 13 channels (compound!)
     |     - 0x46: NES + VRC7 - 11 channels (compound!)
     |     - 0x47: C64 (6581) - 3 channels
     |     - 0x49: Neo Geo CD extended - 16 channels
     |   - bit 7 for non-DefleMask chips:
     |     - 0x80: AY-3-8910 - 3 channels
     |     - 0x81: Amiga - 4 channels
     |     - 0x82: YM2151 alone - 8 channels
     |     - 0x83: YM2612 alone - 6 channels
     |     - 0x84: TIA - 2 channels
     |     - 0x85: VIC-20 - 4 channels
     |     - 0x86: PET - 1 channel
     |     - 0x87: SNES - 8 channels
     |     - 0x88: VRC6 - 3 channels
     |     - 0x89: OPLL (YM2413) - 9 channels
     |     - 0x8a: FDS - 1 channel
     |     - 0x8b: MMC5 - 3 channels
     |     - 0x8c: Namco 163 - 8 channels
     |     - 0x8d: OPN (YM2203) - 6 channels
     |     - 0x8e: PC-98 (YM2608) - 16 channels
     |     - 0x8f: OPL (YM3526) - 9 channels
     |     - 0x90: OPL2 (YM3812) - 9 channels
     |     - 0x91: OPL3 (YMF262) - 18 channels
     |     - 0x92: MultiPCM - 24 channels
     |     - 0x93: Intel 8253 (beeper) - 1 channel
     |     - 0x94: POKEY - 4 channels
     |     - 0x95: RF5C68 - 8 channels
     |     - 0x96: WonderSwan - 4 channels
     |     - 0x97: Philips SAA1099 - 6 channels
     |     - 0x98: OPZ (YM2414) - 8 channels
     |     - 0x99: Pokémon Mini - 1 channel
     |     - 0x9a: AY8930 - 3 channels
     |     - 0x9b: SegaPCM - 16 channels
     |     - 0x9c: Virtual Boy - 6 channels
     |     - 0x9d: VRC7 - 6 channels
     |     - 0x9e: YM2610B - 16 channels
     |     - 0x9f: ZX Spectrum (beeper) - 6 channels
     |     - 0xa0: YM2612 extended - 9 channels
     |     - 0xa1: Konami SCC - 5 channels
     |     - 0xa2: OPL drums (YM3526) - 11 channels
     |     - 0xa3: OPL2 drums (YM3812) - 11 channels
     |     - 0xa4: OPL3 drums (YMF262) - 20 channels
     |     - 0xa5: Neo Geo (YM2610) - 14 channels
     |     - 0xa6: Neo Geo extended (YM2610) - 17 channels
     |     - 0xa7: OPLL drums (YM2413) - 11 channels
     |     - 0xa8: Atari Lynx - 4 channels
     |     - 0xa9: SegaPCM (for Deflemask Compatibility) - 5 channels
     |     - 0xaa: MSM6295 - 4 channels
     |     - 0xab: MSM6258 - 1 channel
     |     - 0xac: Commander X16 (VERA) - 17 channels
     |     - 0xb0: Seta/Allumer X1-010 - 16 channels
     |     - 0xde: YM2610B extended - 19 channels
     |     - 0xe0: QSound - 19 channels
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
  1  | reserved
 4?? | pointers to instruments
 4?? | pointers to wavetables
 4?? | pointers to samples
 4?? | pointers to patterns
 ??? | orders
     | - a table of bytes
     | - size=channels*ordLen
     | - read orders then channels
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
 STR | song comment
  4f | master volume, 1.0f=100% (>=59)
     | this is 2.0f for modules before 59
```

# instrument

```
size | description
-----|------------------------------------
  4  | "INST" block ID
  4  | reserved
  2  | format version (see header)
  1  | instrument type
     | - 0: standard
     | - 1: FM
     | - 2: Game Boy
     | - 3: C64
     | - 4: Amiga/sample
  1  | reserved
 STR | instrument name
 --- | **FM instrument data**
  1  | alg
  1  | feedback
  1  | fms
  1  | ams
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
  1  | dam
  1  | dvb
  1  | egt
  1  | ksl
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
 14  | reserved
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
 4?? | arp macro
     | - before version 31, this macro's values were stored offset by 12.
 4?? | duty macro
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
     | - 4: QSound ADPCM
     | - 5: ADPCM-A
     | - 6: ADPCM-B
     | - 7: X68000 ADPCM
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
  4  | reserved
 ??? | pattern data
     | - size: rows*(4+effectColumns*2)*2
     | - read shorts in this order:
     |   - note
     |   - octave
     |   - instrument
     |   - volume
     |   - effect and effect data...
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
