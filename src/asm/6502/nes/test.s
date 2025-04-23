; command stream player test - NES version
; NES because it's what I am most familiar with, besides Atari 2600

.include "nes.i"

pendingTick=$400
curChan=$401
joyInput=$402
joyPrev=$403

; program ROM
.BANK 1 SLOT 1
.ORGA $8000

main:
  ; initialize processor
  sei
  cld
  ldx #$ff
  txs
  ; clear all memory
  ldx #$07
  stx $01
  lda #$00
  sta $00
  tay
- sta ($00),y
  dey
  bne -
  dex
  stx $01
  bne -
  ; clear zero page
  ldx #$00
- sta $00,x
  dex
  bne -
initPlayer:
  ; initialize command stream player
  jsr fcsInit
initPPU:
  ; wait for PPU to be ready
  bit PPUSTATUS
  ; wait two frames
- bit PPUSTATUS
  bpl -
- bit PPUSTATUS
  bpl -
startPlayer:
  ; draw some text
  lda #$20
  sta PPUADDR
  lda #$40
  sta PPUADDR

  lda #<screenText
  sta $00
  lda #>screenText
  sta $01
  ldy #0
  clc
- lda ($00),y
  beq ++
  sta PPUDATA
  inc $00
  bne -
  inc $01
  bcc -

  ; set palette
++ bit PPUSTATUS
  lda #$3f
  sta PPUADDR
  lda #$00
  sta PPUADDR

  ldx #$00
- lda ppuPalette.w,x
  sta PPUDATA
  inx
  cpx #$10
  bne -

  ; set up PPU
  lda #$0e
  sta PPUMASK
  ; reset scroll position
  bit PPUSTATUS
  lda #$00
  sta PPUSCROLL
  sta PPUSCROLL
  ; set up VBlank interrupt
  lda #$80
  sta PPUCTRL
loop:
  lda pendingTick
  beq loop
  ; wait a bit so we can see the raster
  ldy #$ff
  ldx #$02

- dey
  bne -
  dex
  bne -

  ; raster time display
  lda #$0f
  sta PPUMASK

  jsr fcsTick
  lda #0
  sta pendingTick

  ; raster time display end
  lda #$0e
  sta PPUMASK

  jmp loop

.MACRO ppuPos
  bit PPUSTATUS
  lda #>($2000+((\1))+((\2)<<5))
  sta PPUADDR
  lda #<($2000+((\1))+((\2)<<5))
  sta PPUADDR
.ENDM

; interrupt handlers
nmi:
  php
  pha

  ; read controller
  lda joyInput
  sta joyPrev
  lda #$01
  sta JOY1
  sta joyInput
  lsr
  sta JOY1

- lda JOY1
  lsr
  rol joyInput
  bcc -

  ; PPU update
  ppuPos 10, 4

  lda curChan
  jsr printNum

  lda curChan
  rol
  tax

  ppuPos 12, 6

  lda chanPC.w+1,x
  jsr printNum
  lda chanPC.w,x
  jsr printNum

  ppuPos 12, 7

  lda chanTicks.w+1,x
  jsr printNum
  lda chanTicks.w,x
  jsr printNum

  ppuPos 14, 8
  lda chanNote.w,x
  jsr printNum

  ppuPos 14, 9
  lda chanPitch.w,x
  jsr printNum

  ppuPos 20, 9
  lda chanVibratoDebug.w,x
  jsr printNum

  ppuPos 14, 10
  lda chanVibrato.w,x
  jsr printNum

  ppuPos 14, 11
  lda chanVibratoPos.w,x
  jsr printNum

  ppuPos 14, 12
  lda chanArp.w,x
  jsr printNum

  ppuPos 20, 12
  lda chanArpTicks.w,x
  jsr printNum

  ppuPos 23, 12
  lda chanArpStage.w,x
  jsr printNum

  ppuPos 14, 13
  lda chanPortaSpeed.w,x
  jsr printNum

  ppuPos 20, 13
  lda chanPortaTarget.w,x
  jsr printNum

  ppuPos 12, 14
  lda chanVol.w+1,x
  jsr printNum
  lda chanVol.w,x
  jsr printNum

  ; end of PPU update
  bit PPUSTATUS
  lda #0
  sta PPUSCROLL
  sta PPUSCROLL

  ; process joy input
  lda joyInput
  eor joyPrev
  and joyInput

  ; left
  pha
  and #$02
  beq +
  dec curChan
  bpl +
  lda #0
  sta curChan

  ; right
+ pla
  and #$01
  beq +
  inc curChan

  ; set pendingTick
+ lda #1
  sta pendingTick
  
  ; end
  pla
  plp
  rti

irq:
  rti

printNum:
  pha
  lsr
  lsr
  lsr
  lsr
  tay
  lda hexChar.w,y
  sta PPUDATA
  pla
  and #$0f
  tay
  lda hexChar.w,y
  sta PPUDATA
  rts

; command stream player definition
FCS_MAX_CHAN=8

fcsAddrBase=$30
fcsZeroPage=$0e
fcsGlobalStack=$200
fcsPtr=cmdStream
fcsVolMax=volMaxArray

; command call table
fcsCmdTableLow=fcsCmdTableExample
fcsCmdTableHigh=fcsCmdTableExample

.include "../stream.s"

; data
screenText:
  .db "  Furnace Test Player           "
  .db "                                "
  .db "  channel 00                    "
  .db "                                "
  .db "  PC       $0000   (00)         "
  .db "  tick      0000                "
  .db "  note        00                "
  .db "  pitch       00   ~00          "
  .db "  vib         00                "
  .db "  vibPos      00                "
  .db "  arp         00   (00/00)      "
  .db "  porta       00 -> 00          "
  .db "  vol       0000                "
  .db "  pan       0000                "
  .db 0

hexChar:
  .db "0123456789ABCDEF"

ppuPalette:
  .db $0e, $00, $10, $30
  .db $0e, $00, $10, $30
  .db $0e, $00, $10, $30
  .db $0e, $00, $10, $30

volMaxArray:
  .db $7f, 00
  .db $7f, 00
  .db $7f, 00
  .db $7f, 00
  .db $7f, 00
  .db $7f, 00
  .db $7f, 00
  .db $7f, 00

.ORGA $9000

cmdStream:
  .incbin "../seq.bin"

; vectors
.ORGA $FFFA

.dw nmi
.dw main
.dw irq

; character ROM
.BANK 2 SLOT 2
.ORGA $10000

.incbin "chr.bin"
