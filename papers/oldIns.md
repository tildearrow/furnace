# old instrument format (<127)

this format is used in older versions of Furnace.

# header

.fui files use the following header:

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

this header is not present on instruments inside a .fur file.

# data

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
  1  | vol macro is cutoff (<187) or reserved
     | - from version 187 onwards, volume and cutoff macros are separate.
     | - if this is on and the version is less than 187, move the volume macro into the ALG one.
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

