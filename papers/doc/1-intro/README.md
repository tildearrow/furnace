# introduction

Furnace is a tool which allows you to create music using emulated sound chips from the 8/16-bit era.
For a full list of soundchips that Furnace supports, please see [this list](https://github.com/tildearrow/furnace/tree/master/papers/doc/7-systems).

It has a music tracker interface. think of a piano roll, or a table that scrolls up and plays the notes.

Another core feature of Furnace is its windowing system, similar to that of GEMS or Deflemask, but with a few more features.

## Sound generation

Furnace generates sound from 3 different main types of sound sources.
 - Instruments are the most standard and most used type of sound source in Furnace.
The instrument format is how you can specify parameters and macros for certain channels on certain soundchips, as well as binding samples and wavetables to a format that you can sequence on the note grid.
See [4-instrument](https://github.com/tildearrow/furnace/tree/master/papers/doc/4-instrument) for more details.
 - Wavetables are the way that you create custom waveform shapes for the HuC6280, Seta X1-010, WonderSwan, any PCM chip with wavetable synthesizer support, etc.
Wavetables only work in the sequencer if you bind them to an instrument. See [4-instrument](https://github.com/tildearrow/furnace/tree/master/papers/doc/4-instrument) and [5-wave](https://github.com/tildearrow/furnace/tree/master/papers/doc/5-wave) for more details.
 - Samples are how you play back raw audio streams (samples) on certain channels, on certain soundchips, and in some cases, in certain modes.
To sequence a sample, you do not need to assign it to an instrument, however, to resample samples (change the speed of a sample), you need to bind it to a Sample instrument.
See [6-sample](https://github.com/tildearrow/furnace/tree/master/papers/doc/6-sample) and [4-instrument](https://github.com/tildearrow/furnace/tree/master/papers/doc/4-instrument) for more details.

## Interface/other

Furnace is built to have a user-friendly interface that is intentionally made so that it is quick and easy to get around when working in Furnace.
However, we understand that the interface may not be the easiest to learn, depending on how you learn, so there is documentation on it as well.

See [2-interface](https://github.com/tildearrow/furnace/tree/master/papers/doc/2-interface) and [3-pattern](https://github.com/tildearrow/furnace/tree/master/papers/doc/3-pattern) to view said documentation.
