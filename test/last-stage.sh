#!/bin/bash
echo -n "$1... "
if ./assert_delta "$1"; then
  true
  echo "[1;32mOK[m"
  rm "$1"
else
  echo "[1;31mFAIL FAIL FAIL[m"
  ffmpeg -loglevel quiet -i "$1" -lavfi showspectrumpic -y "$1.png"
fi
