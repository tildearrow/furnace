; Furnace validation player code
; this is NOT the ROM export you're looking for!

; incomplete!

VPOSR = $dff004
COLOR00 = $dff180

cseg
  move.l #0,d0

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
