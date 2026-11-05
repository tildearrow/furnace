# NES instrument editor

the NES instrument editor consists of two tabs.

## DPCM

this tab is somewhat similar to [the Sample instrument editor](sample.md), but it has been tailored for use with NES' DPCM channel.

- **Sample**: specifies which sample should be assigned to the instrument.
- **Use sample map**: enables mapping different samples to notes. see next section for more information.
  - when this option is disabled, 16 notes (from C-0 to D#1 and repeating) will map to the DPCM channel's 16 possible pitches.

### sample map

the sample map allows you to set a sample for each note.

after enabling this option, a table appears with the contents of the sample map.
- the first column represents the input note.
- the second column allows you to type in a sample number for each note.
  - you may press Delete to clear it.
- the third one is used to set the DPCM pitch at which the specified sample will play.
  - for possible values, refer to the table below.
  - you may press Delete to clear it. if no value is specified, the last pitch is used.
- the fourth column allows you to set the initial delta counter value when playing the sample.
  - this is an hexadecimal number.
  - the range is `00` to `7F`.
  - you may press Delete to clear it. if no value is specified, the delta counter isn't altered.
- the fifth and last column provides a combo box for selecting a sample.

you may click and drag in the sample number, pitch, and delta columns to select a range. typing a value will change the entire range.

you may right-click anywhere in the number, pitch and delta columns for additional options:
- **set entire map to this pitch**: sets the DPCM pitch of all notes to the selected cell's.
- **set entire map to this delta counter value**: sets the initial delta counter value of all notes to the selected cell's.
- **set entire map to this sample**: sets the sample number of all notes to the selected cell's.
- **reset pitches**: resets the sample map's DPCM pitches to defaults (15).
- **clear delta counter values**: removes all delta counter values from the map.
- **clear map samples**: removes all samples from the map.

## Macros

- **Volume**: volume sequence.
- **Arpeggio**: pitch sequence.
- **Duty/Noise**: duty cycle and noise mode.
  - pulse duty cycles:
    - `0`: 12.5%
    - `1`: 25%
    - `2`: 50%
    - `3`: 75%
  - noise modes:
    - `0`: long noise
    - `1`: short noise
- **Pitch**: fine pitch.
- **Phase Reset**: trigger restart of waveform.
