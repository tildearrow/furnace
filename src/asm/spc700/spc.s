.include "spc.i"

.BANK 2 SLOT 1
.ORGA $0400

; command stream player definition
FCS_MAX_CHAN=8

fcsAddrBase=$16
fcsZeroPage=$02
fcsGlobalStack=$200
fcsPtr=cmdStream
fcsVolMax=volMaxArray

fcsCmdTableLow=fcsCmdTableExample
fcsCmdTableHigh=fcsCmdTableExample

dir:
  .dsw 8, 0

fcsDummyFunc:
  ret

volMaxArray:
  .db $7f, $00
  .db $7f, $00
  .db $7f, $00
  .db $7f, $00
  .db $7f, $00
  .db $7f, $00
  .db $7f, $00
  .db $7f, $00

fcsCmdTableExample:
  .dsb 256, 0

.include "stream.s"

curTick=$00
curRow=$01
ptr=$02
nextKOn=$04

.MACRO dspWrite
  mov spc_dspAddr, #\1
  mov spc_dspData, #\2
.ENDM

main:
  clrp
  mov x, #$ff
  mov sp, x

  call !fcsInit

  mov spc_timer0, #160
  mov spc_ctrl, #$01

loop:
  mov a, spc_timerRead0
  beq loop
  call !fcsTick
  bra loop

cmdStream:
  .incbin "seq.bin"

.BANK 0 SLOT 3
.ORGA $10000

; HEADER

.db "SNES-SPC700 Sound File Data v0.30" ; magic
.db $1a, $1a ; end of text
.db $1b ; $1a = tag present... $1b = no tag present
.db 30 ; version

; SPC REGS

.dw main ; PC
.db 0, 0, 0 ; A, X, Y
.db 0, 0 ; PSW, SP
.db 0, 0 ; reserved

; TAG (TODO)

.BANK 4 SLOT 4
.ORGA $10100

.dsb $5d, 0
.db >dir
