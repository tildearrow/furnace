# the Furnace file format (.fur)

while Furnace works directly with the .dmf format, I had to create a new format to handle future additions to the program.

this document has the goal of detailing the format.

# header

the header is 32 bytes long.

size | description
-----|------------------------------------
 16  | "-Furnace module-" format magic
  2  | format version
     | - should be 1 for Furnace 0.2
  2  | reserved
  4  | song info pointer
  8  | reserved

# song info

size | description
-----|------------------------------------
  4  | "INFO" block ID
  4  | length of this block
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
  1  | sound chip count
 31  | list of sound chips
     | - possible soundchips:
     |   - 0x00: invalid - 0 channels
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
  4  | reserved
 124 | sound chip parameters (TODO)
  4  | pointer song name
  4  | pointer to song author
 24  | reserved for compatibility flags
 4?? | pointers to instruments
 4?? | pointers to wavetables
 4?? | pointers to samples
 4?? | pointers to patterns
 ??? | orders
     | - a table of shorts
     | - size=channels*ordLen
 ??? | effect columns
     | - size=channels

# instrument

size | description
-----|------------------------------------
  4  | "INST" block ID
  4  | length of this block
  2  | format version (see header)
  1  | instrument type
     | - 0: standard
     | - 1: FM
     | - 2: Game Boy
     | - 3: C64
     | - 4: Amiga/sample
  1  | reserved
  4  | pointer to instrument name
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
  4  | volume macro loop
  4  | arp macro loop
  4  | duty macro loop
  4  | wave macro loop
  1  | arp macro mode
  3  | reserved
 4?? | volume macro
 4?? | arp macro
 4?? | duty macro
 4?? | wave macro

# wavetable

size | description
-----|------------------------------------
  4  | "WAVE" block ID
  4  | length of this block
  4  | pointer to wavetable name
  4  | wavetable size
  4  | wavetable min
  4  | wavetable max
 4?? | wavetable data

# sample

size | description
-----|------------------------------------
  4  | "SMPL" block ID
  4  | length of this block
  4  | pointer to sample name
  4  | length
  4  | rate
  2  | volume
  2  | pitch
  1  | depth
  7  | reserved
 2?? | sample data (always 16-bit)

# pattern

size | description
-----|------------------------------------
  4  | "PATR" block ID
  4  | length of this block
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
