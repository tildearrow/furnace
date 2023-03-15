; Furnace validation player code
; this is NOT the ROM export you're looking for!

; incomplete!

VPOSR = $dff004
COLOR00 = $dff180

chipBase=$dff000

DMACONR = $02
DMACON = $96
AUD0LCH = $a0
AUD0LCL = $a2
AUD0LEN = $a4
AUD0PER = $a6
AUD0VOL = $a8
AUD0DAT = $aa

code_c
start:
  lea chipBase,a0

  move.w #15,DMACON(a0)

testDMACon:
  move.w DMACONR(a0),d0
  btst #0,d0
  bne testDMACon

  lea sampleData(pc),a1
  move.l a1,AUD0LCH(a0)
  move.w #$2000,d0
  move.w d0,AUD0LEN(a0)
  move.w #$a0,AUD0PER(a0)
  move.w #$40,AUD0VOL(a0)
  move.w #$8201,DMACON(a0)

main:
  jsr waitVBlank

  move.w curColor,d0
  move.w d0,COLOR00
  addi.w #1,d0
  move.w d0,curColor

  jsr nextTick

  jmp main

waitVBlank:
  move.l (VPOSR),d0
  and.l #$1ff00,d0
  cmp.l #$12c00,d0
  bne waitVBlank
  rts

nextTick:
  lea state(pc),a4
  move.w (a4),d0
  subi.w #1,d0
  bmi nextTick1
  move.w d0,(a4)
  rts
nextTick1:
  move.l seqAddr(pc),a2
  ; get next command
  move.b (a2)+,d0

testSpecial:
  cmp.b #$f0,d0
  blt testChannel

  cmp.b #$

testChannel:
  cmp.b #$40,d0
  bge nextTickPost

nextTickPost:
  lea seqAddr(pc),a3
  move.l a2,(a3)
  bra nextTick1

data_c

curColor:
  dc.w 0

state:
  dc.w 0 ; ticks

seqAddr:
  dc.l sequence

sampleData:
  incbin "sample.bin"

data_f

sequence:
  incbin "seq.bin"

wavetable:
  incbin "wave.bin"
