# Process this file with autoconf to produce a configure script.
AC_INIT(RtMidi, 6.0.0, gary.scavone@mcgill.ca, rtmidi)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_SRCDIR(RtMidi.cpp)
AC_CONFIG_FILES([rtmidi-config rtmidi.pc Makefile tests/Makefile doc/Makefile doc/doxygen/Doxyfile])
AM_INIT_AUTOMAKE([1.14 -Wall -Werror foreign subdir-objects])

# libtool version: current:revision:age
#
# If the library source code has changed at all since the last update, then
# increment revision (`c:r:a' becomes `c:r+1:a').
#
# If any interfaces have been added, removed, or changed since the last update,
# increment current, and set revision to 0.
#
# If any interfaces have been added since the last public release, then
# increment age.
#
# If any interfaces have been removed since the last public release, then set
# age to 0.
m4_define([lt_current], 7)
m4_define([lt_revision], 0)
m4_define([lt_age], 0)

m4_define([lt_version_info], [lt_current:lt_revision:lt_age])
m4_define([lt_current_minus_age], [m4_eval(lt_current - lt_age)])

SO_VERSION=lt_version_info
AC_SUBST(SO_VERSION)
AC_SUBST(LIBS)
AC_SUBST(api)
AC_SUBST(req)
AC_SUBST(req_libs)

api=""
req=""
req_libs=""

# Fill GXX with something before test.
GXX="no"

# if the user did not provide any CXXFLAGS, we can override them
AS_IF([test "x$CXXFLAGS" = "x" ], [override_cxx=yes], [override_cxx=no])
AS_IF([test "x$CFLAGS" = "x" ], [override_c=yes], [override_c=no])

# Check version number coherency between RtMidi.h and configure.ac
AC_MSG_CHECKING([that version numbers are coherent])
RTMIDI_VERSION_MAJOR=`sed -n 's/#define RTMIDI_VERSION_MAJOR *\([0-9]*\)/\1/p' $srcdir/RtMidi.h`
RTMIDI_VERSION_MINOR=`sed -n 's/#define RTMIDI_VERSION_MINOR *\([0-9]*\)/\1/p' $srcdir/RtMidi.h`
RTMIDI_VERSION_PATCH=`sed -n 's/#define RTMIDI_VERSION_PATCH *\([0-9]*\)/\1/p' $srcdir/RtMidi.h`
RTMIDI_VERSION_BETA=`sed -n 's/#define RTMIDI_VERSION_BETA *\([0-9]*\)/\1/p' $srcdir/RtMidi.h`
AS_IF(
   [test '('   "0$RTMIDI_VERSION_BETA" -le 0 \
            -o "x$RTMIDI_VERSION_MAJOR.$RTMIDI_VERSION_MINOR.${RTMIDI_VERSION_PATCH}beta${RTMIDI_VERSION_BETA}" != "x$PACKAGE_VERSION" ')' \
      -a '('   "0$RTMIDI_VERSION_BETA" -ne 0\
            -o "x$RTMIDI_VERSION_MAJOR.$RTMIDI_VERSION_MINOR.${RTMIDI_VERSION_PATCH}" != "x$PACKAGE_VERSION" ')' ],
   [AC_MSG_FAILURE([testing RTMIDI_VERSION==PACKAGE_VERSION failed, check that RtMidi.h defines RTMIDI_VERSION as "$PACKAGE_VERSION" or that the first line of configure.ac has been updated.])])

# Enable some nice automake features if they are available
m4_ifdef([AM_MAINTAINER_MODE], [AM_MAINTAINER_MODE])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# standards version
m4_include([m4/ax_cxx_compile_stdcxx.m4])
AX_CXX_COMPILE_STDCXX(11, noext, mandatory)

# configure flags
AC_ARG_ENABLE([debug], [AS_HELP_STRING([--enable-debug], [enable various debugging output])])
AC_ARG_WITH(jack, [AS_HELP_STRING([--with-jack], [choose JACK server support])])
AC_ARG_WITH(alsa, [AS_HELP_STRING([--with-alsa], [choose native ALSA sequencer API support (linux only)])])
AC_ARG_WITH(core, [AS_HELP_STRING([--with-core], [ choose CoreMIDI API support (mac only)])])
AC_ARG_WITH(winmm, [AS_HELP_STRING([--with-winmm], [ choose Windows MultiMedia (MM) API support (win32 only)])])
AC_ARG_WITH(winks, [AS_HELP_STRING([--with-winks], [  choose kernel streaming support (win32 only)])])
AC_ARG_WITH(webmidi, [AS_HELP_STRING([--with-webmidi], [  choose Web MIDI support])])
AC_ARG_WITH(android, [AS_HELP_STRING([--with-android], [  choose Android support])])


# Checks for programs.
AC_PROG_CXX(g++ CC c++ cxx)
AM_PROG_AR
AC_PATH_PROG(AR, ar, no)
AS_IF([test "x$AR" = "xno"], [
    AC_MSG_ERROR([Could not find ar - needed to create a library])
])

LT_INIT([win32-dll])
AC_CONFIG_MACRO_DIR([m4])

# Checks for header files.
AC_HEADER_STDC
#AC_CHECK_HEADERS(sys/ioctl.h unistd.h)

# Check for POSIX semaphore support
AC_CHECK_HEADER([semaphore.h], [
  AC_CHECK_LIB(pthread, sem_init,
    AC_DEFINE([HAVE_SEMAPHORE],[1],[Define to 1 if you have POSIX semaphore support on your system.]),
    AC_MSG_WARN([POSIX semaphore support not found; data may be lost on closePort]))
])


# check for debug
AC_MSG_CHECKING(whether to compile debug version)
debugflags=""
object_path=Release
AS_CASE([${enable_debug}],
  [ yes ], [
    AC_MSG_RESULT([yes])
    CPPFLAGS="-D__RTMIDI_DEBUG ${CPPFLAGS}"
    debugflags="${debugflags} -g"
    object_path=Debug
  ],
  [ no ], [
    AC_MSG_RESULT([no!])
    debugflags="${debugflags} -O3"
  ], [
    AC_MSG_RESULT([no])
  ])
# For debugging and optimization ... overwrite default because it has both -g and -O2
AS_IF([test "x$debugflags" != x],
  AS_IF([test "x$override_cxx" = "xyes" ], CXXFLAGS="$CXXFLAGS $debugflags", CXXFLAGS="$debugflags $CXXFLAGS")
  AS_IF([test "x$override_c" = "xyes" ], CFLAGS="$CFLAGS $debugflags", CFLAGS="$debugflags $CFLAGS")
  )

# Check compiler and use -Wall if gnu.
AS_IF([test "x$GXX" = "xyes"], [
  CXXFLAGS="-Wall -Wextra ${CXXFLAGS}"
])

# Checks for doxygen
AC_CHECK_PROG( DOXYGEN, [doxygen], [doxygen] )
AM_CONDITIONAL( MAKE_DOC, [test "x${DOXYGEN}" != x] )

# Copy doc files to build dir if necessary
AC_CONFIG_LINKS( [doc/doxygen/footer.html:doc/doxygen/footer.html] )
AC_CONFIG_LINKS( [doc/doxygen/header.html:doc/doxygen/header.html] )
AC_CONFIG_LINKS( [doc/doxygen/tutorial.txt:doc/doxygen/tutorial.txt] )
AC_CONFIG_LINKS( [doc/images/ccrma.gif:doc/images/ccrma.gif] )
AC_CONFIG_LINKS( [doc/images/mcgill.gif:doc/images/mcgill.gif] )

# Checks for package options and external software
AC_CANONICAL_HOST

# Aggregate options into a single string.
AS_IF([test "x$with_jack"   = "xyes"], [systems="$systems jack"])
AS_IF([test "x$with_alsa"   = "xyes"], [systems="$systems alsa"])
AS_IF([test "x$with_core"   = "xyes"], [systems="$systems core"])
AS_IF([test "x$with_winmm"  = "xyes"], [systems="$systems winmm"])
AS_IF([test "x$with_winks"  = "xyes"], [systems="$systems winks"])
AS_IF([test "x$with_webmidi" = "xyes"], [systems="$systems webmidi"])
AS_IF([test "x$with_android" = "xyes"], [systems="$systems android"])
AS_IF([test "x$with_dummy"  = "xyes"], [systems="$systems dummy"])
required=" $systems "

# If none, assign defaults if any are known for this OS.
# User must specified with-* options for any unknown OS.
AS_IF([test "x$systems" = "x"],
  AS_CASE([$host],
    [*-*-linux*],         [systems="alsa jack"],
    [*-*-gnu],            [systems="jack"],
    [*-*-kfreebsd*-gnu],  [systems="jack"],
    [*-apple*],           [systems="core jack"],
    [*-mingw32*],         [systems="winmm winks jack"],
    [*-mingw64*],         [systems="winmm winks jack"]
  ))

# If any were specifically requested disabled, do it.
AS_IF([test "x$with_jack"   = "xno"], [systems=`echo $systems|tr ' ' \\\\n|grep -v jack`])
AS_IF([test "x$with_alsa"   = "xno"], [systems=`echo $systems|tr ' ' \\\\n|grep -v alsa`])
AS_IF([test "x$with_winmm"  = "xno"], [systems=`echo $systems|tr ' ' \\\\n|grep -v winmm`])
AS_IF([test "x$with_winks"  = "xno"], [systems=`echo $systems|tr ' ' \\\\n|grep -v winks`])
AS_IF([test "x$with_core"   = "xno"], [systems=`echo $systems|tr ' ' \\\\n|grep -v core`])
AS_IF([test "x$with_webmidi" = "xno"], [systems=`echo $systems|tr ' ' \\\\n|grep -v webmidi`])
AS_IF([test "x$with_android" = "xno"], [systems=`echo $systems|tr ' ' \\\\n|grep -v android`])
AS_IF([test "x$with_dummy"  = "xno"], [systems=`echo $systems|tr ' ' \\\\n|grep -v dummy`])
systems=" `echo $systems|tr \\\\n ' '` "

# For each MIDI system, check if it is selected and found.
# Note: Order specified above is not necessarily respected. However,
# *actual* priority is set at run-time, see RtMidiIn::openMidiApi and RtMidiOut::openMidiApi.
# One AS_CASE per system, since they are not mutually-exclusive.

AS_CASE(["$systems"], [*" jack "*], [
  AC_CHECK_LIB(jack, jack_client_open,
    [api="$api -D__UNIX_JACK__"
     req="$req jack"
     need_pthread=yes
     found="$found Jack"
     LIBS="-ljack $LIBS"],
    AS_CASE(["$required"], [*" jack "*],
      AC_MSG_ERROR([JACK support requires the jack library!])))
  AC_CHECK_LIB(jack, jack_port_rename, AC_DEFINE(JACK_HAS_PORT_RENAME), )
])

AS_CASE(["$systems"], [*" alsa "*], [
  AC_CHECK_LIB(asound, snd_seq_open,
    [api="$api -D__LINUX_ALSA__"
     req="$req alsa"
     need_pthread=yes
     found="$found ALSA"
     LIBS="-lasound $LIBS"],
    AS_CASE(["$required"], [*" alsa "*],
      AC_MSG_ERROR([ALSA support requires the asound library!])))
])

AS_CASE(["$systems"], [*" core "*], [
  AC_CHECK_HEADER(CoreMIDI/CoreMIDI.h,
    [api="$api -D__MACOSX_CORE__"
     req_libs="$req_libs -framework CoreMIDI -framework CoreAudio -framework CoreFoundation"
     need_pthread=yes
     found="$found CoreMIDI",
     LIBS="$LIBS -framework CoreMIDI -framework CoreFoundation -framework CoreAudio"],
    AS_CASE(["$required"], [*" core "*],
      AC_MSG_ERROR([CoreMIDI header files not found!])))
])

AS_CASE(["$systems"], [*" winmm "*], [
  AC_CHECK_HEADERS(windows.h mmsystem.h,
    [api="$api -D__WINDOWS_MM__"
     CPPFLAGS="-I$srcdir/include $CPPFLAGS"
     need_ole32=yes
     found="$found WinMM"
     api="$api -D__WINDOWS_MM__"
     LIBS="-lwinmm ${LIBS}"])
])

AS_CASE(["$systems"], [*" winks "*], [
  AC_CHECK_HEADERS(windows.h audioclient.h avrt.h mmdeviceapi.h,
    [api="$api -D__WINDOWS_WINKS__"
     CPPFLAGS="-I$srcdir/include $CPPFLAGS"
     need_ole32=yes
     found="$found kernel-streaming"
     api="$api -D__WINDOWS_WINKS__"
     LIBS="-lsetupapi -lksuser ${LIBS}"])
])

AS_CASE(["$systems"], [*" webmidi "*], [
  AC_CHECK_HEADERS(emscripten.h,
    [api="$api -D__WEB_MIDI_API__"
     found="$found Web MIDI"])
])

AS_CASE(["$systems"], [*" android "*], [
      AC_MSG_ERROR([Android NDK configuration not implemented, use CMAKE])
])

AS_IF([test -n "$need_ole32"], [LIBS="-lole32 $LIBS"])

AS_IF([test -n "$need_pthread"],[
  AC_MSG_CHECKING([for pthread])
  AC_CHECK_LIB(pthread, pthread_create, ,
    AC_MSG_ERROR([RtAudio requires the pthread library!]))])

AC_MSG_CHECKING([for MIDI API])

# Error case: no known realtime systems found.
AS_IF([test x"$api" = "x"], [
  AC_MSG_RESULT([none])
  AC_MSG_ERROR([No known system type found for MIDI support!])
], [
  AC_MSG_RESULT([$found])
])

CPPFLAGS="$CPPFLAGS $api"

AC_OUTPUT

chmod oug+x rtmidi-config
