the following document describes the stack machine used to get the correct plural form in ngettext.

# instructions

```
op | description
---|--------------------
00 | end
01 | push imm
02 | push N
03 | add
04 | sub
05 | mul
06 | div
07 | mod
08 | cmp eq
09 | cmp ne
0a | cmp gt
0b | cmp lt
0c | cmp ge
0d | cmp le
0e | cmp and
0f | cmp or
10 | beq off
11 | bne off
12 | exit
```
