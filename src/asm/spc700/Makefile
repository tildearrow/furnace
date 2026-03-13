all: furnace.spc
furnace.spc: spc.o spc.ini
	wlalink -v -S spc.ini furnace.spc
spc.o: spc.s spc.i stream.s
	wla-spc700 -o spc.o spc.s

clean:
	rm furnace.spc spc.o
