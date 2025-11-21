# the Furnace file format (.fur)

this document has the goal of describing the file format used by Furnace for loading and saving songs.

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

- 228: Furnace 0.6.8.1
- 227: Furnace 0.6.8
- 226: Furnace 0.6.8pre2
- 225: Furnace 0.6.8pre1
- 219: Furnace 0.6.7
- 218: Furnace 0.6.6
- 214: Furnace 0.6.5
- 212: Furnace 0.6.4
- 201: Furnace 0.6.3
- 197: Furnace 0.6.2
- 192: Furnace 0.6.1
- 181: Furnace 0.6
- 180: Furnace 0.6pre18
- 179: Furnace 0.6pre17
- 178: Furnace 0.6pre16
- 177: Furnace 0.6pre15
- 175: Furnace 0.6pre14
- 174: Furnace 0.6pre13
- 173: Furnace 0.6pre12
- 172: Furnace 0.6pre11
- 171: Furnace 0.6pre10
- 169: Furnace 0.6pre9
- 166: Furnace 0.6pre8
- 162: Furnace 0.6pre7
- 161: Furnace 0.6pre6
- 158: Furnace 0.6pre5
- 146: Furnace Pro (joke version)
- 143: Furnace 0.6pre4
- 141: Furnace Tournament Edition (for intro tune contest)
- 133: Furnace 0.6pre3
- 132: Furnace 0.6pre2
- 116: Furnace 0.6pre1.5
- 100: Furnace 0.6pre1
- 75: Furnace dev75/April Fools' 0.6pre0

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

versions that do not appear in this list are `dev???` ones.

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

# song info (>=240)

```
size | description
-----|------------------------------------
  4  | "INF2" block ID
  4  | size of this block
 --- | **song information**
 STR | song name
 STR | song author
 STR | system name
 STR | album/category/game name
 STR | song name (Japanese)
 STR | song author (Japanese)
 STR | system name (Japanese)
 STR | album/category/game name (Japanese)
  4f | A-4 tuning
  1  | automatic system name
 --- | **system definition**
  4f | master volume, 1.0f=100%
  2  | total number of channels
  2  | number of chips
 --- | **chip definition (×numChips)**
  2  | chip ID
  2  | chip channel count
  4f | chip volume
  4f | chip panning
  4f | chip front/rear balance
 --- | **patchbay**
  4  | patchbay connection count
 4?? | patchbay
     | - see next section for more details.
  1  | automatic patchbay
 --- | **song elements (repeated until element type is 0)**
  1  | element type
  4  | number of elements
 4?? | pointers to elements (×numElements)
```

## list of sound chips

this is a list of sound chips, and their nominal channel count.
the channel count is stored in the file in order to allow Furnace to load files with unsupported chips from derivatives of Furnace or newer versions.

- 0x00: invalid (end of chips in previous versions)
- 0x01: YMU759 - 17 channels
- 0x03: SN76489/Sega PSG - 4 channels
- 0x04: Game Boy - 4 channels
- 0x05: PC Engine - 6 channels
- 0x06: NES - 5 channels
- 0x07: C64 (8580) - 3 channels
- 0x47: C64 (6581) - 3 channels
- 0x80: AY-3-8910 - 3 channels
- 0x81: Amiga - 4 channels
- 0x82: YM2151 - 8 channels
- 0x83: YM2612 - 6 channels
- 0x84: TIA - 2 channels
- 0x85: VIC-20 - 4 channels
- 0x86: PET - 1 channel
- 0x87: SNES - 8 channels
- 0x88: VRC6 - 3 channels
- 0x89: OPLL (YM2413) - 9 channels
- 0x8a: FDS - 1 channel
- 0x8b: MMC5 - 3 channels
- 0x8c: Namco 163 - 8 channels
- 0x8d: YM2203 - 6 channels
- 0x8e: YM2608 - 16 channels
- 0x8f: OPL (YM3526) - 9 channels
- 0x90: OPL2 (YM3812) - 9 channels
- 0x91: OPL3 (YMF262) - 18 channels
- 0x92: MultiPCM - 28 channels
- 0x93: Intel 8253 (beeper) - 1 channel
- 0x94: POKEY - 4 channels
- 0x95: RF5C68 - 8 channels
- 0x96: WonderSwan - 4 channels
- 0x97: Philips SAA1099 - 6 channels
- 0x98: OPZ (YM2414) - 8 channels
- 0x99: Pokémon Mini - 1 channel
- 0x9a: AY8930 - 3 channels
- 0x9b: SegaPCM - 16 channels
- 0x9c: Virtual Boy - 6 channels
- 0x9d: VRC7 - 6 channels
- 0x9e: YM2610B - 16 channels
- 0x9f: ZX Spectrum (beeper, SFX-like tildearrow engine) - 6 channels
- 0xa0: YM2612 extended - 9 channels
- 0xa1: Konami SCC - 5 channels
- 0xa2: OPL drums (YM3526) - 11 channels
- 0xa3: OPL2 drums (YM3812) - 11 channels
- 0xa4: OPL3 drums (YMF262) - 20 channels
- 0xa5: Neo Geo (YM2610) - 14 channels
- 0xa6: Neo Geo extended (YM2610) - 17 channels
- 0xa7: OPLL drums (YM2413) - 11 channels
- 0xa8: Atari Lynx - 4 channels
- 0xaa: MSM6295 - 4 channels
- 0xab: MSM6258 - 1 channel
- 0xac: Commander X16 (VERA) - 17 channels
- 0xad: Bubble System WSG - 2 channels
- 0xae: OPL4 (YMF278B) - 42 channels
- 0xaf: OPL4 drums (YMF278B) - 44 channels
- 0xb0: Seta/Allumer X1-010 - 16 channels
- 0xb1: Ensoniq ES5506 - 32 channels
- 0xb2: Yamaha Y8950 - 10 channels
- 0xb3: Yamaha Y8950 drums - 12 channels
- 0xb4: Konami SCC+ - 5 channels
- 0xb5: tildearrow Sound Unit - 8 channels
- 0xb6: YM2203 extended - 9 channels
- 0xb7: YM2608 extended - 19 channels
- 0xb8: YMZ280B - 8 channels
- 0xb9: Namco WSG - 3 channels
- 0xba: Namco C15 - 8 channels
- 0xbb: Namco C30 - 8 channels
- 0xbc: MSM5232 - 8 channels
- 0xbd: YM2612 DualPCM extended - 11 channels
- 0xbe: YM2612 DualPCM - 7 channels
- 0xbf: T6W28 - 4 channels
- 0xc0: PCM DAC - 1 channel
- 0xc1: YM2612 CSM - 10 channels
- 0xc2: Neo Geo CSM (YM2610) - 18 channels
- 0xc3: YM2203 CSM - 10 channels
- 0xc4: YM2608 CSM - 20 channels
- 0xc5: YM2610B CSM - 20 channels
- 0xc6: K007232 - 2 channels
- 0xc7: GA20 - 4 channels
- 0xc8: SM8521 - 3 channels
- 0xc9: M114S - 16 channels (UNAVAILABLE)
- 0xca: ZX Spectrum (beeper, QuadTone engine) - 5 channels
- 0xcb: Casio PV-1000 - 3 channels
- 0xcc: K053260 - 4 channels
- 0xcd: TED - 2 channels
- 0xce: Namco C140 - 24 channels
- 0xcf: Namco C219 - 16 channels
- 0xd0: Namco C352 - 32 channels (UNAVAILABLE)
- 0xd1: ESFM - 18 channels
- 0xd2: Ensoniq ES5503 (hard pan) - 32 channels (UNAVAILABLE)
- 0xd4: PowerNoise - 4 channels
- 0xd5: Dave - 6 channels
- 0xd6: NDS - 16 channels
- 0xd7: Game Boy Advance (direct) - 2 channels
- 0xd8: Game Boy Advance (MinMod) - 16 channels
- 0xd9: Bifurcator - 4 channels
- 0xda: SCSP - 32 channels (UNAVAILABLE)
- 0xdb: YMF271 (OPX) - 48 channels (UNAVAILABLE)
- 0xdc: RF5C400 - 32 channels (UNAVAILABLE)
- 0xdd: YM2612 XGM - 9 channels (UNAVAILABLE)
- 0xde: YM2610B extended - 19 channels
- 0xdf: YM2612 XGM extended - 13 channels (UNAVAILABLE)
- 0xe0: QSound - 19 channels
- 0xe1: PS1 - 24 channels (UNAVAILABLE)
- 0xe2: C64 (6581) with PCM - 4 channels
- 0xe3: Watara Supervision - 4 channels
- 0xe5: µPD1771C-017 - 4 channels
- 0xf0: SID2 - 3 channels
- 0xf1: 5E01 - 5 channels
- 0xf5: SID3 - 7 channels
- 0xfc: Pong - 1 channel
- 0xfd: Dummy System - 8 channels
- 0xfe: reserved for development
- 0xff: reserved for development

notes:

- (UNAVAILABLE) means that the chip hasn't been implemented in Furnace yet.

### special IDs

the following is a list of legacy chip/system IDs.
these must be either flattened (converted to two equivalent chips) or converted to another chip.

those marked with `(compound!)` were from an era where two chips were part of a single system ID.

these IDs will never be present in new song files (with the `INF2` info header). if you see them, reject the file.

- 0x02: Genesis - 10 channels (compound!)
  - flatten to 0x83 (YM2612) + 0x03 (SN76489/Sega PSG)
- 0x08: Arcade (YM2151+SegaPCM) - 13 channels (compound!)
  - flatten to 0x82 (YM2151) + 0x9b (SegaPCM)
  - channel count of the latter shall be 5!
- 0x09: Neo Geo CD (YM2610) - 13 channels
  - convert to 0xa5 (YM2610 proper) and set channel count to 13
- 0x42: Genesis extended - 13 channels (compound!)
  - flatten to 0xa0 (YM2612 extended) + 0x03 (SN76489/Sega PSG)
- 0x43: SMS (SN76489) + OPLL (YM2413) - 13 channels (compound!)
  - flatten to 0x03 (SN76489/Sega PSG) + 0x89 (OPLL)
- 0x46: NES + VRC7 - 11 channels (compound!)
  - flatten to 0x06 (NES) + 0x9d (VRC7)
- 0x49: Neo Geo CD extended - 16 channels
  - convert to 0xa6 (YM2610 extended proper) and set channel count to 16
- 0xa9: SegaPCM (for DefleMask compatibility) - 5 channels
  - convert to 0x9b (SegaPCM) and set channel count to 5

## song elements

the following element types are available:

```
 ## |  ID  | description
----|------|-----------------------------
 00 | ---- | end of element list (end of info header)
 01 | SNG2 | sub-song
 02 | FLAG | chip flags
 03 | ADIR | asset directory**
 04 | INS2 | instrument
 05 | WAVE | wavetable
 06 | SMP2 | sample
 07 | PATN | pattern
 08 | CFLG | compatibility flags*
 09 | CMNT | song comments*
 0a | GROV | groove pattern

* element is unique (number of elements shall be 1)

** first pointer is for instruments, second for wavetables and third for samples
```

# patchbay

Furnace dev135 adds a "patchbay" which allows for arbitrary connection of chip outputs to system outputs.
it also allows connecting outputs to effects and so on.

a connection is represented as an unsigned int in the following format:

- bit 16-31: source port
- bit 0-15: destination port

a port is in the following format (hexadecimal): `xxxy`

- `xxx` (bit 4 to 15) represents a portset.
- `y` (bit 0 to 3) is the port in that portset.

reserved input portsets:
- `000`: system outputs
- `FFF`: "null" portset

reserved output portsets:
- `000` through `chipCount`: chip outputs
- `FFC`: reference file/music player (>=238)
- `FFD`: wave/sample preview
- `FFE`: metronome
- `FFF`: "null" portset

# subsong (>=240)

```
size | description
-----|------------------------------------
  4  | "SNG2" block ID
  4  | size of this block
  4f | ticks per second
     | - 60 is NTSC
     | - 50 is PAL
  1  | initial arpeggio speed
  1  | effect speed divider
  2  | pattern length
     | - the limit is 256.
  2  | orders length
     | - the limit is 256.
  1  | highlight A (rows per beat)
  1  | highlight B (rows per bar)
  2  | virtual tempo numerator
  2  | virtual tempo denominator
  1  | length of speed pattern in entries (fail if this is lower than 1 or higher than 16)
 2?? | speed pattern (always 16 entries)
     | - each speed is an unsigned short
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
 4?? | channel colors
     | - read 4 values per color (RGBA)
     | - if 0, use default color
```

# groove pattern (>=240)

```
size | description
-----|------------------------------------
  4  | "GROV" block ID
  4  | size of this block
  1  | length of groove in entries (fail if this is lower than 1 or higher than 16)
 2?? | groove pattern (always 16 entries)
     | - each speed is an unsigned short
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

# asset directories (>=156)

also known as "folder" in the user interface.

```
size | description
-----|------------------------------------
  4  | "ADIR" block ID
  4  | size of this block
  4  | number of directories
 --- | **asset directory** (×numberOfDirs)
 STR | name (if empty, this is the uncategorized directory)
  2  | number of assets
 1?? | assets in this directory
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
     | - 7: K05 ADPCM
     | - 8: 8-bit PCM
     | - 9: BRR (SNES)
     | - 10: VOX
     | - 11: 8-bit μ-law PCM
     | - 12: C219 PCM
     | - 13: IMA ADPCM
     | - 14: 12-bit PCM (MultiPCM)
     | - 16: 16-bit PCM
  1  | loop direction (>=123) or reserved
     | - 0: forward
     | - 1: backward
     | - 2: ping-pong
  1  | flags (>=129) or reserved
     | - 0: BRR emphasis
  1  | flags 2 (>=159) or reserved
     | - 0: dither
     | - 1: no BRR filters (>=213)
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

# pattern (>=157)

```
size | description
-----|------------------------------------
  4  | "PATN" block ID
  4  | size of this block
  1  | subsong
  1  | channel (<240)
  2  | channel (>=240)
     | - the channel index was 8-bit in previous versions.
     | - in order to accommodate higher channel counts, it has been extended to 16-bit.
  2  | pattern index
 STR | pattern name (>=51)
 ??? | pattern data
     | - read a byte per row.
     | - if it is 0xff, end of data. the rest of the pattern is empty.
     | - if bit 7 is set, then skip N+2 rows. N is bits 0-6.
     |   - for example, $80 means skip 2 rows, $81 means skip 3, $82 means 4 and so on.
     | - if bit 7 is clear, then:
     |   - bit 0: note present
     |   - bit 1: ins present
     |   - bit 2: volume present
     |   - bit 3: effect 0 present
     |   - bit 4: effect value 0 present
     |   - bit 5: other effects (0-3) present
     |   - bit 6: other effects (4-7) present
     |   - if none of these bits are set, then skip 1 row.
     | - if bit 5 is set, read another byte:
     |   - bit 0: effect 0 present
     |   - bit 1: effect value 0 present
     |   - bit 2: effect 1 present
     |   - bit 3: effect value 1 present
     |   - bit 4: effect 2 present
     |   - bit 5: effect value 2 present
     |   - bit 6: effect 3 present
     |   - bit 7: effect value 3 present
     | - if bit 6 is set, read another byte:
     |   - bit 0: effect 4 present
     |   - bit 1: effect value 4 present
     |   - bit 2: effect 5 present
     |   - bit 3: effect value 5 present
     |   - bit 4: effect 6 present
     |   - bit 5: effect value 6 present
     |   - bit 6: effect 7 present
     |   - bit 7: effect value 7 present
     | - then read note, ins, volume, effects and effect values depending on what is present.
     | - for note:
     |   - 0 is C-(-5)
     |   - 179 is B-9
     |   - 180 is note off
     |   - 181 is note release
     |   - 182 is macro release
```

---

# old format blocks

these were present in previous versions of the Furnace file format.

## old song info (<240)

hic sunt dracones!

this info block is messy because it was devised during Furnace's early days, back when it didn't support sub-songs.
the first sub-song is defined here. compatibility flags are all over the place.

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
 32  | list of sound chip IDs
     | - 0x00 means "end of chip list". you must read 32 bytes regardless.
     | - see list at the top of this file for the list of sound chips.
 32  | sound chip volumes (<135) or reserved
     | - signed char, 64=1.0, 127=~2.0
     | - as of version 135 these fields only exist for compatibility reasons.
 32  | sound chip panning (<135) or reserved
     | - signed char, -128=left, 127=right
     | - as of version 135 these fields only exist for compatibility reasons.
 128 | sound chip flag pointers (>=119) or sound chip flags
     | - before 118, these were 32-bit flags.
     | - for conversion details, see oldflags.md.
 STR | song name
 STR | song author
  4f | A-4 tuning
  1  | limit slides (>=36) or reserved
  1  | linear pitch (>=36) or reserved
     | - 0: non-linear
     | - 1: only pitch change (04xy/E5xx) linear (<237) - full linear (>=237)
     |   - partial pitch linearity removed in 237
     | - 2: full linear (>=94, <237)
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
 --- | **extra chip output settings (× chipCount)** (>=135)
  4f | chip volume
  4f | chip panning
  4f | chip front/rear balance
 --- | **patchbay** (>=135)
  4  | patchbay connection count
 4?? | patchbay
     | - see next section for more details.
  1  | automatic patchbay (>=136)
 --- | **a couple more compat flags** (>=138)
  1  | broken portamento during legato
  1  | broken macro during note off in some FM chips (>=155)
  1  | pre note (C64) does not compensate for portamento or legato (>=168)
  1  | disable new NES DPCM features (>=183)
  1  | reset arp effect phase on new note (>=184)
  1  | linear volume scaling rounds up (>=188)
  1  | legacy "always set volume" behavior (>=191)
  1  | legacy sample offset effect (>=200)
 --- | **speed pattern of first song** (>=139)
  1  | length of speed pattern (fail if this is lower than 1 or higher than 16)
 16  | speed pattern (this overrides speed 1 and speed 2 settings)
 --- | **groove list** (>=139)
  1  | number of entries
 ??? | groove entries. the format is:
     | - 1 byte: length of groove
     | - 16 bytes: groove pattern
 --- | **pointers to asset directories** (>=156)
  4  | instrument directories
  4  | wavetable directories
  4  | sample directories
```

# old subsong (<240)

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
 --- | **speed pattern** (>=139)
  1  | length of speed pattern (fail if this is lower than 1 or higher than 16)
 16  | speed pattern (this overrides speed 1 and speed 2 settings)
```


## old instrument (<127)

instruments in older versions of Furnace used a different format. see [oldIns.md](oldIns.md) for more information.

### C64 compatibility note (>=187)

in Furnace dev187 the volume and cutoff macros have been separated, as noted above.
however, there are two other changes as well: **inverted relative (non-absolute) cutoff macro**; and a new, improved Special macro.

if version is less than 187, you must convert the Special macro:
1. do not continue if ex4 is not a Sequence type macro!
2. move bit 0 of ex4 macro data into bit 3.
3. set bit 0 on all steps of ex4 macro to 1.
4. if ex3 is not a Sequence type macro, stop here.
5. if ex3 macro length is 0, stop here.
6. merge the ex3 macro (former Special) into ex4 (former Test).
  - use the largest size (between ex3 and ex4).
  - if the ex3 macro is shorter than the ex4 one, use the last value of ex3, and vice-versa.
  - if the ex4 macro length is 0, expand it to the largest size, and set all steps to 1.

don't worry about loop or release...

## old sample (<102)

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

## old pattern (<157)

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
     |     - 101: note release
     |     - 102: macro release
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
