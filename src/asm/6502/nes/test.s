; command stream player test - NES version
; NES because it's what I am most familiar with, besides Atari 2600

.include "nes.i"

pendingTick=$400

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
  lda #$21
  sta PPUADDR
  lda #$02
  sta PPUADDR

  ldx #$00
- lda helloWorld.w,x
  beq +
  sta PPUDATA
  inx
  bne -

  ; set palette
+ bit PPUSTATUS
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
  ldx #$03

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

; interrupt handlers
nmi:
  lda #1
  sta pendingTick
  rti

irq:
  rti

; command stream player definition
FCS_MAX_CHAN=8
FCS_MAX_STACK=10

fcsAddrBase=$20
fcsZeroPage=$10
fcsGlobalStack=$200
fcsPtr=cmdStream
fcsVolMax=volMaxArray

; command call table
fcsCmdTableLow=fcsCmdTableExample
fcsCmdTableHigh=fcsCmdTableExample

.include "../stream.s"

; data
helloWorld:
  .db "Hello, World!"
  .db 0

ppuPalette:
  .db $0e, $00, $10, $30
  .db $0e, $00, $10, $30
  .db $0e, $00, $10, $30
  .db $0e, $00, $10, $30

volMaxArray:
  .dw $40, $40, $40, $40

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
