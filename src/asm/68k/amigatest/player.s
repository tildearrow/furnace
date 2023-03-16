; Furnace validation player code
; this is NOT the ROM export you're looking for!

; incomplete!

VPOSR = $dff004
VHPOSR = $dff006
COLOR00 = $dff180

chipBase=$dff000

DMACONR = $02
POTGOR = $16
DMACON = $96
ADKCON = $9e
AUDBASE = $a0
AUD0LCH = $a0
AUD0LCL = $a2
AUD0LEN = $a4
AUD0PER = $a6
AUD0VOL = $a8
AUD0DAT = $aa
AUD1VOL = $b8
AUD2VOL = $c8
AUD3VOL = $d8

code_c
init:
  move.b #2,$bfe001
  lea chipBase,a0
  move.w #15,DMACON(a0)
waitCon:
  move.w DMACONR(a0),d0
  andi.w #15,d0
  bne waitCon

  move.w #$8200,DMACON(a0)
  move.w #$40,AUD0VOL(a0)
  move.w #$40,AUD1VOL(a0)
  move.w #$40,AUD2VOL(a0)
  move.w #$40,AUD3VOL(a0)
  lea seqAddr(pc),a0
  lea sequence(pc),a1
  move.l a1,(a0)
main:
  bsr waitVBlank

  move.w #$000,d4
  move.w d4,COLOR00

  lea chipBase,a0
  btst.b #2,POTGOR(a0)
  bne next

  lea state(pc),a0
  move.w #1,2(a0)

next:
  bsr nextTick

  lea state(pc),a0
  tst.w 2(a0)
  beq main
finish:
  lea chipBase,a0
  move.w #15,DMACON(a0)
  clr.l d0
  rts

waitVBlank:
  move.l (VPOSR),d0
  and.l #$1ff00,d0
  cmp.l #$bc00,d0
  bne waitVBlank
waitVBlank2:
  move.l (VPOSR),d0
  and.l #$1ff00,d0
  cmp.l #$bd00,d0
  bne waitVBlank2
  rts

nextTick:
  lea state(pc),a4
  move.w (a4),d0
  subi.w #1,d0
  bmi nextTick0
  move.w d0,(a4)
  rts
nextTick0:
  move.l seqAddr(pc),a2
nextTick1:
  ; get next command
  clr.w d0
  move.b (a2)+,d0
  move.w #$0ff,d4
  move.w d4,COLOR00

testSpecial:
  cmp.w #$f0,d0
  bmi testChannel

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
  lsl.w #8,d2
  or.b (a2)+,d2
  move.w d2,(a4)
  bra endTick
testF6:
  ; f6 - write DMACON
  cmp.b #$f6,d0
  bne testFE
  move.w #$f00,d4
  move.w d4,COLOR00
  clr.w d2
  move.b (a2)+,d2
  lsl.w #8,d2
  or.b (a2)+,d2
  move.w d2,chipBase+DMACON
  ; wait for DMACON to be done
  move.b (VHPOSR),d0
dmaConWait:
  cmp.b (VHPOSR),d0
  beq dmaConWait
  ; wait for DMACON to be done -2
  move.b (VHPOSR),d0
dmaConWait1:
  cmp.b (VHPOSR),d0
  beq dmaConWait1
  bra nextTick1
testFE:
  ; fe - write ADKCON
  cmp.b #$fe,d0
  bne testFF
  clr.w d2
  move.b (a2)+,d2
  lsl.w #8,d2
  or.b (a2)+,d2
  move.w d2,chipBase+ADKCON
  bra nextTick1
testFF:
  ; ff - end of song
  cmp.b #$ff,d0
  bne testOther
theEnd:
  lea sequence(pc),a2
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
  move.w #$f0f,d4
  move.w d4,COLOR00
  clr.l d2
  move.b (a2)+,d2
  lsl.l #8,d2
  or.b (a2)+,d2
  lsl.l #8,d2
  or.b (a2)+,d2
  lea sampleData,a0
  add.l a0,d2
  lea chipBase,a0
  move.b d1,d0
  andi.l #$ff,d0
  addi.l #$a0,d0
  adda.l d0,a0
  move.l d2,(a0)+ ; location
  clr.w d2
  move.b (a2)+,d2
  lsl.w #8,d2
  or.b (a2)+,d2
  move.w d2,(a0) ; length
  bra nextTick1
chanNotZero:
  ; check for 8 (VOL)
  cmp.b #8,d0
  bne chanWaveChange
  ; write volume
  clr.w d2
  move.b (a2)+,d2
  bra chanWrite
chanWaveChange:
  ; check for 1 (wave change)
  cmp.b #1,d0
  bne chanOther
  ; copy wave
  clr.l d2
  move.b (a2)+,d2
  lsl.l #8,d2
  or.b (a2)+,d2
  lsl.l #8,d2
  or.b (a2)+,d2
  add.l #wavetable,d2
  move.l d2,a0

  lea sampleData,a1
  andi.l #$30,d1
  lsl.l #4,d1
  adda.l d1,a1

  clr.l d2
  move.b (a2)+,d2
  lsl.l #8,d2
  or.b (a2)+,d2

  ; don't copy a zero-length wave
  tst.l d2
  beq nextTick1
copyWave:
  move.w (a0)+,(a1)+
  subq.l #2,d2
  bne copyWave

  bra nextTick1
chanOther:
  ; get value and write
  clr.w d2
  move.b (a2)+,d2
  lsl.w #8,d2
  or.b (a2)+,d2
chanWrite:
  move.w #$ff0,d4
  move.w d4,COLOR00
  lea chipBase,a0
  or.b d1,d0
  addi.b #AUDBASE,d0
  andi.l #$ff,d0
  adda.l d0,a0
  move.w d2,(a0)
invalidCmd:
  bra nextTick1

endTick:
  lea seqAddr(pc),a3
  move.l a2,(a3)
  move.w #$000,d4
  move.w d4,COLOR00
  rts

data_c
  cnop 0,4

curColor:
  dc.w 0

state:
  dc.w 0 ; ticks
  dc.w 0 ; quit

  cnop 0,4

seqAddr:
  dc.l 0
  
  cnop 0,4

sequence:
  incbin "seq.bin"

  cnop 0,4

sampleData:
  incbin "sample.bin"

  cnop 0,4

wavetable:
  incbin "wave.bin"

