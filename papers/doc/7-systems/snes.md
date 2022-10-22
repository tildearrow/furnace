# Super Nintendo Entertainment System (SNES)/Super Famicom

the successor to NES to compete with Genesis, packing superior graphics and sample-based audio.

its audio system, developed by Sony, features a DSP chip, SPC700 CPU and 64KB of dedicated SRAM used by both.
this whole system itself is pretty much a separate computer that the main CPU needs to upload its program and samples to.

Furnace communicates with the DSP directly and provides a full 64KB of memory. this memory might be reduced excessively on ROM export to make up for playback engine and pattern data. you can go to window > statistics to see how much memory your samples are using.

some notable features of the DSP are:
- pitch modulation, meaning that you can use 2 channels to make a basic FM synth without eating up too much memory.
- a built in noise generator, useful for hi-hats, cymbals, rides, effects, among other things.
- per-channel echo, which unfortunately eats up a lot of memory but can be used to save channels in songs.
- an 8-tap FIR filter for the echo, which is basically a procedural low-pass filter that you can edit however you want.
- sample loop, but the loop points have to be multiples of 16.
- left/right channel invert for surround sound.
- ADSR and gain envelope modes.
- 7-bit volume per channel.
- sample interpolation, which is basically a low-pass filter that gets affected by the pitch of the channel.

Furnace also allows the SNES to use wavetables (and the wavetable synthesizer) in order to create more 'animated' sounds, using less memory than regular samples. this however is not a hardware feature, and might be difficult to implement on real hardware.

# effects

- `10xx`: set waveform.
- `11xx`: toggle noise mode.
- `12xx`: toggle echo on this channel.
- `13xx`: toggle pitch modulation.
- `14xy`: toggle inverting the left or right channels (x: left, y: right).
- `15xx`: set envelope mode.
  - 0: ADSR.
  - 1: gain (direct).
  - 2: linear decrement.
  - 3: exponential decrement.
  - 4: linear increment.
  - 5: bent line (inverse log) increment.
- `16xx`: set gain (00 to 7F if direct, 00 to 1F otherwise).
- `18xx`: enable echo buffer.
- `19xx`: set echo delay
  - goes from 0 to F.
- `1Axx`: set left echo channel volume.
  - this is a signed number.
  - 00 to 7F for 0 to 127.
  - 80 to FF for -128 to -1.
    - setting this to -128 is not recommended as it may cause echo output to overflow and therefore click.
- `1Bxx`: set right echo channel volume.
  - this is a signed number.
  - 00 to 7F for 0 to 127.
  - 80 to FF for -128 to -1.
    - setting this to -128 is not recommended as it may cause echo output to overflow and therefore click.
- `1Cxx`: set echo feedback.
  - this is a signed number.
  - 00 to 7F for 0 to 127.
  - 80 to FF for -128 to -1.
    - setting this to -128 is not recommended as it may cause echo output to overflow and therefore click.
- `1Dxx`: set noise generator frequency (00 to 1F).
- `20xx`: set attack (0 to F).
  - only in ADSR envelope mode.
- `21xx`: set decay (0 to 7).
  - only in ADSR envelope mode.
- `22xx`: set sustain (0 to 7).
  - only in ADSR envelope mode.
- `23xx`: set release (00 to 1F).
  - only in ADSR envelope mode.
- `30xx`: set echo filter coefficient 0.
- `31xx`: set echo filter coefficient 1.
- `32xx`: set echo filter coefficient 2.
- `33xx`: set echo filter coefficient 3.
- `34xx`: set echo filter coefficient 4.
- `35xx`: set echo filter coefficient 5.
- `36xx`: set echo filter coefficient 6.
- `37xx`: set echo filter coefficient 7.
  - all of these are signed numbers.
  - 00 to 7F for 0 to 127.
  - 80 to FF for -128 to -1.
  - make sure the sum of these is between -128 or 127.
    - failure to comply may result in overflow and therefore clicking.
