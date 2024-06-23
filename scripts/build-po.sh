#!/bin/bash
EXPORT_LANGS=("de" "es" "fr" "fi" "hy" "id" "ko" "nl" "pl" "pt_BR" "ru" "sk" "sv" "th" "tr" "uk" "zh" "zh_HK")

for i in ${EXPORT_LANGS[@]}; do
  echo "compiling $i.po..."
  mkdir -p "po/locale/$i/LC_MESSAGES/" && msgfmt "po/$i.po" -o "po/locale/$i/LC_MESSAGES/furnace.mo"
done
