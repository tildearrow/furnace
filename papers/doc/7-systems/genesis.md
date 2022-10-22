# Sega Genesis/Mega Drive

a video game console that showed itself as the first true rival to Nintendo's video game market near-monopoly in the US during the '80's.

this console is powered by two sound chips: the [Yamaha YM2612](ym2612.md) and [a derivative of the SN76489](sms.md).

# effects

- `10xy`: set LFO parameters.
  - `x` toggles the LFO.
  - `y` sets its speed.
- `11xx`: set feedback of channel.
- `12xx`: set operator 1 level.
- `13xx`: set operator 2 level.
- `14xx`: set operator 3 level.
- `15xx`: set operator 4 level.
- `16xy`: set multiplier of operator.
  - `x` is the operator (1-4).
  - `y` is the mutliplier.
- `17xx`: enable PCM channel.
  - this only works on channel 6.
  - **this effect is there for compatibility reasons** - it is otherwise recommended to use Sample type instruments (which automatically enable PCM mode when used).
- `18xx`: toggle extended channel 3 mode.
  - 0 disables it and 1 enables it.
  - only in extended channel 3 chip.
- `19xx`: set attack of all operators.
- `1Axx`: set attack of operator 1.
- `1Bxx`: set attack of operator 2.
- `1Cxx`: set attack of operator 3.
- `1Dxx`: set attack of operator 4.
- `20xy`: set PSG noise mode.
  - `x` controls whether to inherit frequency from PSG channel 3.
    - 0: use one of 3 preset frequencies (C: A-2; C#: A-3; D: A-4).
    - 1: use frequency of PSG channel 3.
  - `y` controls whether to select noise or thin pulse.
    - 0: thin pulse.
    - 1: noise.
