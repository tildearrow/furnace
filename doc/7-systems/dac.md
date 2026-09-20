# Generic PCM DAC

up to 128 sample channels with freely selectable rate, mono/stereo, and bit depth settings.

with it, you can emulate PCM DACs found in Williams arcade boards, Sound Blasters, MSX TurboR, Atari STe, NEC PC-9801-86, among others.

## effects

none yet.

## info

this chip uses the [Generic Sample](../4-instrument/sample.md) instrument editor.

## chip config

the following options are available in the Chip Manager window:

- **Output rate**: sets the output sample rate of the DAC.
- **Output bit depth**: sets the bit depth of the DAC. higher values provide more resolution.
- **Maximum volume**: sets the max value in the volume column.
- **Stereo**: when enabled, you may use panning effects.
- **Interpolation**: "softens" samples played back at lower rates.
- **Channels**: sets the number of channels, from 1 to 128.
