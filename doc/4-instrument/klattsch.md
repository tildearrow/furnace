# klattsch instrument editor

the Klattsch instrument defines a persistent voice profile. the selected phoneme bank provides the base formants and voice values; these settings shape that base whenever the instrument is used:

- **Transition time**: default spectral transition time, in ticks.
- **Voicing**, **Aspiration**, **Spectral tilt** and **Glottal effort**: use the selected bank record's value or replace it with an instrument value.
- **Gain**: uses the selected bank record's gain or replaces it with an instrument value.
- **Formant bandwidth scale**: scales all three phoneme bandwidths.
- **Formant shift**: scales all three phoneme formants and their bandwidths, changing the apparent vocal-tract size.
- **Vibrato** and **Tremolo**: default modulation rate and depth.

new instruments use the same factory voice as Klattsch and the web playground: a roughly 35 ms transition (2 ticks at Furnace's default tempo), bank voicing, aspiration 0, spectral tilt 0, glottal effort 0.5, gain 3.5, neutral formant scales, and vibrato/tremolo off with 5 Hz rates.

pattern effects override the instrument profile. the effect reset values (`FF` for voice parameters and `00` for gain, bandwidth and formant shift) return control to the instrument. **bank value** means the selected phoneme record supplies that parameter. **neutral** means a formant scale of 1, leaving the bank frequencies and bandwidths unchanged.

the macros are:

- **Volume**: volume sequence.
- **Arpeggio**: pitch sequence.
- **Pitch**: fine pitch.
- **Pan L**: left output level sequence.
- **Pan R**: right output level sequence.

phoneme selection and direct formant targets remain pattern events because they describe articulation rather than the persistent voice. see the [klattsch](../7-systems/klattsch.md) system page for the full effect list and the phoneme table.
