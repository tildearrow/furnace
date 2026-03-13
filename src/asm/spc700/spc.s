.include "spc.i"

.BANK 2 SLOT 1
.ORGA $0200

curTick=$00
curRow=$01
ptr=$02
nextKOn=$04

.MACRO dspWrite
  mov spc_dspAddr, #\1
  mov spc_dspData, #\2
.ENDM

dir:
  .db <piano, >piano
  .db <piano, >piano
  .dsw 255, 0

main:
  clrp
  mov x, #$ff
  mov sp, x

  dspWrite dsp_DIR, #>dir
  dspWrite dsp_FLG, #$20
  dspWrite dsp_MVOL_L, #$7f
  dspWrite dsp_MVOL_R, #$7f

  dspWrite dsp_LVOL, #$20
  dspWrite dsp_RVOL, #$20
  dspWrite dsp_SRCN, #0
  dspWrite dsp_AD, #$ff
  dspWrite dsp_SR, #$f2
  dspWrite dsp_GAIN, #$7f

  dspWrite dsp_LVOL+$10, #$20
  dspWrite dsp_RVOL+$10, #$20
  dspWrite dsp_SRCN+$10, #0
  dspWrite dsp_AD+$10, #$ff
  dspWrite dsp_SR+$10, #$f3
  dspWrite dsp_GAIN+$10, #$7f

  dspWrite dsp_LVOL+$20, #$20
  dspWrite dsp_RVOL+$20, #$20
  dspWrite dsp_SRCN+$20, #0
  dspWrite dsp_AD+$20, #$ff
  dspWrite dsp_SR+$20, #$f3
  dspWrite dsp_GAIN+$20, #$7f

  dspWrite dsp_LVOL+$30, #$20
  dspWrite dsp_RVOL+$30, #$20
  dspWrite dsp_SRCN+$30, #0
  dspWrite dsp_AD+$30, #$ff
  dspWrite dsp_SR+$30, #$f2
  dspWrite dsp_GAIN+$30, #$7f

  mov spc_timer0, #160
  mov spc_ctrl, #$01

loop:
  mov a, spc_timerRead0
  beq loop
  call !nextTick
  bra loop

nextTick:
  dec curTick
  bmi +
  ret
nextRow:
+ mov a, #11
  mov curTick, a

  mov nextKOn, #0

  ; update a channel
  mov x, #0
- call !updateChan
  inc x
  cmp x, #4
  bne -

  ; commit KOF
  mov spc_dspAddr, #dsp_KOF
  mov spc_dspData, #0

  ; wait a bit
  mov x, #3
- dec x
  bpl -

  ; commit KON
  mov spc_dspAddr, #dsp_KON
  mov a, nextKOn
  mov spc_dspData, a

  ; increase current position
  inc curRow
  bpl +
  mov curRow, #0
+ ret

; X: channel
updateChan:
  mov y, curRow
  mov a, !seqChansL+x
  mov ptr, a
  mov a, !seqChansH+x
  mov ptr+1, a

  mov a, [ptr]+y
  bpl +
  ret
+ asl a
  mov y, a

  ; turn channel off and buffer next KON
  mov a, !seqKOn+x
  or a, nextKOn
  mov nextKOn, a

  push x
  mov a, x
  xcn a
  or a, #dsp_FREQL
  mov x, a
  mov spc_dspAddr, a
  mov a, !seqNotes+y
  mov spc_dspData, a
  inc x
  mov a, x
  mov spc_dspAddr, a
  mov a, !(seqNotes+1)+y
  mov spc_dspData, a

  pop x
  ret

piano:
  .db $b0, $77, $77, $66, $55, $53, $33, $22, $10
  .db $94, $0c, $ed, $dc, $ec, $cd, $cf, $dc, $a9

;  0   1   2   3   4   5   6   7   8   9   a
; B-1 C-2 C#2 D#2 E-2 F#2 G#2 A-2 A#2 D-2 F-2
seqNotes:
  .dw  $336,  $367,  $39c,  $40d,  $44b,  $4d0,  $568,  $5ba,  $612,  $3d2,  $48c, 0, 0, 0, 0, 0
  .dw  $66e,  $6d1,  $737,  $819,  $896,  $9a3,  $ad0,  $b74,  $c23,  $7a6,  $917, 0, 0, 0, 0, 0
  .dw  $cdd,  $da0,  $e6f, $1035, $112b, $1346, $15a1, $16eb, $1848,  $f4b, $1231, 0, 0, 0, 0, 0
  .dw $19b9, $1b41, $1cdf, $206a, $2256, $268b, $2b45, $2dd7, $3090, $1e98, $2461, 0, 0, 0, 0, 0

seq0:
  .db $07, $ff, $17, $07, $ff, $17, $07, $ff
  .db $06, $ff, $16, $06, $ff, $16, $06, $ff
  .db $05, $ff, $15, $05, $10, $ff, $20, $10
  .db $08, $ff, $18, $08, $ff, $18, $08, $ff
  .db $07, $ff, $17, $07, $ff, $17, $07, $ff
  .db $06, $ff, $16, $06, $ff, $16, $06, $ff
  .db $05, $ff, $15, $05, $10, $ff, $20, $10
  .db $02, $ff, $12, $02, $ff, $12, $10, $ff

  .db $07, $07, $17, $07, $07, $17, $07, $07
  .db $06, $06, $16, $06, $06, $16, $06, $06
  .db $05, $05, $15, $05, $10, $10, $20, $10
  .db $08, $08, $18, $08, $08, $18, $08, $18
  .db $07, $07, $17, $07, $07, $17, $07, $07
  .db $06, $06, $16, $06, $06, $16, $06, $06
  .db $05, $05, $15, $05, $10, $10, $20, $10
  .db $02, $02, $12, $02, $02, $12, $00, $10

seq1:
  .db $ff, $14, $ff, $ff, $14, $ff, $ff, $15
  .db $ff, $16, $ff, $ff, $16, $ff, $ff, $16
  .db $ff, $19, $ff, $ff, $15, $ff, $ff, $20
  .db $ff, $16, $22, $2a, $16, $22, $23, $16

  .db $ff, $14, $ff, $ff, $14, $ff, $ff, $16
  .db $ff, $16, $ff, $ff, $16, $ff, $ff, $16
  .db $ff, $15, $ff, $ff, $13, $ff, $ff, $15
  .db $ff, $16, $22, $24, $16, $22, $23, $13

  .db $ff, $16, $16, $ff, $16, $16, $ff, $16
  .db $ff, $20, $20, $ff, $20, $20, $ff, $20
  .db $ff, $29, $29, $ff, $29, $29, $ff, $29
  .db $ff, $22, $22, $ff, $22, $22, $ff, $22

  .db $ff, $16, $16, $ff, $16, $16, $ff, $16
  .db $ff, $20, $20, $ff, $20, $20, $ff, $20
  .db $ff, $17, $17, $ff, $20, $20, $ff, $20
  .db $ff, $22, $22, $ff, $22, $22, $ff, $22

seq2:
  .db $12, $16, $ff, $ff, $16, $ff, $ff, $17
  .db $ff, $20, $ff, $ff, $20, $ff, $ff, $20
  .db $ff, $17, $ff, $ff, $20, $ff, $ff, $29
  .db $1a, $15, $16, $20, $22, $20, $16, $1a

  .db $12, $16, $ff, $ff, $16, $ff, $ff, $20
  .db $ff, $20, $ff, $ff, $20, $ff, $ff, $22
  .db $ff, $17, $ff, $ff, $20, $ff, $ff, $17
  .db $14, $15, $16, $17, $20, $17, $16, $15

  .db $ff, $14, $14, $ff, $14, $14, $ff, $14
  .db $ff, $16, $16, $ff, $16, $16, $ff, $16
  .db $ff, $15, $15, $ff, $15, $15, $ff, $15
  .db $ff, $1a, $1a, $ff, $1a, $16, $16, $13

  .db $ff, $14, $14, $ff, $14, $14, $ff, $14
  .db $ff, $16, $16, $ff, $16, $16, $ff, $16
  .db $ff, $15, $15, $ff, $15, $15, $ff, $15
  .db $ff, $14, $14, $ff, $14, $16, $16, $13

seq3:
  .db $22, $ff, $ff, $23, $24, $ff, $32, $ff
  .db $30, $ff, $26, $25, $26, $ff, $22, $ff
  .db $25, $ff, $27, $ff, $30, $ff, $27, $ff
  .db $26, $ff, $32, $ff, $31, $ff, $26, $ff

  .db $24, $23, $22, $23, $24, $ff, $32, $ff
  .db $30, $ff, $26, $25, $26, $ff, $22, $ff
  .db $25, $ff, $27, $ff, $30, $ff, $27, $ff
  .db $26, $ff, $24, $23, $24, $ff, $23, $ff

  .db $22, $ff, $ff, $23, $24, $ff, $32, $ff
  .db $30, $ff, $26, $25, $26, $ff, $22, $ff
  .db $25, $ff, $27, $ff, $30, $ff, $27, $ff
  .db $26, $ff, $32, $ff, $31, $ff, $26, $ff

  .db $24, $23, $22, $23, $24, $ff, $32, $ff
  .db $30, $ff, $26, $25, $26, $ff, $22, $ff
  .db $25, $ff, $27, $ff, $30, $ff, $27, $ff
  .db $26, $ff, $24, $23, $24, $ff, $23, $ff

seqChansH:
  .db >seq0, >seq1, >seq2, >seq3

seqChansL:
  .db <seq0, <seq1, <seq2, <seq3

seqKOn:
  .db 1, 2, 4, 8

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
