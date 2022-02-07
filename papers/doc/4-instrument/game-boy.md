# Game Boy instrument editor

GB instrument editor consists of two tabs: one controlling envelope of sound channels and macro tab containing only four macros:

## Game Boy

- [Volume] - this slider affect the channel volume (range 0-15)
- [Envelope length] - this slider specifies the envelope decay/attack (range 0-7)
- [Sound length] - this slider cuts off the sound after specified length, overriding the previous slider's value

- [UP an DOWN radio buttons] - these buttons alter the behaviour of a second slider. Up makes it specify the envelope attack, down the decay. WARNING: for envelope attack to have any effect, volume should be at the lower rates!

## Macros
- [Volume] - volume sequence
- [Arpeggio] - pitch sequence
- [Duty cycle] - pulse wave channels duty cycle sequence
- [Waveform] - ch3 wavetable sequence
