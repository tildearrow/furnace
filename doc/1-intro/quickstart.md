<!--
NOTES TO AUTHOR:
* keep it casual, don't ELI5 it
* put graphics where {{ subject }} placeholders are
* provide a quickstart.fur with example assets of each type
-->

# quick start guide

first things first: thank you for taking the time to understand Furnace, the world's most comprehensive chiptune tracker. it's amazingly versatile, but it can also be intimidating, even for those already familiar with trackers. this quick start guide will get you on the road to making the chiptunes of your dreams! it will probably take about an hour <!-- estimate, test and revise it when done --> from start to finish.

this guide makes a few assumptions:
* you've already installed Furnace.
* you haven't changed any configuration or layout yet.
* you're working with a PC keyboard. Mac users should already know the equivalents to the `Ctrl` and `Alt` keys.
* you're comfortable with keyboard shortcuts. if not, a lot of this can also be done using buttons or menus, but please try the keyboard first. it's worth it to smooth out the tracking workflow.

if an unfamiliar term comes up or you need more clarification on a term, refer to the [basic concepts](concepts.md) and [glossary](glossary.md) docs.

if you're not already familiar with the ImGui style of interface, you might want to take a quick glance at the [UI components](../2-interface/components.md) documentation. if at any point something goes wrong with the interface – something gets moved to where it's inaccessible, something closes a window or tab unexpectedly, or the like – it can always be reverted to its original state by selecting "reset layout" from the "settings" menu.

with all that said, start up the program and let's get going!

## I've opened Furnace – now what?

there's a lot going on, but the most prominent part of Furnace's interface is the **pattern view** – the spreadsheet-like table that takes up most of the screen.

{{ pattern view }}

click to place the cursor somewhere in this view. it will appear as a medium-blue highlight.try moving around with the up and down arrow keys. also try the `PgUp` and `PgDn` keys to move around faster. the vertical axis represents time, and the view scrolls around a highlighted row that stays put in the center; this is called the **playhead**, and it will follow along when the song is playing.

now try the left and right arrow keys to move between columns. while we're at it, use the `Home` and `End` keys to quickly get to the first and last columns. pressing them twice will also shuttle you to the top or bottom row. when you're done, hit `Home` twice to return to the top-left.

if the playhead is dark red, use the space bar to turn it grey. this puts the keyboard in play mode, which will let us try out notes before committing them to the track.

now, let's play a little! try the first seven letters on the bottom row of the keyboard (for QWERTY, these are `ZXCVBNM`). they should sound out the seven notes of a C major scale, like the white keys of a piano. similarly, the row above that has the black keys. once you're tried those, move two rows up to find the same arrangement but one octave higher, and it extends a little further into the next octave also. to change octaves, use the `/` and `*` keys on the numeric pad. (if you don't have a numeric pad, the keys can be remapped; the [keyboard](../2-interface/keyboard.md) doc explains how.)

![keyboard note entry](../3-pattern/keyboard.png)

press the space bar to change from play to edit mode. now try playing some notes; they should appear in the pattern view, one after another.

{{ notes in one channel }}

each **channel** is a group of columns separated from the others by a faint vertical line, and each channel can only ever play one note at a time. to hear this in action, move the cursor back to the top and press the `Return` key to start playback. you should hear the notes you entered played back quickly, one after another, each cutting off the previous note. if you let it play long enough, it'll wrap around to the start to go through them again; press `Return` again to stop playback.

now let's clear out those notes. you could delete them individually with the `Del` key, but let's try something else first. click and drag to select them all. you'll know they're selected when they have a medium grey background. try moving them around by clicking in the selected area and dragging it to another channel. (this may not work with some input devices.) then hit `Del` to delete them all at once.

{{ selection area? }}

you'll usually want more than one note playing at a time. move back to the start of the pattern in the leftmost column of the leftmost channel – this should clear the selection area. put some different notes next to each other in the same row. only enter notes in the first column of each channel; we'll get to those other columns later. (don't do more than six notes at once yet. we want to stay in the channels labelled "FM" for now.) once those are in place, go back to the top row and use the `Return` key to start playback. they should all sound at the same time as one single chord.

{{ notes across three channels }}

that chord will ring out for quite some time, but let's try stopping it early. a few rows after that chord, use the `Tab` or `1` key to enter a **note off** (sometimes called "note cut") in each channel that has a note. it'll appear as `OFF` in the note column. now try the shortcut `F5` to play from the start without having to move there. you should hear the chord as before, then it will abruptly stop where the note offs are, as though letting off the keys of a piano.

{{ notes across channels with note offs beneath them }}

of course, errors can happen. let's pretend those note offs were a bad idea and undo them with `Ctrl-Z`. Furnace keeps track of multiple levels of undo. undo will work for the pattern view, any text entry box, and a few other places; try it out here and there along the way to get a sense for what it can undo for you! for now, let's change our minds again and put those note offs back with redo, which is `Ctrl-Y`.

before the next part of this guide, save the current **module** – the tracker file that contains everything needed for a song. Use `Ctrl-S` and pick a good spot on your computer for the file. Furnace modules always have a filename that ends in a `.fur` extension.

<!--1
## How do I get more instrument sounds?
* open up another instance, open quickstart.fur
* save an instrument from the pack-in demo
	* where are these on different platforms?
* load that instrument in first instance
* switch to it by typing it in the pattern or selecting in list
* change the instrument of existing notes
* "FM instruments are versatile and complex; explaining them is beyond the scope of this doc. Here's a link for more info." or something to that effect
	* https://youtu.be/cDJ1z-4YsYM "FM Synthesis Crash Course"?
-->

<!--
## How can I change a note's volume?
* volume column
  * it's in hex! (link to hex.md)
		* so is the instrument! and so are the other columns, but we'll get to those
	* you can always get numbers in decimal – menu status bar!
	  * also, the "modified" indicator
* volume limits
  * logarithmic vs linear
* effects!
	* also in hex
	* check out the effect list
	* the basic volume slides
	* try out vibrato?
	* link to effects.md
-->

<!--
## How do I make the song longer?
* "pattern" used two different ways – explain patterns vs orders
* make another order
* change the patterns
* duplicate vs deep clone?
* watch out for pattern repeat during playback!
-->

<!--
## How do I change tempo?
* explain ticks vs BPM
* change speed
* mention speeds and groove but link to the docs instead of explaining
* change row highlights for other time signatures?
-->

<!--
## What about those other channels?
* explain chips
* make a new PSG instrument
* add notes in a square channel
* noise channel
  * learn about chip quirks being a thing – preset freq,  noise length, &c. check the docs!
-->

<!--
## What about samples?
* save one from the quickstart.fur
* load it up
* make an instrument
  * "Generic Sample" vs chip-specific instrument
* go to channel FM 6 and play!
* adjust frequency in sample editor
* per-system frequency limits
-->

<!--
## What about wavetables?
* make a new Game Boy tune
* set up a wavetable
* make an instrument and set a macro
* play in wave channel
-->

<!--
## What's a macro?
* Furnace's most powerful feature!
* set up a plain volume macro – downward ramp
* play some notes with it
* set up a sustain point in the middle
* use note off
* use note release
* set up arpeggio loop
	* using text entry, make sure to get a note off the top of the graph then scroll
* see docs for more
-->

<!--
## Go play!
-->

<!--
-->

<!--
-->

