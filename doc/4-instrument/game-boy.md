# Game Boy instrument editor

GB instrument editor consists of two tabs: one controlling envelope of sound channels and macro tab containing several macros.

## Game Boy

- [Use software envelope] - switch to volume macro instead of envelope
- [Initialize envelope on every note] - forces a volume reset on each new note
- [Volume] - initial channel volume (range 0-15)
- [Length] - envelope decay/attack duration (range 0-7)
- [Sound Length] - cuts off sound after specified length, overriding the Length value

- [Up and Down radio buttons] - Up makes the envelope an attack, down makes it decay. _Note:_ For envelope attack to have any effect, start at a lower volume!

- [Hardware Sequence] - (document this)

## Macros

- [Volume] - volume sequence. _Note:_ This only appears if "Use software envelope" is checked.
- [Arpeggio] - pitch in half-steps
- [Duty/Noise] - pulse wave duty cycle or noise mode sequence
- [Waveform] - ch3 wavetable sequence
- [Panning] - output for left and right channels
- [Pitch] - fine pitch
- [Phase Reset] - trigger restart of waveform
