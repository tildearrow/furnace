# C64 SID instrument editor

the C64 instrument editor consists of two tabs: "C64" to control various parameters of sound channels, and "Macros" containing several macros.

## C64

- **Waveform**: allows selecting a waveform.
  - more than one waveform can be selected at once. in that case a logical AND mix of waves will occur...
    - due to hardware flaws, the mixing is a bit weird and sounds different between the 6581 and the 8580.
  - noise is an exception. it cannot be used with any of the other waveforms.
- **Attack**: determines the rising time for the sound. the bigger the value, the slower the attack. (0 to 15).
- **Decay**: determines the diminishing time for the sound. the higher the value, the longer the decay (0 to 15).
- **Sustain**: sets the volume level at which the sound stops decaying and holds steady (0 to 15).
- **Release**: determines the rate at which the sound fades out after note off. the higher the value, the longer the release (0 to 15).
- **Duty**: specifies the width of a pulse wave (0 to 4095).
- **Reset duty on new note**: overwrite current duty value with the one that is specified in the instrument on new note.
  - only useful when using relative duty macro.
- **Ring Modulation**: when enabled, the channel's output will be multiplied with the previous channel's.
- **Oscillator Sync**: enables oscillator hard sync. as the previous channel's oscillator finishes a cycle, it resets the period of the channel's oscillator, forcing the latter to have the same base frequency. this can produce a harmonically rich sound, the timbre of which can be altered by varying the synchronized oscillator's frequency.

- **Enable filter**: when enabled, this instrument will go through the filter.
- **Initialize filter**: initializes the filter with the specified parameters:
  - **Cutoff**: the filter's point in where frequencies are cut off (0 to 2047).
  - **Resonance**: amplifies or focuses on the cutoff frequency, creating a secondary peak forms and colors the original pitch (0 to 15).
  - **Filter mode**: sets the filter mode. you may pick one or more of the following:
    - **low**: a low-pass filter. the lower the cutoff, the darker the sound.
    - **high**: a high-pass filter. higher cutoff values result in a less "bassy" sound.
    - **band**: a band-pass filter. cutoff determines which part of the sound is heard (from bass to treble).
    - **ch3off**: mutes channel 3 when enabled. it was originally planned for usage with two registers where program could read current oscillator and envelope outputs, thus making vibrato and SFX generation easier. but who wanted to sacrifice one channel out of three! so aforementioned was just done in software, and the feature was never used.
    - multiple filter modes can be selected simultaneously. for example, selecting both "low" and "high" results in a bandstop (notch) filter.

- **Absolute Cutoff Macro**: when enabled, the cutoff macro will go from 0 to 2047, and it will be absolute (in other words, control the cutoff directly rather than being relative).
- **Absolute Duty Macro**: when enabled, the duty macro will go from 0 to 4095.
- **Don't test before new note**: this option disables the one-tick hard reset and test bit before a new note.

## Macros

- **Volume**: volume sequence.
  - warning: volume sequence is global! this means it controls the chip's volume and therefore affects all channels.
- **Arpeggio**: pitch sequence.
- **Duty**: pulse width sequence.
- **Waveform**: select the waveform used by instrument.
- **Pitch**: fine pitch.
- **Cutoff**: filter cutoff.
- **Filter mode**: select the filter mode.
- **Resonance**: filter resonance sequence.
- **Special**: ring and oscillator sync selector, as well as:
  - **gate bit**:
    - set (1): key on. if previous state was 0 it triggers envelope start/restart; if previous state was 1, it does nothing.
    - reset (0): key off. if previous state was 1 it triggers envelope release; if previous state was 0, it does nothing.
  - **test bit**:
    - set (1): immediately mute channel
      - if the channel is a source of ring mod and/or hard sync, those stop working until the bit is reset.
    - reset (0): unmute channel and restore ring mod/hard sync.
- **Attack**: sets envelope attack speed.
  - if you modify attack speed when the envelope is in attack phase it immediately changes.
- **Decay**: sets envelope decay speed.
  - if you modify decay speed when envelope is in decay phase it immediately changes.
- **Sustain**: sets envelope sustain level.
  - if you modify sustain level when envelope is in sustain phase it immediately changes, although you can only go down. for example, 9-to-8 and 8-to-8 both work, but 8-to-9 immediately mutes the channel.
- **Release**: sets envelope release speed.
  - if you modify release speed when envelope is in release phase it immediately changes.
