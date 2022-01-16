# ROM export technical details

## instrument data

TODO

## pattern data

read sequentially.

first byte determines what to read next:

```
NVI..EEE

N: note
V: volume
I: instrument

EEE: effect count (0-7)
```

if you read 0, end of pattern.
otherwise read in following order:

1. note
2. volume
3. instrument
4. effect and effect value

then read number of rows until next value, minus 1.
