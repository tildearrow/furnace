# Ensoniq ES5504 DOCII, ES5505 OTIS, ES5506 OTTO

## Summary

- 32 voice of PCMs
  - per-voice features:
  - 4 pole, 4 mode filter (2 filter coefficient, 12 bit per them)
  - max 16 output, 12 bit volume (ES5504)
  - max 4 stereo output channels, 8 bit logarithmic volume per left/right output (ES5505)
  - max 6 stereo output channels, 12 bit logarithmic volume per left/right output (ES5506)
  - Hardware envelope for volume and filter (ES5506)
  - 16 bit linear PCM
  - 8 bit compressed PCM (ES5506)
  - total accessible memory: 1 MWord (2MByte) per bank (ES5504, ES5505) or 2 MWord (4MByte) per bank (ES5506)
    - CA flag is also usable for bank
    - 1 bit BS flag for bank - 2 bank total (ES5505)
    - 2 bit BS flag for bank - 4 bank total (ES5506)

## Source code

- es550x.hpp: Base header
  - es550x.cpp: Emulation core for common shared features
  - es550x_alu.cpp: Emulation core for shared ALU function
  - es550x_filter.cpp: Emulation core for shared filter function
  - es5504.hpp: ES5504 header
    - es5504.cpp: Emulation core for ES5504 specific features
  - es5505.hpp: ES5505 header
    - es5505.cpp: Emulation core for ES5505 specific features
  - es5506.hpp: ES5506 header
    - es5506.cpp: Emulation core for ES5506 specific features

## Description

After ES5503 DOC's appeared, Ensoniq announces ES5504 DOC II, ES5505 OTIS, ES5506 OTTO.

These are not just PCM chip; but with built-in 4 pole filters and variable voice limits.

It can be trades higher sample rate and finer frequency and Tons of voices, or vice-versa.

These are mainly used with their synthesizers, musical stuffs. It's also mainly paired with ES5510 ESP/ES5511 ESP2 for post processing. ES5506 can be paired with itself, It's called Dual chip configuration and Both chips are can be shares same memory spaces.

ES5505 was also mainly used on Taito's early- to late-90s arcade hardware for their PCM sample based sound system, paired with ES5510 ESP for post processing. It's configuration is borrowed from Ensoniq's 32 Voice synths powered by these chips. It's difference is external logic to adds per-voice bankswitching looks like what Konami doing on K007232.

Atari Panther was will be use ES5505, but finally canceled.

Ensoniq's ISA Sound Card for PC, Soundscape used ES5506, "Elite" model has optional daughterboard with ES5510 for digital effects.

## Related chips

- ES5530 "OPUS" variant is 2-in-one chip with built-in ES5506 and Sequoia.

- ES5540 "OTTOFX" variant is ES5506 and ES5510 merged in single package.

- ES5548 "OTTO48" variant is used at late-90s ensoniq synths and musical instruments, 2 ES5506s are merged in single package, or with 48 voices in chip?

## Chip difference

### ES5504 to ES5505

- Total voice amount is expanded to 32, rather than 25.

- ADC and DAC is completely redesigned.

- it's has now voice-independent 10 bit and Sony/Burr-Brown format DAC.

- Output channel and Volume is changed to 16 mono to 4 stereo, 12 bit Analog to 8 bit Stereo digital, also Floating point-ish format and independent per left and right output.

- Channel 3 is can be Input/Output.

- Channel output is can be accessible at host for test purpose.

- Max sample memory is expanded to 2MWords (1MWords * 2 Banks)

### ES5505 to ES5506

- Frequency is more finer now: 11 bit fraction rather than 9 bit.

- Output channel and Volume is changed to 4 stereo to 6 stereo, 8 bit to 16 bit, but only 12 bit is used for calculation; 4 LSB is used for envelope ramping.

- Transwave flag is added - its helpful for transwave process, with interrupt per voices. Hardware envelope is added - K1, K2, Volume value is can be modified in run-time. also K1, K2 is expanded to 16 bit for finer envelope ramping.

- Filter calculation resolution is expanded to 18 bit.

- All channels are output, Serial output is now partially programmable.

- Max sample memory is expanded to 8MWords (2MWords * 4 Banks)

- Register format between these chips are incompatible.
