; Furnace Command Stream player for 6502 architecture
; written by tildearrow

; usage:
; define the following constants:
; - FCS_MAX_CHAN: the number of channels in your stream
; - fcsAddrBase: player state address base
; - fcsZeroPage: player state address base for zero-page-mandatory variables
; - fcsGlobalStack: player stack (best placed in a page)
; - fcsPtr: pointer to command stream
; - fcsVolMax: pointer to max channel volume array
; - fcsCmdTableLow: low address of command table
; - fcsCmdTableHigh: high address of command table
; - see stream_example.i for an example
; call fcsInit - this sets A to 0 on success or 1 on failure
; call fcsTick on every frame/tick/whatever
; - call your dispatch implementation's tick function afterwards

; notes:
; - short pointers only
; - little-endian only!

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; calculated from player constants
fcsDelays=fcsPtr+8
fcsSpeedDial=fcsPtr+24
fcsStackSize=fcsPtr+40+(FCS_MAX_CHAN*2)

; variables
; these may be read by your command handling routines
fcsArg0=fcsAddrBase ; int
fcsArg1=fcsAddrBase+4 ; int
; something else
fcsTicks=fcsAddrBase+8 ; short
; temporary variables
fcsSendVolume=fcsAddrBase+10 ; char
fcsSendPitch=fcsAddrBase+11 ; char
fcsArpSpeed=fcsAddrBase+12 ; char
fcsCmd=fcsAddrBase+16 ; char[8]
fcsTempPtr=fcsZeroPage ; short
; channel state
chanPC=fcsZeroPage+2 ; short
chanTicks=fcsAddrBase+24 ; short
chanStackPtr=fcsAddrBase+24+(FCS_MAX_CHAN*2) ; char
chanNote=fcsAddrBase+24+(FCS_MAX_CHAN*2)+1 ; char
chanVibratoPos=fcsAddrBase+24+(FCS_MAX_CHAN*4) ; char
chanVibrato=fcsAddrBase+24+(FCS_MAX_CHAN*4)+1 ; char
chanPitch=fcsAddrBase+24+(FCS_MAX_CHAN*6) ; char
chanArp=fcsAddrBase+24+(FCS_MAX_CHAN*6)+1 ; char
chanPortaSpeed=fcsAddrBase+24+(FCS_MAX_CHAN*8) ; char
chanPortaTarget=fcsAddrBase+24+(FCS_MAX_CHAN*8)+1 ; char
chanVol=fcsAddrBase+24+(FCS_MAX_CHAN*10) ; short
chanVolSpeed=fcsAddrBase+24+(FCS_MAX_CHAN*12) ; short
chanPan=fcsAddrBase+24+(FCS_MAX_CHAN*14) ; short
chanArpStage=fcsAddrBase+24+(FCS_MAX_CHAN*16) ; char
chanArpTicks=fcsAddrBase+24+(FCS_MAX_CHAN*16)+1 ; char
chanVibratoDebug=fcsAddrBase+24+(FCS_MAX_CHAN*18) ; char

; may be used for driver detection
fcsDriverInfo:
  .db "Furnace"
  .db 0

; x: channel*2
; a is set to next byte
.MACRO fcsReadNext
  ; a=chanPC[x]
  lda (chanPC,x)
  ; increase PC
  inc chanPC,x
  bne +
  inc chanPC+1,x
+
.ENDM

; same as fcsReadNext, but don't change PC
.MACRO fcsPeekNext
  ; a=chanPC[x]
  lda (chanPC,x)
.ENDM

; note on null
fcsNoteOnNull:
  lda #0
  sta chanVibratoPos,x
  ldy #$00
  jsr fcsDispatchCmd
  rts

; note off, note off env, env release
fcsNoArgDispatchB4:
  tya
  sec
  sbc #$b4
  tay
  jsr fcsDispatchCmd
  rts

fcsOneByteDispatchB4:
  tya
  pha
  fcsReadNext
  sta fcsArg0
  pla
  sec
  sbc #$b4
  tay
  jsr fcsDispatchCmd
  rts

; dispatch subroutines for full commands
fcsNoArgDispatch:
  jsr fcsDispatchCmd
  rts

fcsOneByteDispatch:
  tya
  pha
  fcsReadNext
  sta fcsArg0
  pla
  tay
  jsr fcsDispatchCmd
  rts

fcsTwoByteDispatch:
  tya
  pha
  fcsReadNext
  sta fcsArg0
  fcsReadNext
  sta fcsArg1
  pla
  tay
  jsr fcsDispatchCmd
  rts

fcsOneShortDispatch:
  tya
  pha
  fcsReadNext
  sta fcsArg0
  fcsReadNext
  sta fcsArg0+1
  pla
  tay
  jsr fcsDispatchCmd
  rts

fcsTwoShortDispatch:
  tya
  pha
  fcsReadNext
  sta fcsArg0
  fcsReadNext
  sta fcsArg0+1
  fcsReadNext
  sta fcsArg1
  fcsReadNext
  sta fcsArg1+1
  pla
  tay
  jsr fcsDispatchCmd
  rts

fcsPrePorta:
  fcsReadNext
  pha
  and #$80
  sta fcsArg0
  pla
  and #$40
  sta fcsArg1
  ldy #$0c
  jsr fcsDispatchCmd
  rts

fcsArpTime:
  fcsReadNext
  sta fcsArpSpeed
  rts

fcsVibrato:
  fcsReadNext
  sta chanVibrato,x
  lda #1
  sta fcsSendPitch
  rts

; TODO
fcsVibRange:
fcsVibShape:
  fcsReadNext
  rts

fcsPitch:
  fcsReadNext
  sta chanPitch,x
  lda #1
  sta fcsSendPitch
  rts

fcsArpeggio:
  fcsReadNext
  sta chanArp,x
  rts

fcsVolume:
  fcsReadNext
  sta chanVol+1,x
  lda #0
  sta chanVol,x
  lda #1
  sta fcsSendVolume
  rts

fcsVolSlide:
  fcsReadNext
  sta chanVolSpeed,x
  fcsReadNext
  sta chanVolSpeed+1,x
  rts

fcsPorta:
  fcsReadNext
  sta chanPortaTarget,x
  fcsReadNext
  sta chanPortaSpeed,x
  rts

fcsLegato:
  fcsReadNext
  sta chanNote,x
  sta fcsArg0
  ldy #11
  jsr fcsDispatchCmd
  rts

fcsVolSlideTarget:
  fcsReadNext
  sta chanVolSpeed,x
  fcsReadNext
  sta chanVolSpeed+1,x
  ; TODO: we don't support this yet...
  fcsReadNext
  fcsReadNext
  rts

fcsNoOpOneByte:
  fcsReadNext
  rts

fcsPan:
  fcsReadNext
  sta chanPan.w,x
  sta fcsArg0
  fcsReadNext
  sta chanPan.w+1,x
  sta fcsArg1
  ldy #10
  jsr fcsDispatchCmd
  rts

fcsOptPlaceholder:
  fcsReadNext
  fcsReadNext
  fcsReadNext
  rts

fcsCallI:
  ; get address and relocate it
  fcsReadNext
  clc
  adc #<fcsPtr
  pha
  fcsReadNext
  adc #>fcsPtr
  pha
  ; ignore next two bytes
  jsr fcsIgnoreNext
  jsr fcsIgnoreNext
  ; fcsPushCall BEGIN
  ldy chanStackPtr,x
  lda chanPC,x
  sta fcsGlobalStack,y
  iny
  lda chanPC+1,x
  sta fcsGlobalStack,y
  iny
  sty chanStackPtr,x
  ; fcsPushCall END
  pla
  sta chanPC+1,x
  pla
  sta chanPC,x
  rts

fcsOffWait:
  ldy #0
  sty chanTicks+1,x
  iny
  sty chanTicks,x
  jsr fcsDispatchCmd
  rts

fcsFullCmd:
  ; read command
  fcsReadNext
  tay
  lda fcsFullCmdTable-28,y
  tay
  lda fcsCmdReadTableLow,y
  sta fcsTempPtr
  lda fcsCmdReadTableHigh,y
  sta fcsTempPtr+1
  jmp (fcsTempPtr)

fcsSpeedDialCmd:
  lda fcsSpeedDial-224,y
  pha
  tay
  lda fcsFullCmdTable-28,y
  tay
  lda fcsCmdReadTableLow,y
  sta fcsTempPtr
  lda fcsCmdReadTableHigh,y
  sta fcsTempPtr+1
  pla
  tay
  jmp (fcsTempPtr)

fcsCall:
  ; get address and relocate it
  fcsReadNext
  clc
  adc #<fcsPtr
  pha
  fcsReadNext
  adc #>fcsPtr
  pha
  ; fcsPushCall BEGIN
  ; push channel PC to stack
  ldy chanStackPtr,x
  lda chanPC,x
  sta fcsGlobalStack,y
  iny
  lda chanPC+1,x
  sta fcsGlobalStack,y
  iny
  sty chanStackPtr,x
  ; fcsPushCall END
  pla
  sta chanPC+1,x
  pla
  sta chanPC,x
  rts

; retrieve channel PC from stack
fcsRet:
  ldy chanStackPtr,x
  dey
  lda fcsGlobalStack,y
  sta chanPC+1,x
  dey
  lda fcsGlobalStack,y
  sta chanPC,x
  sty chanStackPtr,x
  rts

fcsJump:
  ; get address and relocate it
  fcsReadNext
  clc
  adc #<fcsPtr
  pha
  fcsPeekNext
  adc #>fcsPtr
  sta chanPC+1,x
  pla
  sta chanPC,x
  rts

; TODO
fcsTickRate:
  rts

fcsWaitS:
  sec
  fcsReadNext
  and #$ff
  sta chanTicks,x
  bne +
  clc
+ fcsReadNext
  adc #$ff
  sta chanTicks+1,x
  rts

fcsWaitC:
  fcsReadNext
  sta chanTicks,x
  lda #0
  sta chanTicks+1,x
  rts

fcsWait1:
  ldy #1
  sty chanTicks,x
  dey
  sty chanTicks+1,x
  rts

fcsStop:
  lda #0
  sta chanPC,x
  sta chanPC+1,x
  rts

fcsNoOp:
  rts

; x: channel*2
; y: command
fcsDispatchCmd:
  ; read command call table
  lda fcsCmdTableLow,y
  sta fcsTempPtr
  lda fcsCmdTableHigh,y
  sta fcsTempPtr+1
  ; check for zero
  lda fcsTempPtr
  ora fcsTempPtr
  beq + ; get out
  ; handle command in dispatch code
  jmp (fcsTempPtr)
  ; only if pointer is zero
+ rts

; x: channel*2
fcsIgnoreNext:
  ; increase PC
  inc chanPC,x
  bne +
  inc chanPC+1,x
+ rts

; x: channel*2 (for speed... char variables are interleaved)
; read commands
fcsChannelCmd:
  ; read next byte
  fcsReadNext
  and #$ff ; touch flags

  ; process and read arguments
  ; if (a<0xb3)
  bpl fcsNote ; handle $00-$7f
  cmp #$b4
  bpl fcsCheckOther

  ; this is a note
  fcsNote:
    sta fcsArg0
    sta chanNote,x
    lda #0
    tay
    sta chanVibratoPos,x
    ; call DIV_CMD_NOTE_ON
    jsr fcsDispatchCmd
    rts

  ; check other instructions
  fcsCheckOther:
    ; check for preset delays
    cmp #$f0
    bmi fcsOther

  ; handler for preset delays
  fcsPresetDelay:
    ; load preset delay and store it
    tay
    lda fcsPtr+8-240,y
    sta chanTicks,x
    lda #0
    sta chanTicks+1,x
    rts

  ; other instructions
  fcsOther:
    ; call respective handler
    tay
    lda fcsInsTableLow-180,y
    sta fcsTempPtr
    lda fcsInsTableHigh-180,y
    sta fcsTempPtr+1
    jmp (fcsTempPtr)

; this is called when vibrato depth is zero
fcsChanPitchShortcut:
  ; extend pitch sign
  lda chanPitch,x
  sta fcsArg0
  bmi +

  ; positive
  lda #0
  sta fcsArg0+1
  ; dispatch command
  ldy #9
  jsr fcsDispatchCmd
  ; end
  jmp fcsChanDoPorta

  ; negative
+ lda #$ff
  sta fcsArg0+1
  ; dispatch command
  ldy #9
  jsr fcsDispatchCmd
  ; end
  jmp fcsChanDoPorta

; x: channel*2
; stuff that goes after command reading
fcsChannelPost:
  ;;; DO VOLUME
  fcsChanDoVolume:
    ; if (sendVolume || chanVolSpeed[x]!=0)
    lda fcsSendVolume
    bne +
    lda chanVolSpeed,x
    ora chanVolSpeed+1,x
    beq fcsChanDoPitch
    ; increase volume
+   lda chanVol,x
    clc
    adc chanVolSpeed,x
    sta chanVol,x
    lda chanVol+1,x
    adc chanVolSpeed+1,x
    sta chanVol+1,x
    ; TODO: handle vol slide with target
    ; get sign of volume speed
    lda chanVolSpeed+1,x
    bpl fcsChanPlus
    fcsChanMinus:
      ; if (chanVol[x]<0)
      bcs fcsChanSubmitVol
      ; chanVol[x]=0
      lda #0
      sta chanVol,x
      sta chanVol+1,x
      beq fcsChanSubmitVol ; shortcut
    fcsChanPlus:
      ; if (chanVol[x]>=fcsVolMax[x] || CARRY)
      bcs + ; overflow check
      lda chanVol+1,x ; comparison check
      cmp fcsVolMax.w,x
      bmi fcsChanSubmitVol
      ; chanVol[x]=fcsVolMax[x]
+     lda fcsVolMax.w,x
      sta chanVol+1,x
      lda #0
      sta chanVol,x
    fcsChanSubmitVol:
      lda chanVol+1,x
      sta fcsArg0
      ldy #$05 ; volume
      jsr fcsDispatchCmd

  ;;; DO PITCH
  fcsChanDoPitch:
    ; check for vibrato depth
    lda chanVibrato,x
    and #$0f ; depth only
    bne +
    jmp fcsChanDoPitch1

    ; update vibrato
    ; 1. store vibrato pitch table offset
+   lda chanVibrato,x
    rol
    rol
    rol
    rol
    and #$f0
    sta fcsTempPtr
    ; 2. update vibrato position
    lda chanVibrato,x
    lsr
    lsr
    lsr
    lsr
    clc
    adc chanVibratoPos,x
    and #$3f
    sta chanVibratoPos,x 
    ; 3. calculate vibrato pitch
    ;   - we use 15 quarter sine tables, one for each vibrato depth
    ;   - 32-63 are negatives of 0-31
    ;   - a&31: zero is zero. otherwise a-1 in the table unless a&16
    ;   - if a&16 then invert a
    tay
    ; check for zero in a&31
    and #$1f
    bne +
    ; it is. load zero
    lda #0
    jmp fcsPostVibratoCalc1
    ; it is not. check a&16
+   and #$10
    bne +
    ; 0-15
    dey
    tya
    and #$0f
    jmp fcsPostVibratoCalc
    ; 16-31
+   tya
    and #$0f
    eor #$0f ; 0-15 -> 15-0

    fcsPostVibratoCalc:
      ; check for 32-63
      pha
      tya
      and #$20
      bne +
      ; 0-31
      pla
      clc
      adc fcsTempPtr
      tay
      lda fcsVibTable-16,y
      jmp fcsPostVibratoCalc1
      ; 32-63 (negate)
+     pla
      clc
      adc fcsTempPtr
      tay
      lda fcsVibTable-16,y
      eor #$ff
      clc
      adc #1

    ; at this point, a contains the vibrato pitch
    fcsPostVibratoCalc1:
      sta fcsArg0
      sta chanVibratoDebug,x
      ; extend sign
      bmi +
      lda #0
      sta fcsArg0+1
      beq ++
+     lda #$ff
      sta fcsArg0+1
      ; extend pitch sign
++    lda chanPitch,x
      sta fcsTempPtr
      bmi +
      lda #0
      sta fcsTempPtr+1
      beq ++
+     lda #$ff
      sta fcsTempPtr+1
      ; add pitch
++    lda fcsArg0
      clc
      adc fcsTempPtr
      sta fcsArg0
      lda fcsArg0+1
      adc fcsTempPtr+1
      sta fcsArg0+1
      ; dispatch command
      ldy #9
      jsr fcsDispatchCmd
      jmp fcsChanDoPorta

  ; check whether we should send pitch
  ; (vibrato depth is zero)
  fcsChanDoPitch1:
    lda fcsSendPitch
    beq fcsChanDoPorta
    jmp fcsChanPitchShortcut

  ;;; DO PORTAMENTO
  fcsChanDoPorta:
    ; if (chanPortaSpeed[x])
    lda chanPortaSpeed,x
    beq fcsChanDoArp
    ; do portamento
    sta fcsArg0
    lda #0
    sta fcsArg0+1
    lda chanPortaTarget,x
    sta fcsArg1
    ldy #8 ; NOTE_PORTA
    jsr fcsDispatchCmd
    ; get out (we can't do porta and arp simultaneously)
    rts

  ;;; DO ARPEGGIO
  fcsChanDoArp:
    ; if (chanArp[x] && !chanPortaSpeed[x])
    lda chanArp,x
    beq fcsChanEnd
    ; if (chanArpTicks[x]==0)
    lda chanArpTicks,x
    bne ++
    ; switch (chanArpStage[x])
    lda chanArpStage,x
    cmp #1 ; mi is 0, eq is 1 and pl is 2
    beq fcsChanArp1
    bpl fcsChanArp2
    ; bmi fcsChanArp0
    fcsChanArp0:
      lda chanNote,x 
      jmp fcsChanArpPost
    fcsChanArp1:
      lda chanArp,x
      lsr
      lsr
      lsr
      lsr
      clc
      adc chanNote,x
      jmp fcsChanArpPost
    fcsChanArp2:
      lda chanArp,x
      and #$0f
      clc
      adc chanNote,x
      jmp fcsChanArpPost

    fcsChanArpPost:
      sta fcsArg0
      ldy #11 ; legato
      jsr fcsDispatchCmd

    ; post-operations
    lda chanArpStage,x
    clc
    adc #1
    cmp #3
    ; if (chanArpStage[x]>=3) chanArpStage[x]=0
    bmi +
    lda #0
+   sta chanArpStage,x

    ; chanArpTicks[x]=fcsArpSpeed
    lda fcsArpSpeed
    sta chanArpTicks,x

    ; chanArpTicks[x]--
++  dec chanArpTicks,x

  ;;; END
  fcsChanEnd:
    rts

; x: channel*2
fcsDoChannel:
  ; initialize
  lda #0
  sta fcsSendVolume
  sta fcsSendPitch

  ; begin processing
  ; chanTicks--
+ lda chanTicks,x
  sec
  sbc #1
  sta chanTicks,x
  bne + ; skip if our counter isn't zero

  ; ticks lower is zero; check upper byte
  ldy chanTicks+1,x
  beq fcsDoChannelLoop ; go to read commands if it's zero as well
  ; decrease ticks upper
  dey
  sty chanTicks+1,x
  ; process channel stuff
  jsr fcsChannelPost
  rts

  ; ticks is zero... read commands until chanTicks is set
  fcsDoChannelLoop:
    lda chanTicks,x
    ora chanTicks+1,x
    bne + ; get out if chanTicks is no longer zero
    jsr fcsChannelCmd ; read next command
    jmp fcsDoChannelLoop

+ jsr fcsChannelPost
  ; end
  rts

fcsTick:
  ; update channel state
  ; for (x=0; x<FCS_MAX_CHAN; x++)
  ldx #0
- jsr fcsDoChannel
  inx
  inx
  cpx #FCS_MAX_CHAN*2
  bne -

  ; increase tick counter
  inc fcsTicks
  bne +
  inc fcsTicks+1

  ; end
+ rts

fcsInit:
  ; set all tick counters to 1
  lda #1
  ldy #0
  ldx #(FCS_MAX_CHAN*2)
- dex
  sty chanTicks,x
  dex
  sta chanTicks,x
  bne -

  ; set channel program counters
  ldx #(FCS_MAX_CHAN*2)
- dex
  lda fcsPtr+40.w,x
  sta chanPC,x
  cpx #0
  bne -

  ; relocate program counters
  ldx #0
- clc
  lda chanPC,x
  adc #<fcsPtr
  sta chanPC,x
  inx
  lda chanPC,x
  adc #>fcsPtr
  sta chanPC,x
  inx
  cpx #(FCS_MAX_CHAN*2)
  bne -

  ; initialize channel stacks
  lda #0
  ldx #0
  ldy #0
- sta chanStackPtr,x
  clc
  adc fcsStackSize.w,y
  clc
  adc fcsStackSize.w,y
  inx
  inx
  iny
  cpx #(FCS_MAX_CHAN*2)
  bne -

  ; set volumes
  ldx #0
- lda fcsVolMax.w,x
  sta chanVol+1,x
  inx
  inx
  cpx #(FCS_MAX_CHAN*2)
  bne -

  ; set arp speed
  lda #1
  sta fcsArpSpeed

  ; success
  lda #0
  rts

; floor(127*sin((x/64)*(2*pi))*depth/15)
fcsVibTable:
  .db 0, 1, 2, 3, 3, 4, 5, 5, 6, 7, 7, 7, 8, 8, 8, 8
  .db 1, 3, 4, 6, 7, 9, 10, 11, 13, 14, 14, 15, 16, 16, 16, 16
  .db 2, 4, 7, 9, 11, 14, 16, 17, 19, 21, 22, 23, 24, 24, 25, 25
  .db 3, 6, 9, 12, 15, 18, 21, 23, 26, 28, 29, 31, 32, 33, 33, 33
  .db 4, 8, 12, 16, 19, 23, 26, 29, 32, 35, 37, 39, 40, 41, 42, 42
  .db 4, 9, 14, 19, 23, 28, 32, 35, 39, 42, 44, 46, 48, 49, 50, 50
  .db 5, 11, 16, 22, 27, 32, 37, 41, 45, 49, 52, 54, 56, 57, 58, 59
  .db 6, 12, 19, 25, 31, 37, 42, 47, 52, 56, 59, 62, 64, 66, 67, 67
  .db 7, 14, 21, 28, 35, 42, 48, 53, 58, 63, 67, 70, 72, 74, 75, 76
  .db 8, 16, 24, 32, 39, 46, 53, 59, 65, 70, 74, 78, 80, 82, 84, 84
  .db 8, 17, 26, 35, 43, 51, 58, 65, 71, 77, 82, 85, 88, 90, 92, 93
  .db 9, 19, 28, 38, 47, 56, 64, 71, 78, 84, 89, 93, 96, 99, 100, 101
  .db 10, 20, 31, 41, 51, 60, 69, 77, 84, 91, 97, 101, 104, 107, 109, 110
  .db 11, 22, 33, 44, 55, 65, 74, 83, 91, 98, 104, 109, 112, 115, 117, 118
  .db 12, 24, 36, 48, 59, 70, 80, 89, 98, 105, 112, 117, 121, 124, 126, 127

; COMMAND TABLE
; $b4 fcsNoArgDispatch,
; $b5 fcsNoArgDispatch,
; $b6 fcsNoArgDispatch,
; $b7 fcsNoArgDispatch,
; $b8 fcsOneByteDispatch,
; $b9 fcsNoOp,
; $ba fcsNoOp,
; $bb fcsNoOp,
; $bc fcsNoOp,
; $bd fcsNoOp,
; $be fcsNoOp,
; $bf fcsNoOp,
; $c0 fcsPrePorta,
; $c1 fcsArpTime,
; $c2 fcsVibrato,
; $c3 fcsVibRange,
; $c4 fcsVibShape,
; $c5 fcsPitch,
; $c6 fcsArpeggio,
; $c7 fcsVolume,
; $c8 fcsVolSlide,
; $c9 fcsPorta,
; $ca fcsLegato,
; $cb fcsVolSlideTarget,
; $cc fcsNoOpOneByte,
; $cd fcsNoOpOneByte,
; $ce fcsNoOpOneByte,
; $cf fcsPan,
; $d0 fcsOptPlaceholder,
; $d1 fcsNoOp,
; $d2 fcsNoOp,
; $d3 fcsNoOp,
; $d4 fcsNoOp,
; $d5 fcsCallI,
; $d6 fcsOffWait,
; $d7 fcsFullCmd,
; $d8 fcsCall,
; $d9 fcsRet,
; $da fcsJump,
; $db fcsTickRate,
; $dc fcsWaitS,
; $dd fcsWaitC,
; $de fcsWait1,
; $df fcsStop,
fcsInsTableHigh:
  .db >fcsNoArgDispatchB4, >fcsNoArgDispatchB4, >fcsNoArgDispatchB4, >fcsNoArgDispatchB4, >fcsOneByteDispatchB4, >fcsNoOp, >fcsNoOp, >fcsNoOp, >fcsNoOp, >fcsNoOp, >fcsNoOp, >fcsNoOp
  .db >fcsPrePorta, >fcsArpTime, >fcsVibrato, >fcsVibRange, >fcsVibShape, >fcsPitch, >fcsArpeggio, >fcsVolume, >fcsVolSlide, >fcsPorta, >fcsLegato, >fcsVolSlideTarget, >fcsNoOpOneByte, >fcsNoOpOneByte, >fcsNoOpOneByte, >fcsPan
  .db >fcsOptPlaceholder, >fcsNoOp, >fcsNoOp, >fcsNoOp, >fcsNoOp, >fcsCallI, >fcsOffWait, >fcsFullCmd, >fcsCall, >fcsRet, >fcsJump, >fcsTickRate, >fcsWaitS, >fcsWaitC, >fcsWait1, >fcsStop
  .db >fcsSpeedDialCmd, >fcsSpeedDialCmd, >fcsSpeedDialCmd, >fcsSpeedDialCmd, >fcsSpeedDialCmd, >fcsSpeedDialCmd, >fcsSpeedDialCmd, >fcsSpeedDialCmd, >fcsSpeedDialCmd, >fcsSpeedDialCmd, >fcsSpeedDialCmd, >fcsSpeedDialCmd, >fcsSpeedDialCmd, >fcsSpeedDialCmd, >fcsSpeedDialCmd, >fcsSpeedDialCmd

fcsInsTableLow:
  .db <fcsNoArgDispatchB4, <fcsNoArgDispatchB4, <fcsNoArgDispatchB4, <fcsNoArgDispatchB4, <fcsOneByteDispatchB4, <fcsNoOp, <fcsNoOp, <fcsNoOp, <fcsNoOp, <fcsNoOp, <fcsNoOp, <fcsNoOp
  .db <fcsPrePorta, <fcsArpTime, <fcsVibrato, <fcsVibRange, <fcsVibShape, <fcsPitch, <fcsArpeggio, <fcsVolume, <fcsVolSlide, <fcsPorta, <fcsLegato, <fcsVolSlideTarget, <fcsNoOpOneByte, <fcsNoOpOneByte, <fcsNoOpOneByte, <fcsPan
  .db <fcsOptPlaceholder, <fcsNoOp, <fcsNoOp, <fcsNoOp, <fcsNoOp, <fcsCallI, <fcsOffWait, <fcsFullCmd, <fcsCall, <fcsRet, <fcsJump, <fcsTickRate, <fcsWaitS, <fcsWaitC, <fcsWait1, <fcsStop
  .db <fcsSpeedDialCmd, <fcsSpeedDialCmd, <fcsSpeedDialCmd, <fcsSpeedDialCmd, <fcsSpeedDialCmd, <fcsSpeedDialCmd, <fcsSpeedDialCmd, <fcsSpeedDialCmd, <fcsSpeedDialCmd, <fcsSpeedDialCmd, <fcsSpeedDialCmd, <fcsSpeedDialCmd, <fcsSpeedDialCmd, <fcsSpeedDialCmd, <fcsSpeedDialCmd, <fcsSpeedDialCmd

fcsCmdReadTableHigh:
  .db >fcsNoArgDispatch
  .db >fcsOneByteDispatch
  .db >fcsTwoByteDispatch
  .db >fcsOneShortDispatch
  .db >fcsTwoShortDispatch

fcsCmdReadTableLow:
  .db <fcsNoArgDispatch
  .db <fcsOneByteDispatch
  .db <fcsTwoByteDispatch
  .db <fcsOneShortDispatch
  .db <fcsTwoShortDispatch

fcsFullCmdTable:
  ; starting from $1c
  .db 1
  .db 1
  .db 1
  .db 4
  .db 1
  ; FM commands
  .db 1
  .db 1
  .db 1
  .db 2
  .db 2
  .db 2
  .db 2
  .db 2
  .db 2
  .db 2
  .db 2
  .db 2
  .db 2
  .db 2
  .db 2
  .db 2
  .db 2
  .db 2
  .db 2
  .db 2
  .db 1
  .db 2
  .db 2
  .db 3
  .db 1
  .db 1
  .db 1
  .db 1
  .db 1
  ; PSG commands
  .db 1
  .db 1
  .db 1
  ; Game Boy commands
  .db 1
  .db 1
  ; PC Engine commands
  .db 1
  .db 1
  ; NES
  .db 1
  .db 1
  ; C64
  .db 1
  .db 1
  .db 1
  .db 1
  .db 1
  .db 1
  .db 1
  .db 1
  .db 3
  .db 3
  ; AY commands
  .db 1
  .db 1
  .db 1
  .db 1
  .db 1
  .db 1
  .db 1
  .db 2
  .db 2
  ; FDS
  .db 1
  .db 1
  .db 1
  .db 1
  .db 1
  ; SAA1099
  .db 1
  ; Amiga
  .db 1
  .db 1
  .db 1
  ; Lynx
  .db 3
  ; QSound
  .db 1
  .db 3
  .db 1
  .db 1
  ; X1-010
  .db 1
  .db 1
  .db 1
  .db 1
  .db 1
  .db 1
  .db 1
  ; WonderSwan
  .db 1
  .db 1
  ; Namco 163
  .db 1
  .db 1
  .db 1
  .db 1
  .db 1
  .db 1
  .db 1
  .db 1
  .db 1
  .db 1
  .db 1
  .db 1
  ; Sound Unit
  .db 2
  .db 2
  .db 2
  .db 2
  .db 1
  .db 1
  ; ADPCM-A
  .db 1
  ; SNES
  .db 1
  .db 1
  .db 1
  .db 1
  .db 1
  .db 1
  .db 1
  .db 1
  .db 1
  .db 1
  .db 2
  ; NES
  .db 1
  .db 1
  .db 1
  ; macro control
  .db 1
  .db 1
  ; surround
  .db 2
  ; FM
  .db 1
  .db 1
  ; ES5506
  .db 1
  .db 4
  .db 4
  .db 2
  .db 2
  .db 3
  .db 1
  .db 1
  .db 2
  .db 2
  .db 1
  ; unused gap
  .db 1
  ; SNES
  .db 1
  .db 1
  ; NES linear counter
  .db 1
  ; ext cmd
  .db 1
  ; C64
  .db 1
  .db 1
  ; ESFM
  .db 2
  .db 2
  .db 2
  .db 2
  ; restart macro
  .db 1
  ; PowerNoise
  .db 2
  .db 2
  ; Dave
  .db 1
  .db 1
  .db 1
  .db 1
  .db 1
  ; MinMod
  .db 1
  ; Bifurcator
  .db 2
  .db 2
  ; FDS AutoMod
  .db 1
  ; OpMask
  .db 1
  ; MultiPCM
  .db 1
  .db 1
  .db 1
  .db 1
  .db 1
  .db 1
  .db 1
  .db 1
  .db 1
  .db 1
  .db 1
  .db 1
  .db 1
  .db 1
  .db 1
  ; SID3
  .db 1
  .db 1
  .db 1
  .db 1
  .db 1
  .db 2
  .db 1
  .db 2
  .db 2
  .db 1
  .db 1
  .db 1
  .db 1
  ; slide
  .db 2
  .db 2
  ; SID3 continued
  .db 1
  .db 1
  .db 1
  .db 1
  .db 1
  ; WonderSwan speaker vol
  .db 1
