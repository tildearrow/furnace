# hexadecimal

the hexadecimal numeral system differs from the decimal system by having 16 digits rather than 10:

```
hex| decimal
---|---------
 0 | 0
 1 | 1
 2 | 2
 3 | 3
 4 | 4
 5 | 5
 6 | 6
 7 | 7
 8 | 8
 9 | 9
 A | 10
 B | 11
 C | 12
 D | 13
 E | 14
 F | 15
```

when there is more than one digit, these are multiplied by 16, 256, 4096 and so on rather than 10, 100, 1000:

```
hex | decimal
----|---------
 00 | 0
 04 | 4
 08 | 8
 0F | 15
 10 | 16
 11 | 17
 12 | 18
 13 | 19
 20 | 32
 30 | 48
 40 | 64
```

# hex to decimal

for example, take hexadecimal number `AA`:

```
          2nd digit -\   /- 1st digit
                     A   A
 16^1*10 = 16*10 = 160 + 10 = 170
```

now for hexadecimal number `4C5F`:

```

      3rd digit -\    /- 2nd digit
 4th digit -\    |    |    /- 1st digit
            4    C    5    F
            |    |    |    |
            |    |    |    15 =        15 =    15 +
            |    |    \16^1*5 =   16 * 5  =    80
            |    \--- 16^2*12 =  256 * 12 =  3072
            \--------- 16^3*4 = 4096 * 4  = 16384
                                           -------
                                          = 19551
```

# decimal to hex

if it's less than 16, just memorize the table at the top of this document.

otherwise find the power of 16 that is closest to the number you want to convert, but no larger than the number.
then divide, and take the remainder.
divide the remainder with the previous power of 16, until the divider is 1.

for example, for the decimal number `220`:

```
220 ÷ 16 = 13 (r = 12)    D
 12 ÷  1 = 12 (stop here) C

= DC
```

now for decimal number `69420`:

```
69420 ÷ 65536 =  1 (r = 3884)  1
 3884 ÷  4096 =  0 (r = 3884)  0
 3884 ÷   256 = 15 (r =   44)  F
   44 ÷    16 =  2 (r =   12)  2
   12 ÷     1 = 12 (stop here) C

= 10F2C
```
