# ROM export technical details

## instrument data

TODO

## macro data

read length, loop and then release (1 byte).
if it is a 2-byte macro, read a dummy byte.

then read data.

## binary command stream

read channel, command and values.

if channel is 80 or higher, then it is a special command:

```
fb xx xx xx xx: set tick rate
fc xx xx: wait xxxx ticks
fd xx: wait xx ticks
fe: wait one tick
ff: stop
```
