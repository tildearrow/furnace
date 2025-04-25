; command stream player test - NES version
; NES because it's what I am most familiar with, besides Atari 2600

.include "nes.i"

pendingTick=$400
curChan=$401
joyInput=$402
joyPrev=$403

tempFreq=$06
mulTemp0=$08
mulTemp1=$09
tempNote=$0a
tempOctave=$0b
temp16=$0c

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
  ; enable audio (not DPCM though)
+ lda #$0f
  sta $4015
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
  jsr dispatchTick
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

; a: factor 1
; y: factor 2
; ya: product
mul8:
  lsr
  sta mulTemp0
  tya
  beq mul8Zero

  dey
  sty mulTemp1
  lda #0

  ; iteration 0
  bcc +
  adc mulTemp1
+ ror

  ; iterations 1-7
.REPEAT 7
  ror mulTemp0
  bcc +
  adc mulTemp1
+ ror
.ENDR

  ; end
  tay
  lda mulTemp0
  ror
mul8Zero:
  rts

; command stream player definition
FCS_MAX_CHAN=8

fcsAddrBase=$30
fcsZeroPage=$0e
fcsGlobalStack=$200
fcsPtr=cmdStream
fcsVolMax=volMaxArray

; dispatch state definition
dcStateBase=$410

dcBaseFreq=dcStateBase ; short
dcPitch=dcStateBase+(2*FCS_MAX_CHAN) ; short
dcFreqChanged=dcStateBase+(4*FCS_MAX_CHAN) ; char

exampleNoteTableLow:
  .db <$d5c, <$c9c, <$be8, <$b3c, <$a9a, <$a02, <$972, <$8ea, <$86a, <$7f2, <$780, <$714, <$6ae

exampleNoteTableHigh:
  .db >$d5c, >$c9c, >$be8, >$b3c, >$a9a, >$a02, >$972, >$8ea, >$86a, >$7f2, >$780, >$714, >$6ae

exampleNoteDelta:
  .db 192, 180, 172, 162, 152, 144, 136, 128, 120, 114, 108, 102

; >>2
noteSubTable:
  .db 0, 12, 24, 36, 48, 60, 72, 84, 96, 108, 120, 132, 144, 156, 168, 180

; >>2
octaveTable:
  .db 0, 0, 0
  .db 1, 1, 1
  .db 2, 2, 2
  .db 3, 3, 3
  .db 4, 4, 4
  .db 5, 5, 5
  .db 6, 6, 6
  .db 7, 7, 7
  .db 8, 8, 8
  .db 9, 9, 9
  .db 10, 10, 10
  .db 11, 11, 11
  .db 12, 12, 12
  .db 13, 13, 13
  .db 14, 14, 14
  .db 15, 15, 15

; command call table
fcsCmdTableLow=cmdTableLow
fcsCmdTableHigh=cmdTableHigh

; calculate note_and octave from single-byte note_
; a: note_from 0-180
; set a to note_, and y to octave
calcNoteOctave:
  ; push a for later use
  pha
  ; divide by 4 for indexing in the octave table
  lsr
  lsr
  tay
  ; get the octave and put it into y
  lda octaveTable,y
  tay
  ; pull a (contains note_from 0-180)
  pla
  ; subtract it with the note_sub table to clamp note_to 0-12
  sec
  sbc noteSubTable,y
  ; end
  rts

dispatchNoteOn:
  lda fcsArg0
  sta dcBaseFreq+1,x
  lda #0
  sta dcBaseFreq,x
  lda #1
  sta dcFreqChanged,x
  rts

dispatchNoteOff:
dispatchPrePorta:
  rts

dispatchPorta:
  ; check whether we already hit target
  lda dcBaseFreq+1,x
  cmp fcsArg1
  bne +
  lda dcBaseFreq,x
  bne +
  rts
  ; we didn't - perform porta
  ; 1. calculate porta speed
+ lda fcsArg0
  sta temp16
  lda #0
  sta temp16+1

  ; <<2 to get 4 (default in Furnace)
  clc
  rol temp16
  rol temp16+1
  clc
  rol temp16
  rol temp16+1

  ; 2. get direction of porta (cs = down, cc = up)
  lda dcBaseFreq+1,x
  cmp fcsArg1
  bcc portaUp
  ; 3. perform porta depending on direction
  portaDown:
    lda dcBaseFreq,x
    sec
    sbc temp16
    sta dcBaseFreq,x
    lda dcBaseFreq+1,x
    sbc temp16+1
    sta dcBaseFreq+1,x

    ; check whether we hit target
    bcs + ; carry set - no underflow
    cmp fcsArg1
    bcs + ; greater or equal to target
    ; we did - let's clamp
    lda #0
    sta dcBaseFreq,x
    lda fcsArg1
    sta dcBaseFreq+1,x
    ; end
+   lda #1
    sta dcFreqChanged,x
    rts
  portaUp:
    lda dcBaseFreq,x
    clc
    adc temp16
    sta dcBaseFreq,x
    lda dcBaseFreq+1,x
    adc temp16+1
    sta dcBaseFreq+1,x

    ; check whether we hit target
    bcc + ; carry clear - no overflow
    cmp fcsArg1
    bcc + ; less than target
    ; we did - let's clamp
    lda #0
    sta dcBaseFreq,x
    lda fcsArg1
    sta dcBaseFreq+1,x
    ; end
+   lda #1
    sta dcFreqChanged,x
    rts

dispatchLegato:
  lda fcsArg0
  sta dcBaseFreq+1,x
  lda #0
  sta dcBaseFreq,x
  lda #1
  sta dcFreqChanged,x
  rts

dispatchPitch:
  lda fcsArg0
  clc
  rol
  sta dcPitch,x
  lda fcsArg0+1
  rol
  sta dcPitch+1,x
  lda #1
  sta dcFreqChanged,x
  rts

cmdTableLow:
  .db <dispatchNoteOn
  .db <dispatchNoteOff
  .db 0 ; note_release
  .db 0 ; env release
  .db 0 ; instrument
  .db 0 ; volume
  .db 0, 0 ; unused
  .db <dispatchPorta
  .db <dispatchPitch
  .db 0 ; panning
  .db <dispatchLegato
  .db <dispatchPrePorta
  .dsb 243, 0

cmdTableHigh:
  .db >dispatchNoteOn
  .db >dispatchNoteOff
  .db 0 ; note_release
  .db 0 ; env release
  .db 0 ; instrument
  .db 0 ; volume
  .db 0, 0 ; unused
  .db >dispatchPorta
  .db >dispatchPitch
  .db 0 ; panning
  .db >dispatchLegato
  .db >dispatchPrePorta
  .dsb 243, 0

handleFreq:
  lda #0
  sta dcFreqChanged,x

  ; frequency handler - LINEAR
  ; calculate effective pitch
  lda dcBaseFreq,x
  clc
  adc dcPitch,x
  sta tempFreq
  lda dcBaseFreq+1,x
  adc dcPitch+1,x
  sta tempFreq+1

  ; a contains note_
  ; check for out of range
  sec
  sbc #60
  bmi bottomPitch
  jsr calcNoteOctave
  sta tempNote
  sty tempOctave
  
  ; load note_from note_table
  tay
  lda exampleNoteTableLow,y
  sta temp16
  lda exampleNoteTableHigh,y
  sta temp16+1

  ; calculate pitch offset
  lda exampleNoteDelta,y
  ldy tempFreq
  jsr mul8
  lda temp16
  sty temp16
  sec
  sbc temp16
  sta temp16
  bcs +
  dec temp16+1

  ; shift by octave
+ ldy tempOctave
  beq +
- lsr temp16+1
  ror temp16
  dey
  bne -

+ lda temp16+1
  and #$f8
  beq +

  bottomPitch:
    lda #$ff
    sta temp16
    lda #$07
    sta temp16+1

  ; make a sound
+ txa
  clc
  ror
  cmp curChan
  bne +
  lda #$3f
  sta $4000
  lda #$08
  sta $4001
  lda temp16
  sta $4002
  lda temp16+1
  ora #$08
  sta $4003
+ rts

dispatchTick:
  ldx #0
- lda dcFreqChanged,x
  beq +
  jsr handleFreq
+ inx
  inx
  cpx #(FCS_MAX_CHAN*2)
  bne -
  rts

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
