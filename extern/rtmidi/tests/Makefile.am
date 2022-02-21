
noinst_PROGRAMS = midiprobe midiout qmidiin cmidiin sysextest midiclock_in midiclock_out	\
	apinames testcapi

AM_CXXFLAGS = -Wall -I$(top_srcdir)
AM_CFLAGS = -Wall -I$(top_srcdir)

midiprobe_SOURCES = midiprobe.cpp
midiprobe_LDADD = $(top_builddir)/librtmidi.la

midiout_SOURCES = midiout.cpp
midiout_LDADD = $(top_builddir)/librtmidi.la

qmidiin_SOURCES = qmidiin.cpp
qmidiin_LDADD = $(top_builddir)/librtmidi.la

cmidiin_SOURCES = cmidiin.cpp
cmidiin_LDADD = $(top_builddir)/librtmidi.la

sysextest_SOURCES = sysextest.cpp
sysextest_LDADD = $(top_builddir)/librtmidi.la

midiclock_in_SOURCES = midiclock.cpp
midiclock_in_LDADD = $(top_builddir)/librtmidi.la

midiclock_out_SOURCES = midiclock.cpp
midiclock_out_LDADD = $(top_builddir)/librtmidi.la

apinames_SOURCES = apinames.cpp
apinames_LDADD = $(top_builddir)/librtmidi.la

testcapi_SOURCES = testcapi.c
testcapi_LDADD = $(top_builddir)/librtmidi.la

EXTRA_DIST = cmidiin.dsp midiout.dsp midiprobe.dsp qmidiin.dsp	\
	sysextest.dsp RtMidi.dsw

TESTS = apinames
