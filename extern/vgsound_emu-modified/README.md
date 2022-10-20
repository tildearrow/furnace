# vgsound_emu V2 (modified)

This is a C++ library of video game sound chip emulation cores. useful for emulators, chiptune trackers, or players.

This is a modified version of vgsound_emu, tailored for Furnace.

## Important

License is now changed to zlib license in vgsound_emu V2, now you must notify your all modifications.

but [vgsound_emu V1 (pre-V2)](https://gitlab.com/cam900/vgsound_emu/-/tree/V1) is still exists, and it's still distributed under [BSD-3-Clause license](https://spdx.org/licenses/BSD-3-Clause.html).([details](https://gitlab.com/cam900/vgsound_emu/-/blob/V1/LICENSE))

## V2 revision changes

formatting codes with clang-format, Encapsulation for Maintenance, Fix GCC 12, Change license to zlib license for notify modifications in derived works from this cores.

## Changelog

See [here](https://gitlab.com/cam900/vgsound_emu/-/blob/main/CHANGELOG.md).

## License

This software is distributed under [zlib License](https://spdx.org/licenses/Zlib.html), unlike [vgsound_emu V1](https://gitlab.com/cam900/vgsound_emu/-/tree/V1)([standard BSD-3-Clause license](https://spdx.org/licenses/BSD-3-Clause.html)([details](https://gitlab.com/cam900/vgsound_emu/-/blob/V1/LICENSE))).
You must notify your modifications at all files you have modified!

See [here](https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE) for details.

## Folders

- vgsound_emu: base folder
  - src: source codes for emulation cores
    - core: core files used in most of emulation cores
      - vox: Dialogic ADPCM core
    - es550x: Ensoniq ES5504, ES5505, ES5506 PCM sound chip families, 25/32 voices with 16/4 stereo/6 stereo output channels
    - k005289: Konami K005289, 2 timers
    - k007232: Konami K007232, 2 PCM channels
    - k053260: Konami K053260, 4 PCM or ADPCM channels with CPU to CPU communication feature
    - msm6295: OKI MSM6295, 4 ADPCM channels
    - n163: Namco 163, NES Mapper with up to 8 Wavetable channels
    - scc: Konami SCC, MSX Mappers with 5 Wavetable channels
    - vrcvi: Konami VRC VI, NES Mapper with 2 Pulse channels and 1 Sawtooth channel
    - x1_010: Seta/Allumer X1-010, 16 Wavetable/PCM channels
    - template: Template for sound emulation core

## Build instruction

### dependencies

- C++11 (or later)
- CMake (3.1 or later is recommended)
- git (for source repository management)
- MSVC or GCC or Clang (for compile)

### Clone repository

type the following on a terminal/console/shell/whatever:

```
git clone https://gitlab.com/cam900/vgsound_emu.git
cd vgsound_emu
```

### Compile with CMake

#### MSVC

type the following on a developer tools command prompt:

```
mkdir build
cd build
cmake ..
msbuild ALL_BUILD.vcxproj
```

#### MinGW, GCC, Clang with MakeFile

type the following on a terminal/console/shell/whatever:

```
mkdir build
cd build
cmake ..
make
```

### CMake options

To add an CMake option from the command line: ```cmake -D<Insert option name here>=<Value> ..```
You can add multiple option with CMake.

#### Available options

| Options | Available Value | Default | Descriptions |
| :-: | :-: | :-: | :-: |
| VGSOUND_EMU_ES5504 | ON/OFF | ON | Use ES5504 core |
| VGSOUND_EMU_ES5505 | ON/OFF | ON | Use ES5505 core |
| VGSOUND_EMU_ES5506 | ON/OFF | ON | Use ES5506 core |
| VGSOUND_EMU_K005289 | ON/OFF | ON | Use K005289 core |
| VGSOUND_EMU_K007232 | ON/OFF | ON | Use K007232 core |
| VGSOUND_EMU_K053260 | ON/OFF | ON | Use K053260 core |
| VGSOUND_EMU_MSM6295 | ON/OFF | ON | Use MSM6295 core |
| VGSOUND_EMU_NAMCO_163 | ON/OFF | ON | Use Namco 163 core |
| VGSOUND_EMU_SCC | ON/OFF | ON | Use SCC core |
| VGSOUND_EMU_VRCVI | ON/OFF | ON | Use VRC VI core |
| VGSOUND_EMU_X1_010 | ON/OFF | ON | Use X1-010 core |

### Link at another project

Copy this repository as submodule first, type the following on a terminal/console/shell/whatever:

```
git submodule add https://gitlab.com/cam900/vgsound_emu.git <Insert submodule path here>
```

Then, Add following options at your CMakeLists.txt file.

example:

```
add_subdirectory(<Insert submodule path here> [EXCLUDE_FROM_ALL])
...
target_include_directories(<Insert your project name here> SYSTEM PRIVATE <Insert submodule path here>)
target_link_libraries(<Insert your project name here> PRIVATE vgsound_emu)
```

## Contributors

- [cam900](https://gitlab.com/cam900)
- [Natt Akuma](https://github.com/akumanatt)
- [James Alan Nguyen](https://github.com/djtuBIG-MaliceX)
- [Laurens Holst](https://github.com/Grauw)
