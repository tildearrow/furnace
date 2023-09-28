#!/bin/bash

echo "compiling Furnace doc (HTML)..."

if [ -e htmldoc ]; then
  rm -r htmldoc
fi

if [ ! -e .venv ]; then
  python3 -m virtualenv .venv || exit 1
fi

source .venv/bin/activate

if [ ! -e .venv/req_installed ]; then
  pip install -r requirements.txt || exit 1
  touch .venv/req_installed
fi

python3 make_htmldoc.py

echo "copying assets..."
for i in `find ../../doc -name "*.png"`; do
  cp "$i" "htmldoc${i#../../doc}"
done
