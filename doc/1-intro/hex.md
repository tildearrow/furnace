# hexadecimal

the hexadecimal numeral system differs from the decimal system by having 16 digits rather than 10:

hex | dec
:--:|---:
`0` | 0
`1` | 1
`2` | 2
`3` | 3
`4` | 4
`5` | 5
`6` | 6
`7` | 7
`8` | 8
`9` | 9
`A` | 10
`B` | 11
`C` | 12
`D` | 13
`E` | 14
`F` | 15

when there is more than one digit, these are multiplied by 16, 256, 4096 and so on rather than 10, 100, 1000:

 hex | dec
:---:|---:
`00` | 0
`04` | 4
`08` | 8
`0F` | 15
`10` | 16
`11` | 17
`12` | 18
`13` | 19
`20` | 32
`30` | 48
`40` | 64

## hex to decimal

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

## decimal to hex

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

## bitmask

a bitmask is a value that actually represents a set of individual binary bits to be toggled, some of which may be grouped to form small binary numbers. these are used by adding up the value of each bit and converting the result to hex. in macros, these are shown in decimal.

bit   |   binary    | decimal
:----:|:-----------:|--------:
bit 7 | `1000 0000` | 128
bit 6 | `0100 0000` |  64
bit 5 | `0010 0000` |  32
bit 4 | `0001 0000` |  16
bit 3 | `0000 1000` |   8
bit 2 | `0000 0100` |   4
bit 1 | `0000 0010` |   2
bit 0 | `0000 0001` |   1

for example, to turn bits 7 and 5 on, and put `110` (decimal 6) in bits 1 to 3:

bit   |   binary    | decimal
:----:|:-----------:|--------:
bit 7 | `1... ....` | 128
bit 6 | `.0.. ....` |   0
bit 5 | `..1. ....` |  32
bit 4 | `...0 ....` |   0
bit 3 | `.... 1...` |   8
bit 2 | `.... .1..` |   4
bit 1 | `.... ..0.` |   0
bit 0 | `.... ...0` |   0

added together, the resulting binary is `1010 1100`, which converts to hex as `AC` and to decimal as 172.

## hex to decimal table

  hex | `0` | `1` | `2` | `3` | `4` | `5` | `6` | `7` | `8` | `9` | `A` | `B` | `C` | `D` | `E` | `F`
-----:|----:|----:|----:|----:|----:|----:|----:|----:|----:|----:|----:|----:|----:|----:|----:|----:
 `00` |   0 |   1 |   2 |   3 |   4 |   5 |   6 |   7 |   8 |   9 |  10 |  11 |  12 |  13 |  14 |  15
 `10` |  16 |  17 |  18 |  19 |  20 |  21 |  22 |  23 |  24 |  25 |  26 |  27 |  28 |  29 |  30 |  31
 `20` |  32 |  33 |  34 |  35 |  36 |  37 |  38 |  39 |  40 |  41 |  42 |  43 |  44 |  45 |  46 |  47
 `30` |  48 |  49 |  50 |  51 |  52 |  53 |  54 |  55 |  56 |  57 |  58 |  59 |  60 |  61 |  62 |  63
 `40` |  64 |  65 |  66 |  67 |  68 |  69 |  70 |  71 |  72 |  73 |  74 |  75 |  76 |  77 |  78 |  79
 `50` |  80 |  81 |  82 |  83 |  84 |  85 |  86 |  87 |  88 |  89 |  90 |  91 |  92 |  93 |  94 |  95
 `60` |  96 |  97 |  98 |  99 | 100 | 101 | 102 | 103 | 104 | 105 | 106 | 107 | 108 | 109 | 110 | 111
 `70` | 112 | 113 | 114 | 115 | 116 | 117 | 118 | 119 | 120 | 121 | 122 | 123 | 124 | 125 | 126 | 127
 `80` | 128 | 129 | 130 | 131 | 132 | 133 | 134 | 135 | 136 | 137 | 138 | 139 | 140 | 141 | 142 | 143
 `90` | 144 | 145 | 146 | 147 | 148 | 149 | 150 | 151 | 152 | 153 | 154 | 155 | 156 | 157 | 158 | 159
 `A0` | 160 | 161 | 162 | 163 | 164 | 165 | 166 | 167 | 168 | 169 | 170 | 171 | 172 | 173 | 174 | 175
 `B0` | 176 | 177 | 178 | 179 | 180 | 181 | 182 | 183 | 184 | 185 | 186 | 187 | 188 | 189 | 190 | 191
 `C0` | 192 | 193 | 194 | 195 | 196 | 197 | 198 | 199 | 200 | 201 | 202 | 203 | 204 | 205 | 206 | 207
 `D0` | 208 | 209 | 210 | 211 | 212 | 213 | 214 | 215 | 216 | 217 | 218 | 219 | 220 | 221 | 222 | 223
 `E0` | 224 | 225 | 226 | 227 | 228 | 229 | 230 | 231 | 232 | 233 | 234 | 235 | 236 | 237 | 238 | 239
 `F0` | 240 | 241 | 242 | 243 | 244 | 245 | 246 | 247 | 248 | 249 | 250 | 251 | 252 | 253 | 254 | 255
