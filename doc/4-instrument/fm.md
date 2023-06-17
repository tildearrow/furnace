# FM synthesis instrument editor

FM editor is divided into 7 tabs:

- [FM] - for controlling the basic parameters of FM sound source.
- [Macros (FM)]- for macros controlling algorithm, feedback and LFO 
- [Macros (OP1)] - for macros controlling FM paramets of operator 1
- [Macros (OP2)] - for macros controlling FM paramets of operator 2
- [Macros (OP3)] - for macros controlling FM paramets of operator 3
- [Macros (OP4)] - for macros controlling FM paramets of operator 4
- [Macros] - for miscellaneous macros controlling volume, argeggio and YM2151 noise generator.

## FM

FM synthesizers Furnace supports are four-operator, meaning it takes four oscillators to produce a single sound. Each operator is controlled by a dozen sliders:

- [Attack Rate (AR)] - determines the rising time for the sound. The bigger the value, the faster the attack. (0-31 range)
- [Decay Rate (DR)]- Determines the diminishing time for the sound. The higher the value, the shorter the decay. It's the initial amplitude decay rate. (0-31 range)
- [Secondary Decay Rate (DR2)/Sustain Rate (SR)] - Determines the diminishing time for the sound. The higher the value, the shorter the decay. This is the long "tail" of the sound that continues as long as the key is depressed. (0-31 range)
- [Release Rate (RR)] - Determines the rate at which the sound disappears after KEY-OFF. The higher the value, the shorter the release. (0-15 range)
- [Sustain Level(SL)] - Determines the point at which the sound ceases to decay and changes to a sound having a constant level. The sustain level is expressed as a fraction of the maximum level. (0-15 range)
- [Total Level (TL)] - Represents the envelope’s highest amplitude, with 0 being the largest and 127 (decimal) the smallest. A change of one unit is about 0.75 dB.
- [Envelope Scale (KSR)] - A parameter that determines the degree to which the envelope execution speed increases according to the pitch. (0-3 range)
- [Frequency Multiplier (MULT)] - Determines the operator frequency in relation to the pitch. (0-15 range)
- [Fine Detune (DT)] - Shifts the pitch a little (0-7 range)
- [Coarse Detune (DT2)] - Shifts the pitch by tens of cents (0-3 range) WARNING: this parameter affects only YM2151 sound source!!!
- [Hardware Envelope Generator (SSG-EG)] - Executes the built-in envelope, inherited from AY-3-8910 PSG. Speed of execution is controlled via Decay Rate. WARNING: this parameter affects only YM2610/YM2612 sound source!!!
- [Algorithm (AL)] - Determines how operators are connected to each other. (0-7 range)
- [Feedback (FB)] - Determines the amount of signal whick operator 1 returns to itself. (0-7 range)
- [Amplitude Modulation (AM)] - Makes the operator affected by LFO.
- [LFO Frequency Sensitivity] - Determines the amount of LFO frequency changes. (0-7 range)
- [LFO Amplitude Sensitivity (AM)] - Determines the amount of LFO frequency changes. (0-3 range)

## Macros

Macros define the sequence of values passed to the given parameter. Via macro, aside previously mentioned parameters, the following can be controlled:

- LFO Frequency
- LFO waveform selection WARNING: this parameter affects only YM2151 sound source!!!
- Amplitude Modulation Depth WARNING: this parameter affects only YM2151 sound source!!!
- Frequency Modulation Depth WARNING: this parameter affects only YM2151 sound source!!!
- Arpeggio Macro: pitch change sequence in semitones. Two modes are available:  
Absolute (default) - Executes the pitch with absolute change based on the pitch of the actual note.
Fixed - Executes at the pitch specified in the sequence regardless of the note pitch.
- Noise Frequency: specifies the noise frequency in noise mode of YM2151's Channel 8 Operator 4 special mode.

Loop
You can loop the execution of part of a sequence. Left-click anywhere on the Loop line at the bottom of the editor to create a loop. You can move the start and end points of the loop by dragging both ends of the loop.
