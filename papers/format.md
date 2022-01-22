# the Furnace file format (.fur)

while Furnace works directly with the .dmf format, I had to create a new format to handle future additions to the program.

this document has the goal of detailing the format.

# header

the header is 32 bytes long.

size | description
-----|------------------------------------
 16  | "-Furnace module-" format magic
  2  | format version
     | - should be 17 for Furnace 0.4
  2  | reserved
  4  | song info pointer
  8  | reserved

# song info

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
     |   - 0x02: Genesis - 10 channels
     |   - 0x03: SMS (SN76489) - 4 channels
     |   - 0x04: Game Boy - 4 channels
     |   - 0x05: PC Engine - 6 channels
     |   - 0x06: NES - 5 channels
     |   - 0x07: C64 (8580) - 3 channels
     |   - 0x08: Arcade (YM2151) - 13 channels
     |   - 0x09: Neo Geo (YM2610) - 13 channels
     |   - bit 6 enables alternate mode:
     |     - 0x42: Genesis extended - 13 channels
     |     - 0x47: C64 (6581) - 3 channels
     |     - 0x49: Neo Geo extended - 16 channels
     |   - bit 7 for non-DefleMask chips:
     |     - 0x80: AY-3-8910 - 3 channels
     |     - 0x81: Amiga - 4 channels
     |     - 0x82: YM2151 alone - 8 channels
     |     - 0x83: YM2612 alone - 6 channels
     |     - 0x84: TIA - 2 channels
     |     - 0x97: Philips SAA1099 - 6 channels
     |     - 0x9a: AY-3-8930 - 3 channels
 32  | sound chip volumes
     | - signed char, 64=1.0, 127=~2.0
 32  | sound chip panning
     | - signed char, -128=left, 127=right
 128 | sound chip parameters (TODO)
 ??? | song name
 ??? | song author
 24  | reserved for compatibility flags
 4?? | pointers to instruments
 4?? | pointers to wavetables
 4?? | pointers to samples
 4?? | pointers to patterns
 ??? | orders
     | - a table of shorts
     | - size=channels*ordLen
     | - read orders than channels
 ??? | effect columns
     | - size=channels

# instrument

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
 ??? | instrument name
 --- | **FM instrument data**
  1  | alg
  1  | feedback
  1  | fms
  1  | ams
  1  | operator count (always 4)
  3  | reserved
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

# wavetable

size | description
-----|------------------------------------
  4  | "WAVE" block ID
  4  | reserved
 ??? | wavetable name
  4  | wavetable size
  4  | wavetable min
  4  | wavetable max
 4?? | wavetable data

# sample

size | description
-----|------------------------------------
  4  | "SMPL" block ID
  4  | reserved
 ??? | sample name
  4  | length
  4  | rate
  2  | volume
  2  | pitch
  1  | depth
  3  | reserved
  4  | loop point (>=19)
     | - -1 means no loop
 2?? | sample data (always 16-bit)

# pattern

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

# the Furnace instrument format (.fui)

the instrument format is pretty similar to the file format, but it also stores wavetables and samples used by the instrument.

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

instrument data follows.

# the Furnace wavetable format (.fuw)

similar to the instrument format...

size | description
-----|------------------------------------
 16  | "-Furnace waveta-" format magic
  2  | format version
  2  | reserved

wavetable data follows.
