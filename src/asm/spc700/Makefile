all: furnace.spc
furnace.spc: spc.o spc.ini
	wlalink -v -S spc.ini furnace.spc
spc.o: spc.s snes.s spc.i stream.s seq.bin sample.bin songData.s songInfo.s
	wla-spc700 -o spc.o spc.s

clean:
	rm furnace.spc spc.o
