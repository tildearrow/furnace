.include "spc.i"

.BANK 2 SLOT 1
.ORGA $0400

; tick rate (*1000)
.include "songSpeed.s"
tickRateTimer=8000000/initTickRate
tickRateFraction=(256*(8000000#initTickRate))/initTickRate

; sample data goes here...
sampleData:
dir:
  .incbin "sample.bin"

waveRAM=dir+$20
actualDir=waveRAM+$480

; song data (instruments, sequence, tables and system config)
.include "songData.s"

; SNES dispatch code
.include "snes.s"

; command stream player definition
FCS_MAX_CHAN=8
FCS_PORTA_SPEED_MULT=4

fcsAddrBase=$24
fcsZeroPage=$12
fcsGlobalStack=$200
fcsPtr=songCmdStream0
fcsVolMax=volMaxArray

fcsCmdTableLow=fcsCmdTableSNESLow
fcsCmdTableHigh=fcsCmdTableSNESHigh

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

.include "stream.s"

; code

tempoAccum=$3ff

main:
  clrp
  mov x, #$ff
  mov sp, x

  call !divReset
  call !fcsInit

  ; TODO: set the right tempo
  mov spc_timer0, #tickRateTimer
  mov spc_ctrl, #$01

loop:
  mov a, spc_timerRead0
  beq loop

  ; interpolate the timer
  mov a, !tempoAccum
  clrc
  adc a, #tickRateFraction
  mov !tempoAccum, a
  mov a, #tickRateTimer
  bcc +
  inc a
+ mov spc_timer0, a
  call !fcsTick
  call !divTick
  jmp !loop

.BANK 0 SLOT 3
.ORGA $10000

; HEADER

.db "SNES-SPC700 Sound File Data v0.30" ; magic
.db $1a, $1a ; end of text
.db $1a ; $1a = tag present... $1b = no tag present
.db 30 ; version

; SPC REGS

.dw main ; PC
.db 0, 0, 0 ; A, X, Y
.db 0, 0 ; PSW, SP
.db 0, 0 ; reserved

; TAG
.include "songInfo.s"

.BANK 4 SLOT 4
.ORGA $10100

.dsb $5d, 0
.db >dir
