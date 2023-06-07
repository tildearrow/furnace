# patterns

the pattern view allows you to edit the song.

![pattern view](pattern.png)

a pattern consists of columns ("channels") and rows.
each column has several subcolumns in this order:

1. note
2. instrument
3. volume
4. effect and effect value (several)

all columns are represented in hexadecimal, except for the note column.

# managing channels

you may mute channels, toggle solo mode, collapse channels or even hide them.

clicking on a channel name mutes that channel.

double-clicking or right-clicking it enables solo mode, in where only that channel will be audible.

clicking the `++` at the top left corner of the pattern view displays additional buttons for channel configuration:

![channel bar](channelbar.png)

to rename and/or hide channels, see the Channels window (window > channels).

![channels](channels.png)

# cursor and selection

you may change the cursor position by clicking anywhere on the pattern.

to select, press and hold the left mouse button. then drag the mouse and release the button to finish selection.

# keyboard layout

## shortcuts

key         | action
------------|-----------------------------------------------------------------
Up/Down     | move cursor up/down by one row or the Edit Step (configurable)
Left/Right  | move cursor left/right
PageUp      | move cursor up by 16 rows
PageDown    | move cursor down by 16 rows
Home        | move cursor to beginning of pattern
End         | move cursor to end of pattern
Shift-Home  | move cursor up by exactly one row, overriding Edit Step
Shift-End   | move cursor down by exactly one row, overriding Edit Step
Shift-Up    | expand selection upwards
Shift-Down  | expand selection downwards
Shift-Left  | expand selection to the left
Shift-Right | expand selection to the right
Backspace   | delete note at cursor and/or pull pattern upwards (configurable)
Delete      | delete selection
Insert      | create blank row at cursor position and push pattern
Ctrl-A      | auto-expand selection (select all)
Ctrl-X      | cut selection
Ctrl-C      | copy selection
Ctrl-V      | paste selection
Ctrl-Z      | undo
Ctrl-Y      | redo
Ctrl-F1     | transpose selection (-1 semitone)
Ctrl-F2     | transpose selection (+1 semitone)
Ctrl-F3     | transpose selection (-1 octave)
Ctrl-F4     | transpose selection (+1 octave)
Space       | toggle note input (edit)

## note input

![keyboard](keyboard.png)

- pressing any of the respective keys will insert a note at the cursor's location, and then advance it by the Edit Step.
- note off turns off the last played note in that channel (key off on FM; note cut otherwise).
- note release triggers macro release (and in FM channels it also triggers key off).
- macro release does the same as above, but does not trigger key off in FM channels.

## instrument/volume input

type any hexadecimal number (0-9 and A-F). the cursor will move by the Edit Step when a suitable value is entered.

## effect input

works like the instrument/volume input.

each effect column has two subcolumns: effect and effect value.
if the effect value is not present, it is treated as `00`.

most effects run until canceled using an effect of the same type with effect value `00`, with some exceptions.

for a list of effects [click here](effects.md).
