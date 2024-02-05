#!/bin/bash
# run after exporting to .ttf
# make sure you're running this on a Linux or Unix-like system with GCC

if [ ! -e binary_to_compressed_c ]; then
  g++ -o binary_to_compressed_c ../extern/imgui_patched/misc/fonts/binary_to_compressed_c.cpp || exit 1
fi

echo "#include \"fonts.h\"" > ../src/gui/font_furicon.cpp
./binary_to_compressed_c icons.ttf furIcons | sed "s/static //" >> ../src/gui/font_furicon.cpp
#xxd -i -n "furIcons" icons.ttf | sed -r "s/^ +//g;s/, /,/g;s/ = /=/g;s/unsigned/const unsigned/g" > ../src/gui/font_furIcons.cpp
