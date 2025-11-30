# troubleshooting

## info

### configuration

Furnace configuration file locations:

- **Windows**: `%USERPROFILE%\AppData\Roaming\furnace`
- **macOS**: `~/Library/Application Support/Furnace`
- **Linux/other**: `~/.config/furnace`

what the files contain:

- **`layout.ini`**: positions, sizes, and docking for each window and tab.
- **`midiIn_.cfg`**: default MIDI input settings.
- **`midiIn_*.cfg`**: settings for each MIDI device Furnace has seen.
- **`presets.cfg`**: user-created system presets.
- **`furnace.cfg`**: all other settings.
- **`furnace.log`**: log of activity in the current or most recent Furnace session.
- **filenames ending in numbers**: rolling backups. 1 is the most recent.

## known issues

### Furnace doesn't keep track of which tabs are selected in tab groups.

this is a limitation of the Dear ImGui graphical interface framework that Furnace uses.

### something's not working or is unavailable on Android.

Android builds are unofficial and unsupported. there are many known issues, especially with the user interface. always test using a desktop version before reporting problems.

## common problems

### input notes aren't audible or sound different from the instrument, wavetable, or sample editor.

- make sure the pattern editing cursor is in an channel appropriate to the instrument type, and check whether that channel is muted.
- all chips have limits on what notes can be played, whether within a frequency range or from a list of preset frequencies.
- for wavetables, check the size of the wavetable. if it doesn't match what the chip expects, Furnace will automatically resample it as needed. an easy way to find out the correct size is to click the "Add" button in the list of wavetables; the new wavetable will match the chip, or a menu will pop up that displays appropriate sizes.
- many sample-based chips need multiple considerations:
  - sample memory limits. use the sample editor to resize samples to fit or change which ones are loaded into which chip. be sure to reference the memory composition window (in the "window" menu under "debug").
  - sample length and loop positions. if the sample editor shows any highlighted (yellow) boxes or numbers, those will have to be adjusted by moving loop points, resampling, trimming, or padding the sample with silence. see the [sample editor documentation](../6-sample/README.md).
  - sample format. Furnace will auto-convert samples to the correct format for the chip; in some cases, this can make a huge difference in sound.
- see the [chip's documentation](../7-systems/) for more info.

### input notes are audible but aren't appearing in the pattern.

- click anywhere in the pattern editing window to set focus.
- if that doesn't work, toggle edit mode with the space bar and try again.

### multi-ins isn't working.

- there can only be as many instruments playing as there are channels to play them.
- playback may be set to "Mono". find it in the play/edit controls and change it to "Poly" or "Chord".

### changing a note's volume seems to have no effect.

some channels don't have adjustable volume, such as the triangle channel on the NES. see the [chip's documentation](../7-systems/) for more info.

### changing one note's volume or pitch affects other notes.

some chips share the same value across multiple channels, such as the SID's global volume or the frequency of the noise channel and pulse channel 3 on the SN7. see the [chip's documentation](../7-systems/) for more info.

### none of the sample editing buttons are working.

the sample is likely in a format that doesn't allow for direct editing. use the sample type selector to convert it to "16-bit PCM" (or "8-bit PCM") and edit from there.
- Furnace will auto-convert such samples as needed for the format the chip expects. however, this can have unexpected effects due to differences in encoding and chip limitations. see the [chip's documentation](../7-systems/) for more info.

### the track sounds different when exported to an audio file.

check in the "Emulation" tab of the "Settings" window to see if the chip has different playback and render cores, and change them as necessary.

### the track sounds different when exported to a VGM file.

- the VGM format doesn't support all chips, or has incomplete support.
- the VGM format only supports a maximum of two instances of any chip.
- the VGM format cannot store mixer or patchbay settings.
- VGM players may introduce their own inaccuracies, whether due to the software itself (such as the deadbeef VGM player for the MD/Genesis) or the emulator it's run on (common with volume macros on the Game Boy).

### a window that should be visible, isn't.

if you see a title bar but no window, it's in "collapsed" mode. double-click the bar to open the window.

<!-- if it only happened within the last few sessions, try restoring from a backup. open the Settings window and select the Interface tab. the Layout section will have an "Import" button, which opens a file dialog. browse to the Furnace configuration directory and choose a previous layout file (for example, `layout.ini.4`). click "OK" to see the results. -->

if it only disappeared within the last few sessions, try restoring the window layout from a backup:
- close Furnace.
- open the configuration directory.
- delete (or rename) `layout.cfg`.
- rename one of the numbered backup files to replace it. they're numbered in reverse chronological order; `.1` is the most recent.
- start Furnace again to see if the window shows up.

if none of the backups help (or you're comfortable trying manual edits first):
- close Furnace.
- open `layout.ini` in your preferred text editor.
- search the section headers for the name of the window; it will be in square brackets. - delete the `Pos` and `Size` lines beneath it.
- if `Collapsed` is set to `1`, change it to `0`.
- save the file.
- start Furnace; the window should appear in its original position.

if none of this works, you can revert to Furnace's defaults:
- select "reset layout" in the "settings" menu.

### when I try to open a `.fur` file, an error dialog pops up that says it has an "invalid info header".

the module header format has changed considerably in Furnace versions after 0.6.8.3 (or more specifically, dev239 onward) in ways that are not backward compatible. the only solution is to upgrade to a newer version of Furnace. don't worry, it will still open older modules.
