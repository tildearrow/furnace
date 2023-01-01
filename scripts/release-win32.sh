#!/bin/bash
# make Windows release
# this script shall be run from Arch Linux with MinGW installed!

if [ ! -e /tmp/furnace ]; then
  ln -s "$PWD" /tmp/furnace || exit 1
fi

cd /tmp/furnace

if [ ! -e win32build ]; then
  mkdir win32build || exit 1
fi

cd win32build

# TODO: potential Arch-ism?
i686-w64-mingw32-cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS="-O2" -DCMAKE_CXX_FLAGS="-O2 -Wall -Wextra -Wno-unused-parameter -Wno-cast-function-type -Werror" -DBUILD_SHARED_LIBS=OFF -DSUPPORT_XP=ON .. || exit 1
make -j8 || exit 1
i686-w64-mingw32-strip -s furnace.exe || exit 1

cd ..

mkdir -p release/win32 || exit 1
cd release/win32

cp ../../LICENSE LICENSE.txt || exit 1
cp ../../win32build/furnace.exe . || exit 1
cp ../../README.md README.txt || exit 1
cp -r ../../papers papers || exit 1
cp -r ../../demos demos || exit 1
cp -r ../../instruments instruments || exit 1

zip -r furnace.zip LICENSE.txt furnace.exe README.txt papers demos instruments

furName=$(git describe --tags | sed "s/v0/0/")

mv furnace.zip furnace-"$furName"-win32.zip
