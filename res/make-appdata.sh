#!/bin/bash

echo "generating furnace.appdata.xml..."

cat furnace.appdata.xml.in > furnace.appdata.xml

echo "  <releases>" >> furnace.appdata.xml

for i in `git log --tags='v*' --no-walk --format="%as/%(describe:tags)"`; do
  releaseDate=${i%/*}
  releaseVer=${i#*/}
  releaseType=stable
  if [[ $releaseVer =~ "pre" ]]; then
    releaseType=development
  fi
  echo "    <release version=\"${releaseVer/pre/~pre}\" date=\"$releaseDate\" type=\"$releaseType\">" >> furnace.appdata.xml
  echo "      <url>https://github.com/tildearrow/furnace/releases/tag/$releaseVer</url>" >> furnace.appdata.xml
  echo "    </release>" >> furnace.appdata.xml
done

echo "  </releases>" >> furnace.appdata.xml

echo "</component>" >> furnace.appdata.xml

echo "done."
