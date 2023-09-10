# using OPLL patch macro

YM2413's biggest flaw (or, rather, cost-saving feature) was that it could use only one user-defined instrument at once. it wasn't monotimbrial; you could use 15 built-in presets and 5 built-in drum tones freely, but for these going off the beaten path, it surely was limiting. however, there is one technique, as amazing as simple: **mid-note preset switching**.

the idea is to use the first patch to put the envelope in an unintended state for the second patch so that it sounds different, with a higher or lower modulation level. the sustain level defines at which "envelope level" the envelope will switch to the sustain state (or release depending on envelope type). if the first patch is used to put the envelope into sustain at a higher or lower envelope state than intended for the second patch, it'll still be in sustain/release but at a higher or lower level than it should be at that point.

therefore, much more variety can be forced out, without using custom instruments. as of July 2023, Furnace is the only tool supporting this feature. it is accessed in 'Macros' tab in OPLL instrument editor.

for example, try putting the first macro value as 14 (acoustic bass preset), followed by 4 (flute preset). this way you will get distortion guitar-like sound this is nothing like other 2413 preset! there are many combination to test out, which is highly recommended (I can only say, 12->1 or 12->4 produces sound similiar to the well-known 4-op FM mallet brass)

## drums using this technique

using OPLL's drum mode, described is systems/opll.md, you gain access to 5 hardcoded drum tones at the expense of 3 melodic FM channels. patch switching eliminates that, as using it, it's also possible to construct percussive sounds, some even fuller than their drum mode counterparts!
un short, noise portion of drums (as in hi-hats), can be created of the very high pitched pseudo-distortion guitar, described as above. for kicks, snares, toms and claps, more effort is needed, however using volume and arpeggio macros will help.

## examples

- [Lman-Clubster cover by Mahbod](https://www.youtube.com/watch?v=jfHs7tSyjXI)
- [OPLL Nation by Mahbod](https://www.youtube.com/watch?v=ou6pEfxByeE)
