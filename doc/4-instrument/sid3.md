# SID3 instrument editor

the SID3 editor is divided into 8 tabs:

- **SID3**: for controlling the basic parameters of SID3 sound source.
- **Wavetable**: for controlling the wavetable synth.
- **Sample**: for various sample settings.
- **Macros (Filter 1)**: for macros controlling parameters of filter 1.
- **Macros (Filter 2)**: for macros controlling parameters of filter 2.
- **Macros (Filter 3)**: for macros controlling parameters of filter 3.
- **Macros (Filter 4)**: for macros controlling parameters of filter 4.
- **Macros**: for other macros.

## Wavetable

this allows you to enable and configure the Furnace wavetable synthesizer. see [this page](wavesynth.md) for more information.

## Sample

for sample settings, see [the Sample instrument editor](sample.md).

the only differences are the lack of an "Use wavetable" option, and the presence of a "Use sample" one.

## SID3

- **Waveform**: allows selecting a waveform.
  - more than one waveform can be selected at once. in that case a logical AND mix of waves will occur...
    - although with default mix mode it does occur a bit wrong (like on 8580 SID chip). see below what happens when other modes are in use.
- **Special wave**: allows selecting a special wave. the wave preview is to the right.
- **Wavetable channel**: replaces and hides some macros and UI elements, and makes instrument operate with last wavetable/sample channel:
  - **Waveform** macro now selects a wavetable
  - **Duty**, **Special Wave**, **Feedback**, **Noise Phase Reset**, **Noise LFSR bits** and **Wave Mix** macros are hidden
  - **1-Bit Noise** macro now controls wavetable/PCM mode (it becomes **Sample Mode** macro)
- **Inv. left** and **Inv. right**: invert the signal of corresponding stereo channels.
- **Attack**: determines the rising time for the sound. the bigger the value, the slower the attack. (0 to 255).
- **Decay**: determines the diminishing time for the sound. the higher the value, the longer the decay (0 to 255).
- **Sustain**: sets the volume level at which the sound stops decaying and holds or also decays, but with different speed (0 to 255).
- **Sustain rate**: sets the speed at which the sound decays after reaching sustain volume level. (0 to 255).
- **Release**: determines the rate at which the sound fades out after note off. the higher the value, the longer the release (0 to 255).
- **Wave Mix Mode**: dictates how different waves on the same channel are mixed together.
- **Duty**: specifies the width of a pulse wave (0 to 65535).
- **Feedback**: specifies the feedback level (0 to 255).
- **Reset duty on new note**: overwrite current duty value with the one that's specified in the instrument on new note.
- **Absolute Duty Macro**: when enabled, the duty macro will go from 0 to 65535 (in other words, control the duty directly rather than being relative).
- **Ring Modulation**: when enabled, the channel's output will be multiplied with the source channel's.
- **Oscillator Sync**: enables oscillator hard sync. as the source channel's oscillator finishes a cycle, it resets the period of the channel's oscillator, forcing the latter to have the same base frequency. this can produce a harmonically rich sound, the timbre of which can be altered by varying the synchronized oscillator's frequency.
- **Phase Modulation**: when enabled, the channel's phase will be modified with the source channel's signal (signal is taken from filtered channel's output if filters are enabled).
- **Separate noise pitch**: when enabled, the noise frequency/pitch will be controllable via special macros: **Noise Arpeggio** and **Noise Pitch**.

Then follow controls for each of the 4 filters:

- **Enable filter**: when enabled, this instrument will go through the filter.
- **Initialize filter**: initializes the filter with the specified parameters:
  - **Cutoff**: the filter's point in where frequencies are cut off (0 to 65535).
  - **Resonance**: amplifies or focuses on the cutoff frequency, creating a secondary peak forms and colors the original pitch (0 to 255).
  - **Filter mode**: sets the filter mode. you may pick one or more of the following:
    - **low**: a low-pass filter. the lower the cutoff, the darker the sound.
    - **high**: a high-pass filter. higher cutoff values result in a less "bassy" sound.
    - **band**: a band-pass filter. cutoff determines which part of the sound is heard (from bass to treble).
    - multiple filter modes can be selected simultaneously. for example, selecting both "low" and "high" results in a bandstop (notch) filter.
  - **Output volume**: sets the filter output volume (0 to 255).
  - **Distortion level**: dictates how hard the signal is distorted (soft clipping). distortion is slightly asymmetrical (0 to 255).
- **Absolute Cutoff Macro**: when enabled, the cutoff macro will go from 0 to 65535, and it will be absolute.
- **Change cutoff with pitch**: when enabled, the cutoff will be scaled according to the frequency offset from specified note.
  - **Decrease cutoff when pitch increases**: if this is enabled, filter cutoff will decrease if you increase the pitch. if this is disabled, filter cutoff will increase if you increase the pitch.
  - **Cutoff change center note**: this note marks the center frequency at which no cutoff scaling is happening. the further you go from it in each direction, the more the cutoff scaling will be.
  - **Cutoff change strength**: how much cutoff will be scaled.
- **Change resonance with pitch**: when enabled, the resonance will be scaled according to the frequency offset from specified note.
  - **Decrease resonance when pitch increases**: if this is enabled, filter resonance will decrease if you increase the pitch. if this is disabled, filter resonance will increase if you increase the pitch.
  - **Resonance change center note**: this note marks the center frequency at which no resonance scaling is happening. the further you go from it in each direction, the more the resonance scaling will be.
  - **Resonance change strength**: how much resonance will be scaled.
- **Filters connection matrix**: controls routing of the filters' signals.
  - **In**: this column connects the filters to ADSR sound output.
  - next 4 columns make up the inter-filters connection matrix.
  - **Out**: this column connects the filters' output to final channel output.


### special noise LFSR configurations

this table contains a list of LFSR configurations that are automatically detected and brought to tune by Furnace. a short description is given. number needs to be pasted into **Noise LFSR bits** macro. it is recommended to place a single bar in **Noise Phase Reset** macro for the consistency of the wave.

| LFSR config  |                                                                   Description                                                                             |
|--------------|:---------------------------------------------------------------------------------------------------------------------------------------------------------:|
|    524288    | wave very close to [SID2](sid2.md) noise mode 1 wave. tonal, without very harsh overtones.                                                                |
|  541065280   | wave resembling vocals, has two main tones at least 2 octaves apart                                                                                       |
|     2068     | wave very close to SID2 noise mode 3 wave. tonal but with harsh timbre.                                                                                   |
|      66      | wave very close to SID2 noise mode 2 wave. timbre is somewhere in-between SID2's noise mode 1 and noise mode 3 waves.                                     |

if you find more interesting waves, please contact LTVA or tildearrow, so they can be added to Furnace frequency correction routine and to this table.

## Filter `x` macros

- **Cutoff**: filter `x` cutoff sequence.
- **Resonance**: filter `x` resonance sequence.
- **Filter toggle**: turns filter `x` on and off.
- **Filter mode**: select filter `x` mode.
- **Distortion level**: filter `x` distortion level sequence.
- **Output Volume**: filter `x` output volume sequence.
- **Channel Input Connection**: connect filter `x` to channel ADSR output.
- **Channel Output Connection**: connect filter `x` output to final channel output.
- **Connection Matrix Row**: connect other filters' outputs to filter `x` input.

## Macros

- **Volume**: volume sequence.
- **Arpeggio**: pitch sequence.
- **Pitch**: fine pitch.
- **Duty**: pulse width sequence.
- **Waveform**: select the waveform used by instrument.
  - in wavetable channel mode controls the wavetable index.
- **Special Wave**: select the special wave used by instrument.
- **Noise Arpeggio**: noise pitch sequence.
  - this macro is visible only if **Separate noise pitch** option is enabled. otherwise noise pitch is controlled by **Arpeggio** and **Pitch** macros.
- **Noise Pitch**: fine pitch.
  - this macro is visible only if **Separate noise pitch** option is enabled.
- **Panning (left)**: output level for left channel.
- **Panning (right)**: output level for right channel.
- **Channel Inversion**: invert signal of left and right channels.
- **Key On/Off**: envelope release/start again control.
- **Special**: ring, oscillator sync and phase modulation selector.
- **Ring Mod Source**: ring modulation source channel.
- **Hard Sync Source**: oscillator sync source channel.
- **Phase Mod Source**: phase modulation source channel.
- **Feedback**: feedback sequence
- **Phase Reset**: trigger restart of waveform.
- **Noise Phase Reset**: trigger restart of noise accumulator and LFSR.
- **Envelope Reset**: trigger restart of envelope (unlike key on/off, envelope is forced to restart from 0 volume level no matter which volume it is outputting now).
- **Attack**: sets envelope attack speed.
  - if you modify attack speed when the envelope is in attack phase it immediately changes.
- **Decay**: sets envelope decay speed.
  - if you modify decay speed when envelope is in decay phase it immediately changes.
- **Sustain**: sets envelope sustain level.
  - if you modify sustain level when envelope is in sustain phase it immediately changes.
- **Sustain Rate**: sets envelope sustain rate.
  - if you modify sustain rate when envelope is in sustain phase it immediately changes.
- **Release**: sets envelope release speed.
  - if you modify release speed when envelope is in release phase it immediately changes.
- **Noise LFSR bits**: sets feedback bits of noise LFSR.
- **1-Bit Noise**: controls noise mode.
  - in wavetable channel mode it's called **Sample Mode**, and macro controls wave/PCM mode of the last channel.
- **Wave Mix**: select the waveform mix mode.
