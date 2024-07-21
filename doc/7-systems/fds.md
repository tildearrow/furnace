# Famicom Disk System

the Famicom Disk System is an expansion device for the Famicom (known as NES outside Japan), a popular console from the '80s.
as it name implies, it allowed people to play games on specialized floppy disks that could be rewritten on vending machines, therefore reducing the cost of ownership and manufacturing.

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

## info

this chip uses the [FDS](../4-instrument/fds.md) instrument editor.

## chip config

the following options are available in the Chip Manager window:

- **Clock rate**: sets the rate at which the chip will run.
