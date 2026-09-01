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

## dev242 - OPL4 mixing levels

## dev241 - fix groove saving

## dev240 - new info header!

## dev239 - total extinction of legacy sample mode

## dev238 - file player

## dev237 - partial pitch linearity removal

## dev236 - fix OPM E5xx range

## dev235 - pattern refactor

## dev234 - extra config paranoia

## dev233 - breaking the limit
