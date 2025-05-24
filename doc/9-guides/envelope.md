# AY-3-8910 / AY8930 / SAA1099 envelope guide

The AY-3-8910 programmable sound generator, aside from normal 4-bit volume control, has an hardware volume envelope. This feature that allows for defining the shape of the volume envelope at arbitrary speed according to 8 preset envelope shapes. One may think, what is any upside of hardware envelope? Well, it's somewhat independent of tone/noise generators, and since it goes so high in frequency, it can be used melodically! This guide explains how to make best use of the AY/SAA envelope.

## AY-3-8910 / AY8930

In the instrument editor:
- Add a single tick to the "Waveform" macro with only `envelope` turned on. This will disable any output, but don't worry.
- Add a single tick to the "Envelope" macro and select `enable`.

If you play a note now, you will hear a very high-pitched squeak. This is because you must set envelope period, which is the frequency at which the hardware envelope runs. You can do it in two ways:
- `23xx` and `24xx` effects (envelope coarse and fine period);
- `29xx` auto-envelope period effect and macros.

Auto-envelope works via numerator and denominator. In general, the higher the numerator, the higher the envelope pitch. The higher the denominator, the lower the envelope pitch. Why are there both of these? Because the envelope generator might be used to mask the tone output (i.e. affect the square wave as well). To do it, set the "Waveform" macro values to both `tone` and `envelope`. The higher the denominator value, then the lower the envelope pitch relative to the square wave output, and similarly with the numerator. With the square-and-envelope setting, a lot of wild, detuned synth instruments can be made.

Back to the hardware envelope itself. Depending on the "Envelope" macro value, different envelope shapes can be obtained. The most basic one, 8, is a sawtooth wave. The `direction` value will invert the envelope, producing the reverse sawtooth. The `alternate` value produces an interesting pseudo-triangular wave, similar to halved sine. That one can also be reversed. `Hold` option disables the envelope.

_Warning:_ The envelope pitch resolution is fairly low; at high pitches it will be detuned. Because of this, it's used mostly for bass.

_Warning_: There is only one hardware envelope generator. You can't use two pitches or two waveforms at once.

## SAA1099 

SAA envelope works a bit differently. It doesn't have its own pitch; instead, it relies on the channel 2/5 pitch. It also has many more parameters than the AY envelope. To use it:
- Go to waveform macro and add a single tick set to 0 (unless you want to have a square wave mask).
- Set up an envelope macro. Turn on `enabled`, `loop`, and depending on the desired shape, `cut` and `direction`. `Resolution` will give you higher pitch range than on the AY.
- Place two notes in the pattern editor. One in channel 2 will control the envelope pitch. The other in channel 3 can be any note you wish; it's just to enable the envelope output.

## examples

- [Demoscene-type Beat by Duccinator](https://www.youtube.com/watch?v=qcBgmpPrlUA)
- [Philips SAA1099 Test by Duccinator](https://www.youtube.com/watch?v=IBh2gr09zjs)
- [Touhou Kaikidan: Mystic Square title theme by ZUN](https://www.youtube.com/watch?v=tUKei7Pz0Fw): Rare instance of AY envelope used for drums, it can be used to mask the noise generator output too
