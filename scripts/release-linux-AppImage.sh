#!/bin/bash
# make linux release
# run on an Ubuntu 16.04 machine or VM for best results.

if [ ! -e /tmp/furnace ]; then
  ln -s "$PWD" /tmp/furnace || exit 1
fi

cd /tmp/furnace

if [ ! -e linuxbuild ]; then
  mkdir linuxbuild || exit 1
fi

cd linuxbuild

cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS="-O2" -DCMAKE_CXX_FLAGS="-O2 -Wall -Wextra -Wno-unused-parameter -Werror" -DWITH_DEMOS=OFF -DWITH_INSTRUMENTS=OFF -DWITH_WAVETABLES=OFF .. || exit 1
make -j4 || exit 1

cd ..

mkdir -p release/linux/furnace.AppDir || exit 1
cd linuxbuild

make DESTDIR=/tmp/furnace/release/linux/furnace.AppDir install || exit 1

cd ../release/linux/furnace.AppDir

cp -v ../../../res/logo.png furnace.png || exit 1
ln -s furnace.png .DirIcon || exit 1
cp -v ../../../res/furnace.desktop . || exit 1
#mkdir -p usr/share/metainfo || exit 1
#cp -v ../../../res/furnace.appdata.xml usr/share/metainfo/org.tildearrow.furnace.metainfo.xml || exit 1
#rm usr/share/metainfo/furnace.appdata.xml || exit 1
cp -v ../../../res/AppRun . || exit 1

#cp /usr/lib/libm.so.6 usr/lib/ || exit 1
#cp /usr/lib/libstdc++.so.6 usr/lib/ || exit 1
#cp /usr/lib/libc.so.6 usr/lib/ || exit 1
#cp /usr/lib/libgcc_s.so.1 usr/lib/ || exit 1

cd usr/bin
strip -s furnace

cd ../../..

[ -e appimagetool-x86_64.AppImage ] || { wget "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage" && chmod 755 appimagetool-x86_64.AppImage; }
./appimagetool-x86_64.AppImage --appimage-extract
ARCH=$(uname -m) ./squashfs-root/AppRun furnace.AppDir
