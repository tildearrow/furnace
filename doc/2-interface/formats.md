# file formats

this is a list of file formats that Furnace supports.

## song/module

- Furnace song (.fur)
- import:
  - DefleMask module (.dmf)
  - FamiTracker module (.ftm/.0cc/.dnm/.eft)
  - Amiga tracker module (.mod)
  - Scream Tracker 3 module (.s3m)
  - FastTracker 2 module (.xm)
  - Impulse Tracker module (.it)
  - Future Composer module (.fc13/.fc14/.fc/.smod)
  - TFM Music Maker module (.tfe)
  - Standard MIDI file (.mid/.midi)
- export:
  - DefleMask module (.dmf)
  - VGM (.vgm)
  - ZSound Music (.zsm)

### MIDI import

MIDI import is a transcription, not a player - it will not sound like the file
did. notes, timing, velocity, sustain, panning, modulation, pitch bend, tempo and program
changes land on a Generic PCM DAC, but no samples are created and no General MIDI
timbre is emulated: the song is silent until you assign samples to its
instruments.

each combination of track and MIDI channel becomes a "part" that owns its own run
of Furnace channels, growing a new one whenever its polyphony rises. notes are
spread sideways rather than dropped, and one part's notes never land in another
part's columns.

each effect a part uses gets its own column, and keeps that column for as long
as the channel exists - a note delay is always in the same place, a pitch slide
never trades places with a pan effect from row to row. a channel only shows as
many columns as the effects it actually uses.

the import dialog offers:

- **volume column**: any combination of note velocity, CC7 (channel volume) and
  CC11 (expression). the enabled sources multiply together, the way they would on
  a MIDI synth. with all of them off, every note is imported at maximum volume.
- **honor sustain pedal (CC64)**: hold notes until the pedal lifts, instead of
  ending them at their note-off.
- **panning (CC10)**: imported as `80xx` panning effects. needs the chip's Stereo
  option, which is on by default.
- **vibrato (CC1)**: imported as `04xy` vibrato. the mod wheel only carries depth -
  the rate is part of a synth's patch, not the file - so **rate** is set once here
  and applies to the whole import, while **max depth** is what a fully-raised wheel
  reaches. the rate is solved against the song's own tick rate, so it holds in
  either tempo mode; at very high tick rates it may come out faster than asked,
  since `04xy` cannot go slower than one step per tick.
- **pitch bend**: see below.
- **tempo**: Base Tempo sets the song's tick rate from the MIDI's own BPM. this is
  exact and keeps a flat speed, and whatever a note misses the row grid by is
  carried in a note delay, at the cost of an unusual tick rate. Groove
  Approximation instead holds the tick rate at 60Hz and carries the tempo in the
  groove, so notes land on whole rows and timing is coarser.
- **quantize**: the row grid - how many rows one whole note takes.
- **ticks/row**: the song speed, and in Base Tempo mode the sub-row resolution.
- **pattern length**, and which channel is the **drum channel**.

these settings are kept for the rest of the session, so importing several files in
a row does not mean setting them up again. they return to their defaults when
Furnace restarts.

#### pitch bend

pitch bend is imported as `F1xx`/`F2xx` single-tick pitch slides. `E5xx` is not
used: it only reaches one semitone either side of centre, while a General MIDI
wheel at its default range of two semitones already covers twice that.

these slides are *relative* - each row carries only the change since the last one.
the values are therefore not meaningful to read on their own, and copying part of a
bend carries pitch along with it. playback and seeking are unaffected.

MIDI has no "ramp" message, so a bend curve is only ever a series of discrete
events, and each one is written only on the row it happens - nothing is drawn
between two events, since there is nothing in the file to draw.

`F1xx`/`F2xx` can only move the pitch so far in a single row. an instant bend
wider than that - a fast wheel dive, or a wide bend range - would otherwise arrive
late, ramping in over the following rows instead of snapping. to avoid that, a
bend too wide for one row moves the note column itself to the nearest semitone
first, and lets the slide cover only the remainder (at most half a semitone).
moving the note this way does not retrigger the note: it is followed by an `EA01`
that keeps the sample running underneath the pitch change, and the next real note
in that column always undoes it with an `EA00`. this is not a legato feature and
exposes no legato behaviour - `EA01`/`EA00` only ever appear as a byproduct of a
wide bend, are written and cleared entirely by the importer, and every ordinary
note-on still retriggers normally. pitch bend is skipped entirely on the drum
channel, since moving its note column would change which drum sounds.

**bend range** decides how far the wheel travels:

- **from file** (the default) follows the range the file declares through RPN 0
  (`CC101`, `CC100`, `CC6`, `CC38`), or General MIDI's two semitones if it never
  says. a file that changes its range partway through is followed.
- **1 to 24 semitones** overrides both, on every channel.

many files bend with a wider wheel than they declare, and nothing in the file gives
that away. if the bends come out too shallow, set this to whatever made the file -
12 is a common choice. a warning is shown after importing a file that bends without
declaring a range.

## instrument

- load/save:
  - Furnace instrument (.fui)
  - DefleMask preset/patch (.dmp)
- load only:
  - TFM Music Maker instrument (.tfi)
  - VGM Music Maker instrument (.vgi)
  - Scream Tracker 3 instrument (.s3i)
  - SoundBlaster instrument (.sbi)
  - Wohlstand OPL instrument (.opli)
  - Wohlstand OPN instrument (.opni)
  - Gens KMod patch dump (.y12)
  - BNK file (AdLib) (.bnk)
  - FF preset bank (.ff)
  - 2612edit GYB preset bank (.gyb)
  - VOPM preset bank (.opm)
  - Wohlstand WOPL bank (.wopl)
  - Wohlstand WOPN bank (.wopn)

## wavetable

- load/save:
  - Furnace wavetable (.fuw)
  - DefleMask wavetable (.dmw)
  - raw wavetable data

## sample

- load/save
  - Wave file (.wav)
  - raw sample data
- load only:
  - Apple/SGI sample (.aiff)
  - Sun/NeXT sample (.au)
  - Audio Visual Research sample (.avr)
  - Apple Core Audio File sample (.caf)
  - FLAC (Free Lossless Audio Codec) (.flac)
  - HMM Tool Kit sample (.htk)
  - Amiga IFF/SVX8/SV16 sample (.iff)
  <!--
  - GNU Octave 2.0 / Matlab 4.2 sample (.mat)
  - GNU Octave 2.1 / Matlab 5.0 sample (.mat)
  -->
  - GNU Octave/Matlab sample (.mat)
  - MPEG-1/2 Audio (.m1a, .mp1, .mp2, .mp3)
  - OGG (OGG Container Format) (.ogg, .oga, .opus)
  - Akai MPC 2k sample (.mpc)
  - Ensoniq PARIS sample (.paf)
  - Portable Voice Format sample (.pvf)
  - RIFF 64 sample (.rf64)
  - Sound Designer II sample (.sd2)
  - Midi Sample Dump Standard sample (.sds)
  - Berkeley/IRCAM/CARL sample (.sf)
  - Creative Labs sample (.voc)
  - SoundFoundry WAVE 64 sample (.w64)
  - NIST Sphere sample (.wav)
  - Psion Series 3 sample (.wve)
  - FastTracker 2 sample (.xi)
  - NES DPCM data (.dmc)
  - SNES Bit Rate Reduction (.brr)
  - PMD YM2608 ADPCM-B sample bank (.ppc)
  - PDR 4-bit AY-3-8910 sample bank (.pps)
  - FMP YM2608 ADPCM-B sample bank (.pvi)
  - MDX OKI ADPCM sample bank (.pdx)
  - FMP 8-bit PCM sample bank (.pzi)
  - PMD 8-bit PCM sample bank (.p86)
  - PMD OKI ADPCM sample bank (.p)
