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
BLUE = $A0
RED = $30
WHITE = $0f
BLACK = 0
SCANLINES = 262
#else
; PAL Colors
BLUE = $90
RED = $40
WHITE = $0E
BLACK = 0
SCANLINES = 262
#endif

NUM_AUDIO_CHANNELS = 2
COUNTER_BLOCK_HEIGHT = 6
COUNTER_HEIGHT = COUNTER_BLOCK_HEIGHT + 3
GRADIENT_FIELD_HEIGHT = 192 - (COUNTER_HEIGHT * 3)
OVERSCAN_HEIGHT = 30
VERTICAL_BANNER_POS = 130

; ----------------------------------
; variables

  SEG.U variables

    ORG $80

frame               ds 1  ; frame counter

audio_song          ds 1  ; what song are we on
audio_song_ptr      ds 2  ; address of song
audio_song_order    ds 1  ; what order are we at in the song
audio_row_idx       ds 1  ; where are we in the current order
audio_pattern_idx   ds 2  ; which pattern is playing on each channel
audio_waveform_sz   ds 2  ; waveform countdown
audio_waveform_idx  ds 2  ; where are we in waveform on each channel
audio_timer         ds 2  ; time left on next action on each channel

speed               ds 1  ; playback speed
debounce_input      ds 1

tmp_pattern_ptr     ds 2
tmp_waveform_ptr    ds 2
tmp_input           ds 1

vis_freq            ds 2
vis_amp             ds 2
vis_gradient        ds 16

; ----------------------------------
; code

  SEG
    ORG $F000

Reset
CleanStart
    ; do the clean start macro
            CLEAN_START

            ; load track
            jsr sub_start_song
            ; playback speed
            lda #1
            sta speed

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
; -- check input
;

            ; debounce input TODO: janky
            lda SWCHA
            lsr
            lsr
            lsr
            lsr
            sta tmp_input
            lda INPT4
            and #$80
            ora tmp_input
            eor #$8f
            sta tmp_input
            beq _end_input ; all zero
            eor debounce_input
            bpl _skip_trigger_pause
            lda speed
            bne _do_pause
            lda #1
            jmp _save_pause
_do_pause
            lda #0
_save_pause
            sta speed
            jmp _end_input
_skip_trigger_pause
            ror
            bcs _up
            ror
            bcs _down
            ror 
            jmp _end_input
_down
            jsr sub_inc_song
            jmp _end_input            
_up
            jsr sub_dec_song
_end_input
            lda tmp_input
            stx debounce_input

;---------------------
; audio tracker

            ldx speed
            beq audio_tracker_off
            jsr sub_r9_tracker
            jmp audio_end
audio_tracker_off
            stx AUDC0
            stx AUDC1
            stx AUDV0
            stx AUDV1
            stx vis_freq
            stx vis_freq + 1
            stx vis_amp
            stx vis_amp + 1
audio_end

;---------------------
; vis timing

            lda #$f0
            sec
            ldx #15
_vis_gradient_setup_loop
            sta vis_gradient,x
            sbc #$10
            bne _vis_gradient_setup_skip_zero
            lda #$f0
_vis_gradient_setup_skip_zero
            dex
            bpl _vis_gradient_setup_loop

            ldy audio_row_idx
            lda (tmp_pattern_ptr),y
            and #$0f
            tax
            lda #$0f
            sta vis_gradient,x

;---------------------
; end vblank

            jsr waitOnVBlank ; SL 34
            sta WSYNC ; SL 35
            lda #1
            sta CTRLPF ; reflect playfield
            lda #$f0
            sta PF0
            lda #0
            sta COLUBK
            sta COLUPF
            sta COLUP0
            sta COLUP1

            lda #12
vis_loop
            tax
            ldy #31
gradient_loop
            sta WSYNC               ;--
            sty GRP1
            SLEEP 14
            tya
            lsr
            ora vis_gradient,x      ;4  23
            sta COLUBK              ;3  29 
            tya
            lsr
            ora vis_gradient + 1,x  ;4  23
            sta COLUBK              ;3  29 
            tya
            lsr
            ora vis_gradient + 2,x  ;4  23
            sta COLUBK              ;3  29 
            tya
            lsr
            ora vis_gradient + 3,x  ;4  23
            sta COLUBK              ;3  29 
            dey                     ;2  67
            bpl gradient_loop
            sta WSYNC
            lda #0
            sta COLUBK
            txa
            sec
            sbc #4
            bpl vis_loop

            ; -
            lda #0
            sta CTRLPF
            sta PF0
            lda #WHITE
            sta COLUPF
            lda vis_freq
            jsr sub_waveform
            lda vis_freq + 1
            jsr sub_waveform

            lda #13
            ldx #0
            jsr sub_respx
            lda #0
            sta HMP0
            ldx #1
            lda #21
            jsr sub_respx
            lda #3
            sta NUSIZ0
            sta NUSIZ1
            lda #1
            sta VDELP0
            sta VDELP1
            lda #WHITE
            sta COLUP0
            sta COLUP1
            SLEEP 6
            ldy #5
title_loop
              ; load and store first 3 
            lda TITLE_GRAPHICS_0,y              ;4   4
            sta GRP0                            ;3   7
            lda TITLE_GRAPHICS_1,y              ;4  11
            sta WSYNC
            sta.w GRP1                            ;3  14
            lda TITLE_GRAPHICS_2,y              ;4  18
            sta.w GRP0                            ;3  21
            ; load next 3 EDF
            ldx TITLE_GRAPHICS_4,y              ;4  25
            txs                                 ;2  27
            ldx TITLE_GRAPHICS_3,y              ;4  31
            lda TITLE_GRAPHICS_5,y              ;4  35
            stx.w GRP1                            ;4  39
            tsx                                 ;2  41
            stx GRP0                            ;3  44
            sta GRP1                            ;3  47
            sty GRP0 
            dey
            bpl title_loop
            ldx #$ff
            txs

            ldx #GRADIENT_FIELD_HEIGHT - 128
footer_loop
            sta WSYNC
            dex
            bne footer_loop
 

;--------------------
; Overscan start

waitOnOverscan
            ldx #OVERSCAN_HEIGHT
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

sub_inc_song
            lda audio_song
            clc
            adc #1
            cmp #NUM_SONGS
            bcc _inc_song_save
            lda #0
_inc_song_save
            sta audio_song
            rts

sub_dec_song
            lda audio_song
            sec
            sbc #1
            bcs _dec_song_save
            lda #(NUM_SONGS - 1)
_dec_song_save
            sta audio_song
            rts

sub_start_song
            lda audio_song
            lda SONG_TABLE_START_LO,y
            sta audio_song_ptr
            lda SONG_TABLE_START_HI,y
            sta audio_song_ptr + 1
            ldy #0
            sty audio_row_idx
            lda (audio_song_ptr),y
            sta audio_pattern_idx
            iny
            lda (audio_song_ptr),y
            sta audio_pattern_idx+1
            iny
            sty audio_song_order
            rts

sub_r9_tracker
            ldx #NUM_AUDIO_CHANNELS - 1
_audio_loop
            ldy audio_timer,x
            beq _audio_next_note
            dey
            sty audio_timer,x
            jmp _audio_next_channel
_audio_next_note
            ldy audio_pattern_idx,x 
            lda PAT_TABLE_START_LO,y
_audio_next_note_t
            sta tmp_pattern_ptr
            lda PAT_TABLE_START_HI,y
            sta tmp_pattern_ptr + 1
            ldy audio_row_idx
            lda (tmp_pattern_ptr),y
_audio_next_note_ty
            tay                       ; y is now waveform ptr
            lda WF_TABLE_START_LO,y
            sta tmp_waveform_ptr
            lda WF_TABLE_START_HI,y
            sta tmp_waveform_ptr + 1
            ldy audio_waveform_idx,x
            lda (tmp_waveform_ptr),y
            beq _audio_advance_tracker ; check for zero 
            lsr                        ; pull first bit
            bcc _set_registers         ; if set go to load registers
            lsr                        ; check second bit
            bcc _set_cx_vx             ; if clear we are loading aud(c|v)x
            lsr                        ; pull duration bit for later set
            sta AUDF0,x             ; store frequency
            sta vis_freq,x
            jmp _set_timer_delta       ; jump to duration 
_set_cx_vx  bcc _set_vx
            sta AUDC0,x
            jmp _set_timer_delta       ; jump to duration
_set_vx
            sta AUDV0,x
_set_timer_delta
            lda #0
            adc #1
            sta audio_timer,x
            jmp _audio_advance_waveform
_set_registers
            ; processing all registers
            pha                        ; save timer
            lsr
            lsr
            sta AUDF0,x
            sta vis_freq,x
            iny
            pla                      
            and #$03
            cmp #$03                   
            bne _set_timer_registers
            lda (tmp_waveform_ptr),y
            iny
_set_timer_registers
            sta audio_timer,x
            lda (tmp_waveform_ptr),y
            lsr
            lsr
            lsr
            lsr
            sta AUDC0,x
            lda (tmp_waveform_ptr),y
            and #$0f
            sta AUDV0,x
            sta vis_amp,x
_audio_advance_waveform
            iny
            sty audio_waveform_idx,x
            jmp _audio_next_channel
_audio_advance_tracker ; got a 0 on waveform
            lda #255
            sta audio_timer,x
            sta audio_waveform_idx,x
_audio_next_channel
            dex
            bpl _audio_loop

            ; update track - check if both waveforms done
            lda audio_waveform_idx
            and audio_waveform_idx+1
            cmp #255
            bne _audio_end            
            lda #0
            sta audio_timer
            sta audio_timer+1
            sta audio_waveform_idx
            sta audio_waveform_idx+1
            ldy audio_row_idx
            iny
            lda (tmp_pattern_ptr),y
            cmp #255
            beq _audio_advance_order
            sty audio_row_idx
            jmp sub_r9_tracker; if not 255 loop back 
_audio_advance_order ; got a 255 on pattern
            lda #0
            sta audio_row_idx
            ldy audio_song_order
            lda (audio_song_ptr),y
            cmp #255
            bne _audio_advance_order_advance_pattern
            ldy #0
            lda (audio_song_ptr),y
_audio_advance_order_advance_pattern
            sta audio_pattern_idx
            iny
            lda (audio_song_ptr),y
            sta audio_pattern_idx+1
            iny
            sty audio_song_order
            jmp sub_r9_tracker;  loop back 
_audio_end
            rts

; sethorizpos-style respx loop, with x = 0,1 for player 0,1
sub_respx
            sta WSYNC           ;-- --
            sec                 ;2   2
_respx_loop
            sbc #15             ;2   4
            bcs _respx_loop     ;2   6
            eor #7              ;2   8
            asl                 ;2  10
            asl                 ;2  12
            asl                 ;2  14
            asl                 ;2  16
            sta HMP0,x          ;4  20 
            sta RESP0,x         ;4  24
            sta WSYNC
            sta HMOVE
            rts

sub_waveform
            lsr ; div frequency by 4 
            lsr ; 
            tax
            ldy #$00
            jsr sub_freq_slice
            ldy #$ff
            jsr sub_freq_slice
            rts

sub_freq_slice
            sta WSYNC               ;-- --
            lda #0                  ;2   2
            sta PF0                 ;3   5
            tya                     ;2   7
            eor VIS_FREQ_PF1,x      ;4  11
            sta PF1                 ;3  14
            tya                     ;2  16
            eor VIS_FREQ_PF2,x      ;4  20
            sta PF2                 ;3  23
            and #$0f                ;2  25
            pha                     ;3  28
            tya                     ;2  30
            eor VIS_FREQ_PF0,x      ;4  34
            sta PF0                 ;3  37
            SLEEP 6                 ;6  43
            pla                     ;4  47
            sta PF2                 ;3  50
            sta WSYNC               ;-- --
            lda #0                  ;2   2
            sta PF0                 ;3   5
            sta PF1                 ;3  12
            sta PF2                 ;3  19
            rts
            

;-----------------------------------------------------------------------------------
; Audio Data 


    #include "TrackData.inc"


;-----------------------------------------------------------------------------------
; Support Data and Macros


VIS_FREQ_PF0
    byte %10101010
    byte %01010101
    byte %00110011
    byte %11001100
  	byte %11100000
    byte %00010000
    byte %00000000
    byte %11110000


VIS_FREQ_PF1
    byte %01010101
    byte %10101010
    byte %11001100
    byte %00110011
	  byte %10001110
	  byte %01110001
	  byte %01111100
    byte %10000011

VIS_FREQ_PF2
    byte %10101010
    byte %01010101
    byte %00110011
    byte %11001100
	  byte %00011100
	  byte %11100011
	  byte %11111000
    byte %00000111

CHANNEL_COLORS
    byte RED, BLUE

;-----------------------------------------------------------------------------------
; the CPU reset vectors

    ORG $FFFA

    .word Reset          ; NMI
    .word Reset          ; RESET
    .word Reset          ; IRQ

    END