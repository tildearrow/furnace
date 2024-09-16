# choosing emulation cores

Furnace achieves the authentic sound of videogame hardware by emulating sound chips as accurately as possible, using **emulator cores**. in some cases there are multiple cores to choose from, each with different strengths and weaknesses. here are the major differences between them all.

- **YM2151 core**:
  - **ymfm**: default playback core. much less CPU usage than Nuked-OPM, but less accurate. recommended for users with mobile, last-gen or earlier hardware.
  - **Nuked-OPM**: default render core. much more accurate than ymfm, due to the emulator being based on an image of the die map taken from a real YM2151. very CPU heavy, only recommended for users with recent hardware.

- **YM2612 core**:
  - **Nuked-OPN2**: default core. lighter on the CPU than Nuked-OPM, can be used to simulate any variant of YM2612.
  - **ymfm**: same as ymfm above.
  - **YMF276-LLE**: a new core written by the author of the Nuked cores, specifically focused on YMF276 emulation. it is very slow and not useful for real-time playback. Produces audio comparable to output of Sega Mega Drive model 2 and later Fujitsu FM Towns computers, doesn't emulate DAC distortion of the original YM2612.

- **SN76489 core**:
  - **MAME**: default core. less accurate than Nuked, but with lower CPU usage. comes from the MAME emulator project.
  - **Nuked-PSG Mod**: more accurate, but not by that much. this originally started as an emulator for the YM7101 PSG sound generator, but was modified to emulate the SN7 as the MAME core was deemed unsatisfactory by some.

- **NES core**:
  - **puNES**: default core. it comes from a dedicated NES emulator.
  - **NSFplay**: higher CPU usage than puNES.

- **FDS core**:
  - **puNES**: default playback core. lower CPU usage and far less accurate.
  - **NSFplay**: default render core. higher CPU usage and much more accurate.

- **SID core**:
  - **reSID**: default playback core. a high quality emulation core. somewhat CPU heavy.
  - **reSIDfp**: default render core. improved version of reSID. the most accurate choice. _very_ CPU heavy.
  - **dSID**: a lightweight open-source core used in DefleMask. not so accurate but it's very CPU light.

- **POKEY core**:
  - **Atari800 (mzpokeysnd)**: does not emulate two-tone mode.
  - **ASAP (C++ port)**: default core. the sound core used in the ASAP player. most accurate option.

- **OPN/OPNA/OPNB cores**:
  - **ymfm only**: lower CPU usage, less accurate FM.
  - **Nuked-OPN2 (FM) + ymfm (SSG/ADPCM)**: default cores. more accurate FM at the cost of more CPU load.
  - **YM2608-LLE**: a new core written by the author of the Nuked cores. high accuracy, but _extremely_ high CPU usage, far beyond any other emulation core. (over 500% CPU time on 10th gen Intel Core i5!) 

- **OPL/OPL2/Y8950 core**:
  - **Nuked-OPL3**: high quality OPL emulation core. slightly off due to tiny differences between OPL and OPL3, but otherwise it is good.
  - **ymfm**: this core is supposed to use less CPU than Nuked-OPL3, but for some reason it actually doesn't.
  - **YM3812-LLE**: a new core written by the author of the Nuked cores. it features _extremely_ accurate emulation.
    - this core uses a *lot* of CPU time. may not be suitable for playback!

- **OPL3 core**:
  - **Nuked-OPL3**: high quality OPL emulation core.
  - **ymfm**: this core is supposed to use less CPU than Nuked-OPL3, but for some reason it actually doesn't.
  - **YMF262-LLE**: a new core written by the author of the Nuked cores. it features _extremely_ accurate emulation.
    - this core uses even more CPU than YM3812-LLE. not suitable for playback or even rendering if you're impatient!

- **ESFM core**:
  - **ESFMu**: the ESFM emulator. best choice but CPU intensive.
  - **ESFMu (fast)**: this is a modification of ESFMu to reduce CPU usage at the cost of less accuracy.

- **OPLL core**:
  - **Nuked-OPLL**: this core is accurate and the default.
  - **emu2413**: a less accurate core that uses less CPU.

- **AY-3-8910/SSG core**:
  - **MAME**: default core.
  - **AtomicSSG**: SSG core extracted from YM2608-LLE.
