# format versions

this document details all format changes during Furnace's history.
it also provides information on applied transformations for the sake of compatibility.

**this is a work in progress!**

## dev251 - OPZ LFO

this version introduces an important change regarding OPZ.
it has been discovered that LFOs are not added together as previously thought - instead, a register bit allows you to select which LFO to use for FMS and AMS. this is a per-channel setting.

additionally, another setting exists to select between two additional "LFO"s for "tremolo" - a hardware feature which essentially provides a second variable-speed volume ramp for each operator.

the field for AMS2 in the instrument format's FM feature is now used to store two bits which select LFOs for FMS and AMS.
the field for FMS2 is now a single-bit toggle between LFO3 and LFO4 for tremolo.

Furnace uses the following criteria to convert FMS2/AMS2 into FMS/AMS/select:

1. get FMS value.
2. get FMS2 value.
3. is FMS2 greater than FMS? if so, set FMS to FMS2 and set FMS LFO select bit. otherwise, discard FMS2.

repeat the same procedure for AMS.

## dev250 - TL ramp

this version contains an initial implementation of TL ramp, a feature found in OPP and OPZ chips that smoothly ramps between one and another TL value with a user-defined speed.

when loading OPM/OPZ instruments from previous versions, set the KSR bit to 0 on all operators. this bit determines whether TL ramp is enabled.

## dev249 - raw frequency mode key

a new note input value has been added to toggle between raw frequency and normal note. by default this is bound to the `-` key.

Furnace will automatically bind the `-` key to raw mode toggle when coming from a previous version. if this key is already bound  the `\` key will be bound instead. if this still is not possible, a warning will be displayed on start-up, informing the user of this new feature.

## dev248 - raw frequency notes

this version prepares for raw frequency mode, where the user may insert a note in raw mode, playing a specific period/frequency value and bypassing frequency calculation.

the pattern format adds a new value for note: `183`. if you encounter this value, read four bytes (a 32-bit little-endian int which contains the raw frequency).

internally, the value `251` is used to indicate a raw note, and the raw value is stored in each row of DivPattern::newData's last four values.

## dev247 - new settings

many users have expressed a need for setting searching, given an overwhelming amount of settings Furnace has.

after several attempts, Eknous has brought us a revamped settings window, with much cleaner code and the ability to search settings.

this version bump is solely due to new settings being a major change.

## dev246 - note refactor

this version changes the way notes are internally stored.

before I introduced negative octaves, notes had a range of C-0 to B-9 (0 to 119).
the addition of negative octaves moved the range to C-(-5) - B-9 (-60 to 119).
these values were consistent between DivChannelState::note, SharedChannel::note and SharedChannel::baseFreq (in linear pitch).

the new pattern format and pattern refactor broke this rule, storing notes from C-(-5) to B-9 as 0 to 179.
additionally, the previous negative range isn't good for pitch calculation and arrays as it relies upon negative values.
sample maps were limited to 120 notes as well.
as of now the notes, as represented in DivChannelState::note, SharedChannel::note and SharedChannel::baseFreq, have a range of 0 to 179, with 0 mapping to C-(-5), 60 being C-0 and 179 meaning B-9.

sample maps in the instrument format now contain 180 entries as opposed to 120.

## dev245 - ADSR/LFO macro behavior

ADSR and LFO macros formerly used an accumulator which goes from 0 to 255. the accumulator is then scaled to the Bottom and Top values.

this however requires a multiplication, which isn't cheap in processors lacking multiplication instructions (such as 6502 and Z80).

ADSR/LFO macros now use a fixed-point accumulator with 8-bit fractional precision. the values for attack, decay, speed and so on directly alter this accumulator, avoiding an expensive multiplication.

to compensate, Furnace will scale these values when loading a previous-version instrument:

- a Furnace bug resulted in inverted ADSR/LFO macros not having full range. compensate for that by setting bottom to `top+((255+(bottom-top)*255)/256)` before you calculate range.
- range is `abs(top-bottom)`.
- for ADSR macros:
  - attack, decay, sus decay and release: `new=old*range/255`
  - sustain: `new=bottom+(((top-bottom)*old)/255)`
- for LFO macros, convert speed depending on the shape:
  - triangle: `new=(range*old)/2`
  - saw: `new=(range*old)/4`
  - square: `new=old*64`

## dev244 - Namco 163 wave position latch

after a poll in the Furnace Discord, it was decided that Namco 163 wave position latch should be on by default.

wave position latch is a feature which turns the wave position effect into an override, disabling the wave position macro and ignoring any wave position settings in the instrument until the latch is cleared with an `FF` value on the aforementioned effect.

if loading a previous version song, set the `posLatch` flag to `false` on each Namco 163 chip.

## dev243 - Namco 163 convenience effects

this version adds two Namco 163 effects for convenience and .ftm import:

- 1Axx: set wave load and play pos (equivalent to 11xx 15xx)
- 1Bxx: set wave load and play length (equivalent to 12xx 16xx)

a small change to the command stream format has been made. the commands for N163 loadpos/loadlen have been retired, and the commands for N163 wave position/length now take in 2 bytes instead of 1.
the first byte is the position/length.
the second byte is a bitset. if bit 0 is set, update playback state. if bit 1 is set, update load state.

## dev242 - OPL4 mixing levels

this version adds four chip flags to OPL4 for mixing levels:

- fmMixL (default 4)
- fmMixR (default 4)
- pcmMixL (default 7)
- pcmMixR (default 7)

when loading an older file, set fmMixL and fmMixR to 7 (previous mixing levels).

## dev241 - fix groove saving

this version fixes groove saving, which broke in dev240.

## dev240 - new info header!

this version introduces the INF2 info header to the Furnace file format, alongside a bunch of other changes.

a new song info header (`INF2`) has been introduced, which:
- cleans up the mess I made when adding sub-songs to Furnace
- allows better forward compatibility and extensibility
- uses 16-bit chip IDs
- moves compatibility flags to another block and is stored as a DivConfig
- stores channel count in the file, allowing chips with dynamic channel count (e.g. Namco 163)

a new sub-song data block has been introduced as well.

TimeBase ("Divider" in the UI) has been REMOVED. it was a DefleMask leftover.
to compensate, the speeds are now 16-bit. older songs will have their speeds converted, but this may fail if you use grooves or change speed mid-song.

dynamic channel count support has been added. the following chips are currently supported:
- Namco 163 (1-8 channels)
- ES5506 (5-32 channels)
- Generic PCM DAC (1-128 channels, with software mixing!)

channel colors have been added (thanks Eknous!).

SegaPCM (compatible 5-channel mode) and Neo Geo CD have been REMOVED. when loading previous files, including .dmf ones, these will have compatible SegaPCM and Neo Geo CD chips converted to normal SegaPCM and YM2610 respectively.
their channel count remains unaltered though. you can fix this by going into the chip manager and clicking the button next to "irregular channel count" in each chip config section.

## dev239 - total extinction of legacy sample mode

this version removes legacy sample mode. files made under previous versions will be converted if legacy sample mode is detected.

legacy sample mode was the original method used to play samples back in the DefleMask and early Furnace days.
it involved use of the `17xx` effect. when this sample mode is enabled, 12 samples are mapped to an octave (from C to B) and their pitches were configurable in the sample editor.
to use more than 12 samples, you had to switch "banks" with the `EBxx` effect.

when Furnace 0.4 came out, the current sample playback method was added. bind samples to instruments, and subsequently play notes using instruments.
this offered greater pitch control compared to legacy sample mode.

the eventual addition of sample maps further rendered legacy sample mode pointless.

as time passed, legacy sample mode remained as a tumor, plaguing every chip implementation and complicating the already-messy code.
this prompted me to remove it. in this version that became a reality.

the following strategy is used to convert legacy sample mode usage to the current one:

1. check whether we have a free instrument slot. if we don't, stop.
2. check which samples are used by instruments of the following types:
  - MSM6258
  - MSM6295
  - ADPCM-A
  - ADPCM-B
  - SegaPCM
  - QSound
  - YMZ280B
  - RF5C68
  - Amiga (Generic Sample)
  - MultiPCM
  - SNES
  - ES5506
  - K007232
  - GA20
  - K053260
  - C140
  - C219
  - NDS
  - GBA DMA
  - GBA MinMod
  - NES
  - Supervision
  - if "Use sample" is enabled:
    - Sound Unit
    - AY-3-8910/SSG
    - AY8930
    - Lynx/Mikey
    - PC Engine
    - X1-010
    - WonderSwan
    - VRC6
  mark samples used by these instruments to exclude them from future checks.
3. for each channel:
  3.0. skip to the next channel if you are:
    - NES/5E01: not in the DPCM channel
    - MMC5/WonderSwan: not in the PCM channel
    - YM2612: not in channel 6
    - YM2610/YM2610B: not in the ADPCM channels
    - YM2612 DualPCM: not in the DualPCM channels
    - YM2608/Y8950: not in the ADPCM channel
    - VRC6: not in the pulse channels
    - PCE/X1-010/AY/AY8930/SegaPCM/MSM6258/MSM6295: keep going
    - on a chip not under this list
  3.1. there are three modes. off, legacy and normal.
  3.2. the initial sample bank is 0. this is a per-channel thing.
  3.3. the initial sample mode is "off", except under these circumstances:
    - NES/5E01/MMC5/YM2608/YM2610/YM2610B/YM2612 DualPCM/SegaPCM/MSM6258/MSM6295/Y8950: legacy
  3.4. the following chips have a legacy toggle:
    - YM2612
    - YM2612 DualPCM
    - PCE
    - X1-010
    - WonderSwan
    - VRC6
  3.5. legacy mode is disabled on note off in these chips:
    - PCE
    - X1-010
    - WonderSwan
  3.6. for each sub-song, scan all patterns in Orders order. left to right, top to bottom.
    - if you already did this pattern, go to the next order.
    - if the pattern doesn't exist, go to the next order.
    - if you find an 17xx effect, check whether the chip has a legacy toggle. in that case, change the sample mode accordingly (0 is off and any other value is legacy mode). remember to erase the effect afterwards!
    - if you find an EBxx effect, change the current sample bank. erase the effect as well.
    - check whether there is an instrument change.
      - if so, check its type. if it's a Generic Sample, an appropriate type for the chip, or the "Use sample" option, enter normal sample mode.
    - if there is a note, and the sample mode is legacy:
      - initialize the "Legacy Samples" instrument (if you haven't).
      - set the instrument to the Legacy Samples one.
      - check the involved sample (12×sampleBank+note).
        - if it's not used by any instrument, set the sample's C-4 rate to its legacy rate.
    - if there is a note off, and note off disables sample mode, set the sample mode to off.
4. the following procedure initializes the "Legacy Samples" instrument... for each bank (group of 12 samples):
  4.1. create a new instrument with Generic Sample type.
  4.2. call it "Legacy Samples" if only one bank exists, or "Legacy Samples (bank BANK)".
  4.3. enable sample map.
  4.4. populate the sample map by assigning 12 samples to each octave.

a warning is displayed when legacy sample mode conversion took place.

## dev238 - file player

an audio file player is added. it supports synchronized playback and is useful for making covers.

## dev237 - partial pitch linearity removal

Furnace began as a DefleMask-compatible tracker, inheriting its peculiar approach to pitch slides/control, known as "partial pitch linearity".

in non-linear pitch, slides and pitch control (E5xx) operate in the period/frequency registers. this results in slide durations not being consistent across octaves and pitch control range also depending on the note.

in linear pitch, slides and pitch control operate in linear space. this guarantees a ±1 semitone range for pitch control and ensures slides will always have the same duration regardless of frequency.

partial pitch linearity is an oddity which combines both linear pitch control and non-linear slides.
however, it is more complicated to maintain and makes it harder to write a driver for.

this version removes partial pitch linearity. songs using partial pitch linearity will be converted to fully linear pitch and a warning is displayed afterwards.

## dev236 - fix OPM E5xx range

the E5xx effect in OPM had an unusual range: 40 to C0. this is a DefleMask quirk which actually maps the effect to the OPM's key fraction range (1/64th of a semitone). however, it is inconsistent with other chips.

Furnace will convert OPM songs made in previous versions to the new range, by scaling 40-C0 to 00-FF.

## dev235 - pattern refactor

this version changes the internal pattern format.
note and octave are no longer separate fields - instead, these have been fused into one.

no changes were made to the file format though.

## dev234 - extra config paranoia

Furnace keeps previous versions of files in the config directory as a safeguard against corruption (typically caused by forced disk unmount or abnormal loss of power).

this version adds start/end markers to config files to ensure these files are complete.

this change was made because under certain circumstances the config file may seem valid but actually is incomplete. happened often on Android.

## dev233 - breaking the limit

the wavetable and sample limit has been increased to 32768 (from 256).

the version bump is mandatory due to addition of new features to the instrument format.
the LW (list of wavetables) and LS (list of samples) features replace the previous WL/SL ones which were limited to 256 entries.

## dev232 - I don't know

I don't exactly remember why did I bump version, but I think we were diagnosing a crash or some issue and did so to tell versions apart.

## dev231 - OPN default chip revision

OPN2 had three or so notable revisions during its history:
- YM2612 (OPN2): the original, with a built-in 9-bit DAC. a tiny defect in the DAC introduces some "distortion" when the output is quiet. present in Model 1 Genesis/Mega Drive.
- YM3438 (OPN2C): this revision fixes the aforementioned defect. present in Model 2 Genesis/Mega Drive.
- YMF276 (OPL2L): this revision is meant to be used with an external DAC. as a result, it has better sound quality. present in the FM Towns computer.

this version changes the default revision to YM2612 (it previously was YM3438).

when loading an older song, the `chipType` flag will be set to 0 (YM3438) if neither the `chipType` nor `ladderEffect` flags are present.

## dev230 - I don't know

now this time I cannot recall why did this version bump occur. I apologize.

## dev229 - VERA accuracy fix

the VERA emulator had inaccurate noise emulation. it was being run at twice the normal rate.

this version addresses that problem.

when loading a previous version song, set the `chipType` flag to 2.

## 0.6.8.1 (228)

this version removes the shitty April Fools' jokes for 2025.
these jokes were horrible and impossible to turn off.

Furnace 0.6.8.2 and 0.6.8.3 share this version number if I remember correctly.

## 0.6.8 (227)

## 0.6.8pre2 (226)

## 0.6.8pre1 (225)

## dev224 - FM fixed block

this version adds a new field to the instrument format, which allows the user to force a specific block (octave) in certain FM chips (OPN, OPL and OPLL).

this parameter is useful to force changes in RS/KSL calculation or SSG-EG frequencies.

## dev223 - Y8950 ADPCM pitch fix

this version fixes Y8950 ADPCM pitch being too high.

when loading a file made with a previous version, set the `compatYPitch` flag to true on every Y8950 chip.

## dev222 - addition of SID3

SID3 is a fantasy chip created by LTVA. it is a rework of his SID2 fantasy chip, now with more channels, higher envelope resolution, special shapes, several modulation modes and even a wavetable channel.

## dev221 - addition of OPL4

this version adds the OPL4 chip. it is an OPL3 with 24 channels of MultiPCM built-in.

the MultiPCM instrument format has seen a few changes.

## dev220 - SNES anti-click

## 0.6.7 (219)
