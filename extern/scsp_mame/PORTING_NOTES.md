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
- [ ] Phase 2 — strip MAME framework from `scspdsp.cpp` (drop `emu.h`,
      replace `address_space *space` with `uint8_t *RAM`).
- [ ] Phase 3 — strip MAME framework from `scsp.{h,cpp}` (drop `device_t`
      bases, sound_stream, emu_timer, save_item, devcb callbacks; expose
      a freestanding `scsp_device` class with a `render(int16_t*, int)`
      method and host-supplied RAM pointer).
- [ ] Phase 4 — implement `scsp_bridge.cpp` for this backend, wrapping a
      single static `scsp_device` instance.
- [ ] Phase 5 — build, smoke test, A/B against aosdk.
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
