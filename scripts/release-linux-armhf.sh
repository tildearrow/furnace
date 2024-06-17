#!/bin/bash
# make linux release
# run on an Ubuntu 20.04 machine or VM for best results.

if [ ! -e /tmp/furnace ]; then
  ln -s "$PWD" /tmp/furnace || exit 1
fi

cd /tmp/furnace

if [ ! -e armbuild ]; then
  mkdir armbuild || exit 1
fi

cd armbuild

cmake -DCMAKE_TOOLCHAIN_FILE=/tmp/furnace/scripts/Cross-Linux-armhf.cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS="-O3" -DCMAKE_CXX_FLAGS="-O3 -Wall -Wextra -Wno-unused-parameter -Werror" -DWITH_PORTAUDIO=OFF -DWITH_DEMOS=ON -DWITH_INSTRUMENTS=ON -DWITH_WAVETABLES=ON -DWITH_LOCALE=ON -DUSE_MOMO=OFF .. || exit 1
make -j4 || exit 1

cd ..

mkdir -p release/linuxarm/furnace || exit 1
cd armbuild

make DESTDIR=/tmp/furnace/release/linuxarm/furnace install || exit 1

cd ../release/linuxarm/furnace

cp -v ../../../res/logo.png .DirIcon || exit 1
#cp -v ../../../res/furnace.desktop . || exit 1

cd usr

mv bin/furnace .. || exit 1
rmdir bin || exit 1

rm -r share/applications
rm -r share/doc
mv share/icons ..
rm -r share/licenses
rm -r share/metainfo

mv share/furnace/demos ..
mv share/furnace/instruments ..
mv share/furnace/wavetables ..
mv share/locale ..
rm -r share/furnace || exit 1
rm -r share || exit 1

cd ..

cp ../../../LICENSE . || exit 1
cp ../../../res/releaseReadme/stable-linux.txt README.md || exit 1
cp -r ../../../papers papers || exit 1
curl "https://tildearrow.org/furnace/doc/latest/manual.pdf" > manual.pdf
rmdir usr || exit 1

strip -s furnace

cd ..

tar -zcv -f furnace.tar.gz furnace
