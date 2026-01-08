targets = \
  feedseek \
  mpg123_to_out123 \
  mpg123_to_wav_replaced_io \
  mpg123_to_wav_reader64 \
  scan \
  id3dump \
  mpglib \
  dump_seekindex \
  extract_frames

all: $(targets)

# On largefile-aware systems you might want to use these instead:
#MPG123_CFLAGS  := $(shell pkg-config --cflags libmpg123_64)
#MPG123_LDFLAGS := $(shell pkg-config --libs   libmpg123_64)
# This works on sane setups where off_t is off_t, and just that.
ifeq ($(MPG123_PREFIX),)
  MPG123_CFLAGS  := $(shell pkg-config --cflags libmpg123)
  MPG123_LDFLAGS := $(shell pkg-config --libs   libmpg123)
  OUT123_CFLAGS  := $(shell pkg-config --cflags libout123)
  OUT123_LDFLAGS := $(shell pkg-config --libs   libout123)
else # Yeah, that's with GNU/Linux in mind, at least GNU ld ...
  MPG123_CFLAGS  := -I$(MPG123_PREFIX)/include
  MPG123_LDFLAGS := -L$(MPG123_PREFIX)/lib -Wl,-rpath $(MPG123_PREFIX)/lib -lmpg123
  OUT123_CFLAGS  := -I$(MPG123_PREFIX)/include
  OUT123_LDFLAGS := -L$(MPG123_PREFIX)/lib -Wl,-rpath $(MPG123_PREFIX)/lib -lout123
endif

# Define to any C99 compiler.
CC = cc -std=c99
CFLAGS = -O

# Oder of libs not that important here...
compile = $(CC) $(CPPFLAGS) $(CFLAGS) $(MPG123_CFLAGS)
linkflags = $(MPG123_LDFLAGS) $(LDFLAGS)

mpg123_to_out123: mpg123_to_out123.c
	$(compile) -o $@ $< $(OUT123_CFLAGS) $(OUT123_LDFLAGS) $(linkflags)

mpg123_to_wav_replaced_io: mpg123_to_wav_replaced_io.c
	$(compile) -o $@ $< $(OUT123_CFLAGS) $(OUT123_LDFLAGS) $(linkflags)

mpg123_to_wav_reader64: mpg123_to_wav_reader64.c
	$(compile) -o $@ $< $(OUT123_CFLAGS) $(OUT123_LDFLAGS) $(linkflags)

feedseek: feedseek.c
	$(compile) -o feedseek feedseek.c $(OUT123_CFLAGS) $(OUT123_LDFLAGS) $(linkflags)

scan: scan.c
	$(compile) -o scan scan.c $(linkflags)

id3dump:id3dump.c
	$(compile) -o id3dump id3dump.c $(linkflags)

dump_seekindex: dump_seekindex.c
	$(compile) -o dump_seekindex dump_seekindex.c $(linkflags)

mpglib: mpglib.c
	$(compile) -o mpglib mpglib.c $(linkflags)

extract_frames: extract_frames.c
	$(compile) -o $@ $< $(linkflags)

clean:
	rm -vf $(targets)
