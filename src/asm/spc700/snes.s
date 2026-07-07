; SNES DivDispatch code.

; I'm never gonna code again
; guilty rastertime got no demo

; addresses
; TODO: theoretically divBase could be $100...
divTempPtr=$00
divTempPtr1=$02
divTempPtr2=$04
divTempPtr3=$06
divTempPtr4=$08
divTempKeyOn=$0a
divTempKeyOff=$0b
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
divChanOctaveShift=divBase+(divChans*14)+1 ; unsigned char

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

divChanBits:
  .db $01, $01
  .db $02, $02
  .db $04, $04
  .db $08, $08
  .db $10, $10
  .db $20, $20
  .db $40, $40
  .db $80, $80

divDefaultIns:
  .dsb 11, 0

; these two tables are used to split the note and octave.

; subtract note with divNoteSubTable[octave].
; this calculates (note%12) using the octave as an index.
divNoteSubTable:
  .db 0, 12, 24, 36, 48, 60, 72, 84, 96, 108, 120, 132, 144, 156, 168, 180

; note is divided by 4 and then we use this table to calculate the octave.
divOctaveTable:
  .db 0, 0, 0    ; C-(-5)
  .db 1, 1, 1
  .db 2, 2, 2
  .db 3, 3, 3
  .db 4, 4, 4
  .db 5, 5, 5    ; C-0
  .db 6, 6, 6
  .db 7, 7, 7
  .db 8, 8, 8
  .db 9, 9, 9
  .db 10, 10, 10
  .db 11, 11, 11
  .db 12, 12, 12
  .db 13, 13, 13
  .db 14, 14, 14
  ; TODO: we don't need these. the max note is B-9.
  .db 15, 15, 15

;;;; ---- MACROS ---- ;;;;

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

;;;; ---- DISPATCH FUNCTIONS ---- ;;;;

; void DivPlatformSNES::tick();
; run a tick.
divTick:
  ; temporary variables
  mov divTempKeyOn, #0
  mov divTempKeyOff, #0
  ; TODO: macro processing loop here...

  ; frequency processing
  mov x, #0
  @freqLoop:
    ; if (freqChanged || keyOn || keyOff)
    mov a, !divChanFlags+x
    and a, #$2c
    beq @@outOfHere

    ; calculate frequency
    call !divCalcFreq
    ; clamp frequency
    mov a, !divChanFlags+x ; raw freq check
    and a, #1
    bne +
    mov a, !(divChanFreq+1)+x ; >$3fff check
    cmp a, #$3f
    bcc +
    ; do clamp
    mov a, #$ff
    mov !divChanFreq+x, a
    mov a, #$3f
    mov !(divChanFreq+1)+x, a
    @@checkKeyOn:
      ; check for key on
      mov a, !divChanFlags+x
      and a, #$08
      beq @@checkKeyOff
      ; disable key on flag
      mov a, !divChanFlags+x
      and a, #~$08
      mov !divChanFlags+x, a
      ; TODO: process key on...
    @@checkKeyOff:
      ; check for key off
      mov a, !divChanFlags+x
      and a, #$04
      beq @@writeNewFreq
      ; disable key off flag
      mov a, !divChanFlags+x
      and a, #~$04
      mov !divChanFlags+x, a
      ; only set key off if sustain mode is direct
      mov a, !divChanSNESFlags+x
      and a, #3
      bne @@writeNewFreq
      ; go ahead
      mov a, divTempKeyOff
      or a, !divChanBits+x
      mov divTempKeyOff, a
    @@writeNewFreq:
      ; write new frequency if necessary
      mov a, !divChanFlags+x
      and a, #$20
      beq @@outOfHere
      ; disable freqChanged flag
      mov a, !divChanFlags+x
      and a, #~$20
      mov !divChanFlags+x, a
      ; write frequency
      mov a, !divChanFreq+x
      mov y, a
      chWriteX 2
      mov a, !(divChanFreq+1)+x
      mov y, a
      chWriteX 3
    @@outOfHere:
      inc x
      inc x
      cmp x, #(divChans*2)
      beq @post1
      jmp !@freqLoop
  @post1:
  ; check whether we should write key off
  mov a, divTempKeyOff

  ; check writeControl
  ; check writeNoise
  ; check writePitchMod
  ; check writeEcho
  ; check writeDryVol
  ; check writeEnv
  ; write key off zero
  ; check shallWriteVol
  ; check whether we should write key on
  ; it's over
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
  mov a, !songShiftList0+1 ; wavePitchTable[1]
  mov !divChanOctaveShift+x, a
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

;;;; ---- PITCH TABLE FUNCTIONS ---- ;;;;

; calculate frequency and store it in divChanFreq[X].
; - X: channel
; alters A and Y.
divCalcFreq:
  ; check whether we are in raw freq mode
  mov a, !divChanFlags+x
  and #$01
  beq @normalFreq
  @rawFreq:
    ; move pitch2 to direct page so we can call addw
    mov a, !(divChanPitch2+1)+x
    mov y, a
    mov a, !divChanPitch2+x
    movw divTempPtr, ya
    ; get baseFreq
    mov a, !(divChanBaseFreq+1)+x
    mov y, a
    mov a, !divChanBaseFreq+x
    ; add and store
    addw ya, divTempPtr
    mov !divChanFreq+x, a
    mov a, y
    mov !(divChanFreq+1)+x, a
    ret
  @normalFreq:
    ; prepare the offset - add base, pitch and pitch 2
    ; move pitch and pitch2 to direct page
    mov a, !(divChanPitch+1)+x ; pitch
    mov y, a
    mov a, !divChanPitch+x
    movw divTempPtr, ya
    mov a, !(divChanPitch2+1)+x ; pitch2
    mov y, a
    mov a, !divChanPitch2+x
    movw divTempPtr1, ya
    ; load baseFreq and add everything
    mov a, !(divChanBaseFreq+1)+x
    mov y, a
    mov a, !divChanBaseFreq+x
    addw ya, divTempPtr
    addw ya, divTempPtr1
    ; YA contains the offset
.ifdef DIV_LINEAR_FREQ
    ; linear pitch - process the offset
    ; first shift left to get the note and fractional part
    mov divTempPtr+1, y
    ; TODO: is this a true arithmetic shift and will screw up the sign bit?
    asl a
    rol divTempPtr+1
    mov divTempPtr, a
    ; divTempPtr+1 contains the note
    ; divTempPtr contains fraction
    ; split note into note/octave
    mov a, divTempPtr
    lsr a
    lsr a
    mov y, a
    mov a, !divOctaveTable+y
    mov divTempPtr3, a
    mov y, a
    mov a, divTempPtr
    setc
    sbc a, !divNoteSubTable+y
    asl a
    mov y, a
    ; Y contains (note<<1) and octave is in divTempPtr3
    ; load the table pointer
    mov a, !divChanPitchTablePtr+x
    mov divTempPtr2, a
    mov a, !(divChanPitchTablePtr+1)+x
    mov divTempPtr2+1, a
    ; get the note frequency and store it
    mov a, [divTempPtr2]+y ; low byte
    mov divTempPtr1, a
    inc y
    mov a, [divTempPtr2]+y ; high byte
    mov divTempPtr1+1, a
    ; get the delta and multiply it by the fractional part
    mov a, y
    clrc
    adc a, #24 ; start reading from delta
    mov y, a
    mov a, [divTempPtr2]+y ; low byte
    mov divTempPtr4, a
    inc y
    mov a, [divTempPtr2]+y ; high byte
    mov divTempPtr4+1, a
    ; divTempPtr1 contains freq
    ; divTempPtr4 contains delta
    ; now multiply delta by fractional part
    mov a, divTempPtr4 ; low byte
    mov y, divTempPtr ; fraction
    mul ya
    mov divTempPtr4, y
    ; multiply the upper byte
    mov a, divTempPtr4+1 ; high byte
    mov y, divTempPtr ; fraction
    mul ya
    ; clear out the upper part so we can addw and have a 16-bit value
    mov divTempPtr4+1, #0
    addw ya, divTempPtr4
    ; might as well add the frequency now
    addw ya, divTempPtr1
    ; store it in divTempPtr1 - this is the unshifted final frequency
    movw divTempPtr1, ya
    ; we need to shift by the octave
    mov a, !divChanOctaveShift+x
    setc
    sbc a, divTempPtr3
    bcs @doShift
    beq @post ; if shift is zero, don't
    ; if we're out of bounds, write max freq
    mov a, #$ff
    mov !divChanFreq+x, a
    mov a, #$3f
    mov !(divChanFreq+1)+x, a
    bra @post
    @doShift:
    ; otherwise begin the shifting process
    mov y, a
    ; load the lower byte for performance
    mov a, divTempPtr1
 -  clrc
    ror divTempPtr1+1
    ror a
    dbnz y, -
    bcc +
    inc a ; rounding
    bne +
    inc divTempPtr1+1
    ; store the frequency
+   mov !divChanFreq+x, a
    mov a, divTempPtr1+1
    mov !(divChanFreq+1)+x, a
.else
    ; non-linear pitch - just store the offset
    mov !divChanFreq+x, a
    mov a, y
    mov !(divChanFreq+1)+x, a
.endif
  @post:
  ret

; calculate base frequency and store it in divTempPtr1.
; - YA: note (move Y last!!!)
; - X: channel
; alters A, Y and divTempPtr2.
divCalcBaseFreq:
  ; Y is loaded and PSW contains the raw note flag.
  bpl isNormal
  @isRawFreq:
    movw divTempPtr1, ya
    clr1 (divTempPtr+1).7
    ; set the raw freq flag
    mov a, !divChanSNESFlags+x
    or a, #$01
    mov !divChanSNESFlags+x, a
    ret
  @isNormal:
    ; pitchTable->getBase(ya);
.ifdef DIV_LINEAR_FREQ
    ; linear pitch - store (note<<7)
    lsr a
    mov divTempPtr1+1, a
    bcs write80
    @write00:
      mov divTempPtr1, #$00
      bra @post
    @write80:
      mov divTempPtr1, #$80
.else
    ; non-linear pitch - read from the table
    ; load the table pointer
    mov a, !divChanPitchTablePtr+x
    mov divTempPtr2, a
    mov a, !(divChanPitchTablePtr+1)+x
    mov divTempPtr2+1, a
    ; split note into note/octave
    mov a, fcsArg0
    lsr a
    lsr a
    mov y, a
    mov a, !divOctaveTable+y
    mov divTempPtr3, a
    mov y, a
    mov a, fcsArg0
    setc
    sbc a, !divNoteSubTable+y
    asl a
    mov y, a
    ; Y contains (note<<1) and octave is in divTempPtr3
    ; let's read the note from the pitch table
    mov a, [divTempPtr2]+y ; low byte
    mov divTempPtr1, a
    inc y
    mov a, [divTempPtr2]+y ; high byte
    mov divTempPtr1+1, a
    ; we need to shift by the octave
    mov a, !divChanOctaveShift+x
    setc
    sbc a, divTempPtr3
    bcs @doShift
    beq @post ; if shift is zero, don't
    ; if we're out of bounds, write max freq
    mov divTempPtr1, #$ff
    mov divTempPtr1+1, #$3f
    bra @post
    @doShift:
    ; otherwise begin the shifting process
    mov y, a
    ; load the lower byte for performance
    mov a, divTempPtr1
 -  clrc
    ror divTempPtr1+1
    ror a
    dbnz y, -
    bcc +
    inc a ; rounding
    bne +
    inc divTempPtr1+1
    ; store the lower byte
+   mov divTempPtr1, a
    ; divTempPtr1 now has the base frequency
.endif
    @post:
    ; clear the raw freq flag
    mov a, !divChanSNESFlags+x
    and a, #~$01
    mov !divChanSNESFlags+x, a
    ret

;;;; ---- COMMAND HANDLERS ---- ;;;;

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
    mov a, !songShiftList0+y
    mov !divChanOctaveShift+x, a
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
      mov a, fcsArg0+1 ; check for raw freq note
      bpl useInitSample
      mov y, #4
      mov a, [divTempPtr]+y
      beq useInitSample
      ; we must read the sample map....
      @@@useSampleMap:
        ; get the sample map sample pointers
        mov y, #11
        mov a, [divTempPtr]+y ; lower
        mov divTempPtr1, a
        inc y
        mov a, [divTempPtr]+y
        mov divTempPtr1+1, a
        inc y
        mov a, [divTempPtr]+y ; upper
        mov divTempPtr2, a
        inc y
        mov a, [divTempPtr]+y
        mov divTempPtr2+1, a
        ; get the sample index now
        mov y, fcsArg0 ; note
        mov a, [divTempPtr1]+y
        mov !divChanSample+x, a
        mov a, [divTempPtr2]+y
        mov !(divChanSample+1)+x, a
        ; get the sample map note pointer
        mov y, #15
        mov a, [divTempPtr]+y
        mov divTempPtr1, a
        inc y
        mov a, [divTempPtr]+y
        mov divTempPtr1+1, a
        ; get the note
        mov y, fcsArg0 ; note
        ; make a copy of the sample note
        ; we don't need the upper byte
        mov divTempPtr2, y
        mov a, [divTempPtr1]+y
        ; store it (override the note)
        mov fcsArg0, a
        ; calculate delta
        setc
        sbc a, divTempPtr2
        mov !divChanSampleNoteDelta+x, a
        bra @@setPitchTable
      @@@useInitSample:
        ; just use the initial sample if we don't need sample map
        inc y
        mov a, [divTempPtr]+y
        mov !divChanSample+x, a
        inc y
        mov a, [divTempPtr]+y
        mov !(divChanSample+1)+x, a
        ; in this case the delta is 0
        mov a, #0
        mov !divChanSampleNoteDelta+x, a
    @@setPitchTable:
      ; change the pitch table
      ; we don't support 16-bit index yet
      mov !divChanSample+x, a
      mov y, a
      mov a, !(songPitchListLow0+17)+y
      mov !divChanPitchTablePtr+x, a
      mov a, !(songPitchListHigh0+17)+y
      mov !(divChanPitchTablePtr+1)+x, a
      mov a, !(songShiftList0+17)+y
      mov !divChanOctaveShift+x, a
    @@postNoteNotNull:
    ; chan[c.chan].useWave=false;
    mov a, !divChanSNESFlags+x
    and a, #~$80
    mov !divChanSNESFlags+x, a
  @post1:
  ; set the active flag
  mov a, !divChanFlags+x
  or a, #$80
  mov !divChanFlags+x, a
  ; check for instrument change
  and a, #$40
  beq @noInsChange
  @insChange:
    ; an instrument change occurred
    ; clear insChanged
    mov a, !divChanFlags+x
    and a, #~$40
    mov !divChanFlags+x, a
    ; copy the new parameters
    mov y, #0
    ; AD/mode
    mov a, [divTempPtr]+y
    mov !divChanAD+x, a
    ; SR
    inc y
    mov a, [divTempPtr]+y
    mov !divChanSR+x, a
    ; gain
    inc y
    mov a, [divTempPtr]+y
    mov !divChanGain+x, a
    ; D2/sus mode
    inc y
    mov a, [divTempPtr]+y
    mov y, a
    lsr a
    lsr a
    mov !divChanD2+x, a
    mov a, y
    and #3
    mov a, divTempPtr1
    mov a, !divChanSNESFlags+x
    and a, #~$03
    or a, divTempPtr1
    or a, #4 ; we must set shallWriteEnv
    mov !divChanSNESFlags+x, a
    bra @post2
  @noInsChange:
    ; check whether the sus mode is not direct
    ; if so we need to set shallWriteEnv
    mov a, !divChanSNESFlags+x
    and a, #3
    beq @post2
    mov a, !divChanSNESFlags+x
    or a, #4
    mov !divChanSNESFlags+x, a
  @post2:
  ; calculate note frequency
  ; first check for null note
  mov a, fcsArg0+1
  bne @doCalc
  mov a, fcsArg0
  cmp a, #$b8 ; 180 - null
  beq @post3
  @doCalc:
    ; we need to calculate frequency
    mov a, fcsArg0
    mov y, fcsArg0+1
    call !divCalcBaseFreq
    mov a, divTempPtr1
    mov !divChanBaseFreq+x, a
    mov a, divTempPtr1+1
    mov !(divChanBaseFreq+1)+x, a
    ; set freqChanged
    mov a, !divChanSNESFlags+x
    or a, #$20
    mov !divChanSNESFlags+x, a
  @post3:
  ; set keyOn
  mov a, !divChanSNESFlags+x
  or a, #$08
  mov !divChanSNESFlags+x, a
  ; TODO: call macroInit here...
  ; TODO: check whether volume macro is present before calling this code
.ifndef DIV_COMPAT_BROKEN_OUT_VOL
  ; check whether volume changed
  mov a, !divChanVol+x
  cmp a, !divChanOutVol+x
  bne noShallWrite
  @shallWrite:
    mov a, !divChanSNESFlags+x
    or a, #$08
    mov !divChanSNESFlags+x, a
  @noShallWrite:
    ; copy the volume
    mov a, !divChanVol+x
    mov !divChanOutVol+x, a
.endif
  ret

divCmdNoteOff:
  ; disable active and keyOn
  ; enable keyOff
  mov a, !divChanFlags+x
  and a, #~$88
  or a, #$04
  mov !divChanFlags+x, a
  ; check sustain mode
  ; if it's direct, stop macro
  ; otherwise begin release by setting shallWriteEnv
  mov a, !divChanSNESFlags+x
  and a, #3
  beq @shouldEndMacro
  @shouldRelease:
    mov a, !divChanSNESFlags+x
    or a, #$04
    mov !divChanSNESFlags+x, a
    ret
  @shouldEndMacro:
    ; TODO: macroInit(NULL) here...
    ret

divCmdNoteOffEnv:
  ; this is the same as divCmdNoteOff, except that it releases macros instead of clearing them out.
  ; disable active and keyOn
  ; enable keyOff
  mov a, !divChanFlags+x
  and a, #~$88
  or a, #$04
  mov !divChanFlags+x, a
  ; check sustain mode
  ; if set, release by setting shallWriteEnv
  mov a, !divChanSNESFlags+x
  and a, #3
  beq @shouldNotWrite
  @shouldRelease:
    mov a, !divChanSNESFlags+x
    or a, #$04
    mov !divChanSNESFlags+x, a
  @shouldNotWrite:
    ; TODO: release() here...
    ret

divCmdEnvRelease:
  ; TODO: release() here...
  ret

divCmdInstrument:
  ; check if the new instrument is different
  mov a, fcsArg0
  cmp a, !divChanIns+x
  beq @noChange
  @change:
    ; change the instrument and set insChanged
    mov !divChanIns+x, a
    mov a, !divChanFlags+x
    or a, #$40
    mov !divChanFlags+x, a
  @noChange:
    ret

divCmdVolume:
  ; check if the new volume is different
  mov a, fcsArg0
  cmp a, !divChanVol+x
  beq @noChange
  @change:
    mov !divChanVol+x, a
    ; TODO: check whether the volume macro is not working
    mov !divChanOutVol+x, a
    ; set the shallWriteVol flag
    mov a, !divChanSNESFlags+x, a
    or a, #$08
    mov !divChanSNESFlags+x, a
  @noChange:
    ret

divCmdPanning:
  ; set panning
  mov a, fcsArg0
  lsr a
  mov !divChanPanL+x, a
  mov a, fcsArg1
  lsr a
  mov !divChanPanR+x, a
  ; set the shallWriteVol flag
  mov a, !divChanSNESFlags+x, a
  or a, #$08
  mov !divChanSNESFlags+x, a
  ret

divCmdPitch:
  ; set the pitch
  movw ya, fcsArg0
  mov !divChanPitch+x, a
  mov a, y
  mov !(divChanPitch+1)+x, a
  ; set freqChanged flag
  mov a, !divChanFlags+x
  or a, #$20
  mov !divChanFlags+x, a
  ret

; TODO: the rest of commands...
