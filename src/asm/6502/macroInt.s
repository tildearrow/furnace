macroState=$50 ; pointer to state
macroAddr=$52  ; pointer to address

; macro state takes 4 bytes
; macroPos bits:
; 7: had
; 6: will

; x: macro
macroIntRun:
  lda macroAddr,x
  ora macroAddr+1,x
  beq :+

  ; do macro
: rts

; set the macro address, then call
; x: macro
macroIntInit:
  lda #0
  sta macroState,x
  sta macroPos,x
  txa
  rol
  tax
  lda macroAddr,x
  ora macroAddr+1,x
  beq :+
  lda #$40
  sta macroState,x
: rts
