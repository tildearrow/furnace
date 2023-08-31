# OPLL FM synthesis instrument editor

The OPLL FM editor is divided into 5 tabs:

- **FM**: for controlling the basic parameters of FM sound source.
- **Macros (FM)**: for macros controlling algorithm, waveform and feedback
- **Macros (OP1)**: for macros controlling FM parameters of operator 1
- **Macros (OP2)**: for macros controlling FM parameters of operator 2
- **Macros**: for miscellaneous macros controlling volume, arpeggio, and preset.

## FM

The OPLL synthesizer is two-operator, meaning it takes two oscillators to produce a single sound.

These apply to the instrument as a whole:
- **Feedback (FB)**: Determines how many times operator 1 returns its output to itself. (0-7 range)
- **Sustain (SUS)**: enables the sustain flag (sets the release rate to 5) 
- algorithm: shows the connection of operators (though they are always connected the same way).
  - Right-click to switch to a preview display of the waveform generated on a new note:
    - Left-click restarts the preview.
    - Middle-click pauses and unpauses the preview.
    - Right-click returns to algorithm view.
- **DC (half-sine carrier)**: Sets the waveform produced by carrier operator to half-sine
- **DM (half-sine modulator)**: Sets the waveform produced by modulator operator to half-sine
- preset dropdown: selects OPLL preset instrument.

These apply to each operator:
- The crossed-arrows button can be dragged to rearrange operators.
- **Amplitude Modulation (AM)**: Makes the operator affected by LFO tremolo.
- **Envelope generator sustain flag (EGS)**: When enabled, value of Sustain Level is in effect.
- **Attack Rate (AR)**: determines the rising time for the sound. The bigger the value, the faster the attack. (0-15 range)
- **Decay Rate (DR)**: Determines the diminishing time for the sound. The higher the value, the shorter the decay. It's the initial amplitude decay rate. (0-15 range)
- **Sustain Level (SL)**: Determines the point at which the sound ceases to decay and changes to a sound having a constant level. The sustain level is expressed as a fraction of the maximum level. (0-15 range)
- **Release Rate (RR)**: Determines the rate at which the sound disappears after KEY-OFF. The higher the value, the shorter the release. (0-15 range)
- **Total Level (TL)**: Represents the envelope’s highest amplitude, with 0 being the largest and 63 (decimal) the smallest. A change of one unit is about 0.75 dB.

![FM ADSR chart](FM-ADSRchart.png)

- **Envelope Scale (KSR)**: Also known as "Key Scale". Determines the degree to which the envelope execution speed increases according to the pitch.
- **Frequency Multiplier (MULT)**: Determines the operator frequency in relation to the pitch. (0-10, 12, 15 range)
- **Pitch Modulation (VIB)**: Makes the operator affected by LFO vibrato.

## macros

Macros define the sequence of values passed to the given parameter. Via macro, along with the previously mentioned parameters, the following can be controlled:

## FM Macros

All parameters are listed above.

## OP1-OP4 Macros

All parameters are listed above.

## Macros

- **Arpeggio**: Pitch change sequence in semitones.
- **Patch**: changes the playing preset mid-note
- **Pitch**: fine pitch.
  - **Relative**: pitch changes are relative to the current pitch, not the note's base pitch.
- **Phase Reset**: Restarts all operators and resets the waveform to its start. Effectively the same as a `0Cxx` retrigger.


# links

[FM instrument tutorial](https://www.youtube.com/watch?v=wS8edjurjDw): A great starting point to learn how create and work with FM sounds. This was made for DefleMask, but all the same principles apply.
