# export

Furnace allows you to export your song in several formats. this section deals with describing the available export options within each tab of the export window.

## Audio

this option allows you to export your song in .wav format. I know I know, no .mp3 or .ogg export yet, but you can use a converter.

- **Export type**:
  - **one file**: exports your song to one .wav file.
  - **multiple files (one per chip)**: exports the output of each chip to .wav files.
  - **multiple files (one per channel)**: exports the output of each channel to .wav files.
    - useful for usage with a channel visualizer such as corrscope.
- **Bit depth**:
  - **16-bit integer**: the most common type of .wav file, perfect for music playback.
  - **32-bit float**: used for advanced audio manipulation. don't select this unless you know what you need it for.
- **Sample rate**: the default is 44100, the most common .wav sample rate.
- **Channels in file**: default is 2 (stereo). Set to 1 for mono.
- **Loops**: sets the number of times the song will loop.
  - does not have effect if the song ends with `FFxx` effect.
- **Fade out (seconds)**: sets the fade out time when the song is over.
  - does not have effect if the song ends with `FFxx` effect.

## VGM

this option allows exporting to a VGM (Video Game Music) file. these can be played back with VGMPlay (for example).

the following settings exist:

- **format version**: sets the VGM format version to use.
  - versions under 1.70 do not support per-chip volumes, and therefore will ignore the Mixer completely.
  - other versions may not support all chips.
  - use this option if you need to export for a quirky player or parser.
    - for example, RYMCast is picky with format versions. if you're going to use this player, select 1.60.
- **loop**: includes loop information. if disabled, the resulting file won't loop.
- **loop trail**: sets how much of the song is written after it loops.
  - the reason this exists is to work around a VGM format limitation in where post-loop state isn't recorded at all.
  - this may change the song length as it appears on a player.
  - **auto-detect**: detect how much to write automatically.
  - **add one loop**: add one more loop.
  - **custom**: allows you to specify how many ticks to add.
    - `0` is effectively none, disabling loop trail completely.
  - this option will not appear if the loop modality isn't set to None as there wouldn't be a need to.
- **add pattern change hints**: this option adds a "hint" when a pattern change occurs. only useful if you're a developer.
  - the format of the "hint" data block that gets written is: `67 66 FE ll ll ll ll 01 oo rr pp pp pp ...`
    - `ll`: length, a 32-bit little-endian number
    - `oo`: order
    - `rr`: initial row (a 0Dxx effect is able to select a different row)
    - `pp`: pattern index (one per channel)
- **direct stream mode**: this option allows DualPCM to work. don't use this for other chips.
  - may or may not play well with hardware VGM players.
- **chips to export**: select which chips are going to be exported.
  - due to VGM format limitations, you can only select up to two of each chip type.
  - some chips will not be available, either because VGM doesn't support these yet, or because you selected an old format version.

## ZSM

ZSM (ZSound Music) is a format designed for the Commander X16 to allow hardware playback.
it may contain data for either YM2151 or VERA chips.
Calliope is one of the programs that supports playback of ZSM files.

the following settings are available:

- **Tick Rate (Hz)**: select the tick rate the song will run at.
  - I suggest you use the same rate as the song's.
  - apparently ZSM doesn't support changing the rate mid-song.
- **loop**: enables loop. if disabled, the song won't loop.
- **optimize size**: removes unnecessary commands to reduce size.

## Text

this option allows you to export your song as a text file.

## Command Stream

this option exports a binary file in Furnace's own command stream format (FCS) which contains a dump of the internal command stream produced when playing the song.

it's not really useful, unless you're a developer and want to use a command stream dump for some reason (e.g. writing a hardware sound driver). see `export-tech.md` in `papers/` for details.

## DMF

this option allows you to save your song as a .dmf which can be opened in DefleMask.

the following systems are supported when saving as 1.0/legacy (0.12):
- Sega Genesis/Mega Drive (YM2612 + SN76489)
- Sega Genesis/Mega Drive (YM2612 + SN76489, extended channel 3)
- Sega Master System
- Game Boy
- PC Engine
- NES
- Commodore 64
- Arcade (YM2151 + SegaPCM 5-channel compatibility)
- Neo Geo CD (DefleMask 1.0+)

the following systems are also supported when saving as 1.1.3+:
- Sega Master System (with FM expansion)
- NES + Konami VRC7
- Famicom Disk System

only use this option if you really need it. there are many features which DefleMask does not support, such as a variety of effects, FM macros and pitched samples, so these will be lost.
