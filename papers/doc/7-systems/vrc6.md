# Konami VRC6

Its one of NES mapper with sound expansion, and one of two VRCs with this feature by Konami.

The chip has 2 pulse wave channel and single sawtooth channel.
volume register is 4 bit for pulse wave and 6 bit for sawtooth, but sawtooth output is corrupted when volume register value is too high. because this register is 8 bit accumulator in technically, its output is wraparoundable.

pulse wave duty cycle is 8 level, it can be ignored and it has potential for DAC at this case: volume register in this mode is DAC output and it can be PCM playback through this mode.
Furnace supports this routine for PCM playback, but it's consume a lot of CPU resource in real hardware.

# effects

- `12xx`: set duty cycle. (0 to 7)
- `17xx`: toggle PCM mode.

* All effects are affects at pulse channels only.