; Furnace Command Stream player for 6502 architecture
; written by tildearrow

; usage:
; define the following constants:
; - FCS_MAX_CHAN: the number of channels in your stream
; - FCS_MAX_STACK: the maximum stack size
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

; may be used for driver detection
fcsDriverInfo:
  .db "Furnace"
  .db 0

; note on null
fcsNoteOnNull:
  lda #0
  sta chanVibratoPos,x
  ldy #$00
  jsr fcsDispatchCmd
  rts

; note off, note off env, env release
fcsNoArgDispatch:
  tya
  sec
  sbc #$b4
  tay
  jsr fcsDispatchCmd
  rts

fcsOneByteDispatch:
  tya
  pha
  jsr fcsReadNext
  sta fcsArg0
  pla
  sec
  sbc #$b4
  tay
  jsr fcsDispatchCmd
  rts

fcsPrePorta:
  jsr fcsReadNext
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
  jsr fcsReadNext
  sta fcsArpSpeed
  rts

fcsVibrato:
  jsr fcsReadNext
  sta chanVibrato,x
  rts

; TODO
fcsVibRange:
fcsVibShape:
  jsr fcsReadNext
  rts

fcsPitch:
  jsr fcsReadNext
  sta chanPitch,x
  lda #1
  sta fcsSendPitch
  rts

fcsArpeggio:
  jsr fcsReadNext
  sta chanArp,x
  rts

fcsVolume:
  jsr fcsReadNext
  sta chanVol+1,x
  lda #0
  sta chanVol,x
  lda #1
  sta fcsSendVolume
  rts

fcsVolSlide:
  jsr fcsReadNext
  sta chanVolSpeed,x
  jsr fcsReadNext
  sta chanVolSpeed+1,x
  rts

fcsPorta:
  jsr fcsReadNext
  sta chanPortaTarget,x
  jsr fcsReadNext
  sta chanPortaSpeed,x
  rts

fcsLegato:
  jsr fcsReadNext
  sta chanNote,x
  sta fcsArg0
  ldy #11
  jsr fcsDispatchCmd
  rts

fcsVolSlideTarget:
  jsr fcsReadNext
  sta chanVolSpeed,x
  jsr fcsReadNext
  sta chanVolSpeed+1,x
  ; TODO: we don't support this yet...
  jsr fcsReadNext
  jsr fcsReadNext
  rts

fcsNoOpOneByte:
  jsr fcsReadNext
  rts

fcsPan:
  jsr fcsReadNext
  sta chanPan,x
  sta fcsArg0
  jsr fcsReadNext
  sta chanPan+1,x
  sta fcsArg1
  ldy #10
  jsr fcsDispatchCmd
  rts

fcsOptPlaceholder:
  jsr fcsReadNext
  jsr fcsReadNext
  jsr fcsReadNext
  rts

fcsCallI:
  ; get address
  jsr fcsReadNext
  pha
  jsr fcsReadNext
  pha
  ; ignore next two bytes
  jsr fcsIgnoreNext
  jsr fcsIgnoreNext
  jsr fcsPushCall
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

; TODO
fcsFullCmd:
  rts

fcsCall:
  ; get address
  jsr fcsReadNext
  pha
  jsr fcsReadNext
  pha
  jsr fcsPushCall
  pla
  sta chanPC+1,x
  pla
  sta chanPC,x
  rts

; push channel PC to stack
fcsPushCall:
  lda chanStackPtr,x
  tay
  lda chanPC,x
  sta fcsGlobalStack,y
  iny
  lda chanPC+1,x
  sta fcsGlobalStack,y
  iny
  tya
  sta chanStackPtr,x
  rts

; retrieve channel PC from stack
fcsRet:
  lda chanStackPtr,x
  tay
  dey
  lda fcsGlobalStack,y
  sta chanPC+1,x
  dey
  lda fcsGlobalStack,y
  sta chanPC,x
  tya
  sta chanStackPtr,x
  rts

fcsJump:
  ; get address
  jsr fcsReadNext
  tay
  jsr fcsReadNext
  ; ignore next two bytes
  jsr fcsIgnoreNext
  jsr fcsIgnoreNext
  ; a has high byte
  ; y has low byte
  sty chanPC,x
  sta chanPC+1,x
  rts

; TODO
fcsTickRate:
  rts

fcsWaitS:
  jsr fcsReadNext
  sta chanTicks,x
  jsr fcsReadNext
  sta chanTicks+1,x
  rts

fcsWaitC:
  jsr fcsReadNext
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
; a is set to next byte
fcsReadNext:
  ; a=chanPC[x]+fcsPtr
  clc
  lda chanPC,x
  adc #<fcsPtr
  sta fcsTempPtr
  lda chanPC+1,x
  adc #>fcsPtr
  sta fcsTempPtr+1

  ; increase PC
  inc chanPC,x
  bne +
  inc chanPC+1,x

  ; read byte and put it into a
  ; this is at the end to ensure flags are set properly
+ ldy #0
  lda (fcsTempPtr),y
  rts

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
  jsr fcsReadNext

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

; x: channel*2
; stuff that goes after command reading
fcsChannelPost:
  ; do volume

  ; do pitch

  ; do portamento

  ; do arpeggio

  ; end
  rts

; x: channel*2
fcsDoChannel:
  ; initialize
  lda #0
  sta fcsSendVolume
  sta fcsSendPitch

  ; check whether this channel is halted (PC = 0)
  lda chanPC,x
  ora chanPC+1,x
  bne +
  rts
  ; channel not halted... begin processing
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

  ; initialize channel stacks
  lda #0
  ldx #0
- sta chanStackPtr,x
  clc
  adc #FCS_MAX_STACK
  inx
  inx
  cpx #(FCS_MAX_CHAN*2)
  bne -
  

  ; set volumes
  ; TODO

  ; success
  lda #0
  rts

; floor(127*sin((x/64)*(2*pi)))
fcsVibTable:
  .db 0, 12, 24, 36, 48, 59, 70, 80, 89, 98, 105, 112, 117, 121, 124, 126
  .db 127, 126, 124, 121, 117, 112, 105, 98, 89, 80, 70, 59, 48, 36, 24, 12
  .db 0, -12, -24, -36, -48, -59, -70, -80, -89, -98, -105, -112, -117, -121, -124, -126
  .db -126, -126, -124, -121, -117, -112, -105, -98, -89, -80, -70, -59, -48, -36, -24, -12

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
  .db >fcsNoArgDispatch, >fcsNoArgDispatch, >fcsNoArgDispatch, >fcsNoArgDispatch, >fcsOneByteDispatch, >fcsNoOp, >fcsNoOp, >fcsNoOp, >fcsNoOp, >fcsNoOp, >fcsNoOp, >fcsNoOp
  .db >fcsPrePorta, >fcsArpTime, >fcsVibrato, >fcsVibRange, >fcsVibShape, >fcsPitch, >fcsArpeggio, >fcsVolume, >fcsVolSlide, >fcsPorta, >fcsLegato, >fcsVolSlideTarget, >fcsNoOpOneByte, >fcsNoOpOneByte, >fcsNoOpOneByte, >fcsPan
  .db >fcsOptPlaceholder, >fcsNoOp, >fcsNoOp, >fcsNoOp, >fcsNoOp, >fcsCallI, >fcsOffWait, >fcsFullCmd, >fcsCall, >fcsRet, >fcsJump, >fcsTickRate, >fcsWaitS, >fcsWaitC, >fcsWait1, >fcsStop

fcsInsTableLow:
  .db <fcsNoArgDispatch, <fcsNoArgDispatch, <fcsNoArgDispatch, <fcsNoArgDispatch, <fcsOneByteDispatch, <fcsNoOp, <fcsNoOp, <fcsNoOp, <fcsNoOp, <fcsNoOp, <fcsNoOp, <fcsNoOp
  .db <fcsPrePorta, <fcsArpTime, <fcsVibrato, <fcsVibRange, <fcsVibShape, <fcsPitch, <fcsArpeggio, <fcsVolume, <fcsVolSlide, <fcsPorta, <fcsLegato, <fcsVolSlideTarget, <fcsNoOpOneByte, <fcsNoOpOneByte, <fcsNoOpOneByte, <fcsPan
  .db <fcsOptPlaceholder, <fcsNoOp, <fcsNoOp, <fcsNoOp, <fcsNoOp, <fcsCallI, <fcsOffWait, <fcsFullCmd, <fcsCall, <fcsRet, <fcsJump, <fcsTickRate, <fcsWaitS, <fcsWaitC, <fcsWait1, <fcsStop


; "dummy" implementation - example only!

fcsDummyFunc:
  rts

fcsVolMaxExample:
  .dw $7f00
  .dw $7f00
  .dw $7f00
  .dw $7f00
  .dw $7f00
  .dw $7f00
  .dw $7f00
  .dw $7f00

; first 64 commands
fcsCmdTableExample:
  .db 0, 0, 0, 0, 0, 0, 0, 0
  .db 0, 0, 0, 0, 0, 0, 0, 0
  .db 0, 0, 0, 0, 0, 0, 0, 0
  .db 0, 0, 0, 0, 0, 0, 0, 0
  .db 0, 0, 0, 0, 0, 0, 0, 0
  .db 0, 0, 0, 0, 0, 0, 0, 0
  .db 0, 0, 0, 0, 0, 0, 0, 0
  .db 0, 0, 0, 0, 0, 0, 0, 0
