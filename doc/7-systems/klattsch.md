# klattsch

a Klatt 1980 style parallel-formant speech synthesizer. each voice is an independent synthesizer with a glottal pulse and noise source and three parallel formant resonators.

the channel count is adjustable from 1 to 16 (default 1) in the chip's configuration.

besides being able to speak more or less intelligible phonemes, taking direct control over the formants lets you use it as a strange resonant instrument.

## phoneme banks

the chip's configuration selects a **phoneme bank**, which determines what the `10xx` values and ARPABET names map to:

- **klatt1980-en**: the English phoneme set (see the table below), indices `00`-`27`.
- **ja-mokhtari-2000** (default): extends klatt1980-en, so it contains every English phoneme at the same index, plus Japanese-tuned vowels `A I U E O` and the alveolar tap `DX` appended at `28`-`2D`. use this to sing Japanese (Japanese consonants are covered by the inherited English set).
- **ja-hecko-2026**: an alternate Japanese tuning contributed by .hecko. it has the same English base and six added symbols, with raw indices `28`-`2D` ordered `I E A O U DX`.

## usage

each voice plays one phoneme at a time. the note sets the fundamental (F0); the `10xx` effect selects the phoneme. when the cursor is on a `10xx` effect value cell, you may type ARPABET names directly (e.g. `AA`, `SH`, or `A` for the Japanese vowel). Furnace converts them to the selected bank's byte values.

voices use Furnace's standard stereo panning effect and left/right pan macros.

Klattsch instruments provide persistent defaults for transition time, voice quality, gain, formant scaling, vibrato and tremolo. pattern effects take precedence over those defaults. see the [Klattsch instrument editor](../4-instrument/klattsch.md) page for details.

some one-letter names are also prefixes (`A` and `AA`, `T` and `TH`, and so on). Furnace writes the short match immediately; if the next letter completes a longer name, it replaces the short one. to enter the Japanese `A` in consecutive cells, type `A`, move to the next cell, then type `A` again.

phoneme changes during a note glide smoothly over the transition time set by `11xx` (in ticks). diphthongs and vowels with glide targets in the selected bank move from their initial to terminal target over two transition periods.

### phoneme list (klatt1980-en, also the base of both Japanese banks)

| hex | phoneme | | hex | phoneme | | hex | phoneme | | hex | phoneme |
|----:|:--------|-|----:|:--------|-|----:|:--------|-|----:|:--------|
| 00 | IY | | 0A | AY | | 14 | N  | | 1E | HH |
| 01 | IH | | 0B | AW | | 15 | NG | | 1F | P  |
| 02 | EH | | 0C | EY | | 16 | F  | | 20 | B  |
| 03 | AE | | 0D | OW | | 17 | TH | | 21 | T  |
| 04 | AA | | 0E | OY | | 18 | S  | | 22 | D  |
| 05 | AO | | 0F | W  | | 19 | SH | | 23 | K  |
| 06 | AH | | 10 | Y  | | 1A | V  | | 24 | G  |
| 07 | UH | | 11 | R  | | 1B | DH | | 25 | CH |
| 08 | UW | | 12 | L  | | 1C | Z  | | 26 | JH |
| 09 | ER | | 13 | M  | | 1D | ZH | | 27 | _ (silence) |

with the ja-mokhtari-2000 bank, these Japanese phonemes are appended: `28` A, `29` I, `2A` U, `2B` E, `2C` O, `2D` DX (tap). with ja-hecko-2026, the same symbols use the order `28` I, `29` E, `2A` A, `2B` O, `2C` U, `2D` DX.

## effect commands

- `10xx`: **set phoneme.** see the table above.
- `11xx`: **set spectral transition time** in ticks. sticky. `00` = instant.
- `12xx`: **set F1 frequency** to `xx`×10 Hz.
- `13xx`: **set F2 frequency** to `xx`×16 Hz.
- `14xx`: **set F3 frequency** to `xx`×16 Hz.
- `15xx`: **set F1 amplitude** (`00` to `FF`).
- `16xx`: **set F2 amplitude** (`00` to `FF`).
- `17xx`: **set F3 amplitude** (`00` to `FF`).
- `18xx`: **set voicing** (`00` to `FE`). sticky. `FF` returns control to the instrument default.
- `19xx`: **set aspiration** (`00` to `FE`). sticky. `FF` returns control to the instrument default.
- `1Axx`: **set spectral tilt** as a two's-complement signed value. `00` to `7F` is neutral to positive (brighter); `80` to `FE` is negative (darker). sticky. `FF` returns control to the instrument default.
- `1Bxx`: **set glottal effort** (`00` to `FE`). sticky. `FF` returns control to the instrument default.
- `1Cxy`: **vibrato.** `x` is rate in Hz, `y` is depth in 4 Hz steps. sticky. `1C00` turns it off.
- `1Dxy`: **tremolo.** `x` is rate in Hz, `y` is depth. sticky. `1D00` turns it off.
- `1Exx`: **set gain** to `xx`/16. sticky. `00` returns to the instrument default.
- `1Fxx`: **set formant bandwidth scale** to `xx`/64. sticky. `40` is neutral; smaller values narrow the resonators into ringing filters. `00` returns to the instrument default.
- `20xx`: **set formant shift** to `xx`/64. sticky. `40` is neutral; values above shift every formant up, shrinking the apparent vocal tract. `00` returns to the instrument default.

formant frequency/amplitude changes (`12xx`-`17xx`) are one-shot nudges: the next phoneme change or note-on replaces them. on the same row as a phoneme or note-on, they apply after that event; on a stop row, they shape the burst. they are absolute frequencies and are not scaled by `20xx`. the sticky effects survive phoneme changes and note-ons until reset.

with a full voicing override, the formant effects also work as a resonant instrument: place `18FE` and drive `12xx`/`13xx` directly.

## tips

- **speech**: place a phoneme every 1-2 rows at typical tempos and keep the transition time short (`1101`-`1102`) so consonants stay crisp. spell words phonetically, not orthographically: "klattsch" is `K L AE T SH`.
- **stops** (P, B, T, D, K, G, CH, JH) work best on a row of their own. closure and burst timing follows that row's groove step. use `12xx`-`17xx` on the stop row to shape its burst, or on a later row to change the sustained sound after it.
- **singing**: use longer transitions (`1103`-`1106`) and vibrato (`1C52` is a good start). changing the note without a note off keeps the current phoneme, so a melisma is just new notes over a held vowel.
- **the `_` phoneme** (`1027`) is silence with neutral formants; use it for gaps inside a phrase without retriggering.
- **note off** fades the voice out over about 30 ms regardless of transition time.
- **choirs**: several voices stacked in the same low-mid register pile up and get muddy. give inner and lower voices some positive tilt (`1A19`-`1A2A`) to thin them out, and reduce their volume relative to the lead; the lower the voice, the more of both.
- **percussion**: unvoiced fricatives (S, SH, F) with an aspiration boost (`19B0` or so) make serviceable shaker and hi-hat textures. reset with `19FF` when done.
- **voice size**: formant shift (`20xx`) moves the formants and their bandwidths together. values above `40` sound smaller and brighter; values below `40` sound larger and darker.
- pitch slides, portamento and arpeggios all work; pitch is continuous, so slides are smooth regardless of speed.
