#!/bin/bash
# run after exporting to .ttf

if [ ! -e binary_to_compressed_c ]; then
  g++ -o binary_to_compressed_c ../extern/imgui_patched/misc/fonts/binary_to_compressed_c.cpp || exit 1
fi

./binary_to_compressed_c icons.ttf furIcons > ../src/gui/font_furicon.cpp
#xxd -i -n "furIcons" icons.ttf | sed -r "s/^ +//g;s/, /,/g;s/ = /=/g;s/unsigned/const unsigned/g" > ../src/gui/font_furIcons.cpp
