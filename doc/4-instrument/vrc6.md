# VRC6 instrument editor

The VRC6 (regular) instrument editor consists of these macros:

- [Volume] - volume sequence
- [Arpeggio] - pitch sequence
- [Duty] - specifies duty cycle for pulse wave channels
- [Pitch] - fine pitch

## VRC6 (saw) instrument editor

This channel has its own instrument, a (currently, as of 0.6) one-of-a-kind thing in Furnace.

The only differences from this instrument type from compared to the regular instrument are that it:
 - has a volume range of 0-63 instead of 0-15.
 - has no duty cycle range
