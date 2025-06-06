#!/bin/bash
# make Windows release
# this script shall be run from Arch Linux with MinGW installed!

if [ ! -e /tmp/furnace ]; then
  mkdir /tmp/furnace || exit 1
  sudo mount --bind $PWD /tmp/furnace || exit 1
fi

cd /tmp/furnace

if [ ! -e xpbuild ]; then
  mkdir xpbuild || exit 1
fi

cd xpbuild

# TODO: potential Arch-ism?
i686-w64-mingw32-cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_C_FLAGS="-O2" -DCMAKE_CXX_FLAGS="-O2 -Wall -Wextra -Wno-unused-parameter -Wno-cast-function-type" -DBUILD_SHARED_LIBS=OFF -DSUPPORT_XP=ON -DWITH_RENDER_DX11=OFF -DSDL_SSE=OFF -DSDL_SSE2=OFF -DSDL_SSE3=OFF -DENABLE_SSE=OFF -DENABLE_SSE2=OFF -DENABLE_AVX=OFF -DENABLE_AVX2=OFF -DUSE_BACKWARD=OFF -DCONSOLE_SUBSYSTEM=OFF -DWITH_LOCALE=ON -DUSE_MOMO=ON -DFORCE_CODEVIEW=OFF .. || exit 1
make -j8 || exit 1

cd ..

mkdir -p release/winxp || exit 1
cd release/winxp

cp ../../LICENSE LICENSE.txt || exit 1
cp ../../xpbuild/furnace.exe . || exit 1
cp ../../xpbuild/furnace.pdb . || echo "WARNING: NO PDB FILE FOUND"
cp ../../res/releaseReadme/stable-win.txt README.txt || exit 1
cp -r ../../papers papers || exit 1
cp -r ../../demos demos || exit 1
cp -r ../../instruments instruments || exit 1
cp -r ../../wavetables wavetables || exit 1
cp -r ../../po/locale locale || exit 1

cp ../../res/docpdf/manual.pdf . || exit 1

i686-w64-mingw32-strip -s furnace.exe || exit 1

# patch to remove GetTickCount64
gcc -o patch_xp ../../scripts/patch_xp.c || exit 1
./patch_xp furnace.exe || exit 1

zip -r furnace.zip LICENSE.txt furnace.exe furnace.pdb README.txt manual.pdf papers demos instruments locale wavetables

furName=$(git describe --tags | sed "s/v0/0/")

mv furnace.zip furnace-"$furName"-win32-XP-ONLY.zip
