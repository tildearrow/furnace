all: furnace.spc
furnace.spc: spc.o spc.ini
	wlalink -v -S spc.ini furnace.spc
spc.o: spc.s snes.s spc.i stream.s sample.bin songData.s songInfo.s songSpeed.s
	wla-spc700 -D DIV_LINEAR_FREQ -o spc.o spc.s

clean:
	rm furnace.spc spc.o
