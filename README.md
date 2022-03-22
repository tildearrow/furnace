# Furnace Tracker

![screenshot](papers/screenshot1.png)

Furnace tracker, a multi-system chiptune tracker.

[downloads](#downloads) | [discussion](#discussion) | [help](#help) | [developer info](#developer-info) | [unofficial packages](#unofficial-packages) | [faq](#faq)

***
## downloads
check out the [Releases](https://github.com/tildearrow/furnace/releases) page. available for Windows, macOS and Linux (AppImage).

## features

- supports the following systems:
  - Sega Genesis
  - Sega Master System
  - Game Boy
  - PC Engine
  - NES
  - Commodore 64
  - Yamaha YM2151 (plus PCM)
  - Neo Geo
  - AY-3-8910 (ZX Spectrum, Atari ST, etc.)
  - Microchip AY8930
  - Philips SAA1099
  - Amiga
  - TIA (Atari 2600/7800)
- multiple sound chips in a single song!
- DefleMask compatibility - loads .dmf modules, .dmp instruments and .dmw wavetables
- clean-room design (guesswork and ABX tests only, no decompilation involved)
- bug/quirk implementation for increased playback accuracy
- VGM and audio file export
- accurate emulation cores whether possible (Nuked, MAME, SameBoy, Mednafen PCE, puNES, reSID, Stella, SAASound and ymfm)
- additional features on top:
  - FM macros!
  - negative octaves
  - arbitrary pitch samples
  - sample loop points
  - SSG envelopes in Neo Geo
  - full duty/cutoff range in C64
  - ability to change tempo mid-song with `Cxxx` effect (`xxx` between `000` and `3ff`)
- open-source under GPLv2 or later.

***
# quick references
 - **discussion**: see the [Discussions](https://github.com/tildearrow/furnace/discussions) section, or (preferably) the [official Discord server](https://discord.gg/EfrwT2wq7z).
 - **help**: check out the [documentation](papers/doc/README.md). it's mostly incomplete, but has details on effects.
## unofficial packages
[![Packaging status](https://repology.org/badge/tiny-repos/furnace.svg)](https://repology.org/project/furnace/versions)

some people have provided packages for Unix/Unix-like distributions. here's a list.
 - **Arch Linux**: [furnace-git is in the AUR.](https://aur.archlinux.org/packages/furnace-git) thank you Essem!
 - **FreeBSD**: [a package in ports](https://www.freshports.org/audio/furnace/) is available courtesy of ehaupt.
 - **Nix**: [package](https://search.nixos.org/packages?channel=unstable&show=furnace&from=0&size=50&sort=relevance&type=packages&query=furnace) thanks to OPNA2608.

***
# developer info

[![Build furnace](https://github.com/tildearrow/furnace/actions/workflows/build.yml/badge.svg)](https://github.com/tildearrow/furnace/actions/workflows/build.yml)

**NOTE: do not download the project's source as a .zip or .tar.gz as these do not include the project's submodules which are necessary to proceed with building. please instead use Git as shown below.**

## dependencies

- CMake
- SDL2
- zlib
- JACK (optional)

SDL2 and zlib are included as submodules.

## getting the source

type the following on a terminal/console: (make sure Git is installed)

```
git clone --recursive https://github.com/tildearrow/furnace.git
cd furnace
```

(the `--recursive` parameter ensures submodules are fetched as well)

## compilation

your typical CMake project.

### Windows using MSVC

as of now tildearrow uses MinGW for Windows builds, but thanks to OPNA2608 this works again!

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
Alternatively, build scripts are provided in the `scripts/` folder in the root of the repository.

### CMake options

To add an option from the command-line: `-D<NAME>=<VALUE>`  
Example: `cmake -DBUILD_GUI=OFF -DWARNINGS_ARE_ERRORS=ON ..`

Available options:

| Name | Default | Description |
| :--: | :-----: | ----------- |
| `BUILD_GUI` | `ON` if not building for Android, otherwise `OFF` | Build the tracker (disable to build only a headless player) |
| `WITH_JACK` | `ON` if system-installed JACK detected, otherwise `OFF` | Whether to build with JACK support. Auto-detects if JACK is available |
| `SYSTEM_FMT` | `OFF` | Use a system-installed version of fmt instead of the vendored one |
| `SYSTEM_LIBSNDFILE` | `OFF` | Use a system-installed version of libsndfile instead of the vendored one |
| `SYSTEM_ZLIB` | `OFF` | Use a system-installed version of zlib instead of the vendored one |
| `SYSTEM_SDL2` | `OFF` | Use a system-installed version of SDL2 instead of the vendored one |
| `WARNINGS_ARE_ERRORS` | `OFF` (but consider enabling this & reporting any errors that arise from it!) | Whether warnings in furnace's C++ code should be treated as errors |

## usage

```
./furnace
```

this opens the program.

```
./furnace -console <file>
```

this will play a compatible file.

```
./furnace -console -view commands <file>
```

this will play a compatible file and enable the commands view.

**note that these commands only actually work in Linux environments. on other command lines, such as Windows' Command Prompt, or MacOS Terminal, it may not work correctly.**

***
# notes

> how do I use Neo Geo SSG envelopes?

the following effects are provided:

- `22xy`: set envelope mode.
  - `x` sets the envelope shape, which may be one of the following:
    - `0: \___` decay
    - `4: /___` attack once
    - `8: \\\\` saw
    - `9: \___` decay
    - `A: \/\/` inverse obelisco
    - `B: \¯¯¯` decay once
    - `C: ////` inverse saw
    - `D: /¯¯¯` attack
    - `E: /\/\` obelisco
    - `F: /___` attack once
  - if `y` is 1 then the envelope will affect this channel.
- `23xx`: set envelope period low byte.
- `24xx`: set envelope period high byte.
- `25xx`: slide envelope period up.
- `26xx`: slide envelope period down.

a lower envelope period will make the envelope run faster.

***
# faq

> how do I use C64 absolute filter/duty?

on Instrument Editor in the C64 tab there are two options to toggle these.
also provided are two effects:

- `3xxx`: set fine duty.
- `4xxx`: set fine cutoff. `xxx` range is 000-7ff.
additionally, you can change the cutoff and/or duty as a macro inside an instrument by clicking the `absolute cutoff macro` and/or `absolute duty macro` checkbox at the bottom of the instrument. (for the filter, you also need to click the checkbox that says `volume macro is cutoff macro`.)

> Q: how do I use PCM on a PCM-capable system?

A: Two possibilities: the recommended way is via creating the "Amiga/Sample" type instrument and assigning sample to it, or via old, Deflemask-compatible method, using `17xx` effect

> Q: my song sounds very odd at a certain point

A: file a bug report. use the Issues page. it's probably another playback inaccuracy.

> Q: my song sounds correct, but it doesn't in DefleMask

A: file a bug report **here**. it still is a playback inaccuracy.

> Q: my C64 song sounds terrible after saving as .dmf!

A: that's a limitation of the DefleMask format. save in Furnace song format instead (.fur).

> Q: how do I solo channels?

A: right click on the channel name.

***
# footnotes

copyright (C) 2021-2022 tildearrow and contributors.

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.


despite the fact this program works with the .dmf file format, it is NOT affiliated with Delek or DefleMask in any way, nor it is a replacement for the original program.
