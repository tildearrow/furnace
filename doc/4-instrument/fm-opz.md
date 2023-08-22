# Yamaha OPM FM synthesis instrument editor

The FM editor is divided into 7 tabs:

- **FM**: for controlling the basic parameters of FM sound source.
- **Macros (FM)**: for macros controlling algorithm, feedback and LFO 
- **Macros (OP1)**: for macros controlling FM parameters of operator 1
- **Macros (OP2)**: for macros controlling FM parameters of operator 2
- **Macros (OP3)**: for macros controlling FM parameters of operator 3
- **Macros (OP4)**: for macros controlling FM parameters of operator 4
- **Macros**: for miscellaneous macros controlling volume, arpeggio, and noise generator.

## FM

The FM synthesizers Furnace supports are four-operator, meaning it takes four oscillators to produce a single sound.

These apply to the instrument as a whole:
- **Feedback (FB)**: Determines how many times operator 1 returns its output to itself. (0-7 range)
- **Algorithm (AL)**: Determines how operators are connected to each other. (0-7 range)
  - Left-click pops up a small "operators changes with volume?" dialog where each operator can be toggled to scale with volume level.
  - Right-click to switch to a preview display of the waveform generated on a new note:
    - Left-click restarts the preview.
    - Middle-click pauses and unpauses the preview.
    - Right-click returns to algorithm view.
- **LFO > Freq**: Determines the amount of LFO frequency changes.
- **LFO > Amp**: Determines the depth of LFO amplitude changes.
- **AM Depth**: The depth that AM (amplitude modulation) takes effect on an operator if the operator has AM enabled.
- **PM Depth**: The depth that PM (phase modulation) takes effect on the LFO.
- **LFO Speed**: How fast the LFO processes. Note that high LFO speeds are very fast. It might seem like it's not very fast if you play a note on a low octave, though.
- **LFO Shape**: The shape that the LFO takes.
  - 0: Sawtooth
  - 1: Square
  - 2: Triangle
  - 3: Random (white noise)
- **AM Depth 2**: Same as AM Depth, but for the 2nd LFO.
- **PM Depth 2**: Same as PM Depth, but for the 2nd LFO.
- **LFO2 Speed**: Same as LFO Speed, but for the 2nd LFO.
- **LFO2 Shape**: Same as LFO Shape, but for the 2nd LFO.

These apply to each operator:
- The crossed-arrows button can be dragged to rearrange operators.
- The **OP1**, **OP2**, **OP3**, and **OP4** buttons enable or disable those operators.
- **Amplitude Modulation (AM)**: Makes the operator affected by LFO.
- **Attack Rate (AR)**: determines the rising time for the sound. The bigger the value, the faster the attack. (0-31 range)
- **Decay Rate (DR)**: Determines the diminishing time for the sound. The higher the value, the shorter the decay. It's the initial amplitude decay rate. (0-31 range)
- **Sustain Level (SL)**: Determines the point at which the sound ceases to decay and changes to a sound having a constant level. The sustain level is expressed as a fraction of the maximum level. (0-15 range)
- **Secondary Decay Rate (DR2) / Sustain Rate (SR)**: Determines the diminishing time for the sound. The higher the value, the shorter the decay. This is the long "tail" of the sound that continues as long as the key is depressed. (0-31 range)
- **Release Rate (RR)**: Determines the rate at which the sound disappears after KEY-OFF. The higher the value, the shorter the release. (0-15 range)
- **Total Level (TL)**: Represents the envelope’s highest amplitude, with 0 being the largest and 127 (decimal) the smallest. A change of one unit is about 0.75 dB.
- **Detune**: Fine detune. It will give your instrument a "phasing" effect. Good for makaing waveforms that aren't static.
- **Detune 2**: Coarse detune. This will give your instrument an atonal sound if used like that, but it could also be used as a way to get a higher fine detune value, given the right multiplier settings.
- **Multiplier**: Multiply the frequency of the operator.
- **EnvScale**: How fast to decay the operator, depending on its pitch. Good for automation of sample-like sounds
- **Waveform**: What waveform the operator should take. Useful for making sounds that sound like more than 4op.
  - Sine: Sinusoidal wave.
  - Triangle: Triangle wave with some of the higher frequencies cut.
  - Cut Sine: Sinusoidal wave but the bottom half of the waveform is cut off.
  - Cut Triangle: Ditto, but with the triangle waveform instead.
  - Squished Sine: A sine wave, but with 2x pitch and the 2nd half of the waveform cut off.
  - Squished Triangle: Ditto, but with a triangle.
  - Squished AbsSine: A sine wave passed through the mathematical `abs()` function, but with the frequency doubled and the 2nd half cut off.
  - Ditto, but with a triangle base waveform instead.
- **Fine**: Fine coarse detune. (Bad name, I know.) This can be used for some more tonal detuned sounds.
- **EnvShift**: Seemingly no effect.
- **Reverb**: How much fake reverb (echo) to apply to an operator. 

![FM ADSR chart](FM-ADSRchart.png)

- **Envelope Scale (KSR)**: Also known as "Key Scale". Determines the degree to which the envelope execution speed increases according to the pitch. (0-3 range)
- **Frequency Multiplier (MULT)**: Determines the operator frequency in relation to the pitch. (0-15 range)
- **Fine Detune (DT)**: Shifts the pitch a little. (0-7 range)
- **Coarse Detune (DT2)**: Shifts the pitch by tens of cents. (0-3 range)


## macros

Macros define the sequence of values passed to the given parameter. Via macro, along with the previously mentioned parameters, the following can be controlled:

## FM Macros

These apply to each operator:
- The crossed-arrows button can be dragged to rearrange operators.
- The **OP1**, **OP2**, **OP3**, and **OP4** buttons enable or disable those operators.
- **Amplitude Modulation (AM)**: Makes the operator affected by LFO.
- **Attack Rate (AR)**: determines the rising time for the sound. The bigger the value, the faster the attack. (0-31 range)
- **Decay Rate (DR)**: Determines the diminishing time for the sound. The higher the value, the shorter the decay. It's the initial amplitude decay rate. (0-31 range)
- **Sustain Level (SL)**: Determines the point at which the sound ceases to decay and changes to a sound having a constant level. The sustain level is expressed as a fraction of the maximum level. (0-15 range)
- **Secondary Decay Rate (DR2) / Sustain Rate (SR)**: Determines the diminishing time for the sound. The higher the value, the shorter the decay. This is the long "tail" of the sound that continues as long as the key is depressed. (0-31 range)
- **Release Rate (RR)**: Determines the rate at which the sound disappears after KEY-OFF. The higher the value, the shorter the release. (0-15 range)
- **Total Level (TL)**: Represents the envelope’s highest amplitude, with 0 being the largest and 127 (decimal) the smallest. A change of one unit is about 0.75 dB.
- **Detune**: Fine detune. It will give your instrument a "phasing" effect. Good for makaing waveforms that aren't static.
- **Detune 2**: Coarse detune. This will give your instrument an atonal sound if used like that, but it could also be used as a way to get a higher fine detune value, given the right multiplier settings.
- **Multiplier**: Multiply the frequency of the operator.
- **EnvScale**: How fast to decay the operator, depending on its pitch. Good for automation of sample-like sounds
You may notice some parameters are missing from here 

## OP1-OP4 Macros

All parameters are listed above.

## Macros

- **Arpeggio**: Pitch change sequence in semitones.
- **Noise Frequency**: specifies the noise frequency in noise mode of YM2151's Channel 8 Operator 4 special mode.
- **Panning**: toggles output on left and right channels.
- **Pitch**: fine pitch.
  - **Relative**: pitch changes are relative to the current pitch, not the note's base pitch.
- **Phase Reset**: Restarts all operators and resets the waveform to its start. Effectively the same as a `0Cxx` retrigger.

# links

[FM instrument tutorial](https://www.youtube.com/watch?v=wS8edjurjDw): A great starting point to learn how create and work with FM sounds. This was made for DefleMask, but all the same principles apply, apart from the macros and parameters and such that aren't present in DefleMask.
