# 2A03 APU register map

## Pulse 1

```
    7 .... 0
00  xx......   duty
    ..x.....   disable length counter
    ...x....   disable envelope
    ....xxxx   volume/envelope speed
01  x.......   enable sweep
    .xxx....   sweep time/period
    ....x...   sweep direction (1 = down)
    .....xxx   sweep shift
02  xxxxxxxx   period low
03  xxxxx...   set length counter
    .....xxx   period high
```

## Pulse 2

```
    7 .... 0
04  xx......   duty
    ..x.....   disable length counter
    ...x....   disable envelope
    ....xxxx   volume/envelope speed
05  x.......   enable sweep
    .xxx....   sweep time/period
    ....x...   sweep direction (1 = down)
    .....xxx   sweep shift
06  xxxxxxxx   period low
07  xxxxx...   set length counter
    .....xxx   period high
```

## Triangle

```
08  x.......   disable length counter
    .xxxxxxx   set linear counter
09  ........
0A  xxxxxxxx   period low
0B  xxxxx...   set length counter
    .....xxx   period high
```

## Noise

```
    7 .... 0
0C  ..x.....   disable length counter
    ...x....   disable envelope
    ....xxxx   volume/envelope speed
0D  ........
0E  x.......   LFSR length (1 = short)
    ....xxxx   pitch
0F  xxxxx...   set length counter
```

## DMC/control

DMC IRQ occurs at the end of the sample.

```
    7 .... 0
10  x.......   enable IRQ
    .x......   loop
    ....xxxx   pitch
11  .xxxxxxx   set DAC value
12  xxxxxxxx   address << 6 (+$C000)
13  xxxxxxxx   length << 4 (+1)
```

address and length changes take effect after DMA.

# Control

frame counter has two speeds:
- 0: 240Hz
- 1: 192Hz

frame IRQ occurs after 4 frame counter cycles and does not occur if the speed bit is set.

```
15w ...x....   enable DMC
    ....x...   enable noise
    .....x..   enable triangle
    ......x.   enable pulse 2
    .......x   enable pulse 1
15r x.......   DMC IRQ
    .x......   frame counter IRQ
    ...x....   sample DMA active
    ....x...   noise on
    .....x..   triangle on
    ......x.   pulse 2 on
    .......x   pulse 1 on
16  ........
17  x.......   frame counter speed
    .x......   disable frame IRQ
```

## Notes

- you must set the sweep direction bit on the pulse channels in order to use periods greater than $3FF.
- disabling length counter but not envelope will result in a looping envelope.
- loading the length counter resets phase.
