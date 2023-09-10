# FM (OPN) instrument editor

The FM editor is divided into 7 tabs:

- **FM**: for controlling the basic parameters of FM sound source.
- **Macros (FM)**: for macros controlling algorithm, feedback and LFO.
- **Macros (OP1)**: for macros controlling FM parameters of operator 1.
- **Macros (OP2)**: for macros controlling FM parameters of operator 2.
- **Macros (OP3)**: for macros controlling FM parameters of operator 3.
- **Macros (OP4)**: for macros controlling FM parameters of operator 4.
- **Macros**: for miscellaneous macros controlling volume, arpeggio, and pitch.

## FM

OPN is four-operator, meaning it takes four oscillators to produce a single sound.

These apply to the instrument as a whole:
- **Algorithm (ALG)**: Determines how operators are connected to each other (0 to 7).
  - Left-click pops up a small "operators changes with volume?" dialog where each operator can be toggled to scale with volume level.
  - Right-click to switch to a preview display of the waveform generated on a new note:
    - Left-click restarts the preview.
    - Middle-click pauses and unpauses the preview.
    - Right-click returns to algorithm view.
- **Feedback (FB)**: Determines how many times operator 1 returns its output to itself (0 to 7).

- **LFO > Freq (FMS)**: Determines how much will LFO have an effect in frequency (0 to 7).
- **LFO > Amp (AMS)**: Determines how much will LFO have an effect in volume (0 to 3).
  - only applies to operators which have AM turned on.
  - does not apply to YM2203.

These apply to each operator:
- The crossed-arrows button can be dragged to rearrange operators.
- The **OP1**, **OP2**, **OP3**, and **OP4** buttons enable or disable those operators.
- **Amplitude Modulation (AM)**: Makes the operator's volume affected by LFO.
  - does not apply to YM2203.
- **Attack Rate (AR)**: determines the rising time for the sound. The bigger the value, the faster the attack (0 to 31).
- **Decay Rate (DR)**: Determines the diminishing time for the sound. The higher the value, the shorter the decay. It's the initial amplitude decay rate (0 to 31).
- **Sustain Level (SL)**: Determines the point at which the sound ceases to decay and changes to a sound having a constant level. The sustain level is expressed as a fraction of the maximum level (0 to 15).
- **Decay Rate 2 (D2R) / Sustain Rate (SR)**: Determines the diminishing time for the sound. The higher the value, the shorter the decay. This is the long "tail" of the sound that continues as long as the key is depressed (0 to 31).
- **Release Rate (RR)**: Determines the rate at which the sound disappears after KEY-OFF. The higher the value, the shorter the release (0 to 15).
- **Total Level (TL)**: Represents the envelope’s highest amplitude, with 0 being the largest and 127 (decimal) the smallest. A change of one unit is about 0.75 dB.
- **Hardware Envelope Generator (SSG-EG)**: Executes the built-in envelope, inherited from AY-3-8910 PSG. Speed of execution is controlled via Decay Rate.

![FM ADSR chart](FM-ADSRchart.png)

- **Envelope Scale (RS/KS)**: Also known as "Key Scale" or "Rate Scale". Determines the degree to which the envelope execution speed increases according to the pitch (0 to 3).
- **Frequency Multiplier (MULT)**: Determines the operator frequency in relation to the pitch (0 to 15).
- **Fine Detune (DT)**: Shifts the pitch a little (0 to 7).


## macros

Macros define the sequence of values passed to the given parameter. Via macro, along with the previously mentioned parameters, the following can be controlled:

## FM Macros

- **LFO Speed**: LFO frequency.
- **OpMask**: toggles each operator.

## OP1-OP4 Macros

All parameters are listed above.

## Macros

- **Arpeggio**: Pitch change sequence in semitones.
- **Panning**: toggles output on left and right channels.
- **Pitch**: fine pitch.
  - **Relative**: when enabled, pitch changes are relative to the current pitch.
- **Phase Reset**: Restarts all operators and resets the waveform to its start.


# links

[FM instrument tutorial](https://www.youtube.com/watch?v=wS8edjurjDw): A great starting point to learn how create and work with FM sounds. This was made for DefleMask, but all the same principles apply.
