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
divChanSampleNoteDelta=divBase+(divChans*6)+1 ; unsigned char
; this is a pointer...
divChanIns=divBase+(divChans*8) ; short
; flags:
; - bit 7: active
; - bit 6: insChanged
; - bit 5: freqChanged
; - bit 4: fixedArp
; - bit 3: keyOn
; - bit 2: keyOff
; - bit 1: inPorta
; - bit 0: rawFreq
divChanVol=divBase+(divChans*10) ; unsigned char
divChanOutVol=divBase+(divChans*10)+1 ; unsigned char
divChanPitchTablePtr=divBase+(divChans*12) ; short
divChanFlags=divBase+(divChans*14) ; unsigned char

;;;; ---- DivPlatformSNES::Channel ---- ;;;;
; condensed to a short
; setPos is true if this is not $ffff
divChanAudPos=divBase+(divChans*16) ; short
; shared with divChanWave
divChanSample=divBase+(divChans*18) ; short
divChanWave=divChanSample
; upper bit: invert
divChanPanL=divBase+(divChans*20) ; unsigned char
; upper bit: invert
divChanPanR=divBase+(divChans*20)+1 ; unsigned char
divChanWtLen=divBase+(divChans*22) ; unsigned char
; flags:
; - bit 7: useWave
; - bit 6: noise
; - bit 5: echo
; - bit 4: pitchMod
; - bit 3: shallWriteVol
; - bit 2: shallWriteEnv
; - bit 0-1: sustain mode
divChanSNESFlags=divBase+(divChans*22)+1 ; unsigned char
; I guess we can load D2 from the instrument. there's no command to change it so yeah
; XDDDAAAA (X: ADSR on)
divChanAD=divBase+(divChans*24) ; unsigned char
; SSSRRRRR
divChanSR=divBase+(divChans*24)+1 ; unsigned char
; dang it.
divChanGain=divBase+(divChans*26) ; unsigned char
divChanD2=divBase+(divChans*26)+1 ; unsigned char

;;;; ---- DivPlatformSNES (global state) ---- ;;;;
divGlobalBase=divBase+(divChans*28)
divGlobalVolL=divGlobalBase ; unsigned char
divGlobalVolR=divGlobalBase+1 ; unsigned char

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

divDefaultIns:
  .dsb 11, 0

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

; dspWrite <addr> <data>
; write a static value into DSP register.
; alters A and Y.
.MACRO dspWrite
  mov spc_dspAddr, #\1
  mov spc_dspData, #\2
.ENDM

; run a tick.
divTick:
  ret

; void DivPlatformSNES::writeOutVol(int ch);
; - x: ch (<<1)
divWriteOutVol:
  ; outVol*panL (rol will cancel the invert flag)
  mov a, !divChanPanL+x
  clrc
  rol a
  mov y, a
  mov a, !divChanOutVol+x
  mul ya
  ; rounding
  and a, #$ff
  bne +
  inc y
  ; (previousResult)*globalVolL
+ mov a, !divGlobalVolL
  clrc
  rol a
  mul ya
  ; rounding
  and a, #$ff ; test A
  beq +
  inc y
+ push y

  ; outVol*panR (rol will cancel the invert flag)
  mov a, !divChanPanR+x
  clrc
  rol a
  mov y, a
  mov a, !divChanOutVol+x
  mul ya
  ; rounding
  and a, #$ff
  bne +
  inc y
  ; (previousResult)*globalVolR
+ mov a, !divGlobalVolR
  clrc
  rol a
  mul ya
  ; rounding
  and a, #$ff ; test A
  beq +
  inc y
  
  ; apply invert flag if necessary
  ; then perform writes
+ mov a, !divChanPanR+x
  bpl +
  mov a, y
  eor a, #$ff
  mov y, a
+ chWriteX 1

  pop y
  mov a, !divChanPanL+x
  bpl +
  mov a, y
  eor a, #$ff
  mov y, a
+ chWriteX 0
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

; void DivPlatformSNES::reset()
divReset:
  ; set the sample table base
  ; I expect the dir to sit at $400
  dspWrite dsp_DIR, #$04
  dspWrite dsp_MVOL_L, #$7f
  dspWrite dsp_MVOL_R, #$7f
  dspWrite dsp_FLG, #0
  
  ; clear state memory
  mov y, #(divGlobalBase-divBase).b
  mov a, #0
- mov !divBase+y, a
  dbnz y, -

  ; initialize channel state
  ; why didn't they add a more complete movw ya?
  ; only direct page? no index? no absolute?

  ; baseFreq
.ifdef DIV_LINEAR_FREQ
  mov a, #$1e
  mov x, #(divChans-1)*2+1 ; upper byte
- mov !divChanBaseFreq+x, a
  dec x
  dec x
  bpl -
.endif

  ; ins
  ; this is slow
  mov x, #(divChans-1)*2+1 ; upper byte
- mov a, #<divDefaultIns
  mov !divChanIns+x, a
  dec x
  mov a, #>divDefaultIns
  mov !divChanIns+x, a
  dec x
  bpl -

  ; vol/outVol/panL/panR
  mov a, #$7f
  mov x, #(divChans-1)*2+1 ; upper byte
- mov !divChanVol+x, a
  mov !divChanPanL+x, a
  dec x
  mov !divChanVol+x, a
  mov !divChanPanL+x, a
  dec x
  bpl -

  ; wtLen
  mov a, #16
  mov x, #(divChans-1)*2
- mov !divChanWtLen+x, a
  dec x
  dec x
  bpl -


  ; assign default pitch table

  ; writeOutVol

  ; set source number

  ; default global state

  ret

; --- COMMAND HANDLERS ---
