# Yamaha OPL

a series of FM sound chips which were very popular in DOS land. it was so popular that even Yamaha made a logo for it!

essentially a downgraded version of Yamaha's other FM chips, with only 2 operators per channel.
however, it also had a drums mode, and later chips in the series added more waveforms (than just the typical sine) and even a 4-operator mode.

the original OPL (Yamaha YM3526) was present as an expansion for the Commodore 64 and MSX computers (erm, a variant of it). it only had 9 two-operator channels and drums mode.

its successor, the OPL2 (Yamaha YM3812), added 3 more waveforms and was one of the more popular chips because it was present on the AdLib card for PC.
later Creative would borrow the chip to make the Sound Blaster, and totally destroyed AdLib's dominance.

the OPL3 (Yamaha YMF262) added 9 more channels, 4 more waveforms, rudimentary 4-operator mode (pairing up to 12 channels to make up to six 4-operator channels), quadraphonic output (sadly Furnace only supports stereo) and some other things.

afterwards everyone moved to Windows and software mixed PCM streaming...

# effects

- 10xx: set AM depth. the following values are accepted:
  - 0: 1dB (shallow)
  - 1: 4.8dB (deep)
  - this effect applies to all channels.
- `11xx`: set feedback of channel.
- `12xx`: set operator 1 level.
- `13xx`: set operator 2 level.
- `14xx`: set operator 3 level.
  - only in 4-op mode (OPL3).
- `15xx`: set operator 4 level.
  - only in 4-op mode (OPL3).
- `16xy`: set multiplier of operator.
  - `x` is the operator (1-4; last 2 operators only in 4-op mode).
  - `y` is the mutliplier.
- 17xx: set vibrato depth. the following values are accepted:
  - 0: normal
  - 1: double
  - this effect applies to all channels.
- `18xx`: toggle drums mode.
  - 0 disables it and 1 enables it.
  - only in drums chip.
- `19xx`: set attack of all operators.
- `1Axx`: set attack of operator 1.
- `1Bxx`: set attack of operator 2.
- `1Cxx`: set attack of operator 3.
  - only in 4-op mode (OPL3).
- `1Dxx`: set attack of operator 4.
  - only in 4-op mode (OPL3).
- `2Axy`: set waveform of operator.
  - `x` is the operator (1-4; last 2 operators only in 4-op mode). a value of 0 means "all operators".
  - `y` is the value.
  - only in OPL2 or higher.
- `30xx`: enable envelope hard reset.
  - this works by inserting a quick release and tiny delay before a new note.
- `50xy`: set AM of operator.
  - `x` is the operator (1-4; last 2 operators only in 4-op mode). a value of 0 means "all operators".
  - `y` determines whether AM is on.
- `51xy` set SL of operator.
  - `x` is the operator (1-4; last 2 operators only in 4-op mode). a value of 0 means "all operators".
  - `y` is the value.
- `52xy` set RR of operator.
  - `x` is the operator (1-4; last 2 operators only in 4-op mode). a value of 0 means "all operators".
  - `y` is the value.
- `53xy`: set VIB of operator.
  - `x` is the operator (1-4; last 2 operators only in 4-op mode). a value of 0 means "all operators".
  - `y` determines whether VIB is on.
- `54xy` set KSL of operator.
  - `x` is the operator (1-4; last 2 operators only in 4-op mode). a value of 0 means "all operators".
  - `y` is the value.
- `55xy` set SUS of operator.
  - `x` is the operator (1-4; last 2 operators only in 4-op mode). a value of 0 means "all operators".
  - `y` determines whether SUS is on.
- `56xx`: set DR of all operators.
- `57xx`: set DR of operator 1.
- `58xx`: set DR of operator 2.
- `59xx`: set DR of operator 3.
  - only in 4-op mode (OPL3).
- `5Axx`: set DR of operator 4.
  - only in 4-op mode (OPL3).
- `5Bxy`: set KSR of operator.
  - `x` is the operator (1-4; last 2 operators only in 4-op mode). a value of 0 means "all operators".
  - `y` determines whether KSR is on.