# raw note entry

in some cases, one might find it limiting to enter notes purely by note names. to allow for maximum flexibility with pitch, note entry can be toggled to "raw mode"; this allows for the entry of hexadecimal values that are sent directly to the chip's frequency registers.

the drawback of this flexibility is that the following effects are unavailable to notes in raw mode:
- `00xy` (arpeggio)
- `04xy` (vibrato)
- `E1xy` (quick pitch slide up)
- `E2xy` (quick pitch slide down)
- `E5xx` (pitch)

arpeggio macros will be ignored as well.

bear in mind that chips differ in how their registers represent frequency. some are accumulators, for which higher numbers will mean higher pitch; others are divisors, such that higher numbers result in lower frequencies.
- as an extreme example, chips in Yamaha's OPN series use a split value where the upper three bits select from overlapping frequency blocks and the lower eleven bits determine the divider.

there is no default key associated with toggling raw mode. one must be assigned via the [keyboard settings](./settings.md#note-input).
