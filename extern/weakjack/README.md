Weak-JACK
=========

This small library abstracts the [JACK](http://jackaudio.org) Application Binary Interface.

Background and Motivation
-------------------------

The jack shared library needs to be installed system-wide (for all jack applications
to share), it can not be part of an application itself.

JACK developers take great care to not break binary compatibility of libjack. An
application compiled with one version of jack will work with all future versions
of jack. However, this only works well on GNU/Linux, BSD and to some extend on OSX.

weak-jack linking is useful (at least) in the following cases:

*   the resulting application should not be directly linked to libjack.[so|dll|dylib]
    IOW: the application should start even if libjack is not installed.
*   the ABI of libjack is not stable. Note, this is only relevant for the Windows .dll
    (e.g. applications compiled and linked with jack-1.9.9 will crash with jack-1.9.10
		 on windows. -- MSVC has a workaround: link by function-name, not ordinal, mingw
		 does not offer this)
*   Reference new API functions (e.g. meta-data API or new latency compensation)
    in the application, which is not available on older versions of jack.

Usage
-----

1. Copy the source files into your application's source (or use a git submodule)
2. replace all `#include<jack/*>` in your sources with `#include "weak-jack.h"`
3. add `weak_libjack.c` to the build-source of your project
   (in case your build-system does not detect `#include` dependencies automatically,
	 also reference the header and .def file).
4. Define `USE_WEAK_JACK` for all platforms where you want to use weak-linking. Usually
   `CFLAGS+=-DUSE_WEAK_JACK  CXXFLAGS+=-DUSE_WEAK_JACK`
5. Do not link your application to libjack (`-ljack`) when `USE_WEAK_JACK` is defined.

Note the jack-headers still need to be present when compiling the application.

The application code itself does not need to be changed.

The first call to `jack_client_open()` will try to find and load libjack, if it cannot be
found, it will fail (return `NULL` and set `jack_status_t` if provided to `JackFailure`.)

It is possible to explicitly initialize and query availability of libjack using
`have_libjack();` it returns 0 if libjack is available and can be used. (see the header
file for non-zero error codes).

Caveats
-------

If libjack is not available, all `jack_*` API calls are turned into no-operation functions.
This is not a problem in general, as jack-applications will not use any part of the jack API if
jack_client_open fails. The only exception here may be `jack_ringbuffer`. Note that the ringbuffer
implementation is also part of libjack and will not be available.

The dummy implementation for the ringbuffer API is safe (read, writes are ignored and return failure
or zero-bytes length), but if your application depends on it to work, you're out of luck :)

The function wrappers in `weak_libjack.def` were collected pragmatically it's quite possible that
some JACK API calls have been missed. If you application fails to link (without -ljack), please report
at https://github.com/x42/weakjack/issues

License
-------

GNU General Public License version 2 (or later).

Alternatives
------------

An alternative, more liberally licensed, implementation that abstracts and wraps jack completely
(incl headers) can be found at
https://github.com/falkTX/Carla/tree/master/source/jackbridge (C++ only),
and a jack2 specific version at https://github.com/sletz/jack2/blob/master/common/JackWeakAPI.c

A variant for python bindings is also provided by falkTX:
https://github.com/falkTX/Cadence/blob/master/src/jacklib.py
