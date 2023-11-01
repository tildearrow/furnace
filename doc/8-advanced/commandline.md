# command line

to start Furnace at the command line, use the following command (may vary by operating system):

`furnace [params] [filename]`

the following parameters may be used:

- `-help`: display help.
- `-audio <engine>`: set audio engine to one of the following:
  - `jack`: JACK Audio Connection Kit. only on Linux.
  - `sdl`: SDL. default.
  - `portaudio`: PortAudio.
- `-output <filename>`: export audio to `filename`.
- `-vgmout <filename>`: output .vgm data to `filename`.
- `-direct`: set VGM export direct stream mode.
- `-zsmout <filename>`: output .zsm data for Commander X16 Zsound.
- `-cmdout <filename>`: output command stream.
- `-binary`: set command stream output format to binary.
- `-loglevel <level>`: set the logging level to one of the following:
  - `error`: critical errors only.
  - `warning`: errors and warnings.
  - `info`: errors, warnings, and useful information.
  - `debug`: all of the above and information useful for debugging.
  - `trace`: most verbose. includes details of inner workings. default.
- `-view <type>`: set visualization of data to one of the following:
  - `pattern`: order and pattern.
  - `commands`: commands.
  - `nothing`: no visualization. default.
- `-info`: get info about a song.
- `-console`: enable console mode.
- `-loops <count>`: set number of loops. `-1` means loop forever.
- `-subsong <number>`: set sub-song.
- `-outmode one|persys|perchan`: set file output mode: all in one file, one file per chip, or one file per channel. default is `one`.
- `-safemode`: enable safe mode (software rendering without audio).
- `-safeaudio`: enable safe mode (software rendering with audio).
- `-benchmark render|seek`: run performance test.
- `-version`: version information.
- `-warranty`: view warranty disclaimer.
