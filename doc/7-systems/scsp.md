# Yamaha YMF292 (SCSP)

the Yamaha YMF292 (Saturn Custom Sound Processor — SCSP) is the sound chip in the Sega Saturn. it has 32 voice slots, 512 KB of dedicated sound RAM, a 128-step on-chip DSP, and runs at a fixed 44.1 kHz output rate.

this chip features:

- 32 voice slots — each slot reads a sample from RAM and applies an envelope, LFO, and amplitude control. Furnace exposes all 32 slots as tracker channels (Slot 1 .. Slot 32) with a 1:1 chan→slot anchor mapping.
- two synthesis modes per instrument:
  - **PCM**: a single slot plays a sample from RAM. this is what most Saturn games use.
  - **FM**: 1–32 operators are mapped to consecutive slots starting at the channel's anchor (op N lands on anchor+N) and modulated via the SCSP's slot-to-slot ring buffer. any user sample can be used as a carrier or modulator.
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

- `20xx`–`3Fxx`: set TL (output level) of operator 1 through 32. `00` = loudest, `FF` = silent.
- `40xy`: set modulation depth (MDL) of operator x (1–16) to y (`0`–`F`).
- `41xy`: set MDL of operator x+16 (so x=1 → op 17, x=16 → op 32) to y.
- `43xx`: set self-feedback amount (`00`–`7F`).

## info

uses the [SCSP](../4-instrument/scsp.md) instrument editor.

- 32 tracker channels, one per hardware slot. a note on Slot N keys hardware slot N; an N-op FM patch on Slot N occupies slots N .. N+opCount-1.
- 512 KB shared sample RAM. user samples are 16-bit signed PCM. samples that don't fit are truncated at upload time.
- when an on-chip DSP program is loaded, slots 0 and 1 are reserved as the stereo wet bus and notes on Slot 1 / Slot 2 are suppressed for the duration.

### channel ↔ slot mapping

each tracker channel maps directly to one hardware slot — Slot 1 is hardware slot 0, Slot 2 is slot 1, …, Slot 32 is slot 31. for PCM and 1-op FM, only the anchor slot is used. for multi-op FM, op 0 lands on the anchor and ops 1..N-1 land on the following slots.

if you key a multi-op FM patch on Slot N and any other channel currently owns one of slots N .. N+opCount-1, that other channel's note is silently stolen (key-off + slot release). this lets you mix any combination of PCM and FM patches across the 32 slots — e.g. eight 4-op voices fill the slot pool exactly, but you can also run 16 PCM + four 4-op voices, or 32 PCM voices, or one 32-op FM patch on Slot 1.

ops past the end of the slot pool (e.g. a 4-op patch keyed on Slot 30) are silently truncated to fit. similarly, ops landing on a DSP-pinned slot suppress the entire note while the DSP is active.

## chip config

no chip-specific options. the SCSP runs at its fixed Saturn clock (22.5792 MHz) and 44.1 kHz output rate.

## TON / SEQ export

`File → Export → Saturn TON Bank` produces a `.ton` instrument bank compatible with the [mid2seq](https://github.com/jfsantos/mid2seq) / saturn_kit toolchain and the SGL sound driver. one voice per FM-mode SCSP instrument; PCM-mode instruments and Furnace-specific FM effect codes (20xx–43xx) are not exported. the TON format itself supports up to 32 layers per voice (one per hardware slot), but most distributed SGL setups were authored with the 6-op convention — a warning is logged when exporting voices with more than 6 ops.

`.ton` files can also be loaded via `File → Open Instrument` — each voice becomes one Furnace SCSP instrument, with built-in waveforms recognized by content match and non-matching PCM imported as new samples.

## DSP

open the `Window → SCSP DSP` panel to edit the on-chip DSP program. the source is USC (micro-source) text; click **Apply** to assemble and push it to the chip without resetting voices. the program is saved with the song.

- writes to `EFREG00` and `EFREG01` are routed to the L/R output mixer automatically.
- inputs to the DSP come from any voice with `IMXL > 0` (set via the instrument's `IMXL` field, or per-row with the `16xx` effect). `ISEL` selects which `MIXSxx` bus the voice writes to.
- ring-buffer length is set by the **RBL** combo: 0=8K (≈186 ms), 1=16K (≈372 ms), 2=32K (≈743 ms — default), 3=64K (≈1.49 s). pick the smallest one whose buffer length exceeds your longest delay-line read.
