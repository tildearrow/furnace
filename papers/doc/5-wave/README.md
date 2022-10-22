# wavetable editor

Wavetable synthesizers, in context of Furnace, are sound sources that operate on extremely short n-bit PCM streams. By extremely short, no more than 256 bytes. This amount of space is nowhere near enough to store an actual sampled sound, it allows certain amount of freedom to define a waveform shape. As of Furnace 0.5.8, wavetable editor affects PC Engine, WonderSwan and channel 3 of Game Boy.

Furnace's wavetable editor is rather simple, you can draw the waveform using mouse or by pasting an MML bit stream in the input field. Maximum wave width (length) is 256 bytes, and maximum wave height (depth) is 256. NOTE: Game Boy, PCE, WonderSwan and Bubble System can handle max 32 byte waveforms, X1-010 can handle max 128 byte waveforms as of now, with 16-level height for GB, X1-010 Envelope, WS, Bubble System and N163, and 32-level height for PCE. If a larger wave is defined for these chips, it will be squashed to fit within the constraints of the chips.

## wavetable synthesizer

Furnace contains a mode for wavetable instruments that allows you to modulate or combine 1 or 2 waves to create unique "animated" sounds. Think of it like a VST or a plugin, as it's basically an extension of regular wavetable soundchips that still allow it to run on real hardware.

This is accomplished by selecting a wave or two, a mode, and adjusting the settings as needed until you come up with a sound that you like, without taking up a load of space. This allows you to create unique sound effects or instruments, that, when used well, almost sound like they're Amiga samples.

Unfortunately, on chips like the HuC6280, you cannot use the wavetable synth to animate waveforms and have them sound smooth, as the chip resets the channel's phase when a waveform is changed while the channel is playing. On certain frequencies, this can be avoided, but not on most, unfortunately.
