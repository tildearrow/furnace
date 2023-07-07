# settings

settings are saved when clicking the **OK** button at the bottom of the dialog.



# General

- **Workspace layout**
  - **Import**: reads a .ini layout file.
  - **Export**: writes current layout to a .ini file.
  - **Reset**: resets layout to default.

- **Initial system**: the system of chips loaded on starting Furnace.
  - **Current system**: sets current chips as default.
  - **Randomize**: set default to a random system.
    - this will not choose a random system at each start.
  - **Reset to defaults**: sets default to "Sega Genesis/Mega Drive".
  - **Name**: name for the default system. may be set to any text.
  - system configuration: same as in the [chip manager](../8-advanced/chip-manager.md) and [mixer](../8-advanced/mixer.md).

- **Play intro on start-up:**
  - **No**: skips intro entirely.
  - **Short**: shows silent title screen briefly.
  - **Full (short when loading song)**: shows animated musical intro unless started with a song (command line, double-clicking a .fur file, etc.)
  - **Full (always)**: always shows animated musical intro.
- **When creating new song**:
  - **Display system preset selector**
  - **Start with initial system**

- **Double-click time (seconds)**: maximum time between mouse clicks to recognize them as a double-click.
- **Toggle channel solo on:** select which interactions with a channel header will toggle solo for that channel.
- **Push value when overwriting instead of clearing it**: in the order list and pattern editors, typing into an already-filled value will shift digits instead of starting fresh.
  - if off: moving the cursor onto the value `A5` and typing a "B" results in `0B`.
  - if on: with the cursor on the value `A5` and typing a "B" results in `5B`.
- **Move cursor up on backspace-delete**
- **Move cursor by edit step on delete**
- **Change current instrument when changing instrument column (absorb)**
- **Delete effect value when deleting effect**
- **Change order when scrolling outside of pattern bounds**:
  - if off, the pattern edit cursor will stay locked within the current order.
  - if on, moving the cursor past the edge of the previous or next order will move to that order.
- **Move cursor by edit step on insert (push)**
- **Move cursor to end of clipboard content when pasting**
- **Don't scroll when moving cursor**
- **Double click selects entire column**
- **Allow docking editors**
- **Don't raise pattern editor on click**
- **Focus pattern editor when selecting instrument**
- **Restart song when changing chip properties**
- **Use system file picker**: use native OS file dialog instead of Furnace's.
- **Only allow window movement when clicking on title bar**
- **Enable event delay**
  - may cause issues with high-polling-rate mice when previewing notes.
- **Power-saving mode**
  - saves power by lowering the frame rate to 2fps when idle.
  - may cause issues under Mesa drivers!
- **Disable threaded input (restart after changing!)**
  - threaded input processes key presses for note preview on a separate thread (on supported platforms), which reduces latency.
  - however, crashes have been reported when threaded input is on. enable this option if that is the case.
- **Remember window position**
  - remembers the window's last position on start-up.
- **New instruments are blank**
- **Save unused patterns**
- **Compress when saving**
  - use zlib to compress saved songs.
- **Cursor follows current order when moving it**
  - applies when playback is stopped.
- **Audio export loop/fade out time:**
  - **Set to these values on start-up:**
    - **Loops**: number of additional times to play through `0Bxx` song loop.
    - **Fade out (seconds)**: length of fade out after final loop.
  - **Remember last values**
- **Note preview behavior:**
  - **Never**
  - **When cursor is in Note column**
  - **When cursor is in Note column or not in edit mode**
  - **Always**
- **Wrap pattern cursor horizontally:**
  - **No**
  - **Yes**
  - **Yes, and move to next/prev row**
- **Wrap pattern cursor vertically:**
  - **No**
  - **Yes**
  - **Yes, and move to next/prev pattern**
- **Cursor movement keys behavior:**
  - **Move by one**
  - **Move by Edit Step**
- **Effect input cursor behavior:**
  - **Move down**
  - **Move to effect value (otherwise move down)**
  - **Move to effect value/next effect and wrap around**
- **Allow dragging selection:**
  - **No**
  - **Yes**
  - **Yes (while holding Ctrl only)**



# Audio/MIDI

- **Backend**: select SDL or JACK for audio output.
  - only appears on Linux, or MacOS compiled with JACK support
- **Device**: audio device for playback.
- **Sample rate**
- **Outputs**: select number of audio outputs created, up to 16.
  - only appears when Backend is JACK.
- **Channels**: number of output channels to use.
- **Buffer size**: size of buffer in both samples and milliseconds.
- **Quality**: selects quality of resampling. low quality reduces CPU load.
- **Metronome volume**
- **Low-latency mode (experimental!)**: reduces latency by running the engine faster than the tick rate. useful for live playback/jam mode.
  - _warning:_ experimental! may produce glitches. only enable if your buffer size is small (10ms or less).
- **Force mono audio**
- **Software clipping**: clips output to nominal range (-1.0 to 1.0) before passing it to the audio device.
  - this avoids activating Windows' built-in limiter.
- **want:** displays requested audio configuration.
- **got:** displays actual audio configuration returned by audio backend.

- **MIDI input**
- **MIDI output**
- **MIDI input settings**
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
    - **`+`** button: adds a new action.
    - window-with-arrow button: new action with learning! press a button or move a slider/knob/something on your device.
    - each action has the following:
      - **Type**
      - **Channel**
      - **Note/Control**
      - **Velocity/Value**
      - **Action**
      - **Learn**
      - **Remove**

- **MIDI output settings**
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

# Emulation
- **Arcade/YM2151 core**
  - **ymfm**
  - **Nuked-OPM**
- **Genesis/YM2612 core**
  - **Nuked-OPN2**
  - **ymfm**
- **SN76489 core**
  - **MAME**
  - **Nuked-PSG Mod**
- **NES core**
  - **puNES**
  - **NSFplay**
- **FDS core**
  - **puNES**
  - **NSFplay**
- **SID core**
  - **reSID**
  - **reSIDfp**
- **POKEY core**
  - **Atari800 (mzpokeysnd)**
  - **ASAP (C++ port)**
- **OPN/OPNA/OPNB cores**
  - **ymfm only**
  - **Nuked-OPN2 (FM) + ymfm (SSG/ADPCM)**

- **PC Speaker strategy:**
  - **evdev SND_TONE**
  - **KIOCSOUND on /dev/tty1**
  - **/dev/port**
  - **KIOCSOUND on standard output**
  - **outb()**

- **Sample ROMs:**
  - **OPL4 YRW801 path**
  - **MultiPCM TG100 path**
  - **MultiPCM MU5 path**



# Appearance

- **Render driver**
- **Automatic UI scaling factor**: automatically match the OS's UI scaling.
- **UI scaling factor**: only if "Automatic UI scaling factor" is off.
- **Main font**: if "Custom...", a file path selector will appear beneath.
- **Size**
- **Pattern font**: if "Custom...", a file path selector will appear beneath.
- **Size**
- **Icon size**
- **Display Japanese characters**\
  **Display Chinese (Simplified) characters**\
  **Display Chinese (Traditional) characters**\
  **Display Korean characters**
  - only toggle these options if you have enough graphics memory.
  - these are a temporary solution until dynamic font atlas is implemented in Dear ImGui.

- **Number of recent files**

- **Pattern view labels:**
- **Note off (3-char)**: default is `OFF`
- **Note release (3-char)**: default is `===`.
- **Macro release (3-char)**: default is `REL`.
- **Empty field (3-char)**: default is `...`.
- **Empty field (2-char)**: default is `..`.

- **Orders row number format:**
  - **Decimal**
  - **Hexadecimal**
- **Pattern row number format:**
  - **Decimal**
  - **Hexadecimal**
- **FM parameter names:**
  - **Friendly**
  - **Technical**
  - **Technical (alternate)**

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
- **Play/edit controls layout:**
  - **Classic**
  - **Compact**
  - **Compact (vertical)**
  - **Split**
- **Position of buttons in Orders:**
  - **Top**
  - **Left**
  - **Right**
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
- **Macro editor layout:**
  - **Unified**
  - **Mobile**
  - **Grid**
  - **Single (with list)**
  - **Single (combo box)**

- **Namco 163 chip name**

- **Channel colors:**
  - **Single**
  - **Channel type**
  - **Instrument type**
- **Channel name colors:**
  - **Single**
  - **Channel type**
  - **Instrument type**
- **Channel style:**
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

- **Colorize instrument editor using instrument type**
- **Use separate colors for carriers/modulators in FM editor**
- **Unified instrument/wavetable/sample list**
- **Horizontal instrument list**
- **Use standard OPL waveform names**
- **Overflow pattern highlights**
- **Display previous/next pattern**
- **Use German notation**: display `B` notes as `H`, and `A#` notes as `B`.
- **Single-digit effects for 00-0F**
- **Center pattern view**: centers pattern horizontally in view.
- **Unsigned FM detune values**
- **Highlight channel at cursor in Orders**
- **About screen party time**
  - _warning:_ may cause epileptic seizures.

- **Use compact wave editor**
- **Use classic macro editor vertical slider**
- **Rounded window corners**
- **Rounded buttons**
- **Rounded menu corners**
- **Borders around widgets**
- **Disable fade-in during start-up**
  
- **Oscilloscope settings:**
  - **Rounded corners**
  - **Fill entire window**
  - **Waveform goes out of bounds**
  - **Border**

- **Pattern view spacing after:**
  - **Note**
  - **Instrument**
  - **Volume**
  - **Effect**
  - **Effect value**
- **Color scheme**
  - **Import**
  - **Export**
  - **Reset defaults**
  - **General**
    - **Color scheme type:**
      - **Dark**
      - **Light**
    - **Frame shading**
  - several more categories...



# Keyboard

- **Import**
- **Export**
- **Reset defaults**
- several categories of keybinds...
  - click on a keybind then enter a key or key combination to change it
  - right-click to clear the keybind
