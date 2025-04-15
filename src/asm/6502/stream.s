; Furnace Command Stream player for 6502 architecture
; written by tildearrow

; usage:
; call fcsInit - this sets A to 0 on success or 1 on failure
; call fcsTick on every frame/tick/whatever
; - call your dispatch implementation's tick function afterwards

; notes:
; - short pointers only
; - little-endian only!

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; constants
FCS_MAX_CHANS=8 ; maximum number of channels (up to 127, but see below!)
FCS_MAX_STACK=16 ; stack depth per channel (FCS_MAX_STACK*FCS_MAX_CHAN<256)

; player constants - change if necessary
fcsAddrBase=$20 ; player state address base
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
fcsCmdTable=fcsCmdTableExample

; variables
; these may be read by your command handling routines
fcsArg0=fcsAddrBase ; int
fcsArg1=fcsAddrBase+4 ; int
; something else
fcsTicks=fcsAddrBase+8 ; short
; temporary variables
fcsSendVolume=fcsAddrBase+10 ; char
fcsSendPitch=fcsAddrBase+11 ; char
; channel state
chanPC=fcsAddrBase+16 ; short
chanTicks=fcsAddrBase+16+(FCS_MAX_CHAN*2) ; short
chanStackPtr=fcsAddrBase+16+(FCS_MAX_CHAN*4) ; char

; may be used for driver detection
fcsDriverInfo:
  .byte "Furnace"
  .byte 0

; x: channel*2 (for speed... char variables are interleaved)
; read commands
fcsChannelCmd:
  ; read next byte

  ; process and read arguments

  ; end
  rts

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
  beq :+
  rts
  ; channel not halted... begin processing
  ; chanTicks--
: lda chanTicks,x
  sec
  sbc #1
  sta chanTicks,x
  bne :+ ; skip if our counter isn't zero

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
    bne :+ ; get out if chanTicks is no longer zero
    jsr fcsChannelCmd ; read next command
    jmp fcsDoChannelLoop

: jsr fcsChannelPost
  ; end
  rts

fcsTick:
  ; update channel state
  ; for (x=0; x<FCS_MAX_CHANS; x++)
  ldx #0
: jsr fcsDoChannel
  inx
  inx
  cpx #FCS_MAX_CHANS*2
  bne :-

  ; increase tick counter
  inc fcsTicks
  bne :+
  inc fcsTicks+1
  
  ; end
: rts

fcsInit:
  ; set all tick counters to 1
  lda #1
  ldy #0
  ldx #(FCS_MAX_CHANS*2)-1
: sty fcsTicks,x
  dex
  sta fcsTicks,x
  dex
  bne :-

  ; set channel program counters
  ldx #(FCS_MAX_CHANS*2)-1
: lda fcsPtr+40,x
  sta chanPC,x
  dex
  bne :-

  ; TODO: relocate and more...

  ; success
  lda #0
  rts

; floor(127*sin((x/64)*(2*pi)))
fcsVibTable:
  .byte 0, 12, 24, 36, 48, 59, 70, 80, 89, 98, 105, 112, 117, 121, 124, 126
  .byte 127, 126, 124, 121, 117, 112, 105, 98, 89, 80, 70, 59, 48, 36, 24, 12
  .byte 0, -12, -24, -36, -48, -59, -70, -80, -89, -98, -105, -112, -117, -121, -124, -126
  .byte -126, -126, -124, -121, -117, -112, -105, -98, -89, -80, -70, -59, -48, -36, -24, -12

; "dummy" implementation - example only!

fcsDummyFunc:
  rts

fcsVolMaxExample:
  .byte 127, 127, 127, 127, 127, 127, 127, 127

; first 64 commands
fcsCmdTableExample:
  .word 0, 0, 0, 0, 0, 0, 0, 0
  .word 0, 0, 0, 0, 0, 0, 0, 0
  .word 0, 0, 0, 0, 0, 0, 0, 0
  .word 0, 0, 0, 0, 0, 0, 0, 0
  .word 0, 0, 0, 0, 0, 0, 0, 0
  .word 0, 0, 0, 0, 0, 0, 0, 0
  .word 0, 0, 0, 0, 0, 0, 0, 0
  .word 0, 0, 0, 0, 0, 0, 0, 0
