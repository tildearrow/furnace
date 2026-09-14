# ESS ESFM

an FM synthesizer core included in a series of sound card chipsets made by ESS, which were mildly popular in the DOS days during the mid-late 90s.

at a cursory glance, it looks like just an [OPL3 clone](opl.md). but hidden under a veil of mystery is its exclusive "native mode", revealing an impressive superset of features, including 4-operator support on all 18 channels, semi-modular operator routing, per-operator pitch control, and even a few unique features.

for a long time, not much was known about the inner workings of ESFM's native mode, since ESS did not release any documentation to developers on how to use it. this has thankfully changed in recent years thanks to reverse-engineering efforts from the community.

thanks to ESS's decision to not release any documentation to developers and lock down usage of native mode behind a couple of General MIDI drivers shipping with rather lackluster patch sets, ESFM's native mode was unfortunately not very well used over its original lifespan.

## effects

- `10xy`: **set AM depth.**
  - `x` is the operator from 1 to 4. a value of `0` means "all operators".
  - `y` is either `0` (1dB, shallow) or `1` (4.8dB, deep).
- `12xx`: **set operator 1 level.**
- `13xx`: **set operator 2 level.**
- `14xx`: **set operator 3 level.**
- `15xx`: **set operator 4 level.**
- `16xy`: **set multiplier of operator.**
  - `x` is the operator (1-4).
  - `y` is the new MULT value..
- `17xy`: **set vibrato depth.**
  - `x` is the operator from 1 to 4. a value of `0` means "all operators".
  - `y` is either `0` (normal) or `1` (double).
- `19xx`: **set attack of all operators.**
- `1Axx`: **set attack of operator 1.**
- `1Bxx`: **set attack of operator 2.**
- `1Cxx`: **set attack of operator 3.**
- `1Dxx`: **set attack of operator 4.**
- `20xy`: **set panning of operator 1.**
  - `x` determines whether to output on left.
  - `y` determines whether to output on right.
- `21xy`: **set panning of operator 2.**
  - `x` determines whether to output on left.
  - `y` determines whether to output on right.
- `22xy`: **set panning of operator 3.**
  - `x` determines whether to output on left.
  - `y` determines whether to output on right.
- `23xy`: **set panning of operator 4.**
  - `x` determines whether to output on left.
  - `y` determines whether to output on right.
- `24xy`: **set output level of operator.**
  - `x` is the operator from 1 to 4. a value of `0` means "all operators".
  - `y` is the value.
- `25xy`: **set modulation input level of operator.**
  - `x` is the operator from 1 to 4. a value of `0` means "all operators".
  - `y` is the value.
- `26xy`: **set envelope delay of operator.**
  - `x` is the operator from 1 to 4. a value of `0` means "all operators".
  - `y` is the value.
- `27xx`: **set operator 4 noise mode.**
  - `0`: noise off
  - `1`: square + noise
  - `2`: ring mod from operator 3 + noise
  - `3`: ring mod from operator 3 + double pitch modulation input
    - note: emulation issues. subject to change!
- `2Axy`: **set waveform of operator.**
  - `x` is the operator from 1 to 4. a value of `0` means "all operators".
  - `y` is the value.
- `2Exx`: **enable envelope hard reset.**
- `2Fxy`: **set fixed frequency block (octave).**
  - `x` is the operator from 1 to 4.
  - `y` is the block/octave from 0 to 7.
- `3xyy`: **set fixed frequency f-num.**
  - `x` contains operator number and high bits of f-num may be any of the following:
    - `0` to `3` for operator 1
    - `4` to `7` for operator 2
    - `8` to `B` for operator 3
    - `C` to `F` for operator 4
  - `y` are the lower bits of f-num.
- `40xx`: **set operator 1 detune.**
- `41xx`: **set operator 2 detune.**
- `42xx`: **set operator 3 detune.**
- `43xx`: **set operator 4 detune.**
- `50xy`: **set AM of operator.**
  - `x` is the operator from 1 to 4. a value of `0` means "all operators".
  - `y` determines whether AM is on.
- `51xy`: **set SL of operator.**
  - `x` is the operator from 1 to 4. a value of `0` means "all operators".
  - `y` is the value.
- `52xy`: **set RR of operator.**
  - `x` is the operator from 1 to 4. a value of `0` means "all operators".
  - `y` is the value.
- `53xy`: **set VIB of operator.**
  - `x` is the operator from 1 to 4. a value of `0` means "all operators".
  - `y` determines whether VIB is on.
- `54xy`: **set KSL of operator.**
  - `x` is the operator from 1 to 4. a value of `0` means "all operators".
  - `y` is the value.
- `55xy`: **set SUS of operator.**
  - `x` is the operator from 1 to 4. a value of `0` means "all operators".
  - `y` determines whether SUS is on.
- `56xx`: **set DR of all operators.**
- `57xx`: **set DR of operator 1.**
- `58xx`: **set DR of operator 2.**
- `59xx`: **set DR of operator 3.**
- `5Axx`: **set DR of operator 4.**
- `5Bxy`: **set KSR of operator.**
  - `x` is the operator from 1 to 4. a value of `0` means "all operators".
  - `y` determines whether KSR is on.

## info

this chip uses the [FM (ESFM)](../4-instrument/fm-esfm.md) instrument editor.
