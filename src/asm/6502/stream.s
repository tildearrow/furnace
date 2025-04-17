; Furnace Command Stream player for 6502 architecture
; written by tildearrow

; usage:
; call fcsInit - this sets A to 0 on success or 1 on failure
; call fcsTick on every frame/tick/whatever
; - call your dispatch implementation's tick function afterwards

; notes:
; - short pointers only
; - little-endian only!

.include "6502base.i"

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; constants
FCS_MAX_CHAN=8 ; maximum number of channels (up to 127, but see below!)
FCS_MAX_STACK=16 ; stack depth per channel (FCS_MAX_STACK*FCS_MAX_CHAN<256)

; player constants - change if necessary
fcsAddrBase=$20 ; player state address base
fcsZeroPage=$10 ; player state address base for zero-page-mandatory variables
fcsGlobalStack=$200 ; player stack (best placed in a page)
fcsPtr=$8000 ; pointer to command stream
fcsVolMax=fcsVolMaxExample ; pointer to max channel volume array

; calculated from player constants
fcsDelays=fcsPtr+8
fcsSpeedDial=fcsPtr+24

; command call table
; - see src/engine/dispatch.h for a list of commands to be potentially handled
;   - do not implement HINT commands - the player will never send these
;   - no need to implement commands not pertaining to the target system
; - a zero pointer means "don't handle"
fcsCmdTableLow=fcsCmdTableExample
fcsCmdTableHigh=fcsCmdTableExample

; variables
; these may be read by your command handling routines
fcsArg0=fcsAddrBase ; int
fcsArg1=fcsAddrBase+4 ; int
; something else
fcsTicks=fcsAddrBase+8 ; short
; temporary variables
fcsSendVolume=fcsAddrBase+10 ; char
fcsSendPitch=fcsAddrBase+11 ; char
fcsCmd=fcsAddrBase+16 ; char[8]
fcsTempPtr=fcsZeroPage ; short
; channel state
chanPC=fcsZeroPage+2 ; short
chanTicks=fcsAddrBase+24 ; short
chanStackPtr=fcsAddrBase+24+(FCS_MAX_CHAN*2) ; char
chanNote=fcsAddrBase+24+(FCS_MAX_CHAN*2)+1 ; char
chanVibratoPos=fcsAddrBase+24+(FCS_MAX_CHAN*4) ; char

; may be used for driver detection
fcsDriverInfo:
  .db "Furnace"
  .db 0

; note on null
fcsNoteOnNull:
  lda #0
  sta chanVibratoPos,x
  jsr fcsDispatchCmd
  rts

; note off, note off env, env release
fcsNoArgDispatch:
  jsr fcsDispatchCmd
  rts

fcsOneByteDispatch:
  tya
  pha
  jsr fcsReadNext
  sta fcsArg0
  pla
  tay
  jsr fcsDispatchCmd
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
  adc #>fcsPtr
  sta fcsTempPtr
  lda chanPC+1,x
  adc #<fcsPtr
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

; x: channel*2 (for speed... char variables are interleaved)
; read commands
fcsChannelCmd:
  ; read next byte
  jsr fcsReadNext

  ; process and read arguments
  ; if (a<0xb3)
  bmi fcsNote ; handle $00-$7f
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
  beq +
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
  lda fcsPtr+40,x
  sta chanPC,x
  bne -

  ; success
  lda #0
  rts

; floor(127*sin((x/64)*(2*pi)))
fcsVibTable:
  .db 0, 12, 24, 36, 48, 59, 70, 80, 89, 98, 105, 112, 117, 121, 124, 126
  .db 127, 126, 124, 121, 117, 112, 105, 98, 89, 80, 70, 59, 48, 36, 24, 12
  .db 0, -12, -24, -36, -48, -59, -70, -80, -89, -98, -105, -112, -117, -121, -124, -126
  .db -126, -126, -124, -121, -117, -112, -105, -98, -89, -80, -70, -59, -48, -36, -24, -12

; "dummy" implementation - example only!

fcsDummyFunc:
  rts

fcsVolMaxExample:
  .db 127, 127, 127, 127, 127, 127, 127, 127

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
