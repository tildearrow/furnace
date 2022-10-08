# Super NES

The successor to NES to compete with Genesis. Now packing superior graphics and sample-based audio. Also known as Super Famicom in Japan.

Its audio subsystem, developed by Sony, features the DSP chip, SPC700 microcontroller and 64KB of dedicated SRAM used by both. This whole system itself is pretty much a separate computer that the main CPU needs to upload its program and samples to.

Furnace communicates with the DSP directly and provides a full 64KB of memory. This memory might be reduced excessively on ROM export to make up for playback engine and pattern data. As of version 0.6pre2, you can go to `window -> statistics` to see how much memory your samples are using.

Some notable features of the DSP are:
- It has pitch modulation, meaning that you can use 2 channels to make a basic FM synth without eating up too much memory
- It has a built in noise generator, useful for hihats, cymbals, rides, sfx, among other things.
- It famously features per-channel echo, which unfortunately eats up a lot of memory but can be used to save channels in songs.
- It can loop samples, but the loop points have to be multiples of 16.
- It can invert the left and/or right channels, for surround sound.
- It features ADSR, similar to the Commodore 64, but its functionality is closer to the OPL(L|1|2|3)'s implementation of ADSR.
- It features an 8-tap FIR filter, which is basically a procedural low-pass filter that you can edit however you want.
- 7-bit volume, per-channel.
- Per-channel interpolation, which is basically a low-pass filter that gets affected by the pitch of the channel.

Furnace also allows the SNES to use wavetables (and the wavetable synthesizer) in order to create more 'animated' sounds, using less memory than regular samples. This, however, is not a hardware feature, and might be difficult to implement on real hardware.

# effects

Note: this chip has a signed left/right level. Which can be used for inverted (surround) stereo. A signed 8-bit value means 80 - FF = -128 - -1. Other values work normally. A value of -128 is not recommended as it could cause overflows.

- `10xx`: Set waveform.
- `11xx`: Toggle noise generator mode.
- `12xx`: Toggle echo on this channel.
- `13xx`: Toggle pitch modulation.
- `14xy`: Toggle inverting the left or right channels. (x: left, y: right)
- `15xx`: Set envelope mode. (0: ADSR, 1: gain/direct, 2: decrement, 3: exponential, 4: increment, 5: bent)
- `16xx`: Set gain. (00 to 7F if direct, 00 to 1F otherwise)
- `18xx`: Enable echo buffer.
- `19xx`: Set echo delay. (0 to F)
- `1Axx`: Set left echo channel volume.
- `1Bxx`: Set right echo channel volume.
- `1Cxx`: Set echo feedback.
- `1Dxx`: Set noise generator frequency. (00 to 1F)
- `20xx`: Set attack (0 to F)
- `21xx`: Set decay (0 to 7)
- `22xx`: Set sustain (0 to 7)
- `23xx`: Set release (00 to 1F)
- `30xx`: Set echo filter coefficient 0
- `31xx`: Set echo filter coefficient 1
- `32xx`: Set echo filter coefficient 2
- `33xx`: Set echo filter coefficient 3
- `34xx`: Set echo filter coefficient 4
- `35xx`: Set echo filter coefficient 5
- `36xx`: Set echo filter coefficient 6
- `37xx`: Set echo filter coefficient 7
