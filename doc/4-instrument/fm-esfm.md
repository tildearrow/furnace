# ESFM instrument editor

the ESFM editor is divided into 6 tabs:

- **FM**: for controlling the basic parameters of FM sound source.
- **Macros (OP1)**: for macros controlling FM parameters of operator 1.
- **Macros (OP2)**: for macros controlling FM parameters of operator 2.
- **Macros (OP3)**: for macros controlling FM parameters of operator 3.
- **Macros (OP4)**: for macros controlling FM parameters of operator 4.
- **Macros**: for other macros (volume/arp/pitch/pan/noise mode).

## FM

ESFM is four-operator, but it is different from the rest of FM chips.

the concept of an algorithm does not exist in ESFM. instead, modulation routing is arbitrary, with each operator having output level and modulation input parameters.

these apply to the instrument as a whole:
- **OP4 Noise Mode**: determines the mode used to produce noise in operator 4.
  - Normal: noise is disabled.
  - Snare: takes the snare noise generation mode from OPL. square + noise.
  - HiHat: ring modulates with operator 3 and adds noise.
  - Top: ring modulates with operator 3 and double pitch modulation input.
    - this mode is not emulated correctly. subject to change!

these apply to each operator:
- the crossed-arrows button can be dragged to rearrange operators.
- **Amplitude Modulation (AM)**: makes the operator affected by LFO tremolo.
- **Sustain flag (SUS)**: when enabled, value of Sustain Level is in effect.
- **AM Depth (AMD)**: when enabled, LFO tremolo is deeper.
- **Attack Rate (AR)**: determines the rising time for the sound. the bigger the value, the faster the attack (0 to 15).
- **Decay Rate (DR)**: determines the diminishing time for the sound. the higher the value, the shorter the decay. it's the initial amplitude decay rate (0 to 15).
- **Sustain Level (SL)**: determines the point at which the sound ceases to decay and changes to a sound having a constant level. the sustain level is expressed as a fraction of the maximum level (0 to 15).
- **Release Rate (RR)**: determines the rate at which the sound disappears after note off. the higher the value, the shorter the release (0 to 15).

- **Total Level (TL)**: represents the envelope’s highest amplitude, with 0 being the largest and 63 (decimal) the smallest. a change of one unit is about 0.75 dB.
- **Output Level (OL)**: determines the volume at which the operator will be output.
- **Modulation Input (MI)**: determines how much to take from the previous operator for modulation.
  - this controls feedback level in the case of operator 1.
- **Key Scale Level (KSL)**: also known as "Level Scale". determines the degree to which the amplitude decreases according to the pitch.

![FM ADSR chart](FM-ADSRchart.png)

- **Key Scale Rate (KSR)**: also known as "Rate Scale". determines the degree to which the envelope execution speed increases according to the pitch.
- **Frequency Multiplier (MULT)**: sets the coarse pitch offset in relation to the note (0 to 15). 0 is -1 octave, 1 is 0 octaves, 2 is 1 octave, 3 is 1 octave 7 semitones, and so on.
  - note that values 11, 13 and 14 behave as 10, 12 and 12 respectively.
- **Waveform Select (WS)**: changes the waveform of the operator (0 to 7).
- **Vibrato (VIB)**: makes the operator affected by LFO vibrato.
- **FM Depth (FMD)**: when enabled, vibrato is deeper.

- **Tune**: sets the coarse tune of the operator.
- **Detune**: sets the fine tune of the operator.

- **Left (L)**: output on the left channel.
- **Right (R)**: output on the right channel.

### fixed frequency mode

each operator has a Fixed Frequency mode. once enabled, the operator runs at the specified frequency regardless of the note.

## macros

these macros allow you to control several parameters of FM per tick.

## OP1-OP4 Macros

all parameters are listed above.

## Macros

- **Volume**: volume sequence.
- **Arpeggio**: pitch sequence.
- **OP4 Noise Mode**: noise mode sequence.
- **Panning**: enables output on left/right channels.
- **Pitch**: fine pitch.
  - **Relative**: when enabled, pitch changes are relative to the current pitch.
- **Phase Reset**: restarts all operators and resets the waveform to its start.
