# ROM export technical details

## instrument data

TODO

## macro data

read length, loop and then release (1 byte).
if it is a 2-byte macro, read a dummy byte.

then read data.

## binary command stream

Furnace Command Stream, split version.

```
size | description
-----|------------------------------------
  4  | "FCS\0" format magic
  4  | channel count
 4?? | pointers to channel data
 2?? | preset delays
     | - 16 values
 1?? | speed dial commands
     | - 16 values
```

read channel, command and values.

if channel is 80 or higher, then it is a special command:

```
fb xx xx xx xx: set tick rate
fc xx xx: wait xxxx ticks
fd xx: wait xx ticks
fe: wait one tick
ff: stop
```
