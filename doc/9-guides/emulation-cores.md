# choosing emulation cores

Furnace achieves the authentic sound of videogame hardware by emulating sound chips accurately as possible, using **emulator cores**. in some cases there are multiple cores to choose from, each with different strengths and weaknesses. here are the major differences between them all.

- **Arcade/YM2151 core**:
  - **ymfm**: default. much less CPU usage than Nuked-OPM, but less accurate. recommended for users with last-gen or earlier hardware.
  - **Nuked-OPM**: much more accurate than ymfm, due to the emulator being based on an image of the die map taken from a real YM2151. very CPU heavy, only recommended for users with recent hardware.

- **Genesis/YM2612 core**:
  - **Nuked-OPN2**: default. a little lighter on the CPU than Nuked-OPM.
  - **ymfm**: same as ymfm above.

- **SN76489 core**:
  - **MAME**: default. less accurate than Nuked, but with lower CPU usage. comes from the MAME emulator project.
  - **Nuked-PSG Mod**: more accurate, but not by that much. this originally started as an emulator for the YM7101 PSG sound generator, but was modified to emulate the SN7 as the MAME core was deemed unsatisfactory by some.

- **NES core**:
  - **puNES**: default. it comes from a dedicated NES emulator.
  - **NSFplay**: higher CPU usage than puNES.

- **FDS core**:
  - **puNES**: default. lower CPU usage and far less accurate.
  - **NSFplay**: higher CPU usage and much more accurate.

- **SID core**:
  - **reSID**: default. a high quality emulation core. somewhat CPU heavy.
  - **reSIDfp**: improved version of reSID. the most accurate choice. _extremely_ CPU heavy.
  - **dSID**: a lightweight open-source core used in DefleMask. not so accurate but it's very CPU light.

- **POKEY core**:
  - **Atari800 (mzpokeysnd)**: does not emulate two-tone mode.
  - **ASAP (C++ port)**: default. the sound core used in the ASAP player. most accurate option.

- **OPN/OPNA/OPNB cores**:
  - **ymfm only**: lower CPU usage, less accurate FM.
  - **Nuked-OPN2 (FM) + ymfm (SSG/ADPCM)**: default. more accurate FM at the cost of more CPU load.
