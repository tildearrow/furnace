# menu bar

the menu bar allows you to select from five menus: file, edit, settings, window and help.

## file

- **new...**: opens the new song dialog to choose a system.
  - click a system name to create a new song with it.
  - some systems have several variants, which are inside a group.
- **open...**: opens the file picker, allowing you to select a song to open.
  - see [file formats](formats.md) for a list of formats Furnace is able to open.
- **open recent**: contains a list of the songs you've opened before.
  - **clear history**: erases the file history.
- **save**: saves the current song.
  - opens the file picker if this is a new song, or a backup.
- **save as...**: opens the file picker, allowing you to save the song under a different name.
- **export...**: allows you to export your song into other formats, such as audio files, VGM and more. see the [export](export.md) page for more information.
- **manage chips**: opens the [Chip Manager](../8-advanced/chip-manager.md) dialog.
- **restore backup**: restores a previously saved backup.
  - Furnace keeps up to 5 backups of a song.
  - the backup directory is located in:
    - Windows: `%USERPROFILE%\AppData\Roaming\furnace\backups`
    - macOS: `~/Library/Application Support/Furnace/backups`
    - Linux/other: `~/.config/furnace/backups`
  - this directory grows in size as you use Furnace. remember to delete old backups periodically to save space.
  - **do NOT rely on the backup system as auto-save!** you should save a restored backup because Furnace will not save backups of backups.
- **exit**: closes Furnace.

## edit

- **...**: does nothing except prevent accidental clicks on later menu items if the menu is too tall to fit on the program window.
- **undo**: reverts the last action.
- **redo**: repeats what you undid previously.
- **cut**: moves the current selection in the pattern view to clipboard.
- **copy**: copies the current selection in the pattern view to clipboard.
- **paste**: inserts the clipboard's contents in the cursor position.
  - you may be able to paste from OpenMPT as well.
- **paste special...**: variants of the paste feature.
  - **paste mix**: inserts the clipboard's contents in the cursor position, but does not erase the occupied region.
  - **paste mix (background)**: does the same thing as paste mix, but doesn't alter content which is already there.
  - **paste with ins (foreground)**: same thing as paste mix, but changes the instrument.
  - **paste with ins (background)**: same thing as paste mix (background), but changes the instrument.
  - **paste flood**: inserts the clipboard's contents in the cursor position, and repeats until it hits the end of a pattern.
  - **paste overflow**: paste, but it will keep pasting even if it runs over another pattern.
- **delete**: clears the contents in the selection.
- **select all**: changes the selection so it covers a larger area.
  - if the selection is wide, it will select the rows in a column.
  - if the selection is tall, it will select the entire column.
  - if a column is already selected, it will select the entire channel.
  - if a channel is already selected, it will select the entire pattern.
- **operation mask**: toggles which columns will be affected by the listed operations. [more information here.](../8-advanced/opmask.md)
- **input latch**: determines which data are placed along with a note. [more information here.](../8-advanced/inputlatch.md)
- **note/octave up/down**: transposes notes in the current selection.
- **values up/down**: changes values in the current selection by ±1 or ±16.
- **transpose**: transpose notes or change values by a specific amount.
- **interpolate**: fills in gaps in the selection by interpolation between values.
- **change instrument...**: changes the instrument number in a selection.
- **gradient/fade...**: replace the selection with a "gradient" that goes from the beginning of the selection to the end.
  - does not affect the note column.
  - **Nibble mode**: when enabled, the fade will be per-nibble (0 to F) rather than per-value (00 to FF).
    - use for effects like `04xy` (vibrato).
- **scale...**: scales values in the selection by a specific amount.
  - use to change volume in a selection for example.
- **randomize**: replaces the selection with random values.
  - does not affect the note column.
  - **Nibble mode**: when enabled, the randomization will be per-nibble (0 to F) rather than per-value (00 to FF).
  - **Set effect:**: only appears when the selection includes an effect column. if enabled, an input box will appear. instead of being randomized, all effect types in the selection will be changed to the value entered.
- **invert values**: `00` becomes `FF`, `01` becomes `FE`, `02` becomes `FD` and so on.
- **flip selection**: flips the selection so it is backwards.
- **collapse/expand amount**: allows you to specify how much to collapse/expand in the next two menu items.
- **collapse**: shrinks the selected contents.
- **expand**: expands the selected contents.
- **collapse pattern**: same as collapse, but affects the entire pattern.
- **expand pattern**: same as expand, but affects the entire pattern.
- **collapse song**: same as collapse, but affects the entire song.
  - it also changes speeds and pattern length to compensate.
- **expand song**: same as expand, but affects the entire song.
  - it also changes speeds and pattern length to compensate.
- **find/replace**: shows [the Find/Replace window](../8-advanced/find-replace.md).
- **clear...**: opens a window that allows you to mass-delete things like songs, unused instruments, and the like.

## settings

- **full screen**: expands the Furnace window so it covers your screen.
- **lock layout**: prevents you from dragging/resizing docked windows, or docking more.
- **pattern visualizer**: toggles pattern view particle effects when the song plays.
- **reset layout**: resets the workspace to its defaults.
- **user systems...**: shows the User Systems window. this is detailed in [the User Systems documentation](../8-advanced/user-systems.md).
- **settings...**: shows the Settings window. these are detailed in [the Settings documentation](settings.md).

## window

all these menu items show or hide their associated windows.

- song
  - **[song comments](../8-advanced/comments.md)**
  - **[song information](song-info.md)**
  - **[subsongs](song-info.md)**
  - **[channels](../8-advanced/channels.md)**
  - **[chip manager](../8-advanced/chip-manager.md)**
  - **[orders](order-list.md)**
  - **[pattern](../3-pattern/README.md)**
  - **[pattern manager](../8-advanced/pat-manager.md)**
  - **[mixer](../8-advanced/mixer.md)**
  - **[compatibility flags](../8-advanced/compat-flags.md)**
- assets
  - **[instruments](../4-instrument/README.md)**
  - **[samples](../6-sample/README.md)**
  - **[wavetables](../5-wave/README.md)**
  - **[instrument editor](../4-instrument/README.md)**
  - **[sample editor](../6-sample/README.md)**
  - **[wavetable editor](../5-wave/README.md)**
- visualizers
  - **[oscilloscope](../8-advanced/osc.md)**
  - **[oscilloscope (per-channel)](../8-advanced/chanosc.md)**
  - **[oscilloscope (X-Y)](../8-advanced/xyosc.md)**
  - volume meter
  - **[tuner](../8-advanced/tuner.md)**
  - **[spectrum](../8-advanced/spectrum.md)**
- tempo
  - **[clock](../8-advanced/clock.md)**
  - **[grooves](../8-advanced/grooves.md)**
  - **[speed](song-info.md)**
- debug
  - **[log viewer](../8-advanced/log-viewer.md)**
  - **[register view](../8-advanced/regview.md)**
  - **[statistics](../8-advanced/stats.md)**
  - **[memory composition](../8-advanced/memory-composition.md)**
- **[effect list](../3-pattern/effects.md)**
- **[play/edit controls](play-edit-controls.md)**
- **[piano/input pad](../8-advanced/piano.md)**
- **[reference music player](../8-advanced/refPlayer.md)**
- **[multi-ins setup](../8-advanced/multi-ins.md)**

## help

- **online manual**: opens the online manual in your default browser.
  - the online manual is frequently updated to match Furnace's development; it may contain material that doesn't apply to the most recent stable release.
- **effect list**: displays the [effect list](../3-pattern/effects.md).
- **debug menu**: this menu contains various debug utilities.
  - unless you are working with the Furnace codebase, it's not useful.
- **inspector**: this option shows the Dear ImGui Metrics/Debugger window.
  - unless you are working with the Furnace codebase, it's not useful.
- **panic**: this resets all chips while the song is playing, effectively silencing everything.
- **welcome screen**: displays the welcome screen that appears when Furnace is first run.
- **about...**: displays the About screen.

at the end of the menu bar, more information may be shown:
- during editing, information about the data under the cursor will be shown here:
  - note or note modifier.
  - instrument number and name.
  - volume in decimal, hex, and percentage.
  - effect type and description.
- during playback, these values will be displayed:
  - `speed/groove @ tick rate (BPM) | order | row | elapsed time`
- if any changes or edits have been made but not yet saved, "modified" will appear.
