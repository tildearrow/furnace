# song info

- "Name" is for the track's title.
- "Author" is used to list the contributors to a song. If the song is a cover of someone else's track, it's customary to list their name first, followed by `[cv. YourName]`.
- "Album" can be used to store the associated album name, the name of the game the song is from, or whatever.
- "System" is for the game or computer the track is designed for. This is automatically set when creating a new tune, but it can be changed to anything one wants. The "Auto" button will provide a guess based on the chips in use.

All of this metadata will be included in a VGM export. This isn't the case for a WAV export, however.

"Tuning (A-4)" allows one to set tuning based on the note A-4, which should be 440 in most cases. Opening an Amiga MOD will set it to 436 for hardware compatibility.

# subsongs

This window allows one to create **subsongs** – multiple individual songs within a single file. Each song has its own order list and patterns, but all songs within a file share the same chips, samples, and so forth.

- The drop-down box selects the current subsong.
- The `+` button adds a new subsong.
- The `−` button permanently deletes the current subsong (unless it's the only one).
- "Name" sets the title of the current subsong.
- The box at the bottom can store any arbitrary text, like a separate "Comments" box for the current subsong.

# speed

There are multiple ways to set the tempo of a song.

**Tick Rate** sets the frequency of ticks per second, the rate at which notes and effects are processed.
- All values are allowed for all chips, but most chips have hardware limitations that mean they should stay at either 60 (approximately NTSC) or 50 (exactly PAL).
- Clicking the Tick Rate button switches to a more traditional **Base Tempo** BPM setting.

**Speed** sets the number of ticks per row.
- Clicking the "Speed" button changes to more complex modes covered in the [grooves] page.

**Virtual Tempo** simulates any arbitrary tempo without altering the tick rate. It does this by adding or skipping ticks to approximate the tempo. The two numbers represent a ratio applied to the actual tick rate. Example:
- Set tick rate to 150 BPM (60 Hz) and speed to 6.
- Set the first virtual tempo number (numerator) to 200.
- Set the second virtual tempo number (denominator) to 150.
- The track will play at 200 BPM.
- The ratio doesn't have to match BPM numbers. Set the numerator to 4 and the denominator to 5, and the virtual BPM becomes 150 × 4/5 = 120.

**Divider** changes the effective tick rate. A tick rate of 60Hz and a divisor of 6 will result in ticks lasting a tenth of a second each!

**Highlight** sets the pattern row highlights:
- The first value represents the number of rows per beat.
- The second value represents the number of rows per measure.
- These don't have to line up with the music's actual beats and measures. Set them as preferred for tracking. _Note:_ These values are used for the metronome and calculating BPM.

**Pattern Length** is the length of each pattern in rows. This affects all patterns in the song, and every pattern must be the same length. (Individual patterns can be cut short by `0Bxx`, `0Dxx`, and `FFxx` commands.)

**Song Length** shows how many orders are in the order list. Decreasing it will hide the orders at the bottom. Increasing it will restore those orders; increasing it further will add new orders of all `00` patterns.
