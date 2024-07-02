#!/bin/bash
sizes=(16 32 64 128 256 512)
if [ ! -e icon.iconset ]; then
  mkdir icon.iconset
fi
for i in "${sizes[@]}"; do
  echo "making $i..."
  convert logo.png -filter Mitchell -scale "$i"x"$i" icon.iconset/icon_"$i"x"$i".png
done
for i in "${sizes[@]}"; do
  echo "making $i@2x..."
  convert logo.png -filter Mitchell -scale "$((i*2))"x"$((i*2))" icon.iconset/icon_"$i"x"$i""@2x.png"
done

convert \
  -background none \
  icon_16x16.svg \
  icon.iconset/icon_256x256.png \
  \( -clone 0 \) \
  \( -clone 1 -resize 32x32 -extent 32x32 \) \
  \( -clone 1 -resize 48x48 -extent 48x48  \) \
  \( -clone 1 -resize 256x256 -extent 256x256 \) \
  -delete 0 \
  -delete 0 \
  icon.ico