To generate a new letter icon:

- Open `icons.sfd` in FontForge.
- Open "IBM Plex Sans Medium" to the side. Return to the icons font.
- Copy the two baselines from `E0F0` to the new codepoint.
- Open the new codepoint.
- Select the upper baseline.
- Use `Element | Insert Text Outlines...` to place text:
  - Turn off "Scale so text width matches path length".
  - Align "centered".
  - Type first line of text into the box.
  - Click the "Insert" button (might need window resize to be visible).
- Delete the baseline.
- If there's a line 2, repeat for the lower baseline, including deletion.
- Select all.
- Use `Element | Transformations | Transform...` to Scale Uniformly (from glyph origin) and Move as needed:
  - One letter: 12000%, X 896, Y 15.5.
  - Two letters: 10000%, X 896, Y 99.2.
  - Three letters: 8000%, X 896, Y 183.
  - Two lines: 8000%, X 896, Y 618.5.
- If resulting glyph is too wide, repeat all steps starting with "IBM Plex Sans Condensed Medium" instead.
  - Remember to close the other IBM Plex font or select the proper one in the text dialog.

Notes:

- Codepoints `E0F0` to `E0F4` are for reference only.
- The back layer of each of `E0F1` to `E0F4` has lines that show the expected tops and bottoms of text. Round letters and numerals will reach slightly above or below.
- The WSG icon (`E11F`) has been slightly kerned to fit.
