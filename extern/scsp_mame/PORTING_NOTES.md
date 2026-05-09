# MAME SCSP Extraction Notes

This directory holds a freestanding extraction of the SCSP (YMF292-F) emulator
from the MAME project. The goal is to replace the aosdk-derived sources in
`extern/scsp/` (which carry the pre-2016 MAME non-commercial license) with
GPL-compatible BSD-3-Clause sources, so that the SCSP integration can be
upstreamed to Furnace.

## Source

Files copied verbatim from `mame/src/devices/sound/`:

- `scsp.h`
- `scsp.cpp`
- `scspdsp.h`
- `scspdsp.cpp`

Each file has its own SPDX header declaring `BSD-3-Clause` as the per-file
license — see `LICENSE` for the rationale and full text.

## Lineage

Same authors as the aosdk lineage (ElSemi, R. Belmont; bugfixes by
kingshriek), so the emulator behavior should be very close. The MAME copy
has had post-2008 fixes that the aosdk snapshot does not, so output will
not be sample-identical to the aosdk backend.

## Build flag

The CMake option `SCSP_BACKEND` selects which sources are compiled:

- `SCSP_BACKEND=aosdk` (default, current) — compiles `extern/scsp/`
- `SCSP_BACKEND=mame` — compiles `extern/scsp_mame/` (this directory)

Both backends implement the C API in `extern/scsp/scsp_bridge.h`, so
`src/engine/platform/scsp.cpp` does not need to change.

## Extraction status

- [x] Phase 1 — sources vendored unmodified, build-flag scaffolding added.
- [x] Phase 2 — `scspdsp.{h,cpp}` stripped of MAME framework. `emu.h` gone;
      `address_space *space` replaced with host-supplied `uint8_t *RAM` +
      `RAMMask`; `BIT()` and `util::sext()` reimplemented locally; MAME
      integer aliases swapped for `<cstdint>` types; `std::clamp` (C++17)
      replaced with a local helper since Furnace builds as C++14.
- [x] Phase 3 — `scsp.{h,cpp}` rewritten as a freestanding class. Drops
      `device_t`/`device_sound_interface`/`device_rom_interface` bases;
      replaces `sound_stream` with `render(int16_t*, int n_frames)`;
      replaces `emu_timer` with sample-counted countdown timers ticked in
      `render()`; replaces `devcb_write*` with plain function-pointer
      callbacks (`set_irq_cb`, `set_main_irq_cb`); replaces
      `device_rom_interface::read_byte/read_word` with inline BE
      accessors off the host-supplied `m_RAM[]`; replaces
      `machine().rand()` with a local LCG; drops all ~70 `save_item()`
      calls and the `device_post_load`/`device_clock_changed`/
      `rom_bank_pre_change`/`sound_stream_update` overrides;
      `set_output_gain()` becomes a per-channel `m_output_gain[]`
      multiplier applied in `render()`. MAME integer aliases are kept
      via local `using` declarations so the algorithmic code matches
      upstream line-for-line.
- [x] Phase 4 — `scsp_bridge.cpp` implements the same C API as the
      aosdk bridge on top of a single static `scsp_device` instance plus
      a host-owned `sat_ram[SCSP_RAM_SIZE]`. DSP poke functions write
      directly to `m_DSP.{COEF,MADRS,MPRO,RBL,RBP}` (same layout as the
      aosdk DSP struct, so the byte-order/field-mapping logic from the
      aosdk bridge transfers verbatim). Furnace links cleanly with
      `cmake -DSCSP_BACKEND=mame ..`.
- [ ] Phase 5 — runtime smoke test, A/B against aosdk.
- [ ] Phase 6 — once parity is verified, delete `extern/scsp/` and flip the
      default to `mame`.

## What gets stripped (Phase 3 details)

The modern MAME SCSP is written as a `device_t` subclass that expects to
live inside a MAME machine. For freestanding use we strip:

| MAME-ism | Replacement |
|---|---|
| `device_t`, `device_sound_interface` bases | none — freestanding class |
| `device_rom_interface` base | host-supplied `uint8_t *m_RAM` (512 KB sound RAM) |
| `device_start()`, `device_post_load()`, `device_clock_changed()` | constructor + explicit `init()` |
| `sound_stream * m_stream` + `sound_stream_update()` | `void render(int16_t* out, int n_frames)` |
| `emu_timer * m_timerA/B/C` + `TIMER_CALLBACK_MEMBER` | plain `int` counters ticked per sample |
| `devcb_write8 m_irq_cb`, `devcb_write_line m_main_irq_cb` | `void (*irq_cb)(int)` function pointer |
| ~70 `save_item(NAME(...))` calls | dropped — Furnace does not use MAME savestates |
| `logerror(...)` | `#define logerror(...) ((void)0)` |
| `space->read_word(ADDR)` (in `scspdsp.cpp`, MRT/MWT instructions) | inline BE word read off `m_RAM[]` |

The DSP struct (`SCSPDSP`) layout — `COEF[64]`, `MADRS[32]`, `MPRO[128*4]`,
`RBP`, `RBL` — is identical to what `src/engine/scspdspasm.cpp` emits, so
the assembler plugs in unchanged.
