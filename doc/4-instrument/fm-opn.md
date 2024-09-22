# FM (OPN) instrument editor

the FM editor is divided into 7 tabs:

- **FM**: for controlling the basic parameters of FM sound source.
- **Macros (FM)**: for macros controlling algorithm, feedback and LFO.
- **Macros (OP1)**: for macros controlling FM parameters of operator 1.
- **Macros (OP2)**: for macros controlling FM parameters of operator 2.
- **Macros (OP3)**: for macros controlling FM parameters of operator 3.
- **Macros (OP4)**: for macros controlling FM parameters of operator 4.
- **Macros**: for other macros (volume/arp/pitch).

## FM

OPN is four-operator, meaning it takes four oscillators to produce a single sound.

these apply to the instrument as a whole:
- **Algorithm (ALG)**: determines how operators are connected to each other (0 to 7).
  - left-click pops up a small "operators changes with volume?" dialog where each operator can be toggled to scale with volume level.
  - right-click to switch to a preview display of the waveform generated on a new note:
    - left-click restarts the preview.
    - middle-click pauses and unpauses the preview.
    - right-click returns to algorithm view.
- **Feedback (FB)**: determines how many times operator 1 returns its output to itself (0 to 7).

- **LFO > Freq (FMS)**: determines how much will LFO have an effect in frequency (0 to 7).
- **LFO > Amp (AMS)**: determines how much will LFO have an effect in volume (0 to 3).
  - only applies to operators which have AM turned on.
  - does not apply to YM2203.

these apply to each operator:
- the crossed-arrows button can be dragged to rearrange operators.
- the **OP1**, **OP2**, **OP3**, and **OP4** buttons enable or disable those operators.
- **Amplitude Modulation (AM)**: makes the operator's volume affected by LFO.
  - does not apply to YM2203.
- **Attack Rate (AR)**: determines the rising time for the sound. the bigger the value, the faster the attack (0 to 31).
- **Decay Rate (DR)**: determines the diminishing time for the sound. the higher the value, the shorter the decay. it's the initial amplitude decay rate (0 to 31).
- **Sustain Level (SL)**: determines the point at which the sound ceases to decay and changes to a sound having a constant level. the sustain level is expressed as a fraction of the maximum level (0 to 15).
- **Decay Rate 2 (D2R) / Sustain Rate (SR)**: determines the diminishing time for the sound. the higher the value, the shorter the decay. this is the long "tail" of the sound that continues as long as the key is depressed (0 to 31).
- **Release Rate (RR)**: determines the rate at which the sound disappears after note off. the higher the value, the shorter the release (0 to 15).
- **Total Level (TL)**: represents the envelope’s highest amplitude, with 0 being the largest and 127 (decimal) the smallest. a change of one unit is about 0.75 dB.
- **Hardware Envelope Generator (SSG-EG)**: executes the built-in envelope, inherited from AY-3-8910 PSG. speed of execution is controlled via envelope parameters.

![FM ADSR chart](FM-ADSRchart.png)

- **Envelope Scale (RS/KS)**: also known as "Key Scale" or "Rate Scale". determines the degree to which the envelope execution speed increases according to the pitch (0 to 3).
- **Frequency Multiplier (MULT)**: sets the coarse pitch offset in relation to the note (0 to 15). the values follow the harmonic scale. for example, 0 is -1 octave, 1 is 0 octaves, 2 is 1 octave, 3 is 1 octave 7 semitones, and so on.
- **Fine Detune (DT)**: shifts the pitch a little (0 to 7).


## macros

these macros allow you to control several parameters of FM per tick.

## FM Macros

- **LFO Speed**: LFO frequency.
- **OpMask**: toggles each operator.

## OP1-OP4 Macros

all parameters are listed above.

## Macros

- **Arpeggio**: pitch change sequence in semitones.
- **Panning**: toggles output on left and right channels.
- **Pitch**: fine pitch.
  - **Relative**: when enabled, pitch changes are relative to the current pitch.
- **Phase Reset**: restarts all operators and resets the waveform to its start.


## links

[FM instrument tutorial](https://www.youtube.com/watch?v=wS8edjurjDw): A great starting point to learn how create and work with FM sounds. this was made for DefleMask, but all the same principles apply.
