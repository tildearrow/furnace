    processor 6502
    include "vcs.h"
    include "macro.h"

NTSC = 0
PAL60 = 1

    IFNCONST SYSTEM
SYSTEM = NTSC
    ENDIF

; ----------------------------------
; constants

#if SYSTEM = NTSC
; NTSC Colors
WHITE = $0f
BLACK = 0
LOGO_COLOR = $C4
SCANLINES = 262
#else
; PAL Colors
WHITE = $0E
BLACK = 0
LOGO_COLOR = $53
SCANLINES = 262
#endif

NUM_AUDIO_CHANNELS = 2

; ----------------------------------
; variables

  SEG.U variables

    ORG $80

frame              ds 1  ; frame counter

audio_tracker      ds 2  ; current track index
audio_waveform_idx ds 2  ; current channel index
audio_timer        ds 2  ; time left on audio
tmp_waveform_ptr   ds 2  ; current waveform ptr

vis_frequency      ds 2
vis_amplitude      ds 2
vis_waveform       ds 2
vis_height         ds 2
vis_gradient       ds 2
vis_pattern        ds 2

; ----------------------------------
; code

  SEG
    ORG $F000

Reset
CleanStart
    ; do the clean start macro
            CLEAN_START

            ; load track
            lda #PAT_S00_C00_P00
            sta audio_tracker
            lda #PAT_S00_C01_P00
            sta audio_tracker + 1

newFrame

    ; 3 scanlines of vertical sync signal to follow

            ldx #%00000010
            stx VSYNC               ; turn ON VSYNC bit 1
            ldx #0

            sta WSYNC               ; wait a scanline
            sta WSYNC               ; another
            sta WSYNC               ; another = 3 lines total

            stx VSYNC               ; turn OFF VSYNC bit 1

    ; 37 scanlines of vertical blank to follow

;--------------------
; VBlank start

            lda #%00000010
            sta VBLANK

            lda #42    ; vblank timer will land us ~ on scanline 34
            sta TIM64T

            inc frame ; new frame

    ; check reset switches
            lda #$01
            bit SWCHB
            bne _end_switches
            jmp CleanStart
_end_switches

;
; -- audio tracker
;

            ldx #NUM_AUDIO_CHANNELS - 1
audio_loop 
            lda audio_tracker,x
            beq _audio_next_channel
            ldy audio_timer,x
            dey
            beq _audio_next_note
            sty audio_timer,x
            jmp _audio_next_channel
_audio_next_note
            tay
_audio_next_note_ty
            lda AUDIO_TRACKS,y
            tay
            lda WF_TABLE_START,y
            sta tmp_waveform_ptr
            lda WF_TABLE_START+1,y
            sta tmp_waveform_ptr + 1
            ldy audio_waveform_idx,x
            lda (tmp_waveform_ptr),y
            cmp #255
            beq _audio_advance_tracker
            sta AUDC0,x
            sta vis_waveform,x
            iny
            lda (tmp_waveform_ptr),y
            sta AUDF0,x
            sta vis_frequency,x
            iny
            lda (tmp_waveform_ptr),y
            sta AUDV0,x
            sta vis_amplitude,x
            iny
            lda (tmp_waveform_ptr),y
            sta audio_timer,x
            iny
            sty audio_waveform_idx,x
            jmp _audio_next_channel
_audio_advance_tracker ; got a 255 on waveform
            lda #$0
            sta audio_waveform_idx,x
            ldy audio_tracker,x
            iny 
            lda AUDIO_TRACKS,y ; store next track #
            cmp #255
            beq _audio_change_track
            sty audio_tracker,x
            jmp _audio_next_note_ty; if not 255 loop back 
_audio_change_track ; got a 255 on track
            iny
            lda AUDIO_TRACKS,y ; loop
            sta audio_tracker,x
            jmp _audio_next_note ; if not 255 loop back 
_audio_next_channel
            dex
            bpl audio_loop

;---------------------
; vis timing

            ldx #NUM_AUDIO_CHANNELS - 1
vis_calc_loop
            lda vis_amplitude,x
            asl
            asl
            sta vis_height,x
            lda frame
            and #$f0
            cpx #$00
            beq _vis_store_gradient
            clc
            adc #$80
_vis_store_gradient
            sta vis_gradient,x
            dex
            bpl vis_calc_loop

;---------------------
; end vblank

            jsr waitOnVBlank ; SL 34
            sta WSYNC ; SL 35
            lda #1
            sta CTRLPF ; reflect playfield
            lda #LOGO_COLOR
            sta COLUPF

            lda vis_height
            tay
            sec
            sbc #96
            tax
top_loop
            sta WSYNC
            inx
            bmi top_loop

            lda #$00
vis_c0_loop
            sta WSYNC
            sta COLUBK
            tya
            and #$0f
            ora vis_gradient
            dey
            bpl vis_c0_loop

            lda vis_height + 1
            tay
            sec
            sbc #96
            tax

            lda #$00
vis_c1_loop
            sta WSYNC
            sta COLUBK
            tya
            and #$0f
            ora vis_gradient + 1
            dey
            bpl vis_c1_loop

            lda #$00
            sta COLUBK
end_loop
            sta WSYNC
            inx
            bmi end_loop  

;--------------------
; Overscan start

waitOnOverscan
            ldx #30
waitOnOverscan_loop
            sta WSYNC
            dex
            bne waitOnOverscan_loop
            jmp newFrame

;------------------------
; vblank sub

waitOnVBlank
            ldx #$00
waitOnVBlank_loop          
            cpx INTIM
            bmi waitOnVBlank_loop
            stx VBLANK
            rts 
;-----------------------------------------------------------------------------------
; Audio Data 

    ALIGN 256

AUDIO_TRACKS
    byte 0

    #include "R9Data.inc"

;-----------------------------------------------------------------------------------
; the CPU reset vectors

    ORG $FFFA

    .word Reset          ; NMI
    .word Reset          ; RESET
    .word Reset          ; IRQ

    END