dnl configure.ac: source for the configure script

dnl copyright by the mpg123 project - free software under the terms of the LGPL 2.1
dnl see COPYING and AUTHORS files in distribution or http://mpg123.org
dnl initially written by Nicholas J. Humfrey

dnl Autoconf 2.71 wants some changes, hope we kept it fine for
dnl 2.69 at least.
AC_PREREQ([2.69])

dnl ############# Initialisation

dnl Set version numbers from header version.h and each library API header.
dnl Do not forget the re-generate configure on version changes. This does
dnl not happen automatically, as it would with m4_include() usage, wich
dnl is too fragile with arbitrary header contents.

m4_define([V_HEADER], m4_esyscmd(grep MPG123_ src/version.h))
m4_define([V_MAJOR], m4_bregexp(V_HEADER,
        [#define MPG123_MAJOR\s+\([0-9]+\)],
        [\1]))
m4_define([V_MINOR], m4_bregexp(V_HEADER,
        [\#define MPG123_MINOR\s+\([0-9]+\)],
        [\1]))
m4_define([V_PATCH], m4_bregexp(V_HEADER,
        [\#define MPG123_PATCH\s+\([0-9]+\)],
        [\1]))
m4_define([V_SUFFIX], m4_bregexp(V_HEADER,
        [\#define MPG123_SUFFIX\s+"\(.+\)"],
        [\1]))
m4_undefine([V_HEADER])

AC_INIT([mpg123], V_MAJOR.V_MINOR.V_PATCH[]V_SUFFIX, [maintainer@mpg123.org])

m4_define([V_HEADER], m4_esyscmd(grep -e _API_VERSION -e _PATCHLEVEL src/include/mpg123.h src/include/out123.h src/include/syn123.h))

dnl libmpg123
m4_define([API_VERSION], m4_bregexp(V_HEADER,
        [\#define MPG123_API_VERSION\s+\([0-9]+\)],
        [\1]))
m4_define([LIB_PATCHLEVEL], m4_bregexp(V_HEADER,
        [\#define MPG123_PATCHLEVEL\s+\([0-9]+\)],
        [\1]))

dnl libout123
m4_define([OUTAPI_VERSION], m4_bregexp(V_HEADER,
        [\#define OUT123_API_VERSION\s+\([0-9]+\)],
        [\1]))
m4_define([OUTLIB_PATCHLEVEL], m4_bregexp(V_HEADER,
        [\#define OUT123_PATCHLEVEL\s+\([0-9]+\)],
        [\1]))

dnl libsyn123
m4_define([SYNAPI_VERSION], m4_bregexp(V_HEADER,
        [\#define SYN123_API_VERSION\s+\([0-9]+\)],
        [\1]))
m4_define([SYNLIB_PATCHLEVEL], m4_bregexp(V_HEADER,
        [\#define SYN123_PATCHLEVEL\s+\([0-9]+\)],
        [\1]))

m4_undefine([V_HEADER])

dnl Since we want to be backwards compatible, both sides get set to API_VERSION.
LIBMPG123_VERSION=API_VERSION:LIB_PATCHLEVEL:API_VERSION
LIBOUT123_VERSION=OUTAPI_VERSION:OUTLIB_PATCHLEVEL:OUTAPI_VERSION
LIBSYN123_VERSION=SYNAPI_VERSION:SYNLIB_PATCHLEVEL:SYNAPI_VERSION
AC_SUBST([LIBMPG123_VERSION])
AC_SUBST([LIBOUT123_VERSION])
AC_SUBST([LIBSYN123_VERSION])

AC_CONFIG_SRCDIR(src/mpg123.c)
AC_CONFIG_AUX_DIR(build)
AC_CONFIG_SRCDIR(doc)
AC_CONFIG_MACRO_DIR([m4])
AC_CANONICAL_HOST

dnl Version 1.7 of automake is recommended
dnl Not sure what minimal version does not choke on sub directories.
dnl Testing with 1.14.
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_HEADERS([src/config.h])

dnl You tell me how quoting this normally would work to avoid expansion!
if test 2 -lt "$(grep "PKG_"CHECK_MODULES "$0"|wc -l)"; then
  AC_MSG_ERROR([Pkg-config was absent when generating this configure script.
  Re-run autoreconf with pkg-config installed top get a working one.])
fi
dnl For some reason, this has to be after the check above.
PKG_PROG_PKG_CONFIG([])

# You get strange symptoms like jack module build failing because the AC_C_CONST failed to detect the working const support.
# In that case, the test failed because -Werror, not because no const there...
# After looking again, there are possibly more tests being obscured by false failures.
AC_MSG_CHECKING([for -Werror in CFLAGS (It breaks tests)])
if echo "$CFLAGS" | grep Werror; then
	AC_MSG_RESULT([yes])
	AC_MSG_WARN([You have -Werror in CFLAGS. That may break some tests and make this configure bogus.
If you want paranoid compilation, use --enable-nagging option, which adds -Werror for gcc.
Also note that you shall not run make distcheck after configuring with --enable-nagging.
distcheck uses the generated CFLAGS...
Anyhow, continuing at your own risk.])
else
	AC_MSG_RESULT([no])
fi

buffer=enabled # try to build with buffer by default

dnl ############# Compiler and tools Checks

LT_LDFLAGS=-export-dynamic
EXEC_LT_LDFLAGS=
be_static=no
all_static=no
lib_static=no
AC_MSG_CHECKING([if you are up to something totally static with LDFLAGS/CFLAGS])
for f in $LDFLAGS $CFLAGS
do
	case "$f" in
		-all-static)
			be_static=yes
			all_static=yes
			lib_static=yes
		;;
		-static)
			be_static=yes
			lib_static=yes
		;;
		-static-libgcc)
			lib_static=yes
		;;
	esac
done
if test "x$be_static" = xyes; then
	AC_MSG_RESULT([yes])
	LT_LDFLAGS=-all-static
	EXEC_LT_LDFLAGS="$LT_LDFLAGS"
else
	AC_MSG_RESULT([no])
fi
if test "x$all_static" = xyes; then
	AC_MSG_WARN( Use -static in LDFLAGS for all-static linking! Your compiler may blow up on that -all-static. )
fi

AM_PROG_AS
m4_version_prereq(2.70, [AC_PROG_CC], [AC_PROG_CC_C99])
AC_PROG_CPP
AC_PROG_INSTALL
# Just look, no touch.
oldLIBS=$LIBS
LT_LIB_M
dnl Test if -lrt is required for clock_gettime() (old glibc).
AC_SEARCH_LIBS([clock_gettime], [rt])
if echo " $LIBS" | grep -q -w -- -lrt; then
  LIBRT=-lrt
fi
# Could actually be missing.
AC_CHECK_FUNCS([clock_gettime])
LIBS=$oldLIBS
AC_SUBST(LIBM)
dnl "Checking for egrep is broken after removal of libltdl stuff... checks use $EGREP, so searching it here."
AC_PROG_EGREP
AC_C_CONST
AC_C_INLINE
AC_C_BIGENDIAN

if test "x$lib_static" = xyes; then
        CC="$CC -static-libgcc"
        CCLD="$CC"
	echo "Adding -static-libgcc"
fi

dnl ############# Use Libtool for dynamic module loading

modules=auto
AC_ARG_ENABLE(modules,
[AS_HELP_STRING([--disable-modules], [dynamically loadable output module support (default enabled in shared library builds)])],
[
	if test "x$enableval" = xyes
	then
		modules=enabled
	else
		modules=disabled
	fi
],
[
	if test "x$be_static" = "xyes"; then
		modules=disabled
	else
		modules=auto
	fi
])

if test x"$enable_shared" = xno; then
	modules=disabled
	LT_LDFLAGS=
else
	AC_DEFINE(DYNAMIC_BUILD, 1, [ Define if building with dynamcally linked libmpg123])
fi

dnl We need the windows header also for checking the module mechanism.
AC_CHECK_HEADERS([windows.h])
# enable win32 code
if test "x$ac_cv_header_windows_h" = xyes -a "x$host_os" != xcygwin -a "x$host_os" != xmidipix; then
  win32_specific_codes=enabled
fi

android_build=no
case "$host" in
	*-*-mingw*)
		win32=yes
		AC_MSG_CHECKING([if this is a UWP build])
			AC_PREPROC_IFELSE([AC_LANG_PROGRAM(
			  [[#include <winapifamily.h>
			   #if WINAPI_FAMILY_PARTITION (WINAPI_PARTITION_DESKTOP)
			   # error Win32 Desktop build
			   #endif
			  ]],[[;]])
			],[
				uwp_build=yes
				AC_MSG_RESULT([yes])
			],[
				uwp_build=no
				AC_MSG_RESULT([no])
			])
		if test x"$uwp_build" = xyes; then
		  AC_DEFINE( [WINDOWS_UWP], 1, [Windows UWP build] )
		fi
	;;
	linux*)
		dnl Android is linux, but a bit different
		AC_MSG_CHECKING([for an Android system])
		AC_PREPROC_IFELSE([AC_LANG_PROGRAM(
				[[#ifndef __ANDROID__
				 # error Not Android
				 #endif
				]],[[;]])
		],[
		  android_build=yes
		  AC_MSG_RESULT([yes])
		],[
		  AC_MSG_RESULT([no])
		])
	;;
	*)
		win32=no
	;;
esac


if test x$win32_specific_codes = xenabled; then
  # Check GetThreadErrorMode
  if test x"$modules" != xdisabled; then
    AC_MSG_CHECKING([if we have GetThreadErrorMode])
      AC_LINK_IFELSE([AC_LANG_SOURCE([
      #include <windows.h>
      int main(){
        return GetThreadErrorMode();
      }])],[win32_thread_error=yes],[win32_winver_bump=yes])
    if test "x$win32_thread_error" != "xyes"; then
      AC_MSG_CHECKING([if we have GetThreadErrorMode])
        AC_LINK_IFELSE([AC_LANG_SOURCE([
        #define WINVER 0x600
        #define _WIN32_WINNT 0x600
        #include <windows.h>
        int main(){
          return GetThreadErrorMode();
        }])],[win32_thread_error=yes],[win32_thread_error=no])
    fi
    if test "x$win32_thread_error" = "xyes"; then
      AC_MSG_RESULT([yes])
    else
      AC_MSG_RESULT([no])
      if test "x$modules" = xenabled; then
        AC_MSG_ERROR([GetThreadErrorMode is required for modules but not found.])
      else
        AC_MSG_WARN([Disabling modules because of missing GetThreadErrorMode()])
        modules=disabled
      fi
    fi
  fi
fi

LIBDL=

if test x"$modules" = xdisabled
then
  echo "Modules disabled, not checking for dynamic loading."
else
  have_dl=no
  dl_missing=no
  # The dlopen() API is either in libc or in libdl.
  if test "x$ac_cv_header_windows_h" = xyes &&
     test "x$win32" = xyes; then
    AC_MSG_CHECKING([if LoadLibrary should be used])
    AC_LINK_IFELSE([AC_LANG_SOURCE([

#include <windows.h>

int main() {
  LoadLibraryW(0);
  GetProcAddress(0, 0);
  FreeLibrary(0);
}
    ])], [
    have_dl=yes
    AC_MSG_RESULT([Using LoadLibrary])
    ],
  [AC_MSG_RESULT([no])])
  else
    # Check for libdl functions, but only apply them for libcompat_dl.
    # This looks too messy. Can I avoid messing with LIBS here?
    oldLIBS=$LIBS
    AC_SEARCH_LIBS(dlopen, dl)
    AC_SEARCH_LIBS(dlsym, dl)
    AC_SEARCH_LIBS(dlclose, dl)
    AC_CHECK_HEADER(dlfcn.h)
    AC_CHECK_FUNCS(dlopen dlsym dlclose, [ have_dl=yes ], [ dl_missing=yes] )
    LIBS=$oldLIBS
    # A better way to check if the library was really needed?
    if test "x$ac_cv_search_dlopen"  = x-ldl ||
       test "x$ac_cv_search_dlsym"   = x-ldl ||
       test "x$ac_cv_search_dlclose" = x-ldl
    then
       LIBDL=-ldl
    fi
  fi
  if test x"$dl_missing" = xyes; then
    AC_MSG_WARN([Some dynamic loading functions missing.])
    have_dl=no
  fi
  if test x"$modules" = xenabled -a x"$have_dl" = xno; then
    AC_MSG_ERROR([Modules enabled but no runtime loader found! This will not work...])
  fi
  if test x"$modules" = xauto; then
    if test x"$have_dl" = xyes; then
      modules=enabled
      echo "We found a runtime loader: Modules enabled."
    else
      echo "We did not find a runtime loader: Modules disabled."
      modules=disabled
    fi
  fi
fi

AC_SUBST(LIBDL)

dnl Configure libtool

LT_INIT([win32-dll disable-static])

if test x"$modules" = xdisabled
then
  echo "Modules disabled."
else
  # Enable module support in source code
  AC_DEFINE( USE_MODULES, 1, [Define if modules are enabled] )
  # Export the module file suffix as LT_MODULE_EXT
  LT_SYS_MODULE_EXT
fi
AM_CONDITIONAL( [HAVE_MODULES], [test "x$modules" = xenabled] )

# Ensure that a possibly hijacked libtool does not build any static libs.
if test x"$enable_static" = xno; then
	LIB_CFLAGS=-shared
else
	LIB_CFLAGS=
fi

AC_SUBST(LIB_CFLAGS)
AC_SUBST(LT_LDFLAGS)
AC_SUBST(EXEC_LT_LDFLAGS)

dnl ############## Component selection

AC_ARG_ENABLE(components,
  [AS_HELP_STRING([--disable-components], [(not) build end-user programs with default components, select individual components via --enable-libmpg123 etc.])],
  [
    if test "x$enableval" = xyes
    then
      build_all=yes
    else
      build_all=no
    fi
  ],
  [
    build_all=yes
  ]
)

AC_ARG_ENABLE(programs,
  [AS_HELP_STRING([--enable-programs], [build (only) programs (with --disable-components)])],
  [
    if test "x$enableval" = xyes
    then
      build_programs=yes
    else
      build_programs=no
    fi
  ],
  [
    build_programs=no
  ]
)

AC_ARG_ENABLE(libmpg123,
  [AS_HELP_STRING([--enable-libmpg123], [build (only) libmpg123 (with --disable-components)])],
  [
    if test "x$enableval" = xyes
    then
      build_libmpg123=yes
    else
      build_libmpg123=no
    fi
  ],
  [
    build_libmpg123=no
  ]
)

AC_ARG_ENABLE(libout123,
  [AS_HELP_STRING([--enable-libout123], [build (only) libout123 (with --disable-components)])],
  [
    if test "x$enableval" = xyes
    then
      build_libout123=yes
    else
      build_libout123=no
    fi
  ],
  [
    build_libout123=no
  ]
)

AC_ARG_ENABLE(libout123-modules,
  [AS_HELP_STRING([--enable-libout123-modules], [build (only) libout123 modules (with --disable-components)])],
  [
    if test "x$enableval" = xyes
    then
      build_libout123_modules=yes
    else
      build_libout123_modules=no
    fi
  ],
  [
    build_libout123_modules=no
  ]
)

AC_ARG_ENABLE(libsyn123,
  [AS_HELP_STRING([--enable-libsyn123], [build (only) libsyn123 (with --disable-components)])],
  [
    if test "x$enableval" = xyes
    then
      build_libsyn123=yes
    else
      build_libsyn123=no
    fi
  ],
  [
    build_libsyn123=no
  ]
)

if test "x$build_all" = xyes; then
  build_programs=yes
  build_libmpg123=yes
  build_libout123=yes
  build_libout123_modules=yes
  build_libsyn123=yes
fi

AM_CONDITIONAL([BUILD_PROGRAMS],  [ test "x$build_programs" = xyes ])
AM_CONDITIONAL([BUILD_LIBMPG123], [ test "x$build_libmpg123" = xyes ])
AM_CONDITIONAL([BUILD_LIBOUT123], [ test "x$build_libout123" = xyes ])
AM_CONDITIONAL([BUILD_LIBOUT123_MODULES], [ test "x$build_libout123_modules" = xyes ])
AM_CONDITIONAL([BUILD_LIBOUT123_OR_MODULES], [ test "x$build_libout123" = xyes || test "x$build_libout123_modules" = xyes ])
AM_CONDITIONAL([BUILD_LIBSYN123], [ test "x$build_libsyn123" = xyes ])
# If we install libraries, prompting pkgconfig and include directories.
AM_CONDITIONAL([NEED_LIB], [ test "x$build_libmpg123" = xyes || test "x$build_libout123" = xyes || test "x$build_libout123_modules" = xyes || test "x$build_libsyn123" = xyes ])
AM_CONDITIONAL([NEED_MAINLIB], [ test "x$build_libmpg123" = xyes || test "x$build_libout123" = xyes || test "x$build_libsyn123" = xyes ])

components=""

if test "x$build_programs" = xyes; then
  components="$components programs"
fi

if test "x$build_libmpg123" = xyes; then
  components="$components libmpg123"
fi
if test "x$build_libout123" = xyes; then
  components="$components libout123"
fi
if test "x$build_libout123_modules" = xyes; then
  components="$components libout123-modules"
fi
if test "x$build_libsyn123" = xyes; then
  components="$components libsyn123"
fi

components=$(echo $components)

dnl ############## Configurable Options

AC_ARG_ENABLE(debug,
              [AS_HELP_STRING([--enable-debug], [turn on (lots of) debugging printouts])],
              [
                if test "x$enableval" = xyes
                then
                  debugging="enabled"
                else
                  debugging="disabled"
                fi
              ],
              [ debugging="disabled" ]
)

AC_ARG_ENABLE(xdebug,
              [AS_HELP_STRING([--enable-xdebug], [turn on (yet more) extreme debugging printouts])],
              [
                if test "x$enableval" = xyes
                then
                  xdebugging="enabled"
                else
                  xdebugging="disabled"
                fi
              ],
              [ xdebugging="disabled" ]
)

if test x"$xdebugging" = xenabled; then
  debugging=enabled
  AC_DEFINE(XDEBUG, 1, [ Define for extreme debugging. ])
fi

AC_ARG_ENABLE(nagging,
              [AS_HELP_STRING([--enable-nagging], [turn on GCC's pedantic nagging with error on warnings])],
              [
                if test "x$enableval" = xyes
                then
                  nagging="enabled"
               else
                  nagging="disabled"
                fi
              ],
              [ nagging="disabled" ]
)

if test x"$debugging" = xenabled; then
	AC_DEFINE(DEBUG, 1, [ Define if debugging is enabled. ])
fi

AC_ARG_ENABLE(gapless,
              [AS_HELP_STRING( [--disable-gapless], [turn off gapless decoding (please don't)])],
              [
                if test "x$enableval" = xyes
                then
                  gapless="enabled"
                else
                  gapless="disabled"
                fi
              ],
              [ gapless="enabled" ]
)
if test "x$gapless" = xenabled; then
  AC_DEFINE(GAPLESS, 1, [ Define if gapless is enabled. ])
fi

AC_ARG_ENABLE(fifo,
              [AS_HELP_STRING( [--enable-fifo], [FIFO support for control interface (auto-enabled on linux)])],
              [
                if test "x$enableval" = xyes
                then
                  fifo="enabled"
                else
                  fifo="disabled"
                fi
              ],
              [
                fifo="auto"
              ]
)

dnl Optional objects list, depends on decoder choice and core feature selection.
dnl Not just for specific decoders anymore...

dnl Core features that can be disabled to reduce binary size.

id3v2=enabled
AC_ARG_ENABLE(id3v2,
              [AS_HELP_STRING( [--disable-id3v2], [no ID3v2 parsing])],
              [
                if test "x$enableval" = xno; then
                  id3v2="disabled"
                fi
              ], [])

# id3v2 depends on strings... so check that in between.
string=enabled
AC_ARG_ENABLE(string,
              [AS_HELP_STRING( [--disable-string], [no string API (this will disable ID3v2; main mpg123 won't build anymore)])],
              [
                if test "x$enableval" = xno; then
                  string="disabled"
                fi
              ], [])

if test "x$string" = "xdisabled"; then
  AC_DEFINE(NO_STRING, 1, [ Define to disable string functions. ])
  id3v2=disabled
  AC_MSG_WARN([ID3v2 support disabled because of string API being disabled.])
fi
AM_CONDITIONAL([HAVE_STRING], [test "x$string" = xenabled])

if test "x$id3v2" = "xdisabled"; then
  AC_DEFINE(NO_ID3V2, 1, [ Define to disable ID3v2 parsing. ])
fi

icy=enabled
AC_ARG_ENABLE(icy,
              [AS_HELP_STRING( [--disable-icy], [no ICY metainfo parsing/conversion (main mpg123 won't build!)])],
              [
                if test "x$enableval" = xno; then
                  icy="disabled"
                fi
              ], [])

if test "x$icy" = "xdisabled"; then
  AC_DEFINE(NO_ICY, 1, [ Define to disable ICY handling. ])
fi
AM_CONDITIONAL([HAVE_ICY], [test "x$icy" = xenabled])

ntom=enabled
AC_ARG_ENABLE(ntom,
              [AS_HELP_STRING( [--disable-ntom], [no flexible resampling])],
              [
                if test "x$enableval" = xno; then
                  ntom="disabled"
                fi
              ], [])

if test "x$ntom" = "xdisabled"; then
  AC_DEFINE(NO_NTOM, 1, [ Define to disable ntom resampling. ])
fi
AM_CONDITIONAL([HAVE_NTOM], [test "x$ntom" = xenabled])

downsample=enabled
AC_ARG_ENABLE(downsample,
              [AS_HELP_STRING( [--disable-downsample], [no downsampled decoding])],
              [
                if test "x$enableval" = xno; then
                  downsample="disabled"
                fi
              ], [])

if test "x$downsample" = "xdisabled"; then
  AC_DEFINE(NO_DOWNSAMPLE, 1, [ Define to disable downsampled decoding. ])
fi

feeder=enabled
AC_ARG_ENABLE(feeder,
              [AS_HELP_STRING( [--disable-feeder], [no feeder decoding, no buffered readers])],
              [
                if test "x$enableval" = xno; then
                  feeder="disabled"
                fi
              ], [])

if test "x$feeder" = "xdisabled"; then
  AC_DEFINE(NO_FEEDER, 1, [ Define to disable feeder and buffered readers. ])
fi

moreinfo=enabled
AC_ARG_ENABLE(moreinfo,
              [AS_HELP_STRING( [--disable-moreinfo], [no extra information for frame analyzers])],
              [
                if test "x$enableval" = xno; then
                  moreinfo="disabled"
                fi
              ], [])

if test "x$moreinfo" = "xdisabled"; then
  AC_DEFINE(NO_MOREINFO, 1, [ Define to disable analyzer info. ])
fi

messages=enabled
AC_ARG_ENABLE(messages,
              [AS_HELP_STRING( [--disable-messages], [no error/warning messages on the console])],
              [
                if test "x$enableval" = xno; then
                  messages="disabled"
                fi
              ], [])

if test "x$messages" = "xdisabled"; then
  AC_DEFINE(NO_WARNING, 1, [ Define to disable warning messages. ])
  AC_DEFINE(NO_ERRORMSG,   1, [ Define to disable error messages. ])
  AC_DEFINE(NO_ERETURN, 1, [ Define to disable error messages in combination with a return value (the return is left intact). ])
fi

runtimetables=disabled
AC_ARG_ENABLE(runtime-tables,
[AS_HELP_STRING([--enable-runtime-tables], [calculate tables at runtime saving size at the expense of additional computation at load time])],
[
  if test "x$enableval" = xyes; then
    runtimetables=enabled
  fi
]
, [])

if test "x$runtimetables" = "xenabled"; then
  AC_DEFINE(RUNTIME_TABLES, 1, [ Define for calculating tables at runtime. ])
fi

AM_CONDITIONAL([HAVE_RUNTIME_TABLES], [test "x$runtimetables" = xenabled] )

newhuff=enabled
AC_ARG_ENABLE(new-huffman,
[AS_HELP_STRING([--disable-new-huffman], [new huffman decoding scheme by Taihei (faster on modern CPUs at least, so on by default)])],
[
  if test "x$enableval" = xno; then
    newhuff=disabled
  fi
]
, [])

if test "x$newhuff" = "xenabled"; then
  AC_DEFINE(USE_NEW_HUFFTABLE, 1, [ Define for new Huffman decoding scheme. ])
fi

integers=quality
AC_ARG_ENABLE(int-quality,
[AS_HELP_STRING([--disable-int-quality], [use rounding instead of fast truncation for integer output, where possible])],
[
  if test "x$enableval" = xno; then
    integers=fast
  fi
], [])

if test $integers = quality; then
  AC_DEFINE(ACCURATE_ROUNDING,   1, [ Define to use proper rounding. ])
fi
AM_CONDITIONAL([HAVE_ACCURATE], [ test x"$integers" = xquality ])

int16=enabled
AC_ARG_ENABLE(16bit,
              [AS_HELP_STRING( [--disable-16bit], [no 16 bit integer output])],
              [
                if test "x$enableval" = xno; then
                  int16="disabled"
                fi
              ], [])

int8=enabled
AC_ARG_ENABLE(8bit,
              [AS_HELP_STRING( [--disable-8bit], [no 8 bit integer output])],
              [
                if test "x$enableval" = xno; then
                  int8="disabled"
                fi
              ], [])

int32=enabled
AC_ARG_ENABLE(32bit,
              [AS_HELP_STRING( [--disable-32bit], [no 32 bit integer output (also 24 bit)])],
              [
                if test "x$enableval" = xno; then
                  int32="disabled"
                fi
              ], [])

real=enabled
AC_ARG_ENABLE(real,
              [AS_HELP_STRING( [--disable-real], [no real (floating point) output])],
              [
                if test "x$enableval" = xno; then
                  real="disabled"
                fi
              ], [])

equalizer=enabled
AC_ARG_ENABLE(equalizer,
              [AS_HELP_STRING( [--disable-equalizer], [no equalizer support])],
              [
                if test "x$enableval" = xno; then
                  equalizer="disabled"
                fi
              ], [])

AC_ARG_WITH([cpu], [AS_HELP_STRING([--with-cpu=<type>], [select CPU optimization. Choices are:])

	generic[[_fpu]]    Use generic processor code with floating point arithmetic
	generic_float    Plain alias to generic_fpu now... float output is a normal runtime option!
	generic_nofpu    Use generic processor code with fixed point arithmetic (p.ex. ARM)
	generic_dither   Use generic processor code with floating point arithmetic and dithering for 1to1 16bit decoding.
	i386[[_fpu]]       Use code optimized for i386 processors with floating point arithmetic
	i386_nofpu       Use code optimized for i386 processors with fixed point arithmetic
	i486             Use code optimized for i486 processors (only usable alone!)
	i586             Use code optimized for i586 processors
	i586_dither      Use code optimized for i586 processors with dithering (noise shaping), adds 256K to binary size
	3dnow            Use code optimized for 3DNow processors
	3dnow_vintage    Use code optimized for older 3DNow processors (K6 family)
	3dnowext         Use code optimized for 3DNowExt processors (K6-3+, Athlon)
	3dnowext_alone   Really only 3DNowExt decoder, without 3DNow fallback for flexible rate
	3dnow_vintage    Use code optimized for older extended 3DNow processors (like K6-III+)
	mmx              Use code optimized for MMX processors
	mmx_alone        Really only MMX decoder, without i586 fallback for flexible rate
	sse              Use code optimized for SSE processors
	sse_vintage      Use code optimized for older SSE processors (plain C DCT36)
	sse_alone        Really only SSE decoder, without i586 fallback for flexible rate
	avx              Use code optimized for x86-64 with AVX processors
	x86              Pack all x86 opts into one binary (excluding i486, including dither)
	x86-64           Use code optimized for x86-64 processors (AMD64 and Intel64, including AVX and dithered generic)
	altivec          Use code optimized for Altivec processors (PowerPC G4 and G5)
	ppc_nofpu        Use code optimized for PowerPC processors with fixed point arithmetic
	neon             Use code optimized for ARM NEON SIMD engine (Cortex-A series)
	arm_fpu          Pack neon and generic[[_dither]] decoders, for ARM processors with FPU and/or NEON
	arm_nofpu        Use code optimized for ARM processors with fixed point arithmetic
	neon64           Use code optimized for AArch64 NEON SIMD engine
	aarch64          Pack neon64 and generic[[_dither]] decoders, for 64bit ARM processors
])

use_yasm=auto
AC_ARG_ENABLE(yasm,
              [AS_HELP_STRING( [--enable-yasm], [enforce yasm instad of default assembler for some optimizations (AVX, currently)])],
              [
                if test "x$enableval" = xyes; then
                  use_yasm="enabled"
                else
                  use_yasm="disabled"
                fi
              ], [])
if test x"$use_yasm" = xenabled; then
  AC_MSG_ERROR([Yasm for AVX is currently broken and might go away.])
fi

AC_ARG_ENABLE(ieeefloat,
  [AS_HELP_STRING( [--disable-ieeefloat], [use special hackery relying on IEEE 754 floating point storage format (to accurately round to 16 bit integer at bit more efficiently in generic decoder, enabled by default, disable in case you have a very special computer)])],
  [
    if test "x$enableval" = xyes; then
      ieee=enabled
    else
      ieee=disabled
    fi
  ], [ ieee=enabled ])

if test "x$ieee" = xenabled; then
  echo "We assume IEEE754 floating point format."
  AC_DEFINE(IEEE_FLOAT,   1, [ Define to indicate that float storage follows IEEE754. ])
fi

AC_ARG_ENABLE(cases,
  [AS_HELP_STRING([--disable-cases], [include special cases for likely parameter values (channel count, encoding sizes in libsyn123 routines) in the hope of better optimization at the expense of some code bloat])],
  [
     if test "x$enableval" = xyes; then
       specialcases=enabled
     else
       specialcases=disabled
     fi
  ], [ specialcases=enabled ])

if test "x$specialcases" = xdisabled; then
  AC_DEFINE(SYN123_NO_CASES, 1, [ Define to not duplicate some code for likely cases in libsyn123. ])
fi

sys_cppflags=
newoldwritesample=disabled
case $host in
  aarch64-*linux*|arm64-*linux*|aarch64-*bsd*|arm64-*bsd*|aarch64-apple-darwin*|arm64-apple-darwin*)
    cpu_type="aarch64"
  ;;
  arm*-*-linux*-*eabihf|armv7hl*-*-linux*)
    cpu_type="arm_fpu"
  ;;
  arm*-*-linux*)
    # check that... perhaps we are better off on arm with kernel math emulation
    cpu_type="arm_nofpu"
  ;;
  armv7*-apple-darwin*)
    cpu_type="arm_fpu"
  ;;
  i386-*-linux*|i386-*-kfreebsd*-gnu)
    cpu_type="i386_fpu"
    newoldwritesample=enabled
  ;;
  i486-*-linux*|i486-*-kfreebsd*-gnu)
    cpu_type="i486"
    newoldwritesample=enabled
  ;;
  i586-*-linux*|i586-*-kfreebsd*-gnu)
    cpu_type="x86"
    newoldwritesample=enabled
  ;;
  i686-*-linux*|i686-*-kfreebsd*-gnu)
    cpu_type="x86"
    newoldwritesample=enabled
  ;;
  x86_64-*-linux*|x86_64-*-kfreebsd*-gnu)
    cpu_type="x86-64"
  ;;
  *-*-linux*|*-*-kfreebsd*-gnu)
    cpu_type="generic_fpu"
  ;;
  i?86-apple-darwin10*)
    AC_MSG_CHECKING([if CPU type supports x86-64])
    case `sysctl -n hw.optional.x86_64` in
      1)
        AC_MSG_RESULT([yes])
        cpu_type="x86-64"
      ;;
      *)
        AC_MSG_RESULT([no])
        cpu_type="x86"
        newoldwritesample=enabled
      ;;
    esac
  ;;
  i?86-apple-darwin*)
    cpu_type="x86"
    newoldwritesample=enabled
  ;;
  x86_64-apple-darwin*)
    cpu_type="x86-64"
  ;;
  *-apple-darwin*)
    AC_MSG_CHECKING([if CPU type supports AltiVec])
    case `machine` in
      ppc7400 | ppc7450 | ppc970)
        AC_MSG_RESULT([yes])
        cpu_type="altivec"
      ;;
      *)
        AC_MSG_RESULT([no])
        cpu_type="generic_fpu"
      ;;
    esac
  ;;
  i?86-*-dragonfly* | i?86-*-freebsd* | i?86-*-midnightbsd* | i?86-*-mirbsd* | i?86-*-netbsd* | i?86-*-openbsd* | i?86-*-haiku*)
    cpu_type="x86"
    newoldwritesample=enabled
  ;;
  x86_64-*-dragonfly* | x86_64-*-freebsd* | x86_64-*-midnightbsd* | x86_64-*-mirbsd* | x86_64-*-netbsd* | x86_64-*-openbsd* | x86_64-*-haiku*)
    cpu_type="x86-64"
  ;;
  *-*-dragonfly* | *-*-freebsd* | *-*-midnightbsd* | *-*-mirbsd* | *-*-netbsd* | *-*-openbsd*)
    cpu_type="generic_fpu"
  ;;
  i386-*-solaris*)
    cpu_type=x86
    newoldwritesample=enabled
  ;;
  x86_64-*-solaris*)
    cpu_type=x86-64
  ;;
  *-*-solaris*)
    cpu_type="generic_fpu"
  ;;
  # os2-emx = OS/2 with some Unix fun; so p.ex. buffer works.
  # Till we sorted out the assembler troubles, generic CPU is default.
  i386-pc-os2-emx)
    cpu_type=generic_fpu
    newoldwritesample=enabled
  ;;
  x86_64-pc-os2-emx)
    # We are optimistic hat the future knows OS/2 on x86-64;-)
    cpu_type=generic_fpu
  ;;
  *-pc-os2-emx)
    cpu_type="generic_fpu"
  ;;
  *-dec-osf*)
    cpu_type="generic_fpu"
  ;;
  x86_64-*-cygwin*)
    cpu_type="x86-64"
  ;;
  i686-*-cygwin*)
    cpu_type="x86"
    newoldwritesample=enabled
  ;;
  i586-*-cygwin*)
    cpu_type="x86"
    newoldwritesample=enabled
  ;;
  i486-*-cygwin*)
    cpu_type="i486"
    newoldwritesample=enabled
  ;;
  i386-*-cygwin*)
    cpu_type="i386"
    newoldwritesample=enabled
  ;;
  *-cygwin*)
    cpu_type="generic_fpu"
  ;;
  i@<:@3-7@:>@86-*-mingw32*)
    LIBS="$LIBS"
    buffer=disabled
    cpu_type="x86"
    newoldwritesample=enabled
  ;;
  x86_64-*-mingw32*)
    LIBS="$LIBS"
    buffer=disabled
    cpu_type="x86-64"
  ;;
  i386-*-nto-qnx*)
    cpu_type="x86"
    newoldwritesample=enabled
  ;;
  *-ibm-aix*)
    AC_MSG_WARN([AIX system detected. You might want to --disable-largefile when trouble about conflicting types for lseek64 and friends occurs.])
    # Altivec instead? It is developed for MacOS ...
    cpu_type=generic_fpu
    # no struct winsize without _ALL_SOURCE
    sys_cppflags=-D_ALL_SOURCE
  ;;
	i386-*)
		AC_MSG_WARN([Unknown host operating system])
		cpu_type="i386"
		buffer=disabled
		sys_cppflags=-DGENERIC
    newoldwritesample=enabled
	;;
	i486-*)
		AC_MSG_WARN([Unknown host operating system])
		cpu_type="i486"
		buffer=disabled
		sys_cppflags=-DGENERIC
    newoldwritesample=enabled
	;;
	i586-*)
		AC_MSG_WARN([Unknown host operating system])
		cpu_type="x86"
		buffer=disabled
		sys_cppflags=-DGENERIC
    newoldwritesample=enabled
	;;
	i686-*)
		AC_MSG_WARN([Unknown host operating system])
		cpu_type="x86"
		buffer=disabled
		sys_cppflags=-DGENERIC
    newoldwritesample=enabled
	;;
	x86_64-*)
		AC_MSG_WARN([Unknown host operating system])
		cpu_type="x86-64"
		buffer=disabled
		sys_cppflags=-DGENERIC
	;;
  *)
  	AC_MSG_WARN([Unknown host operating system])
    cpu_type="generic_fpu"
    buffer=disabled
    sys_cppflags=-DGENERIC
  ;;
esac

AC_ARG_ENABLE(buffer,
  [AS_HELP_STRING([--disable-buffer], [disable audio buffer code (default enabled by some heuristics)])],
  [
    if test "x$enableval" = xyes
    then
      echo "Note: Enabling buffer per request... perhaps it will not build anyway."
      buffer="enabled"
    else
      echo "Note: Disabling buffer per request."
      buffer="disabled"
    fi
  ]
)

AC_ARG_ENABLE(newoldwritesample,
[AS_HELP_STRING( [--enable-newoldwritesample], [new/old WRITE_SAMPLE macro for non-accurate 16 bit output, faster on certain CPUs (default on on 32 bit x86)])],
[
	if test "x$enableval" = xyes
	then
		newoldwritesample=enabled
	else
		newoldwritesample=disabled
	fi
])

dnl Did user choose other CPU type ?
if test "x$with_cpu" != "x"; then
	cpu_type=$with_cpu
fi

# Flag for 32 bit synth output or post-processing.
case "$cpu_type" in
*_nofpu)
  synth32=false
  AC_DEFINE(NO_SYNTH32, 1, [ Define for post-processed 32 bit formats. ])
;;
*)
  synth32=true
;;
esac

if test "x$int16" = "xdisabled"; then
  AC_DEFINE(NO_16BIT, 1, [ Define to disable 16 bit integer output. ])
fi

AM_CONDITIONAL([HAVE_SYNTH16], [test "x$int16" = "xenabled"])

# 8bit works only through 16bit
if test "x$int16" = "xdisabled"; then
  int8=disabled
fi
if test "x$int8" = "xdisabled"; then
  AC_DEFINE(NO_8BIT, 1, [ Define to disable 8 bit integer output. ])
fi

AM_CONDITIONAL([HAVE_SYNTH8], [test "x$int8" = "xenabled"])

if test "x$int32" = "xdisabled"; then
  AC_DEFINE(NO_32BIT, 1, [ Define to disable 32 bit and 24 bit integer output. ])
fi
AM_CONDITIONAL([HAVE_SYNTH32], [ $synth32 && test x"$int32" = xenabled ])

if test "x$real" = "xdisabled"; then
  AC_DEFINE(NO_REAL, 1, [ Define to disable real output. ])
fi
AM_CONDITIONAL([HAVE_SYNTHREAL], [ $synth32 && test x"$real" = xenabled ] )

if test "x$equalizer" = "xdisabled"; then
  AC_DEFINE(NO_EQUALIZER, 1, [ Define to disable equalizer. ] )
fi
AM_CONDITIONAL([HAVE_EQUALIZER], [ test x"$equalizer" = xenabled ] )

layer1=enabled
AC_ARG_ENABLE(layer1,
              [AS_HELP_STRING( [--disable-layer1], [no layer I decoding])],
              [
                if test "x$enableval" = xno; then
                  layer1="disabled"
                  AC_DEFINE(NO_LAYER1, 1, [ Define to disable layer I. ])
                fi
              ], [])

AM_CONDITIONAL([HAVE_LAYER1], [test "x$layer1" = xenabled] )

layer2=enabled
AC_ARG_ENABLE(layer2,
              [AS_HELP_STRING( [--disable-layer2], [no layer II decoding])],
              [
                if test "x$enableval" = xno; then
                  layer2="disabled"
                  AC_DEFINE(NO_LAYER2, 1, [ Define to disable layer II. ])
                fi
              ], [])

AM_CONDITIONAL([HAVE_LAYER2], [test "x$layer2" = xenabled] )

layer3=enabled
AC_ARG_ENABLE(layer3,
              [AS_HELP_STRING( [--disable-layer3], [no layer III decoding])],
              [
                if test "x$enableval" = xno; then
                  layer3="disabled"
                  AC_DEFINE(NO_LAYER3, 1, [ Define to disable layer III. ])
                fi
              ], [])

AM_CONDITIONAL([HAVE_LAYER3], [test "x$layer3" = xenabled] )

AC_ARG_WITH([audio], [
AS_HELP_STRING([--with-audio=<list of modules>], [Select a list (or only one) of audio output modules (comma or space separated list). Choices are:])

	aix            Use AIX as default audio output sub-system
	alib           Use Alib as default audio output sub-system (for HPUX)
	alsa           Use ALSA as default audio output sub-system (libasound)
	tinyalsa       Use ALSA as default audio output sub-system (tinyalsa)
	arts           Use aRts as default audio output sub-system (KDE sound server)
	dummy          Use dummy as default audio (when no sound card is available)
	esd            Use ESoundD as default audio output sub-system
	hp             Use HP as default audio output sub-system
	jack           Use JACK as default low-latency audio server
	coreaudio      Use Mac OS X as default audio output sub-system (CoreAudio)
	mint           Use MinT as default audio output sub-system (Atari)
	nas            Use NAS as default audio output (Network Audio System)
	os2            Use OS2 as default audio output sub-system
	oss            Use OSS as default audio output sub-system (/dev/dsp)
	portaudio      Use PortAudio as default audio output sub-system
	pulse          Use Pulse audio server as default audio output sub-system
	qsa            Use QSA as default audio output sub-system
	sdl            Use SDL as default audio output sub-system (Simple DirectMedia Layer)
	sgi            Use SGI as default audio output sub-system (IRIX)
	sndio          Use OpenBSD's sndio as default audio output sub-system
	sun            Use Sun as default audio output sub-system (/dev/audio)
	win32          Use Win32 audio as default audio output sub-system
	win32_wasapi   Use Win32 wasapi audio as default audio output sub-system
])
AC_ARG_WITH([default-audio], [
  --with-default-audio=<list of modules>  Select a list of audio output modules to try at runtime, even if not built right now (comma or space separated). See --with-audio for options.
])

AC_ARG_WITH([optimization], [
  --with-optimization=0   No Optimization
  --with-optimization=1   Limited Optimization (-O) (for gcc)
  --with-optimization=2   Default Optimization (-O2 ...) (for gcc)
  --with-optimization=3   More Optimize than default (-O3 ...) (for gcc)
  --with-optimization=4   Optimize yet more (-O4 ...) (for gcc)
])

AC_ARG_WITH([seektable], [
  --with-seektable=<size> choose size of seek index table (0 disables it), default 1000
])


dnl ############## Assembler, compiler properties

# based on posting from John Dalgliesh <johnd@defyne.org> on ffmpeg (LGPL) mailing list
# extended to use balign if present
AC_MSG_CHECKING([if .balign is present])
echo '.balign 4' > conftest.s
if $CCAS $CCASFLAGS -c -o conftest.o conftest.s 1>/dev/null 2>&1; then

  AC_MSG_RESULT([yes])
  AC_DEFINE(ASMALIGN_BALIGN, 1, [ Define if .balign is present. ])

else

AC_MSG_RESULT([no])

# find if .align arg is power-of-two or not
asmalign_exp="unknown"
if test x"$asmalign_exp" = xunknown; then
	AC_MSG_CHECKING([if .align takes 2-exponent])
	asmalign_exp="no"
	echo '.align 3' > conftest.s
	if $CCAS $CCASFLAGS -c -o conftest.o conftest.s 1>/dev/null 2>&1; then
		asmalign_exp="yes"
		AC_MSG_RESULT([yes])
	else
		AC_MSG_RESULT([no])
	fi
	rm -f conftest.o conftest.s
fi
if test x"$asmalign_exp" = xyes; then
	AC_DEFINE(ASMALIGN_EXP, 1, [ Define if .align takes 3 for alignment of 2^3=8 bytes instead of 8. ])
else
	AC_DEFINE(ASMALIGN_BYTE, 1, [ Define if .align just takes byte count. ])
fi

fi

ccalign="unknown"
if test x"$ccalign" = xunknown; then
	AC_MSG_CHECKING([__attribute__((aligned(16)))])
	ccalign="no"
	echo '__attribute__((aligned(16))) float var;' > conftest.c
	if $CC -c -o conftest.o conftest.c >/dev/null 2>&1; then
		ccalign="yes"
		AC_MSG_RESULT([yes])
	else
		AC_MSG_RESULT([no])
	fi
	rm -f conftest.o conftest.c
fi

dnl We apply alignment hints only to cpus that need it.
dnl See further below for the definition of CCALIGN

avx_support="unknown"
if test x"$avx_support" = xunknown; then
	AC_MSG_CHECKING([if assembler supports AVX instructions])
	avx_support="no"
	echo '.text' > conftest.s
	echo 'vaddps %ymm0,%ymm0,%ymm0' >> conftest.s
	if $CCAS $CCASFLAGS -c -o conftest.o conftest.s 1>/dev/null 2>&1; then
		avx_support="yes"
		AC_MSG_RESULT([yes])
	else
		AC_MSG_RESULT([no])
	fi
	rm -f conftest.o conftest.s
fi

check_yasm=no
if test x"$avx_support" = xno || test x"$use_yasm" = xenabled; then
  check_yasm=yes
fi
if test x"$use_yasm" = xdisabled; then
  check_yasm=no
fi

if test x"$check_yasm" = xyes; then
	AC_CHECK_PROGS(YASM,[yasm],no)
	if test x"$YASM" != xno; then
		AC_MSG_CHECKING([if $YASM supports GAS syntax and AVX instructions])
		echo '.text' > conftest.s
		echo 'vaddps %ymm0,%ymm0,%ymm0' >> conftest.s
		if $YASM -pgas -rcpp -o conftest.o conftest.s 1>/dev/null 2>&1; then
			avx_support="yes"
			YASMFLAGS="-pgas -rgas -mamd64"
			AC_MSG_RESULT([yes])
		else
			AC_MSG_RESULT([no])
		fi
		rm -f conftest.o conftest.s
	else
		if test x"$use_yasm" = xenabled; then
			AC_MSG_ERROR([Yasm enforced but not found!])
		fi
	fi
else
	YASM="no"
fi

if test "x$cpu_type" = "xavx"; then
	if test "x$avx_support" != "xyes"; then
		AC_MSG_ERROR([Assembler doesn't understand AVX instructions.])
	fi
fi

AC_SUBST(YASM)
AC_SUBST(YASMFLAGS)

dnl ############## Really basic headers, needed for other checks.

AC_MSG_CHECKING([for basic C99 features])
AC_LINK_IFELSE(
   [AC_LANG_SOURCE([`cat "$srcdir/test_c99.c"`])],
   [have_c99=yes; AC_MSG_RESULT([yes])],
   [have_c99=no; AC_MSG_RESULT([no])]
)
if test "x$have_c99" = xno; then
  AC_MSG_ERROR([Your toolchain lacks basic C99 features. See test_c99.c for the requirements.])
fi

dnl Is it too paranoid to specifically check for stdint.h and limits.h?
AC_CHECK_HEADERS([unistd.h sched.h sys/ioctl.h sys/types.h sys/time.h sys/wait.h sys/resource.h sys/signal.h signal.h sys/select.h dirent.h sys/stat.h])

dnl ############## LFS stuff

portable_api=no
AC_ARG_ENABLE(portable,
              [AS_HELP_STRING( [--enable-portable], [only build portable API (no off_t, no internal I/O -- and no end-user programs)])],
              [
                if test "x$enableval" = xyes; then
                  portable_api="yes"
                fi
              ], [])
if test "x$portable_api" = xyes; then
  AC_DEFINE(PORTABLE_API, 1, [ Define to only include portable library API (no off_t, no internal I/O). ])
fi

AM_CONDITIONAL([HAVE_LFS_WRAP], [ test x"$portable_api" = xno ] )

# Check for shape-shifting off_t just for the mpg123 client application.
# At least at first, mpg123 itself will excercise the wrappers, otherwise
# we don't know they work.
# Do not use AC_CHECK_SIZEOF twice to avoid redefinition of SIZEOF_OFF_T
# in further tests via confdefs.h.
AC_MSG_CHECKING([switched off_t size])
AC_COMPUTE_INT(switched_off_size, sizeof(off_t), [
#undef  _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#include <sys/types.h>
])
AC_MSG_RESULT([$switched_off_size])
AC_MSG_CHECKING([unswitched off_t size])
AC_COMPUTE_INT(unswitched_off_size, sizeof(off_t), [
#undef  _FILE_OFFSET_BITS
#include <sys/types.h>
])
AC_MSG_RESULT([$unswitched_off_size])
# Now the size without meddling, possibly enforced to 64 bits by system settings.
AC_CHECK_SIZEOF(off_t, [], [
#include <sys/types.h>
])

# Some paranoia about the limited choices we accept.
# 32 or 64 bit offsets, an the switched one better not be smaller.
if test "x$ac_cv_sizeof_off_t" = x0; then
  AC_MSG_ERROR([Not even off_t found. I need some POSIX.])
fi
if test "x$ac_cv_sizeof_off_t" != x8 && 
   test "x$ac_cv_sizeof_off_t" != x4; then
  AC_MSG_ERROR([unexpected size of off_t])
fi

if test "x$ac_cv_sizeof_off_t" != "x$switched_off_size"; then
  if test "x$switched_off_size" != x8; then
    AC_MSG_ERROR([bad switched off_t size])
  fi
  lfs_sensitive=yes
  AC_DEFINE(LFS_SENSITIVE, 1, [ System redefines off_t when defining _FILE_OFFSET_BITS to 64. ])
else
  lfs_sensitive=no
fi

forced_off_64=no
if test "x$unswitched_off_size" != "x$ac_cv_sizeof_off_t"; then
  if test "x$ac_cv_sizeof_off_t" = x8; then
    AC_MSG_NOTICE([Detected system with enforced 64 bit offsets, dropping suffixless symbols for uncryptic ABI breakage.])
    AC_DEFINE([FORCED_OFF_64], 1, [System setup enforces 64 bit offsets where 32 bit would be native.])
    forced_off_64=yes
  fi
fi

use_largefile=auto

AC_MSG_CHECKING([if native off_t is already 64 bits])
if test "x$ac_cv_sizeof_off_t" = x8; then
  AC_MSG_RESULT([yes])
  use_largefile=no
else
  AC_MSG_RESULT([no])
fi

AC_ARG_ENABLE(largefile,
              [AS_HELP_STRING([--disable-largefile], [to not attempt to use 64 bit file offsets internally])],
              [  if test "x$enableval" = xno; then
                  use_largefile=no
                fi
              ],
              [])

# Large file support without the magic. Check for the API explicitly.
# The shape-shifting off_t has to be avoided for the library builds.
# The mpg123 application might still use it to check if the API works.
# Or we limit it to portable, off_t-less API at some point.

if test x"$use_largefile" != xno; then

# Expect usual_LARGEFILE64_SOURCE mechanics with off64_t and lseek64 present.
AC_CHECK_FUNCS([lseek64])
AC_CHECK_TYPE(off64_t, [have_off64_t=yes], [have_off64_t=no], [[
#define _LARGEFILE64_SOURCE
#include <sys/types.h>
#include <unistd.h>
]])
if test "x$have_off64_t" = xyes && test "x$ac_cv_func_lseek64" = xyes; then
  use_largefile=yes
  AC_DEFINE(LFS_LARGEFILE_64, 1, [ Define if we use _LARGEFILE64_SOURCE with off64_t and lseek64. ])
  AC_MSG_CHECKING([if we have O_LARGEFILE])
  AC_COMPILE_IFELSE([AC_LANG_SOURCE([
    #define _LARGEFILE64_SOURCE
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <unistd.h>
    #include <fcntl.h>

    int foo()
    {
      return open("foo", O_RDONLY|O_LARGEFILE);
    }
  ])],[have_o_largefile=yes],[have_o_largefile=no])
  AC_MSG_RESULT([$have_o_largefile])
  if test x"$have_o_largefile" = xyes; then
    AC_DEFINE(HAVE_O_LARGEFILE, 1, [ Define if O_LARGEFILE flag for open(2) exists. ])
  fi
else
  use_largefile=no
fi

fi # use_largefile auto or yes

dnl ############## Function Checks

AC_CHECK_FUNCS([mmap],[have_mmap=yes],[have_mmap=no])
if test "x$have_mmap" = "xno"; then
  AC_CHECK_HEADERS([sys/ipc.h sys/shm.h],[], [buffer=disabled])
  AC_CHECK_FUNCS([shmget shmat shmdt shmctl],[], [buffer=disabled])
fi

# Check if system supports termios
AC_SYS_POSIX_TERMIOS

AC_MSG_CHECKING([if we have wincon.h])
AC_COMPILE_IFELSE([AC_LANG_SOURCE([
#include <windows.h>
#include <wincon.h>
void *v = &ReadConsoleInput;
])],[ac_cv_header_wincon_h=yes],[ac_cv_header_wincon_h=no])
AC_MSG_RESULT([$ac_cv_header_wincon_h])
AS_IF([test "x$ac_cv_header_wincon_h" = "xyes"],[AC_DEFINE([HAVE_WINCON_H],[1],[Define to 1 if you have the <wincon.h> header file.])],[])

term_type=none
if test "x$ac_cv_sys_posix_termios" = "xyes"; then
  AC_DEFINE_UNQUOTED([HAVE_TERMIOS], 1, 
  [Define this if you have the POSIX termios library])
  term_type=posix
elif test "x$ac_cv_header_windows_h" = "xyes" &&
     test "x$ac_cv_header_wincon_h"  = "xyes"; then
  term_type=win32
fi

AC_CHECK_FUNCS( random setenv unsetenv )

# Check for sched_setscheduler
AC_CHECK_FUNCS( sched_setscheduler setuid getuid)

# Check for setpriority
AC_CHECK_FUNCS( setpriority )

# strtok_s for MSVCRT, not the C11 variant
AC_CHECK_FUNCS( strerror strerror_l uselocale strtok_r strtok_s)

AC_CHECK_FUNCS( setlocale nl_langinfo mbstowcs wcstombs wcswidth iswprint )

AC_CHECK_FUNCS( atoll )

AC_CHECK_FUNCS( mkfifo, [ have_mkfifo=yes ], [ have_mkfifo=no ] )

AC_CHECK_FUNCS( fork execvp, [ have_fork=yes ], [have_fork=no; break] )

AC_CHECK_FUNCS( ctermid )

dnl For Windows and OS/2.
dnl Checking for both the functions as well as the flag symbols.
dnl Taking care with _setmode() being preferred on Windows, and
dnl OS/2 providing _setmode, but not the underscored flags.
#AC_CHECK_FUNCS( _setmode setmode )
AC_LINK_IFELSE( [AC_LANG_SOURCE(
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

int main()
{
  _setmode(STDIN_FILENO, _O_BINARY);
  _setmode(STDOUT_FILENO, _O_TEXT);
  return 0;
} )], [AC_DEFINE( HAVE__SETMODE, 1, [for Win/DOS system with _setmode()] )], [] )

AC_LINK_IFELSE( [AC_LANG_SOURCE(
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

int main()
{
  setmode(STDIN_FILENO, O_BINARY);
  setmode(STDOUT_FILENO, O_TEXT);
  return 0;
} )], [AC_DEFINE( HAVE_SETMODE, 1, [for Win/DOS system with setmode()] )], [] )

dnl ############## Header and Library Checks

# locale headers
AC_CHECK_HEADERS([locale.h langinfo.h wchar.h wctype.h])
# Headers for network (http) stuff
network_internal=unknown
AC_CHECK_HEADERS([netdb.h sys/param.h sys/socket.h netinet/in.h arpa/inet.h])
if test "x$ac_cv_header_netdb_h" = "xyes"      &&
   test "x$ac_cv_header_sys_param_h" = "xyes"  &&
   test "x$ac_cv_header_sys_socket_h" = "xyes" &&
   test "x$ac_cv_header_netinet_in_h" = "xyes" &&
   test "x$ac_cv_header_arpa_inet_h"  = "xyes"; then
  have_network=yes
  network_internal=posix
else
  have_network=no
fi

dnl trying to get that socket lib settled in one line
AC_SEARCH_LIBS(gethostbyname, nsl socket network) 
dnl OK, two lines... Solaris needs -lnsl -lsocket
AC_SEARCH_LIBS(socket, socket) 

AC_CHECK_FUNCS( getaddrinfo, [ have_ipv6=yes ], [ have_ipv6=no ] )

# Checks for maths libraries.
AC_CHECK_LIB([m], [sqrt])
AC_CHECK_LIB([mx], [powf])

# attempt to make the signal stuff work... also with GENERIC - later
#if test x"$ac_cv_header_sys_signal_h" = xyes; then
#	AC_CHECK_FUNCS( sigemptyset sigaddset sigprocmask sigaction )
#	if test x"$ac_cv_func_sigemptyset" = xyes &&
#	   test x"$ac_cv_func_sigaddset" = xyes &&
#	   test x"$ac_cv_func_sigprocmask" = xyes &&
#	   test x"$ac_cv_func_sigaction" = xyes; then
#	AC_DEFINE( 
#fi

# Optionally use platform macros for byte swapping.
AC_CHECK_HEADERS([byteswap.h])

dnl ############## Choose compiler flags and CPU

# do not assume gcc here, so no flags by default
ADD_CFLAGS=""
ADD_CPPFLAGS="$sys_cppflags"
ADD_LDFLAGS=""
LIBS="$LIBS"

# Remove that if it is settled that tcsetattr on OS/2 is unsalvageable anyway.
# The only user of os2.h is the output module.
AC_CHECK_HEADERS([os2.h])

# If debugging is enabled, just enable debugging symbols.
# All other stuff enters nagging territory.
if test x"$debugging" = xenabled; then
	ADD_CFLAGS="-g"
fi
# gcc specific...
if test x"$GCC" = xyes; then
	if test x"$nagging" = xenabled; then
		ADD_CFLAGS="$ADD_CFLAGS -Wall -Werror -std=c99 -pedantic"
	fi
fi

dnl Only try the attribute_align_arg mumbo-jumbo on x86, x86-64 warns/errors out on that attribute.
dnl ...dunno even what about other architectures.
case $host in
  i?86-*)
    AC_DEFINE(ABI_ALIGN_FUN, 1, [ Define if your architecture wants/needs/can use attribute_align_arg and alignment checks. It is for 32bit x86... ])
 ;;
esac

dnl Choice of extra libmpg12 sources via lots of AM_CONDITIONALS.
dnl Those are defined after this block of CPU selection. Make sure
dnl that any new conditional gets added down there before using it
dnl here in definitions.

s_altivec="HAVE_ALTIVEC"
s_i386="HAVE_I386"
s_i486="$s_i386 HAVE_I486"
s_i586="$s_i386 HAVE_I586"
s_i586d="$s_i386 HAVE_I586_DITHER"
s_3dnow="$s_i386 HAVE_3DNOW"
s_3dnowext="$s_i386 HAVE_3DNOWEXT HAVE_MMXTAB"
s_3dnow_vintage="$s_3dnow HAVE_3DNOW_VINTAGE"
s_3dnowext_vintage="$s_3dnowext HAVE_3DNOWEXT_VINTAGE"

s_mmx="$s_i386 HAVE_MMX HAVE_MMXTAB"
s_sse_vintage="$s_i386 HAVE_MMXTAB HAVE_SSE_VINTAGE"
s_sse="$s_sse_vintage HAVE_SSE"
s_x86_64="HAVE_X86_64 HAVE_X86_64_MONO"
s_x86_64_mono_synths="HAVE_X86_64_MONO"
s_x86_64_avx="HAVE_AVX"
s_x86multi="HAVE_GETCPUFLAGS"
s_x86_64_multi="HAVE_GETCPUFLAGS_X86_64"
s_dither="HAVE_DITHER"
s_neon="HAVE_NEON"
s_neon64="HAVE_NEON64"
s_arm_multi="HAVE_GETCPUFLAGS_ARM"
s_arm="HAVE_ARM"

# SSE and newer invoke dct64 for float numbers when producing s32 or float output,
# as well as accurate rounding.
AM_CONDITIONAL([HAVE_FLOATDCT], [ test x"$integers" = xquality || test x"$int32" = xenabled || test x"$real" = xenabled ])

dnl CPU specific compiler flags and sources
case $cpu_type in
  generic)
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_GENERIC -DREAL_IS_FLOAT"
    more_sources=""
    ccalign=no
  ;;
  generic_dither)
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_GENERIC_DITHER -DREAL_IS_FLOAT"
    more_sources="$s_dither"
    ccalign=no
  ;;
  generic_fpu | generic_float)
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_GENERIC -DREAL_IS_FLOAT"
    more_sources=""
    ccalign=no
  ;;
  generic_nofpu)
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_GENERIC -DREAL_IS_FIXED"
    more_sources=
    ccalign=no
  ;;
  ppc_nofpu)
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_GENERIC -DOPT_PPC -DREAL_IS_FIXED"
    more_sources=
    ccalign=no
  ;;
  arm_nofpu)
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_ARM -DREAL_IS_FIXED"
    more_sources="$s_arm"
    ccalign=no
  ;;
  altivec)
    ADD_CFLAGS="$ADD_CFLAGS -maltivec"
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DREAL_IS_FLOAT -DOPT_ALTIVEC"
    more_sources="$s_altivec"
    AC_MSG_CHECKING([if $CC accepts -faltivec])
    touch conftest.c
    if $CC -faltivec -c -o conftest.o conftest.c >/dev/null 2>&1; then
        ADD_CFLAGS="$ADD_CFLAGS -faltivec"
        AC_MSG_RESULT([yes])
    else
        AC_MSG_RESULT([no])
    fi
    rm -f conftest.o conftest.c
  ;;
  neon)
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_NEON -DREAL_IS_FLOAT"
    more_sources="$s_neon"
  ;;
  arm_fpu)
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_MULTI -DOPT_GENERIC -DOPT_GENERIC_DITHER -DOPT_NEON -DREAL_IS_FLOAT"
    more_sources="$s_neon $s_dither $s_arm_multi"
  ;;
  neon64)
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_NEON64 -DREAL_IS_FLOAT"
    more_sources="$s_neon64"
  ;;
  aarch64)
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_MULTI -DOPT_GENERIC -DOPT_GENERIC_DITHER -DOPT_NEON64 -DREAL_IS_FLOAT"
    more_sources="$s_neon64 $s_dither $s_arm_multi"
  ;;
  i386) 
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_I386 -DREAL_IS_FLOAT"
    more_sources="$s_i386"
    ccalign=no
  ;;
  i386_fpu) 
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_I386 -DREAL_IS_FLOAT"
    more_sources="$s_i386"
    ccalign=no
  ;;
  i386_nofpu) 
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_I386 -DREAL_IS_FIXED"
    more_sources="$s_i386"
    have_cond
    ccalign=no
  ;;
  i486) 
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_I486 -DREAL_IS_FLOAT"
    more_sources="$s_i486"
    ccalign=no
  ;;
  i586) 
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_I586 -DREAL_IS_FLOAT"
    more_sources="$s_i586"
  ;;
  i586_dither) 
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_I586_DITHER -DREAL_IS_FLOAT"
    more_sources="$s_i586d $s_dither"
  ;;
  3dnow)
    # legacy 3dnow had the 3dnow paired with i586...
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_MULTI -DOPT_I586 -DOPT_3DNOW -DREAL_IS_FLOAT"
    more_sources="$s_i586 $s_3dnow $s_x86multi "
  ;;
  3dnow_vintage)
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_MULTI -DOPT_I586 -DOPT_3DNOW_VINTAGE -DREAL_IS_FLOAT"
    more_sources="$s_i586 $s_3dnow_vintage $s_x86multi "
  ;;
  3dnow_alone)
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_3DNOW -DREAL_IS_FLOAT"
    more_sources="$s_3dnow"
  ;;
  3dnowext_alone) 
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_3DNOWEXT -DREAL_IS_FLOAT"
    more_sources="$s_3dnowext"
  ;;
  3dnowext_vintage) 
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_MULTI -DOPT_3DNOW -DOPT_3DNOWEXT_VINTAGE -DREAL_IS_FLOAT"
    more_sources="$s_3dnowext_vintage $s_3dnow $s_x86multi"
  ;;
  3dnowext) 
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_MULTI -DOPT_3DNOW -DOPT_3DNOWEXT -DREAL_IS_FLOAT"
    more_sources="$s_3dnowext $s_3dnow $s_x86multi"
  ;;
  mmx_alone|mmx) 
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_MMX -DREAL_IS_FLOAT"
    more_sources="$s_mmx"
  ;;
  sse_alone|sse) 
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_SSE -DREAL_IS_FLOAT"
    more_sources="$s_sse"
  ;;
  sse_vintage) 
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_SSE_VINTAGE -DREAL_IS_FLOAT"
    more_sources="$s_sse_vintage"
  ;;
  avx) 
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_AVX -DREAL_IS_FLOAT"
    more_sources="$s_x86_64_avx $s_x86_64_mono_synths"
	if test "x$YASM" != "xno"; then
		use_yasm_for_avx="yes"
	fi
  ;;
  x86|x86_dither)
    # plain C dct36 always there, for vintage/non-vintage
    # Selection of non-/vintage sources implies that
    # $s_3dnow_vintage contains $s_3dnow,
    # $s_sse contains $s_sse_vintage.
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_MULTI -DOPT_GENERIC -DOPT_GENERIC_DITHER -DOPT_I386 -DOPT_I586 -DOPT_I586_DITHER -DOPT_MMX -DOPT_3DNOW -DOPT_3DNOW_VINTAGE -DOPT_3DNOWEXT -DOPT_3DNOWEXT_VINTAGE -DOPT_SSE -DOPT_SSE_VINTAGE -DREAL_IS_FLOAT"
    more_sources="$s_i386 $s_i586 $s_i586d $s_mmx $s_3dnow_vintage $s_3dnowext_vintage $s_sse $s_x86multi $s_dither"
  ;;
  x86-64_alone) 
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_X86_64 -DREAL_IS_FLOAT"
    more_sources="$s_x86_64"
  ;;
  x86-64|x86-64_all|x86-64_dither)
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_MULTI -DOPT_X86_64 -DOPT_GENERIC -DOPT_GENERIC_DITHER -DREAL_IS_FLOAT"
    more_sources="$s_x86_64 $s_dither $s_x86_64_multi"
	if test "x$avx_support" = "xyes"; then
		ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_AVX"
		more_sources="$more_sources $s_x86_64_avx"
		if test "x$YASM" != "xno"; then
			use_yasm_for_avx="yes"
		fi
	fi
  ;;
  *)
  	AC_MSG_ERROR([Unknown CPU type '$cpu_type'])
  ;;
esac

# Use yasm instead of the default assembler for AVX sources
if test "x$use_yasm_for_avx" = "xyes"; then
	case $host_os in
		*cygwin*|*mingw*)
			YASM_FORMAT="-f win64"
		;;
		*darwin*)
			YASM_FORMAT="-f macho"
		;;
		*)
			YASM_FORMAT="-f elf"
		;;
	esac
	AC_DEFINE(USE_YASM_FOR_AVX, 1, [Define to use yasm for assemble AVX sources.])
fi
AM_CONDITIONAL([USE_YASM_FOR_AVX], [test "x$use_yasm_for_avx" = xyes])
AC_SUBST(YASM_FORMAT)

# Check if we want feature report function.
AC_MSG_CHECKING([if we want feature report function])
feature_report=enabled
AC_ARG_ENABLE(feature_report,
	[AS_HELP_STRING([--disable-feature_report], [Disable feature report function])],
	[
		if test "x$enableval" = xno; then
		AC_MSG_RESULT([no])
		feature_report=disabled
		else
		AC_MSG_RESULT([yes])
		fi
	],
	[AC_MSG_RESULT([yes])])

AM_CONDITIONAL([HAVE_FEATURE], [test "x$feature_report" = xenabled])

# Return 0 if first arg appears in list of following arguments.
# Return 1 otherwise.
word_in_list() {
  word=$1
  shift
  # Avoid naming conflict with the outside!
  # Is local not POSIX sh?
  for given_word in "$@"
  do
    if test "x$word" = "x$given_word"; then
      return 0
    fi
  done
  return 1
}

dnl Really, having to write code again to generate configure.ac code
dnl because I cannot generate macro invocations in a shell loop?
dnl Autoconf is a bit ridiculous here. There's something to say about
dnl a build system with one language that can express the stuff, not this
dnl intermingling of m4 and shell on top of each other.
dnl for c in HAVE_ALTIVEC HAVE_I386 HAVE_I486 HAVE_I586 HAVE_I586_DITHER \
dnl   HAVE_3DNOW HAVE_3DNOWEXT HAVE_MMXTAB HAVE_3DNOW_VINTAGE HAVE_3DNOWEXT_VINTAGE \
dnl   HAVE_MMX HAVE_SSE_VINTAGE HAVE_SSE HAVE_X86_64 HAVE_X86_64_MONO HAVE_AVX \
dnl   HAVE_GETCPUFLAGS HAVE_GETCPUFLAGS_X86_64 HAVE_DITHER HAVE_NEON HAVE_NEON64 \
dnl   HAVE_GETCPUFLAGS_ARM HAVE_ARM
dnl do
dnl   echo "AM_CONDITIONAL([$c], [word_in_list $c \$more_sources])"
dnl done

AM_CONDITIONAL([HAVE_ALTIVEC], [word_in_list HAVE_ALTIVEC $more_sources])
AM_CONDITIONAL([HAVE_I386], [word_in_list HAVE_I386 $more_sources])
AM_CONDITIONAL([HAVE_I486], [word_in_list HAVE_I486 $more_sources])
AM_CONDITIONAL([HAVE_I586], [word_in_list HAVE_I586 $more_sources])
AM_CONDITIONAL([HAVE_I586_DITHER], [word_in_list HAVE_I586_DITHER $more_sources])
AM_CONDITIONAL([HAVE_3DNOW], [word_in_list HAVE_3DNOW $more_sources])
AM_CONDITIONAL([HAVE_3DNOWEXT], [word_in_list HAVE_3DNOWEXT $more_sources])
AM_CONDITIONAL([HAVE_MMXTAB], [word_in_list HAVE_MMXTAB $more_sources])
AM_CONDITIONAL([HAVE_3DNOW_VINTAGE], [word_in_list HAVE_3DNOW_VINTAGE $more_sources])
AM_CONDITIONAL([HAVE_3DNOWEXT_VINTAGE], [word_in_list HAVE_3DNOWEXT_VINTAGE $more_sources])
AM_CONDITIONAL([HAVE_MMX], [word_in_list HAVE_MMX $more_sources])
AM_CONDITIONAL([HAVE_SSE_VINTAGE], [word_in_list HAVE_SSE_VINTAGE $more_sources])
AM_CONDITIONAL([HAVE_SSE], [word_in_list HAVE_SSE $more_sources])
AM_CONDITIONAL([HAVE_X86_64], [word_in_list HAVE_X86_64 $more_sources])
AM_CONDITIONAL([HAVE_X86_64_MONO], [word_in_list HAVE_X86_64_MONO $more_sources])
AM_CONDITIONAL([HAVE_AVX], [word_in_list HAVE_AVX $more_sources])
AM_CONDITIONAL([HAVE_GETCPUFLAGS], [word_in_list HAVE_GETCPUFLAGS $more_sources])
AM_CONDITIONAL([HAVE_GETCPUFLAGS_X86_64], [word_in_list HAVE_GETCPUFLAGS_X86_64 $more_sources])
AM_CONDITIONAL([HAVE_DITHER], [word_in_list HAVE_DITHER $more_sources])
AM_CONDITIONAL([HAVE_NEON], [word_in_list HAVE_NEON $more_sources])
AM_CONDITIONAL([HAVE_NEON64], [word_in_list HAVE_NEON64 $more_sources])
AM_CONDITIONAL([HAVE_GETCPUFLAGS_ARM], [word_in_list HAVE_GETCPUFLAGS_ARM $more_sources])
AM_CONDITIONAL([HAVE_ARM], [word_in_list HAVE_ARM $more_sources])

dnl Finally insert the code switch for alignment, above cpu selection having possibly changed the setting.
if test x"$ccalign" = xyes; then
	AC_DEFINE(CCALIGN, 1, [ Define if __attribute__((aligned(16))) shall be used ])
fi

dnl ############## Output module choice

# The full list of supported modules to check, first come, first serve.
check_modules="pulse alsa tinyalsa oss coreaudio sndio sun win32 win32_wasapi os2 esd jack portaudio sdl nas arts openal"
# Only check qsa before all else on QNX.
# It would mask ALSA otherwise.
case $host in
  *-nto-qnx*)
    check_modules="qsa $check_modules"
  ;;
esac

# The final list.
output_modules=

check_forced=no
check_failed=no
if test "x$with_audio" != "x"; then
	check_modules=`echo $with_audio|tr , ' '`
	echo "Limiting outputs to build according to your preference: $check_modules"
	check_forced=yes
fi

# Dummy output module is always built alongside libout123, but not
# when just building the modules.
if test "x$build_libout123" = xyes && ! echo "$check_modules" | grep -qw dummy; then
	check_modules="$check_modules dummy"
fi

# Now go through the modules to check and do the chores.
for m in $check_modules
do
	case "$m" in
		dummy)
			AC_MSG_CHECKING([if you are too dumbing dumb for the dummy])
			AC_MSG_RESULT([no])
			output_modules="$output_modules dummy"
			HAVE_DUMMY=yes
		;;
		oss)
			AC_CHECK_HEADERS([sys/soundcard.h linux/soundcard.h machine/soundcard.h])
			if test "x${ac_cv_header_sys_soundcard_h}" = "xyes" \
				 -o "x${ac_cv_header_linux_soundcard_h}" = "xyes" \
				 -o "x${ac_cv_header_machine_soundcard_h}" = "xyes";
			then
				output_modules="$output_modules oss"
				HAVE_OSS="yes"
			else
				check_failed=yes
			fi
		;;
		tinyalsa)
			TINYALSA_LIBS="-ltinyalsa"
			# Check for ALSA
			AC_CHECK_LIB( [tinyalsa], [pcm_open],
				[ AC_CHECK_HEADER( [tinyalsa/asoundlib.h],
					[ output_modules="$output_modules tinyalsa" HAVE_TINYALSA="yes"],
					[ AC_MSG_WARN([Found TINYALSA library but NOT header files on your system]) ] )
				]
			)
			if test "x$HAVE_TINYALSA" != xyes; then
				check_failed=yes
			fi
		;;
		# QSA and ALSA are not distinguishable in these tests, need to block
		# each other and play with test order depending on platform.
		alsa)
			if test "x$HAVE_QSA" = xyes; then
				HAVE_ALSA=no
			else

			ALSA_LIBS="-lasound"
			# Check for ALSA
			AC_CHECK_LIB( [asound], [snd_pcm_open],
				[ AC_CHECK_HEADER( [alsa/asoundlib.h],
					[ output_modules="$output_modules alsa" HAVE_ALSA="yes"],
					[ AC_MSG_WARN([Found ALSA library but NOT header files on your system]) ] )
				]
			)
			if test "x$HAVE_ALSA" != xyes; then
				check_failed=yes
			fi

			fi # HAVE_QSA
		;;
		qsa)
			if test "x$HAVE_ALSA" = xyes; then
				HAVE_QSA=no
			else

			QSA_LIBS="-lasound"
			# Check for QSA
			AC_CHECK_LIB( [asound], [snd_pcm_open],
				[ AC_CHECK_HEADER( [sys/asoundlib.h],
					[ output_modules="$output_modules qsa" HAVE_QSA="yes"],
					[ AC_MSG_WARN([Found QSA library but NOT header files on your system]) ] )
				]
			)
			if test "x$HAVE_QSA" != xyes; then
				check_failed=yes
			fi

			fi # HAVE_ALSA
		;;
		jack)
			# JACK module uses semaphores, hence -lpthread.
			AC_CHECK_LIB( [pthread], [sem_post],
				[ PKG_CHECK_MODULES(JACK, jack,
					[
						output_modules="$output_modules jack"
						HAVE_JACK="yes"
						JACK_LIBS="$JACK_LIBS -lpthread"
					], [ HAVE_JACK="no" check_failed=yes ] )
				],
				[ HAVE_JACK=no check_failed=yes ]
			)
		;;
		pulse)
			PKG_CHECK_MODULES(PULSE, libpulse-simple, output_modules="$output_modules pulse" HAVE_PULSE="yes", HAVE_PULSE="no" check_failed=yes)
		;;
		esd)
			PKG_CHECK_MODULES(ESD, esound, output_modules="$output_modules esd" HAVE_ESD="yes", HAVE_ESD="no" check_failed=yes)
		;;
		portaudio)
			# Remember: This looks only insane because you chose an insane tab width!
			PORTAUDIO_LIBS="-lportaudio"
			PORTAUDIO_CFLAGS=
			case $host in
				*-*-mingw32)
					# We tested portaudio with MME
					if test "x$HAVE_PORTAUDIO" != "xyes"; then
						AC_CHECK_LIB( [portaudio], [Pa_Initialize],
							[ AC_CHECK_HEADER( [portaudio.h],
								[ output_modules="$output_modules portaudio" HAVE_PORTAUDIO_WINMM="yes" HAVE_PORTAUDIO="yes" ],
								[ AC_MSG_WARN([Found PortAudio library but NOT header files on your system]) ] )
								PORTAUDIO_LIBS="$PORTAUDIO_LIBS -lwinmm"
							],
							[ HAVE_PORTAUDIO="no"],
							[ -lwinmm ]
						)
					fi
				;;
				*)
					AC_CHECK_LIB( [portaudio], [Pa_Initialize],
						[ AC_CHECK_HEADER( [portaudio.h],
							[ output_modules="$output_modules portaudio" HAVE_PORTAUDIO="yes" ],
							[ AC_MSG_WARN([Found PortAudio library but NOT header files on your system]) ] )
						]
					)
				;;
			esac
			if test "x$HAVE_PORTAUDIO" != xyes; then
				check_failed=yes
			else
				# See if we have v19 or v18
				AC_CHECK_LIB( [portaudio], [Pa_GetVersion], [:], [AC_DEFINE( [PORTAUDIO18], 1, [Define if portaudio v18 API is wanted.]) ], [$PORTAUDIO_LIBS] )
			fi
		;;
		sdl)
			PKG_CHECK_MODULES(SDL, sdl2, output_modules="$output_modules sdl" HAVE_SDL="yes", HAVE_SDL="no" check_failed=yes)
			if ! test "$HAVE_SDL" = "yes"; then
				check_failed=no
				PKG_CHECK_MODULES(SDL, sdl, output_modules="$output_modules sdl" HAVE_SDL="yes", HAVE_SDL="no" check_failed=yes)
			fi
		;;
		nas)
			NAS_LIBS=-laudio
			AC_CHECK_LIB( [audio], [AuOpenServer],
				[ AC_CHECK_HEADER( [audio/audiolib.h],
					[ output_modules="$output_modules nas" HAVE_NAS="yes"],
					[ AC_MSG_WARN([Found NAS library but NOT header files on your system]) ] )
				]
			)
			if test "x$HAVE_NAS" != xyes; then
				check_failed=yes
			fi
		;;
		win32)
			# Check for windows ... and win32 audio
			# Does not work... instead just check for header
			# AC_CHECK_LIB( [winmm], [waveOutOpen] )
			WIN32_LIBS=-lwinmm
			if test x$ac_cv_header_windows_h = xyes; then
				output_modules="$output_modules win32"
				HAVE_WIN32=yes
			else
				HAVE_WIN32=no
				check_failed=yes
			fi
		;;
		win32_wasapi)
			# Check for windows ... and win32 wasapi audio
			# Does not work... instead just check for header
			# AC_CHECK_LIB( [avrt], [] )
            AC_MSG_CHECKING([if we have wasapi headers])
            OLD_LIBS=$LIBS
            LIBS="$LIBS -lole32 -lavrt"
            AC_LINK_IFELSE([AC_LANG_SOURCE([
#define COBJMACROS 1
#define _WIN32_WINNT 0x601
#include <initguid.h>
#include <audioclient.h>
#include <mmdeviceapi.h>
#include <avrt.h>
int main(){
  /* UUID Checks */
  GUID *IDs[] = {
    &CLSID_MMDeviceEnumerator,
    &IID_IMMDeviceEnumerator,
    &IID_IAudioClient,
    &IID_IAudioRenderClient
  };
  return 0;
}
    ])], [HAVE_WIN32_WASAPI=yes], [HAVE_WIN32_WASAPI=no])
			LIBS=$OLD_LIBS
			WIN32_WASAPI_LIBS="-lole32 -lavrt"
			AC_MSG_RESULT([$HAVE_WIN32_WASAPI])
			if test "x$HAVE_WIN32_WASAPI" = xyes; then
				output_modules="$output_modules win32_wasapi"
			else
				check_failed=yes
			fi
		;;
		sndio)
			PKG_CHECK_MODULES(SNDIO, sndio, output_modules="$output_modules sndio" HAVE_SNDIO="yes", HAVE_SNDIO="no" check_failed=yes)
		;;
		sun)
			AC_CHECK_HEADERS([sun/audioio.h sys/audioio.h asm/audioio.h sys/audio.h])
			if test "x${ac_cv_header_sun_audioio_h}" = "xyes" \
				 -o "x${ac_cv_header_sys_audioio_h}" = "xyes" \
				 -o "x${ac_cv_header_asm_audioio_h}" = "xyes";
			then
				output_modules="$output_modules sun"
				HAVE_SUN="yes"
			else
				check_failed=yes
			fi
		;;
		coreaudio)
			COREAUDIO_LIBS="-framework AudioToolbox"
			AC_CHECK_HEADERS([AudioUnit/AudioUnit.h AudioToolbox/AudioToolbox.h])
			if test "x${ac_cv_header_AudioUnit_AudioUnit_h}" = "xyes" \
				 -a "x${ac_cv_header_AudioToolbox_AudioToolbox_h}" = "xyes";
			then
				OLD_LIBS=$LIBS
				LIBS="$LIBS $COREAUDIO_LIBS"
    			AC_MSG_CHECKING([if AudioUnit functions are available within AudioToolbox])
				AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <AudioUnit/AudioUnit.h>]], [[AudioUnitInitialize(NULL);]])],[AC_MSG_RESULT([yes])],[AC_MSG_RESULT([no])
					 COREAUDIO_LIBS="$COREAUDIO_LIBS -framework AudioUnit"
				])
				LIBS=$OLD_LIBS
				AC_CHECK_HEADERS([CoreServices/CoreServices.h],
					[COREAUDIO_LIBS="$COREAUDIO_LIBS -framework CoreServices"]
				)
				if test x"$modules" = xdisabled; then
					AC_MSG_WARN([Disabling buffer because of directly linked CoreAudio! Use the module if you need the buffer.])
					buffer=disabled
				fi
				output_modules="$output_modules coreaudio"
				HAVE_COREAUDIO="yes"
			else
				check_failed=yes
			fi
		;;
		arts)
			AC_MSG_CHECKING([for artsc])
			if artsc-config > /dev/null 2>&1; then
				AC_MSG_RESULT([yes])
				output_modules="$output_modules arts"
				HAVE_ARTS=yes
				ARTS_LIBS=`artsc-config --libs`
				ARTS_CFLAGS=`artsc-config --cflags`
			else
				AC_MSG_RESULT([no])
				check_failed=yes
			fi
		;;
		openal)
			AC_CHECK_HEADERS([OpenAL/al.h OpenAL/alc.h AL/al.h AL/alc.h al.h alc.h])
			if test "x${ac_cv_header_OpenAL_al_h}" = "xyes" \
				 -a "x${ac_cv_header_OpenAL_alc_h}" = "xyes";
			then #Mac OS X
				output_modules="$output_modules openal"
				OPENAL_LIBS="-framework OpenAL"
				OPENAL_CFLAGS="-DOPENAL_SUBDIR_OPENAL"
				HAVE_OPENAL="yes"
			elif test "x${ac_cv_header_AL_al_h}" = "xyes" \
				 -a "x${ac_cv_header_AL_alc_h}" = "xyes";
			then #Linux
				output_modules="$output_modules openal"
				OPENAL_LIBS="-lopenal"
				OPENAL_CFLAGS="-DOPENAL_SUBDIR_AL"
				HAVE_OPENAL="yes"
			elif test "x${ac_cv_header_al_h}" = "xyes" \
				 -a "x${ac_cv_header_alc_h}" = "xyes";
			then #Windows?
				output_modules="$output_modules openal"
				OPENAL_LIBS="-lopenal"
				OPENAL_CFLAGS=""
				HAVE_OPENAL="yes"
			else
				check_failed=yes
			fi
		;;
		os2)
			OS2_LIBS="-lcx -lmmpm2"
			OS2_CFLAGS="-idirafter /@unixroot/usr/include/os2tk45"
			oldcflags="$CFLAGS"
			CFLAGS="$CFLAGS $OS2_CFLAGS"
			AC_CHECK_HEADERS([os2.h])
			# os2me.h depends on os2.h
			# Yes, that way of coding it is ugly.
			if test "x${ac_cv_header_os2_h}" = xyes
			then
				# We mimick exactly the way how the header will be used.
				# It seems to be picky...
				AC_CHECK_HEADERS([os2me.h], [], [], [#define INCL_OS2MM
#undef VERSION
#define INCL_DOS
#define INCL_VIO
#define INCL_KBD
#include <os2.h>
#])
			fi
			if test "x${ac_cv_header_os2_h}" = xyes \
			   -a   "x${ac_cv_header_os2me_h}" = xyes
			then
				output_modules="$output_modules os2"
				HAVE_OS2=yes
			else
				check_failed=yes
			fi
			CFLAGS="$oldcflags"
		;;
# from here on only forced tests, untested code
		hp)
			# What's the deal with that and alib?
			UNSUPPORTED_AUDIO=yes
			AC_CHECK_HEADER([sys/audio.h], [output_modules="$output_modules hp" HAVE_HP=yes], [check_failed=yes])
		;;
		alib)
			UNSUPPORTED_AUDIO=yes
			# ALIB_CFLAGS="-I/opt/audio/include"
			ALIB_LIBS=-lAlib
			# These headers may not be all about audio but they are used.
			AC_CHECK_HEADERS([ Alib.h CUlib.h netdb.h netinet/in.h netinet/tcp.h])
			if test "x${ac_cv_header_Alib_h}" = xyes \
			   -a   "x${ac_cv_header_CUlib_h}" = xyes \
			   -a   "x${ac_cv_header_netdb_h}" = xyes \
			   -a   "x${ac_cv_header_netinet_in_h}" = xyes \
			   -a   "x${ac_cv_header_netinet_tcp_h}" = xyes
			then
				output_modules="$output_modules alib"
				HAVE_ALIB=yes
			else
				check_failed=yes
			fi
		;;
		mint)
			UNSUPPORTED_AUDIO=yes
			AC_CHECK_HEADERS([audios.h], [output_modules="$output_modules mint" HAVE_MINT=yes], [check_failes=yes])
		;;
		aix)
			UNSUPPORTED_AUDIO=yes
			AC_CHECK_HEADERS([sys/audio.h], [output_modules="$output_modules aix" HAVE_AIX=yes], [check_failed=yes])
		;;
		sgi)
			UNSUPPORTED_AUDIO=yes
			SGI_LIBS=-laudio
			AC_CHECK_HEADER([dmedia/audio.h], [output_modules="$output_modules sgi" HAVE_SGI=yes], [check_failed=yes])
		;;
		*)
			AC_MSG_ERROR([Unsupported/-known output '$m' demanded!])
		;;
	esac
done

if test "x$check_forced" = xyes -a "x$UNSUPPORTED_AUDIO" = xyes; then
	AC_MSG_WARN([You requested bulding of an unsupported audio module. Be prepared for happy hacking and please tell us about your experience!])
fi

if test "x$check_forced" = xyes -a "x$check_failed" = "xyes"; then
	AC_MSG_ERROR([One/some of your requested audio modules failed the test!])
fi

# When you extend check_modules, you should extend this:
#for i in alsa qsa oss coreaudio sndio sun win32 win32_wasapi esd jack portaudio pulse sdl nas aix alib arts hp os2 sgi mint openal dummy
#do echo $i; done |
#perl -ne 'chomp; $big = uc($_); print <<EOT;
#AC_SUBST(${big}_LIBS)
#AC_SUBST(${big}_LDFLAGS)
#AC_SUBST(${big}_CFLAGS)
#AM_CONDITIONAL( [HAVE_$big], [test "x\$HAVE_$big" = xyes] )
#EOT
#'
AC_SUBST(TINYALSA_LIBS)
AC_SUBST(TINYALSA_LDFLAGS)
AC_SUBST(TINYALSA_CFLAGS)
AM_CONDITIONAL( [HAVE_TINYALSA], [test "x$HAVE_TINYALSA" = xyes] )
AC_SUBST(ALSA_LIBS)
AC_SUBST(ALSA_LDFLAGS)
AC_SUBST(ALSA_CFLAGS)
AM_CONDITIONAL( [HAVE_ALSA], [test "x$HAVE_ALSA" = xyes] )
AC_SUBST(QSA_LIBS)
AC_SUBST(QSA_LDFLAGS)
AC_SUBST(QSA_CFLAGS)
AM_CONDITIONAL( [HAVE_QSA], [test "x$HAVE_QSA" = xyes] )
AC_SUBST(OSS_LIBS)
AC_SUBST(OSS_LDFLAGS)
AC_SUBST(OSS_CFLAGS)
AM_CONDITIONAL( [HAVE_OSS], [test "x$HAVE_OSS" = xyes] )
AC_SUBST(COREAUDIO_LIBS)
AC_SUBST(COREAUDIO_LDFLAGS)
AC_SUBST(COREAUDIO_CFLAGS)
AM_CONDITIONAL( [HAVE_COREAUDIO], [test "x$HAVE_COREAUDIO" = xyes] )
AC_SUBST(SNDIO_LIBS)
AC_SUBST(SNDIO_LDFLAGS)
AC_SUBST(SNDIO_CFLAGS)
AM_CONDITIONAL( [HAVE_SNDIO], [test "x$HAVE_SNDIO" = xyes] )
AC_SUBST(SUN_LIBS)
AC_SUBST(SUN_LDFLAGS)
AC_SUBST(SUN_CFLAGS)
AM_CONDITIONAL( [HAVE_SUN], [test "x$HAVE_SUN" = xyes] )
AC_SUBST(WIN32_LIBS)
AC_SUBST(WIN32_LDFLAGS)
AC_SUBST(WIN32_CFLAGS)
AM_CONDITIONAL( [HAVE_WIN32], [test "x$HAVE_WIN32" = xyes] )
AC_SUBST(WIN32_WASAPI_LIBS)
AC_SUBST(WIN32_WASAPI_LDFLAGS)
AC_SUBST(WIN32_WASAPI_CFLAGS)
AM_CONDITIONAL( [HAVE_WIN32_WASAPI], [test "x$HAVE_WIN32_WASAPI" = xyes] )
AC_SUBST(ESD_LIBS)
AC_SUBST(ESD_LDFLAGS)
AC_SUBST(ESD_CFLAGS)
AM_CONDITIONAL( [HAVE_ESD], [test "x$HAVE_ESD" = xyes] )
AC_SUBST(JACK_LIBS)
AC_SUBST(JACK_LDFLAGS)
AC_SUBST(JACK_CFLAGS)
AM_CONDITIONAL( [HAVE_JACK], [test "x$HAVE_JACK" = xyes] )
AC_SUBST(PORTAUDIO_LIBS)
AC_SUBST(PORTAUDIO_LDFLAGS)
AC_SUBST(PORTAUDIO_CFLAGS)
AM_CONDITIONAL( [HAVE_PORTAUDIO], [test "x$HAVE_PORTAUDIO" = xyes] )
AC_SUBST(PULSE_LIBS)
AC_SUBST(PULSE_LDFLAGS)
AC_SUBST(PULSE_CFLAGS)
AM_CONDITIONAL( [HAVE_PULSE], [test "x$HAVE_PULSE" = xyes] )
AC_SUBST(SDL_LIBS)
AC_SUBST(SDL_LDFLAGS)
AC_SUBST(SDL_CFLAGS)
AM_CONDITIONAL( [HAVE_SDL], [test "x$HAVE_SDL" = xyes] )
AC_SUBST(NAS_LIBS)
AC_SUBST(NAS_LDFLAGS)
AC_SUBST(NAS_CFLAGS)
AM_CONDITIONAL( [HAVE_NAS], [test "x$HAVE_NAS" = xyes] )
AC_SUBST(AIX_LIBS)
AC_SUBST(AIX_LDFLAGS)
AC_SUBST(AIX_CFLAGS)
AM_CONDITIONAL( [HAVE_AIX], [test "x$HAVE_AIX" = xyes] )
AC_SUBST(ALIB_LIBS)
AC_SUBST(ALIB_LDFLAGS)
AC_SUBST(ALIB_CFLAGS)
AM_CONDITIONAL( [HAVE_ALIB], [test "x$HAVE_ALIB" = xyes] )
AC_SUBST(ARTS_LIBS)
AC_SUBST(ARTS_LDFLAGS)
AC_SUBST(ARTS_CFLAGS)
AM_CONDITIONAL( [HAVE_ARTS], [test "x$HAVE_ARTS" = xyes] )
AC_SUBST(HP_LIBS)
AC_SUBST(HP_LDFLAGS)
AC_SUBST(HP_CFLAGS)
AM_CONDITIONAL( [HAVE_HP], [test "x$HAVE_HP" = xyes] )
AC_SUBST(OS2_LIBS)
AC_SUBST(OS2_LDFLAGS)
AC_SUBST(OS2_CFLAGS)
AM_CONDITIONAL( [HAVE_OS2], [test "x$HAVE_OS2" = xyes] )
AC_SUBST(SGI_LIBS)
AC_SUBST(SGI_LDFLAGS)
AC_SUBST(SGI_CFLAGS)
AM_CONDITIONAL( [HAVE_SGI], [test "x$HAVE_SGI" = xyes] )
AC_SUBST(MINT_LIBS)
AC_SUBST(MINT_LDFLAGS)
AC_SUBST(MINT_CFLAGS)
AM_CONDITIONAL( [HAVE_MINT], [test "x$HAVE_MINT" = xyes] )
AC_SUBST(OPENAL_LIBS)
AC_SUBST(OPENAL_LDFLAGS)
AC_SUBST(OPENAL_CFLAGS)
AM_CONDITIONAL( [HAVE_OPENAL], [test "x$HAVE_OPENAL" = xyes] )
AC_SUBST(DUMMY_LIBS)
AC_SUBST(DUMMY_LDFLAGS)
AC_SUBST(DUMMY_CFLAGS)
AM_CONDITIONAL( [HAVE_DUMMY], [test "x$HAVE_DUMMY" = xyes] )
# Hackery to get rid of module .la files.

output_modules=`echo $output_modules`
AC_SUBST(output_modules)
for f in $output_modules
do
  output_modules_la="$output_modules_la output_$f.la"
done
AC_SUBST(output_modules_la)

list_without()
{
  lw_word=$1
  lw_spc=
  for w in "$@"
  do
    if test x"$w" != x"$lw_word"; then
      printf '%s%s' "$lw_spc" "$w"
      lw_spc=' '
    fi
  done
}

first_word()
{
  echo $1
}

# Did user choose default audio subsystem ?
# That's (beginning of) the list for mpg123's internal default.
if test "x$with_default_audio" != "x"; then
	default_output_modules=`echo "$with_default_audio" | tr , ' '`
else
	default_output_modules=`list_without dummy $output_modules`
fi

default_output_module=`first_word $default_output_modules`

# Without actual modules, there is only the builtin one.
if test x"$modules" = xdisabled; then
  # Module-less build needs _some_ default module.
  if test -z "$default_output_module"; then
    default_output_module=dummy
  fi
  default_output_modules=$default_output_module
fi

# Setup the static build.
# The conditionals always need to be defined by configure, even if
# HAVE_MODULES is FALSE!
# Here's a script for that tedious list, perhaps to be outsourced together with the one in #src/output/Makefile.am
#for i in dummy tinyalsa alsa qsa coreaudio esd jack nas oss portaudio pulse sdl sndio sun win32 win32_wasapi aix alib arts hp os2 sgi mint openal
#do echo $i; done |
#perl -ne 'chomp; $big = uc($_); print <<EOT;
#AM_CONDITIONAL([BUILD_${big}], [ test "$_" = "\$default_output_module" ])
#EOT
#'

AM_CONDITIONAL([BUILD_DUMMY], [ test "dummy" = "$default_output_module" ])
AM_CONDITIONAL([BUILD_TINYALSA], [ test "tinyalsa" = "$default_output_module" ])
AM_CONDITIONAL([BUILD_ALSA], [ test "alsa" = "$default_output_module" ])
AM_CONDITIONAL([BUILD_QSA], [ test "qsa" = "$default_output_module" ])
AM_CONDITIONAL([BUILD_COREAUDIO], [ test "coreaudio" = "$default_output_module" ])
AM_CONDITIONAL([BUILD_ESD], [ test "esd" = "$default_output_module" ])
AM_CONDITIONAL([BUILD_JACK], [ test "jack" = "$default_output_module" ])
AM_CONDITIONAL([BUILD_NAS], [ test "nas" = "$default_output_module" ])
AM_CONDITIONAL([BUILD_OSS], [ test "oss" = "$default_output_module" ])
AM_CONDITIONAL([BUILD_PORTAUDIO], [ test "portaudio" = "$default_output_module" ])
AM_CONDITIONAL([BUILD_PULSE], [ test "pulse" = "$default_output_module" ])
AM_CONDITIONAL([BUILD_SDL], [ test "sdl" = "$default_output_module" ])
AM_CONDITIONAL([BUILD_SNDIO], [ test "sndio" = "$default_output_module" ])
AM_CONDITIONAL([BUILD_SUN], [ test "sun" = "$default_output_module" ])
AM_CONDITIONAL([BUILD_WIN32], [ test "win32" = "$default_output_module" ])
AM_CONDITIONAL([BUILD_WIN32_WASAPI], [ test "win32_wasapi" = "$default_output_module" ])
AM_CONDITIONAL([BUILD_AIX], [ test "aix" = "$default_output_module" ])
AM_CONDITIONAL([BUILD_ALIB], [ test "alib" = "$default_output_module" ])
AM_CONDITIONAL([BUILD_ARTS], [ test "arts" = "$default_output_module" ])
AM_CONDITIONAL([BUILD_HP], [ test "hp" = "$default_output_module" ])
AM_CONDITIONAL([BUILD_OS2], [ test "os2" = "$default_output_module" ])
AM_CONDITIONAL([BUILD_SGI], [ test "sgi" = "$default_output_module" ])
AM_CONDITIONAL([BUILD_MINT], [ test "mint" = "$default_output_module" ])
AM_CONDITIONAL([BUILD_OPENAL], [ test "openal" = "$default_output_module" ])

default_modstring=`echo "$default_output_modules"|tr ' ' ,`
AC_DEFINE_UNQUOTED( DEFAULT_OUTPUT_MODULE, "$default_modstring", [The default audio output module(s) to use] )

dnl ############## Compiler Optimizations

CFLAGS="$ADD_CFLAGS $CFLAGS"
AM_CONDITIONAL([BUILD_BUFFER], [ test x"$buffer" = xenabled ])
if test x"$buffer" = xdisabled; then
	ADD_CPPFLAGS="$ADD_CPPFLAGS -DNOXFERMEM"
fi
if test x"$newoldwritesample" = xenabled; then
	ADD_CPPFLAGS="$ADD_CPPFLAGS -DNEWOLD_WRITE_SAMPLE"
fi
CPPFLAGS="$ADD_CPPFLAGS $CPPFLAGS"
LDFLAGS="$ADD_LDFLAGS $LDFLAGS"

# None chosen?
if test "x$with_optimization" = "x"; then
	if test x"$debugging" = xenabled; then
		with_optimization="0"
	else
		# enable (gcc specific) default opts only with gcc
		if test "x$GCC" = xyes; then
			with_optimization="2"
		else
			with_optimization="0"
		fi
	fi
fi

case $with_optimization in
  0)
    # No Optimizations
    CFLAGS="$CFLAGS"
  ;;
  1)
    CFLAGS="-O $CFLAGS"
  ;;
  2)
    CFLAGS="-O2 -fomit-frame-pointer -funroll-all-loops -finline-functions -ffast-math $CFLAGS"
  ;;
  3)
    CFLAGS="-O3 -fomit-frame-pointer -funroll-all-loops -finline-functions -ffast-math $CFLAGS"
  ;;
  4)
    CFLAGS="-O4 -fomit-frame-pointer -funroll-all-loops -finline-functions -ffast-math $CFLAGS"
  ;;
  *)
  	AC_MSG_ERROR([Unknown optimizations level '$with_optimization'])
  ;;
esac

dnl If using gcc, prevent a bad alignment option from breaking things.
dnl Let's default to safe 16-byte alignment for any special x86 or altivec stuff.
if test "x$GCC" = xyes && echo "$CFLAGS" | $GREP 'mpreferred-stack-boundary=' > /dev/null; then
	case $cpu_type in
		x86 | i586* | mmx | sse | 3dnow* | altivec)
			AC_MSG_WARN( [ You are trying to mess with stack alignment. I know better. ] )
			CFLAGS="$CFLAGS -mpreferred-stack-boundary=4"
		;;
	esac
fi

dnl ############## Seektable size

if test "x$with_seektable" = "x"; then
	seektable=1000
else
	seektable=$with_seektable
fi

if test "$seektable" -gt 0; then
	AC_DEFINE(FRAME_INDEX, 1, [ Define if frame index should be used. ])
fi
# Define that always... also without frame index enabled.
AC_DEFINE_UNQUOTED(INDEX_SIZE, $seektable, [size of the frame index seek table])

dnl ############## Win32 function checks
# Check if we want Unicode for Win32. Cygwin and Midipix does not need _wopen
COMPAT_LIBS=

win32_unicode=unneeded
win32_sockets=disabled
win32_sockets_working=no
win32_wide_working=no
win32_winver_bump=no

### mingw.org may not work properly for newer APIs
case $host_os in
  *mingw*)
    AC_CHECK_HEADER([_mingw_mac.h], [], [
      AC_MSG_WARN(You are using the mingw.org distribution)
      AC_MSG_WARN(mpg123 may not build properly)
      AC_MSG_WARN(Please switch to using mingw-w64 if possible)
    ])
  ;;
esac

dnl We do not support non-unicode Windows.
if test "x$win32_specific_codes" = xenabled; then
#### Check for Wide functions
  AC_CHECK_FUNC([_wopen], [win32_unicode=enabled],[win32_unicode=disabled])
  if test "x$uwp_build" = xyes; then
    AC_DEFINE([WANT_WIN32_UNICODE], [1], [ Define to use Unicode for Windows ])
  else
  AC_MSG_CHECKING([if we want Unicode File Open for Win32])
  if test "x$win32_unicode" = xenabled; then
    dnl We need to include the header for PathCombineW checking as
    dnl the actual symbol has a very funny name.
    oldlibs=$LIBS
    LIBS="$LIBS -lshlwapi"
    AC_MSG_RESULT([yes])
    AC_MSG_CHECKING([if Unicode functions working])
    AC_LINK_IFELSE([AC_LANG_SOURCE([
      #include <windows.h>
      #include <shlwapi.h>
      #include <stdlib.h>
      int main()
      {
        MultiByteToWideChar (0, 0, NULL, 0, NULL, 0);
        WideCharToMultiByte (0, 0, NULL, 0, NULL, 0, NULL, NULL);
        PathCombineW(0,0,0);
        return 0;
      }
    ])], [win32_wide_working=yes], [win32_winver_bump=yes])
    if test "x$win32_wide_working" = xno; then
      AC_LINK_IFELSE([AC_LANG_SOURCE([
        #define WINVER 0x501
        #define _WIN32_WINNT 0x501
        #include <windows.h>
        #include <shlwapi.h>
        #include <stdlib.h>
        int main()
        {
          MultiByteToWideChar (0, 0, NULL, 0, NULL, 0);
          WideCharToMultiByte (0, 0, NULL, 0, NULL, 0, NULL, NULL);
          PathCombineW(0,0,0);
          return 0;
        }
      ])], [win32_wide_working=yes], [AC_MSG_RESULT([no])])
    fi
    LIBS=$oldlibs
    if test "x$win32_wide_working" = xyes; then
      COMPAT_LIBS="-lshlwapi $COMPAT_LIBS"
      AC_MSG_RESULT([yes])
      AC_DEFINE([WANT_WIN32_UNICODE], [1], [ Define to use Unicode for Windows ])
    else
      AC_MSG_ERROR([Unicode support for Win32 not working])
    fi
  else
    AC_MSG_ERROR([Unicode File Open for Win32 not available])
  fi
  fi

#### Check for Network functions
  AC_CHECK_HEADERS([ws2tcpip.h], [win32_sockets=enabled], [AC_MSG_WARN([Please update your headers to support winsock 2.2.])])
  AC_MSG_CHECKING([if we want Win32 sockets])
  if test "x$win32_sockets" = "xenabled"; then
  AC_MSG_RESULT([yes])
  AC_MSG_CHECKING([if winsock2 API is available])
    wsoldlibs="$LIBS"
    LIBS="$LIBS -lws2_32"
    AC_LINK_IFELSE([AC_LANG_SOURCE([
      #include <winsock2.h>
      #include <ws2tcpip.h>
      #include <stdlib.h>
      int main()
      {
        getaddrinfo(NULL, NULL, NULL, NULL);
        freeaddrinfo(NULL);
        return 0;
      }
    ])], [win32_sockets_working=yes], [win32_winver_bump=yes])
    if test "x$win32_sockets_working" = "xno"; then
      AC_MSG_CHECKING([deeper if winsock2 API is available])
      AC_LINK_IFELSE([AC_LANG_SOURCE([
        #define WINVER 0x501
        #define _WIN32_WINNT 0x501
        #include <winsock2.h>
        #include <ws2tcpip.h>
        #include <stdlib.h>
        int main()
        {
          getaddrinfo(NULL, NULL, NULL, NULL);
          freeaddrinfo(NULL);
          return 0;
        }
      ])], [win32_sockets_working=yes], [AC_MSG_RESULT([no])])
    fi
    if test "x$win32_sockets_working" = "xyes"; then
      AC_MSG_RESULT([yes])
      network_internal=winsock2
      have_network=yes
      have_ipv6=yes
    else
      AC_MSG_WARN([Please update your headers to support winsock 2.2.])
    fi
      LIBS="$wsoldlibs"
  else
    AC_MSG_RESULT([no])
  fi

#### Check for Win32 Named Pipe functions
win32_fifo_working=no
winver=""
if test x$win32_specific_codes = xenabled; then
  AC_MSG_CHECKING([if we have Named Pipes])
  if test "x$fifo" != "xdisabled"; then
    AC_LINK_IFELSE([AC_LANG_SOURCE([
    #include <windows.h>
      int main(){
        CreateNamedPipeA(NULL,PIPE_ACCESS_DUPLEX|FILE_FLAG_OVERLAPPED,PIPE_TYPE_BYTE,1,255,255,0,NULL);
        CreateNamedPipeW(NULL,PIPE_ACCESS_DUPLEX|FILE_FLAG_OVERLAPPED,PIPE_TYPE_BYTE,1,255,255,0,NULL);
        return 0;
      }])],[win32_fifo_working=yes],[win32_winver_bump=yes])
    if test "x$win32_fifo_working" != "xyes"; then
    AC_LINK_IFELSE([AC_LANG_SOURCE([
    #define WINVER 0x501
    #define _WIN32_WINNT 0x501
    #include <windows.h>
      int main(){
        CreateNamedPipeA(NULL,PIPE_ACCESS_DUPLEX|FILE_FLAG_OVERLAPPED,PIPE_TYPE_BYTE,1,255,255,0,NULL);
        CreateNamedPipeW(NULL,PIPE_ACCESS_DUPLEX|FILE_FLAG_OVERLAPPED,PIPE_TYPE_BYTE,1,255,255,0,NULL);
        return 0;
      }])],[win32_fifo_working=yes],[win32_fifo_working=no])
    fi
  fi
  have_mkfifo=$win32_fifo_working
  if test "x$win32_fifo_working" = "xyes"; then
    AC_MSG_RESULT([yes])
    AC_DEFINE([WANT_WIN32_FIFO], [1], [ Define to use Win32 named pipes ])
  else
    AC_MSG_RESULT([no])
  fi
fi

#### WINVER Bump
if test x$win32_specific_codes = xenabled; then
  AC_MSG_CHECKING([if WINVER and _WIN32_WINNT needs version bumps])
  if test "x$win32_winver_bump" = "xyes"; then
    AC_MSG_RESULT([yes])
    AC_DEFINE([WINVER], [0x0600], [ Windows Vista and later APIs ])
    AC_DEFINE([_WIN32_WINNT], [0x0600], [ Windows Vista and later APIs ])
    # getaddrinfo and freeaddrinfo are available for Win2K and above
    # Bug: MinGW.org w32api ws2tcpip.h incorrectly wants (_WIN32_WINNT >= 0x0501) for getaddrinfo and freeaddrinfo
    # MultiByteToWideChar and WideCharToMultiByte are available for Win2K and above
    # GetThreadErrorMode only available on Vista and Later
  else
    AC_MSG_RESULT([no])
  fi
fi

fi #END OF WIN32 CHECKS

#### Check mingw.org for EOVERFLOW
AC_MSG_CHECKING([if we have EOVERFLOW macro])
AC_COMPILE_IFELSE([AC_LANG_SOURCE([
#include <errno.h>
int i = (EOVERFLOW) + 0;
])],[eoverflow_present=yes],[eoverflow_present=no])
AC_MSG_RESULT([$eoverflow_present])
AS_IF([test "x$eoverflow_present" = "xyes"],[],[AC_DEFINE([EOVERFLOW],[EFBIG],[Use EFBIG as substitude for EOVERFLOW, mingw.org may lack the latter])])

# old network choice

AC_ARG_ENABLE(ipv6,
              [AS_HELP_STRING( [--enable-ipv6], [IPv6 support in internal network stack])],
              [
                if test "x$enableval" = xyes
                then
                  ipv6="enabled"
                else
                  ipv6="disabled"
                fi
              ],
              [
                ipv6="auto"
              ]
)

dnl make it clear that auto is the default for both enable and with options.
network_type=auto

dnl This is to be kept in sync with --with-network, it is a shortcut that
dnl keep the old semantics of being able to --disable-network support altogether.
dnl The default should match the --with-network default.
AC_ARG_ENABLE(network,
              [AS_HELP_STRING( [--enable-network], [network support (http streams / webradio), if available (overruled by --with-network!)])],
              [
                if test "x$enableval" = xyes
                then
                  network_type=auto
                else
                  network_type=disabled
                fi
              ],
              [
              ]
)

# new network choice

AC_MSG_CHECKING([for preferred network support])
AC_ARG_WITH([network],
  [AS_HELP_STRING([--with-network=<type>],
[Available options, depending on platform, are auto, none, internal, winhttp, wininet (or wininethttp for both), and exec (wget or curl binaries).
The internal code is always built in addition to external options for plain HTTP (esp. Shoutcast v1) support.
The external option is for HTTPS by default, but can be used for HTTP, too.])],
  [
    case "$withval" in
      exec)
        network_type="exec"
        AC_MSG_RESULT([exec])
        ;;
      winhttp)
        network_type="winhttp"
        AC_MSG_RESULT([winhttp])
        ;;
      wininet)
        network_type="wininet"
        AC_MSG_RESULT([wininet])
        ;;
      wininethttp)
        network_type="wininethttp"
        AC_MSG_RESULT([wininethttp])
        ;;
      internal)
        network_type="internal"
        AC_MSG_RESULT([internal])
        ;;
      yes|auto)
        network_type="auto"
        AC_MSG_RESULT([auto])
        ;;
      no|none)
        network_type="disabled"
        AC_MSG_RESULT([none])
        ;;
      *)
        AC_MSG_ERROR([Unknown network option "$withval"])
        ;;
     esac
  ],
  [
    AC_MSG_RESULT([$network_type])
  ]
)

if test x$network_type = xauto; then
  AC_MSG_CHECKING([for automatic network support])
  if test "x$have_fork" = "xyes"; then
    network_type=exec
  elif test "x$win32_specific_codes" = "xenabled"; then
    network_type=wininethttp
  elif test "x$have_network" = "xyes"; then
    network_type="internal"
  else
    network_type="disabled"
  fi
  AC_MSG_RESULT([$network_type])
fi

# new net123
if test x$network_type != xdisabled; then
  AC_DEFINE([NET123], [1], [ Define to for new net123 network stack. ])
fi
if test x$network_type = xexec -a x$have_fork != xyes; then
  AC_MSG_ERROR([exec network support selected but fork not available])
fi
if test x$network_type = xwininet -o x$network_type = xwinhttp -o x$network_type = xwininethttp && test x$win32_specific_codes != xenabled; then
  AC_MSG_ERROR([wininet or winhttp is currently only for Windows])
fi

#### Use Win32 support codes
AM_CONDITIONAL([WIN32_CODES], [ test "x$win32_specific_codes" = xenabled ])

if test x"$ipv6" = xauto; then
	AC_MSG_CHECKING([IPv6 use])
	if test x"$have_ipv6" = xyes -a x"$network_type" != xdisabled; then
		ipv6=enabled
	else
		ipv6=disabled
	fi
	AC_MSG_RESULT([$ipv6])
fi

if test x$network_type != xdisabled -a x$network_internal = xwinsock2; then
  AC_DEFINE([WANT_WIN32_SOCKETS], [1], [ Define to use Win32 sockets ])
fi

AM_CONDITIONAL([NETWORK_WINSOCK], [ test x$network_type != xdisabled -a x$network_internal = xwinsock2 ])
AM_CONDITIONAL([NET123], [ test x$network_type != xdisabled ])
AM_CONDITIONAL([NET123_EXEC], [ test x$network_type = xexec ])
AM_CONDITIONAL([NET123_WINHTTP], [ test x$network_type = xwinhttp || test x$network_type = xwininethttp ])
AM_CONDITIONAL([NET123_WININET], [ test x$network_type = xwininet || test x$network_type = xwininethttp ])

case "$network_type" in
  exec)
    AC_DEFINE(NET123_EXEC,    1, [ Define for executable-based networking (for HTTPS). ])
  ;;
  winhttp)
    AC_DEFINE(NET123_WINHTTP, 1, [ Define for winhttp networking (for HTTPS). ])
  ;;
  wininet)
    AC_DEFINE(NET123_WININET, 1, [ Define for wininet networking (for HTTPS). ])
  ;;
  wininethttp)
    AC_DEFINE(NET123_WININET, 1, [ Define for wininet networking (for HTTPS). ])
    AC_DEFINE(NET123_WINHTTP, 1, [ Define for winhttp networking (for HTTPS). ])
  ;;
esac

dnl ############## Terminal choice

AM_CONDITIONAL( [TERM_POSIX], [test "x$term_type" = xposix] )
AM_CONDITIONAL( [TERM_NONE], [test "x$term_type" = xnone] )
AM_CONDITIONAL( [TERM_WIN32], [test "x$term_type" = xwin32] )

dnl ############## FIFO enable

if test x"$fifo" = xauto; then
	if test x"$have_mkfifo" = xyes; then
		fifo=enabled
	else
		fifo=disabled
	fi
fi

if test x"$fifo" = xenabled; then
	AC_DEFINE(FIFO, 1, [ Define if FIFO support is enabled. ])
	if test x"$have_mkfifo" = xno; then
		AC_MSG_WARN( [ You forced FIFO code while I think there is no mkfifo() available! ] )
	fi
fi

dnl ############## Network enable
if test x"$network_type" != xdisabled; then
	AC_DEFINE(NETWORK, 1, [ Define if network support is enabled. ])
	if test x"$have_network" = xno; then
		AC_MSG_WARN( [ You forced network code while I think there is support missing! ] )
	fi
fi

if test x"$ipv6" = xenabled; then
	AC_DEFINE(IPV6, 1, [ Define if IPV6 support is enabled. ])
	if test x"$have_ipv6" = xno; then
		AC_MSG_WARN( [ You forced IPv6 code while I think there is no getaddrinfo() available! ] )
	fi
fi

dnl ############## Library cleanup

PROG_LIBS=$LIBS
LIBS=
LIBMPG123_LIBS="$LIBM $COMPAT_LIBS"
LIBSYN123_LIBS=$LIBM
LIBOUT123_LIBS="$LIBRT $LIBM $COMPAT_LIBS"

AC_SUBST(PROG_LIBS)
AC_SUBST(LIBM)
AC_SUBST(LIBMPG123_LIBS)
AC_SUBST(LIBSYN123_LIBS)
AC_SUBST(LIBOUT123_LIBS)

dnl ############## Final Output

AC_CONFIG_FILES([
	Makefile
	libmpg123.pc
	libout123.pc
	libsyn123.pc
	mpg123.spec
])

AC_OUTPUT

dnl ############## Display Message

default_offset_bits=`expr "$ac_cv_sizeof_off_t" "*" "8"`

if test "x$build_programs" = xyes && ! test "x$portable_api" = xyes; then
  build_mpg123_programs=yes
else
  build_mpg123_programs=no
fi

echo "
  $PACKAGE_NAME $PACKAGE_VERSION

  Install path ............ $prefix
  Components .............. $components
  Main mpg123 program(s)... $build_mpg123_programs
  CPU Optimization ........ $cpu_type
  Compiler Optimization ... $with_optimization"
if test x"$use_yasm_for_avx" = xyes; then
  echo "  Use yasm (for AVX only) . enabled"
else
  echo "  Use yasm (for AVX only) . disabled"
fi

echo "  Gapless Support ......... $gapless
  Debugging ............... $debugging
  Terminal control ........ $term_type
  Extreme debugging ....... $xdebugging
  Seek table size ......... $seektable
  FIFO support ............ $fifo
  Buffer .................. $buffer"
if test x$network_type != xdisabled; then
  if test x$network_type != xinternal; then
    echo "  External network ........ $network_type"
  fi
  echo "  Internal network type ... $network_internal
  IPv6 (getaddrinfo) ...... $ipv6"
fi
echo "  largefile sensitive ..... $lfs_sensitive"
echo "  default offsets ......... $default_offset_bits"
echo "  explicit 64 bit offsets . $use_largefile"
echo "  forced 64 bit offsets ... $forced_off_64"
echo "  only portable API ....... $portable_api"

if test "x$forced_off_64" = xyes; then
  echo
  echo "Note: Ensure that client programs are built with _FILE_OFFSET_BITS defined to 64, too."
  echo "      Old binaries built for 32 bit off_t may fail missing symbols in mpg123 libraries."
  echo
fi

echo "
  Core libmpg123 features:
  Layer I ................. $layer1
  Layer II ................ $layer2
  Layer III ............... $layer3
  NtoM resampling ......... $ntom
  downsampled decoding .... $downsample
  Feeder/buffered input ... $feeder
  ID3v2 parsing ........... $id3v2
  String API .............. $string
  ICY parsing/conversion .. $icy
  Error/warning messages .. $messages
  Win32 Unicode File Open.. $win32_unicode
  Feature Report Function.. $feature_report
  Output formats (nofpu will disable all but 16 or 8 bit!):
  8 bit integer ........... $int8
  16 bit integer .......... $int16
  32/24 bit integer ....... $int32
  real (32 bit float) ..... $real
  Equalizer ............... $equalizer
  Optimization detail:
  Integer conversion ...... $integers
  IEEE 754 hackery ........ $ieee
  New/old WRITE_SAMPLE .... $newoldwritesample
  new Huffman scheme ...... $newhuff
  runtime tables .......... $runtimetables

Note: Disabling core features is not commonly done and some combinations might
      not build/work. If you encounter such a case, help yourself (and provide
      a patch) or just poke the maintainers."

echo

echo "  libsyn123 special cases . $specialcases"
echo

echo "  Modules ................. $modules"
echo "  Checked audio modules ... $check_modules"
echo "  Detected audio support .. $output_modules"
echo "  Runtime order ........... $default_output_modules"

if test x"$modules" = xdisabled; then
  echo
  echo "The _single_ active output module $default_output_module is being statically linked in."
fi
if test x"$with_optimization" = x0; then
  echo
  echo "No optimization flags chosen, ensure something basic in your CFLAGS at least..."
fi

if test x"$cpu_type" = xi486; then
  echo
  echo "WARNING: You selected the i486 decoder. This is not recommened for regular use."
  echo "It was designed for actual i486 CPUs a long time ago. Generic C code is likely"
  echo "to perform better nowadays. Also, the decoder misses features like volume"
  echo "scaling and has clipping issues. Continue if you are into software archeology"
  echo "only."
fi

if test "x$use_extlib" = xyes; then
  echo
  echo "Warning: You opted to use external builds of the mpg123 libraries. They need to"
  echo "         match the headers shipped with this source (which are used during build"
  echo "         regardless)."
  echo
fi

echo
echo "  CPPFLAGS='$CPPFLAGS'"
echo "  CFLAGS='$CFLAGS'"
echo "  PROG_LIBS='$PROG_LIBS'"
echo "       (derived from LIBS, only used for end-user binaries and modules)"
echo "  LIBDL='$LIBDL'"
echo "  LIBMPG123_LIBS='$LIBMPG123_LIBS'"
echo "  LIBSYN123_LIBS='$LIBSYN123_LIBS'"
echo "  LIBOUT123_LIBS='$LIBOUT123_LIBS'"
echo
echo "Next type 'make' and then 'make install'."
