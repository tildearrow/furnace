all: player

player: player.s sample.bin seq.bin wave.bin
	vasmm68k_mot -Fhunkexe -kick1hunks -nosym -o player player.s
