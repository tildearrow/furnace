The canonical location of this document can be found at https://github.com/X16Community/x16-docs/blob/master/X16%20Reference%20-%20Appendix%20G%20-%20ZSM%20File%20Format.md

#### Zsound Repo

The Zsound suite of Commander X16 audio tools contains the original ZSM reference player, found at:  
https://github.com/ZeroByteOrg/zsound/

A newer, more flexible, and more recently maintained library with PCM support, ZSMKit, is avalable at:  
https://github.com/mooinglemur/ZSMKit/

#### Current ZSM Revision: 1

ZSM is a standard specifying both a data stream format and a file structure for containing the data stream. This document provides the standard for both the ZSM stream format and for the ZSM container file format.

Whenever it becomes necessary to modify the ZSM standard in such a way that existing software will not be compatible with files using the newer standard, this version number will be incremented, up to a maximum value of 254.

Version 255 (-1) is reserved for internal use by the player

#### Headerless Data File Format:

 Since Kernal version r39, it is possible to load data files that do not have the CBM 2-byte load-to-address header. As of version r41, this functionality is equally accessible in the standard interactive BASIC interface. As the "PRG" header is no longer necessary, ZSM files will NOT contain this header in order to appear as any other common data file such as ``.wav``, ``.png``, etc. As such, users and programs must use the "headerless mode" when loading a ZSM into memory on the Commander X16. The previously-suggested dummy PRG header has been incorporated to the ZSM header as a magic header for file identity verification purposes.


## ZSM file composition

 Offset|Length|Field
 --|--|--
 0x00|16|ZSM HEADER
 0x10|variable|ZSM STREAM
 ?|?|(optional) PCM HEADER
 ?|variable|(optional) PCM DATA

### ZSM Header

The ZSM header is 16 bytes long.

- All multi-byte values are little endian unless specified otherwise
- All offsets are relative to the beginning of the ZSM header

Offset|Length|Field|Description
---|---|---|---
0x00|2|Magic Header| The string 'zm' (binary 0x7a 0x6d)
0x02|1|Version| ZSM Version. 0-0xFE (0xFF is reserved)
0x03|3|Loop Point|Offset to the starting point of song loop. 0 = no loop.
0x06|3|PCM offset|Offset to the beginning of the PCM index table (if present). 0 = no PCM header or data is present.
0x09|1|FM channel mask|Bit 0-7 are set if the corresponding OPM channel is used by the music.
0x0a|2|PSG channel mask|Bits 0-15 are set if the corresponding PSG channel is used by the music.
0x0c|2|Tick Rate|The rate (in Hz) for song delay ticks. *60Hz (one tick per frame) is recommended.*
0x0e|2|reserved| Reserved for future use. Set to zero.


### ZSM Music Data Stream Format

Byte 0|Byte 1 (variable)|Byte n|Byte n+1 (variable)|...|End of stream
---|---|---|---|---|---
CMD|DATA|CMD|DATA|...|0x80

#### CMD (command) byte values
CMD bytes are bit-packed to hold a command Type ID and a value (n) as follows:

CMD|Bit Pattern|Type|Arg. Bytes|Action
---|--|--|--|-----
0x00-0x3F|`00nnnnnn`|PSG write|1  | Write the following byte into PSG register offset *n*. (from 0x1F9C0 in VRAM)
 0x40     |`01000000`|EXTCMD  |1+?| The following byte is an extension command. (see below for EXTCMD syntax)
 0x41-0x7F|`01nnnnnn`|FM write |2*n*  | Write the following *n* reg/val pairs into the YM2151.
0x80|`10000000`|EOF   |0  |This byte MUST be present at the end of the data stream. Player may loop or halt as necessary.
0x81-0xFF|`1nnnnnnn`|Delay   |0  |Delay *n* ticks.

#### EXTCMD:
The EXTCMD byte is formatted as `ccnnnnnn` where `c`=channel and `n`=number of bytes that follow. If the player wishes to ignore a channel, it can simply advance `n` bytes and continue processing. See EXTCMD Channel Specifications below for more details.

### PCM Header

If the PCM header exists in the ZSM file, it will immediately follow the 0x80 end-of-data marker. The PCM header exists only if at least one PCM instrument exists.

Since each instrument defined is 16 bytes, the size of the PCM header can be calculated
as 4+(16*(last_instrument_index+1)).

Offset|Type|Value
--|--|--
0x00-0x02|String|"PCM"
0x03|Byte|The last PCM instrument index
0x04-0x13|Mixed|Instrument definition for instrument 0x00
0x14-0x23|Mixed|(optional) Instrument definition for instrument 0x01
...

### Instrument definition
Offset|Type|Value
--|--|--
0x00|Byte|This instrument's index
0x01|Bitmask|AUDIO_CTRL: 00**DC**0000: D is set for 16-bit, and clear for 8-bit. C is set for stereo and clear for mono
0x02-0x04|24-bit int|Little-endian offset into the PCM data block
0x05-0x07|24-bit int|Little-endian length of PCM data
0x08|Bitmask|Features: **L**xxxxxxx: L is set if the sample is looped
0x09-0x0b|24-bit int|Little-endian loop point offset (relative, 0 is the beginning of this instrument's sample)
0x0c-0x0f|...|Reserved for expansion

Any offset values contained in the PCM data header block are relative to the beginning of the PCM sample data section, not to the PCM header or ZSM header. The intention is to present the digital audio portion as a set of digi clips ("samples" in tracker terminology) whose playback can be triggered by EXTCMD channel zero.

### PCM Sample Data

This is a blob of PCM data with no internal formatting. Offsets into this blob are provided via the PCM header. The end of this blob will be the end of the ZSM file.

## EXTCMD Channel Scifications

Extension commands provide optional functionality within a ZSM music file. EXTCMD may be ignored by any player. EXTCMD defines 4 "channels" of message streams. Players may implement support for any, all, or none of the channels as desired. An EXTCMD may specify up to 63 bytes of data. If more data than this is required, then it must be broken up into multiple EXTCMDs.

##### EXTCMD in ZSM stream context:

...|CMD 0x40|EXTCMD|N bytes|CMD|...
---|---|---|---|---|---

##### EXTCMD byte format:

Bit Pattern|C|N
--|--|--
`ccnnnnnn`|Extension Channel ID|Number of bytes that follow



##### EXTCMD Channels:
0. PCM instrument channel
1. Expansion Sound Devices
2. Synchronization events
3. Custom

The formatting of the data within these 4 channels is presently a work in progress. Definitions for channels 0-3 will be part of the official ZSM specifications and implemented in the Zsound library. Significant changes within one of these three channels' structure may result in a new ZSM version number being issued. The formatting and content of the 3 official EXTCMD channels will be covered here.

The Custom channel data may take whatever format is desired for any particular purpose with the understanding that the general ecosystem of ZSM-aware applications will most likely ignore them.

### EXTCMD Channel:
#### 0: PCM audio

This EXTCMD stream can contain one or more command + argument pairs.

command|meaning|argument|description
---|---|---|---
0x00|AUDIO_CTRL byte|byte|This byte sets PCM channel volume and/or clears the FIFO
0x01|AUDIO_RATE byte|byte|A value from 0x00-0x80 to set the sample rate (playback speed)
0x02|Instrument trigger|byte|Triggers the PCM instrument specified by this byte index

#### 1: Expansion Sound Devices

This channel is for data intended for "well-known" expansion hardware used with the Commander X16. As the community adopts various expansion hardware, such devices will be given a standard "ID" number so that all ZSM files will agree on which device is being referenced by expansion HW data.

The specification of new chip IDs should not affect the format of ZSM itself, and thus will not result in a ZSM version update. Players will simply need to update their list of known hardware.

Players implementing this channel should implement detection routines during init to determine which (if any) expansion hardware is present. Any messages intended for a chip that is not present in the system should be skipped.

An expansion HW write will contain the following data:

Chip ID|`nnnnnn-1` bytes of data
--|--
one byte|nnnnnn-1 bytes

- The length of the EXTCMD `nnnnnn` encompasses the chip_id byte and the data bytes which follow.

##### Chip IDs
* 0x00 - reserved
* 0x01 - MIDI 1 (Primary MIDI)
    * MIDI data embedded in ZSM is limited to status bytes 0x80-0xF8 inclusive, and their arguments, i.e. data which is intended to be transmitted to a synthesizer.
    * Metadata such as MIDI header data, ticks per beat, track names, lyrics, and tempo are not included in the data.
    * Delta times are not embedded in the MIDI data. Native ZSM delays are used instead.
    * With the exception of multiple EXTCMDs for the same Chip ID within the same tick, the first byte of data must be a status byte 0x80-0xF8.
    * Status continuation is not permitted from prior ticks, but is allowed within the same tick.
        * This restriction allows for simultaneous access to the MIDI device by multiple agents, such as a song player and sound effects player on non-overlapping MIDI channels.
    * An example of an EXTCMD containing a single note-on event might look as follows: `0x40 0x44 0x01 0x90 0x69 0x7F`
* 0x02 - MIDI 2 (Secondary MIDI)
    * A second separate MIDI stream can be used by players that support multiple active MIDI devices. The data format and restrictions are the same as for Chip ID 0x01.
* 0x03 - reserved
* 0x70-0x7F - Private use area
    * These will be ignored by the stock reference players, but can be used for testing or for custom purposes for a particular application.
* 0x80-0xFF - reserved for possible expansion to 15-bit chip IDs

#### 2: Synchronization Events

The purpose of this channel is to provide for music synchronization cues that applications may use to perform operations in sync with the music (such as when the Goombas jump in New Super Mario Bros in time with the BOP! BOP! notes in the music). It is intended for the reference player to provide a sync channel callback, passing the data bytes to the callback function, and then to proceed with playback.

The synchronization format currently defines these event types:

Event Type|Description|Message Format
--|--|--
`0x00`|Generic sync message|`xx` (any value from `0x00`-`0xff`)
`0x01`|Song tuning|a signed byte indicating an offset from A-440 tuning represented in 256ths of a semitone

An example of an EXTCMD containing one sync event might look as follows: `0x40 0x82 0x00 0x05`


#### 3: Custom

The purpose for this channel is that any project with an idea that does not fit neatly into the above categories may pack data into the project's music files in whatever form is required. It should be understood that these ZSMs will not be expected to use the extended behaviors outside of the project they were designed for. The music itself, however, should play properly. The only constraint is that the data must conform to the EXTCMD byte - supplying exactly the specified number of bytes per EXTCMD.

The reference playback library in Zsound will implement this channel as a simple callback passing the memory location and data size to the referenced function, and take no further action internally.
