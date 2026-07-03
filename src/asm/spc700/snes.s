; SNES DivDispatch code.

; addresses
; TODO: theoretically divBase could be $100...
divTempPtr=$00
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
divChanVol=divBase+(divChans*10) ; unsigned char
divChanOutVol=divBase+(divChans*10)+1 ; unsigned char
divChanPitchTablePtr=divBase+(divChans*12) ; short
; flags:
; - bit 7: active
; - bit 6: insChanged
; - bit 5: freqChanged
; - bit 4: fixedArp
; - bit 3: keyOn
; - bit 2: keyOff
; - bit 1: inPorta
; - bit 0: rawFreq
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
; - bit 5: echoOn
; - bit 0-4: noiseFreq
divNoiseFreq=divGlobalBase+2 ; unsigned char
divEchoVolL=divGlobalBase+3 ; signed char
divEchoVolR=divGlobalBase+4 ; signed char
divEchoFeedback=divGlobalBase+5 ; signed char
divEchoFIR=divGlobalBase+6 ; signed char[8]
divEchoDelay=divGlobalBase+14 ; unsigned char
divDryVolL=divGlobalBase+15 ; signed char
divDryVolR=divGlobalBase+16 ; signed char
; - bit 7: writeControl
; - bit 6: writeNoise
; - bit 5: writePitchMod
; - bit 4: writeEcho
; - bit 3: writeDryVol
; - bit 1: antiClick
divWriteFlags=divGlobalBase+17 ; unsigned char

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

; dspWriteA <addr>
; write the value of A into DSP register.
.MACRO dspWriteA
  mov spc_dspAddr, #\1
  mov spc_dspData, a
.ENDM

; void DivPlatformSNES::tick();
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

; void DivPlatformSNES::initEcho()
divInitEcho:
  mov a, !divNoiseFreq
  and a, #$04 ; echoOn
  bne echoIsOn ; because the code for on is too long
  echoIsOff:
    dspWrite $2c, #0
    dspWrite $3c, #0
    mov a, !divNoiseFreq ; control
    dspWriteA $6c
    dspWrite $7d, #0
    dspWrite $6d, #$ff
    ret
  echoIsOn:
    ; calculate ESA
    mov a, #$1f
    setc
    sbc a, !divEchoDelay
    clrc
    rol a
    rol a
    rol a
    dspWriteA $6d
    mov a, !divEchoDelay
    dspWriteA $7d
    mov a, !divEchoFeedback
    dspWriteA $0d
    mov a, !divEchoVolL
    dspWriteA $2c
    mov a, !divEchoVolR
    dspWriteA $3c
    ; write FIR (unrolled)
    mov a, !divEchoFIR+7
    dspWriteA $7f
    mov a, !divEchoFIR+6
    dspWriteA $6f
    mov a, !divEchoFIR+5
    dspWriteA $5f
    mov a, !divEchoFIR+4
    dspWriteA $4f
    mov a, !divEchoFIR+3
    dspWriteA $3f
    mov a, !divEchoFIR+2
    dspWriteA $2f
    mov a, !divEchoFIR+1
    dspWriteA $1f
    mov a, !divEchoFIR+0
    dspWriteA $0f
    ret

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

  ; ins/default pitch table
  ; this is slow
  mov x, #(divChans-1)*2+1 ; upper byte
- mov a, #<divDefaultIns
  mov !divChanIns+x, a
  mov a, !songPitchListHigh0+1 ; wavePitchTable[1]
  mov !(divChanPitchTablePtr+1)+x, a
  dec x
  mov a, #>divDefaultIns
  mov !divChanIns+x, a
  mov a, !songPitchListLow0+1 ; wavePitchTable[1]
  mov !divChanPitchTablePtr+x, a
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

  ; writeOutVol/set source number
  mov x, #0
- call !divWriteOutVol
  mov x, a
  lsr a
  mov a, y
  chWriteX 4 ; source number
  inc x
  inc x
  cmp x, #divChans*2
  bne -

  ; initial global state
  mov x, #17
- mov a, !songChipData0+x
  mov !divGlobalBase+x, a
  dec x
  bpl -

  ; load initial echo mask
  mov a, !songChipData0+18
  mov x, #(divChans*2)-2
- ror a ; test the lowest bit
  bcc +
  ; set echo
  mov y, a
  mov a, #$20
  mov !divChanSNESFlags+x, a
  mov a, y
+ dec x
  dec x
  bpl -

  ; finish up
  call !divInitEcho
  ret

; --- COMMAND HANDLERS ---
divCmdNoteOn:
  ; DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_SNES);
  mov a, !divChanIns+x
  mov y, !(divChanIns+1)+x
  movw divTempPtr, ya
  ; if (ins->amiga.useWave)
  mov y, #4
  mov a, [divTempPtr]+y ; TODO: optimize by comparing here...
  cmp a, #2
  bne @noUseWave
  @useWave:
    ; chan[c.chan].useWave=true;
    mov a, !divChanSNESFlags+x
    or a, #$80
    mov !divChanSNESFlags+x, a
    ; chan[c.chan].sampleNoteDelta=0;
    mov a, #0
    mov !divChanSampleNoteDelta+x, a
    ; chan[c.chan].wtLen=ins->amiga.waveLen+1;
    mov y, #5
    mov a, [divTempPtr]+y
    mov !divChanWtLen+x, a
    ; chan[c.chan].pitchTable=&wavePitchTable[((chan[c.chan].wtLen>>4)-1)&15];
    mov y, a
    mov a, !songPitchListLow0+y
    mov !divChanPitchTablePtr+x, a
    mov a, !songPitchListHigh0+y
    mov !(divChanPitchTablePtr+1)+x, a
    ; if (chan[c.chan].insChanged)
    mov a, !divChanFlags+x
    and a, #$40
    beq @@insNotChanged
    @@insChanged:
      ; setWidth/changeWave1 HERE...
    @@insNotChanged:
      ; ws.init HERE...
    jmp !@post1
  @noUseWave:
    mov a, fcsArg0+1
    bne @@noteNotNull
    mov a, fcsArg0
    cmp a, #$b8 ; 180 - null
    beq @@postNoteNotNull
    @@noteNotNull:
      ; assign sample and pitch table
      
    @@postNoteNotNull:
    ; chan[c.chan].useWave=false;
    mov a, !divChanSNESFlags+x
    and a, #~$80
    mov !divChanSNESFlags+x, a
    ; man this is so boring...
  @post1:
  ret