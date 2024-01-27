# song info

- **Name**: the track's title.
- **Author**: the author(s) of this track.
- **Album**: the associated album name (or the name of the game the song is from).
- **System**: the game console or computer the track is designed for. this is automatically set when creating a new tune, but in advanced mode, it can be changed to anything one wants. the **Auto** button will provide a guess based on the chips in use.

all of this metadata will be included in a VGM export. this isn't the case for an audio export, however.

- _**Tuning (A-4)**_: set tuning based on the note A-4, which should be 440 in most cases. opening an Amiga MOD will set it to 436 for hardware compatibility. available only in advanced mode.

## subsongs

this window allows one to create **subsongs** - multiple individual songs within a single file. each song has its own order list and patterns, but all songs within a file share the same chips, samples, and so forth.

- the drop-down box selects the current subsong.
- the **`+`** button adds a new subsong.
- the **`−`** button permanently deletes the current subsong (unless it's the only one).
- **Name**: title of the current subsong.
- the box at the bottom can store any arbitrary text, like a separate "Comments" box for the current subsong.

## speed

there are multiple ways to set the tempo of a song.

items in _italic_ don't appear in basic mode and are only available in advanced mode.

**Tick Rate**: the frequency of ticks per second, thus the rate at which notes and effects are processed.
- all values are allowed for all chips, though most chips have hardware limitations that mean they should stay at either 60 (approximately NTSC) or 50 (exactly PAL).
- clicking the Tick Rate button switches to a more traditional **Base Tempo** BPM setting.

**Speed**: the number of ticks per row.
- clicking the "Speed" button changes to more complex modes covered in the [grooves](../8-advanced/grooves.md) page.

_**Virtual Tempo**:_ Simulates any arbitrary tempo without altering the tick rate. it does this by adding or skipping ticks to approximate the tempo. the two numbers represent a ratio applied to the actual tick rate. example:
- set tick rate to 150 BPM (60 Hz) and speed to 6.
- set the first virtual tempo number (numerator) to 200.
- set the second virtual tempo number (denominator) to 150.
- the track will play at 200 BPM.
- the ratio doesn't have to match BPM numbers. set the numerator to 4 and the denominator to 5, and the virtual BPM becomes 150 × 4/5 = 120.
- another way to accomplish this with more control over the results is to use grooves. see the page on [grooves](../8-advanced/grooves.md) for details.

_**Divider**:_ Changes the effective tick rate. a tick rate of 60Hz and a divisor of 6 will result in ticks lasting a tenth of a second each!

**Highlight**: sets the pattern row highlights:
- the first value represents the number of rows per beat.
- the second value represents the number of rows per measure.
- these don't have to line up with the music's actual beats and measures. set them as preferred for tracking.
  - note: these values are used for the metronome and calculating BPM.

**Pattern Length**: the length of each pattern in rows. this affects all patterns in the song, and every pattern must be the same length. (Individual patterns can be cut short by `0Bxx`, `0Dxx`, and `FFxx` commands.)

_**Song Length**:_ how many orders are in the order list. decreasing it will hide the orders at the bottom. increasing it will restore those orders; increasing it further will add new orders of all `00` patterns.
