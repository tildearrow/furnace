#!/bin/bash
xxd -i -n tileDataC tile.gif | sed -r "s/^ +//g;s/, /,/g;s/ = /=/g;s/unsigned/static const unsigned/g" > ../src/gui/tileData.h
