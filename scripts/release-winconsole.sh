#!/bin/bash
# make Windows release
# this script shall be run from Arch Linux with MinGW installed!

if [ ! -e /tmp/furnace ]; then
  ln -s "$PWD" /tmp/furnace || exit 1
fi

cd /tmp/furnace

if [ ! -e winCbuild ]; then
  mkdir winCbuild || exit 1
fi

cd winCbuild

# TODO: potential Arch-ism?
x86_64-w64-mingw32-cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_C_FLAGS="-O2" -DCMAKE_CXX_FLAGS="-O2 -Wall -Wextra -Wno-unused-parameter -Wno-cast-function-type -Wno-deprecated-declarations -Werror" -DUSE_BACKWARD=ON -DCONSOLE_SUBSYSTEM=ON -DWITH_LOCALE=ON -DUSE_MOMO=ON .. || exit 1
make -j8 || exit 1

cd ..

mkdir -p release/winconsole || exit 1
cd release/winconsole

cp ../../LICENSE LICENSE.txt || exit 1
cp ../../winCbuild/furnace.exe . || exit 1
cp ../../res/releaseReadme/stable-win.txt README.txt || exit 1
cp -r ../../papers papers || exit 1
cp -r ../../demos demos || exit 1
cp -r ../../instruments instruments || exit 1
cp -r ../../wavetables wavetables || exit 1
cp -r ../../po/locale locale || exit 1

cp ../../res/docpdf/manual.pdf . || exit 1

x86_64-w64-mingw32-strip -s furnace.exe || exit 1

zip -r furnace.zip LICENSE.txt furnace.exe README.txt manual.pdf papers demos instruments wavetables

furName=$(git describe --tags | sed "s/v0/0/")

mv furnace.zip furnace-"$furName"-win64-console.zip
