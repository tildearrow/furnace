#!/bin/sh
set -e
set -x
sweep="$srcdir/src/tests/sweep.mp3"
sweepsweep="src/tests/sweepsweep.mp3"
stripsweep="src/tests/stripsweep.mp3"

echo "Test successful decoding as such:"
c1=$(src/tests/decode_fixed "$sweep" | wc -c)

echo "Test that track is decoded only once:"
cat "$sweep" "$sweep" > "$sweepsweep"
c2=$(src/tests/decode_fixed "$sweepsweep" | wc -c)
test "$c1" = "$c2"

echo "Test that stripped track is decoded twice:"
src/mpg123-strip -n < "$sweepsweep" > "$stripsweep"
c3=$(src/tests/decode_fixed "$stripsweep" | wc -c)
# Needs to be even larger as gapless info vanished.
test "$c3" -gt "$(($c2*2))"

echo PASS
