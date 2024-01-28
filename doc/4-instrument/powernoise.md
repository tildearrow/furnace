# PowerNoise instrument editor

the PowerNoise instrument editor consists of two tabs.

## LFSR-based synthesis

PowerNoise employs LFSR-based synthesis for the noise channels, using linear-feedback shift registers for sound generation.

a linear-feedback shift register is one method used for random number generation.
it works by shifting a sequence of binary numbers (bits), taking the last bit into the output. then one of the bits is either pushed back into the register, or combined with another, doing a XOR (exclusive or) operation and then being pushed back.

think of it as a conveyor carrying glass bottles. each bottle may be empty or carrying water.
the bottle at the end is taken. if there's water, then the output is 1. if it's empty, the output is 0.
depending on the LFSR configuration:
- a bottle is pushed into the conveyor. it is either empty or filled with water depending on the bottle at a specific position in the conveyor (this is called a "tap"), or
- two bottles at specific positions ("taps") are looked at and combined as follows:
  - if the bottles are identical, an empty bottle is pushed.
  - if one bottle has water but the other is empty, a watee bottle is pushed.
the process is repeated indefinitely.

PowerNoise uses either one or two taps for the LFSR, configurable via the Control macro.

the LFSR must be initialized before it can produce sound. the Load LFSR macro allows you to do so.

by default the LFSR is configured to produce square waves, by having a single tap in position 1 and an alternating LFSR pattern.

## Macros

- **Volume**: volume sequence.
- **Arpeggio**: pitch sequence.
- **Panning (left)**: output level for left channel.
- **Panning (right)**: output level for right channel.
- **Pitch**: fine pitch.
- **Phase Reset**: trigger reloading the LFSR.
- **Control**: channel settings:
  - **slope AM**: when enabled, this channel's output and the slope channel go through amplitude modulation.
  - **tap B**: enables use of two taps for the LFSR.
- **Tap A Location**: sets the position of the first tap.
- **Tap B Location**: sets the position of the second tap.
- **Load LFSR**: allows you to load the LFSR with a specific pattern.

## PowerNoise tab

this tab allows you to change the base octave - important when you have set a longer LFSR pattern.

## PowerNoise (slope) instrument editor

this channel has its own instrument type, as it does not use LFSR-based synthesis but instead generates saw waves.

I will finish this section later... 
