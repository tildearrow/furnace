# concepts and terms

- a **song** (also called **module**) is a file for a tracker that contains at least one **subsong**.
- each Furnace song involves at least one **chip**, an emulation of a specific audio processor.

## tracking

the **[pattern view](../3-pattern/README.md)** is similar to spreadsheet that displays the following:
- each labeled column represents a **channel** of sound provided by the chips in use.
- each **note** starts a sound playing. within a channel, only one note can play at a time.
- each note is assigned an **[instrument](../4-instrument/README.md)** which describes what it will sound like.
- an **effect** is a command that changes some aspect of playback. it can alter note pitch, volume, timing, and more.
- an instrument **macro** is an automated sequence of effects that applies to every note of that instrument.
- during playback, the **playhead** moves down, scrolling through the pattern view, triggering the notes that it encounters.

## structure

the **order list** is a smaller spreadsheet showing the overall song structure.
- a song is made up of a list of orders.
- an **order** is a set of numbered patterns used for each channel.
- each channel has its own unique list of patterns.
- each **pattern** contains note and effect data for that channel only.
- patterns may be used multiple times in the order list. changing a pattern's data in one order will affect the same pattern used in other orders.
- each pattern is made of the same number of rows as seen in the tracker view.
- during playback, the **playhead** moves down as described previously. when it reaches the end of the pattern view, it will go to the next order.
- if the last order is reached and the playhead reaches the end of the pattern view, it will go back to the beginning of the song.

## time

- a **tick** is the smallest measure of time to which all note, effect, and macro times are quantized.
- during playback, each **row** lasts a number of ticks determined by the song's **speed** value(s).

## sound

sound chips have different capabilities. even within the same chip, each channel may have its own ways of making sound.
- some channels use one or more waveform **generators** (sine, square, noise...) to build up a sound.
  - of special note are **FM (frequency modulation)** channels, which use a number of generators called **operators** that can interact to make very complex sounds.
- some channels use **[samples](../6-sample/README.md)** which are (usually) recordings of sounds, often with defined loop points to allow a note to sustain.
- some channels use **[wavetables](../5-wave/README.md)**, which are very short samples of fixed length that automatically loop.
