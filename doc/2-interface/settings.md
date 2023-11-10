# settings

settings are saved when clicking the **OK** or **Apply** buttons at the bottom of the dialog.

## General

### Program

- **Render backend**: changing this may help with performace issues.
- **Late render clear**: this option is only useful when using old versions of Mesa drivers. it force-waits for VBlank by clearing after present, reducing latency. default is off.
- **Power-saving mode**: saves power by lowering the frame rate to 2fps when idle. default is on.
  - may cause issues under Mesa drivers!
- **Disable threaded input (restart after changing!)**: processes key presses for note preview on a separate thread (on supported platforms), which reduces latency. default is off.
  - however, crashes have been reported when threaded input is on. enable this option if that is the case.
- **Enable event delay**: may cause issues with high-polling-rate mice when previewing notes. default is off.
- **Per-channel oscilloscope threads**: runs the per-channel oscilloscope in separate threads for a performance boost when there are lots of channels. default is 0 (no additional threads).

### File

- **Use system file picker**: uses native OS file dialog instead of Furnace's. default is on.
- **Number of recent files**: number of files that will be remembered in the _open recent..._ menu. default is 10.
- **Compress when saving**: uses zlib to compress saved songs. default is on.
- **Save unused patterns**: stores unused patterns in a saved song. default is off.
- **Use new pattern format when saving**: stores patterns in the new, optimized and smaller format. only disable if you need to work with older versions of Furnace. default is on.
- **Don't apply compatibility flags when loading .dmf**: does exactly what the option says. your .dmf songs may not play correctly after enabled. default is off.
- **Play after opening song:**
  - **No**: default.
  - **Only if already playing**
  - **Yes**
- **Audio export loop/fade out time:**
  - **Set to these values on start-up:**
    - **Loops**: number of additional times to play through `0Bxx` song loop.
    - **Fade out (seconds)**: length of fade out after final loop.
  - **Remember last values**: default.
- **Store instrument name in .fui**: when enabled, saving an instrument will store its name. this may increase file size. default is on.
- **Load instrument name from .fui**: when enabled, loading an instrument will use the stored name (if present). otherwise, it will use the file name. default is on.

### New Song

- **Initial system**: the system of chips loaded on starting Furnace.
  - **Current system**: sets current chips as default.
  - **Randomize**: sets default to a random system.
    - this will not choose a random system at each start.
  - **Reset to defaults**: sets default to "Sega Genesis/Mega Drive".
  - **Name**: name for the default system. may be set to any text. default is blank.
  - **Configure**: same as in the [chip manager](../8-advanced/chip-manager.md) and [mixer](../8-advanced/mixer.md).
- **When creating new song**:
  - **Display system preset selector**: default.
  - **Start with initial system**
  - **Default author name**

### Start-up

- **Play intro on start-up:**
  - **No**: skips intro entirely. default.
  - **Short**: shows silent title screen briefly.
  - **Full (short when loading song)**: shows animated musical intro unless started with a song (command line, double-clicking a .fur file, etc.)
  - **Full (always)**: always shows animated musical intro.
- **Disable fade-in during start-up**: default is off.
- **About screen party time**: default is off.
  - _warning:_ may cause epileptic seizures.

### Behavior

- **New instruments are blank**: when enabled, adding FM instruments will make them blank (rather than loading the default one). default is off.

## Audio

### Output

- **Backend**: selects a different backend for audio output.
  - SDL: the default one.
  - JACK: the JACK Audio Connection Kit (low-latency audio server). only appears on Linux, or MacOS compiled with JACK support.
  - PortAudio: this may or may not perform better than the SDL backend.
- **Driver**: select a different audio driver if you're having problems with the default one. default is "Automatic".
  - only appears when Backend is SDL.
- **Device**: audio device for playback. default is "&lt;System Default&gt;".
  - if using PortAudio backend, devices will be prefixed with the audio API that PortAudio is going to use:
    - Windows WASAPI: a modern audio API available on Windows Vista and later, featuring an (optional) Exclusive Mode. be noted that your buffer size setting may be ignored.
    - Windows WDM-KS: low-latency, direct to hardware output mechanism. may not work all the time and prevents your audio device from being used for anything else!
    - Windows DirectSound: this is the worst choice. best to move on.
    - MME: an old audio API. doesn't have Exclusive Mode.
    - Core Audio: the only choice in macOS.
    - ALSA: low-level audio output on Linux. may prevent other applications from using your audio device.
- **Sample rate**: audio output rate. default is 44100.
  - a lower rate decreases quality and isn't really beneficial.
  - if using PortAudio backend, be careful about this value.
- **Outputs**: number of audio outputs created, up to 16.
  - only appears when Backend is JACK.
- **Channels**: mono, stereo, or something. default is stereo.
- **Buffer size**: size of buffer in both samples and milliseconds. default is 1024.
  - setting this to a low value may cause stuttering/glitches in playback (known as "underruns" or "xruns").
  - setting this to a high value increases latency.
- **Exclusive mode**: enables Exclusive Mode, which may offer latency improvements.
  - only available on WASAPI devices in the PortAudio backend!
- **Low-latency mode**: reduces latency by running the engine faster than the tick rate. useful for live playback/jam mode. default is off.
  - only enable if your buffer size is small (10ms or less).
- **Force mono audio**: use if you're unable to hear stereo audio (e.g. single speaker or hearing loss in one ear). default is off.
- **want:** displays requested audio configuration.
- **got:** displays actual audio configuration returned by audio backend.

### Mixing

- **Quality**: selects quality of resampling. low quality reduces CPU load by a small amount. default is "High".
- **Software clipping**: clips output to nominal range (-1.0 to 1.0) before passing it to the audio device. default is off.
  - this avoids activating Windows' built-in limiter.
  - this option shall be enabled when using PortAudio backend with a DirectSound device.
- **DC offset correction**: apply a filter to remove DC bias, where the output is overall above or below zero. default is on.

### Metronome

- **Volume**: sets volume of metronome. default is 100%.

### Sample preview

- **Volume**: sets volume of sample preview. default is 50%.

## MIDI

### MIDI input

- **MIDI input**: input device.
- **Note input**: enables note input. disable if you intend to use this device only for binding actions. default is on.
- **Velocity input**: enables velocity input when entering notes in the pattern. default is off.
- **Map MIDI channels to direct channels**: when enabled, notes from MIDI channels will be mapped to channels rather than the cursor position. default is off.
- **Map Yamaha FM voice data to instruments**: when enabled, Furnace will listen for any transmitted Yamaha SysEx patches. default is off.
  - this option is only useful if you have a Yamaha FM synthesizer (e.g. TX81Z).
  - selecting a voice or using the "Voice Transmit?" option will send a patch, and Furnace will create a new instrument with its data.
  - this may also be triggered by clicking on "Receive from TX81Z" in the instrument editor (OPZ only).
- **Program change is instrument selection**: changes the current instrument when a program change event is received. default is on.
- **Value input style**: changes the way values are entered when the pattern cursor is not in the Note column. the following styles are available:
  - **Disabled/custom**: no value input through MIDI.
  - **Two octaves (0 is C-4, F is D#5)**: maps keys in two octaves to single nibble input. default. the layout is:
    ```
    -  octave n  --- octave n+1 -
      1 3   6 8 A   D F   # # # 
     0 2 4 5 7 9 B C E # # # # #
    ```
  - **Raw (note number is value)**: the note number becomes the input value. not useful if you want to input anything above 7F.
  - **Two octaves alternate (lower keys are 0-9, upper keys are A-F)**: maps keys in two octaves, but with a different layout:
    ```
    -  octave n  --- octave n+1 -
      A B   C D E   F #   # # # 
     0 1 2 3 4 5 6 7 8 9 # # # #
    ```
  - **Use dual control change (one for each nibble)**: maps two control change events to the nibbles of a value.
    - **CC of upper nibble**: select the CC number that will change the upper nibble.
    - **CC of lower nibble**: select the CC number that will change the lower nibble.
  - **Use 14-bit control change**: maps two control change events that together form a single 14-bit CC. some MIDI controllers do these.
    - **MSB CC**: select the CC containing the upper portion of the control.
    - **LSB CC**: select the CC containing the lower portion of the control.
  - **Use single control change**: maps one control change event. not useful if you want to input odd numbers.
    - **Control**: select the CC number that will change the value.
- **Per-column control change**: when enabled, you can map several control change events to a channel's columns. default is 1.00.
- **Volume curve**: adjust the velocity to volume curve.
- **Actions**: this allows you to bind note input and control change events to actions.
  - **`+`** button: adds a new action.
  - window-with-arrow button: new action with learning! press a button or move a slider/knob/something on your device.
  - each action has the following:
    - **Type**: type of event.
    - **Channel**: channel of event.
    - **Note/Control**: the note/control change number.
    - **Velocity/Value**: the velocity or control value
    - **Action**: the GUI action to perform.
    - **Learn**: after clicking on this button, do something in your MIDI device and Furnace will map that to this action.
    - **Remove**: remove this action.

### MIDI output

- **MIDI output**: output device.
- **Output mode:**
  - **Off (use for TX81Z)**: don't output anything. use if you plan to use Furnace as sync master, or the "Receive from TX81Z" button in the OPZ instrument editor.
  - **Melodic**: output MIDI events. default.
- **Send Program Change**: output program change events when instrument change commands occur. default is off.
- **Send MIDI clock**: output MIDI beat clock. default is off.
- **Send MIDI timecode**: output MIDI timecode. default is off.
  - **Timecode frame rate**: sets the timing standard used for MIDI timecode.
    - **Closest to Tick Rate**: automatically sets the rate based on the song's Tick Rate.
    - **Film (24fps)**: output at 24 codes per second.
    - **PAL (25fps)**: output at 25 codes per second.
    - **NTSC drop (29.97fps)**: output at ~29.97 codes per second, skipping frames 0 and 1 of each minute that doesn't divide by 10.
    - **NTSC non-drop (30fps)**: output at 30 codes per second.

## Emulation

### Cores

- **Playback Core(s)**: core(s) to use for playback.
- **Render Core(s)**: core(s) to use when exporting audio.

all of these are covered in the [guide to choosing emulation cores](../9-guides/emulation-cores.md).

- **PC Speaker strategy**: this is covered in the [PC speaker page](../7-systems/pcspkr.md).

## Keyboard

### Keyboard

- **Import**
- **Export**
- **Reset defaults**

a list of keybinds is displayed.
- click on a keybind. then enter a key or key combination to change it.
- right-click to clear the keybind.
- the full list is in the [keyboard](keyboard.md) page.

#### note input

the settings for note input keybinds operate differently. each entry in the list of keybinds is made of the following:
- **Key**: key assignment.
- **Type**: type of note input. left-click cycles through "Note", "Note off", "Note release", and "Macro release".
  - note: the list is sorted by type. on changing a key's type, it will instantly move to its new sorting position!
- **Value**: number of semitones above C at the current octave. only appears for note type binds.
- **Remove**: removes the keybind from the list.

below all the binds, select a key from the dropdown list to add it. it will appear at or near the top of the list as a note with value 0.

## Interface

### Layout

- **Workspace layout**
  - **Import**: reads a .ini layout file.
  - **Export**: writes current layout to a .ini file.
  - **Reset**: resets layout to default.
- **Allow docking editors**: when enabled, you'll be able to dock instrument/wave/sample editors. default is on.
- **Remember window position**: remembers the window's last position on start-up. default is on.
- **Only allow window movement when clicking on title bar**: default is on.
- **Center pop-up windows**: default is on.
- **Play/edit controls layout:**
  - **Classic**
  - **Compact**
  - **Compact (vertical)**
  - **Split**: default.
- **Position of buttons in Orders:**
  - **Top**
  - **Left**
  - **Right**: default.

### Mouse

- **Double-click time (seconds)**: maximum time between mouse clicks to recognize them as a double-click. default is 0.30.
- **Don't raise pattern editor on click** default is off.
- **Focus pattern editor when selecting instrument** default is on.
- **Note preview behavior:** allows you to disable note preview when entering notes in the pattern.
  - **Never**: don't preview notes at all.
  - **When cursor is in Note column**: only when the cursor is in the Note column. default.
  - **When cursor is in Note column or not in edit mode**: erm... yeah.
  - **Always**: always preview notes.
- **Allow dragging selection:**
  - **No**: don't allow drag-and-drop.
  - **Yes**: allow drag-and-drop.
  - **Yes (while holding Ctrl only)**: allow drag-and-drop but only when holding Control (Command on macOS). default.
- **Toggle channel solo on:** selects which interactions with a channel header will toggle solo for that channel.
  - **Right-click or double click**: default.
  - **Right-click**
  - **Double-click**
- **Double click selects entire column**: when enabled, double clicking on a cell of the pattern will select the entire column. default is on.

### Cursor behavior

- **Insert pushes entire channel row**: when enabled, pressing Insert will push the entire channel rather than the column at the cursor position. default is on.
- **Pull delete affects entire channel row**: when enabled, pull deleting (Backspace by default) will pull the entire channel rather than the column at the cursor position. default is on.
- **Push value when overwriting instead of clearing it**: in the order list and pattern editors, typing into an already-filled value will shift digits instead of starting fresh. for example:
  - if off (default): moving the cursor onto the value `A5` and typing a "B" results in `0B`.
  - if on: moving the cursor onto the value `A5` and typing a "B" results in `5B`.
- **Effect input behavior:**
  - **Move down**: after entering an effect (or effect value), the cursor moves down.
  - **Move to effect value (otherwise move down)**: after entering an effect, the cursor moves to its value. if entering a value, the cursor moves down. default.
  - **Move to effect value/next effect and wrap around**: after entering an effect or effect value, the cursor moves right. if it was on the last column, it jumps back to the first effect.
- **Delete effect value when deleting effect**: if enabled, deleting effect will also delete its value. default is on.
- **Change current instrument when changing instrument column (absorb)**: if enabled, typing on the instrument column will also select the instrument you've typed. default is off.
- **Remove instrument value when inserting note off/release**: if enabled, inserting a note off or release on a row that has instrument value will remove the instrument value. default is off.
- **Remove volume value when inserting note off/release**: same as above, but for volume. default is off.

### Cursor movement

- **Wrap horizontally:** selects what to do when the cursor hits horizontal boundaries.
  - **No**: don't wrap the cursor. default.
  - **Yes**: wrap the cursor.
  - **Yes, and move to next/prev row**: wrap the cursor and move it to the other row.
- **Wrap vertically:** selects what to do when the cursor hits vertical boundaries.
  - **No**: don't wrap the cursor. default.
  - **Yes**: wrap the cursor.
  - **Yes, and move to next/prev pattern**: wrap the cursor and go to the next/previous order.
  - **Yes, and move to next/prev pattern (wrap around)**: same as the previous option, but also wraps around the song.
- **Cursor movement keys behavior:** allows you to select how much will the cursor move by when pressing cursor movement keys.
  - **Move by one**: default.
  - **Move by Edit Step**
- **Move cursor by edit step on delete**: when deleting, moves the cursor by Edit Step. default is off.
- **Move cursor by edit step on insert (push)**: when inserting, moves the cursor by Edit Step. default is off.
- **Move cursor up on backspace-delete**: when pull deleting (Backspace by default), moves cursor up. default is on.
- **Move cursor to end of clipboard content when pasting**: allows you to choose what happens after pasting.
  - if on (default), the cursor will move to the end of the clipboard content.
  - if off, the cursor won't move.

### Scrolling

- **Change order when scrolling outside of pattern bounds**:
  - **No**: the pattern edit cursor will stay locked within the current order. default.
  - **Yes**: moving the cursor past the edge of the previous or next order will move to that order, but not past the start or end of a song.
  - **Yes, and wrap around song**: as above, but will wrap from song end to start.
- **Cursor follows current order when moving it**: default is on.
  - applies when playback is stopped.
- **Don't scroll when moving cursor**: default is off.
- **Move cursor with scroll wheel**
  - **No**: default.
  - **Yes**
  - **Inverted**

## Appearance

### Scaling

- **Automatic UI scaling factor**: automatically matches the OS's UI scaling. default is on.
- **UI scaling factor**: only appears if "Automatic UI scaling factor" is off. default is 1.00x.
- **Icon size**: default is 16.

### Text

- **Font renderer**: this allows you to select which font renderer library to use. there are two choices:
  - **stb_truetype**: the original font renderer used in Furnace 0.6 and before.
  - **FreeType**: this font renderer has support for more font formats and provides more settings. introduced in Furnace 0.6.1. default.
- **Main font**: font for the user interface. default is "IBM Plex Sans", size 18.
- **Header font**: font for section headers. default is "IBM Plex Sans", size 27.
- **Pattern font** font for the pattern view, the order list, and related. default is "IBM Plex Mono", size 18.
  - if "Custom...", a file path selector will appear.
  - **Size**: font size.
- **Display Japanese characters**, **Display Chinese (Simplified) characters**, **Display Chinese (Traditional) characters** and **Display Korean characters**: only toggle these options if you have enough graphics memory. default for all is off.
  - these are a temporary solution until dynamic font atlas is implemented in Dear ImGui.

#### FreeType-specific settings

- **Anti-aliased fonts**: when enabled, fonts will be rendered smooth. default is on.
- **Support bitmap fonts**: this option allows you to enable the loading of bitmap fonts. default is off.
  - be noted that this may force non-bitmap fonts to undesired sizes!
- **Hinting**: this option allows you to define how crisp fonts are rendered.
  - **Off**: disable font hinting. at small sizes, fonts may be blurry. default.
  - **Slight**: enable slight font hinting.
  - **Normal**: enable font hinting.
  - **Full**: enable harsh font hinting. fonts may look ugly/distorted to some people.
- **Auto-hinter**: some fonts contain hinting data, but some don't. this allows you to select what happens.
  - **Disable**: only rely upon font hinting data.
  - **Enable**: prefer font hinting data if present. default.
  - **Force**: ignore font hinting data.

### Program

- **Title bar:**
  - **Furnace**
  - **Song Name - Furnace**: default.
  - **file_name.fur - Furnace**
  - **/path/to/file.fur - Furnace**
- **Display system name on title bar**: default is on.
- **Display chip names instead of "multi-system" in title bar**. default is off.
- **Status bar:**
  - **Cursor details**: default.
  - **File path**
  - **Cursor details or file path**
  - **Nothing**
- **Capitalize menu bar**: default is off.
- **Display add/configure/change/remove chip menus in File menu**: if enabled, the "manage chips" item in the file menu is split into the four listed items for quick access. default is off.

### Orders

- **Highlight channel at cursor in Orders**: default is on.
- **Orders row number format:**
  - **Decimal**
  - **Hexadecimal**: default.

### Pattern

- **Center pattern view**: centers pattern horizontally in view. default is off.
- **Overflow pattern highlights**: default is off.
- **Display previous/next pattern**: default is on.
- **Pattern row number format:**
  - **Decimal**: default.
  - **Hexadecimal**
- **Pattern view labels:**
  - **Note off (3-char)**: default is "`OFF`".
  - **Note release (3-char)**: default is "`===`".
  - **Macro release (3-char)**: default is "`REL`".
  - **Empty field (3-char)**: default is "`...`".
  - **Empty field (2-char)**: default is "`..`".
- **Pattern view spacing after:** number of pixels of space between columns.
  - **Note**: default is 0.
  - **Instrument**: default is 0.
  - **Volume**: default is 0.
  - **Effect**: default is 0.
  - **Effect value**: default is 0.
- **Single-digit effects for 00-0F**: default is off.
- **Use flats instead of sharps**: default is off.
- **Use German notation**: display `B` notes as `H`, and `A#` notes as `B`. default is off.

### Channel

- **Channel style:** sets the appearance of channel headers in pattern view.
  - **Classic**
  - **Line**: default.
  - **Round**
  - **Split button**
  - **Square border**
  - **Round border**
- **Channel volume bar:**
  - **None**: default.
  - **Simple**
  - **Stereo**
  - **Real**
  - **Real (stereo)**
- **Channel feedback style:**
  - **Off**
  - **Note**: default.
  - **Volume**
  - **Active**
- **Channel font:**
  - **Regular**
  - **Monospace**: default.
- **Center channel name**: default is on.
- **Channel colors:**
  - **Single**
  - **Channel type**: default.
  - **Instrument type**
- **Channel name colors:**
  - **Single**: default.
  - **Channel type**
  - **Instrument type**

### Assets

- **Unified instrument/wavetable/sample list**: combines all three types of assets into one list. default is off.
  - the buttons act as appropriate to the currently selected asset or header.
- **Horizontal instrument list**: when there are more instruments than there is room to display them...
  - if on, scroll horizontally through multiple columns.
  - if off (default), scroll vertically in one long column.
  - only appears if "Unified instrument/wavetable/sample list" is off.
- **Instrument list icon style:**
  - **None**
  - **Graphical icons**: default.
  - **Letter icons**
- **Colorize instrument editor using instrument type**: default is off.
- **Display instrument type menu when adding instrument**: default is on.
  - if turned off, the menu can still be opened by right-clicking the add button.

### Macro Editor

- **Macro editor layout:**
  - **Unified**: default.
  - **Grid**
  - **Single (with list)**
- **Use classic macro editor vertical slider**: default is off.

### Wave Editor

- **Use compact wave editor**

### FM Editor

- **FM parameter names:**
  - **Friendly**: default is on.
  - **Technical**
  - **Technical (alternate)**
- **Use standard OPL waveform names**: default is off.
- **FM parameter editor layout:**
  - **Modern**
  - **Compact (2x2, classic)**
  - **Compact (1x4)**
  - **Compact (4x1)**
  - **Alternate (2x2)**: default.
  - **Alternate (1x4)**
  - **Alternate (4x1)**
- **Position of Sustain in FM editor:**
  - **Between Decay and Sustain Rate**: default.
  - **After Release Rate**
- **Use separate colors for carriers/modulators in FM editor**: default is off.
- **Unsigned FM detune values**: uses the internal representation of detune values, such that detune amounts of -1, -2, and -3 are shown as 5, 6, and 7. default is off.

### Statistics

- **Chip memory usage unit:** unit for displaying memory usage in the Statistics window.
  - **Bytes**
  - **Kilobytes**: default.

### Oscilloscope

- **Rounded corners**: default is on.
- **Border**: default is on.
- **Mono**: displays a single monaural waveform of all sound mixed together. default is on.
  - if turned off, waves will be drawn on top of each other for each output channel. 
  - all colors are configurable via _Settings > Color > Color scheme > Oscilloscope > Wave (non-mono)._
- **Anti-aliased**: smoothes the lines of the waveform. default is on.
  - slight performance cost and slightly buggy.
- **Fill entire window**: removes the gap between the waveform and the edge of the window. default is off.
- **Waveform goes out of bounds**: allows the waveform to draw past the top and bottom of the oscilloscope. default is off.

### Windows

- **Rounded window corners**: default is on.
- **Rounded buttons**: default is on.
- **Rounded menu corners** default is off.
- **Borders around widgets**: draws thin borders on buttons, checkboxes, text widgets, and the like. default is off.



## Color

### Color scheme

- **Import**
- **Export**
- **Reset defaults**
- **General**
  - **Color scheme type:**
    - **Dark**
    - **Light**
  - **Frame shading**: applies a gradient effect to buttons and input boxes.
- several more categories...
