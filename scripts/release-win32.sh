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
i686-w64-mingw32-cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_C_FLAGS="-O2 -march=i586" -DCMAKE_CXX_FLAGS="-O2 -Wall -Wextra -Wno-unused-parameter -Wno-cast-function-type -march=i586" -DBUILD_SHARED_LIBS=OFF -DSUPPORT_XP=OFF -DWITH_RENDER_DX11=ON -DUSE_BACKWARD=ON -DSDL_SSE=OFF -DSDL_SSE2=OFF -DSDL_SSE3=OFF -DENABLE_SSE=OFF -DENABLE_SSE2=OFF -DENABLE_AVX=OFF -DENABLE_AVX2=OFF -DWITH_LOCALE=ON -DUSE_MOMO=ON .. || exit 1
make -j8 || exit 1

cd ..

mkdir -p release/win32 || exit 1
cd release/win32

cp ../../LICENSE LICENSE.txt || exit 1
cp ../../win32build/furnace.exe . || exit 1
cp ../../res/releaseReadme/stable-win.txt README.txt || exit 1
cp -r ../../papers papers || exit 1
cp -r ../../demos demos || exit 1
cp -r ../../instruments instruments || exit 1
cp -r ../../wavetables wavetables || exit 1
cp -r ../../po/locale locale || exit 1

cp ../../res/docpdf/manual.pdf . || exit 1

i686-w64-mingw32-strip -s furnace.exe || exit 1

zip -r furnace.zip LICENSE.txt furnace.exe README.txt manual.pdf papers demos instruments locale wavetables

furName=$(git describe --tags | sed "s/v0/0/")

mv furnace.zip furnace-"$furName"-win32.zip
