# Furnace

did i say prepare?

this is a work-in-progress chip music player (currently) for the .dmf format.

## features

- supports Sega Genesis, Master System and Game Boy (for now, with more systems coming soon)
- clean-room design (zero reverse-engineered code and zero decompilation; using official DMF specs, guesswork and ABX tests only)
- bug/quirk implementation for increased playback accuracy
- accurate emulation cores (Nuked, MAME and SameBoy)
- open-source. GPLv2.

## dependencies

SDL2. untested on Windows/macOS.

## compilation

your typical CMake project. clone and:

```
mkdir build
cd build
cmake ..
make
```

## usage

```
./furnace <file>
```

this will play a file (must be in .dmf format).
