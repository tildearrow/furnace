# Atari 2600 export format

ROM export format for the Atari 2600

# prerequisites

This ROM export is designed to be compiled with dasm.

# process

Use the `--romout` command line option to export track data from furnace:

```
furnace --romout <target directory> <furnace file>
```

Once this is done, copy the contents of `src/asm/6502/atari2600` into the target directory

You will need to set the environment variable `DASM_HOME` with the location of a dasm install.

Type `make` to build.

To run with Stella, type:

```
stella roms/Player_NTSC.a26
```

# sequence format

Songs contain pattern sequences.
Pattern sequences contain notes distributed in two channels.
Notes are obtained by a compressed register dump from Furnace.
The note "waveform" is delta-encoded in order to obtain further data compression. 

Waveform format:

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


