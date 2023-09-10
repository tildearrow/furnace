# AY-3-8910 / AY8930 / SAA1099 envelope guide

the AY-3-8910 programmable sound generator, aside from normal 4-bit volume control, has an hardware volume envelope. this feature that allows for defining the shape of the volume envelope at arbitrary speed according to 8 preset envelope shapes. one may think, what is any upside of hardware envelope? Well, it's somewhat independent of tone/noise generators, and since it goes so high in frequency, it can be used melodically! This guide explains how to make best use of the AY/SAA envelope.

## AY-3-8910 / AY8930

in the instrument editor:
- add a single tick to the "Waveform" macro with only `envelope` turned on. this will disable any output, but don't worry.
- add a single tick to the "Envelope" macro and select `enable`.

if you play a note now, you will hear a very high-pitched squeak. this is because you must set envelope period, which is the frequency at which the hardware envelope runs. you can do it in two ways:
- `23xx` and `24xx` effects (envelope coarse and fine period);
- `29xx` auto-envelope period effect and macros.

auto-envelope works via numerator and denominator. in general, the higher the numerator, the higher the envelope pitch. the higher the denominator, the lower the envelope pitch. why are there both of these? Because the envelope generator might be used to mask the tone output (i.e. affect the square wave as well). to do it, set the "Waveform" macro values to both `tone` and `envelope`. the higher the denominator value, then the lower the envelope pitch relative to the square wave output, and similarly with the numerator. with the square-and-envelope setting, a lot of wild, detuned synth instruments can be made.

back to the hardware envelope itself. depending on the "Envelope" macro value, different envelope shapes can be obtained. the most basic one, 8, is a sawtooth wave. the `direction` value will invert the envelope, producing the reverse sawtooth. the `alternate` value produces an interesting pseudo-triangular wave, similiar to halved sine. that one can also be reversed. `Hold` option disables the envelope.

_Warning:_ the envelope pitch resolution is fairly low; at high pitches it will be detuned. because of this, it's used mostly for bass.

_Warning_: there is only one hardware envelope generator. you can't use two pitches or two waveforms at once.

## SAA1099 

SAA envelope works a bit differently. it doesn't have its own pitch; instead, it relies on the channel 2/5 pitch. it also has many more parameters than the AY envelope. to use it:
- go to waveform macro and add a single tick set to 0 (unless you want to have a square wave mask).
- set up an envelope macro. turn on `enabled`, `loop`, and depending on the desired shape, `cut` and `direction`. `Resolution` will give you higher pitch range than on the AY.
- place two notes in the pattern editor. one in channel 2 will control the envelope pitch. the other in channel 3 can be any note you wish; it's just to enable the envelope output.

## examples

- [Demoscene-type Beat by Duccinator](https://www.youtube.com/watch?v=qcBgmpPrlUA)
- [Philips SAA1099 Test by Duccinator](https://www.youtube.com/watch?v=IBh2gr09zjs)
- [Touhou Kaikidan: Mystic Square title theme by ZUN](https://www.youtube.com/watch?v=tUKei7Pz0Fw): rare instance of AY envelope used for drums, it can be used to mask the noise generator output too