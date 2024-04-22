# v1.5.9 (2022-09-21)
- Fix the envelope threshold for DAMP to ATTACK state transition (Issue #12).

# v1.5.7 (2022-09-14)
- Silence some pedantic warnings.
- Update minimum cmake version to 3.0.
- Fix the problem where min/max function conflict with the Visual C++ macros.

# v1.5.6 (2021-02-28)
- Update YMF281 ROM patches.

# v1.5.5 (2021-02-05)
- Fix the problem where the output sound is broken due to the mixing of integer and floating point types in the process of rate conversion calculation (degraded at v1.5.4).

# <s>v1.5.4 (2021 02-04)</s>
- Fix the problem where the internal sample rate is calculated as int instead of double.
- Replace older "OPLL_dump2patch" to "OPLL_dumpToPatch".

# v1.5.3 (2021 01-31)
- Change min/max macros to inline functions to suppress compiler errors/warnings.
 
# v1.5.2 (2020 03-04)
- Fix unused constants and variables.
- Fix comments.

# v1.5.1 (2020 02-18)
- Fix piano attack rate.

# v1.5.0 (2020 02-12)
- Fix the modulator decay rate of the acoustic bass patch.
- Fix the modulator's key-off release rate.
- Do not reset carrier's phase when modulator DP finishes.
- Remove deferred rhythm mode switching.
- Improve white noise emulation.

# v1.4.0 (2020 02-08)
- Refactor API and internals.
- Add OPLL_setChipType. OPLL_setChipMode is deprecated.

# v1.3.0 (2020 02-03)
- Add fine-grained panning (OPLL_setPanFine).

# v1.2.7 (2020 01-12)
- Reactivate output array of carrier slot for backward compatibility.

# v1.2.6 (2020 01-11)
- Fix [timing of envelope damping](https://github.com/digital-sound-antiques/emu2413/wiki/Envelope-Damp-and-KeyOn-Noise).

# v1.2.4 (2020 01-07)
- Fix top-cym and hi-hat calculation.

# v1.2.3 (2020 01-07)
- Remove modulator phase delay.

# v1.2.2 (2020 01-06)
- Fix envelope behavior if ARx4+Rks >= 60 is set during attack phase.
- Tweak ROM voice parameters.
- Refactor envelope generator.

# v1.2.0 (2020 01-05)
- Support mirror registers: 0x19-1f, 0x29-1f and 0x39-3f.
- Fix feedback model.

# v1.1.0 (2020 01-03)
Major Update: playback quality and emulation accuracy have been improved drastically.

- Improve [ROM instruments](https://github.com/digital-sound-antiques/emu2413/wiki/YM2413-Estimated-ROM-Instruments).
- Change dB-based sine and exp tables to log2-based.
- Improve damper rate when key-on.
- Improve pitch and amplitude modulator.
- Improve envelope generator.
- Fix the problem where key-on flags are not shared between rhythm and melody slots.
- Improve internal [sample rate converter](https://github.com/digital-sound-antiques/emu2413/wiki/Sample-Rate-Converter).
- Implement test register.
  - Both [test mode](https://github.com/digital-sound-antiques/emu2413/wiki/DAC-in-test-mode) and [non-test mode](https://github.com/digital-sound-antiques/emu2413/wiki/Use-FM-channel-as-DAC) DAC patterns are supported.
  - There are still very few VGMs using YM2413 DAC on the web. If you would like to test it, try [vgm-conv](https://github.com/digital-sound-antiques/vgm-conv) which is capable to generate DAC stream from YM2612 VGM files.
- Semantic versioning.
- Support VS2010 again.

# v0.74 (2019 10-24)

- Fix broken AM and PM waves.

# v0.73 (2019 10-22)

- Fix top-cym volume.

# v0.72 (2019 10-21)

- Fix critical bug on force damp routine.
- Fix top-cym, hi-hat waveform and white noise freq.

# v0.71 (2019 10-20)

- Fix too strong LPF on rate conversion.
- Improve shape of envelope in attack phase.

# v0.70 (2019 10-13)

- Force to damp before keyon
- Dump size changed from to 8 bytes per voice.
- Replaced snare, hi-hat, top-cym generator.

# v0.65 (2019 05-24)

- Fix YM2413 and VRC7 patches.

# v0.63 (2016 09-06)

- Support per-channel output.

# v0.62 (2015 12-13)

- Changed own integer types to C99 stdint.h types.

# v0.61 (2004 04-10)

- Added YMF281B tone (defined by Chabin).

# v0.30 (2001 01-16)

- 1st beta release.

# v0.20 (2001 01-15)

- 1st alpha release.

# v0.10 (2001 01-08)

- 1st experimental version.
