# settings

settings are saved when clicking the **OK** button at the bottom of the dialog.



## General

### Program

- **Render backend**
  - changing this may help with performace issues.
- **Late render clear**
- **Power-saving mode**: save power by lowering the frame rate to 2fps when idle.
  - may cause issues under Mesa drivers!
- **Disable threaded input (restart after changing!)**: process key presses for note preview on a separate thread (on supported platforms), which reduces latency.
  - however, crashes have been reported when threaded input is on. enable this option if that is the case.
- **Enable event delay**
  - may cause issues with high-polling-rate mice when previewing notes.

### File

- **Use system file picker**: use native OS file dialog instead of Furnace's.
- **Number of recent files**: number of files to show in the _open recent..._ menu.
- **Compress when saving**
  - use zlib to compress saved songs.
- **Save unused patterns**
- **Use new pattern format when saving**
- **Don't apply compatibility flags when loading .dmf**
- **Audio export loop/fade out time:**
  - **Set to these values on start-up:**
    - **Loops**: number of additional times to play through `0Bxx` song loop.
    - **Fade out (seconds)**: length of fade out after final loop.
  - **Remember last values**

### Chip

- **Initial system**: the system of chips loaded on starting Furnace.
  - **Current system**: set current chips as default.
  - **Randomize**: set default to a random system.
    - this will not choose a random system at each start.
  - **Reset to defaults**: set default to "Sega Genesis/Mega Drive".
  - **Name**: name for the default system. may be set to any text.
  - **Configure:**: same as in the [chip manager](../8-advanced/chip-manager.md) and [mixer](../8-advanced/mixer.md).
- **When creating new song**:
  - **Display system preset selector**
  - **Start with initial system**
- **Restart song when changing chip properties**

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

- **Backend**: select SDL or JACK for audio output.
  - only appears on Linux, or MacOS compiled with JACK support
- **Driver**
- **Device**: audio device for playback.
- **Sample rate**
- **Outputs**: select number of audio outputs created, up to 16.
  - only appears when Backend is JACK.
- **Channels**: number of output channels to use.
- **Buffer size**: size of buffer in both samples and milliseconds.
- **Low-latency mode (experimental!)**: reduce latency by running the engine faster than the tick rate. useful for live playback/jam mode.
  - _warning:_ experimental! may produce glitches. only enable if your buffer size is small (10ms or less).
- **Force mono audio**
- **want:** displays requested audio configuration.
- **got:** displays actual audio configuration returned by audio backend.

### Mixing

- **Quality**: select quality of resampling. low quality reduces CPU load.
- **Software clipping**: clip output to nominal range (-1.0 to 1.0) before passing it to the audio device.
  - this avoids activating Windows' built-in limiter.

### Metronome

- **Metronome volume**



## MIDI

### MIDI input

- **MIDI input**: input device.
- **Note input**
- **Velocity input**
- **Map MIDI channels to direct channels**
- **Map Yamaha FM voice data to instruments**
- **Program change is instrument selection**
- **Value input style**:
  - **Disabled/custom**
  - **Two octaves (0 is C-4, F is D#5)**
  - **Raw (note number is value)**
  - **Two octaves alternate (lower keys are 0-9, upper keys are A-F)**
  - **Use dual control change (one for each nibble)**
    - **CC of upper nibble**
    - **CC of lower nibble**
  - **Use 14-bit control change**
    - **MSB CC**
    - **LSB CC**
  - **Use single control change**
    - **Control**
- **Per-column control change**
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
- **Volume curve**
- **Actions:**
  - **`+`** button: add a new action.
  - window-with-arrow button: new action with learning! press a button or move a slider/knob/something on your device.
  - each action has the following:
    - **Type**
    - **Channel**
    - **Note/Control**
    - **Velocity/Value**
    - **Action**
    - **Learn**
    - **Remove**

### MIDI output

- **MIDI output**: output device.
- **Output mode:**
  - **Off (use for TX81Z)**
  - **Melodic**
- **Send Program Change**
- **Send MIDI clock**
- **Send MIDI timecode**
  - **Timecode frame rate:**
    - **Closest to Tick Rate**
    - **Film (24fps)**
    - **PAL (25fps)**
    - **NTSC drop (29.97fps)**
    - **NTSC non-drop (30fps)**



## Emulation

### Cores

- **Arcade/YM2151 core**\
  **Genesis/YM2612 core**\
  **SN76489 core**\
  **NES core**\
  **FDS core**\
  **SID core**\
  **POKEY core**\
  **OPN/OPNA/OPNB cores**: all of these are covered in the [guide to choosing emulation cores](../9-guides/emulation-cores.md).

- **PC Speaker strategy**: this is covered in the [PC speaker system doc](../7-systems/pcspkr.md).

- **Sample ROMs:**
  - **OPL4 YRW801 path**
  - **MultiPCM TG100 path**
  - **MultiPCM MU5 path**



## Keyboard

### Keyboard

- **Import**
- **Export**
- **Reset defaults**
- several categories of keybinds...
  - click on a keybind then enter a key or key combination to change it
  - right-click to clear the keybind



## Interface

### Layout

- **Workspace layout**
  - **Import**: reads a .ini layout file.
  - **Export**: writes current layout to a .ini file.
  - **Reset**: resets layout to default.
- **Allow docking editors**
- **Remember window position**: remember the window's last position on start-up.
- **Only allow window movement when clicking on title bar**
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
- **Toggle channel solo on:** select which interactions with a channel header will toggle solo for that channel.
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

- **Automatic UI scaling factor**: automatically match the OS's UI scaling.
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

### Orders

- **Highlight channel at cursor in Orders**
- **Orders row number format:**
  - **Decimal**
  - **Hexadecimal**

### Pattern

- **Center pattern view**: center pattern horizontally in view.
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

- **Channel style:** set the appearance of channel headers in pattern view.
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

- **Unified instrument/wavetable/sample list**
- **Horizontal instrument list**
- **Colorize instrument editor using instrument type**

### Macro Editor

- **Macro editor layout:**
  - **Unified**
  - **Mobile**
  - **Grid**
  - **Single (with list)**
  - **Single (combo box)**
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
- **Unsigned FM detune values**: use internal representation of detune values, such that detune amounts of -1, -2, and -3 are shown as 5, 6, and 7.

### Statistics

- **Chip memory usage unit:** unit for displaying memory usage in the Statistics window.
  - **Bytes**
  - **Kilobytes**

### Oscilloscope

- **Rounded corners**
- **Border**
- **Mono**: display a single monaural waveform of all sound mixed together.
  - if turned off, waves will be drawn on top of each other for each output channel. their colors are configurable via _Settings > Color > Color scheme > Oscilloscope > Wave (non-mono)._
- **Anti-aliased**: smooth the lines of the waveform.
  - slight performance cost and slightly buggy.
- **Fill entire window**: remove the gap between the waveform and the edge of the window.
- **Waveform goes out of bounds**: allow the waveform to draw past the top and bottom of the oscilloscope.

### Windows

- **Rounded window corners**
- **Rounded buttons**
- **Rounded menu corners**
- **Borders around widgets**: draw thin borders on buttons, checkboxes, text widgets, and the like.



## Color

### Color scheme

- **Import**
- **Export**
- **Reset defaults**
- **General**
  - **Color scheme type:**
    - **Dark**
    - **Light**
  - **Frame shading**: apply a gradient effect to buttons and input boxes.
- several more categories...
