# Famicom Disk System

the Famicom Disk System is an expansion device for the Famicom (known as NES outside Japan), a popular console from the '80s. as its name implies, it allowed people to play games on specialized floppy disks that could be rewritten on vending machines, therefore reducing the cost of ownership and manufacturing.

it also offers an additional 6-bit, 64-byte wavetable sound channel with (somewhat limited) FM capabilities, which is what Furnace supports.

## effects

- `10xx`: **change wave.**
- `11xx`: **set modulation depth.**
- `12xy`: **set modulation speed high byte and toggle on/off.**
  - `x` is the toggle. a value of `1` turns on the modulator.
  - `y` is the speed.
- `13xx`: **set modulation speed low byte.**
- `14xx`: **set modulator position.**
- `15xx`: **set modulator wave.**
  - `xx` points to a wavetable. it should (preferably) have a height of 7 with the values mapping to:
    - 0: +0
    - 1: +1
    - 2: +2
    - 3: +3
    - 4: reset
    - 5: -3
    - 6: -2
    - 7: -1
  - **do not use this effect.** it only exists for compatibility reasons
- `16xy`: **enable automatic modulation speed mode.**
  - in this mode the modulation speed is set to the channel's notes, multiplied by a fraction.
  - `x` is the numerator.
  - `y` is the denominator.
  - if `x` or `y` are 0 this will disable automatic modulation speed mode.

## info

this chip uses the [FDS](../4-instrument/fds.md) instrument editor.

## chip config

the following options are available in the Chip Manager window:

- **Clock rate**: sets the rate at which the chip will run.

## frequency modulation (FM)

the FDS features a method of FM that changes the frequency of the playing note using a short looping table of 32 values. the modulation table is similar to DPCM in that it isn't a series of direct values, but deltas that determine the amount of change from the previous value.
- deltas range from -3 to 3. in the instrument editor, -4 resets the modulation value to 0, meaning the note plays at its base frequency.
- modulation values are 7-bit, ranging from -64 to 63; moving past those bounds will overflow to the other side (63 + 1 = -64).