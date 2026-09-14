all: test.nes
test.nes: test.o test.ini
	wlalink -v -S test.ini test.nes
test.o: test.s nes.i chr.bin ../stream.s ../seq.bin
	wla-6502 -o test.o test.s

clean:
	rm test.nes test.o
