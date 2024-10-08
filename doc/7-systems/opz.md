# Yamaha OPZ (YM2414)

**disclaimer: despite the name, this has nothing to do with teenage engineering's OP-Z synth!**

this is the YM2151's little-known successor, used in the Yamaha TX81Z and a few other Yamaha synthesizers. oh, and the Korg Z3 too.

it adds these features on top of the YM2151:
- 8 waveforms (but they're different from the OPL ones)
- per-channel (possibly) linear volume control separate from TL
- increased multiplier precision (in 1/16ths)
- 4-step envelope generator shift (minimum TL)
- another LFO
- no per-operator key on/off
- fixed frequency mode per operator (kind of like OPN family's extended channel mode but with a bit less precision and for all 8 channels)
- "reverb" effect (actually extends release)

unlike the YM2151, this chip is officially undocumented. very few efforts have been made to study the chip and document it...
therefore emulation of this chip in Furnace is incomplete and uncertain.

no plans have been made for TX81Z MIDI passthrough, because:
- Furnace works with register writes rather than MIDI commands
- the MIDI protocol is slow (would not be enough).
- the TX81Z is very slow to process a note on/off or parameter change event.
- the TL range has been reduced to 0-99, but the chip goes from 0-127.

## effects

- `10xx`: **set noise frequency of channel 8 operator 4.** `00` disables noise while `01` to `20` enable it.
- `11xx`: **set feedback of channel.**
- `12xx`: **set operator 1 level.**
- `13xx`: **set operator 2 level.**
- `14xx`: **set operator 3 level.**
- `15xx`: **set operator 4 level.**
- `16xy`: **set multiplier of operator.**
  - `x` is the operator (1-4).
  - `y` is the new MULT value..
- `17xx`: **set LFO speed.**
- `18xx`: **set LFO waveform.** `xx` may be one of the following:
  - `00`: saw
  - `01`: square
  - `02`: triangle
  - `03`: noise
- `19xx`: **set attack of all operators.**
- `1Axx`: **set attack of operator 1.**
- `1Bxx`: **set attack of operator 2.**
- `1Cxx`: **set attack of operator 3.**
- `1Dxx`: **set attack of operator 4.**
- `1Exx`: **set LFO AM depth.**
- `1Fxx`: **set LFO PM depth.**
- `24xx`: **set LFO 2 speed.**
- `25xx`: **set LFO 2 waveform.** `xx` may be one of the following:
  - `00`: saw
  - `01`: square
  - `02`: triangle
  - `03`: noise
- `26xx`: **set LFO 2 AM depth.**
- `27xx`: **set LFO 2 PM depth.**
- `28xy`: **set reverb of operator.**
  - `x` is the operator (1-4). a value of `0` means "all operators".
  - `y` is the value.
- `2Axy`: **set waveform of operator.**
  - `x` is the operator (1-4). a value of `0` means "all operators".
  - `y` is the value.
- `2Bxy`: **set EG shift of operator.**
  - `x` is the operator (1-4). a value of `0` means "all operators".
  - `y` is the value.
- `2Cxy`: **set fine multiplier of operator.**
  - `x` is the operator (1-4). a value of `0` means "all operators".
  - `y` is the value.
- `2Fxx`: **enable envelope hard reset.**
  - this works by inserting a quick release and tiny delay before a new note.
- `3xyy`: **set fixed frequency of operator 1/2.**
  - `x` is the block (`0-7` for operator 1; `8-F` for operator 2).
  - `y` is the frequency. fixed frequency mode will be disabled if this is less than 8.
  - the actual frequency is: `y*(2^x)`.
- `4xyy`: **set fixed frequency of operator 3/4.**
  - `x` is the block (`0-7` for operator 3; `8-F` for operator 4).
  - `y` is the frequency. fixed frequency mode will be disabled if this is less than 8.
  - the actual frequency is: `y*(2^x)`.
- `50xy`: **set AM of operator.**
  - `x` is the operator (1-4). a value of `0` means "all operators".
  - `y` determines whether AM is on.
- `51xy`: **set SL of operator.**
  - `x` is the operator (1-4). a value of `0` means "all operators".
  - `y` is the value.
- `52xy`: **set RR of operator.**
  - `x` is the operator (1-4). a value of `0` means "all operators".
  - `y` is the value.
- `53xy`: **set DT of operator.**
  - `x` is the operator (1-4). a value of `0` means "all operators".
  - `y` is the value:
    - `0`: +0
    - `1`: +1
    - `2`: +2
    - `3`: +3
    - `4`: -0
    - `5`: -3
    - `6`: -2
    - `7`: -1
- `54xy`: **set RS of operator.**
  - `x` is the operator (1-4). a value of `0` means "all operators".
  - `y` is the value.
- `55xy`: **set DT2 of operator.**
  - `x` is the operator (1-4). a value of `0` means "all operators".
  - `y` is the value.
- `56xx`: **set DR of all operators.**
- `57xx`: **set DR of operator 1.**
- `58xx`: **set DR of operator 2.**
- `59xx`: **set DR of operator 3.**
- `5Axx`: **set DR of operator 4.**
- `5Bxx`: **set D2R/SR of all operators.**
- `5Cxx`: **set D2R/SR of operator 1.**
- `5Dxx`: **set D2R/SR of operator 2.**
- `5Exx`: **set D2R/SR of operator 3.**
- `5Fxx`: **set D2R/SR of operator 4.**

## info

this chip uses the [FM (OPZ)](../4-instrument/fm-opz.md) instrument editor.

## chip config

the following options are available in the Chip Manager window:

- **Pseudo-PAL**: run the chip on a PAL clock. such a configuration has never been used in hardware.
- **Broken pitch macro/slides**: due to an oversight, pitch slides were twice as fast in older versions of Furnace. this option exists for compatibility.
