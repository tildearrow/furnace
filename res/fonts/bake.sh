#!/bin/bash

# "file" "symbol" "license" "output"
fonts=(
"Exo-Medium.ttf" "font_exo" "papers/exo-license.txt" "font_exo.cpp"
"FontAwesome.otf" "iconFont" "Font Awesome by Dave Gandy - http://fontawesome.io" "font_icon.cpp"
"IBMPlexMono-Regular.otf" "font_plexMono" "papers/ibm-plex-license.txt" "font_plexMono.cpp"
"IBMPlexSans-Regular.otf" "font_plexSans" "papers/ibm-plex-license.txt" "font_plexSans.cpp"
"IBMPlexSansJP-Regular.otf" "font_plexSansJP" "papers/ibm-plex-license.txt" "font_plexSansJP.cpp"
"IBMPlexSansKR-Regular.otf" "font_plexSansKR" "papers/ibm-plex-license.txt" "font_plexSansKR.cpp"
"LiberationSans-Regular.ttf" "font_liberationSans" "papers/liberation-license.txt" "font_liberationSans.cpp"
"mononoki-Regular.ttf" "font_mononoki" "papers/mononoki-license.txt" "font_mononoki.cpp"
"ProggyClean.ttf" "font_proggyClean" "papers/proggy-license.txt" "font_proggyClean.cpp"
"PTMono.ttf" "font_ptMono" "papers/pt-mono-license.txt" "font_ptMono.cpp"
"unifont_jp-15.1.05.otf" "font_unifont" "papers/unifont-license.txt" "font_unifont.cpp"
)

fontCount=${#fonts[@]}
for i in `seq 0 4 $((fontCount-1))`; do
  fontPath="${fonts[i]}"
  fontSym="${fonts[i+1]}"
  fontLicense="${fonts[i+2]}"
  fontOut="../../src/gui/${fonts[i+3]}"
  echo "$fontPath"
  echo "// $fontLicense" > "$fontOut"
  echo "// File: '$fontPath' ($(stat -c %s $fontPath) bytes)" >> "$fontOut"
  echo "#include \"fonts.h\"" >> "$fontOut"
  cat "$fontPath" | zlib-flate -compress=9 | xxd -i -n "$fontSym""_compressed_data" | sed -r "s/^ +//g;s/, /,/g;s/ = /=/g;s/unsigned/const unsigned/g;s/compressed_data_len/compressed_size/" >> "$fontOut"
done
