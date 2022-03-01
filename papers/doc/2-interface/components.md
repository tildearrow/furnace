# UI components

the user interface consists of several components. this paper describes some of them.

## windows

TODO: image

windows may be moved, collapsed, closed or even docked around the workspace.

to move a window, press and hold the mouse button while on title bar or any empty space on it.
then drag your mouse, and release it to stop moving.

to resize a window, drag any of the bottom corners (marked by triangular tabs).

to collapse a window, click on the triangle in the title bar.
clicking again expands it.

to close a window, click on the `X` at the top right corner.

### arrangement and docking

windows may be docked, which comes in handy.

to dock a window, drag it from its title bar to another location in the workspace or to the location of another window.

while dragging, an overlay with five options will appear, allowing you to select where and how to dock that window.
the options are:

```
        UP
 LEFT CENTER RIGHT
       DOWN
```

drag your mouse cursor to any of the options to dock the window.

if you drag the window to `CENTER`, the window will be maximized to cover the workspace (if you do this on the workspace), or it will appear as another tab (if you do this on a window).

otherwise the window will be split in two, with the first half covered by the window you docked and the second half covered by the other window.

when a window is docked, its title bar turns into a tab bar, and the function provided by the "collapse" triangle at the top left changes.

if this triangle is clicked, a menu will appear with a single option: "Hide tab bar".
selecting this option will hide the tab bar of that window.
to bring it back, click on the top left corner.

to undock a window, drag its tab away from where it is docked. then it will be floating again.

## text fields

TODO: image

text fields are able to hold... text.

click on a text field to start editing, and click away to stop editing.

the following keyboard shortcuts work while on a text field:

- `Ctrl-X`: cut
- `Ctrl-C`: copy
- `Ctrl-V`: paste
- `Ctrl-Z`: undo
- `Ctrl-Y`: redo
- `Ctrl-A`: select all

(replace Ctrl with Command on macOS)

## number input fields

TODO: image

these work similar to text fields, but you may only input numbers.

they also usually have two buttons which allow you to increase/decrease the amount when clicked (and rapidly do so when click-holding).

## sliders

TODO: image

sliders are used for controlling values in a quick manner by being dragged.

alternatively, right-clicking or Ctrl-clicking or a slider (Command-click on macOS) will turn it into a number input field for a short period of time, allowing you to input fine values.
