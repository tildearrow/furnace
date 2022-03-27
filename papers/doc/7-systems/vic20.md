# Commodore VIC-20

The Commodore VIC-20 was Commodore's major attempt at making a personal home computer, and is the percursor to the Commodore 64. The VIC-20 was also known as the VC-20 in Germany, and the VIC-1001 in Japan.

It has 4 PSG voices that has a limited but wide tuning range, and like the SN76489, the last voice is dedicated to playing pseudo-white noise. Every voice on the VIC-20 has a high-pass and low-pass filter applied to it, which is likely how Rob Yannes got the inspiration to put custom filter modes on the C64's SID.

The 3 pulse wave channels also have different octaves that they can play notes on. The first channel is the bass channel, and it can play notes from octaves 2 to octaves 4. The next is the 'mid/chord' channel, and it plays notes from octaves 3 to 5. And rather obviously, the 3rd pulse channel is typically the lead channel, can play notes from octaves 4 to 6.

## effect commands

 - `10xx` Switch waveform (Only the values 00 though 0F are unique. Everything else is a copy. For example, `1006` is the same as `10f6`.)
