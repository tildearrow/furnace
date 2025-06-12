# clipboard format

when copying pattern data from Furnace, it's stored in the clipboard as plain text.

```
org.tildearrow.furnace - Pattern Data (144)
```

this top line of text is always the same except for the number in parentheses, which is the internal format version. for example, 0.6.8.3 is `232`.

the second line is a number between 0 and 18 (decimal) which indicates which column the clip starts from.
- `0`: note.
- `1`: instrument.
- `2`: volume.
- `3`: effect 1 type.
- `4`: effect 1 value. effect type is always included in the clip, even if skipped over.
- `5`: effect 2 type.
- `6`: effect 2 value. effect type is always included in the clip, even if skipped over.
- `7`: effect 3 type...
- ...and so on.

examples of the starting column:

```
org.tildearrow.furnace - Pattern Data (144)
0
D-6007F08080706|...........|
...............|...........|
...............|A#500..080F|
...............|...........|
```

```
org.tildearrow.furnace - Pattern Data (144)
1
007F08080706|...........|
............|...........|
............|A#500..080F|
............|...........|
```

```
org.tildearrow.furnace - Pattern Data (144)
2
7F08080706|...........|
..........|...........|
..........|A#500..080F|
..........|...........|
```

```
org.tildearrow.furnace - Pattern Data (144)
3
08080706|...........|
........|...........|
........|A#500..080F|
........|...........|
```

```
org.tildearrow.furnace - Pattern Data (144)
4
08080706|...........|
........|...........|
........|A#500..080F|
........|...........|
```

```
org.tildearrow.furnace - Pattern Data (144)
5
0706|...........|
....|...........|
....|A#500..080F|
....|...........|
```

```
org.tildearrow.furnace - Pattern Data (144)
6
0706|...........|
....|...........|
....|A#500..080F|
....|...........|
```

```
org.tildearrow.furnace - Pattern Data (144)
0
...........|
...........|
A#500..080F|
...........|
```

each line following the column number is verbatim from the pattern view with channels separated by `|`. each line also ends in `|`.

notes use the default settings for note display (no German notation), including note off `OFF`, note release `===`, and macro release `REL`.
