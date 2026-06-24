; SNES DivDispatch code.

; addresses
divBase=$300
divChans=8

;;;; ---- SharedChannel ---- ;;;;
; divChanFreq is temporary (we only read it during freq calc).
divChanBaseFreq=divBase ; short
divChanPitch=divBase+(divChans*2) ; short
divChanPitch2=divBase+(divChans*4) ; short
; shared by arpOff and baseNoteOverride
divChanArpState=divBase+(divChans*6) ; unsigned char
divChanIns=divBase+(divChans*6)+1 ; unsigned char
divChanSampleNoteDelta=divBase+(divChans*8) ; unsigned char
; flags:
; - bit 7: active
; - bit 6: insChanged
; - bit 5: freqChanged
; - bit 4: fixedArp
; - bit 3: keyOn
; - bit 2: keyOff
; - bit 1: inPorta
; - bit 0: rawFreq
divChanFlags=divBase+(divChans*8)+1 ; unsigned char
divChanVol=divBase+(divChans*10) ; unsigned char
divChanOutVol=divBase+(divChans*10)+1 ; unsigned char
divChanPitchTablePtr=divBase+(divChans*12) ; short

;;;; ---- DivPlatformSNES::Channel ---- ;;;;
; condensed to a short
; setPos is true if this is not $ffff
divChanAudPos=divBase+(divChans*14) ; short
; shared with divChanWave
divChanSample=divBase+(divChans*16) ; short
divChanWave=divChanSample
; upper bit: invert
divChanPanL=divBase+(divChans*18) ; unsigned char
; upper bit: invert
divChanPanR=divBase+(divChans*18)+1 ; unsigned char
divChanWtLen=divBase+(divChans*20) ; unsigned char
; flags:
; - bit 7: useWave
; - bit 6: noise
; - bit 5: echo
; - bit 4: pitchMod
; - bit 3: shallWriteVol
; - bit 2: shallWriteEnv
; - bit 0-1: sustain mode
divChanSNESFlags=divBase+(divChans*20)+1 ; unsigned char
; I guess we can load D2 from the instrument. there's no command to change it so yeah
; XDDDAAAA (X: ADSR on)
divChanAD=divBase+(divChans*22) ; unsigned char
; SSSRRRRR
divChanSR=divBase+(divChans*22)+1 ; unsigned char
; dang it.
divChanGain=divBase+(divChans*24) ; unsigned char
divChanD2=divBase+(divChans*24)+1 ; unsigned char

;;;; ---- LOOK-UP TABLES ---- ;;;;
divChanOffs:
  .db $00, $00
  .db $10, $10
  .db $20, $20
  .db $30, $30
  .db $40, $40
  .db $50, $50
  .db $60, $60
  .db $70, $70

; chWriteX <REG>
; writes Y into channel X's register REG.
; alters A and PSW(C).
.MACRO chWriteX
  ; TODO: optimize?
  mov a, !divChanOffs+x
  clrc
  adc a, #\1
  mov spc_dspAddr, a
  mov spc_dspData, y
.ENDM

; run a tick.
divTick:
  ret

; void DivPlatformSNES::writeEnv(int ch);
; - x: ch (<<1)
divWriteEnv:
  mov a, !divChanAD+x
  bpl notUseEnv
  useEnv:
    mov a, !divChanSNESFlags+x
    and a, #3
    mov y, !divChanFlags+x
    bmi +
    or a, #4
    asl a
+   push x
    mov x, a
    ; implemented as a jump table depending on active and ADSR mode.
    jmp [!divWriteEnvSubTable+x]
  notUseEnv:
    ; enter gain mode
    mov y, #0
    chWriteX 5
    ; write current gain
    mov a, !divChanGain+x
    mov y, a
    chWriteX 7
    ret

divWriteEnvDirect:
  ; write ADSR
  pop x
  mov a, !divChanAD+x
  mov y, a
  chWriteX 5
  mov a, !divChanSR+x
  mov y, a
  chWriteX 6
  ret

divWriteEnvActive:
  ; write ADS and D2
  pop x
  mov a, !divChanAD+x
  mov y, a
  chWriteX 5
  mov a, !divChanSR+x
  and a, #$e0
  or a, !divChanD2+x
  mov y, a
  chWriteX 6
  ret

divWriteEnvDecLin:
  ; release - linear
  pop x
  mov a, !divChanSR+x
  and a, #$1f
  or a, #$80
  mov y, a
  chWriteX 7
  mov y, #0
  chWriteX 5
  ret

divWriteEnvDecExp:
  ; release - exponential
  pop x
  mov a, !divChanSR+x
  and a, #$1f
  or a, #$a0
  mov y, a
  chWriteX 7
  mov y, #0
  chWriteX 5
  ret

divWriteEnvDelayed:
  pop x
  mov a, !divChanSR+x
  mov y, a
  chWriteX 6
  ret

; bit 2: active
; bit 0-1: release mode
divWriteEnvSubTable:
  .dw divWriteEnvDirect, divWriteEnvDecLin, divWriteEnvDecExp, divWriteEnvDelayed
  .dw divWriteEnvDirect, divWriteEnvActive, divWriteEnvActive, divWriteEnvActive

; reset.
divReset:
  ret

; --- COMMAND HANDLERS ---
