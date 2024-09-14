# frequently asked questions

## general

### how do I use Furnace?

the [quick start guide](quickstart.md) and [concepts list](concepts.md) are great first steps in getting used to the tracker way. after that, try exploring the demo songs to learn more about how to get the sounds you want.

### is there an Android build?

yes and no. Furnace has been successfully built for Android and it even has some rudimentary touch UI support, but it's extremely unfinished and extremely unsupported. there are many good reasons that it's not in the official releases yet.

### is there an iOS build?

nope.

### will Furnace ever have a piano roll or DAW interface?

there are no plans for this.

### I've lost my song – what do I do?

Furnace keeps backups of the songs you've worked on before. go to **file > restore backup**.

## tracking

### how do I compose in 6/8, 5/4, or other time signatures?

set the pattern length to a multiple of the number of beats (6 or 5 as mentioned above). don't forget to change the row highlight values to match!

### how do I track triplets, quintuplets, or other tuplets?

there are two common methods:
- use delay commands (`EDxx`) to offset notes into their correct places. this is good for the occasional set of tuplets, but if you expect to use a lot of them...
- plan ahead for the song to have them by making your pattern length a multiple of that number. remember to adjust row highlight values to match.

depending on the tempo of the song, it may only be possible to get perfectly even tuplets by changing the tick rate. mind that the result may not be authentic to hardware.

### why do certain notes not play low enough, high enough, or in tune?

each chip has its own set of limitations regarding what frequencies it can play. if these limits are likely to be found in normal tracking, they'll be mentioned in [that chip's documentation](../7-systems/).

### does Furnace support tunings other than twelve tone equal temperament?

no. both tracker and format are built on that scale and tuning.

### can I add effects to the output like EQ or reverb?

not yet, but it's in the early planning stages.

## chips

### will Furnace support the Sony PlayStation?

it's in the plans, with no target date.

### will Furnace support the Sega Saturn?

it also is in the plans, with no target date.

### will Furnace support more Yamaha FM chips?

once a chip has an available emulation cores written in C++ and licensed under GPL2, it'll be considered for addition.

### will Furnace support the Nintendo 64?

the N64 lacks any form of audio synthesizer chip. many games use MIDI or XM or other such formats internally, but everything is mixed in software and sent to a simple stereo DAC.

### will Furnace support this obscure PCM-only chip?

probably not, as with very few exceptions these are effectively all the same.

### will Furnace support the Roland MT-32?

no. MT-32 is used with MIDI in 99.999% of situations. it lacks a direct register interface.

also, Furnace is not a MIDI tracker....

## importing

### will Furnace import MIDI files?

nope. Furnace is not a MIDI tracker.

### why does this imported file sound wrong?

Furnace makes every attempt to import non-Furnace files as accurately and completely as possible, but there are fundamental differences between formats that cannot be directly translated. an import from a non-Furnace format should always be considered the starting point of a conversion, not a final product.

### can I import VGM or NSF?

nope. it's a feature that's been requested many times, but there are no plans to implement it.

for NSF import, you can use [a modified version of FamiTracker called NSFImport](http://rainwarrior.ca/projects/nes/nsfimport.html) and then import the resulting .ftm into Furnace.
it's all speed 1 though, so don't expect any songs to be nicely laid out with instruments and all.

### how can I use an SF2 soundfont?

one way is to use [OpenMPT](https://openmpt.org/) to open the SF2 file, and save WAV files from there.

### how do I import instruments from this SNES game?

use [split700](https://github.com/gocha/split700) to extract the BRR samples from an SPC. there is presently no way to import envelopes or other parameters.

### how do I import instruments from this Sega Genesis game?

extract FM patches from a VGM file using [vgm2pre](https://github.com/vgmtool/vgm2pre) or similar tool. bear in mind that these are only the parameters for the FM synth, and the way the instrument is heard in-game may include pitch bends or other effects that can't be extracted.

for PSG instruments, see the next question.

### how do I import instruments from this NES/SMS/GB/C64/etc. game?

PSG chips (such as those in the systems mentioned) don't have any inherent concept of instruments or patches. all of that is handled in software, and each sound driver has its own way of doing things. generally, the only option is to recreate the instrument from scratch.

## exporting

### will Furnace export MIDI files?

nope. Furnace is not a MIDI tracker.

### why does this exported VGM sound weird when I play it in other software?

just as Furnace offers a choice of emulation cores, VGM players may use different cores with varying degrees of accuracy. also, some aspects of a song may not be supported by the VGM format, such as chip clock speeds or mixer levels.

### why does this exported DMF sound wrong in DefleMask?

while Furnace did start life as a DMF player, it's grown in functionality quite a bit, and many Furnace features simply don't exist in that format. there are also cases where the emulation cores in DefleMask sound different from those available in Furnace.

### when will Furnace be able to export to a ROM for a particular system or an emulated music format?

each system will need its own method of converting Furnace tracker files into code that can be played back on hardware. this requires writing a driver for the hardware in question, which is no small task. that having been said, there are several efforts in progress, both for direct export from Furnace itself and external converters such as [furSPC](https://github.com/AnnoyedArt1256/furSPC), [furNES](https://github.com/AnnoyedArt1256/furNES), and [furC64](https://github.com/AnnoyedArt1256/furC64).

### can Furnace export MP3/OGG/FLAC files?

not presently. for now, use an external converter such as ffmpeg.

## more help

if a question isn't answered within this manual, check in the [GitHub Discussions](https://github.com/tildearrow/furnace/discussions) to see if it's answered there, and post if needed.
