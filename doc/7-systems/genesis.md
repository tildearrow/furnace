# Sega Genesis/Mega Drive

a video game console that showed itself as the first true rival to Nintendo's video game market near-monopoly in the US during the '80s.

this console is powered by two sound chips: the [Yamaha YM2612](ym2612.md) and [a derivative of the SN76489](sms.md).

## effects

- `10xy`: **set LFO parameters.**
  - `x` toggles the LFO.
  - `y` sets its speed.
- `11xx`: **set feedback of channel.**
- `12xx`: **set operator 1 level.**
- `13xx`: **set operator 2 level.**
- `14xx`: **set operator 3 level.**
- `15xx`: **set operator 4 level.**
- `16xy`: **set multiplier of operator.**
  - `x` is the operator (1-4).
  - `y` is the new MULT value..
- `17xx`: **enable PCM channel.**
  - this only works on channel 6.
  - _this effect is here for compatibility reasons!_ it is otherwise recommended to use Sample type instruments (which automatically enable PCM mode when used).
- `18xx`: **toggle extended channel 3 mode.**
  - `0` disables it and `1` enables it.
  - only in extended channel 3 chip.
- `19xx`: **set attack of all operators.**
- `1Axx`: **set attack of operator 1.**
- `1Bxx`: **set attack of operator 2.**
- `1Cxx`: **set attack of operator 3.**
- `1Dxx`: **set attack of operator 4.**
- `20xy`: **set PSG noise mode.**
  - `x` controls whether to inherit frequency from PSG channel 3.
    - `0`: use one of 3 preset frequencies (`C`: A-2; `C#`: A-3; `D`: A-4).
    - `1`: use frequency of PSG channel 3.
  - `y` controls whether to select noise or thin pulse.
    - `0`: thin pulse.
    - `1`: noise.



## system modes

## extended channel 3

in ExtCh mode, channel 3 is split into one column for each of its four operators. feedback and LFO levels are shared. the frequency of each operator may be controlled independently with notes and effects. this can be used for more polyphony or more complex sounds.

all four operators are still combined according to the algorithm in use. for example, algorithm 7 acts as four independent sine waves. algorithm 4 acts as two independent 2-op sounds. even with algorithm 0, placing a note in any operator triggers that operator alone.

## CSM

CSM is short for "Composite Sinusoidal Modeling". CSM works by sending key-on and key-off commands to channel 3 at a specific frequency, controlled by the added "CSM Timer" channel. this can be used to create vocal formants (speech synthesis!) or other complex effects.

CSM is beyond the scope of this documentation. for more information, see this [brief SSG-EG and CSM video tutorial](https://www.youtube.com/watch?v=IKOR0TUlnWU).

## DualPCM

[info here.](ym2612.md)

## Sega CD

this isn't a mode so much as a chip configuration. it adds the [Ricoh RF5C68](ricoh.md) found in the Sega CD add-on, providing 8 channels of PCM.

## chip config

see [YM2612](ym2612.md) and [SN76489](sms.md) for respective configuration.
