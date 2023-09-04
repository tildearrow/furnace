# settings

settings are saved when clicking the **OK** or **Apply** buttons at the bottom of the dialog.

## General

### Program

- **Render backend**: changing this may help with performace issues.
- **Late render clear**: this option is only useful when using old versions of Mesa drivers. it force-waits for VBlank by clearing after present, reducing latency.
- **Power-saving mode**: saves power by lowering the frame rate to 2fps when idle.
  - may cause issues under Mesa drivers!
- **Disable threaded input (restart after changing!)**: processes key presses for note preview on a separate thread (on supported platforms), which reduces latency.
  - however, crashes have been reported when threaded input is on. enable this option if that is the case.
- **Enable event delay**: may cause issues with high-polling-rate mice when previewing notes.

### File

- **Use system file picker**: uses native OS file dialog instead of Furnace's.
- **Number of recent files**: number of files that will be remembered in the _open recent..._ menu.
- **Compress when saving**: uses zlib to compress saved songs.
- **Save unused patterns**: stores unused patterns in a saved song.
- **Use new pattern format when saving**: stores patterns in the new, optimized and smaller format. only disable if you need to work with older versions of Furnace.
- **Don't apply compatibility flags when loading .dmf**: does exactly what the option says. your .dmf songs may not play correctly after enabled.
- **Play after opening song:**
  - No
  - Only if already playing
  - Yes
- **Audio export loop/fade out time:**
  - **Set to these values on start-up:**
    - **Loops**: number of additional times to play through `0Bxx` song loop.
    - **Fade out (seconds)**: length of fade out after final loop.
  - **Remember last values**

### Chip

- **Initial system**: the system of chips loaded on starting Furnace.
  - **Current system**: sets current chips as default.
  - **Randomize**: sets default to a random system.
    - this will not choose a random system at each start.
  - **Reset to defaults**: sets default to "Sega Genesis/Mega Drive".
  - **Name**: name for the default system. may be set to any text.
  - **Configure**: same as in the [chip manager](../8-advanced/chip-manager.md) and [mixer](../8-advanced/mixer.md).
- **When creating new song**:
  - **Display system preset selector**
  - **Start with initial system**

### Start-up

- **Play intro on start-up:**
  - **No**: skips intro entirely.
  - **Short**: shows silent title screen briefly.
  - **Full (short when loading song)**: shows animated musical intro unless started with a song (command line, double-clicking a .fur file, etc.)
  - **Full (always)**: always shows animated musical intro.
- **Disable fade-in during start-up**
- **About screen party time**
  - _warning:_ may cause epileptic seizures.

### Behavior

- **New instruments are blank**



## Audio

### Output

- **Backend**: selects a different backend for audio output.
  - SDL: the default one.
  - JACK: the JACK Audio Connection Kit (low-latency audio server). only appears on Linux, or MacOS compiled with JACK support.
  - PortAudio: this may or may not perform better than the SDL backend.
- **Driver**: select a different audio driver if you're having problems with the default one.
  - only appears when Backend is SDL.
- **Device**: audio device for playback.
  - if using PortAudio backend, devices will be prefixed with the audio API that PortAudio is going to use:
    - Windows WASAPI: a modern audio API available on Windows Vista and later, featuring an (optional) Exclusive Mode. be noted that your buffer size setting may be ignored.
    - Windows WDM-KS: low-latency, direct to hardware output mechanism. may not work all the time and prevents your audio device from being used for anything else!
    - Windows DirectSound: this is the worst choice. best to move on.
    - MME: an old audio API. doesn't have Exclusive Mode.
    - Core Audio: the only choice in macOS.
    - ALSA: low-level audio output on Linux. may prevent other applications from using your audio device.
- **Sample rate**: audio output rate.
  - a lower rate decreases quality and isn't really beneficial.
  - if using PortAudio backend, be careful about this value.
- **Outputs**: number of audio outputs created, up to 16.
  - only appears when Backend is JACK.
- **Channels**: mono, stereo or something.
- **Buffer size**: size of buffer in both samples and milliseconds.
  - setting this to a low value may cause stuttering/glitches in playback (known as "underruns" or "xruns").
  - setting this to a high value increases latency.
- **Exclusive mode**: enables Exclusive Mode, which may offer latency improvements.
  - only available on WASAPI devices in the PortAudio backend!
- **Low-latency mode (experimental!)**: reduces latency by running the engine faster than the tick rate. useful for live playback/jam mode.
  - only enable if your buffer size is small (10ms or less).
- **Force mono audio**: use if you're unable to hear stereo audio (e.g. single speaker or hearing loss in one ear).
- **want:** displays requested audio configuration.
- **got:** displays actual audio configuration returned by audio backend.

### Mixing

- **Quality**: selects quality of resampling. low quality reduces CPU load by a small amount.
- **Software clipping**: clips output to nominal range (-1.0 to 1.0) before passing it to the audio device.
  - this avoids activating Windows' built-in limiter.
  - this option shall be enabled when using PortAudio backend with a DirectSound device.

### Metronome

- **Metronome volume**

## MIDI

### MIDI input

- **MIDI input**: input device.
- **Note input**: enables note input. disable if you intend to use this device only for binding actions.
- **Velocity input**: enables velocity input when entering notes in the pattern.
- **Map MIDI channels to direct channels**: when enabled, notes from MIDI channels will be mapped to channels rather than the cursor position.
- **Map Yamaha FM voice data to instruments**: when enabled, Furnace will listen for any transmitted Yamaha SysEx patches.
  - this option is only useful if you have a Yamaha FM synthesizer (e.g. TX81Z).
  - selecting a voice or using the "Voice Transmit?" option will send a patch, and Furnace will create a new instrument with its data.
  - this may also be triggered by clicking on "Receive from TX81Z" in the instrument editor (OPZ only).
- **Program change is instrument selection**: changes the current instrument when a program change event is received.
- **Value input style**: changes the way values are entered when the pattern cursor is not in the Note column. the following styles are available:
  - **Disabled/custom**: no value input through MIDI.
  - **Two octaves (0 is C-4, F is D#5)**: maps keys in two octaves to single nibble input. the layout is:
    - ` - octave n -- octave n+1 -`
    - ` 1 3   6 8 A   D F   # # # `
    - `0 2 4 5 7 9 B C E # # # # #`
  - **Raw (note number is value)**: the note number becomes the input value. not useful if you want to input anything above 7F.
  - **Two octaves alternate (lower keys are 0-9, upper keys are A-F)**: maps keys in two octaves, but with a different layout:
    - ` - octave n -- octave n+1 -`
    - ` A B   C D E   F #   # # # `
    - `0 1 2 3 4 5 6 7 8 9 # # # #`
  - **Use dual control change (one for each nibble)**: maps two control change events to the nibbles of a value.
    - **CC of upper nibble**: select the CC number that will change the upper nibble.
    - **CC of lower nibble**: select the CC number that will change the lower nibble.
  - **Use 14-bit control change**: maps two control change events that together form a single 14-bit CC. some MIDI controllers do these.
    - **MSB CC**: select the CC containing the upper portion of the control.
    - **LSB CC**: select the CC containing the lower portion of the control.
  - **Use single control change**: maps one control change event. not useful if you want to input odd numbers.
    - **Control**: select the CC number that will change the value.
- **Per-column control change**: when enabled, you can map several control change events to a channel's columns.
  - **Instrument**\
    **Volume**\
    **Effect `x` type**\
    **Effect `x` value**
    - **Disabled/custom**
    - **Use dual control change (one for each nibble)**
      - **CC of upper nibble**
      - **CC of lower nibble**
    - **Use 14-bit control change**
      - **MSB CC**
      - **LSB CC**
    - **Use single control change (imprecise)**
      - **Control**
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
  - **Melodic**: output MIDI events.
- **Send Program Change**: output program change events when instrument change commands occur.
- **Send MIDI clock**: output MIDI beat clock.
- **Send MIDI timecode**: output MIDI timecode.
  - **Timecode frame rate**: sets the timing standard used for MIDI timecode.
    - **Closest to Tick Rate**: automatically sets the rate based on the song's Tick Rate.
    - **Film (24fps)**: output at 24 codes per second.
    - **PAL (25fps)**: output at 25 codes per second.
    - **NTSC drop (29.97fps)**: output at ~29.97 codes per second, skipping frames 0 and 1 of each minute that doesn't divide by 10.
    - **NTSC non-drop (30fps)**: output at 30 codes per second.

## Emulation

### Cores

- **Arcade/YM2151 core**\
  **Genesis/YM2612 core**\
  **SN76489 core**\
  **NES core**\
  **FDS core**\
  **SID core**\
  **POKEY core**\
  **OPN/OPNA/OPNB cores**:
  - **Playback Core(s)**: core(s) to use for realtime playback.
  - **Render Core(s)**: core(s) to use for exporting audio.
  - all of these are covered in the [guide to choosing emulation cores](../9-guides/emulation-cores.md).

- **PC Speaker strategy**: this is covered in the [PC speaker system doc](../7-systems/pcspkr.md).

## Keyboard

### Keyboard

- **Import**
- **Export**
- **Reset defaults**
- [grouped list of keybinds...](keyboard.md)
  - click on a keybind then enter a key or key combination to change it
  - right-click to clear the keybind



## Interface

### Layout

- **Workspace layout**
  - **Import**: reads a .ini layout file.
  - **Export**: writes current layout to a .ini file.
  - **Reset**: resets layout to default.
- **Allow docking editors**
- **Remember window position**: remembers the window's last position on start-up.
- **Only allow window movement when clicking on title bar**
- **Center pop-up windows**
- **Play/edit controls layout:**
  - **Classic**
  - **Compact**
  - **Compact (vertical)**
  - **Split**
- **Position of buttons in Orders:**
  - **Top**
  - **Left**
  - **Right**

### Mouse

- **Double-click time (seconds)**: maximum time between mouse clicks to recognize them as a double-click.
- **Don't raise pattern editor on click**
- **Focus pattern editor when selecting instrument**
- **Note preview behavior:**
  - **Never**
  - **When cursor is in Note column**
  - **When cursor is in Note column or not in edit mode**
  - **Always**
- **Allow dragging selection:**
  - **No**
  - **Yes**
  - **Yes (while holding Ctrl only)**
- **Toggle channel solo on:** selects which interactions with a channel header will toggle solo for that channel.
  - Right-click or double click
  - Right-click
  - Double-click
- **Double click selects entire column**

### Cursor behavior

- **Insert pushes entire channel row**
- **Pull delete affects entire channel row**
- **Push value when overwriting instead of clearing it**: in the order list and pattern editors, typing into an already-filled value will shift digits instead of starting fresh.
  - if off: moving the cursor onto the value `A5` and typing a "B" results in `0B`.
  - if on: moving the cursor onto the value `A5` and typing a "B" results in `5B`.
- **Effect input behavior:**
  - **Move down**
  - **Move to effect value (otherwise move down)**
  - **Move to effect value/next effect and wrap around**
- **Delete effect value when deleting effect**
- **Change current instrument when changing instrument column (absorb)**
- **Remove instrument value when inserting note off/release**
- **Remove volume value when inserting note off/release**


### Cursor movement

- **Wrap horizontally:**
  - **No**
  - **Yes**
  - **Yes, and move to next/prev row**
- **Wrap vertically:**
  - **No**
  - **Yes**
  - **Yes, and move to next/prev pattern**
  - **Yes, and move to next/prev pattern (wrap around)**
- **Cursor movement keys behavior:**
  - **Move by one**
  - **Move by Edit Step**
- **Move cursor by edit step on delete**
- **Move cursor by edit step on insert (push)**
- **Move cursor up on backspace-delete**
- **Move cursor to end of clipboard content when pasting**

### Scrolling

- **Change order when scrolling outside of pattern bounds**:
  - **No**: the pattern edit cursor will stay locked within the current order.
  - **Yes**: moving the cursor past the edge of the previous or next order will move to that order, but not past the start or end of a song.
  - **Yes, and wrap around song**: as above, but will wrap from song end to start.
- **Cursor follows current order when moving it**
  - applies when playback is stopped.
- **Don't scroll when moving cursor**
- **Move cursor with scroll wheel**



## Appearance

### Scaling

- **Automatic UI scaling factor**: automatically matches the OS's UI scaling.
- **UI scaling factor**: only appears if "Automatic UI scaling factor" is off.
- **Icon size**

### Text

- **Main font**: overall interface font.\
  **Header font**: font for section headers.\
  **Pattern font** font for the pattern view, the order list, and related.
  - if "Custom...", a file path selector will appear.
  - **Size**: font size.
- **Display Japanese characters**\
  **Display Chinese (Simplified) characters**\
  **Display Chinese (Traditional) characters**\
  **Display Korean characters**
  - only toggle these options if you have enough graphics memory.
  - these are a temporary solution until dynamic font atlas is implemented in Dear ImGui.

### Program

- **Title bar:**
  - **Furnace**
  - **Song Name - Furnace**
  - **file_name.fur - Furnace**
  - **/path/to/file.fur - Furnace**
- **Display system name on title bar**
- **Display chip names instead of "multi-system" in title bar**
- **Status bar:**
  - **Cursor details**
  - **File path**
  - **Cursor details or file path**
  - **Nothing**
- **Capitalize menu bar**
- **Display add/configure/change/remove chip menus in File menu**: if enabled, the "manage chips" item in the file menu is split into the four listed items for quick access.

### Orders

- **Highlight channel at cursor in Orders**
- **Orders row number format:**
  - **Decimal**
  - **Hexadecimal**

### Pattern

- **Center pattern view**: centers pattern horizontally in view.
- **Overflow pattern highlights**
- **Display previous/next pattern**
- **Pattern row number format:**
  - **Decimal**
  - **Hexadecimal**
- **Pattern view labels:**
  - **Note off (3-char)**: default is `OFF`
  - **Note release (3-char)**: default is `===`.
  - **Macro release (3-char)**: default is `REL`.
  - **Empty field (3-char)**: default is `...`.
  - **Empty field (2-char)**: default is `..`.
- **Pattern view spacing after:** number of pixels of space between columns.
  - **Note**
  - **Instrument**
  - **Volume**
  - **Effect**
  - **Effect value**
- **Single-digit effects for 00-0F**
- **Use flats instead of sharps**
- **Use German notation**: display `B` notes as `H`, and `A#` notes as `B`.

### Channel

- **Channel style:** sets the appearance of channel headers in pattern view.
  - **Classic**
  - **Line**
  - **Round**
  - **Split button**
  - **Square border**
  - **Round border**
- **Channel volume bar:**
  - **None**
  - **Simple**
  - **Stereo**
  - **Real**
  - **Real (stereo)**
- **Channel feedback style:**
  - **Off**
  - **Note**
  - **Volume**
  - **Active**
- **Channel font:**
  - **Regular**
  - **Monospace**
- **Center channel name**
- **Channel colors:**
  - **Single**
  - **Channel type**
  - **Instrument type**
- **Channel name colors:**
  - **Single**
  - **Channel type**
  - **Instrument type**

### Assets

- **Unified instrument/wavetable/sample list**: combines all three types of assets into one list.
  - the buttons act as appropriate to the currently selected asset or header.
- **Horizontal instrument list**: when there are more instruments than there is room to display them...
  - if on, scroll horizontally through multiple columns.
  - if off, scroll vertically in one long column.
  - only appears if "Unified instrument/wavetable/sample list" is off.
- **Instrument list icon style:**
  - **None**
  - **Graphical icons**
  - **Letter icons**
- **Colorize instrument editor using instrument type**
- **Display instrument type menu when adding instrument**
  - if turned off, the menu can still be opened by right-clicking the add button.

### Macro Editor

- **Macro editor layout:**
  - **Unified**
  - **Grid**
  - **Single (with list)**
- **Use classic macro editor vertical slider**

### Wave Editor

- **Use compact wave editor**

### FM Editor

- **FM parameter names:**
  - **Friendly**
  - **Technical**
  - **Technical (alternate)**
- **Use standard OPL waveform names**
- **FM parameter editor layout:**
  - **Modern**
  - **Compact (2x2, classic)**
  - **Compact (1x4)**
  - **Compact (4x1)**
  - **Alternate (2x2)**
  - **Alternate (1x4)**
  - **Alternate (4x1)**
- **Position of Sustain in FM editor:**
  - **Between Decay and Sustain Rate**
  - **After Release Rate**
- **Use separate colors for carriers/modulators in FM editor**
- **Unsigned FM detune values**: uses the internal representation of detune values, such that detune amounts of -1, -2, and -3 are shown as 5, 6, and 7.

### Statistics

- **Chip memory usage unit:** unit for displaying memory usage in the Statistics window.
  - **Bytes**
  - **Kilobytes**

### Oscilloscope

- **Rounded corners**
- **Border**
- **Mono**: displays a single monaural waveform of all sound mixed together.
  - if turned off, waves will be drawn on top of each other for each output channel. 
  - all colors are configurable via _Settings > Color > Color scheme > Oscilloscope > Wave (non-mono)._
- **Anti-aliased**: smoothes the lines of the waveform.
  - slight performance cost and slightly buggy.
- **Fill entire window**: removes the gap between the waveform and the edge of the window.
- **Waveform goes out of bounds**: allows the waveform to draw past the top and bottom of the oscilloscope.

### Windows

- **Rounded window corners**
- **Rounded buttons**
- **Rounded menu corners**
- **Borders around widgets**: draws thin borders on buttons, checkboxes, text widgets, and the like.



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
