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

# -DWITH_PORTAUDIO=OFF: Ubuntu 16.04 doesn't like it
cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS="-O3" -DCMAKE_CXX_FLAGS="-O3 -Wall -Wextra -Wno-unused-parameter -Werror" -DWITH_PORTAUDIO=OFF -DWITH_DEMOS=ON -DWITH_INSTRUMENTS=ON -DWITH_WAVETABLES=ON -DWITH_LOCALE=ON -DUSE_MOMO=ON .. || exit 1
make -j4 || exit 1

cd ..

mkdir -p release/linux/furnace || exit 1
cd linuxbuild

make DESTDIR=/tmp/furnace/release/linux/furnace install || exit 1

cd ../release/linux/furnace

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
mv share/locale ..

mv share/furnace/demos ..
mv share/furnace/instruments ..
mv share/furnace/wavetables ..
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
