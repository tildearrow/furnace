# instrument list

![instruments window](instruments.png)

Buttons from left to right:
- **Add**: pops up a menu to select which type of instrument to add. if only one instrument type is available, the menu is skipped.
  - If the "Display instrument type menu when adding instrument" setting is disabled, this skips the menu and creates an instrument according to the chip under the cursor.
  - Right-clicking always brings up the menu.
- **Duplicate**: Duplicates the currently selected instrument.
- **Open**: Brings up a file dialog to load a file as a new instrument at the end of the list.
- **Save**: Brings up a file dialog to save the currently selected asset.
  - Instruments are saved as Furnace instrument (.fui) files.
  - Wavetables are saved as Furnace wavetable (.fuw) files. 
  - Samples are saved as standard wave (.wav) files.
  - Right-clicking brings up a menu with the applicable items from this list:
    - **save instrument as .dmp...**: saves the selected instrument in DefleMask format.
    - **save wavetable as .dmw...**: saves the selected wavetable in DefleMask format.
    - **save raw wavetable...**: saves the selected wavetable as raw data.
    - **save raw sample...**: saves the selected sample as raw data.
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

Everything from the wavetables list applies here also, with the addition of one button before the Delete button:
- **Preview**: Plays the selected sample at its default note.
  - Right-clicking stops the sample playback.
