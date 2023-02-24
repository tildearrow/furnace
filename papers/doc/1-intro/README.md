# Introduction

Furnace (Tracker) is a tool which allows you to create music using emulated sound chips from the 8/16-bit era.
For a full list of soundchips that Furnace supports, please see [Section 7](https://github.com/tildearrow/furnace/blob/master/papers/doc/7-systems/README.md).

It has a music tracker interface. think of a piano roll, or a table that scrolls up and plays the notes.

Another core feature of Furnace is its windowing system, similar to that of GEMS or Deflemask, but with a few more features.

## Sound generation

Furnace generates sound from 3 different main types of sound sources.
 - Instruments are the most standard and most used type of sound source in Furnace.
The instrument format is how you can specify parameters and macros for certain channels on certain soundchips, as well as binding samples and wavetables to a format that you can sequence on the note grid.
See [Section 4](https://github.com/tildearrow/furnace/blob/master/papers/doc/4-instrument/README.md) for more details.
 - Wavetables are the way that you create custom waveform shapes for the HuC6280, Seta X1-010, WonderSwan, any PCM chip with wavetable synthesizer support, etc.
Wavetables only work in the sequencer if you bind them to an instrument. See Section 4 and [Section 5](https://github.com/tildearrow/furnace/blob/master/papers/doc/5-wave/README.md) for more details.
 - Samples are how you play back raw audio streams (samples) on certain channels, on certain soundchips, and in some cases, in certain modes.
To sequence a sample, you do not need to assign it to an instrument, however, to resample samples (change the speed of a sample), you need to bind it to a Sample instrument.
See [Section 6](https://github.com/tildearrow/furnace/blob/master/papers/doc/6-sample/README.md) and Section 4 for more details.

## Interface/other

Furnace is built to have a user-friendly interface that is intentionally made so that it is quick and easy to get around when working in Furnace.
However, we understand that the interface may not be the easiest to learn, depending on how you learn, so there is documentation on it as well.

See [Section 2](https://github.com/tildearrow/furnace/blob/master/papers/doc/2-interface/README.md) and [Section 3](https://github.com/tildearrow/furnace/blob/master/papers/doc/3-pattern/README.md) to view said documentation.

Previous: None ┊ Next: [Section 2: Interface](https://github.com/tildearrow/furnace/blob/master/papers/doc/2-interface/README.md)
