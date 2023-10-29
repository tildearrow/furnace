#!/bin/sh

if [ $# -lt 2 ]; then
  echo "usage: $0 input output"
  exit 1
fi

#echo "generating $2..."

cat "$1" > "$2"

echo "  <releases>" >> "$2"

for i in `git log --tags='v*' --no-walk --format="%as/%(describe:tags)"`; do
  releaseDate=${i%/*}
  releaseVer=${i#*/}
  releaseVerProper=$releaseVer
  case $releaseVer in
    *pre*)
      releaseType=development
      releaseVerProper="${releaseVer%pre*}~pre${releaseVer#*pre}"
      ;;
    *) releaseType=stable;;
  esac
  echo "    <release version=\"${releaseVerProper}\" date=\"$releaseDate\" type=\"$releaseType\">" >> "$2"
  echo "      <url>https://github.com/tildearrow/furnace/releases/tag/$releaseVer</url>" >> "$2"
  echo "    </release>" >> "$2"
done

echo "  </releases>" >> "$2"

echo "</component>" >> "$2"

#echo "done."
