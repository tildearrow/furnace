
DEFAULT_TARGET: all

ROMDIR = roms
INCS = $(wildcard _*.asm)
ASMS = Player.asm
SYSTEMS = NTSC
ROMS = $(foreach ASM, $(ASMS), $(foreach SYSTEM,$(SYSTEMS),$(ROMDIR)/$(ASM:.asm=)_$(SYSTEM).a26))
MACHINE_INC = /opt/homebrew/Cellar/dasm/2.20.14.1/machines/atari2600/

all: $(ROMDIR) $(ROMS) 

$(ROMDIR):
	mkdir -p $@
	touch $@

$(ROMS): $(ASMS) $(INCS)
	$(eval ASM := $(word 2,$(subst _, ,$(subst /, ,$@))))
	$(eval SYSTEM := $(word 2,$(subst ., ,$(subst _, ,$@))))
	dasm $(ASM).asm -I$(MACHINE_INC) -p10 -f3 -v4 -o$@ -s$(@:.a26=.sym) -l$(@:.a26=.lst) -MSYSTEM=$(SYSTEM) > $(@:.a26=.log)

.PHONY: clean
clean:
	rm $(ROMDIR)/*
