# Bubble System/K005289

a Konami's 2 channel wavetable sound generator logic used at their arcade hardware Bubble System.

It's configured with K005289, 4 bit PROM and DAC.

Also known as K005289, but that's just part of the logic used for pitch and wavetable ROM address. Waveform select and Volume control are tied with AY-3-8910 port.

furnace emulates this configurations as single system, waveform format is 15 level and 32 width.

# effects

- `10xx`: change wave.
