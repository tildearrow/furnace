# NEC PC Engine instrument editor

The PCE instrument editor consists of these macros:

- [Volume] - volume sequence
- [Arpeggio] - pitch in half-steps
- [Noise] - enable noise mode (ch5 and ch6 only)
- [Waveform] - wavetable sequence
- [Panning (left)] - output level for left channel
- [Panning (right)] - output level for right channel
- [Pitch] - fine pitch
- [Phase Reset] - trigger restart of waveform

It also has wavetable synthesizer support, but unfortunately, it clicks a lot when in use on the HuC6280.
