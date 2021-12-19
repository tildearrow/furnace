# Furnace

![screenshot](papers/screenshot.png)

this is a work-in-progress editor for DefleMask module files (.dmf).

## features

- supports Sega Genesis, Master System, Game Boy, PC Engine, NES, C64, YM2151/PCM and Neo Geo!
- clean-room design (zero reverse-engineered code and zero decompilation; using official DMF specs, guesswork and ABX tests only)
- bug/quirk implementation for increased playback accuracy
- accurate emulation cores whether possible (Nuked, MAME, SameBoy, Mednafen PCE, puNES, reSID and ymfm)
- open-source. GPLv2.

## downloads

coming very soon!

# developer info

## dependencies

- CMake
- SDL2
- zlib
- JACK (optional)

SDL2 and zlib are included as submodules.

## compilation

your typical CMake project. clone (including submodules) and:

### Windows using MSVC

**no longer tested!** as of now tildearrow uses MinGW for Windows builds...

from the developer tools command prompt:

```
mkdir build
cd build
cmake ..
msbuild ALL_BUILD.vcxproj
```

### macOS and Linux

```
mkdir build
cd build
cmake ..
make
```

## usage

```
./furnace
```

this opens the program.

```
./furnace -console <file>
```

this will play a .dmf file.

```
./furnace -console -view commands <file>
```

this will play a .dmf file and enable the commands view.

```
./furnace -output audio.wav <file>
```

this will render a .dmf file to .wav.

# notes

> my song plays different after looping!

that's because Furnace does not reset channel status when looping.

> my song sounds very odd at a certain point

file a bug report. use the Issues page.

it's probably another playback inaccuracy.

> my song sounds correct, but it doesn't in DefleMask

file a bug report **here**. it still is a playback inaccuracy.
