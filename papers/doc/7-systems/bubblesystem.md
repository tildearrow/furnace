# Bubble System WSG

a Konami's 2 channel wavetable sound generator logic used at their arcade hardware Bubble System.

It's configured with K005289, 4 bit PROM and DAC.

Also known as K005289, but that's just part of the logic used for pitch and wavetable ROM address.
Waveform select and Volume control are tied with single AY-3-8910 IO for both channels.
Another AY-3-8910 IO is used for reading sound hardware status.

furnace emulates this configurations as single system, waveform format is 15 level and 32 width.

# effects

- `10xx`: change wave.
