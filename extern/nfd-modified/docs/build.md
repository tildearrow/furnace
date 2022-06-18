# Building NFD #

Most of the building instructions are included in [README.md](/README.md). This file just contains apocrypha.

## Running Premake5 Directly ##

*You shouldn't have to run Premake5 directly to build Native File Dialog.  This is for package maintainers or people with exotic demands only!*

1. [Clone premake-core](https://github.com/premake/premake-core)
2. [Follow instructions on how to build premake](https://github.com/premake/premake-core/wiki/Building-Premake)
3. `cd` to `build`
4. Type `premake5 <type>`, where <type> is the build you want to create.

### Package Maintainer Only ###

I support a custom Premake action: `premake5 dist`, which generates all of the checked in project types in subdirectories.  It is useful to run this command if you are submitting a pull request to test all of the supported premake configurations.  Do not check in the built projects; I will do so while accepting your pull request.

## SCons build (deprecated) ##

As of 1.1.6, the deprecated and unmaintained SCons support is removed.

## Compiling with Mingw ##

Use the Makefile in `build/gmake_windows` to build Native File Dialog with mingw.  Mingw has many distributions and not all of them are reliable.  Here is what worked for me, the primary author of Native File Dialog:

1. Use mingw64, not mingw32.  Downloaded from [sourceforge.net](https://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win32/Personal%20Builds/mingw-builds/installer/mingw-w64-install.exe/download).
2. When prompted in the intsaller, install the basic compiler and g++.
3. Add the installed bin dir to command prompt path.
4. Run `set CC=g++` to enforce `g++` instead of the default, `cc` for compiling and linking.
5. In `build/gmake_windows`, run `mingw32-make config=release_x64 clean`.  Running clean ensures no Visual Studio build products conflict which can cause link errors.
6. Now run `mingw32-make config=release_x64`.

The author has not attempted to build or even install an x86 toolchain for mingw.

If you report an issue, be sure to run make with `verbose=1` so commands are visible.

## Adding NFD source directly to your project ##

Lots of developers add NFD source directly to their projects instead of using the included build scripts.  As of 1.1.6, this is an acknowledged approach to building.  Of course, everyone has a slightly different toolchain with various warnings and linters enabled.  If you run a linter or catch a warning, please consider submitting a pull request to help NFD build cleanly for everyone.
