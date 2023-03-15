; Furnace validation player code
; this is NOT the ROM export you're looking for!

; incomplete!

VPOSR = $dff004
COLOR00 = $dff180

chipBase=$dff000

DMACONR = $02
DMACON = $96
ADKCON = $9e
AUDBASE = $a0
AUD0LCH = $a0
AUD0LCL = $a2
AUD0LEN = $a4
AUD0PER = $a6
AUD0VOL = $a8
AUD0DAT = $aa

code_c
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

testF1:
  ; f1 - next tick
  cmp.b #$f1,d0
  bne testF2
  ; end of tick
  move.w #0,(a4)
  bra endTick
testF2:
  ; f2 - wait (char)
  cmp.b #$f2,d0
  bne testF3
  move.b (a2)+,d0
  andi.w #$ff,d0
  move.w d0,(a4)
  bra endTick
testF3:
  ; f3 - wait (short)
  cmp.b #$f3,d0
  bne testF6
  clr.w d2
  move.b (a2)+,d2
  rol.w #8,d2
  or.b (a2)+,d2
  move.w d2,(a4)
  bra endTick
testF6:
  ; f6 - write DMACON
  cmp.b #$f6,d0
  bne testFE
  clr.w d2
  move.b (a2)+,d2
  rol.w #8,d2
  or.b (a2)+,d2
  move.w d2,chipBase+DMACON
  bra nextTick1
testFE:
  ; fe - write ADKCON
  cmp.b #$fe,d0
  bne testFF
  clr.w d2
  move.b (a2)+,d2
  rol.w #8,d2
  or.b (a2)+,d2
  move.w d2,chipBase+ADKCON
  bra nextTick1
testFF:
  ; ff - end of song
  cmp.b #$ff,d0
  bne testOther
  bra nextTick1
testOther:
  ; something else
  bra nextTick1

testChannel:
  cmp.b #$40,d0
  bge invalidCmd
  ; process channel
  move.b d0,d1
  andi.b #15,d0
  ; check for 0
  bne chanNotZero
  ; write loc/len
  clr.l d2
  move.b (a2)+,d2
  rol.w #8,d2
  or.b (a2)+,d2
  rol.w #8,d2
  or.b (a2)+,d2
  add.l sampleData(pc),d2
  lea chipBase,a0
  or.b d1,d0
  addi.b #AUDBASE,d0
  andi.w #$ff,d0
  adda.w d0,a0
  move.l d2,(a0) ; location
  clr.w d2
  move.b (a2)+,d2
  rol.w #8,d2
  or.b (a2)+,d2
  adda.w #4,a0
  move.l d2,(a0) ; length
  bra nextTick1
chanNotZero:
  ; check for 8 (VOL)
  cmp.b #8,d0
  bne chanOther
  ; write volume
  clr.w d2
  move.b (a2)+,d2
  bra chanWrite
chanOther:
  ; get value and write
  clr.w d2
  move.b (a2)+,d2
  rol.w #8,d2
  or.b (a2)+,d2
chanWrite:
  lea chipBase,a0
  or.b d1,d0
  addi.b #AUDBASE,d0
  andi.w #$ff,d0
  adda.w d0,a0
  move.w d2,(a0)
invalidCmd:
  bra nextTick1

endTick:
  lea seqAddr(pc),a3
  move.l a2,(a3)
  rts

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
