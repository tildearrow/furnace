#!/bin/bash
sizes=(16 32 64 128 256 512)
for i in $sizes; do
  echo "making $i..."
  convert logo.png -filter Mitchell -scale "$i"x"$i" icon/icon_"$i"x"$i".png
done
for i in $sizes; do
  echo "making $i@2x..."
  convert logo.png -filter Mitchell -scale "$((i*2))"x"$((i*2))" icon/icon_"$i"x"$i""@2x.png"
done
