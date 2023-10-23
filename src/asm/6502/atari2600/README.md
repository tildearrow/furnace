# Atari 2600 export format

ROM export format for the Atari 2600

# prerequisites

This ROM export is designed to be compiled with dasm.

# process

You can export track data from the furnace command line as follows

```
furnace --romout <target directory> <furnace file>
```

To compile:

- copy Makefile, Player.asm and Player_core.asm into the target directory
- You will need to set the environment variable `DASM_HOME` with the location of a dasm install.
- Type `make` to build.

Detailed errors will be available in `roms/Player_*.log`

To run with Stella, type:

```
stella roms/Player_NTSC.a26
```

# sequence format

- Songs contain pattern sequences.
- Pattern sequences contain notes.
- Notes are obtained by a compressed register dump from Furnace

Notes are encoded as follows:

```
  fffff010 wwwwvvvv           frequency + waveform + volume, duration 1
  fffff100 wwwwvvvv           " " ", duration 2
  fffff110 dddddddd wwwwvvvv  " " ", duration d
  xxxx0001                    volume = x >> 4, duration 1 
  xxxx1001                    volume = x >> 4, duration 2
  xxxx0101                    wave = x >> 4, duration 1
  xxxx1101                    wave = x >> 4, duration 2
  xxxxx011                    frequency = x >> 3, duration 1
  xxxxx111                    frequency = x >> 3, duration 2
  00000000                    stop
```


