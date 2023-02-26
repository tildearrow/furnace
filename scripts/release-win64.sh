#!/bin/bash
# make Windows release
# this script shall be run from Arch Linux with MinGW installed!

if [ ! -e /tmp/furnace ]; then
  ln -s "$PWD" /tmp/furnace || exit 1
fi

cd /tmp/furnace

if [ ! -e winbuild ]; then
  mkdir winbuild || exit 1
fi

cd winbuild

# TODO: potential Arch-ism?
x86_64-w64-mingw32-cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_C_FLAGS="-O2" -DCMAKE_CXX_FLAGS="-O2 -Wall -Wextra -Wno-unused-parameter -Wno-cast-function-type -Wno-deprecated-declarations -Werror" .. || exit 1
make -j8 || exit 1
#x86_64-w64-mingw32-strip -s furnace.exe || exit 1

cd ..

mkdir -p release/windows || exit 1
cd release/windows

cp ../../LICENSE LICENSE.txt || exit 1
cp ../../winbuild/furnace.exe . || exit 1
cp ../../README.md README.txt || exit 1
cp -r ../../papers papers || exit 1
cp -r ../../demos demos || exit 1
cp -r ../../instruments instruments || exit 1
cp -r ../../wavetables wavetables || exit 1

zip -r furnace.zip LICENSE.txt furnace.exe README.txt papers demos instruments wavetables

furName=$(git describe --tags | sed "s/v0/0/")

mv furnace.zip furnace-"$furName"-win64.zip
