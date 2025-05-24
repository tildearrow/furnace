# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.2.2] - 2023-08-13

### Fixed

* Fixed invalid regex in src/create_symbols_file.py
* Fixed passing null pointer to printf %s in tests

## [1.2.1] - 2023-08-12

### Added

* RISC OS support to `sndfile-play`, thanks @ccawley2011
* Move semantics to `SndFileHandle` C++ class, thanks @haydaralaidrus

### Fixed

* Various typos, thanks @@uniontech-lilinjie
* Handling of absolute `CMAKE_INSTALL_LIBDIR`/`CMAKE_INSTALL_INCLUDEDIR`, thanks
  @Staudey (issue #908)
* Add `localtime_s` support to `sndfile-metadata-set`, thanks @neheb (issue #907)
* Linking with CMake against `Ogg::ogg`, thanks @FtZPetruska
* CMake `mpg123` module handling bugs, thanks @FtZPetruska
* CMake dependencies handling, thanks @FtZPetruska
* Various `Ogg` & `Opus` format fixes, thanks @weiliang (issue #888)
* Redefining `ssize_t` when building with Autotools, thanks @ccawley2011
  (issue #934)
* Bug related to incorrect `realloc` use, thanks @Halmoni100
* Style errors, thanks to @arthurt
* AIFF format related bugs, thanks to @arthurt
* Reading of MP3 files without Xing or INFO headers, thanks @arthurt
* Coding style of `src/mpeg_decode.c`, thanks @arthurt
* Various documentation types, thanks @luzpaz
* Intrinsics inclusion for MSVC and ARM64/ARM64EC, thanks @frysee
* `sf_open_fd`() regression, thanks @brentr (PR #950)
* WAV format related bug, thanks @magnus-nomono (issue #930)

### Removed

* Obsolete file `libsndfile.spec.in``, thanks @janstary

## [1.2.0] - 2022-12-25

### Fixed

* Searching for LAME dependency with CMake build system (issue #821).
* CMake build from Autotools tarball (issue #816).
* Build on UWP platform (issue #824).
* Fix signed integer overflow (issue #785).
* Skipping large wav chunks on stdin (PR #819).

### Removed

* Maximum samplerate limit, thanks @drmpeg, @justacec (issue #850).
  
  In version 1.1.0, an artificial limit of 655350 Hz was created, but as it
  turned out, this is not enough for some scenarios.

## [1.1.0] - 2022-03-27

### Added

* MPEG Encode/Decode Support.

  Uses libmpg123 for decode, liblame for encode. Encoding and decoding support
  is independent of each other and is split into separate files. MPEG support
  is generalized as subformats, `SF_FORMAT_MPEG_LAYER`(I,II,III) so that it
  might be used by other containers (`MPEG1WAVEFORMAT` for example), but also
  contains a major format `SF_FORMAT_MPEG` for 'mp3 files.'

  Encoding Status:
  * Layer III encoding
  * ID3v1 writing
  * ID3v2 writing
  * Lame/Xing Tag writing
  * Bitrate selection command
  * VBR or CBR
  
  Decoding Status:
  * Layers I/II/III decoding
  * ID3v1 reading
  * ID3v2 reading
  * Seeking
* New fuzzer for OSS-Fuzz, thanks @DavidKorczynski.
* This `CHANGELOG.md`. All notable changes to this project will be documented in
  this file. The old `NEWS` file has been renamed to `NEWS.OLD` and is no longer
  updated.
* Add support for decoding MPEG III Audio in WAV files.
* `SECURITY.md` file to give people instructions for reporting security
  vulnerabilities, thanks @zidingz.
* Support for [Vcpkg manifest mode](https://vcpkg.readthedocs.io/en/latest/users/manifests/).

  If you have problems with manifest mode, disable it with `VCPKG_MANIFEST_MODE`
  switch.
* [Export CMake targets from the build tree (PR #802)](https://cmake.org/cmake/help/latest/guide/importing-exporting/index.html#exporting-targets-from-the-build-tree)
* CIFuzz fuzzer, thanks to @AdamKorcz (PR #796)

### Changed

* `SFC_SET_DITHER_ON_READ` and `SFC_SET_DITHER_ON_WRITE` enums comments in
  public header, thanks @SmiVan (issue #677).
* `ENABLE_SNDFILE_WINDOWS_PROTOTYPES` define is deprecated and not needed
  anymore.

  Previously, in order for the [`sf_wchar_open`()](http://libsndfile.github.io/libsndfile/api.html#open)
  function to become available on   the Windows platform, it was required to
  perform certain actions:

  ```c
  #include <windows.h>
  #define ENABLE_SNDFILE_WINDOWS_PROTOTYPES 1
  #including <sndfile.h>
  ```

  These steps are no longer required and the `sf_wchar_open`() function is
  always available on the Windows platform.
* Use UTF-8 as internal path encoding on Windows platform.

  This is an internal change to unify and simplify the handling of file paths.

  On the Windows platform, the file path is always converted to UTF-8 and
  converted to UTF-16 only for calls to WinAPI functions.

  The behavior of the functions for opening files on other platforms does not
  change.
* Switch to .xz over .bz2 for release tarballs.
* Disable static builds using Autotools by default. If you want static
  libraries, pass --enable-static to ./configure

### Fixed

* Typo in `docs/index.md`.
* Typo in `programs/sndfile-convert.c`, thanks @fjl.
* Memory leak in `caf_read_header`(), credit to OSS-Fuzz ([issue 30375](https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=30375)).
* Stack overflow in `guess_file_type`(), thanks @bobsayshilol, credit to
  OSS-Fuzz ([issue 29339](https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=29339)).
* Abort in fuzzer, thanks @bobsayshilol, credit to OSS-Fuzz
  ([issue 26257](https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=26257)).
* Infinite loop in `svx_read_header`(), thanks @bobsayshilol, credit to OSS-Fuzz
  ([issue 25442](https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=25442)).
* GCC and Clang pedantic warnings, thanks @bobsayshilol.
* Normalisation issue when scaling floating point data to `int` in
  `replace_read_f2i`(), thanks @bobsayshilol, (issue #702).
* Missing samples when doing a partial read of Ogg file from index till the end
  of file, thanks @arthurt (issue #643).
* sndfile-salvage: Handle files > 4 GB on Windows OS
* Undefined shift in `dyn_get_32bit`(), credit to OSS-Fuzz
  ([issue 27366](https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=27366)).
* Integer overflow in `nms_adpcm_update`(), credit to OSS-Fuzz
  ([issue 25522](https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=25522)).
* Integer overflow in `psf_log_printf`(), credit to OSS-Fuzz
  ([issue 28441](https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=28441)),
  ([issue 25624](https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=25624)).
* ABI version incompatibility between Autotools and CMake build on Apple
  platforms.

  Now ABI must be compatible with Autotools builds. Note that this change
  requires CMake >= 3.17 for building dylib on Apple platforms.

* Fix build with Autotools + MinGW toolchain on Windows platform.

  See https://github.com/msys2/MINGW-packages/issues/5803 for details.

### Security

* Heap buffer overflow in `wavlike_ima_decode_block`(), thanks @bobsayshilol,
  credit to OSS-Fuzz ([issue 25530](https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=25530)).
* Heap buffer overflow in `msadpcm_decode_block`(), thanks @bobsayshilol,
  credit to OSS-Fuzz ([issue 26803](https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=26803)).
* Heap buffer overflow in `psf_binheader_readf`(), thanks @bobsayshilol,
  credit to OSS-Fuzz ([issue 26026](https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=26026)).
* Index out of bounds in `psf_nms_adpcm_decode_block`(), credit to OSS-Fuzz
  ([issue 25561](https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=25561)).
* Heap buffer overflow in `flac_buffer_copy`(), thanks @yuawn,  @bobsayshilol.
* Heap buffer overflow in `copyPredictorTo24`(), thanks @bobsayshilol,
  credit to OSS-Fuzz ([issue 27503](https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=27503)).
* Uninitialized variable in `psf_binheader_readf`(), thanks @shao-hua-li,
  credit to OSS-Fuzz ([issue 25364](https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=25364)).

[Unreleased]: https://github.com/libsndfile/libsndfile/compare/1.2.2...HEAD
[1.2.2]: https://github.com/libsndfile/libsndfile/compare/1.2.1...1.2.2
[1.2.1]: https://github.com/libsndfile/libsndfile/compare/1.2.0...1.2.1
[1.2.0]: https://github.com/libsndfile/libsndfile/compare/1.1.0...1.2.0
[1.1.0]: https://github.com/libsndfile/libsndfile/compare/1.0.31...1.1.0
