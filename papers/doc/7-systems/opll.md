# Yamaha YM2413/OPLL
The YM2413, otherwise known as OPLL, is a cost-reduced FM synthesis sound chip manufactured by Yamaha Corporation and based on the Yamaha YM3812 sound chip (OPL2).

As of Furnace version 0.5.7pre4, the OPLL sound chip is not yet emulated. It is, however, emulated in Deflemask as of version 1.1.0. Support for loading .DMFs which contain the YM2413 was added in Furnace version 0.5.7pre4.

## Technical specifications
The YM2413 is equipped with the following features:
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

## Effect commands
TODO: Add effect commands here when YM2413 emulation is added.
