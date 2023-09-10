# OPL FM synthesis instrument editor

The OPL FM editor is divided into 7 tabs:

- **FM**: for controlling the basic parameters of FM sound source.
- **Macros (FM)**: for macros controlling algorithm and feedback.
- **Macros (OP1)**: for macros controlling FM parameters of operator 1.
- **Macros (OP2)**: for macros controlling FM parameters of operator 2.
- **Macros (OP3)**: for macros controlling FM parameters of operator 3 (only when 4-op flag is set and only on OPL3!).
- **Macros (OP4)**: for macros controlling FM parameters of operator 4 (only when 4-op flag is set and only on OPL3!).
- **Macros**: for miscellaneous macros controlling volume, arpeggio, and OPL3 panning.

## FM

the OPL synthesizers are nominally two-operator (OPL3 supports 4-operator mode on up to six channels), meaning it takes two oscillators to produce a single sound.

These apply to the instrument as a whole:
- **Algorithm (ALG)**: Determines how operators are connected to each other (0-1 range and OPL1 and OPL2; 0-3 range on OPL3 4op mode).
  - Left-click pops up a small "operators changes with volume?" dialog where each operator can be toggled to scale with volume level.
  - Right-click to switch to a preview display of the waveform generated on a new note:
    - Left-click restarts the preview.
    - Middle-click pauses and unpauses the preview.
    - Right-click returns to algorithm view.
- **Feedback (FB)**: Determines how many times operator 1 returns its output to itself (0 to 7).

- **4-op**: Enables 4-operator FM instrument editor mode (only on OPL3).
- **Drums**: Enables OPL drum mode editor.

These apply to each operator:
- The crossed-arrows button can be dragged to rearrange operators.
- **Amplitude Modulation (AM)**: Makes the operator affected by LFO tremolo.
- **Sustain flag (SUS)**: When enabled, value of Sustain Level is in effect.
- **Attack Rate (AR)**: determines the rising time for the sound. The bigger the value, the faster the attack (0 to 15).
- **Decay Rate (DR)**: Determines the diminishing time for the sound. The higher the value, the shorter the decay. It's the initial amplitude decay rate (0 to 15).
- **Sustain Level (SL)**: Determines the point at which the sound ceases to decay and changes to a sound having a constant level. The sustain level is expressed as a fraction of the maximum level (0 to 15).
- **Release Rate (RR)**: Determines the rate at which the sound disappears after note off. The higher the value, the shorter the release (0 to 15).
- **Total Level (TL)**: Represents the envelope’s highest amplitude, with 0 being the largest and 63 (decimal) the smallest. A change of one unit is about 0.75 dB.
- **Key Scale Level (KSL)**: Also known as "Level Scale". Determines the degree to which the amplitude decreases according to the pitch.

![FM ADSR chart](FM-ADSRchart.png)

- **Key Scale Rate (KSR)**: Also known as "Rate Scale". Determines the degree to which the envelope execution speed increases according to the pitch.
- **Frequency Multiplier (MULT)**: Determines the operator frequency in relation to the pitch (0-15 range but be noted that 11, 13 and 14 have no effect!).
- **Waveform Select (WS)**: Changes the waveform of the operator (OPL2 and OPL3 only, 0-3 range on OPL2 and 0-7 on OPL3).
- **Vibrato (VIB)**: Makes the operator affected by LFO vibrato.

## macros

Macros define the sequence of values passed to the given parameter. Via macro, along with the previously mentioned parameters, the following can be controlled:

## FM Macros

All parameters are listed above.

## OP1-OP4 Macros

All parameters are listed above.

## Macros

- **Volume**: volume sequence.
- **Arpeggio**: pitch sequence.
- **Panning**: enables output on left/right/rear channels. OPL3 only.
- **Pitch**: fine pitch.
  - **Relative**: when enabled, pitch changes are relative to the current pitch.
- **Phase Reset**: restarts all operators and resets the waveform to its start.

# OPL (drums) instrument editor

this is similar to the OPL instrument editor, but sets the parameters of snare, tom, top and hi-hat directly once a drums instrument is activated.
