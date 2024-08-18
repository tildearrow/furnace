# SID2 instrument editor

the SID2 instrument editor consists of two tabs: "SID2" to control various parameters of sound channels, and "Macros" containing several macros.

## SID2

- **Waveform**: allows selecting a waveform.
  - more than one waveform can be selected at once. in that case a logical AND mix of waves will occur...
    - although with default mix mode it does occur a bit wrong (like on 8580 SID chip). see below what happens when other modes are in use.
  - noise is an exception. it cannot be used with any of the other waveforms.
    - again, only when default mix mode is on.
- **Attack**: determines the rising time for the sound. the bigger the value, the slower the attack. (0 to 15).
- **Decay**: determines the diminishing time for the sound. the higher the value, the longer the decay (0 to 15).
- **Sustain**: sets the volume level at which the sound stops decaying and holds steady (0 to 15).
- **Release**: determines the rate at which the sound fades out after note off. the higher the value, the longer the release (0 to 15).
- **Duty**: specifies the width of a pulse wave (0 to 4095).
- **Ring Modulation**: when enabled, the channel's output will be multiplied with the previous channel's.
- **Oscillator Sync**: enables oscillator hard sync. as the previous channel's oscillator finishes a cycle, it resets the period of the channel's oscillator, forcing the latter to have the same base frequency. this can produce a harmonically rich sound, the timbre of which can be altered by varying the synchronized oscillator's frequency.

- **Enable filter**: when enabled, this instrument will go through the filter.
- **Initialize filter**: initializes the filter with the specified parameters:
  - **Cutoff**: the filter's point in where frequencies are cut off (0 to 4095).
  - **Resonance**: amplifies or focuses on the cutoff frequency, creating a secondary peak forms and colors the original pitch (0 to 255).
  - **Filter mode**: sets the filter mode. you may pick one or more of the following:
    - **low**: a low-pass filter. the lower the cutoff, the darker the sound.
    - **high**: a high-pass filter. higher cutoff values result in a less "bassy" sound.
    - **band**: a band-pass filter. cutoff determines which part of the sound is heard (from bass to treble).
    - multiple filter modes can be selected simultaneously. for example, selecting both "low" and "high" results in a bandstop (notch) filter.

- **Noise Mode**: dictates how noise behaves.
  - 0 means usual "white" noise.
  - 1-3 provide different tonal waves (in other words, small excerpts of noise are looped, creating a wave with tonal sound). mode 1 provides more "pure" tonal sound while modes 2 and 3 provide harsh, rich sounds, which can be further modified by filtering them.
  - when *only* noise wave is enabled, frequency calculation is altered in a way that this noise wave stays in tune, so wave can freely be used to play actual music.
- **Wave Mix Mode**: dictates how different waves on the same channel are mixed together.
  - mode 0 does it the same way as on 8580 SID chip.
  - mode 1 does a bitwise AND between all the enabled waves (including noise!).
  - modes 2 and 3 operate in the same way, but they do bitwise OR and bitwise XOR. 

- **Absolute Cutoff Macro**: when enabled, the cutoff macro will go from 0 to 4095, and it will be absolute (in other words, control the cutoff directly rather than being relative).
- **Absolute Duty Macro**: when enabled, the duty macro will go from 0 to 4095.

## Macros

- **Volume**: volume sequence.
- **Arpeggio**: pitch sequence.
- **Pitch**: fine pitch.
- **Duty**: pulse width sequence.
- **Waveform**: select the waveform used by instrument.
- **Phase Reset**: trigger restart of envelope.
- **Cutoff**: filter cutoff.
- **Filter Mode**: select the filter mode.
- **Filter Toggle**: turns filter on and off.
- **Resonance**: filter resonance sequence.
- **Special**: ring and oscillator sync selector, as well as:
  - **gate bit**:
    - set (1): key on. if previous state was 0 it triggers envelope start/restart; if previous state was 1, it does nothing.
    - reset (0): key off. if previous state was 1 it triggers envelope release; if previous state was 0, it does nothing.
- **Attack**: sets envelope attack speed.
  - if you modify attack speed when the envelope is in attack phase it immediately changes.
- **Decay**: sets envelope decay speed.
  - if you modify decay speed when envelope is in decay phase it immediately changes.
- **Sustain**: sets envelope sustain level.
  - if you modify sustain level when envelope is in sustain phase it immediately changes. note that, unlike SID chips, you can change sustain level in any direction (both 2->5 and 5->2 work and do not trigger envelope release).
- **Release**: sets envelope release speed.
  - if you modify release speed when envelope is in release phase it immediately changes.
- **Noise Mode**: select the noise mode.
- **Wave Mix**: select the waveform mix mode.
