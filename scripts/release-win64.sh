#!/bin/bash
# make Windows release
# this script shall be run from Linux with MinGW installed!

if [ ! -e /tmp/furnace ]; then
  ln -s "$PWD" /tmp/furnace || exit 1
fi

cd /tmp/furnace

if [ ! -e winbuild ]; then
  mkdir winbuild || exit 1
fi

cd winbuild

x86_64-w64-mingw32-cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS="-O2" -DCMAKE_CXX_FLAGS="-O2 -Wall -Wextra -Wno-unused-parameter -Werror" .. || exit 1
make -j8 || exit 1
x86_64-w64-mingw32-strip -s furnace.exe || exit 1

cd ..

mkdir -p release/windows || exit 1
cd release/windows

cp ../../LICENSE LICENSE.txt || exit 1
cp ../../winbuild/furnace.exe . || exit 1
cp ../../README.md README.txt || exit 1
cp -r ../../papers papers || exit 1

zip -r furnace.zip LICENSE.txt furnace.exe README.txt papers
