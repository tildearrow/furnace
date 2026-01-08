## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_builddir)/include

lib_LTLIBRARIES = libogg.la

libogg_la_SOURCES = framing.c bitwise.c crctable.h
libogg_la_LDFLAGS = -no-undefined -version-info @LIB_CURRENT@:@LIB_REVISION@:@LIB_AGE@

# build and run the self tests on 'make check'

noinst_PROGRAMS = test_bitwise test_framing

test_bitwise_SOURCES = bitwise.c
test_bitwise_CFLAGS = -D_V_SELFTEST

test_framing_SOURCES = framing.c crctable.h
test_framing_CFLAGS = -D_V_SELFTEST

TEST_ENV = @TEST_ENV@
LOG_COMPILER=$(LIBTOOL) --mode=execute $(VALGRIND) $(TEST_ENV)

TESTS = $(noinst_PROGRAMS)
check_PROGRAMS = $(noinst_PROGRAMS)

debug:
	$(MAKE) all CFLAGS="@DEBUG@"

profile:
	$(MAKE) all CFLAGS="@PROFILE@"
