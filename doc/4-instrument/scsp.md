# Yamaha YMF292 (SCSP) instrument editor

the SCSP instrument editor has tabs for the synthesis mode, envelope, LFO, DSP routing, and macros. the **mode** selector at the top switches between PCM (sample playback) and FM (1–6 operators routed to hardware slots).

## mode

- **PCM**: single sample playback. set a sample (built-in or user) and tune the slot envelope/LFO. typical for sequenced PCM songs.
- **FM**: 1–6 operators with FM modulation. each operator can use a built-in waveform or a user sample (the SCSP doesn't distinguish — every slot reads RAM, FM is just slot-to-slot routing of the result).

## envelope

shared between PCM (the slot envelope) and FM (the per-op envelope, for ops not overridden in the FM panel):

- **AR**: attack rate (`00`–`1F`).
- **D1R**: decay 1 rate (`00`–`1F`).
- **D2R**: decay 2 rate (`00`–`1F`).
- **DL**: decay level (`00`–`1F`).
- **RR**: release rate (`00`–`1F`).
- **KRS**: key-rate scaling (`00`–`0F`). set to `0F` to disable scaling — matches the documented rate tables.
- **EGHOLD**: hold the envelope at full level until note-off.

## LFO

- **LFO frequency**: `00`–`1F`.
- **PLFOWS / PLFOS**: pitch LFO waveform and scale.
- **ALFOWS / ALFOS**: amplitude LFO waveform and scale.
- **LFO reset on key-on**: reset the LFO phase whenever a new note starts.

## DSP routing

these route the slot's audio to and from the on-chip DSP unit (configurable via `Window → SCSP DSP`).

- **IMXL** (DSP send): `00`–`07`. `0` = no send, `7` = full level. set this to `7` on instruments you want processed by the DSP.
- **ISEL** (DSP input bus select): `00`–`0F`. picks which `MIXS<n>` bus the voice writes to. default `0` matches the typical single-effect program.
- **DISDL / DIPAN**: direct (dry) send level and pan.
- **EFSDL / EFPAN**: effect register send level and pan. these gate the DSP's `EFREG<n>` output — used for advanced custom routing. for a typical wet+dry setup, leave these at the defaults; the DSP unit auto-wires `EFREG00`/`EFREG01` to L/R when a program is loaded.

## FM panel (FM mode only)

- **op count**: `1`–`6`. each operator uses one hardware slot, so a 4-op patch consumes 4 slots per voice.
- **per-op tabs**: select the operator to edit.
- **carrier toggle**: when on, this op's audio reaches the slot output (and through it the direct/effect mixer). when off, the op is a modulator — its output drives other slots' phase via the FM ring buffer but is not heard directly.
- **freq ratio** / **freq fixed**: the operator's frequency relative to the played note (ratio) or in fixed Hz.
- **level**: operator output level (`0`–`127`). roughly `1.0 - tl/128`.
- **AR / D1R / DL / D2R / RR**: per-op envelope.
- **mod source**: index of the operator that modulates this one (`-1` = no FM input).
- **mod depth** (MDL): `0`–`15` modulation index.
- **feedback**: self-modulation amount (`0`–`127`). distinct from "mod source on the same op" — adds harmonics / distortion via the slot's own previous output.
- **waveform** (`0`–`9`): sine, sawtooth, square, triangle, organ, brass, strings, piano, flute, bass.
- **use sample**: when checked, this op reads from a user sample in RAM instead of the built-in waveform. any sample length / loop config is allowed; the FM math assumes a 1024-sample modulator cycle, so a long sample as a modulator will alias (use as a carrier for cleaner results).

## macros

- **Volume**: volume sequence.
- **Arpeggio**: pitch sequence.
- **Pitch**: fine pitch.
- **Panning**: stereo balance.
- **Phase Reset**: trigger sample/waveform restart.
