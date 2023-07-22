# find/replace

Furnace has a powerful find-and-replace function that can take the repetitive work out of mass editing.

# find

![find dialog](find-find.png)

all data that can be found within a pattern can be searched for here.

- a find term contains:
  - **Note**: note.\
    **Ins**: instrument.\
    **Volume**: volume.\
    **Effect**: effect type.\
    **Value**: effect value. all of these have the following choices for what data will be found:
    - **ignore**: ignore this.
    - **equals**: match the given value exactly.
    - **not equal**: match everything but the given value.
    - **between**: match anything between and including the given values.
    - **not between**: match anything outside the given range of values.
    - **any**: match all values.
    - **none**: match blanks only.
  - **-**: remove find term. if only one find term exists, it is cleared.
  - **Add effect**: adds another Effect and Value to the term, each set representing additional effects columns.
  - **Remove effect**: removes last Effect and Value from the term.
- **+**: adds another find term.
- **Search range**: restricts the find to the whole **Song**, the current **Selection**, or the currently viewed **Pattern**.
- **Confine to channels**: restricts the find to the selected channels and the channels between them.
- **Match effect position**: chooses how the order of effect types and effect values will matter when finding them.
  - **No**: no attention is paid to what order the effects appear in.
  - **Lax**: matches effects if they appear in the same order as selected above.
  - **Strict**: effects may only match in their correponding effects columns.
- **Find**: finds everything that matches the terms and displays it in a list.
  - the **order**, **row**, and **channel** columns are as they say.
  - the **go** column of buttons will snap the pattern cursor to the location of the find.

# replace

![replace dialog](find-replace.png)

- the replacement term contains:
  - **Note**: note.\
    **Ins**: instrument.\
    **Volume**: volume.\
    **Effect**: effect type.\
    **Value**: effect value. all of these have the following choices for how they alter the found data:
    - **set**: changes found data to this value.
    - **add**: adds this value to the found data. it may be negative for   subtraction. notes are calculated in semitones.
    - **add (overflow)**: as "add" above, but values will wrap around; for example,   adding 13 to `FF` will result in `0C`.
    - **scale**: multiply value to this percentage; for example, scaling `1A` by   `150` results in `27`. not available for "note".
    - **clear**: erases data.
  - **Add effect**: adds another Effect and Value to be replaced according to how   they were found.
  - **Remove effect**: removes last Effect and Value.
- **Effect replace mode**:
  - **Replace matches only**: replaces only the effect columns that match.
  - **Replace matches, then free spaces**: replaces matched effects; if there are effect columns without data, those will be filled in with the additional effect replacements.
  - **Clear effects**: overwrites effect data with replacement effects.
  - **Insert in free spaces**: replaces nothing; replacement effects are inserted in free effects columns when available.
- **Replace**: finds everything from the "Find" tab and replaces it as directed.
