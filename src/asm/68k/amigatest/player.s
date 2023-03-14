; Furnace validation player code
; this is NOT the ROM export you're looking for!

; incomplete!

VPOSR = $dff004
COLOR00 = $dff180
DMACONR = $dff002
DMACON = $dff096
AUD0LCH = $dff0a0
AUD0LCL = $dff0a2
AUD0LEN = $dff0a4
AUD0PER = $dff0a6
AUD0VOL = $dff0a8
AUD0DAT = $dff0aa

code_c
  move.w #15,d0
  move.w d0,DMACON

testDMACon:
  move.w DMACON,d0
  btst #0,d0
  bne testDMACon

  lea sampleData(pc),a0
  move.l a0,AUD0LCH
  move.w #$2000,d0
  move.w d0,AUD0LEN
  move.w #$a0,d0
  move.w d0,AUD0PER
  move.w #$40,d0
  move.w d0,AUD0VOL
  move.l #$8201,d0
  move.w d0,DMACON

main:
  jsr waitVBlank
  
  move.w curColor,d0
  move.w d0,COLOR00
  addi.w #1,d0
  move.w d0,curColor

  jmp main

waitVBlank:
  move.l (VPOSR),d0
  and.l #$1ff00,d0
  cmp.l #$12c00,d0
  bne waitVBlank
  rts

data_c

curColor:
  dc.w 0

sampleData:
  incbin "sample.bin"

data_f

sequence:
  incbin "seq.bin"

wavetable:
  incbin "wave.bin"
