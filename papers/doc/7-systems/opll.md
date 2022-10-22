# Yamaha YM2413/OPLL

the YM2413, otherwise known as OPLL, is a cost-reduced FM synthesis sound chip, based on the Yamaha YM3812 (OPL2). thought OPL was downgraded enough? :p

OPLL also spawned a few derivative chips, the best known of these is:
- the famous Konami VRC7. used in the Japan-only video game Lagrange Point, it was **another** cost reduction on top of the OPLL! this time just 6 channels...
- Yamaha YM2423, same chip as YM2413, just a different patch set
- Yamaha YMF281, ditto

# technical specifications

the YM2413 is equipped with the following features:

- 9 channels of 2 operator FM synthesis
- A drum/percussion mode, replacing the last 3 voices with 5 rhythm channels
- 1 user-definable patch (this patch can be changed throughout the course of the song)
- 15 pre-defined patches which can all be used at the same time
- Support for ADSR on both the modulator and the carrier
- Sine and half-sine based FM synthesis
- 9 octave note control
- 4096 different frequencies for channels
- 16 unique volume levels (NOTE: Volume 0 is NOT silent.)
- Modulator and carrier key scaling
- Built-in hardware vibrato support

# effects

- `11xx`: set feedback of channel.
- `12xx`: set operator 1 level.
- `13xx`: set operator 2 level.
- `16xy`: set multiplier of operator.
  - `x` is the operator (1 or 2).
  - `y` is the mutliplier.
- `18xx`: toggle drums mode.
  - 0 disables it and 1 enables it.
  - only in drums chip.
- `19xx`: set attack of all operators.
- `1Axx`: set attack of operator 1.
- `1Bxx`: set attack of operator 2.
- `50xy`: set AM of operator.
  - `x` is the operator (1-2). a value of 0 means "all operators".
  - `y` determines whether AM is on.
- `51xy` set SL of operator.
  - `x` is the operator (1-2). a value of 0 means "all operators".
  - `y` is the value.
- `52xy` set RR of operator.
  - `x` is the operator (1-2). a value of 0 means "all operators".
  - `y` is the value.
- `53xy`: set VIB of operator.
  - `x` is the operator (1-2). a value of 0 means "all operators".
  - `y` determines whether VIB is on.
- `54xy` set KSL of operator.
  - `x` is the operator (1-2). a value of 0 means "all operators".
  - `y` is the value.
- `55xy` set EGT of operator.
  - `x` is the operator (1-2). a value of 0 means "all operators".
  - `y` determines whether EGT is on.
- `56xx`: set DR of all operators.
- `57xx`: set DR of operator 1.
- `58xx`: set DR of operator 2.
- `5Bxy`: set KSR of operator.
  - `x` is the operator (1-2). a value of 0 means "all operators".
  - `y` determines whether KSR is on.