#!/bin/bash

FUR_VERSION="0.6.8"

EXPORT_LANGS=("de" "es" "fr" "fi" "hy" "id" "ja" "ko" "nl" "pl" "pt_BR" "ru" "sk" "sv" "th" "tr" "uk" "zh" "zh_HK")

echo '#
msgid ""
msgstr ""' > po/furnace.pot
echo '"Project-Id-Version: furnace '"$FUR_VERSION"'\n"' >> po/furnace.pot
echo '"MIME-Version: 1.0\n"
"Content-Type: text/plain; charset=UTF-8\n"
"Content-Transfer-Encoding: 8bit\n"
' >> po/furnace.pot

find src/ -type f -regex ".*\(cpp\|h\)$" | xargs xgettext --omit-header -k_ -k_N -L C++ --from-code=UTF-8 -j -o po/furnace.pot || exit 1

cd po
for i in ${EXPORT_LANGS[@]}; do
  if [ -e "$i".po ]; then
    echo "merging $i"".po..."
    msgmerge --backup=none -N -U "$i".po furnace.pot || exit 1
  else
    echo "creating $i"".po..."
    msginit -i furnace.pot -l "$i".UTF-8 --no-translator || exit 1
  fi
done
