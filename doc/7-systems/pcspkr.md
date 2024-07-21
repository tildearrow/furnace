# PC Speaker

40 years of one square beep - and still going! single channel, no volume control...

## real output

so far this is the only chip in Furnace which has a real hardware output option.
to enable it, select file > configure chip... > PC Speaker > Use system beeper.

be noted that this will only work on Linux as Windows does not provide any user-space APIs to address the PC speaker directly!

you may configure the output method by going in Settings > Emulation > PC Speaker strategy:

- `evdev SND_TONE`: uses input events to control the beeper.
  - requires write permission to `/dev/input/by-path/platform-pcspkr-event-spkr`.
  - is not 100% frequency-accurate as `SND_TONE` demands frequencies, but Furnace uses raw timer periods...
- `KIOCSOUND on /dev/tty1`: sends the `KIOCSOUND` ioctl to control the beeper.
  - may require running Furnace as root.
- `/dev/port`: writes to `/dev/port` to control the beeper.
  - requires read/write permission to `/dev/port`.
- `KIOCSOUND on standard output`: sends the `KIOCSOUND` ioctl to control the beeper.
  - requires running Furnace on a TTY.
- `outb()`: uses the low-level kernel port API to control the beeper.
  - requires running Furnace as root, or granting it `CAP_SYS_RAWIO` to the Furnace executable: `sudo setcap cap_sys_rawio=ep ./furnace`.

real hardware output only works on BIOS/UEFI (non-Mac) x86-based machines! attempting to do this under any other device **will not work**, or may even brick the device (if using `/dev/port` or `outb()`)!
oh, and of course you also need the beeper to be present in your machine. some laptops connect the beeper output to the built-in speakers (or the audio output jack), and some other don't do this at all.

## effects

ha! effects...

## info

this chip uses the [Beeper](../4-instrument/beeper.md) instrument editor.

## chip config

the following options are available in the Chip Manager window:

- **Clock rate**: sets the rate at which the chip will run.
- **Speaker type**: select which speaker to use:
  - **Unfiltered**: raw square wave.
  - **Cone**: filter it to simulate the sound of a cone speaker.
  - **Piezo**: simulate the tiny speaker present in most PCs from the 2000s.
  - **Use system beeper**: use the actual PC speaker in your machine for output. only works on Linux!
- **Reset phase on frequency change**: reset phase every time the frequency changes. many modern motherboards tend to do this.
