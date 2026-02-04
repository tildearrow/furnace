#!/bin/bash
# renders all files in test/songs/ and outputs them for delta testing.
# useful when doing changes to playback.
# requires GNU parallel.

testDir=$(date +%Y%m%d%H%M%S)
if [ -e "test/result" ]; then
  lastTest=$(ls "test/result" | tail -2 | head -1 || echo "")
else
  lastTest=""
fi

echo "lastTest is $lastTest"

if [ -e "test/assert_delta" ]; then
  echo "assert_delta present."
else
  echo "compiling assert_delta..."
  gcc -Wall -Wextra -Werror -o "test/assert_delta" "test/assert_delta.c" -lsndfile || exit 1
fi
  

echo "furnace test suite begin..."
echo "--- STEP 1: render test files"
mkdir -p "test/result/$testDir" || exit 1
ls "test/songs/" | parallel --verbose -j8 ./build/furnace -output "test/result/$testDir/{0}.wav" "test/songs/{0}"
echo "--- STEP 2: calculate deltas"
if [ -z $lastTest ]; then
  echo "skipping since this apparently is your first run."
else
  mkdir -p "test/delta/$testDir" || exit 1
  ls "test/result/$testDir/" | parallel --verbose -j4 ffmpeg -loglevel fatal -i "test/result/$lastTest/{0}" -i "test/result/$testDir/{0}" -filter_complex stereotools=phasel=1:phaser=1,amix=inputs=2:duration=longest -c:a pcm_s16le -y "test/delta/$testDir/{0}"
fi
echo "--- STEP 3: check deltas"
if [ -z $lastTest ]; then
  echo "skipping since this apparently is your first run."
else
  ls "test/delta/$testDir/*.wav" | parallel -t -j6 bash "test/last-stage.sh" '{}'
fi
