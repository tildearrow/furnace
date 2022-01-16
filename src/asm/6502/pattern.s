patPos=$90
tempByte=$80

nextNote=$a0
nextIns=$a1
nextVolume=$b0
nextRow=$b1
nextEffect1=$c0
nextEffect2=$d0
nextEffect3=$e0
nextEffect4=$f0

.macro nextByte
  lda (patPos,x)
  inc patPos,x
  bcc :+
  inc patPos+1,x
: clc
.endmacro
  
; read next row.
; load X with channel<<1
readNext:
  lda #$ff
  sta nextNote,x   ; clear last values
  sta nextIns,x
  sta nextVolume,x
  sta nextEffect1,x
  sta nextEffect2,x
  sta nextEffect3,x
  sta nextEffect4,x
  nextByte
  sta tempByte ; temporary store
  beq endOfPat
  lda #$20     ; start pattern check
  bit tempByte ; NVI..EEE
  bpl :+
  php          ; read note
  nextByte
  sta nextNote,x
  plp
: bvc :+
  php          ; read volume
  nextByte
  sta nextVolume,x
  plp
: bne :+
  nextByte     ; read instrument
  sta nextIns,x
: lda tempByte
  and #7
  tay
  txa
  pha
  clv
readEffectLoop:
  dey                 ; check if we're done
  beq readLength
  nextByte            ; effect
  sta nextEffect1,x
  nextByte            ; effect val
  sta nextEffect1+1,x
  txa                 ; add 16 to offset
  adc #$10
  tax
  bvc readEffectLoop
readLength:
  pla
  tax
  nextByte
  sta nextRow,x
endOfPat:
  rts
