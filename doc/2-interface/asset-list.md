# instrument list

![instruments window](instruments.png)

Buttons from left to right:
- **Add**: Creates a new, default instrument.
- **Duplicate**: Duplicates the currently selected instrument.
- **Open**: Brings up a file dialog to load a file as a new instrument at the end of the list.
- **Save**: Brings up a file dialog to save the currently selected instrument.
- **Toggle folders/standard view**: Enables (and disables) folder view, explained below.
- **Move up**: Moves the currently selected instrument up in the list. Pattern data will automatically be adjusted to match.
- **Move down**: Same, but downward.
- **Delete**: Deletes the currently selected instrument. Pattern data will be adjusted to use the next available instrument in the list.

## folder view

![instruments window in folder view](instruments-folder.png)

In folder view, the "Move up" and "Move down buttons disappear and a new one appears:
- **New folder**: Creates a new folder.

Instruments may be dragged from folder to folder and even rearranged within folders without changing their associated numbers.

Right-clicking on a folder allows one to rename or delete it. Deleting a folder does not remove the instruments in it.

# wavetable list

![wavetables window](wavetables.png)

Everything from the instrument list applies here also, with one major difference: Moving waves around with the buttons will change their associated numbers in the list but _not_ in pattern or instrument data. Be careful!

# sample list

![samples window](samples.png)

Everything from the wavetables list applies here also, with the addition of two buttons before the Delete button:
- **Preview**: Plays the selected sample at its default note.
- **Stop preview**: Stops the sample playback.
