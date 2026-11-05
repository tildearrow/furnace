#!/bin/sh

set -e
export LC_ALL=C

sweep="$srcdir/src/tests/sweep.mp3"
out=src/tests/plain_id3.out.txt
ref="$srcdir/src/tests/plain_id3.txt"

src/tests/plain_id3 "$sweep" > src/tests/plain_id3.out.txt

if ! diff -q "$ref" "$out"; then
  exit 1
fi
echo PASS
