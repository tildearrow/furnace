#!/bin/bash
# make macOS release
# this script shall be run in macOS with CMake and the dev tools installed

# no, I won't use XCode...

if [ ! -e macbuild ]; then
  mkdir macbuild || exit 1
fi

cd macbuild

cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS="-O2" -DCMAKE_CXX_FLAGS="-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Werror" -DBUILD_PROGRAMS=OFF -DBUILD_EXAMPLES=OFF -DBUILD_TESTING=OFF -DENABLE_EXTERNAL_LIBS=OFF -DENABLE_MPEG=OFF .. || exit 1
make -j4 || exit 1
cpack || exit 1

cd ..

mkdir -p release/macos || exit 1
cd release/macos

cp ../../macbuild/_CPack_Packages/Darwin/Bundle/Furnace*.dmg .
cp ../../LICENSE LICENSE.txt || exit 1
cp ../../README.md README.txt || exit 1
cp -r ../../papers papers || exit 1

zip -r furnace.zip LICENSE.txt Furnace.dmg README.txt papers
