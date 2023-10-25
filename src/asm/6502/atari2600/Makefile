
DEFAULT_TARGET: all

DASM_INC = $(DASM_HOME)/machines/atari2600/

ROMDIR = roms
INCS = $(wildcard *_*.asm)
ASMS = Player.asm MiniPlayer.asm
SYSTEMS = NTSC PAL
ROMS = $(foreach ASM, $(ASMS), $(foreach SYSTEM,$(SYSTEMS),$(ROMDIR)/$(ASM:.asm=)_$(SYSTEM).a26))

all: $(ROMDIR) $(ROMS) 

$(ROMDIR):
	mkdir -p $@
	touch $@

$(ROMS): $(ASMS) $(INCS)
	$(eval ASM := $(word 2,$(subst _, ,$(subst /, ,$@))))
	$(eval SYSTEM := $(word 2,$(subst ., ,$(subst _, ,$@))))
	$(DASM_HOME)/bin/dasm $(ASM).asm -I$(DASM_INC) -p10 -f3 -v4 -o$@ -s$(@:.a26=.sym) -l$(@:.a26=.lst) -MSYSTEM=$(SYSTEM) > $(@:.a26=.log)

.PHONY: clean
clean:
	rm $(ROMDIR)/*
