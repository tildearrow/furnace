To generate a new letter icon:

- Open `icons.sfd` in FontForge.
- Open "IBM Plex Sans Medium" to the side. Return to the icons font.
- Copy char EFF0 (the one with just two baselines) to the new codepoint.
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
- Use `Element | Transformations | Transform...` to Scale Uniformly and Move as needed:
  - One letter: 12000%, X -896, Y 120.5.
  - Two letters: 10000%, X -896, Y 204.2.
  - Three letters: 8000%, X -896, Y 288.
  - Two lines: 8000%, X -896, Y 723.5.
- If resulting glyph is too wide, repeat all steps starting with "IBM Plex Sans Condensed Medium" instead.
  - Remember to close the other IBM Plex font or select the proper one in the text dialog.

Note: Codepoints EFF0 to EFF4 are for reference and shouldn't be used.

Note: WSG icon (F01F) has been slightly kerned to fit.
