# SGU-1 instrument editor

the SGU-1 instrument editor is divided into 7 tabs:

- **FM**: for controlling the basic parameters of the 4-operator FM sound source.
- **Macros (OP1)**: for macros controlling parameters of operator 1.
- **Macros (OP2)**: for macros controlling parameters of operator 2.
- **Macros (OP3)**: for macros controlling parameters of operator 3.
- **Macros (OP4)**: for macros controlling parameters of operator 4.
- **Macros**: for other macros (volume/arp/duty/pan/pitch/filter/sync/ring).
- **Sound Unit**: for sample settings and the hardware sequencer.

## FM

SGU-1 is four-operator, meaning it takes four oscillators to produce a single sound.

like [ESFM](fm-esfm.md), SGU-1 does not have an algorithm selection. instead, it uses a fixed operator arrangement but allows you to independently control the output and modulation input levels of each operator. this allows it to reproduce common four-operator algorithms, as well as unique combinations where operators act as modulators and carriers at the same time.

additionally, SGU-1 extends each operator with hard sync, ring modulation, a waveform parameter (WPAR), and an OPN-style sustain rate (SR/D2R).

- **operator routing preview**: shows how operators are connected with each other and with the audio output (at the bottom).
  - left-click pops up a small "operators changes with volume?" dialog where each operator can be toggled to scale with volume level.
  - right-click switches to a preview display of the waveform generated on a new note:
    - left-click restarts the preview.
    - middle-click pauses and unpauses the preview.
    - right-click returns to algorithm view.

these apply to each operator:

- the crossed-arrows button can be dragged to rearrange operators.
- **Amplitude Modulation (AM)**: makes the operator affected by LFO tremolo.
- **AM Depth (DAM)**: when enabled, LFO tremolo is deeper (1dB off; 4.8dB on).
- **Envelope Delay (DL)**: determines the delay time before the envelope is triggered. the bigger the value, the longer the delay (0 to 7).
  - a change of one unit doubles or halves the delay time (2^(DL+8) samples).
  - a value of 0 results in no delay.
- **Attack Rate (AR)**: determines the rising time for the sound. the bigger the value, the faster the attack (0 to 31).
- **Decay Rate (DR)**: determines the diminishing time for the sound. the higher the value, the shorter the decay. it's the initial amplitude decay rate (0 to 31).
- **Sustain Level (SL)**: determines the point at which the sound ceases to decay and changes to a sound having a constant level. the sustain level is expressed as a fraction of the maximum level (0 to 15).
- **Sustain Rate (D2R/SR)**: determines the rate at which the sound decays after reaching the sustain level while the key is held (0 to 31). this is an OPN-style feature not present in OPL or ESFM.
- **Release Rate (RR)**: determines the rate at which the sound disappears after note off. the higher the value, the shorter the release (0 to 15).
- **Total Level (TL)**: represents the envelope's highest amplitude, with 0 being the largest and 127 being the smallest. a change of one unit is about 0.75 dB.
  - SGU-1 has 7-bit TL (0-127), offering finer control than OPL's 6-bit (0-63).
- **Key Scale Level (KSL)**: also known as "Level Scale". determines the degree to which the amplitude decreases according to the pitch (0 to 3).
- **Key Scale Rate (KSR)**: also known as "Rate Scale". determines the degree to which the envelope execution speed increases according to the pitch (0 to 3).
  - SGU-1 has 2-bit KSR (0-3), offering finer control than OPL's 1-bit (0-1).
- **Frequency Multiplier (MULT)**: sets the coarse pitch ratio in relation to the note (0 to 15).
  - SGU-1 uses OPL-style multiplier mapping: `0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 12, 12, 15, 15` (`0` = half frequency).
- **Detune (DT)**: shifts the pitch in fine steps (0 to 7).
  - SGU-1 detune mapping is: `0=-3`, `1=-2`, `2=-1`, `3=0`, `4=+1`, `5=+2`, `6=+3`, `7=0`.
- **Waveform Select (WS)**: changes the waveform of the operator (0 to 7). see [waveforms](#waveforms) below.
- **Waveform Parameter (WPAR)**: per-operator wave shaping parameter (0 to 15). the meaning depends on the selected waveform. see [waveforms](#waveforms) below.
- **Hard Sync (SYNC)**: when enabled, this operator's phase resets whenever the previous operator's phase wraps around. creates hard-edged, harmonically rich timbres. for operator 1, the previous operator is operator 4.
- **Ring Modulation (RING)**: when enabled, this operator's output is multiplied by the previous operator's output, producing sum and difference frequencies for bell-like or metallic tones. for operator 1, the previous operator is operator 4.
- **Vibrato (VIB)**: makes the operator affected by LFO vibrato.
- **Vibrato Depth (DVB)**: when enabled, vibrato is deeper.

![FM ADSR chart](FM-ADSRchart.png)

### routing controls

- **Output Level (OL)**: sets the output level from this operator to the audio output (0 to 7).
  - 7 is the loudest level and 1 is the softest, while 0 disables audio output.
  - a change of one unit is about 6 dB.
  - this output scaling factor is applied after TL and envelope scaling have been performed.
- **Modulation Input Level (MI)**: sets the modulation level from the previous operator to this operator (0 to 7).
  - 7 is the strongest level and 1 is the weakest, while 0 disables modulation.
  - a change of one unit is about 6 dB.
  - for operator 1 this controls the **feedback level**.
  - this modulation scaling factor is applied after the previous operator's TL and envelope scaling have been performed, but is unaffected by OL above.

### common algorithms

this table contains a list of modulation input/output level values which resemble common algorithms in Yamaha FM chips.

**note**: MI1 is not included as it is the feedback level.

| algorithm                      | OL1 | MI2 | OL2 | MI3 | OL3 | MI4 | OL4 |
|--------------------------------|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| OPN algorithm **0**            |  0  |  7  |  0  |  7  |  0  |  7  |  7  |
| OPN algorithm **4**            |  0  |  7  |  7  |  0  |  0  |  7  |  7  |
| OPN algorithm **6**            |  0  |  7  |  7  |  0  |  7  |  0  |  7  |
| OPN algorithm **7**            |  7  |  0  |  7  |  0  |  7  |  0  |  7  |
| OPL3 algorithm **1**           |  7  |  0  |  0  |  7  |  0  |  7  |  7  |
| OPL3 algorithm **3**           |  7  |  0  |  0  |  7  |  7  |  0  |  7  |
| OPL3 algorithm **1** (variant) |  0  |  7  |  0  |  7  |  7  |  0  |  7  |

### waveforms

each operator can use one of 8 waveforms. the **Waveform Parameter (WPAR)** provides per-operator wave shaping whose meaning depends on the selected waveform.

- `0`: **sine.**
  - WPAR bit 0 (SKEW): shifts the peak position using the channel duty value.
  - WPAR bit 1 (HALF): half-sine - the negative portion is silenced.
  - WPAR bit 2 (ABS): absolute sine - the negative portion is mirrored to positive.
- `1`: **triangle.**
  - same WPAR modifiers as sine (SKEW, HALF, ABS).
- `2`: **sawtooth.**
  - WPAR bit 0: inverted (falling instead of rising).
  - WPAR bits 2:1: quantize the waveform (0: no quantization, 1-3: progressively coarser steps).
- `3`: **pulse.**
  - WPAR `0`: uses the channel pulse width (set with the Duty macro or `12xx` effect).
  - WPAR `1` to `7`: fixed per-operator pulse width of x/8 (1/8, 2/8, etc.).
- `4`: **noise.** white noise using a 32-bit LFSR.
- `5`: **periodic noise.** metallic/tonal noise using a configurable 6-bit LFSR.
  - WPAR bits 1:0 select the tap configuration:
    - `0`: taps 3,4 (~31 states)
    - `1`: taps 2,3 (~31 states)
    - `2`: taps 0,2,3 (~63 states)
    - `3`: taps 0,2,3,5 (~63 states)
  - this is per-operator, allowing different noise timbres in each operator.
- `6`: **reserved.** outputs silence.
- `7`: **sample.** uses PCM sample data as the operator waveform.
  - reads a 1024-sample chunk from sample memory starting at the channel's PCM reset address.
  - the phase (0-1023) indexes into this region, looping naturally through phase wraparound.
  - a sample selector appears in the FM tab when this waveform is selected.

### fixed frequency mode

each operator has a Fixed Frequency mode. once enabled, the operator ignores the channel pitch and derives its frequency from the MUL and DT register values.

the fixed frequency is determined by the formula: `freq = (8 + (MUL * 247 + 7) / 15) << DT`, producing a range of approximately 8 to 32640 Hz.

when fixed frequency mode is enabled, the DT slider controls the octave/scaling factor rather than acting as a detune offset.

## Sound Unit

this tab controls sample settings and the hardware sequencer.

for sample settings, see [the Sample instrument editor](sample.md).

the differences are:

- the presence of a "Use sample" option.

the hardware sequencer allows automating sweep parameters. the available commands are:

- **VOL**: set volume sweep speed, amount, and boundary.
- **PITCH**: set frequency sweep speed, amount, and boundary.
- **CUT**: set cutoff sweep speed, amount, and boundary.
- **WAIT**: pause for a number of ticks.
- **WAIT_REL**: pause until key release.
- **LOOP**: jump to a position in the sequence.
- **LOOP_REL**: jump to a position until key release.

## OP1-OP4 Macros

these macros allow you to control several parameters of each operator per tick.

many operator parameters listed in the FM section above are available as macros.

### operator arpeggio and pitch macros

among the available macros are **Op. Arpeggio** and **Op. Pitch**.

in the current SGU-1 implementation, these do not stack per operator. for each channel, the first operator (OP1 to OP4) with an active Op. Arpeggio macro is used as the arpeggio source, and the first operator with an active Op. Pitch macro is used as the pitch source.

the **Detune (DT)** FM parameter is still respected when using these macros.

### fixed frequency macros

when fixed frequency is enabled for an operator, the editor currently shows **Block** and **FreqNum** in place of **Op. Arpeggio**/**Op. Pitch**.

however, SGU-1 fixed frequency is derived directly from **MULT** and **DT** (see formula above), and fixed-frequency effect/macro routing is not currently implemented in the SGU backend. use **MULT**/**DT** (or the **Fixed Freq** control) to shape fixed pitch.

## Macros

- **Volume**: volume sequence (0 to 127).
- **Arpeggio**: pitch sequence.
- **Duty**: pulse width sequence (0 to 127).
- **Panning**: stereo panning sequence.
- **Pitch**: fine pitch.
  - **Relative**: when enabled, pitch changes are relative to the current pitch.
- **Phase Reset**: restarts all operators and resets the waveform to its start.
- **Cutoff**: filter cutoff sequence (0 to 16383).
- **Resonance**: filter resonance sequence (0 to 255).
  - values that are too high may distort the output!
- **Filter Control**: filter parameter/ring mod sequence (0 to 15).
  - **bit 0**: ring modulation with next channel (wraps from the last channel to channel 1).
  - **bit 1**: low-pass filter. the lower the cutoff, the darker the sound.
  - **bit 2**: high-pass filter. higher cutoff values result in a less "bassy" sound.
  - **bit 3**: band-pass filter. cutoff determines which part of the sound is heard (from bass to treble).
- **Sync Timer**: sets the phase reset timer (0 to 65535).
- **Op. Sync**: per-operator hard sync bitmask (0 to 15).
  - bit 0: operator 1 sync.
  - bit 1: operator 2 sync.
  - bit 2: operator 3 sync.
  - bit 3: operator 4 sync.
- **Op. Ring**: per-operator ring modulation bitmask (0 to 15).
  - bit 0: operator 1 ring mod.
  - bit 1: operator 2 ring mod.
  - bit 2: operator 3 ring mod.
  - bit 3: operator 4 ring mod.
- **Ch. Ring Mod**: enables inter-channel ring modulation (0 or 1).
