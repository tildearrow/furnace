#!/bin/bash
testDir=20251109215858
if ./assert_delta "delta/$testDir/$1"; then
  true
  #echo "[1;32mOK[m"
else
echo -n "$1... "
  echo "[1;31mFAIL FAIL FAIL[m"
  mpv "delta/$testDir/$1"
  #ffmpeg -loglevel quiet -i "test/delta/$testDir/$1" -lavfi showspectrumpic -y "test/delta/$testDir/$1.png"
fi
