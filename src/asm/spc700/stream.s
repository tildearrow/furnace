; Furnace Command Stream player for SPC700 architecture
; written by tildearrow

; this is a port of the 6502 player.

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
chanTremoloPos=fcsAddrBase+24+(FCS_MAX_CHAN*18) ; char
chanTremolo=fcsAddrBase+24+(FCS_MAX_CHAN*18)+1 ; char
chanPanbrelloPos=fcsAddrBase+24+(FCS_MAX_CHAN*20) ; char
chanPanbrello=fcsAddrBase+24+(FCS_MAX_CHAN*20)+1 ; char

; may be used for driver detection
fcsDriverInfo:
  .db "Furnace"
  .db 0

; x: channel*2
; a is set to next byte
.MACRO fcsReadNext
  ; a=chanPC[x]
  mov a, [chanPC+x]
  ; increase PC
  ; (seriously? can't use incw here?)
  inc chanPC+x
  bne +
  inc (chanPC+1)+x
+
.ENDM

; same as fcsReadNext, but don't change PC
.MACRO fcsPeekNext
  ; a=chanPC[x]
  mov a, [chanPC+x]
.ENDM

; note on null
fcsNoteOnNull:
  mov a, #0
  mov chanVibratoPos+x, a
  mov y, #$00
  call !fcsDispatchCmd
  ret

; note off, note off env, env release
fcsNoArgDispatchB4:
  mov a, y
  setc
  sbc a, #$b4
  mov y, a
  call !fcsDispatchCmd
  ret

fcsOneByteDispatchB4:
  mov a, y
  push a
  fcsReadNext
  mov fcsArg0, a
  pop a
  setc
  sbc a, #$b4
  mov y, a
  call !fcsDispatchCmd
  ret

; dispatch subroutines for full commands
fcsNoArgDispatch:
  call !fcsDispatchCmd
  ret

fcsOneByteDispatch:
  mov a, y
  push a
  fcsReadNext
  mov fcsArg0, a
  pop a
  mov y, a
  call !fcsDispatchCmd
  ret

fcsTwoByteDispatch:
  mov a, y
  push a
  fcsReadNext
  mov fcsArg0, a
  fcsReadNext
  mov fcsArg1, a
  pop a
  mov y, a
  call !fcsDispatchCmd
  ret

fcsOneShortDispatch:
  mov a, y
  push a
  fcsReadNext
  mov fcsArg0, a
  fcsReadNext
  mov fcsArg0+1, a
  pop a
  mov y, a
  call !fcsDispatchCmd
  ret

fcsTwoShortDispatch:
  mov a, y
  push a
  fcsReadNext
  mov fcsArg0, a
  fcsReadNext
  mov fcsArg0+1, a
  fcsReadNext
  mov fcsArg1, a
  fcsReadNext
  mov fcsArg1+1, a
  pop a
  mov y, a
  call !fcsDispatchCmd
  ret

fcsPrePorta:
  fcsReadNext
  push a
  and a, #$80
  mov fcsArg0, a
  pop a
  and a, #$40
  mov fcsArg1, a
  mov y, #$0c
  call !fcsDispatchCmd
  ret

fcsArpTime:
  fcsReadNext
  mov fcsArpSpeed, a
  ret

fcsVibrato:
  fcsReadNext
  mov chanVibrato+x, a
  mov a, #1
  mov fcsSendPitch, a
  ret

; TODO
fcsVibRange:
fcsVibShape:
  fcsReadNext
  ret

fcsPitch:
  fcsReadNext
  mov chanPitch+x, a
  mov a, #1
  mov fcsSendPitch, a
  ret

fcsArpeggio:
  fcsReadNext
  mov chanArp+x, a
  ret

fcsVolume:
  fcsReadNext
  mov (chanVol+1)+x, a
  mov a, #0
  mov chanVol+x, a
  mov a, #1
  mov fcsSendVolume, a
  ret

fcsVolSlide:
  fcsReadNext
  mov chanVolSpeed+x, a
  fcsReadNext
  mov (chanVolSpeed+1)+x, a
  ret

fcsPorta:
  fcsReadNext
  mov chanPortaTarget+x, a
  fcsReadNext
  mov chanPortaSpeed+x, a
  ret

fcsLegato:
  fcsReadNext
  mov chanNote+x, a
  mov fcsArg0, a
  mov y, #11
  call !fcsDispatchCmd
  ret

fcsVolSlideTarget:
  fcsReadNext
  mov chanVolSpeed+x, a
  fcsReadNext
  mov chanVolSpeed+1+x, a
  ; TODO: we don't support this yet...
  fcsReadNext
  fcsReadNext
  ret

fcsNoOpOneByte:
fcsPanSlide: ; TODO
  fcsReadNext
  ret

fcsTremolo:
  fcsReadNext
  mov chanTremolo+x, a
  mov a, #1
  mov fcsSendVolume, a
  ret

fcsPanbrello:
  fcsReadNext
  mov chanPanbrello+x, a
  ret

fcsPan:
  fcsReadNext
  mov !chanPan+x, a
  mov fcsArg0, a
  fcsReadNext
  mov !(chanPan+1)+x, a
  mov fcsArg1, a
  mov y, #10
  call !fcsDispatchCmd
  ret

fcsOptPlaceholder:
  fcsReadNext
  fcsReadNext
  fcsReadNext
  ret

fcsCallI:
  ; get address and relocate it
  fcsReadNext
  clrc
  adc a, #<fcsPtr
  push a
  fcsReadNext
  adc a, #>fcsPtr
  push a
  ; ignore next two bytes
  call !fcsIgnoreNext
  call !fcsIgnoreNext
  ; fcsPushCall BEGIN
  mov y, chanStackPtr+x
  mov a, chanPC+x
  mov !fcsGlobalStack+y, a
  inc y
  mov a, chanPC+1+x
  mov !fcsGlobalStack+y, a
  inc y
  mov chanStackPtr+x, y
  ; fcsPushCall END
  pop a
  mov chanPC+1+x, a
  pop a
  mov chanPC+x, a
  ret

fcsOffWait:
  mov y, #0
  mov chanTicks+1+x, y
  inc y
  mov chanTicks+x, y
  call !fcsDispatchCmd
  ret

fcsFullCmd:
  ; read command
  fcsReadNext
  mov y, a
  mov a, !(fcsFullCmdTable-28)+y
  mov y, a
  mov a, !fcsCmdReadTableLow+y
  mov !fcsFullCmdJ+1, a
  mov a, !fcsCmdReadTableHigh+y
  mov !fcsFullCmdJ+2, a
  fcsFullCmdJ:
  jmp !$0000 ; modified by previous instructions

fcsSpeedDialCmd:
  mov a, !(fcsSpeedDial-224)+y
  push a
  mov y, a
  mov a, !(fcsFullCmdTable-28)+y
  mov y, a
  mov a, !fcsCmdReadTableLow+y
  mov !fcsSpeedDialCmdJ+1, a
  mov a, !fcsCmdReadTableHigh+y
  mov !fcsSpeedDialCmdJ+2, a
  pop a
  mov y, a
  fcsSpeedDialCmdJ:
  jmp !$0000 ; modified by previous instructions

fcsPresetVol:
  mov a, !(fcsSpeedDial-224)+y
  mov chanVol+1+x, a
  mov a, #0
  mov chanVol+x, a
  mov a, #1
  mov fcsSendVolume, a
  ret

fcsPresetIns:
  mov a, !(fcsSpeedDial-224)+y
  mov fcsArg0, a
  mov y, #4
  call !fcsDispatchCmd
  ret

fcsCall:
  ; get address and relocate it
  fcsReadNext
  clrc
  adc a, #<fcsPtr
  push a
  fcsReadNext
  adc a, #>fcsPtr
  push a
  ; fcsPushCall BEGIN
  ; push channel PC to stack
  mov y, chanStackPtr+x
  mov a, chanPC+x
  mov !fcsGlobalStack+y, a
  inc y
  mov a, chanPC+1+x
  mov !fcsGlobalStack+y, a
  inc y
  mov chanStackPtr+x, y
  ; fcsPushCall END
  pop a
  mov chanPC+1+x, a
  pop a
  mov chanPC+x, a
  ret

; retrieve channel PC from stack
fcsRet:
  mov y, chanStackPtr+x
  dec y
  mov a, !fcsGlobalStack+y
  mov chanPC+1+x, a
  dec y
  mov a, !fcsGlobalStack+y
  mov chanPC+x, a
  mov chanStackPtr+x, y
  ret

fcsJump:
  ; get address and relocate it
  fcsReadNext
  clrc
  adc a, #<fcsPtr
  push a
  fcsPeekNext
  adc a, #>fcsPtr
  mov chanPC+1+x, a
  pop a
  mov chanPC+x, a
  ret

; TODO
fcsTickRate:
  ret

fcsWaitS:
  setc
  fcsReadNext
  and a, #$ff
  mov chanTicks+x, a
  bne +
  clrc
+ fcsReadNext
  adc a, #$ff
  mov chanTicks+1+x, a
  ret

fcsWaitC:
  fcsReadNext
  mov chanTicks+x, a
  mov a, #0
  mov chanTicks+1+x, a
  ret

fcsWait1:
  mov y, #1
  mov chanTicks+x, y
  dec y
  mov chanTicks+1+x, y
  ret

fcsStop:
  mov a, #0
  mov chanPC+x, a
  mov chanPC+1+x, a
  ret

fcsNoOp:
  ret

; x: channel*2
; y: command
fcsDispatchCmd:
  ; read command call table
  ; check for zero
  ; hack: we can check just the high byte. zero page is already used for playback state
  mov a, !fcsCmdTableHigh+y
  beq + ; get out
  ; store the pointer
  mov !fcsDispatchCmdJ+2, a
  mov a, !fcsCmdTableLow+y
  mov !fcsDispatchCmdJ+1, a
  ; handle command in dispatch code
  fcsDispatchCmdJ:
  jmp !$0000 ; modified by previous instructions
  ; only if pointer is zero
+ ret

; x: channel*2
fcsIgnoreNext:
  ; increase PC
  inc chanPC+x
  bne +
  inc chanPC+1+x
+ ret

; x: channel*2 (for speed... char variables are interleaved)
; read commands
fcsChannelCmd:
  ; read next byte
  fcsReadNext
  and a, #$ff ; touch flags

  ; process and read arguments
  ; if (a<0xb3)
  bpl fcsNote ; handle $00-$7f
  cmp a, #$b4
  bpl fcsCheckOther

  ; this is a note
  fcsNote:
    nop
    mov fcsArg0, a
    mov chanNote+x, a
    mov a, #0
    mov y, a
    mov chanVibratoPos+x, a
    ; call DIV_CMD_NOTE_ON
    call !fcsDispatchCmd
    ret

  ; check other instructions
  fcsCheckOther:
    ; check for preset delays
    cmp a, #$f0
    bmi fcsOther

  ; handler for preset delays
  fcsPresetDelay:
    ; load preset delay and store it
    mov y, a
    mov a, !(fcsPtr+8-240)+y
    mov chanTicks+x, a
    mov a, #0
    mov chanTicks+1+x, a
    ret

  ; other instructions
  fcsOther:
    ; call respective handler
    mov y, a
    mov a, !(fcsInsTableLow-180)+y
    mov !fcsOtherJ+1, a
    mov a, !(fcsInsTableHigh-180)+y
    mov !fcsOtherJ+2, a
    fcsOtherJ:
    jmp !$0000 ; modified by previous instructions

; this is called when vibrato depth is zero
fcsChanPitchShortcut:
  ; extend pitch sign
  mov a, chanPitch+x
  mov fcsArg0, a
  bmi +

  ; positive
  mov a, #0
  mov fcsArg0+1, a
  ; dispatch command
  mov y, #9
  call !fcsDispatchCmd
  ; end
  jmp !fcsChanDoPorta

  ; negative
+ mov a, #$ff
  mov fcsArg0+1, a
  ; dispatch command
  mov y, #9
  call !fcsDispatchCmd
  ; end
  jmp !fcsChanDoPorta

; x: channel*2
; stuff that goes after command reading
fcsChannelPost:
  ;;; DO VOLUME
  fcsChanDoVolume:
    ; if (sendVolume || chanVolSpeed[x]!=0)
    mov a, fcsSendVolume
    bne +
    mov a, chanVolSpeed+x
    or a, chanVolSpeed+1+x
    beq fcsChanDoPitch
    ; increase volume
+   mov a, chanVol+x
    clrc
    adc a, chanVolSpeed+x
    mov chanVol+x, a
    mov a, chanVol+1+x
    adc a, chanVolSpeed+1+x
    mov chanVol+1+x, a
    ; TODO: handle vol slide with target
    ; get sign of volume speed
    mov a, chanVolSpeed+1+x
    bpl fcsChanPlus
    fcsChanMinus:
      ; if (chanVol[x]<0)
      bcs fcsChanSubmitVol
      ; chanVol[x]=0
      mov a, #0
      mov chanVol+x, a
      mov chanVol+1+x, a
      beq fcsChanSubmitVol ; shortcut
    fcsChanPlus:
      ; if (chanVol[x]>=fcsVolMax[x] || CARRY)
      bcs + ; overflow check
      mov a, chanVol+1+x ; comparison check
      cmp a, !fcsVolMax+x
      bmi fcsChanSubmitVol
      ; chanVol[x]=fcsVolMax[x]
+     mov a, !fcsVolMax+x
      mov chanVol+1+x, a
      mov a, #0
      mov chanVol+x, a
    fcsChanSubmitVol:
      mov a, chanVol+1+x
      mov fcsArg0, a
      mov y, #$05 ; volume
      call !fcsDispatchCmd

  ;;; DO PITCH
  fcsChanDoPitch:
    ; check for vibrato depth
    mov a, chanVibrato+x
    and a, #$0f ; depth only
    bne +
    jmp !fcsChanDoPitch1

    ; update vibrato
    ; TODO: we can use multiplication here.
    ; 1. store vibrato pitch table offset
+   mov a, chanVibrato+x
    xcn a
    and a, #$f0
    mov fcsTempPtr, a
    ; 2. update vibrato position
    mov a, chanVibrato+x
    lsr a
    lsr a
    lsr a
    lsr a
    clrc
    adc a, chanVibratoPos+x
    and a, #$3f
    mov chanVibratoPos+x, a
    ; 3. calculate vibrato pitch
    ;   - we use 15 quarter sine tables, one for each vibrato depth
    ;   - 32-63 are negatives of 0-31
    ;   - a&31: zero is zero. otherwise a-1 in the table unless a&16
    ;   - if a&16 then invert a
    mov y, a
    ; check for zero in a&31
    and a, #$1f
    bne +
    ; it is. load zero
    mov a, #0
    jmp !fcsPostVibratoCalc1
    ; it is not. check a&16
+   and a, #$10
    bne +
    ; 0-15
    dec y
    mov a, y
    and a, #$0f
    jmp !fcsPostVibratoCalc
    ; 16-31
+   mov a, y
    and a, #$0f
    eor a, #$0f ; 0-15 -> 15-0

    fcsPostVibratoCalc:
      ; check for 32-63
      push a
      mov a, y
      and a, #$20
      bne +
      ; 0-31
      pop a
      clrc
      adc a, fcsTempPtr
      mov y, a
      mov a, !(fcsVibTable-16)+y
      jmp !fcsPostVibratoCalc1
      ; 32-63 (negate)
+     pop a
      clrc
      adc a, fcsTempPtr
      mov y, a
      mov a, !(fcsVibTable-16)+y
      eor a, #$ff
      clrc
      adc a, #1

    ; at this point, a contains the vibrato pitch
    fcsPostVibratoCalc1:
      mov fcsArg0, a
      ; extend sign
      bmi +
      mov a, #0
      mov fcsArg0+1, a
      beq ++
+     mov a, #$ff
      mov fcsArg0+1, a
      ; extend pitch sign
++    mov a, chanPitch+x
      mov fcsTempPtr, a
      bmi +
      mov a, #0
      mov fcsTempPtr+1, a
      beq ++
+     mov a, #$ff
      mov fcsTempPtr+1, a
      ; add pitch
++    mov a, fcsArg0
      clrc
      adc a, fcsTempPtr
      mov fcsArg0, a
      mov a, fcsArg0+1
      adc a, fcsTempPtr+1
      mov fcsArg0+1, a
      ; dispatch command
      mov y, #9
      call !fcsDispatchCmd
      jmp !fcsChanDoPorta

  ; check whether we should send pitch
  ; (vibrato depth is zero)
  fcsChanDoPitch1:
    mov a, fcsSendPitch
    beq fcsChanDoPorta
    jmp !fcsChanPitchShortcut

  ;;; DO PORTAMENTO
  fcsChanDoPorta:
    ; if (chanPortaSpeed[x])
    mov a, chanPortaSpeed+x
    beq fcsChanDoArp
    ; do portamento
    mov fcsArg0, a
    mov a, #0
    mov fcsArg0+1, a
    mov a, chanPortaTarget+x
    mov fcsArg1, a
    mov y, #8 ; NOTE_PORTA
    call !fcsDispatchCmd
    ; get out (we can't do porta and arp simultaneously)
    ret

  ;;; DO ARPEGGIO
  fcsChanDoArp:
    ; if (chanArp[x] && !chanPortaSpeed[x])
    mov a, chanArp+x
    beq fcsChanEnd
    ; if (chanArpTicks[x]==0)
    mov a, chanArpTicks+x
    bne ++
    ; switch (chanArpStage[x])
    mov a, chanArpStage+x
    cmp a, #1 ; mi is 0, eq is 1 and pl is 2
    beq fcsChanArp1
    bpl fcsChanArp2
    ; bmi fcsChanArp0
    fcsChanArp0:
      mov a, chanNote+x 
      jmp !fcsChanArpPost
    fcsChanArp1:
      mov a, chanArp+x
      lsr a
      lsr a
      lsr a
      lsr a
      clrc
      adc a, chanNote+x
      jmp !fcsChanArpPost
    fcsChanArp2:
      mov a, chanArp+x
      and a, #$0f
      clrc
      adc a, chanNote+x
      jmp !fcsChanArpPost

    fcsChanArpPost:
      mov fcsArg0, a
      mov y, #11 ; legato
      call !fcsDispatchCmd

    ; post-operations
    mov a, chanArpStage+x
    clrc
    adc a, #1
    cmp a, #3
    ; if (chanArpStage[x]>=3) chanArpStage[x]=0
    bmi +
    mov a, #0
+   mov chanArpStage+x, a

    ; chanArpTicks[x]=fcsArpSpeed
    mov a, fcsArpSpeed
    mov chanArpTicks+x, a

    ; chanArpTicks[x]--
++  dec chanArpTicks+x

  ;;; END
  fcsChanEnd:
    ret

; x: channel*2
fcsDoChannel:
  ; initialize
  mov a, #0
  mov fcsSendVolume, a
  mov fcsSendPitch, a

  ; begin processing
  ; chanTicks--
+ mov a, chanTicks+x
  setc
  sbc a, #1
  mov chanTicks+x, a
  bne + ; skip if our counter isn't zero

  ; ticks lower is zero; check upper byte
  mov y, chanTicks+1+x
  beq fcsDoChannelLoop ; go to read commands if it's zero as well
  ; decrease ticks upper
  dec y
  mov chanTicks+1+x, y
  ; process channel stuff
  call !fcsChannelPost
  ret

  ; ticks is zero... read commands until chanTicks is set
  fcsDoChannelLoop:
    mov a, chanTicks+x
    or a, chanTicks+1+x
    bne + ; get out if chanTicks is no longer zero
    call !fcsChannelCmd ; read next command
    jmp !fcsDoChannelLoop

+ call !fcsChannelPost
  ; end
  ret

fcsTick:
  ; update channel state
  ; for (x=0; x<FCS_MAX_CHAN; x++)
  mov x, #0
- call !fcsDoChannel
  inc x
  inc x
  cmp x, #FCS_MAX_CHAN*2
  bne -

  ; increase tick counter
  inc fcsTicks
  bne +
  inc fcsTicks+1

  ; end
+ ret

fcsInit:
  ; set all tick counters to 1
  mov a, #1
  mov y, #0
  mov x, #(FCS_MAX_CHAN*2)
- dec x
  mov chanTicks+x, y
  dec x
  mov chanTicks+x, a
  bne -

  ; set channel program counters
  mov x, #(FCS_MAX_CHAN*2)
- dec x
  mov a, !(fcsPtr+40)+x
  mov chanPC+x, a
  cmp x, #0
  bne -

  ; relocate program counters
  mov x, #0
- clrc
  mov a, chanPC+x
  adc a, #<fcsPtr
  mov chanPC+x, a
  inc x
  mov a, chanPC+x
  adc a, #>fcsPtr
  mov chanPC+x, a
  inc x
  cmp x, #(FCS_MAX_CHAN*2)
  bne -

  ; initialize channel stacks
  mov a, #0
  mov x, #0
  mov y, #0
- mov chanStackPtr+x, a
  clrc
  adc a, !(fcsPtr+40+(FCS_MAX_CHAN*2))+y
  clrc
  adc a, !(fcsPtr+40+(FCS_MAX_CHAN*2))+y
  inc x
  inc x
  inc y
  cmp x, #(FCS_MAX_CHAN*2)
  bne -

  ; set volumes
  mov x, #0
- mov a, !fcsVolMax+x
  mov chanVol+1+x, a
  inc x
  inc x
  cmp x, #(FCS_MAX_CHAN*2)
  bne -

  ; set arp speed
  mov a, #1
  mov fcsArpSpeed, a

  ; success
  mov a, #0
  ret

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
; $cc fcsTremolo,
; $cd fcsPanbrello,
; $ce fcsPanSlide,
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
; $e0-$e5 fcsPresetIns,
; $e6-$eb fcsPresetVol,
; $ec-$ef fcsSpeedDialCmd,
fcsInsTableHigh:
  .db >fcsNoArgDispatchB4, >fcsNoArgDispatchB4, >fcsNoArgDispatchB4, >fcsNoArgDispatchB4, >fcsOneByteDispatchB4, >fcsNoOp, >fcsNoOp, >fcsNoOp, >fcsNoOp, >fcsNoOp, >fcsNoOp, >fcsNoOp
  .db >fcsPrePorta, >fcsArpTime, >fcsVibrato, >fcsVibRange, >fcsVibShape, >fcsPitch, >fcsArpeggio, >fcsVolume, >fcsVolSlide, >fcsPorta, >fcsLegato, >fcsVolSlideTarget, >fcsTremolo, >fcsPanbrello, >fcsPanSlide, >fcsPan
  .db >fcsOptPlaceholder, >fcsNoOp, >fcsNoOp, >fcsNoOp, >fcsNoOp, >fcsCallI, >fcsOffWait, >fcsFullCmd, >fcsCall, >fcsRet, >fcsJump, >fcsTickRate, >fcsWaitS, >fcsWaitC, >fcsWait1, >fcsStop
  .db >fcsPresetIns, >fcsPresetIns, >fcsPresetIns, >fcsPresetIns, >fcsPresetIns, >fcsPresetIns,
  .db >fcsPresetVol, >fcsPresetVol, >fcsPresetVol, >fcsPresetVol, >fcsPresetVol, >fcsPresetVol,
  .db >fcsSpeedDialCmd, >fcsSpeedDialCmd, >fcsSpeedDialCmd, >fcsSpeedDialCmd

fcsInsTableLow:
  .db <fcsNoArgDispatchB4, <fcsNoArgDispatchB4, <fcsNoArgDispatchB4, <fcsNoArgDispatchB4, <fcsOneByteDispatchB4, <fcsNoOp, <fcsNoOp, <fcsNoOp, <fcsNoOp, <fcsNoOp, <fcsNoOp, <fcsNoOp
  .db <fcsPrePorta, <fcsArpTime, <fcsVibrato, <fcsVibRange, <fcsVibShape, <fcsPitch, <fcsArpeggio, <fcsVolume, <fcsVolSlide, <fcsPorta, <fcsLegato, <fcsVolSlideTarget, <fcsTremolo, <fcsPanbrello, <fcsPanSlide, <fcsPan
  .db <fcsOptPlaceholder, <fcsNoOp, <fcsNoOp, <fcsNoOp, <fcsNoOp, <fcsCallI, <fcsOffWait, <fcsFullCmd, <fcsCall, <fcsRet, <fcsJump, <fcsTickRate, <fcsWaitS, <fcsWaitC, <fcsWait1, <fcsStop
  .db <fcsPresetIns, <fcsPresetIns, <fcsPresetIns, <fcsPresetIns, <fcsPresetIns, <fcsPresetIns,
  .db <fcsPresetVol, <fcsPresetVol, <fcsPresetVol, <fcsPresetVol, <fcsPresetVol, <fcsPresetVol,
  .db <fcsSpeedDialCmd, <fcsSpeedDialCmd, <fcsSpeedDialCmd, <fcsSpeedDialCmd

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
  .db 2
  .db 2
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
