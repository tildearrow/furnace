# AY-3-8910/8930/SAA1099 envelope guide

AY-3-8910 programmable sound generator, aside of normal 4-bit volume control, has an hardware volume envelope - a feature that allows you for defining shape of volume envelope at arbibrary speed, according to 8 preset envelope shapes. One may think, what is any upside of hardware envelope? Well, it's somewhat independent of tone/noise generators, and it goes so high in frequency, it can be used melodically! This guide explains how to abuse AY/SAA envelope.

## AY-3-8910/AY8930

going into instrument editor, first set the waveform macro value to `envelope`. This will disable any output, but don't worry. Then, go to `Envelope` macro and select `enable`. You will hear a very high-pitched squeak. This is because you must set envelope period - the frequency at which hardware envelope runs. You can do it in two ways:
- either via 23xx and 24xx effects (envelope coarse and fine period) or...
- 29xx auto-envelope period effect and macros

Auto-envelope works via numerator and denominator. In general, the higher the numerator, the higher the envelope pitch. The higher the denominator, the lower the envelope pitch. Why there are both of these? Because, envelope generator might be used to mask the tone output (i.e. affect the square wave as well). To do it, set the waveform macro values to both square and envelope. Then, the higher the denominator value, then the lower the envelope pitch relative to the square wave output, analogously the numerator. With square + envelope setting, a lot of wild, detuned, synth instruments can do made.

Back to the hardware envelope itself. Depending of the `Envelope` macro value, different envelope shapes can be obtained. The most basic one, at 8 is a sawtooth wave. The `direction` value will invert the envelope, producing the reverse sawtooth. The `alternate` value produces an interesting pseudo-triangular wave, similiar to halved sine. That one can also be reversed. `Hold` option disables the envelope.

WARNING: the envelope pitch resolution is fairly low, at high pitched it will be detuned. Hence, it was used mostly for bass.
WARNING: there is only one hardware envelope generator. So, you cant use two pitches/two waveforms at once.

## SAA1099 

SAA envelope works a bit differently, It doesn't have its own pitch, it reles on a channel 2/5 pitch. It also has much more parameters than AY envelope. To use it: go to waveform macro, and set it to 0 (unless you want to have sqaure wave mask). Then, set up an envelope macro: tuen on enabled, loop and, depending on a desired shape, cut and direction. Resolution will give you higher pitch range than on AY.
Then lay two notes in pattern editor: the one in channel 2 will control the envelope pitch, the one in channel 3 can be any note you wish, its just to enable the envelope output.

## examples

- [Demoscene-type Beat by Duccinator](https://www.youtube.com/watch?v=qcBgmpPrlUA)
- [Philips SAA1099 Test by Duccinator](https://www.youtube.com/watch?v=IBh2gr09zjs)
- [Touhou Kaikidan: Mystic Square title theme by ZUN](https://www.youtube.com/watch?v=tUKei7Pz0Fw) /rare instance of AY envelope used for drums, it can be used to mask the noise generator output too