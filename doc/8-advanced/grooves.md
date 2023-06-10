# grooves

Grooves are macros for speed.

A groove is the equivalent of repeating `0Fxx` commands on each row to get a cycle of speeds. For example, a groove of "6 4 5 3" makes the first row 6 ticks long, the next row 4 ticks, then 5, 3, 6, 4, 5, 3...


![groove](groove.png)

To set the song's groove:
- Open the "Speed" window.
- Click the "Speed" button so it becomes "Speeds" (effectively a groove of two speeds).
- Click again so it becomes "Groove".
- Enter a sequence of up to 16 speeds.


![groove patterns](grooves.png)

The "Grooves" window is for entering preset groove patterns.
- The `+` button adds a new groove pattern; click in the pattern to edit it.
- The `×` buttons remove them.

A single `09xx` command will switch to the matching numbered groove pattern.
