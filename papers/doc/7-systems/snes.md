# Super NES

The successor to NES to compete with Genesis. Now packing with superior graphics and sample-based audio. Also known as Super Famicom.

Its audio subsystem, developed by Sony, features the DSP chip, SPC700 microcontroller and 64KB of dedicated SRAM used by both. This whole system itself is pretty much a separate computer that the main CPU needs to upload its program and samples to.

The DSP chip can

Furnace communicates with the DSP directly and provide a full 64KB memory. This memory might be reduced excessively on ROM export to make up for playback engine and pattern data.

# effects

Note: this chip has a signed left/right level. Which can be used for inverted (surround) stereo. A signed 8-bit value means 80 - FF = -128 - -1. Other values work normally. A value of -128 is not recommended as it could cause overflows.

- `10xx`: Set echo feedback level. This effect will apply to all channels.
- `11xx`: Set echo left level (signed 8-bit). This effect will apply to all channels.
- `12xx`: Set echo right level (signed 8-bit). This effect will apply to all channels.
- `13xx`: Set the length of the echo delay buffer. This will also affect the size of the sample RAM!
