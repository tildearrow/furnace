#!/bin/bash
# run after exporting to .ttf

echo "#include \"fonts.h\"" > ../src/gui/font_furicon.cpp
cat icons.ttf | zlib-flate -compress=9 | xxd -i -n "furIcons_compressed_data" | sed -r "s/^ +//g;s/, /,/g;s/ = /=/g;s/unsigned/const unsigned/g;s/compressed_data_len/compressed_size/" >> ../src/gui/font_furicon.cpp
