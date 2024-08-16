# Commodore Amiga

a computer with a desktop OS, lifelike graphics and 4 channels of PCM sound in 1985? no way!

in this very computer music trackers were born...

imported MOD files use this chip, and will set A-4 tuning to 436.

## amplitude/period modulation

Amiga has support for (rather primitive) amplitude and period (frequency) modulation.
however, nobody has used this feature as it is rather useless, not well-documented and works in a complicated way.

Amiga sample playback is done by two chips: Paula (the one that you probably know) and Agnus (the one that actually feeds Paula with samples).
Agnus has several DMA (direct memory access) units which read from chip memory independent of the CPU. four of these DMA units are used for samples.

when DMA is enabled, Paula requests sample data from Agnus, and then plays these samples back.
there's a catch though. since the data bus is 16-bit, Paula requests **two** 8-bit samples at once! this explains why:
- the sample length registers are in words rather than bytes (thereby allowing samples up to 131070 in length)
- the maximum playback rate (31250Hz PAL; ~31469Hz NTSC) is two times the HBlank rate (Agnus fetches samples on HBlank, around 15625Hz on PAL or ~15734Hz on NTSC)

during normal sample playback, the first sample is output and then the second. afterwards, two more samples are fetched, and so on.

now, when amplitude or period modulation are enabled, things work differently.
the channel is silenced, and the two 8-bit samples are **treated as a big-endian 16-bit number**, which is then written to the next channel's volume or period.

in the case of amplitude modulation, only the second sample is significant because the volume register uses 7 bits (to represent 0 to 64 (65 to 127 are treated as 64)) and the other bits are ignored.

in the case of period modulation, both samples are significant. the first sample is the upper byte, and the second is the lower byte.

## effects

- `10xx`: **toggle low-pass filter.** `0` turns it off and `1` turns it on.
- `11xx`: **toggle amplitude modulation with the next channel.**
  - does not work on the last channel.
- `12xx`: **toggle period (frequency) modulation with the next channel.**
  - does not work on the last channel.
- `13xx`: **change wave.**
  - only works when "Mode" is set to "Wavetable" in the instrument.

## info

this chip uses the [Generic Sample](../4-instrument/sample.md) instrument editor.

- the maximum rate for sample playback is technically 31469Hz but anything higher than 28867Hz will sound glitchy on hardware.
- sample lengths and loop will be set to an even number.
- samples can't be longer than 131070.

## chip config

the following options are available in the Chip Manager window:

- **Stereo separation**: sets the amount of left/right separation.
- **Model**: allows you to change the chipset.
  - Amiga 500 (OCS): has a low-pass filter on top of the user-selectable filter.
  - Amiga 1200 (AGA): doesn't have the aforementioned low-pass filter.
- **Chip memory**: more chip memory means more space for samples.
- **PAL**: run the chip at PAL clock (3.54MHz) instead of NTSC (3.58MHz).
- **Bypass frequency limits**: when enabled, the ~31KHz frequency limit is disabled, allowing you to play higher notes.
