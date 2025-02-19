; Furnace validation player code
; this is NOT the ROM export you're looking for!

VPOSR = $dff004
VHPOSR = $dff006
COLOR00 = $dff180

chipBase=$dff000

DMACONR = $02
POTGOR = $16
DMACON = $96
ADKCON = $9e
AUDBASE = $a0
AUD0LCH = $a0
AUD0LCL = $a2
AUD0LEN = $a4
AUD0PER = $a6
AUD0VOL = $a8
AUD0DAT = $aa
AUD1VOL = $b8
AUD2VOL = $c8
AUD3VOL = $d8

execBase = $4
exec_AllocMem = -198
exec_FreeMem = -210
exec_AddIntServer = -168
exec_RemIntServer = -174
exec_Wait = -318
exec_TypeOfMem = -534

SIGBREAKF_CTRL_C = $1000

code_c

;;; PROGRAM CODE

init:
  ; set up sample data if necessary
  lea sampleDataPtr(pc),a0
  lea sampleData(pc),a1
  move.l a1,(a0)
  lea state(pc),a0
  move.b #0,2(a0)

  ; TypeOfMem(sampleData)
  lea sampleData(pc),a1
  move.l execBase,a6
  jsr (exec_TypeOfMem,a6)
  btst #1,d0
  bne init0

  ; allocate sample data in chip memory and copy to it
  lea state(pc),a0
  move.b #1,2(a0)
  ; AllocMem(sizeof(sampleData),MEMF_PUBLIC|MEMF_CHIP)
  move.l #wavetable,d0 ; sizeof(sampleData)
  subi.l #sampleData,d0
  move.l #$3,d1
  move.l execBase,a6
  jsr (exec_AllocMem,a6)
  ; check success
  bne initC
  ; failed
  move.l #1,d0
  rts
initC:
  lea sampleDataPtr(pc),a0
  move.l d0,(a0)

  ; copy memory
  move.l d0,a1
  lea sampleData(pc),a0
  move.l #wavetable,d2
  subi.l #sampleData,d2

copySampleLoop:
  move.w (a0)+,(a1)+
  subq.l #2,d2
  bne copySampleLoop

init0:
  ; allocate interrupt
  ; AllocMem(sizeof(struct Interrupt),MEMF_PUBLIC|MEMF_CLEAR)
  move.l #24,d0 ; sizeof(struct Interrupt)
  move.l #$10001,d1
  move.l execBase,a6
  jsr (exec_AllocMem,a6)
  ; check success
  bne init1
  ; failed
  bsr freeSampleData
  move.l #1,d0
  rts
init1:
  ; store
  lea state(pc),a0
  move.l d0,4(a0)
  
  ; set up interrupt
  move.l d0,a0
  ; struct Node
  move.b #2,8(a0)      ; ln_Type
  move.b #-60,9(a0)    ; ln_Pri
  lea nodeName(pc),a1  ; ln_Name
  move.l a1,10(a0)
  lea state(pc),a1     ; is_Data
  add.l #8,a1
  move.l a1,14(a0)
  lea nextTick(pc),a1  ; is_Code (!!!)
  move.l a1,18(a0)

  ; start audio
  move.b #2,$bfe001
  lea chipBase,a0
  move.w #15,DMACON(a0)
waitCon:
  move.w DMACONR(a0),d0
  andi.w #15,d0
  bne waitCon

  move.w #$8200,DMACON(a0)
  move.w #$40,AUD0VOL(a0)
  move.w #$39,AUD1VOL(a0)
  move.w #$40,AUD2VOL(a0)
  move.w #$40,AUD3VOL(a0)
  lea seqAddr(pc),a0
  lea sequence(pc),a1
  move.l a1,(a0)
main:
  ; setup interrupt
  ; AddIntServer(INTB_VERTB,state+4)
  move.l #5,d0
  lea state(pc),a2
  add.l #4,a2
  move.l (a2),a1
  move.l execBase,a6
  jsr (exec_AddIntServer,a6)

main1:
  ; wait until break
  bsr waitVBlank

finish:
  ; kill interrupt
  ; RemIntServer(INTB_VERTB,state+4)
  move.l #5,d0
  lea state(pc),a2
  add.l #4,a2
  move.l (a2),a1
  move.l execBase,a6
  jsr (exec_RemIntServer,a6)

  lea chipBase,a0
  move.w #15,DMACON(a0)

  ; free interrupt data
  ; FreeMem(state+4,sizeof(struct Interrupt))
  lea state(pc),a2
  add.l #4,a2
  move.l (a2),a1
  move.l #24,d0 ; sizeof(struct Interrupt)
  move.l execBase,a6
  jsr (exec_FreeMem,a6)

  bsr freeSampleData
  clr.l d0
  rts

freeSampleData:
  lea state(pc),a0
  move.b 2(a0),d0
  tst.b d0
  beq noFreeNeeded

  move.l sampleDataPtr(pc),a1
  move.l #wavetable,d0 ; sizeof(sampleData)
  subi.l #sampleData,d0
  move.l execBase,a6
  jsr (exec_FreeMem,a6)

noFreeNeeded:
  rts

;;; PLAYER CODE

waitVBlank:
  ; Wait(SIGBREAKF_CTRL_C)
  move.l #SIGBREAKF_CTRL_C,d0
  move.l execBase,a6
  jsr (exec_Wait,a6)
  rts

nextTick:
  ; save registers
  movem.l d0-d7/a0-a7,-(sp)

  move.w #$0ff,d1
  move.w d1,COLOR00

  lea state(pc),a4
  move.w (a4),d0
  subi.w #1,d0
  bmi nextTick0
  move.w d0,(a4)
  bra endNextTick

nextTick0:
  move.l seqAddr(pc),a2
nextTick1:
  ; get next command
  clr.w d0
  move.b (a2)+,d0
  move.w #$0ff,d4
  move.w d4,COLOR00

testSpecial:
  cmp.w #$f0,d0
  bmi testChannel

testF1:
  ; f1 - next tick
  cmp.b #$f1,d0
  bne testF2
  ; end of tick
  move.w #0,(a4)
  bra endTick
testF2:
  ; f2 - wait (char)
  cmp.b #$f2,d0
  bne testF3
  move.b (a2)+,d0
  andi.w #$ff,d0
  move.w d0,(a4)
  bra endTick
testF3:
  ; f3 - wait (short)
  cmp.b #$f3,d0
  bne testF6
  clr.w d2
  move.b (a2)+,d2
  lsl.w #8,d2
  or.b (a2)+,d2
  move.w d2,(a4)
  bra endTick
testF6:
  ; f6 - write DMACON
  cmp.b #$f6,d0
  bne testFE
  move.w #$f00,d4
  move.w d4,COLOR00
  clr.w d2
  move.b (a2)+,d2
  lsl.w #8,d2
  or.b (a2)+,d2
  move.w d2,chipBase+DMACON
  ; wait for DMACON to be done
  move.b (VHPOSR),d0
dmaConWait:
  cmp.b (VHPOSR),d0
  beq dmaConWait
  ; wait for DMACON to be done -2
  move.b (VHPOSR),d0
dmaConWait1:
  cmp.b (VHPOSR),d0
  beq dmaConWait1
  bra nextTick1
testFE:
  ; fe - write ADKCON
  cmp.b #$fe,d0
  bne testFF
  clr.w d2
  move.b (a2)+,d2
  lsl.w #8,d2
  or.b (a2)+,d2
  move.w d2,chipBase+ADKCON
  bra nextTick1
testFF:
  ; ff - end of song
  cmp.b #$ff,d0
  bne testOther
theEnd:
  lea sequence(pc),a2
testOther:
  ; something else
  bra nextTick1

testChannel:
  cmp.b #$40,d0
  bge invalidCmd
  ; process channel
  move.b d0,d1
  andi.b #15,d0
  ; check for 0
  bne chanNotZero
sampleWrite:
  ; write loc/len
  move.w #$f0f,d4
  move.w d4,COLOR00
  clr.l d2
  move.b (a2)+,d2
  lsl.l #8,d2
  or.b (a2)+,d2
  lsl.l #8,d2
  or.b (a2)+,d2
  move.l sampleDataPtr(pc),a0
  add.l a0,d2
  lea chipBase,a0
  move.b d1,d0
  andi.l #$ff,d0
  addi.l #$a0,d0
  adda.l d0,a0
  move.l d2,(a0)+ ; location
  clr.w d2
  move.b (a2)+,d2
  lsl.w #8,d2
  or.b (a2)+,d2
  move.w d2,(a0) ; length
  bra nextTick1
chanNotZero:
  ; check for 8 (VOL)
  cmp.b #8,d0
  bne chanSampleBook
  ; write volume
  clr.w d2
  move.b (a2)+,d2
  bra chanWrite
chanSampleBook:
  ; check for 2 (loc/len from book)
  cmp.b #2,d0
  bne chanWaveChange

  move.w #$f0f,d4
  move.w d4,COLOR00
  clr.l d3
  move.b (a2)+,d3
  bsr getSampleBook

  ; write loc/len
  move.l sampleDataPtr(pc),a0
  add.l a0,d2
  lea chipBase,a0
  move.b d1,d0
  andi.l #$f0,d0
  addi.l #$a0,d0
  adda.l d0,a0
  move.l d2,(a0)+ ; location
  move.w d3,(a0) ; length

  bra nextTick1
chanWaveChange:
  ; check for 1 (wave change)
  cmp.b #1,d0
  bne chanWaveBookB
  ; copy wave
  clr.l d2
  move.b (a2)+,d2
  lsl.l #8,d2
  or.b (a2)+,d2
  lsl.l #8,d2
  or.b (a2)+,d2
  add.l #wavetable,d2
  move.l d2,a0

  move.l sampleDataPtr(pc),a1
  andi.l #$30,d1
  lsl.l #4,d1
  adda.l d1,a1

  clr.l d2
  move.b (a2)+,d2
  lsl.l #8,d2
  or.b (a2)+,d2

  bsr copyWave
  bra nextTick1
chanWaveBookB:
  ; check for 3 (wave change from book)
  cmp.b #3,d0
  bne chanWaveBookW

  clr.l d3
  move.b (a2)+,d3
  bsr getWaveBook

  move.l sampleDataPtr(pc),a1
  andi.l #$30,d1
  lsl.l #4,d1
  adda.l d1,a1

  bsr copyWave
  bra nextTick1
chanWaveBookW:
  ; check for 4 (wave change from book short)
  cmp.b #4,d0
  bne chanOther

  clr.l d3
  move.b (a2)+,d3
  lsl.l #8,d3
  or.b (a2)+,d3
  bsr getWaveBook

  move.l sampleDataPtr(pc),a1
  andi.l #$30,d1
  lsl.l #4,d1
  adda.l d1,a1

  bsr copyWave
  bra nextTick1
chanOther:
  ; get value and write
  clr.w d2
  move.b (a2)+,d2
  lsl.w #8,d2
  or.b (a2)+,d2
chanWrite:
  move.w #$ff0,d4
  move.w d4,COLOR00
  lea chipBase,a0
  or.b d1,d0
  addi.b #AUDBASE,d0
  andi.l #$ff,d0
  adda.l d0,a0
  move.w d2,(a0)
invalidCmd:
  bra nextTick1

endTick:
  lea seqAddr(pc),a3
  move.l a2,(a3)
  bra endNextTick

endNextTick:
  move.w #$222,d4
  move.w d4,COLOR00

  movem.l (sp)+,d0-d7/a0-a7
  move.l #chipBase,a0
  moveq.l #0,d0
  rts

; a0: source. a1: destination. d2: length.
copyWave:
  ; don't copy a zero-length wave
  tst.l d2
  beq noCopy
copyWaveLoop:
  move.w (a0)+,(a1)+
  subq.l #2,d2
  bne copyWaveLoop
noCopy:
  rts

; put wave book entry in a0/d2. d3: index (modified).
getWaveBook:
  lea waveBook(pc),a3
  lsl.l #2,d3
  move.l (a3,d3),d2
  move.l d2,d3
  rol.l #8,d2
  andi.l #$ff,d2
  bne getWaveBook2
  move.w #$100,d2
getWaveBook2:
  andi.l #$ffffff,d3
  add.l #wavetable,d3
  move.l d3,a0
  rts

; get sample book entry in d2/d3. d3: index.
getSampleBook:
  lea sampleBook(pc),a3
  lsl.l #3,d3
  adda.l d3,a3
  move.l (a3)+,d2
  move.l (a3)+,d3
  andi.l #$ffffff,d2
  andi.l #$ffff,d3
  rts

data_c
  cnop 0,4

nodeName:
  dc.b "amigatest", 0

curColor:
  dc.w 0

state:
  dc.w 0 ; ticks
  dc.w 0 ; had to copy?
  dc.l 0 ; interrupt pointer
  dc.l 0 ; interrupt data
  cnop 0,4

sampleDataPtr:
  dc.l 0
  cnop 0,4

seqAddr:
  dc.l 0
  cnop 0,4

sampleBook:
  incbin "sbook.bin"
  cnop 0,4

waveBook:
  incbin "wbook.bin"
  cnop 0,4

sequence:
  incbin "seq.bin"
  cnop 0,4


sampleData:
  incbin "sample.bin"
  cnop 0,4

wavetable:
  incbin "wave.bin"
