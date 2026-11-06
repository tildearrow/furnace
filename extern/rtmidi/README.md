# RtMidi

![Build Status](https://github.com/thestk/rtmidi/actions/workflows/ci.yml/badge.svg)
[![Conan Center](https://shields.io/conan/v/rtmidi)](https://conan.io/center/rtmidi)

A set of C++ classes that provide a common API for realtime MIDI input/output across Linux (ALSA & JACK), Macintosh OS X (CoreMIDI & JACK), Windows (Multimedia Library, UWP & Windows MIDI Services), Web MIDI, iOS and Android.

By Gary P. Scavone, 2003-2023.

This distribution of RtMidi contains the following:

- `doc`:      RtMidi documentation (also online at https://caml.music.mcgill.ca/~gary/rtmidi/)
- `tests`:    example RtMidi programs
- `examples`: example apps for iOS and Android

On Unix systems, run `./configure` and then `make` in the top level directory.  This builds both the library and the test programs in `tests/`.  For installation and building on Windows, refer to [WINDOWS.md](WINDOWS.md).

If you checked out the code from git, please run `./autogen.sh` before `./configure`.

`./configure --help` lists the available MIDI API options (`--with-alsa`, `--with-jack`, `--with-core`, `--with-winmm`); by default those appropriate to the system are selected.  RtMidi can also be built with CMake.

On Linux, ALSA support needs its development files (`libasound2-dev` on Debian and Ubuntu, `alsa-lib-devel` on Fedora); without them both `./configure` and CMake build without ALSA, and `./configure` may find no MIDI API if no other backend is available.

Android support uses the AMidi API and must be built with CMake.  See [examples/android/README.md](examples/android/README.md).  Windows MIDI Services is also CMake only, and opt-in: configure with `-DRTMIDI_API_WMS=ON`.  See [WINDOWS.md](WINDOWS.md#windows-midi-services) for getting Microsoft's preview package, falling back to WinMM, and what to tell your users.

If you compile `RtMidi.cpp` yourself, define the backends you want: `RTMIDI_API_ALSA`, `RTMIDI_API_JACK`, `RTMIDI_API_CORE`, `RTMIDI_API_WINMM`, `RTMIDI_API_WMS`, `RTMIDI_API_UWP`, `RTMIDI_API_WEBMIDI` or `RTMIDI_API_AMIDI` (and `RTMIDI_DEBUG` for debug output).  These replace `__LINUX_ALSA__`, `__UNIX_JACK__`, `__MACOSX_CORE__`, `__WINDOWS_MM__` and the other names starting with underscores, which C++ reserves (#133).  The old names still work in 6.x but are deprecated and will be removed in 7.0.0.

## Overview

RtMidi is a set of C++ classes (`RtMidiIn`, `RtMidiOut`, and API specific classes) that provide a common API (Application Programming Interface) for realtime MIDI input/output across Linux (ALSA, JACK), Macintosh OS X (CoreMIDI, JACK), Windows (Multimedia Library, UWP, Windows MIDI Services), Web MIDI, iOS and Android.  RtMidi significantly simplifies the process of interacting with computer MIDI hardware and software.  It was designed with the following goals:

  - object oriented C++ design
  - simple, common API across all supported platforms
  - only one header and one source file for easy inclusion in programming projects
  - MIDI device enumeration

MIDI input and output functionality are separated into two classes, `RtMidiIn` and `RtMidiOut`.  Each class instance supports only a single MIDI connection.  RtMidi does not provide timing functionality (i.e., output messages are sent immediately).  Input messages are timestamped with delta times in seconds (via a `double` floating point type).  MIDI data is passed to the user as raw bytes using an `std::vector<unsigned char>`.

RtMidi is also offered as a module, which is enabled with `RTMIDI_BUILD_MODULES`, and is accessed with `import rt.midi;`. The classes live in namespace `rt::midi`.  `RtMidi.h` ends with `using namespace rt::midi;` unless the preprocessor definition `RTMIDI_USE_NAMESPACE` is set, so with the header `rt::midi::MidiApi` and `::MidiApi` are both valid; the CMake option `RTMIDI_USE_NAMESPACE` (on by default) sets that definition for the module build.

## Windows

See [WINDOWS.md](WINDOWS.md) for building RtMidi with Visual Studio or CMake, using it in your own project, Windows MIDI Services, and notes such as using the Microsoft GS Wavetable Synth.

## Further reading

For complete documentation on RtMidi, see the `doc` directory of the distribution or surf to https://caml.music.mcgill.ca/~gary/rtmidi/.

## Legal and ethical

The RtMidi license is similar to the MIT License, with the added *feature* that modifications be sent to the developer.  Please see [LICENSE](LICENSE).
