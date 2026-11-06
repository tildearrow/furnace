
noinst_PROGRAMS = midiprobe midiout qmidiin cmidiin sysextest sysexchunked midiclock_in midiclock_out	\
	apinames testcapi cgetmessage sysexinterleaved

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

sysexchunked_SOURCES = sysexchunked.cpp
sysexchunked_LDADD = $(top_builddir)/librtmidi.la

midiclock_in_SOURCES = midiclock.cpp
midiclock_in_LDADD = $(top_builddir)/librtmidi.la

midiclock_out_SOURCES = midiclock.cpp
midiclock_out_LDADD = $(top_builddir)/librtmidi.la

apinames_SOURCES = apinames.cpp
apinames_LDADD = $(top_builddir)/librtmidi.la

testcapi_SOURCES = testcapi.c
testcapi_LDADD = $(top_builddir)/librtmidi.la
# testcapi.c is C, but it links against a C++ library.  With a shared
# librtmidi the C++ runtime is pulled in through the library's own
# dependencies; a static build has no such link, so the C linker fails on
# every C++ symbol it needs.  Naming a C++ source here makes Automake choose
# the C++ linker in both cases.  The file is never compiled and need not exist.
nodist_EXTRA_testcapi_SOURCES = dummy.cxx

# Exits 77 (skipped) where there is no MIDI loopback; see cgetmessage.c.
cgetmessage_SOURCES = cgetmessage.c
cgetmessage_LDADD = $(top_builddir)/librtmidi.la
nodist_EXTRA_cgetmessage_SOURCES = dummy.cxx

# Exits 77 (skipped) where there is no MIDI loopback; see sysexinterleaved.cpp.
sysexinterleaved_SOURCES = sysexinterleaved.cpp
sysexinterleaved_LDADD = $(top_builddir)/librtmidi.la

EXTRA_DIST = AlsaHotplug.cmake RunAlsaHotplug.cmake README-hotplug.md \
	RtMidi.sln Directory.Build.props \
	apinames.vcxproj apinames.vcxproj.filters \
	cmidiin.vcxproj cmidiin.vcxproj.filters \
	midiclock.vcxproj midiclock.vcxproj.filters \
	midiout.vcxproj midiout.vcxproj.filters \
	midiprobe.vcxproj midiprobe.vcxproj.filters \
	qmidiin.vcxproj qmidiin.vcxproj.filters \
	sysexchunked.vcxproj sysexchunked.vcxproj.filters \
	sysexinterleaved.vcxproj sysexinterleaved.vcxproj.filters \
	sysextest.vcxproj sysextest.vcxproj.filters \
	testcapi.vcxproj testcapi.vcxproj.filters

TESTS = apinames cgetmessage sysexinterleaved

if HAVE_ALSA
noinst_PROGRAMS += alsahotplug
alsahotplug_SOURCES = alsahotplug.cpp $(top_srcdir)/RtMidi.cpp
alsahotplug_CPPFLAGS = $(AM_CPPFLAGS) $(CPPFLAGS) -URTMIDI_API_JACK -U__UNIX_JACK__
alsahotplug_LDFLAGS = -Wl,--wrap=snd_seq_client_info_get_card -Wl,--wrap=realpath \
  -Wl,--wrap=__realpath_chk -Wl,--wrap=pipe2 -Wl,--wrap=pthread_create \
  -Wl,--wrap=snd_seq_open -Wl,--wrap=snd_seq_event_input -Wl,--wrap=snd_seq_subscribe_port \
  -Wl,--wrap=snd_seq_poll_descriptors -Wl,--wrap=snd_seq_query_next_client
TESTS += alsahotplug
if NATIVE_BUILD
# Match CMake: building ALSA also runs its noninteractive regression test.
all-local: alsahotplug
	$(MAKE) $(AM_MAKEFLAGS) check-TESTS TESTS=alsahotplug
endif
endif
