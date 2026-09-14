# export

Furnace allows you to export your song in several formats. this section deals with describing the available export options.

## audio

this option allows you to export your song to another audio format.

- **Export type**:
  - **one file**: exports your song to a single file.
  - **multiple files (one per chip)**: exports the output of each chip to separate files.
  - **multiple files (one per channel)**: exports the output of each channel to separate files.
    - ideal for use with a channel visualizer such as [corrscope](https://github.com/corrscope/corrscope).
- **File Format**: select the output file format. each format has its own options.
  - **Wave**: lossless uncompressed .wav format. largest file size but perfect quality. most useful for files that will need further editing.
    - **Bit depth**: default is 16-bit integer.
      - integer formats should be supported by all players, but are clipped to -1.0/+1.0 range.
      - float formats are supported by most software players and have infinite range, but take more space.
    - **Sample rate**: affects the quality of the output file.
      - default is 44100, "CD quality".
      - lower sample rates lose fidelity as upper frequencies disappear.
      - higher sample rates gain frequencies that can't be heard at the cost of file size and rendering time.
    - **Channels in file**: default is 2 (stereo). set to 1 for mono.
  - **Opus**: modern lossy compressed .ogg format. better audio quality than MP3 or Vorbis. not supported by all players, especially Apple devices.
    - **Channels in file**: default is 2 (stereo). set to 1 for mono.
    - **Bit rate**: higher numbers result in better quality sound at the cost of larger file size. default is 128000.
  - **FLAC (Free Lossless Audio Codec)**: lossless compressed .flac format. good for archival or editing, as it preserves original sound data.
    - **Sample rate**: affects the quality of the output file. see above.
    - **Channels in file**: default is 2 (stereo). set to 1 for mono.
    - **Compression level**: higher levels take slightly longer to pack but yield much better compression. default is 6.0.
  - **Vorbis**: lossy compressed .ogg format. better quality sound than MP3 with smaller file size. fairly common support except for Apple devices.
    - **Sample rate**: affects the quality of the output file. see above.
    - **Channels in file**: default is 2 (stereo). set to 1 for mono.
    - **Quality**: compression rate. higher numbers produce larger files of higher fidelity. default is 8.0.
  - **MP3**: lossy compressed .mp3 format. an old standard for compressed audio;  it enjoys widespread support but has poor quality.
    - **Sample rate**: affects the quality of the output file. see above.
    - **Channels in file**: default is 2 (stereo). set to 1 for mono.
    - **Bit rate mode**:
      - **Constant**: fixed bit rate regardless of content. default.
        - **Bit rate**: higher numbers result in better quality sound at the cost of larger file size. default is 128000.
      - **Variable**: bit rate adapts to the content. generates smaller files for the same quality. not all players support this mode.
        - **Quality**: higher numbers produce larger files of higher fidelity. default is 6.0.        
      - **Average**: a compromise; the steady bit rate of "constant" with higher quality, but less efficiently compressed than "variable". not all players support this mode.
        - **Bit rate**: higher numbers result in better quality sound at the cost of larger file size. default is 128000.
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
- **speed drift compensation**:
  - **none**: normal export.
  - **DeadFish VgmPlay (1.02×)**: adjusts speed to account for inaccuracy in [VgmPlay](https://www.mjsstuf.x10host.com/pages/vgmPlay/vgmPlay.htm), a Sega Genesis VGM player by DeadFish.

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

## ROM

depending on the system, this option may appear to allow you to export your song to a working ROM image or code that can be built into one. export options are explained in the system's accompanying documentation.

the following formats and systems are supported:
- TIunA assembly, using [Atari 2600 (with software pitch driver)](../7-systems/tia.md).
- iPod .tone alarm, using [PC Speaker](../7-systems/pcspkr.md).
- GRUB_INIT_TUNE, using [PC Speaker](../7-systems/pcspkr.md).

## text

this option allows you to export your song as a text file.

## command stream

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
