#!/bin/bash
# make linux release

if [ ! -e /tmp/furnace ]; then
  ln -s "$PWD" /tmp/furnace || exit 1
fi

cd /tmp/furnace

if [ ! -e linuxbuild ]; then
  mkdir linuxbuild || exit 1
fi

cd linuxbuild

cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS="-O2" -DCMAKE_CXX_FLAGS="-O2 -Wall -Wextra -Wno-unused-parameter -Werror" .. || exit 1
make -j4 || exit 1

cd ..

mkdir -p release/linux/furnace.AppDir || exit 1
cd linuxbuild

make DESTDIR=/tmp/furnace/release/linux/furnace.AppDir install || exit 1

cd ../release/linux/furnace.AppDir

cp ../../../res/logo.png furnace.png || exit 1
ln -s furnace.png .DirIcon || exit 1
cp ../../../res/furnace.desktop . || exit 1
cp ../../../res/AppRun . || exit 1

cp /usr/lib/libm.so.6 usr/lib/ || exit 1
cp /usr/lib/libstdc++.so.6 usr/lib/ || exit 1
cp /usr/lib/libc.so.6 usr/lib/ || exit 1
cp /usr/lib/libgcc_s.so.1 usr/lib/ || exit 1

cd ..

[ -e appimagetool-x86_64.AppImage ] || { wget "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage" && chmod 755 appimagetool-x86_64.AppImage; }
ARCH=$(uname -m) ./appimagetool-x86_64.AppImage furnace.AppDir

#zip -r furnace.zip LICENSE.txt Furnace*.dmg README.txt papers
