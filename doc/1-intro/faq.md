# frequently asked questions

## general

### how do I use Furnace?

the [quick start guide](quickstart.md) and [concepts list](concepts.md) are great first steps in getting used to the tracker way. after that, try exploring the demo songs to learn more about how to get the sounds you want.

### is there an Android build?

yes and no. Furnace can be successfully built for Android and it even has some rudimentary touch UI support, but it's extremely unfinished and extremely unsupported. there are many good reasons that it's not in the official releases yet.

### is there an iOS build?

nope:

- in order to test any iOS app on an actual device, the developer must pay a hefty 99 USD per year to just enroll for Apple Developer Program. Furnace is free and non-profit.
- developing for iOS requires an Apple computer and an Apple device for testing.
- even if an alternate store were used, you would still need a method to sideload an app to your device. jailbreaking an iOS device isn't an easy task.

for these reasons, an iOS build will never happen until a major shift occurs in Apple's policies.

### what are the system requirements for Furnace to run?

Furnace is developed to be able to run on operating systems as old as Windows XP, on downright ancient (Pentium III) 32-bit CPUs, with Linux ABIs dating as old as 2015. safe to assume any computer made past 2010, no matter how underpowered at time of release, will be able to run Furnace perfectly.

|     | **Minimum** | **Recommended** |
|:---:|:------------:|:--------------:|
| OS  | Windows XP SP3 32-bit, Ubuntu 16.04, Android 6.0, or MacOS 10.9 | Windows Vista SP2 64-bit, Ubuntu 20.04, Android 11, or MacOS 10.15 |
| CPU | Intel Pentium 3, AMD Athlon 64, or 32-bit ARMv7 | fourth-generation Intel Ivy Bridge, AMD Zen, or AArch64 ARM CPU* |
| GPU | any (software rendering) | any GPU supporting OpenGL 3.0, OpenGL ES 2.0 or DirectX 11 |
| storage | 25MB (program) + 200MB (config and backups) |60MB (program and included assets) + 1GB (config and backups) |
| RAM | 768 MB | 2 GB |
| display | 800x600 | 1280x720 |

some emulation cores may have higher processor requirements:
- ESFMu: 2.0GHz Skylake/Zen, or 2.9GHz Cortex-A76/Snapdragon 845/Apple M1
- Nuked-OPM and reSID: 3.0GHz Skylake/Zen, or 2.9GHz Cortex-A76/Snapdragon 845/Apple M1
- reSIDfp: 4.0GHz Skylake/Zen 2, or Cortex-A710/Snapdragon 870/Apple M1 

### will Furnace ever have a piano roll or DAW interface?

there are no plans for this. Furnace is a tracker, and changing the interface would involve a long and painful rewrite.

### I've lost my song – what do I do?

Furnace keeps backups of the songs you've worked on before. go to **file > restore backup**.

## workflow

### how do I compose in 6/8, 5/4, or other time signatures?

set the pattern length to a multiple of the number of beats (6 or 5 as mentioned above). don't forget to change the row highlight values to match!

### how do I do triplets, quintuplets, or other tuplets?

there are two common methods:
- use delay commands (`EDxx`) to offset notes into their correct places. this is good for the occasional set of tuplets, but if you expect to use a lot of them...
- plan ahead for the song to have them by making your pattern length a multiple of that number. remember to adjust row highlight values to match.

depending on the tempo of the song, it may only be possible to get perfectly even tuplets by changing the tick rate. mind that this may hinder playback in games or sound engines that use the vertical blank interval for their timing.

### why do certain notes not play low enough, high enough, or in tune?

each chip has its own set of limitations regarding what frequencies it can play. if these limits are likely to be found in normal tracking, they'll be mentioned in [that chip's documentation](../7-systems/).

### can I add effects to the output like EQ or reverb?

not yet, but it's in the early development stage.

## chips

### will Furnace support the Sony PlayStation?

it's in the plans, with no target date.

Sony Playstation uses a stereo 24 channel sample-based synthesizer using a custom ADPCM-like format for samples and providing a 512 kilobytes of RAM for these.

### will Furnace support the Sega Saturn?

it also is in the plans, with no target date.

the Sega Saturn uses an extremely complicated Yamaha YMF292 sound chip, employing both FM and sample based-synthesis with 32 channels (or rather, FM operators). each channel can frequency modulate another channel, which lowers the polyphony count according to how many operators are used per voice, with no clear concept of operator routing.

### will Furnace support the Nintendo 64?

the N64 lacks any form of audio synthesizer chip. many games use MIDI or XM or other such formats internally, but everything is mixed in software and sent to a simple stereo DAC.

### will Furnace support this obscure PCM-only chip?

probably not, as with very few exceptions these are effectively all the same.

### will Furnace support the Roland MT-32?

no. MT-32 is used with MIDI in 99.999% of situations. it lacks a direct register interface.

also, Furnace is not a MIDI tracker....

## importing

### will Furnace import MIDI files?

nope.

trackers aren't exactly optimal for MIDI, compared to dedicated MIDI editors. for example, trackers are designed to only have a single note on/off, not an array of 128 different notes to turn on/off in the same tick and channel. even if one were to make a new 16-channel system which maps to each MIDI channel, true polyphony in the same channel would still be impossible. moreover, dynamic channel allocation and handling effect states between channel would get messy very fast.

for these reasons, Furnace is not a MIDI tracker.

### why does this imported file sound wrong?

there are fundamental differences between formats that cannot be directly translated. an import should always be considered the starting point of a conversion, not a final product.

### can I import VGM or NSF?

nope. it's a feature that's been requested many times, but there are no plans to implement it. VGM files are raw register dumps, and NSF/KSS/SID files are RAM programs with driver code and song data optimized for said driver. neither of them translate nicely into a pattern view.

for NSF import, you can use [a modified version of FamiTracker called NSFImport](http://rainwarrior.ca/projects/nes/nsfimport.html) and then import the resulting .ftm into Furnace. it's all speed 1 though, so don't expect any songs to be nicely laid out with instruments and all.

### how can I use an SF2 soundfont?

one way is to use [OpenMPT](https://openmpt.org/) to open the SF2 file, and save WAV files from there. [Polyphone](https://www.polyphone.io/) is another way.

### how do I import instruments from this SNES game?

use [split700](https://github.com/gocha/split700) to extract the BRR samples from an SPC. there is presently no way to import envelopes or other parameters.

### how do I import instruments from this Sega Genesis game?

extract FM patches from a VGM file using [vgm2pre](https://github.com/vgmtool/vgm2pre) or similar tools like [OPN2 Bank Editor](https://github.com/Wohlstand/OPN2BankEditor) or [YM2608 Tone Editor](https://github.com/rerrahkr/YM2608-Tone-Editor). bear in mind that these are only the parameters for the FM synth, and the way the instrument is heard in-game may include pitch bends or other effects that can't be extracted.

for PSG instruments, see the next question.

### how do I import instruments from this NES/SMS/GB/C64/etc. game?

PSG chips (such as those in the systems mentioned) don't have any inherent concept of instruments or patches. all of that is handled in software, and each sound driver has its own way of doing things. generally, the only option is to recreate the instrument from scratch.

## exporting

### will Furnace export MIDI files?

nope. Furnace is not a MIDI tracker.

### why does this exported VGM sound weird when I play it in other software?

just as Furnace offers a choice of emulation cores, VGM players may use different cores with varying degrees of accuracy. also, some aspects of a song may not be supported by the VGM format, such as chip clock speeds.

### why does this exported DMF sound wrong in DefleMask?

while Furnace did start life as a DMF player, it's grown in functionality quite a bit, and many Furnace features simply don't exist in that format. there are also cases where the emulation cores in DefleMask sound different from those available in Furnace.

### when will Furnace be able to export to a ROM for a particular system or an emulated music format?

each system will need its own method of converting Furnace songs into code that can be played back on hardware. this requires writing a driver for the hardware in question, which is no small task. that having been said, there are several efforts in progress, both for direct export from Furnace itself and external converters such as [furSPC](https://github.com/AnnoyedArt1256/furSPC), [furNES](https://github.com/AnnoyedArt1256/furNES), and [furC64](https://github.com/AnnoyedArt1256/furC64).

Furnace 0.7 is expected to support exporting ROM for SNES, C64 and Game Boy. there is no ETA for this version.

### can Furnace export MP3/OGG/FLAC files?

not presently. for now, use an external converter such as FFmpeg.

## other

if a question isn't answered within this manual, check in the [GitHub Discussions](https://github.com/tildearrow/furnace/discussions) to see if it's answered there, and post if needed.
