# Yamaha OPL3/YMF262

The Yamaha OPL3/YMF262 was an FM sound chip developed by Yamaha (obviously) and released in 1994.

The OPL3 saw most of its use in PC sound cards (such as the SoundBlaster 16 and the Pro AudioSpectrum).

Unfortunately, developers who wanted to port their OPL/OPL2 music to the OPL3 were very lazy in doing so, so most of them ended up disreguarding the additions to the OPL3 entirely, and would use entirely the same MIDI driver and patches.

## Sound capabilities

 - 18 channels 2-op of FM synthesis
 - 8 unique waveforms which can be used on the carrier or the modulator
 - A "split operators" mode that makes it so that the first and second operators are their own "voices" (each take the base pitch of the note to play and add the frequency multiplier's pitch to it)
 - Hard panning (left, center, and right only)
 - A rhythm mode where the last 3 voices are replaced with 5 drum channels
 - A 4-op mode where 12 FM channels are combined to make 6 4-op FM channels
 - A "tremolo" mode where AM (amplitude modulation) is applied, but unfortunately without any strength or speed controls.
 - A built in vibrato mode which enables vibrato without a music driver doing it for it
 - ADSR support on the carrier and/or modulator

## Effect commands
As of Furnace version dev63, there are no working effect commands for the OPL3.
