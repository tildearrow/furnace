# Yamaha YMZ280B (PCMD8)

8-channel PCM/ADPCM sample-based sound chip designed for use with arcade machines. it lived throughout mid to late 90s.

it has 16-level stereo panning, up to 16-bit PCM and up to 16MB of external PCM data. 4-bit ADPCM has a sample playback rate limit of 44.1 kHz with a tuning resolution of 256 Hz, while 8-bit and 16-bit PCM can go up to 88.2kHz with a tuning resolution of 512 Hz.

## effects

none so far.

## info

this chip uses the [YMZ280B](../4-instrument/ymz280b.md) instrument editor.

4-bit ADPCM format can be selected in the sample editor as "YMZ/YMU ADPCM". for this format, loop start and end points must align to multiples of 2 samples, and samples will be padded to a multiple of 2 samples.

PCM samples have a maximum playback rate of the chip's clock divided by 192.

## chip config

the following options are available in the Chip Manager window:

- **Clock rate**: sets the rate at which the chip will run.
