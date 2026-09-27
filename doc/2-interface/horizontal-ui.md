# horizontal song and piano roll editing

Enable **Settings > Interface > Layout > Horizontal song and piano roll editing**, then click **Apply** or **OK**. This desktop mode replaces the central Pattern view with **Orders - Song**. Disable the option to return to the tracker. The setting is saved in your configuration; songs remain ordinary Furnace files.

## Orders - Song

Time runs left to right. Each column is an order, and each lane is a chip channel. The colored, translucent bricks show the channel's pattern, with its hexadecimal ID in bold italic at the upper right and a miniature note preview. At small lane heights only the ID is shown. The smallest height fits the ID; previews return when you zoom in vertically. Bricks containing only events or effects are labeled **Events / FX** when there is enough space.

- Click a brick to select its channel and order. Double-click it to open the piano roll.
- Drag a brick onto another order **on the same channel** to assign the same pattern there.
- Right-click a brick to enter a pattern ID in hexadecimal or assign an unused empty pattern. Press Enter to apply the ID.
- Use the **Orders** menu or an order heading's context menu to add, remove, duplicate, clone, append, or move orders left and right.
- **Duplicate** shares patterns. **Clone** makes independent copies. Editing a shared pattern changes every order referencing it on that channel.
- The speaker button beside each channel mutes or unmutes it. **Follow playback** scrolls the song view when playback reaches another order.

## Navigation

Hover over the arrangement, piano roll, or event lanes:

| Input | Action |
| --- | --- |
| Wheel | Scroll vertically |
| Shift + wheel | Scroll horizontally |
| Ctrl / Cmd + wheel | Zoom horizontally around the pointer |
| Alt / Option + wheel | Zoom vertically around the pointer |
| Horizontal wheel / trackpad axis | Scroll horizontally |

These use the FL Studio Playlist wheel conventions consistently in both views. FL Studio's own piano roll also uses Alt + wheel for note properties; in this mode it always navigates the view. In the event panel, vertical zoom changes event-lane spacing. The piano keyboard and channel names stay fixed during horizontal scrolling. Notes and their event lanes share the same time scale and scroll position.

## Piano roll

The floating editor displays one channel and one order at a time. Its title area identifies the pattern and the number of orders sharing it. Use the channel selector to switch channels. Opening another brick changes the editor to that brick, even if the window was previously collapsed. **Close** returns to the song view.

- Click empty grid space to insert a note with the current instrument and **Length**, measured in rows. Drag while inserting to set its end.
- Drag a note to move it in time or change its pitch. Drag its right edge to resize it. **Snap** controls the row increment.
- Right-click a note, or select it and press Delete, to erase it. Conflicting note events and instrument/volume-only events are protected from being overwritten.
- Click the vertical keyboard to audition a pitch with the current instrument.
- Use the ruler, event lanes, left/right arrows, or **Row** input to select a row. The Row input is decimal.
- Undo and redo use Furnace's existing history. A completed note gesture is one undo step.

The event panel shows note cuts/releases, instruments, volume, and each effect column horizontally. Effect commands are displayed above their values so both remain legible at normal zoom. Click a row and use the inspector below to edit it, including rows without a note. Instrument, volume, effect-command, and effect-value inputs use hexadecimal: press Enter to apply; erase the text and press Enter to clear. Right-click an event lane to clear that event (both command and value for an effect). **FX lanes** controls the channel's active effect-column count, up to eight. **Effects reference** opens Furnace's effect list.

## How notes map to Furnace

A channel still has one note event per row. Displayed note spans end at the next note event or the pattern boundary. Drawing or resizing a note writes a note cut at its end when no event already occupies that row. Moving a note carries its onset instrument and volume; effects stay at their original rows. Deleting a note removes its following cut, but retains independent release commands and effects.

The preview is a view of the pattern data: envelopes, retriggers, note-delay commands, pattern breaks, jumps, and other effects may change the audible timing. Noise and sample channels retain their original chip-specific pitch semantics. This mode does not introduce polyphonic channels, arbitrary clip lengths, or a different song format. Raw-frequency notes remain visible in the Events lane; their raw bytes can be edited in tracker mode.
