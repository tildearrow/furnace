#!/bin/bash
# make macOS release
# this script shall be run in macOS with CMake and the dev tools installed

# no, I won't use XCode...

if [ ! -e /tmp/furnace ]; then
  ln -s "$PWD" /tmp/furnace || exit 1
fi

cd /tmp/furnace

if [ ! -e macbuild ]; then
  mkdir macbuild || exit 1
fi

cd macbuild

cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_DEPLOYMENT_TARGET="10.9" -DCMAKE_C_FLAGS="-O2" -DCMAKE_CXX_FLAGS="-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Werror" -DWITH_JACK=OFF -DCMAKE_EXE_LINKER_FLAGS="" -DBUILD_PROGRAMS=OFF -DBUILD_EXAMPLES=OFF -DBUILD_TESTING=OFF -DENABLE_EXTERNAL_LIBS=OFF -DENABLE_MPEG=OFF .. || exit 1
make -j4 || exit 1
cpack || exit 1

cd ..

mkdir -p release/macos || exit 1
cd release/macos

cp ../../macbuild/_CPack_Packages/Darwin/Bundle/Furnace*.dmg .
