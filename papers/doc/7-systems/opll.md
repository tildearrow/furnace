# Yamaha YM2413/OPLL

the YM2413, otherwise known as OPLL, is a cost-reduced FM synthesis sound chip, based on the Yamaha YM3812 (OPL2). thought OPL was downgraded enough? :p

# technical specifications

the YM2413 is equipped with the following features:

- 9 channels of 2 operator FM synthesis
- A drum/percussion mode, replacing the last 3 voices with 3 rhythm channels
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
  - only in drums system.
- `19xx`: set attack of all operators.
- `1Axx`: set attack of operator 1.
- `1Bxx`: set attack of operator 2.
