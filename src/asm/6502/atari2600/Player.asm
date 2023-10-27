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
GRADIENT_FIELD_HEIGHT = 193 - (COUNTER_HEIGHT * 3)
OVERSCAN_HEIGHT = 30
VERTICAL_BANNER_POS = 130
MAX_SPEED = 4

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
audio_pattern_ptr   ds 2
audio_waveform_idx  ds 2  ; where are we in waveform on each channel
audio_waveform_ptr  ds 2
audio_timer         ds 2  ; time left on next action on each channel

audio_registers
audio_cx            ds 2  ; 
audio_fx            ds 2  ; 
audio_vx            ds 2


audio_stack_ptr     ds 2
audio_buffer        ds (6 * 8)
audio_buffer_end

speed               ds 1  ; playback speed
debounce_input      ds 1

tmp_input           ds 1
tmp_update_ctl      ds 1

vis_gradient        ds 16
vis_title_start     ds 1
vis_title_end       ds 1


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
            lda #5
            sta vis_title_end

newFrame

    ; 3 scanlines of vertical sync signal to follow

            ldx #%00000010
            stx VSYNC               ; turn ON VSYNC bit 1
            ldx #0

            sta WSYNC               ; wait a scanline
            sta WSYNC               ; another
            sta WSYNC               ; another = 3 lines total

            stx VSYNC               ; turn OFF VSYNC bit 1

    ; 34 scanlines of vertical blank to follow

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

;--------------------
; handle input

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
            jmp _save_speed
_do_pause
            lda #0
            jmp _save_speed
_skip_trigger_pause
            ror
            bcs _up
            ror
            bcs _down
            ror 
            bcs _left
            ror 
            bcs _right
            jmp _end_input
_down
            jsr sub_inc_song
            jmp _end_input            
_up
            jsr sub_dec_song
            jmp _end_input  
_left
            lda #$ff
            jmp _add_speed
_right
            lda #1
_add_speed
            clc
            adc speed
            bmi _do_pause
            cmp #MAX_SPEED
            bmi _save_speed
            lda #MAX_SPEED
_save_speed
            sta speed
_end_input
            lda tmp_input
            sta debounce_input

;---------------------
; audio tracker

            ldx speed
            bne audio_tracker_on
            ldy #5
_audio_on_pause
            stx audio_fx,y
            dey
            bpl _audio_on_pause
audio_tracker_on
            lda SPEED_UPDATE_PATTERN,x
            sta tmp_update_ctl
            
            ; fill stack with audio
            lda #audio_buffer
            sta audio_stack_ptr
_audio_buffer_loop
            lsr tmp_update_ctl
            bcc _audio_skip_update
            jsr sub_play_song
_audio_skip_update
            ldy #5
_audio_update_loop
            lda audio_registers,y
            sta (audio_stack_ptr),y
            dey
            bpl _audio_update_loop
            jsr sub_advance_audio
            bne _audio_buffer_loop
            lda #audio_buffer
            sta audio_stack_ptr
            
;---------------------
; vis timing

            lda #$f0
            ldx #15
_vis_gradient_setup_loop
            sta vis_gradient,x
            sec
            sbc #$10
            dex
            bpl _vis_gradient_setup_loop

            ldy audio_row_idx
            lda (audio_pattern_ptr),y
            and #$0f
            tax
            lda #$0f
            sta vis_gradient,x

            lda frame
            beq _vis_save_title
            bpl _vis_skip_title
            and #$07
            bne _vis_skip_title
            lda vis_title_start
            clc
            adc #6
            cmp #((TITLE_LENGTH - 5) * 6)
            beq _vis_skip_title
_vis_save_title
            sta vis_title_start
            clc
            adc #5
            sta vis_title_end ; BUGBUG initially uninitialized
_vis_skip_title

;---------------------
; end vblank

waitOnVBlank
            ldx #$00
waitOnVBlank_loop          
            cpx INTIM
            bmi waitOnVBlank_loop
            stx VBLANK
            sta WSYNC ; SL 38
            jsr sub_deque_audio ; 0
            stx COLUPF
            inx ; x = 1
            stx CTRLPF ; reflect playfield
            lda #$f0
            sta PF0

            lda #12
vis_loop
            tax
            ldy #31
gradient_loop
            sta WSYNC               ;--
            SLEEP 17
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
            jsr sub_deque_audio ; 1-4
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
            lda audio_fx
            jsr sub_waveform
            lda audio_fx + 1
            jsr sub_waveform


; Title display 
            sta WSYNC                           ;-- --
            lda #3                              ;2   2
            sta NUSIZ0                          ;3   5
            sta NUSIZ1                          ;3   8
            lda #1                              ;2  10
            sta VDELP0                          ;3  13
            sta VDELP1                          ;3  16
            lda #$f0                            ;2  18
            sta HMP0                            ;3  21 
            lda #$00                            ;2  23
            sta RESP0                           ;3  26
            sta RESP1                           ;3  29
            sta HMP1                            ;3  32
            sta WSYNC                           ;-- --
            sta HMOVE                           ;3   3
            lda #WHITE
            sta COLUP0
            sta COLUP1
            ldy vis_title_end
title_loop
              ; load and store first 3 
            lda TITLE_GRAPHICS_0,y              ;4   4
            sta GRP0                            ;3   7
            lda TITLE_GRAPHICS_1,y              ;4  11
            sta WSYNC
            sta.w GRP1                          ;3  14
            lda TITLE_GRAPHICS_2,y              ;4  18
            sta.w GRP0                          ;3  21
            ; load next 3 EDF
            ldx TITLE_GRAPHICS_4,y              ;4  25
            txs                                 ;2  27
            ldx TITLE_GRAPHICS_3,y              ;4  31
            lda TITLE_GRAPHICS_5,y              ;4  35
            stx.w GRP1                          ;4  39
            tsx                                 ;2  41
            stx GRP0                            ;3  44
            sta GRP1                            ;3  47
            sta GRP0 
            dey
            cpy vis_title_start
            bpl title_loop
            ldx #$ff ; restore stack
            txs

;--------------------
; Footer + Overscan 

            ldy #16
            ldx #(GRADIENT_FIELD_HEIGHT - 128) + OVERSCAN_HEIGHT
overscan_loop
            sta WSYNC
            dey 
            bpl _overscan_skip_deque
            jsr sub_deque_audio
            ldy #32
_overscan_skip_deque
            dex
            bne overscan_loop
            jmp newFrame

sub_advance_audio
            lda audio_stack_ptr
            clc
            adc #6
            sta audio_stack_ptr
            cmp #audio_buffer_end
            rts

sub_deque_audio
            ldy #5
_deque_audio_loop
            lda (audio_stack_ptr),y
            sta AUDC0,y
            dey
            bpl _deque_audio_loop
            jsr sub_advance_audio
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

  #include "Player_core.asm"

;-----------------------------------------------------------------------------------
; Audio

    ORG $F300 ; leave F300 - FF80 for the actual song


    #include "Track_data.asm"

;-----------------------------------------------------------------------------------
; Graphics

    ORG $FF80

    #include "Track_meta.asm"

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

SPEED_UPDATE_PATTERN
    byte %00000000
    byte %00000001
    byte %00010001
    byte %01010101
    byte %11111111 ; screen unstable

;-----------------------------------------------------------------------------------
; the CPU reset vectors

    ORG $FFFA

    .word Reset          ; NMI
    .word Reset          ; RESET
    .word Reset          ; IRQ

    END