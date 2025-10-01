# Amiga/PCM sound source instrument editor

the Generic Sample instrument editor consists of a sample selector and several macros:

## Sample

- **Sample**: specifies which sample should be assigned to the instrument.
- **Use wavetable**: uses wavetable instead of a sample.
  - only available in Amiga and Generic PCM DAC.
- **Use sample map**: enables mapping different samples to notes. see next section for more information.

### sample map

the sample map allows you to set a sample for each note. this can be used to create more realistic instruments, split key instruments, drum kits and more.

after enabling this option, a table appears with the contents of the sample map.
- the first column represents the input note.
- the second column allows you to type in a sample number for each note.
  - you may press Delete to clear it.
- the third one is used to set the note at which the specified sample will play.
- the fourth and last column provides a combo box for selecting a sample.

you may click and drag in the sample number and note columns to select a range. typing a sample number or note will change the entire range.

you may right-click anywhere in the number and note columns for additional options:
- **set entire map to this note**: sets the note number of all notes to the selected cell's.
- **set entire map to this sample**: sets the sample number of all notes to the selected cell's.
- **reset notes**: resets the sample map's notes to defaults (a chromatic scale).
- **clear map samples**: removes all samples from the map.

## Macros

- **Volume**: volume sequence. does not apply to some chips.
- **Arpeggio**: pitch sequence.
- **Waveform**: waveform sequence.
  - only appears when "Use wavetable" is enabled.
- **Panning (left)**: output level for left channel.
- **Panning (right)**: output level for right channel.
- **Pitch**: fine pitch.
- **Phase Reset**: trigger restart of waveform.
