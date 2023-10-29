# VRC6 instrument editor

the VRC6 (regular) instrument editor consists of two tabs.

## Sample

for sample settings, see [the Sample instrument editor](sample.md).

the only differences are the lack of an "Use wavetable" option, and the presence of a "Use sample" one.

note that using samples on VRC6 is CPU expensive!

## Macros

- **Volume**: volume sequence.
- **Arpeggio**: pitch sequence.
- **Duty**: specifies duty cycle for pulse wave channels.
- **Pitch**: fine pitch.

## VRC6 (saw) instrument editor

this channel has its own instrument type, a one-of-a-kind thing in Furnace that was decided as a compromise during a debate.

the only differences from this instrument type compared to the regular one are:
- the lack of a Sample tab.
- it has a volume range of 0-63 instead of 0-15.
- it lacks a duty cycle macro.
  - if you come from FamiTracker, this may seem strange to you, but it isn't.
