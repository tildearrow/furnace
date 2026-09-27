# Horizontal UI validation

The horizontal UI is isolated in `src/gui/horizontalUI.{h,cpp}`. Row transformations live in `horizontalPattern.h`; settings and desktop window routing are small hooks in the existing GUI. The engine and song format are unchanged.

## Automated row-operation regression tests

From the repository root:

```sh
c++ -std=c++14 -fsanitize=address,undefined -g test/horizontal-pattern.cpp -o /tmp/horizontal-pattern-test
/tmp/horizontal-pattern-test
```

Cases cover adjacent notes, moving and resizing, collision rollback, instrument and volume preservation, stationary effects (including effect column eight), independent release commands, deletion, and pattern bounds.

## Desktop build used for validation

With the required Git submodules initialized and system SDL2 and FreeType installed:

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DSYSTEM_SDL2=ON -DSYSTEM_FREETYPE=ON \
  -DWITH_PORTAUDIO=OFF -DUSE_RTMIDI=OFF -DUSE_BACKWARD=OFF -DWITH_JACK=OFF
cmake --build build -j 8
```

## GUI checks performed

A Debug build was exercised using Xvfb at 1440 x 1000, the software renderer, an isolated configuration, and SDL dummy audio. These check rendering, playback position, and data; they do not constitute a listening test.

- Enabled and disabled horizontal mode through Settings > Interface > Layout, applied the setting, and verified configuration persistence and restoration of the tracker.
- Drew adjacent notes, dragged a note in time and pitch, resized its end, entered `0F06`, and used Undo/Redo. Saved and reloaded the song, then used `furnace -txtout` to verify the resulting notes, cuts, and effect values.
- Cloned an order through the integrated Orders menu and verified distinct pattern IDs and matching data in the exported song.
- Opened a temporary copy of `demos/genesis/Shovel_Knight_Title.fur` (Jake Kaufman; Genesis cover by Bernie). Inspected the ten-channel arrangement, 192-row patterns, event-only bricks, lead melody, instrument/volume events, and effect lanes. Ran playback and observed matching playheads.
- Checked wheel vertical scrolling, Shift + wheel horizontal scrolling, Ctrl + wheel horizontal zoom, and Alt + wheel vertical zoom in both views. Checked horizontal alignment between the piano roll and event lanes and stationary channel/piano labels.
- Zoomed Orders to minimum and maximum heights. Minimum height fits the enlarged bold italic ID; note previews and secondary labels are hidden at compact heights. Adjacent bricks meet at their grid borders.
- Reopened a collapsed piano roll by double-clicking a brick; it expands and focuses again.
- Replaced synthetic bold/italic ID transforms with a bundled outline font. Verified smooth coverage in software-renderer screenshots and legibility at minimum lane height. The font subset generator is deterministic and retains all hexadecimal glyphs, weight 700, and italic outlines.

For manual retesting, use a copy of a demo song. See `doc/2-interface/horizontal-ui.md` for controls and pattern-data semantics.
