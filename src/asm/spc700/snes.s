; SNES DivDispatch code.

.define DIV_LINEAR_FREQ

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
; - bit 4: released
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
; - bit 5: !echoOn
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
; bit fields
divEchoState=divGlobalBase+18 ; unsigned char
divNoiseState=divGlobalBase+19 ; unsigned char
divPitchModState=divGlobalBase+20 ; unsigned char

;;;; ---- LOOK-UP TABLES ---- ;;;;
; used to determine the position of registers for a channel.
divChanOffs:
  .db $00, $00
  .db $10, $10
  .db $20, $20
  .db $30, $30
  .db $40, $40
  .db $50, $50
  .db $60, $60
  .db $70, $70

; chan*2 - first column is positive and second is negative
divChanBits:
  .db $01, $fe
  .db $02, $fd
  .db $04, $fb
  .db $08, $f7
  .db $10, $ef
  .db $20, $df
  .db $40, $bf
  .db $80, $7f

; pointers to wave memory
divWaveAddr:
  .dw $420, $4b0, $540, $5d0, $660, $6f0, $780, $810

; pointers to dir entries (for each channel)
divDirChanAddr:
  .dw $400, $404, $408, $40c, $410, $414, $418, $41c

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
  ; macro processing
  ; CRAP

  ; frequency processing
  mov x, #0
  @freqLoop:
    ; if (freqChanged || keyOn || keyOff)
    mov a, !divChanFlags+x
    and a, #$2c
    bne +
    jmp !@@outOfHere

    ; calculate frequency
+   call !divCalcFreq
    ; clamp frequency
    mov a, !divChanFlags+x ; raw freq check
    and a, #1
    bne @@checkKeyOn
    mov a, divTempPtr+1 ; >$3fff check
    cmp a, #$3f
    bcc @@checkKeyOn
    ; do clamp
    mov a, #$ff
    mov divTempPtr, a
    mov a, #$3f
    mov divTempPtr+1, a
    @@checkKeyOn:
      ; check for key on
      mov a, !divChanFlags+x
      and a, #$08
      beq @@checkKeyOff
      ; disable key on flag
      mov a, !divChanFlags+x
      and a, #~$08
      mov !divChanFlags+x, a
      ; find sample dir location for this channel
      ; X is (chan<<1) so one more shift will do
      mov a, x
      asl a
      mov y, a
      ; test whether we're using wavetables
      mov a, !divChanSNESFlags+x
      bpl @@@usingSample
      @@@usingWave:
        ; location in wave memory
        ; lower byte
        mov a, !divWaveAddr+x
        mov !$400+y, a ; start
        mov !$402+y, a ; loop
        ; upper byte
        mov a, !(divWaveAddr+1)+x
        mov !$401+y, a ; start
        mov !$403+y, a ; loop
        bra @@@post
      @@@usingSample:
        ; retrieve sample pointer from sample table
        push x
        mov a, !divChanSample+x
        mov x, a
        mov a, !songSampleStartLow0+x
        mov !$400+y, a
        mov a, !songSampleStartHigh0+x
        mov !$401+y, a
        mov a, !songSampleLoopLow0+x
        mov !$402+y, a
        mov a, !songSampleLoopHigh0+x
        mov !$403+y, a
        pop x
      @@@post:
      ; set key on and key off
      mov a, divTempKeyOn
      or a, !divChanBits+x
      mov divTempKeyOn, a
      mov a, divTempKeyOff
      or a, !divChanBits+x
      mov divTempKeyOff, a
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
      mov y, divTempPtr
      chWriteX 2
      mov y, divTempPtr+1
      chWriteX 3
    @@outOfHere:
      inc x
      inc x
      cmp x, #(divChans*2)
      beq @checkTempKeyOff
      jmp !@freqLoop
  @checkTempKeyOff:
    ; check whether we should write key off
    mov a, divTempKeyOff
    beq @cacheGlobalFlags
    ; TODO: anti-click...
    ; I expect this to work. normally there should be a delay...
    dspWriteA $5c
  @cacheGlobalFlags:
    ; cache global flags so we can use bbc instead of loading the
    ; flags each time...
    mov a, !divWriteFlags
    mov divTempPtr, a
  @checkWriteControl:
    ; check writeControl
    bbc divTempPtr.7, @checkWriteNoise
    clr1 divTempPtr.7
    ; write control (noise frequency + echo)
    mov a, !divNoiseFreq
    dspWriteA $6c
  @checkWriteNoise:
    ; check writeNoise
    bbc divTempPtr.6, @checkWritePitchMod
    clr1 divTempPtr.6
    mov a, !divNoiseState
    dspWriteA $3d
  @checkWritePitchMod:
    ; check writePitchMod
    bbc divTempPtr.5, @checkWriteEcho
    clr1 divTempPtr.5
    mov a, !divPitchModState
    dspWriteA $2d
  @checkWriteEcho:
    ; check writeEcho
    bbc divTempPtr.4, @checkWriteDryVol
    clr1 divTempPtr.4
    mov a, !divEchoState
    dspWriteA $4d
  @checkWriteDryVol:
    ; check writeDryVol
    bbc divTempPtr.3, @writeCache
    clr1 divTempPtr.3
    mov a, !divDryVolL
    dspWriteA $0c
    mov a, !divDryVolR
    dspWriteA $1c
  @writeCache:
    ; write the cached global write flags back
    mov a, divTempPtr
    mov !divWriteFlags, a
  @checkWriteEnv:
    ; check writeEnv (of each channel)
    mov x, #0
-   mov a, !divChanSNESFlags+x
    and a, #$04
    beq +
    ; disable the flag
    mov a, !divChanSNESFlags+x
    and a, #~$04
    mov !divChanSNESFlags+x, a
    ; write env
    call !divWriteEnv
+   inc x
    inc x
    cmp x, #divChans*2
    bne -
  @checkDisableKeyOff:
    ; write key off zero (if set)
    mov a, divTempKeyOff
    beq @checkShallWriteVol
    dspWrite dsp_KOF, #0
  @checkShallWriteVol:
    ; check shallWriteVol
    mov x, #0
-   mov a, !divChanSNESFlags+x
    and a, #$08
    beq +
    ; disable the flag
    mov a, !divChanSNESFlags+x
    and a, #~$08
    mov !divChanSNESFlags+x, a
    ; write volume
    call !divWriteOutVol
+   inc x
    inc x
    cmp x, #divChans*2
    bne -
  @checkTempKeyOn:
    ; check whether we should write key on
    mov a, divTempKeyOn
    beq @theEnd
    dspWriteA $4c
  @theEnd:
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
  and a, #$ff ; test A
  beq +
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
  and a, #$ff ; test A
  beq +
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
  ; test whether ADSR is enabled
  mov a, !divChanAD+x
  bpl notUseEnv
  useEnv:
    mov a, !divChanFlags+x
    bpl +
    mov a, !divChanSNESFlags+x
    and a, #3
    or a, #4
    asl a
    push x
    mov x, a
    ; implemented as a jump table depending on active and ADSR mode.
    jmp [!divWriteEnvSubTable+x]
+   mov a, !divChanSNESFlags+x
    and a, #3
    asl a
    push x
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
  and a, #$20 ; !echoOn
  beq echoIsOn ; because the code for on is too long
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
    asl a
    asl a
    asl a
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
    mov a, !divNoiseFreq ; control
    dspWriteA $6c
    ret

; void DivPlatformSNES::reset()
divReset:
  ; set the sample table base
  ; I expect the dir to sit at $400
  dspWrite dsp_DIR, #$04
  dspWrite dsp_MVOL_L, #$7f
  dspWrite dsp_MVOL_R, #$7f
  dspWrite dsp_FLG, #$20
  
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
- mov a, #>divDefaultIns
  mov !divChanIns+x, a
  mov a, !songPitchListHigh0+1 ; wavePitchTable[1]
  mov !divChanPitchTablePtr+x, a
  dec x
  mov a, #<divDefaultIns
  mov !divChanIns+x, a
  mov a, !songPitchListLow0+1 ; wavePitchTable[1]
  mov !divChanPitchTablePtr+x, a
  mov a, !songShiftList0+1 ; wavePitchTable[1]
  mov !divChanOctaveShift+x, a
  dec x
  bpl -

  ; vol/outVol/panL/panR
  ; what????
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

  ; initial global state
  mov x, #18
- mov a, !songInitState0+x
  mov !divGlobalBase+x, a
  dec x
  bpl -

  ; writeOutVol/set source number
  mov x, #0
- call !divWriteOutVol
  mov a, x
  lsr a
  mov y, a
  chWriteX 4 ; source number
  inc x
  inc x
  cmp x, #divChans*2
  bne -

  ; finish up
  call !divInitEcho
  ret

;;;; ---- PITCH TABLE FUNCTIONS ---- ;;;;

; calculate frequency and store it in divTempPtr.
; - X: channel
; alters A and Y.
divCalcFreq:
  ; check whether we are in raw freq mode
  mov a, !divChanFlags+x
  and a, #$01
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
    mov divTempPtr, a
    mov a, y
    mov divTempPtr+1, a
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
    mov a, divTempPtr+1
    lsr a
    lsr a
    mov y, a
    mov a, !divOctaveTable+y
    mov divTempPtr3, a
    mov y, a
    mov a, divTempPtr+1
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
    adc a, #23 ; start reading from delta (-1 because Y was inc'd)
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
    bne + ; if shift is zero, don't
    ; if we're here, don't shift - just return
    mov a, divTempPtr1
    mov divTempPtr, a
    mov a, divTempPtr1+1
    mov divTempPtr+1, a
    ret
    ; otherwise check whether we're out of bounds
+   bcs @doShift
    ; if we're out of bounds, write max freq
    mov a, #$ff
    mov divTempPtr, a
    mov a, #$3f
    mov divTempPtr+1, a
    bra @post
    @doShift:
    ; otherwise begin the shifting process
    mov y, a
    ; load the lower byte for performance
    mov a, divTempPtr1
 -  lsr divTempPtr1+1
    ror a
    dbnz y, -
    bcc +
    inc a ; rounding
    bne +
    inc divTempPtr1+1
    ; store the frequency
+   mov divTempPtr, a
    mov a, divTempPtr1+1
    mov divTempPtr+1, a
.else
    ; non-linear pitch - just store the offset
    mov divTempPtr, a
    mov a, y
    mov divTempPtr+1, a
.endif
  @post:
  ret

; calculate base frequency and store it in divTempPtr1.
; - YA: note (move Y last!!!)
; - X: channel
; alters A, Y and divTempPtr2.
divCalcBaseFreq:
  ; Y is loaded and PSW contains the raw note flag.
  bpl @isNormal
  @isRawFreq:
    movw divTempPtr1, ya
    clr1 (divTempPtr+1).7
    ; set the raw freq flag
    mov a, !divChanFlags+x
    or a, #$01
    mov !divChanFlags+x, a
    ret
  @isNormal:
    ; pitchTable->getBase(ya);
.ifdef DIV_LINEAR_FREQ
    ; linear pitch - store (note<<7)
    lsr a
    mov divTempPtr1+1, a
    bcs @write80
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
 -  lsr divTempPtr1+1
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
    mov a, !divChanFlags+x
    and a, #~$01
    mov !divChanFlags+x, a
    ret

;;;; ---- MACRO FUNCTIONS ---- ;;;;

; this implementation of DivMacroInt only supports 16-bit values.

; divTempPtr is the macro pointer.
; divTempPtr2 is the new tick timer (if the callback isn't made).
; divMacroCallback is a callback on value.
; the value will be on YA.
; bit 30 is enabled if carry is set.
; X is (chan<<1).
; if your callback is run, assume the tick timer is 0 (one tick).
divRunMacro:
  mov y, #0
  mov a, [!divTempPtr]+y
  bmi divMacroIsCommand
  divMacroIsValue:
    ; FORTUNATELY... this instruction exists...
    incw divTempPtr
    mov divTempPtr2, #0
    clrc
    ; FILL THIS IN BEFORE CALLING!!!!!!!!!
    divMacroCallback:
    jmp !$0000
  divMacroIsCommand:
    push x
    mov x, a
    jmp [!(divMacroCmdTable-128)+x]

; $90
divMacroCmdU8:
  pop x
  incw divTempPtr
  mov a, [!divTempPtr]+y
  incw divTempPtr
  mov divTempPtr2, #0
  clrc
  bra divMacroCallback

; $92
divMacroCmdS8:
  pop x
  mov divTempPtr2, #0
  incw divTempPtr
  mov a, [!divTempPtr]+y
  bmi @neg
  @pos:
    incw divTempPtr
    clrc
    bra divMacroCallback
  @neg:
    incw divTempPtr
    dec y
    clrc
    bra divMacroCallback

; $94
divMacroCmdS16:
  pop x
  mov divTempPtr2, #0
  incw divTempPtr
  mov a, [!divTempPtr]+y
  push a
  incw divTempPtr
  mov a, [!divTempPtr]+y
  incw divTempPtr
  mov y, a
  pop a
  clrc
  bra divMacroCallback

; $98
divMacroCmdS8Bit30:
  pop x
  mov divTempPtr2, #0
  incw divTempPtr
  mov a, [!divTempPtr]+y
  bmi @neg
  @pos:
    incw divTempPtr
    setc
    bra divMacroCallback
  @neg:
    incw divTempPtr
    dec y
    setc
    bra divMacroCallback

; $96 - what the hell. I am not implementing this one.
divMacroCmdS32:
  stop

; $80
divMacroCmdStop:
  pop x
  mov divTempPtr, #$80
  mov divTempPtr+1, #0
  mov divTempPtr2, #0
  ret

; $82
divMacroCmdJump:
  pop x
actuallyJump:
  incw divTempPtr
  mov a, [divTempPtr]+y
  eor a, #$ff
  push a
  incw divTempPtr
  mov a, [divTempPtr]+y
  eor a, #$ff
  mov y, a
  pop a
  addw ya, divTempPtr
  movw divTempPtr, ya
  jmp !divRunMacro

; $84
divMacroCmdJumpNoRel:
  pop x
  mov a, !divChanSNESFlags+x
  and a, #$10
  beq actuallyJump
  ; released - don't jump
  incw divTempPtr
  jmp !divRunMacro

; $86
divMacroCmdWaitRel:
  pop x
  mov a, !divChanSNESFlags+x
  and a, #$10
  bne @hasReleased
  mov divTempPtr2, #0
  ret
  @hasReleased:
    incw divTempPtr
    jmp !divRunMacro

; $88-$8e
divMacroCmdNotImpl:
  pop x
  ret

; $a0-$be
divMacroCmdWaitLow:
  pop x
  incw divTempPtr
  and a, #$1e
  lsr a
  dec a
  mov divTempPtr2, a
  ret

; $c0-$de
divMacroCmdWaitHigh:
  pop x
  incw divTempPtr
  and a, #$1e
  asl a
  asl a
  asl a
  dec a
  mov divTempPtr2, a
  ret

; could be optimized...
divMacroCmdTable:
  .dw divMacroCmdStop
  .dw divMacroCmdJump
  .dw divMacroCmdJumpNoRel
  .dw divMacroCmdWaitRel
  .dw divMacroCmdNotImpl
  .dw divMacroCmdNotImpl
  .dw divMacroCmdNotImpl
  .dw divMacroCmdNotImpl
  .dw divMacroCmdU8
  .dw divMacroCmdS8
  .dw divMacroCmdS16
  .dw divMacroCmdS32
  .dw divMacroCmdS8Bit30
  .dw divMacroCmdNotImpl
  .dw divMacroCmdNotImpl
  .dw divMacroCmdNotImpl
  .dw divMacroCmdWaitLow
  .dw divMacroCmdWaitLow
  .dw divMacroCmdWaitLow
  .dw divMacroCmdWaitLow
  .dw divMacroCmdWaitLow
  .dw divMacroCmdWaitLow
  .dw divMacroCmdWaitLow
  .dw divMacroCmdWaitLow
  .dw divMacroCmdWaitLow
  .dw divMacroCmdWaitLow
  .dw divMacroCmdWaitLow
  .dw divMacroCmdWaitLow
  .dw divMacroCmdWaitLow
  .dw divMacroCmdWaitLow
  .dw divMacroCmdWaitLow
  .dw divMacroCmdWaitLow
  .dw divMacroCmdWaitHigh
  .dw divMacroCmdWaitHigh
  .dw divMacroCmdWaitHigh
  .dw divMacroCmdWaitHigh
  .dw divMacroCmdWaitHigh
  .dw divMacroCmdWaitHigh
  .dw divMacroCmdWaitHigh
  .dw divMacroCmdWaitHigh
  .dw divMacroCmdWaitHigh
  .dw divMacroCmdWaitHigh
  .dw divMacroCmdWaitHigh
  .dw divMacroCmdWaitHigh
  .dw divMacroCmdWaitHigh
  .dw divMacroCmdWaitHigh
  .dw divMacroCmdWaitHigh
  .dw divMacroCmdWaitHigh

;;;; ---- MACRO HANDLERS ---- ;;;;

divMacroVol:
  ; volume macro - linear volume scale
  mov y, a
  mov a, !divChanVol+x
  asl a
  mul ya
  ; rounding
  and a, #$ff ; test A
  beq +
  inc y
+ mov a, y
  mov !divChanOutVol+x, a
  ret

divMacroArp:
  ret

divMacroDuty:
  ret

divMacroPitch:
  ret

;;;; ---- COMMAND HANDLERS ---- ;;;;

divCmdNoteOn:
  ; DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_SNES);
  mov a, !(divChanIns+1)+x
  mov y, a
  mov a, !divChanIns+x
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
      bmi @@@useInitSample
      mov y, #4
      mov a, [divTempPtr]+y
      beq @@@useInitSample
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
        mov y, #5
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
      mov a, !divChanSample+x
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
    mov y, a ; make a copy so we can take sus mode
    lsr a
    lsr a
    mov !divChanD2+x, a
    mov a, y ; retrieve sus mode and apply it
    and a, #3
    mov divTempPtr1, a ; so we can and later
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
    mov a, !divChanFlags+x
    or a, #$20
    mov !divChanFlags+x, a
  @post3:
  ; set keyOn
  mov a, !divChanFlags+x
  or a, #$08
  mov !divChanFlags+x, a
  ; TODO: call macroInit here...
  ; TODO: check whether volume macro is present before calling this code
.ifndef DIV_COMPAT_BROKEN_OUT_VOL
  ; check whether volume changed
  mov a, !divChanVol+x
  cmp a, !divChanOutVol+x
  bne @noShallWrite
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
  ; we assume this command is sent only when the instrument has actually changed
  ; change the instrument and set insChanged
  mov y, fcsArg0
  mov a, !songInsListLow0+y
  mov !divChanIns+x, a
  mov a, !songInsListHigh0+y
  mov !(divChanIns+1)+x, a
  mov a, !divChanFlags+x
  or a, #$40
  mov !divChanFlags+x, a
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
    mov a, !divChanSNESFlags+x
    or a, #$08
    mov !divChanSNESFlags+x, a
  @noChange:
    ret

divCmdPanning:
  ; set panning
  mov a, !divChanPanL+x
  rol a ; store invert bit
  mov a, fcsArg0
  ror a
  mov !divChanPanL+x, a
  mov a, !divChanPanR+x
  rol a ; store invert bit
  mov a, fcsArg1
  ror a
  mov !divChanPanR+x, a
  ; set the shallWriteVol flag
  mov a, !divChanSNESFlags+x
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

divCmdWave:
  ; TODO when wave is implemented
  ret

divCmdNotePorta:
  ; set freqChanged flag ahead of time
  mov a, !divChanFlags+x
  or a, #$20
  mov !divChanFlags+x, a
  ; calculate target frequency
  mov a, !divChanSampleNoteDelta+x
  mov y, #0
  addw ya, fcsArg1
  call !divCalcBaseFreq
  ; divTempPtr1 is target
  ; load the current freq and compare
  mov a, !(divChanBaseFreq+1)+x
  mov y, a
  mov a, !divChanBaseFreq+x
  cmpw ya, divTempPtr1
  bpl @mustDecrease
  @mustIncrease:
    ; destFreq>baseFreq
    ; add the speed
    addw ya, fcsArg0
    ; compare again, as we could have reached destination
    cmpw ya, divTempPtr1
    bpl @reached
    ; not reached - write YA
    mov !divChanBaseFreq+x, a
    mov a, y
    mov !(divChanBaseFreq+1)+x, a
    ret
  @mustDecrease:
    ; destFreq<baseFreq
    ; subtract the speed
    subw ya, fcsArg0
    ; compare again, as we could have reached destination
    cmpw ya, divTempPtr1
    bmi @reached
    beq @reached
    ; not reached - write YA
    mov !divChanBaseFreq+x, a
    mov a, y
    mov !(divChanBaseFreq+1)+x, a
    ret
  @reached:
    ; reached - write destFreq
    mov a, divTempPtr1
    mov !divChanBaseFreq+x, a
    mov a, divTempPtr1+1
    mov !(divChanBaseFreq+1)+x, a
    ret

divCmdLegato:
  ; calculate new baseFreq
  ; TODO: what about that hacky legato mess?
  mov a, !divChanSampleNoteDelta+x
  mov y, #0
  addw ya, fcsArg0
  call !divCalcBaseFreq
  mov a, divTempPtr1
  mov !divChanBaseFreq+x, a
  mov a, divTempPtr1+1
  mov !(divChanBaseFreq+1)+x, a
  ; set freqChanged flag
  mov a, !divChanFlags+x
  or a, #$20
  mov !divChanFlags+x, a
  ret

divCmdSamplePos:
  ; TODO
  ret

divCmdStdNoiseMode:
  ; set writeNoise flag
  mov a, !divWriteFlags
  or a, #$40
  mov !divWriteFlags, a
  ; set noise mode
  mov a, fcsArg0
  beq @setDisabled
  @setEnabled:
    mov a, !divChanBits+x
    or a, !divNoiseState
    mov !divNoiseState, a
    ret
  @setDisabled:
    mov a, !(divChanBits+1)+x
    and a, !divNoiseState
    mov !divNoiseState, a
    ret

divCmdSnesPitchMod:
  ; set writePitchMod flag
  mov a, !divWriteFlags
  or a, #$20
  mov !divWriteFlags, a
  ; set pitch mod state
  mov a, fcsArg0
  beq @setDisabled
  @setEnabled:
    mov a, !divChanBits+x
    or a, !divPitchModState
    mov !divPitchModState, a
    ret
  @setDisabled:
    mov a, !(divChanBits+1)+x
    and a, !divPitchModState
    mov !divPitchModState, a
    ret

divCmdSnesInvert:
  ; set shallWriteVol flag
  mov a, !divChanSNESFlags+x
  or a, #$08
  mov !divChanSNESFlags+x, a
  ; unpack invert bits
  @testLeft:
    mov a, fcsArg0
    and a, #$f0 ; left channel
    beq @@leftFalse
    @@leftTrue:
      mov a, !divChanPanL+x
      or a, #$80
      mov !divChanPanL+x, a
      bra @testRight
    @@leftFalse:
      mov a, !divChanPanL+x
      and a, #$7f
      mov !divChanPanL+x, a
  @testRight:
    mov a, fcsArg0
    and a, #$0f ; left channel
    beq @@rightFalse
    @@rightTrue:
      mov a, !divChanPanR+x
      or a, #$80
      mov !divChanPanR+x, a
      ret
    @@rightFalse:
      mov a, !divChanPanR+x
      and a, #$7f
      mov !divChanPanR+x, a
  ret

divCmdSnesGain:
  ; set gain
  mov a, fcsArg0
  mov !divChanGain+x, a
  ; set shallWriteEnv flag
  mov a, !divChanSNESFlags+x
  or a, #$04
  mov !divChanSNESFlags+x, a
  ret

divCmdStdNoiseFreq:
  ; set noise frequency
  ; preserve echo bit
  mov a, !divNoiseFreq
  and a, #~$1f
  or a, fcsArg0
  mov !divNoiseFreq, a
  ; set writeControl flag
  mov a, !divWriteFlags
  or a, #$80
  mov !divWriteFlags, a
  ret

divCmdFmAr:
  ; set attack
  mov a, !divChanAD+x
  and a, #$f0
  or a, fcsArg0
  mov !divChanAD+x, a
  ; schedule an envelope update if necessary
  bpl +
  mov a, !divChanSNESFlags+x
  or a, #$04
  mov !divChanSNESFlags+x, a
+ ret

divCmdFmDr:
  ; shift up
  mov a, fcsArg0
  xcn a
  mov fcsArg0, a
  ; set decay
  mov a, !divChanAD+x
  and a, #$8f
  or a, fcsArg0
  mov !divChanAD+x, a
  ; schedule an envelope update if necessary
  bpl +
  mov a, !divChanSNESFlags+x
  or a, #$04
  mov !divChanSNESFlags+x, a
+ ret

divCmdFmSl:
  ; shift up
  mov a, fcsArg0
  xcn a
  asl a
  mov fcsArg0, a
  ; set sustain
  mov a, !divChanSR+x
  and a, #$1f
  or a, fcsArg0
  mov !divChanSR+x, a
  ; schedule an envelope update if necessary
  mov a, !divChanAD+x
  bpl +
  mov a, !divChanSNESFlags+x
  or a, #$04
  mov !divChanSNESFlags+x, a
+ ret

divCmdFmRr:
  ; set release
  mov a, !divChanSR+x
  and a, #$e0
  or a, fcsArg0
  mov !divChanSR+x, a
  ; schedule an envelope update if necessary
  mov a, !divChanAD+x
  bpl +
  mov a, !divChanSNESFlags+x
  or a, #$04
  mov !divChanSNESFlags+x, a
+ ret

divCmdSnesEcho:
  ; set writeEcho flag
  mov a, !divWriteFlags
  or a, #$10
  mov !divWriteFlags, a
  ; set echo state
  mov a, fcsArg0
  beq @setDisabled
  @setEnabled:
    mov a, !divChanBits+x
    or a, !divEchoState
    mov !divEchoState, a
    ret
  @setDisabled:
    mov a, !(divChanBits+1)+x
    and a, !divEchoState
    mov !divEchoState, a
    ret

divCmdSnesEchoDelay:
  ; set echo delay
  mov a, fcsArg0
  mov !divEchoDelay, a
  ; check whether echo is on
  mov a, !divNoiseFreq
  and a, #$20
  bne +
  ; if we're here, echo is on.
  ; calculate ESA
  mov a, #$1f
  setc
  sbc a, !divEchoDelay
  asl a
  asl a
  asl a
  dspWriteA $6d
  ; write echo delay
  mov a, !divEchoDelay
  dspWriteA $7d
+ ret

divCmdSnesEchoEnable:
  ; toggle echo (global)
  mov a, fcsArg0
  beq @disable
  @enable:
    mov a, !divNoiseFreq
    and a, #~$20
    mov !divNoiseFreq, a
    call !divInitEcho
    ret
  @disable:
    mov a, !divNoiseFreq
    or a, #$20
    mov !divNoiseFreq, a
    call !divInitEcho
    ret

divCmdSnesEchoFeedback:
  ; set echo feedback
  mov a, fcsArg0
  mov !divEchoFeedback, a
  ; check whether echo is on
  mov a, !divNoiseFreq
  and a, #$20
  bne +
  ; if we're here, echo is on. write feedback
  mov a, !divEchoFeedback
  dspWriteA $0d
+ ret

divCmdSnesEchoFir:
  ; set echo filter
  mov y, fcsArg0
  mov a, fcsArg1
  mov !divEchoFIR+y, a
  ; check whether echo is on
  mov a, !divNoiseFreq
  and a, #$20
  bne +
  ; calculate tap address
  mov a, fcsArg0
  xcn a
  or a, #$0f
  mov spc_dspAddr, a
  ; write tap
  mov spc_dspData, fcsArg1
+ ret

divCmdSnesEchoVolLeft:
  mov a, fcsArg0
  mov !divEchoVolL, a
  ; check whether echo is on
  ; TODO: do we really need to check?
  ; how about you remove the check?
  mov a, !divNoiseFreq
  and a, #$20
  bne +
  mov a, !divEchoVolL
  dspWriteA $2c
+ ret

divCmdSnesEchoVolRight:
  mov a, fcsArg0
  mov !divEchoVolR, a
  ; check whether echo is on
  ; TODO: do we really need to check?
  ; how about you remove the check?
  mov a, !divNoiseFreq
  and a, #$20
  bne +
  mov a, !divEchoVolR
  dspWriteA $3c
+ ret

divCmdSnesGlobalVolLeft:
  ; set dry vol left
  mov a, fcsArg0
  mov !divDryVolL, a
  ; set writeDryVol flag
  mov a, !divWriteFlags
  or a, #$08
  mov !divWriteFlags, a
  ret

divCmdSnesGlobalVolRight:
  ; set dry vol right
  mov a, fcsArg0
  mov !divDryVolR, a
  ; set writeDryVol flag
  mov a, !divWriteFlags
  or a, #$08
  mov !divWriteFlags, a
  ret

; TODO: macro control commands
divCmdMacroOff:
divCmdMacroOn:
divCmdMacroRestart:
  ret

fcsCmdTableSNESLow:
  .db <divCmdNoteOn
  .db <divCmdNoteOff
  .db <divCmdNoteOffEnv
  .db <divCmdEnvRelease
  .db <divCmdInstrument
  .db <divCmdVolume
  .db 0 ; unused (GET_VOLUME)
  .db 0 ; unused (GET_VOLMAX)
  .db <divCmdNotePorta ; note porta
  .db <divCmdPitch
  .db <divCmdPanning
  .db <divCmdLegato ; legato
  .db 0 ; pre porta
  .db 0 ; unused (PRE_NOTE)
  .db 0 ; unused (hint)
  .db 0 ; unused (hint)
  .db 0 ; unused (hint)
  .db 0 ; unused (hint)
  .db 0 ; unused (hint)
  .db 0 ; unused (hint)
  .db 0 ; unused (hint)
  .db 0 ; unused (hint)
  .db 0 ; unused (hint)
  .db 0 ; unused (hint)
  .db 0 ; unused (hint)
  .db 0 ; unused (hint)
  .db 0 ; unused (hint)
  .db 0 ; unused (hint)
  .db 0 ; sample mode
  .db 0 ; sample freq
  .db 0 ; sample bank
  .db <divCmdSamplePos ; sample pos
  .db 0 ; sample dir
  .db 0 ; FM hard reset
  .db 0 ; FM LFO
  .db 0 ; FM LFO wave
  .db 0 ; FM TL
  .db 0 ; FM AM
  .db <divCmdFmAr ; FM AR
  .db <divCmdFmDr ; FM DR
  .db <divCmdFmSl ; FM SL
  .db 0 ; FM D2R
  .db <divCmdFmRr ; FM RR
  .db 0 ; FM_DT
  .db 0 ; FM_DT2
  .db 0 ; FM_RS
  .db 0 ; FM_KSR
  .db 0 ; FM_VIB
  .db 0 ; FM_SUS
  .db 0 ; FM_WS
  .db 0 ; FM_SSG
  .db 0 ; FM_REV
  .db 0 ; FM_EG_SHIFT
  .db 0 ; FM_FB
  .db 0 ; FM_MULT
  .db 0 ; FM_FINE
  .db 0 ; FM_FIXFREQ
  .db 0 ; FM_EXTCH
  .db 0 ; FM_AM_DEPTH
  .db 0 ; FM_PM_DEPTH
  .db 0 ; FM_LFO2
  .db 0 ; FM_LFO2_WAVE
  .db <divCmdStdNoiseFreq ; STD_NOISE_FREQ
  .db <divCmdStdNoiseMode ; STD_NOISE_MODE
  .db <divCmdWave ; WAVE
  .db 0 ; GB_SWEEP_TIME
  .db 0 ; GB_SWEEP_DIR
  .db 0 ; PCE_LFO_MODE
  .db 0 ; PCE_LFO_SPEED
  .db 0 ; NES_SWEEP
  .db 0 ; NES_DMC
  .db 0 ; C64_CUTOFF
  .db 0 ; C64_RESONANCE
  .db 0 ; C64_FILTER_MODE
  .db 0 ; C64_RESET_TIME
  .db 0 ; C64_RESET_MASK
  .db 0 ; C64_FILTER_RESET
  .db 0 ; C64_DUTY_RESET
  .db 0 ; C64_EXTENDED
  .db 0 ; C64_FINE_DUTY
  .db 0 ; C64_FINE_CUTOFF
  .db 0 ; AY_ENVELOPE_SET
  .db 0 ; AY_ENVELOPE_LOW
  .db 0 ; AY_ENVELOPE_HIGH
  .db 0 ; AY_ENVELOPE_SLIDE
  .db 0 ; AY_NOISE_MASK_AND
  .db 0 ; AY_NOISE_MASK_OR
  .db 0 ; AY_AUTO_ENVELOPE
  .db 0 ; AY_IO_WRITE
  .db 0 ; AY_AUTO_PWM
  .db 0 ; FDS_MOD_DEPTH
  .db 0 ; FDS_MOD_HIGH
  .db 0 ; FDS_MOD_LOW
  .db 0 ; FDS_MOD_POS
  .db 0 ; FDS_MOD_WAVE
  .db 0 ; SAA_ENVELOPE
  .db 0 ; AMIGA_FILTER
  .db 0 ; AMIGA_AM
  .db 0 ; AMIGA_PM
  .db 0 ; LYNX_LFSR_LOAD
  .db 0 ; QSOUND_ECHO_FEEDBACK
  .db 0 ; QSOUND_ECHO_DELAY
  .db 0 ; QSOUND_ECHO_LEVEL
  .db 0 ; QSOUND_SURROUND
  .db 0 ; X1_010_ENVELOPE_SHAPE
  .db 0 ; X1_010_ENVELOPE_ENABLE
  .db 0 ; X1_010_ENVELOPE_MODE
  .db 0 ; X1_010_ENVELOPE_PERIOD
  .db 0 ; X1_010_ENVELOPE_SLIDE
  .db 0 ; X1_010_AUTO_ENVELOPE
  .db 0 ; X1_010_SAMPLE_BANK_SLOT
  .db 0 ; WS_SWEEP_TIME
  .db 0 ; WS_SWEEP_AMOUNT
  .db 0 ; N163_WAVE_POSITION
  .db 0 ; N163_WAVE_LENGTH
  .db 0 ; N163_WAVE_UNUSED1
  .db 0 ; N163_WAVE_UNUSED2
  .db 0 ; N163_WAVE_LOADPOS
  .db 0 ; N163_WAVE_LOADLEN
  .db 0 ; N163_WAVE_UNUSED3
  .db 0 ; N163_CHANNEL_LIMIT
  .db 0 ; N163_GLOBAL_WAVE_LOAD
  .db 0 ; N163_GLOBAL_WAVE_LOADPOS
  .db 0 ; N163_UNUSED4
  .db 0 ; N163_UNUSED5
  .db 0 ; SU_SWEEP_PERIOD_LOW
  .db 0 ; SU_SWEEP_PERIOD_HIGH
  .db 0 ; SU_SWEEP_BOUND
  .db 0 ; SU_SWEEP_ENABLE
  .db 0 ; SU_SYNC_PERIOD_LOW
  .db 0 ; SU_SYNC_PERIOD_HIGH
  .db 0 ; ADPCMA_GLOBAL_VOLUME
  .db <divCmdSnesEcho ; SNES_ECHO
  .db <divCmdSnesPitchMod ; SNES_PITCH_MOD
  .db <divCmdSnesInvert ; SNES_INVERT
  .db 0 ; SNES_GAIN_MODE
  .db <divCmdSnesGain ; SNES_GAIN
  .db <divCmdSnesEchoEnable ; SNES_ECHO_ENABLE
  .db <divCmdSnesEchoDelay ; SNES_ECHO_DELAY
  .db <divCmdSnesEchoVolLeft ; SNES_ECHO_VOL_LEFT
  .db <divCmdSnesEchoVolRight ; SNES_ECHO_VOL_RIGHT
  .db <divCmdSnesEchoFeedback ; SNES_ECHO_FEEDBACK
  .db <divCmdSnesEchoFir ; SNES_ECHO_FIR
  .db 0 ; NES_ENV_MODE
  .db 0 ; NES_LENGTH
  .db 0 ; NES_COUNT_MODE
  .db <divCmdMacroOff ; MACRO_OFF
  .db <divCmdMacroOn ; MACRO_ON
  .db 0 ; SURROUND_PANNING
  .db 0 ; FM_AM2_DEPTH
  .db 0 ; FM_PM2_DEPTH
  .db 0 ; ES5506_FILTER_MODE
  .db 0 ; ES5506_FILTER_K1
  .db 0 ; ES5506_FILTER_K2
  .db 0 ; ES5506_FILTER_K1_SLIDE
  .db 0 ; ES5506_FILTER_K2_SLIDE
  .db 0 ; ES5506_ENVELOPE_COUNT
  .db 0 ; ES5506_ENVELOPE_LVRAMP
  .db 0 ; ES5506_ENVELOPE_RVRAMP
  .db 0 ; ES5506_ENVELOPE_K1RAMP
  .db 0 ; ES5506_ENVELOPE_K2RAMP
  .db 0 ; ES5506_PAUSE
  .db 0 ; HINT_ARP_TIME
  .db <divCmdSnesGlobalVolLeft ; SNES_GLOBAL_VOL_LEFT
  .db <divCmdSnesGlobalVolRight ; SNES_GLOBAL_VOL_RIGHT
  .db 0 ; NES_LINEAR_LENGTH
  .db 0 ; EXTERNAL
  .db 0 ; C64_AD
  .db 0 ; C64_SR
  .db 0 ; ESFM_OP_PANNING
  .db 0 ; ESFM_OUTLVL
  .db 0 ; ESFM_MODIN
  .db 0 ; ESFM_ENV_DELAY
  .db <divCmdMacroRestart ; MACRO_RESTART

fcsCmdTableSNESHigh:
  .db >divCmdNoteOn
  .db >divCmdNoteOff
  .db >divCmdNoteOffEnv
  .db >divCmdEnvRelease
  .db >divCmdInstrument
  .db >divCmdVolume
  .db 0 ; unused (GET_VOLUME)
  .db 0 ; unused (GET_VOLMAX)
  .db >divCmdNotePorta ; note porta
  .db >divCmdPitch
  .db >divCmdPanning
  .db >divCmdLegato ; legato
  .db 0 ; pre porta
  .db 0 ; unused (PRE_NOTE)
  .db 0 ; unused (hint)
  .db 0 ; unused (hint)
  .db 0 ; unused (hint)
  .db 0 ; unused (hint)
  .db 0 ; unused (hint)
  .db 0 ; unused (hint)
  .db 0 ; unused (hint)
  .db 0 ; unused (hint)
  .db 0 ; unused (hint)
  .db 0 ; unused (hint)
  .db 0 ; unused (hint)
  .db 0 ; unused (hint)
  .db 0 ; unused (hint)
  .db 0 ; unused (hint)
  .db 0 ; sample mode
  .db 0 ; sample freq
  .db 0 ; sample bank
  .db >divCmdSamplePos ; sample pos
  .db 0 ; sample dir
  .db 0 ; FM hard reset
  .db 0 ; FM LFO
  .db 0 ; FM LFO wave
  .db 0 ; FM TL
  .db 0 ; FM AM
  .db >divCmdFmAr ; FM AR
  .db >divCmdFmDr ; FM DR
  .db >divCmdFmSl ; FM SL
  .db 0 ; FM D2R
  .db >divCmdFmRr ; FM RR
  .db 0 ; FM_DT
  .db 0 ; FM_DT2
  .db 0 ; FM_RS
  .db 0 ; FM_KSR
  .db 0 ; FM_VIB
  .db 0 ; FM_SUS
  .db 0 ; FM_WS
  .db 0 ; FM_SSG
  .db 0 ; FM_REV
  .db 0 ; FM_EG_SHIFT
  .db 0 ; FM_FB
  .db 0 ; FM_MULT
  .db 0 ; FM_FINE
  .db 0 ; FM_FIXFREQ
  .db 0 ; FM_EXTCH
  .db 0 ; FM_AM_DEPTH
  .db 0 ; FM_PM_DEPTH
  .db 0 ; FM_LFO2
  .db 0 ; FM_LFO2_WAVE
  .db >divCmdStdNoiseFreq ; STD_NOISE_FREQ
  .db >divCmdStdNoiseMode ; STD_NOISE_MODE
  .db >divCmdWave ; WAVE
  .db 0 ; GB_SWEEP_TIME
  .db 0 ; GB_SWEEP_DIR
  .db 0 ; PCE_LFO_MODE
  .db 0 ; PCE_LFO_SPEED
  .db 0 ; NES_SWEEP
  .db 0 ; NES_DMC
  .db 0 ; C64_CUTOFF
  .db 0 ; C64_RESONANCE
  .db 0 ; C64_FILTER_MODE
  .db 0 ; C64_RESET_TIME
  .db 0 ; C64_RESET_MASK
  .db 0 ; C64_FILTER_RESET
  .db 0 ; C64_DUTY_RESET
  .db 0 ; C64_EXTENDED
  .db 0 ; C64_FINE_DUTY
  .db 0 ; C64_FINE_CUTOFF
  .db 0 ; AY_ENVELOPE_SET
  .db 0 ; AY_ENVELOPE_LOW
  .db 0 ; AY_ENVELOPE_HIGH
  .db 0 ; AY_ENVELOPE_SLIDE
  .db 0 ; AY_NOISE_MASK_AND
  .db 0 ; AY_NOISE_MASK_OR
  .db 0 ; AY_AUTO_ENVELOPE
  .db 0 ; AY_IO_WRITE
  .db 0 ; AY_AUTO_PWM
  .db 0 ; FDS_MOD_DEPTH
  .db 0 ; FDS_MOD_HIGH
  .db 0 ; FDS_MOD_LOW
  .db 0 ; FDS_MOD_POS
  .db 0 ; FDS_MOD_WAVE
  .db 0 ; SAA_ENVELOPE
  .db 0 ; AMIGA_FILTER
  .db 0 ; AMIGA_AM
  .db 0 ; AMIGA_PM
  .db 0 ; LYNX_LFSR_LOAD
  .db 0 ; QSOUND_ECHO_FEEDBACK
  .db 0 ; QSOUND_ECHO_DELAY
  .db 0 ; QSOUND_ECHO_LEVEL
  .db 0 ; QSOUND_SURROUND
  .db 0 ; X1_010_ENVELOPE_SHAPE
  .db 0 ; X1_010_ENVELOPE_ENABLE
  .db 0 ; X1_010_ENVELOPE_MODE
  .db 0 ; X1_010_ENVELOPE_PERIOD
  .db 0 ; X1_010_ENVELOPE_SLIDE
  .db 0 ; X1_010_AUTO_ENVELOPE
  .db 0 ; X1_010_SAMPLE_BANK_SLOT
  .db 0 ; WS_SWEEP_TIME
  .db 0 ; WS_SWEEP_AMOUNT
  .db 0 ; N163_WAVE_POSITION
  .db 0 ; N163_WAVE_LENGTH
  .db 0 ; N163_WAVE_UNUSED1
  .db 0 ; N163_WAVE_UNUSED2
  .db 0 ; N163_WAVE_LOADPOS
  .db 0 ; N163_WAVE_LOADLEN
  .db 0 ; N163_WAVE_UNUSED3
  .db 0 ; N163_CHANNEL_LIMIT
  .db 0 ; N163_GLOBAL_WAVE_LOAD
  .db 0 ; N163_GLOBAL_WAVE_LOADPOS
  .db 0 ; N163_UNUSED4
  .db 0 ; N163_UNUSED5
  .db 0 ; SU_SWEEP_PERIOD_LOW
  .db 0 ; SU_SWEEP_PERIOD_HIGH
  .db 0 ; SU_SWEEP_BOUND
  .db 0 ; SU_SWEEP_ENABLE
  .db 0 ; SU_SYNC_PERIOD_LOW
  .db 0 ; SU_SYNC_PERIOD_HIGH
  .db 0 ; ADPCMA_GLOBAL_VOLUME
  .db >divCmdSnesEcho ; SNES_ECHO
  .db >divCmdSnesPitchMod ; SNES_PITCH_MOD
  .db >divCmdSnesInvert ; SNES_INVERT
  .db 0 ; SNES_GAIN_MODE
  .db >divCmdSnesGain ; SNES_GAIN
  .db >divCmdSnesEchoEnable ; SNES_ECHO_ENABLE
  .db >divCmdSnesEchoDelay ; SNES_ECHO_DELAY
  .db >divCmdSnesEchoVolLeft ; SNES_ECHO_VOL_LEFT
  .db >divCmdSnesEchoVolRight ; SNES_ECHO_VOL_RIGHT
  .db >divCmdSnesEchoFeedback ; SNES_ECHO_FEEDBACK
  .db >divCmdSnesEchoFir ; SNES_ECHO_FIR
  .db 0 ; NES_ENV_MODE
  .db 0 ; NES_LENGTH
  .db 0 ; NES_COUNT_MODE
  .db >divCmdMacroOff ; MACRO_OFF
  .db >divCmdMacroOn ; MACRO_ON
  .db 0 ; SURROUND_PANNING
  .db 0 ; FM_AM2_DEPTH
  .db 0 ; FM_PM2_DEPTH
  .db 0 ; ES5506_FILTER_MODE
  .db 0 ; ES5506_FILTER_K1
  .db 0 ; ES5506_FILTER_K2
  .db 0 ; ES5506_FILTER_K1_SLIDE
  .db 0 ; ES5506_FILTER_K2_SLIDE
  .db 0 ; ES5506_ENVELOPE_COUNT
  .db 0 ; ES5506_ENVELOPE_LVRAMP
  .db 0 ; ES5506_ENVELOPE_RVRAMP
  .db 0 ; ES5506_ENVELOPE_K1RAMP
  .db 0 ; ES5506_ENVELOPE_K2RAMP
  .db 0 ; ES5506_PAUSE
  .db 0 ; HINT_ARP_TIME
  .db >divCmdSnesGlobalVolLeft ; SNES_GLOBAL_VOL_LEFT
  .db >divCmdSnesGlobalVolRight ; SNES_GLOBAL_VOL_RIGHT
  .db 0 ; NES_LINEAR_LENGTH
  .db 0 ; EXTERNAL
  .db 0 ; C64_AD
  .db 0 ; C64_SR
  .db 0 ; ESFM_OP_PANNING
  .db 0 ; ESFM_OUTLVL
  .db 0 ; ESFM_MODIN
  .db 0 ; ESFM_ENV_DELAY
  .db >divCmdMacroRestart ; MACRO_RESTART
