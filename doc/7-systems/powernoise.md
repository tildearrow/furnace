# PowerNoise

a fantasy sound chip created by The Beesh-Spweesh! and jvsTSX for the Hexheld fantasy video game console.

its design employs linear-feedback shift registers (LFSR) for sound generation. this technology is used in many random number generators to produce noise, but it is also capable of producing other tones.

it has three noise channels and one "slope" channel capable of generating a variety of saw waves.

it outputs stereo sound with 4-bit volume control per channel and 2-bit master volume control.

refer to [its instrument type's documentation](../4-instrument/powernoise.md) for details on sound synthesis.

## effect commands

- `20xx`: **load LFSR value (low byte).**
  - on the slope channel, this sets its accumulator (from `00` to `7F`).
- `21xx`: **load LFSR value (high byte).**
- `22xx`: **write to I/O port A.**
- `23xx`: **write to I/O port B.**

## info

this chip uses the [PowerNoise](../4-instrument/powernoise.md) instrument editor.
