#!/bin/bash

mkdir -p compressed
for i in *.ttf *.otf; do
  echo "$i"
  cat "$i" | zlib-flate -compress=9 > compressed/"$i".zl
done
