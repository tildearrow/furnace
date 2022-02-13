SUBDIRS = . tests
if MAKE_DOC
SUBDIRS += doc
endif

lib_LTLIBRARIES = %D%/librtmidi.la
%C%_librtmidi_la_CXXFLAGS = -DRTMIDI_EXPORT
%C%_librtmidi_la_LDFLAGS = -no-undefined -export-dynamic -version-info @SO_VERSION@
%C%_librtmidi_la_SOURCES = \
  %D%/RtMidi.cpp \
  %D%/rtmidi_c.cpp

rtmidi_incdir = $(includedir)/rtmidi
rtmidi_inc_HEADERS = \
  %D%/RtMidi.h \
  %D%/rtmidi_c.h

pkgconfigdatadir = $(libdir)/pkgconfig
pkgconfigdata_DATA = rtmidi.pc

EXTRA_DIST = autogen.sh README.md msw rtmidi-config.in contrib cmake CMakeLists.txt
