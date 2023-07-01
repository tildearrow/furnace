# concepts and terms

- A **module** is a file for a tracker that contains at least one **song**.
- Each Furnace module involves at least one **[chip](../7-systems/README.md)**, an emulation of a specific audio processor.

## tracking

The **[pattern view](../3-pattern/README.md)** is like a spreadsheet that displays the following:
- Each labeled column represents a **channel** of sound provided by the chips in use.
- Each **note** starts a sound playing. Within a channel, only one note can play at a time.
- Each note is assigned an **[instrument](../4-instrument/README.md)** which describes what it will sound like.
- An **effect** is a command that changes some aspect of playback. It can alter note pitch, volume, timing, and more.
- An instrument **macro** is an automated sequence of effects that applies to every note of that instrument.

## structure

The **order list** is a smaller spreadsheet showing the overall song structure.
- A song is made up of a list of **orders**.
- An **order** is a set of numbered **patterns** used for each channel.
- Each channel has its own unique list of patterns.
- Each pattern contains note and effect data for that channel only.
- Patterns may be used multiple times in the order list. Changing a pattern's data in one order will affect the same pattern used in other orders.

## time

- Each pattern is made of the same number of **rows** as seen in the tracker view.
- During playback, Each row lasts a number of **ticks** determined by its **speed** value.
- A tick is the smallest measure of time to which all note, effect, and macro times are quantized.

## sound

Different chips have different capabilities. Even within the same chip, each channel may have its own ways of making sound.
- Some channels use one or more waveform **generators** (sine, square, noise...) to build up a sound.
- Of special note are **[FM (frequency modulation)](../4-instrument/fm.md)** channels, which use a number of generators called **operators** that can interact to make very complex sounds.
- Some channels use **[samples](../6-sample/README.md)** - recordings of sounds, often with defined loop points to allow a note to sustain.
- Some channels use **[wavetables](../5-wave/README.md)**, which are like very short samples of fixed length that automatically loop.