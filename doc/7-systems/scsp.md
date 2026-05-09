# Yamaha YMF292 (SCSP)

the Yamaha YMF292 (Saturn Custom Sound Processor — SCSP) is the sound chip in the Sega Saturn. it has 32 voice slots, 512 KB of dedicated sound RAM, a 128-step on-chip DSP, and runs at a fixed 44.1 kHz output rate.

this chip features:

- 32 voice slots — each slot reads a sample from RAM and applies an envelope, LFO, and amplitude control. Furnace exposes 8 logical channels and allocates slots dynamically.
- two synthesis modes per instrument:
  - **PCM**: a single slot plays a sample from RAM (built-in or user). this is what most Saturn games use.
  - **FM**: 1–6 operators are mapped to slots and modulated via the SCSP's slot-to-slot ring buffer. any RAM contents — built-in waveform or user sample — can be used as a carrier or modulator.
- a programmable on-chip DSP with up to 128 micro-instructions, 64 coefficients, 32 address constants, and a 8K–64K-word delay-line ring buffer. used for reverb, delay, chorus, filtering, etc.
- 16-bit signed PCM samples in RAM (no 8-bit / no compression in Furnace's emulation).

## effects

### shared (PCM and FM)

- `11xx`: set LFO frequency (`00`–`1F`).
- `12xx`: set pitch LFO depth (`00`–`07`).
- `13xx`: set amplitude LFO depth (`00`–`07`).
- `14xx`: set key-rate scaling (`00`–`0F`).
- `16xx`: set DSP send level (`00`–`07` — IMXL).
- `17xx`: set DSP pan (`00`–`1F`).
- `18xx`: set direct send level (`00`–`07` — DISDL).
- `19xx`: set direct pan (`00`–`1F`).

### FM-mode only (Furnace performance tools)

these are not preserved by Saturn SEQ / TON export. they exist to let you sequence FM patches inside Furnace.

- `20xx`–`25xx`: set TL (output level) of operator 1 through 6. `00` = loudest, `FF` = silent.
- `30xx`–`35xx`: set modulation depth (MDL) of operator 1 through 6. `00`–`0F`.
- `40xx`: set carrier waveform — `0` sine, `1` saw, `2` square, `3` triangle, `4` organ, `5` brass, `6` strings, `7` piano, `8` flute, `9` bass.
- `41xx`: set modulator waveform (same indices as `40xx`).
- `43xx`: set self-feedback amount (`00`–`7F`).

## info

uses the [SCSP](../4-instrument/scsp.md) instrument editor.

- 8 logical channels (one note each), backed by up to 8 hardware slots in PCM mode or `8 × opCount` slots in FM mode.
- 512 KB shared sample RAM. user samples are 16-bit signed PCM. samples that don't fit are truncated at upload time.
- when an on-chip DSP program is loaded, slots 0 and 1 are reserved as the stereo wet bus, so the simultaneous-voice limit drops to 6.

### why 8 channels and not 32?

the SCSP has 32 hardware slots, and there's no distinct "operator" register — an FM operator *is* a slot whose phase output is routed to another slot via the slot-to-slot ring buffer. so "32 slots" and "up to 32 operators" are the same resource. a single PCM voice uses 1 slot; a 4-op FM voice uses 4. the chip is one shared pool of 32 resources, mixed across PCM and FM however you like.

exposing all 32 as fixed tracker channels would be misleading: a row that columns "channel 5" would mean different things depending on what instrument was loaded — sometimes it's a free voice, sometimes it's the third operator of a 4-op patch you placed two columns to the left. that breaks pattern preview, rendering, and the tracker's mental model.

instead, Furnace exposes 8 logical channels (one note each). a runtime allocator reserves contiguous slot runs per note: 1 slot for PCM, `opCount` slots for FM, with LRU stealing on overflow. eight 4-op voices would technically fit (8 × 4 = 32 slots, the chip's exact capacity), but the FM ring-buffer math requires each voice's operators to occupy adjacent slot indices, so the slots are split into "FM voices" of one fixed size at allocation time. eight channels covers a typical Saturn-game arrangement (lead + bass + 2 accompaniment + 4 drum/SFX). real Saturn games drive voice allocation from SEQ events at a finer granularity than the tracker model — Furnace's 8-channel grid is a deliberate simplification for editing.

## chip config

no chip-specific options. the SCSP runs at its fixed Saturn clock (22.5792 MHz) and 44.1 kHz output rate.

## TON / SEQ export

`File → Export → Saturn TON Bank` produces a `.ton` instrument bank compatible with the [mid2seq](https://github.com/jfsantos/mid2seq) / saturn_kit toolchain and the SGL sound driver. one voice per FM-mode SCSP instrument; PCM-mode instruments and Furnace-specific FM effect codes (20xx–43xx) are not exported.

`.ton` files can also be loaded via `File → Open Instrument` — each voice becomes one Furnace SCSP instrument, with built-in waveforms recognized by content match and non-matching PCM imported as new samples.

## DSP

open the `Window → SCSP DSP` panel to edit the on-chip DSP program. the source is bebhionn-style USC text; click **Apply** to assemble and push it to the chip without resetting voices. the program is saved with the song.

- writes to `EFREG00` and `EFREG01` are routed to the L/R output mixer automatically.
- inputs to the DSP come from any voice with `IMXL > 0` (set via the instrument's `IMXL` field, or per-row with the `16xx` effect). `ISEL` selects which `MIXSxx` bus the voice writes to.
- ring-buffer length is set by the **RBL** combo: 0=8K (≈186 ms), 1=16K (≈372 ms), 2=32K (≈743 ms — default), 3=64K (≈1.49 s). pick the smallest one whose buffer length exceeds your longest delay-line read.
