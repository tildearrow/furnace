# menu bar

the menu bar allows you to select from five menus: file, edit, settings, window and help.

items in _italics_ don't appear in basic mode and are only available in advanced mode.

# file

- **new...**: creates a new song.
- **open...**: opens the file picker, allowing you to select a song to open.
- **open recent**: contains a list of the songs you've opened before.
  - **clear history**: erases the file history.

- **save**: saves the current song.
  - opens the file picker if this is a new song, or a backup.
- **save as...**: opens the file picker, allowing you to save the song under a different name.
- **save as .dmf (1.1.3+)...**: opens the file picker, allowing you to save your song as a .dmf which is compatible with DefleMask 1.1.3 onwards.
  - this will only work with the systems mentioned in the next option, plus:
    - Sega Master System (with FM expansion)
    - NES + Konami VRC7
    - Famicom Disk System
  - only use this option if you really need it. there are features which DefleMask does not support, like some effects and FM macros, so these will be lost.
- **save as .dmf (1.0/legacy)...**: opens the file picker, allowing you to save your song as a .dmf which is compatible with DefleMask Legacy (0.12) or 1.0.
  - this will only work on the following systems:
    - Sega Genesis/Mega Drive (YM2612 + SN76489)
    - Sega Genesis/Mega Drive (YM2612 + SN76489, extended channel 3)
    - Sega Master System
    - Game Boy
    - PC Engine
    - NES
    - Commodore 64
    - Arcade (YM2151 + SegaPCM 5-channel compatibility)
    - Neo Geo CD (DefleMask 1.0+)
  - only use this option if you really need it. there are features which DefleMask does not support, like some effects and FM macros, so these will be lost.

- **export audio...**: opens the file picker, allowing you to export your song to a .wav file. see next section for more details.
- **export VGM...**: opens the file picker, allowing you to export your song to a .vgm file. see next section for more details.
- **export ZSM...**: opens the file picker, allowing you to export your song to a .zsm file. see next section for more details.
  - only available when there's a YM2151 and/or VERA.
- **export command stream...**: export song data to a command stream file. see next section for more details.
  - this option is for developers.

- **manage chips**: opens the [Chip Manager](../8-advanced/chip-manager.md) dialog.

- **restore backup**: restores a previously saved backup.
  - Furnace keeps up to 5 backups of a song.
  - the backup directory is located in:
    - Windows: `%USERPROFILE%\AppData\Roaming\furnace\backups`
    - macOS: `~/Library/Application Support/Furnace/backups`
    - Linux/other: `~/.config/furnace/backups`
  - this directory grows in size as you use Furnace. remember to delete old backups periodically to save space.

- **exit**: I think you know what this does.

## export audio

this option allows you to export your song in .wav format. I know I know, no .mp3 or .ogg export yet, but you can use a converter.

there are two parameters:

- **Loops**: sets the number of times the song will loop.
  - does not have effect if the song ends with `FFxx` effect.
- **Fade out (seconds)**: sets the fade out time when the song is over.
  - does not have effect if the song ends with `FFxx` effect.

and three export choices:

- **one file**: exports your song to one .wav file.
- **multiple files (one per chip)**: exports the output of each chip to .wav files.
- **multiple files (one per channel)**: exports the output of each channel to .wav files.
  - useful for usage with a channel visualizer such as corrscope.

## export VGM

this option allows exporting to a VGM (Video Game Music) file. these can be played back with VGMPlay (for example).

the following settings exist:

- **format version**: sets the VGM format version to use.
  - versions under 1.70 do not support per-chip volumes, and therefore will ignore the Mixer completely.
  - other versions may not support all chips.
  - use this option if you need to export for a quirky player or parser.
    - for example, RYMCast is picky with format versions. if you're going to use this player, select 1.60.
- **loop**: includes loop information. if disabled, the resulting file won't loop.
- **loop trail**: sets how much of the song is written after it loops.
  - the reason this exists is to work around a VGM format limitation in where post-loop state isn't recorded at all.
  - this may change the song length as it appears on a player.
  - **auto-detect**: detect how much to write automatically.
  - **add one loop**: add one more loop.
  - **custom**: allows you to specify how many ticks to add.
    - `0` is effectively none, disabling loop trail completely.
  - this option will not appear if the loop modality isn't set to None as there wouldn't be a need to.
- **chips to export**: select which chips are going to be exported.
  - due to VGM format limitations, you can only select up to two of each chip type.
  - some chips will not be available, either because VGM doesn't support these yet, or because you selected an old format version.
- **add pattern change hints**: this option adds a "hint" when a pattern change occurs. only useful if you're a developer.
  - the format of the "hint" data block that gets written is: `67 66 FE ll ll ll ll 01 oo rr pp pp pp ...`
    - `ll`: length, a 32-bit little-endian number
    - `oo`: order
    - `rr`: initial row (a 0Dxx effect is able to select a different row)
    - `pp`: pattern index (one per channel)
- **direct stream mode**: this option allows DualPCM to work. don't use this for other chips.
  - may or may not play well with hardware VGM players.

click on **click to export** to begin exporting.

## export ZSM

ZSM (ZSound Music) is a format designed for the Commander X16 to allow hardware playback.
it may contain data for either YM2151 or VERA chips.
Calliope is one of the programs that supports playback of ZSM files.

the following settings are available:

- **Tick Rate (Hz)**: select the tick rate the song will run at.
  - I suggest you use the same rate as the song's.
  - apparently ZSM doesn't support changing the rate mid-song.
- **loop**: enables loop. if disabled, the song won't loop.
- **optimize size**: removes unnecessary commands to reduce size.

click on **Begin Export** to... you know.

## export command stream

this option exports a text or binary file which contains a dump of the internal command stream produced when playing the song.

it's not really useful, unless you're a developer and want to use a command stream dump for some reason (e.g. writing a hardware sound driver).

- **export (binary)**: exports in Furnace's own command stream format (FCS). see `export-tech.md` in `papers/` for details.
- **export (text)**: exports the command stream as a text file. only useful for analysis, really.

# edit

- **...**: does nothing except prevent accidental clicks on later menu items if the menu is too tall to fit on the program window.

- **undo**: reverts the last action.
- **redo**: repeats what you undid previously.

- **cut**: moves the current selection in the pattern view to clipboard.
- **copy**: copies the current selection in the pattern view to clipboard.
- **paste**: inserts the clipboard's contents in the cursor position.
- _**paste special...**:_ variants of the paste feature.
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

- _**operation mask**:_ toggles which columns will be affected by the listed operations. [more information here.](../8-advanced/opmask.md)
- _**input latch**:_ determines which data are placed along with a note. [more information here.](../8-advanced/inputlatch.md)

- **note/octave up/down**: transposes notes in the current selection.

- **values up/down**: changes values in the current selection by ±1 or ±16.

- **transpose**: transpose notes or change values by a specific amount.

- **interpolate**: fills in gaps in the selection by interpolation between values.
- **change instrument**: changes the instrument number in a selection.
- **gradient/fade**: replace the selection with a "gradient" that goes from the beginning of the selection to the end.
  - does not affect the note column.
  - **Nibble mode**: when enabled, the fade will be per-nibble (0 to F) rather than per-value (00 to FF).
    - use for effects like `04xy` (vibrato).
- **scale**: scales values in the selection by a specific amount.
  - use to change volume in a selection for example.
- **randomize**: replaces the selection with random values.
  - does not affect the note column.
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

- _**find/replace**:_ shows [the Find/Replace window](../8-advanced/find-replace.md).

- **clear**: allows you to mass-delete things like songs, instruments and the like.

# settings

- **full screen**: expands the Furnace window so it covers your screen.
- **lock layout**: prevents you from dragging/resizing docked windows, or docking more.
- **basic mode**: toggles [Basic Mode](basic-mode.md).
- **visualizer**: toggles pattern view particle effects when the song plays.
- **reset layout**: resets the workspace to its defaults.
- **settings...**: shows the Settings window. these are detailed in [settings.md].

# window

all these menu items show or hide their associated windows.

- [song information](song-info.md)
- [subsongs](song-info.md)
- [speed](song-info.md)
- [instruments](../4-instrument/README.md)
- [wavetables](../5-wave/README.md)
- [samples](../6-sample/README.md)
- [orders](order-list.md)
- [pattern](../3-pattern/README.md)
- _[mixer](mixer.md)_
- _[grooves](grooves.md)_
- _[channels](channels.md)_
- _[pattern manager](pat-manager.md)_
- _[chip manager](chip-manager.md)_
- _[compatibility flags](compat-flags.md)_
- [song comments](comments.md)

- [piano](piano.md)
- [oscilloscope](osc.md)
- [oscilloscopes (per-channel)](chanosc.md)
- [clock](clock.md)
- [register view](regview.md)
- [log viewer](log-viewer.md)
- [stats](stats.md)

# help

- **effect list**: displays the effect list.
- **debug menu**: this menu contains various debug utilities.
  - unless you are working with the Furnace codebase, it's not useful.
- **inspector**: this option shows the Dear ImGui Metrics/Debugger window.
  - unless you are working with the Furnace codebase, it's not useful.
- **panic**: this resets all chips while the song is playing, effectively silencing everything.
- **about...**: displays the About screen.

at the end of the menu bar, more information may be shown:
- during editing, information about the data under the cursor will be shown here:
  - note or note modifier.
  - instrument number and name.
  - volume in decimal, hex, and percentage.
  - effect type and description.
- during playback, the current values of the following will be listed:\
  > speed/groove @ tick rate (BPM) | order | row | elapsed time.
- if any changes or edits have been made but not yet saved, "modified" will appear.
