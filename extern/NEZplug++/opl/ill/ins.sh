#!/usr/bin/sh

gcc -o ./bin2c bin2c.c
./bin2c vrc7.ill>i_vrc7.h
./bin2c fmpac.ill>i_fmpac.h
./bin2c fmunit.ill>i_fmunit.h
rm -f ./bin2c
rm -f ./bin2c.exe
