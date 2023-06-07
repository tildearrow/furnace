# Bubble System WSG

a Konami-made 2 channel wavetable sound generator logic used on the Bubble System arcade board, configured with K005289, a 4-bit PROM and DAC.

however, the K005289 is just part of the logic used for pitch and wavetable ROM address.
waveform select and volume control are tied with single AY-3-8910 IO for both channels.
another AY-3-8910 IO is used for reading sound hardware status.

Furnace emulates this configuration as a "chip" with 32x16 wavetables.

# effects

- `10xx`: change wave.
