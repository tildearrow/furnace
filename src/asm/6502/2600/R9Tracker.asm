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

vis_song            ds 1
vis_order           ds 1
vis_row             ds 1


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
audio_end

;---------------------
; vis timing

            lda audio_song
            asl
            asl
            asl
            sta vis_song
            lda audio_song_order
            asl
            asl
            sta vis_order
            lda audio_row_idx
            sta vis_row

;---------------------
; end vblank

            jsr waitOnVBlank ; SL 34
            sta WSYNC ; SL 35
            lda #1
            sta CTRLPF ; reflect playfield
            lda #255
            sta COLUP0
            lda #BLACK
            sta COLUBK
            sta COLUPF
            lda #0
            sta GRP0
            lda #WHITE
            sta COLUP0

            ldx #100
header_loop
            sta WSYNC
            dex
            bpl header_loop 

            lda vis_song
            jsr sub_display_counter
            lda vis_order
            jsr sub_display_counter
            lda vis_row
            jsr sub_display_counter
   
            ldx #100
footer_loop
            sta WSYNC
            dex
            bmi footer_loop  

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

sub_display_counter
            sta WSYNC
            ldx #WHITE              ;2    2
            stx COLUBK              ;3    5
            sec                     ;2   10
._counter_resp_loop 
            sbc #15                 ;2   12
            bcs ._counter_resp_loop ;2/3 14
            tay                     ;2   16
            lda LOOKUP_STD_HMOVE,y  ;4   20
            sta HMP0                ;3   23
            sta RESP0               ;3   26
            sta WSYNC
            sta HMOVE
            lda #BLACK
            sta COLUBK
            lda #$ff
            sta GRP0
            SLEEP 10 ; make safe for HMOVE            
            lda #0
            sta HMP0
            ldx #6
._counter_block_loop
            sta WSYNC
            dex
            bpl ._counter_block_loop
            lda #WHITE
            sta COLUBK
            lda #0 
            sta GRP0
            sta WSYNC
            sta COLUBK
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
            sta AUDF0,x                ; store frequency
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


;-----------------------------------------------------------------------------------
; Audio Data 

    ALIGN 256

AUDIO_TRACKS
    byte 0

    #include "R9Data.inc"


;-----------------------------------------------------------------------------------
; Support Data and Macros

LOOKUP_STD_HMOVE = STD_HMOVE_END - 256
    ; standard lookup for hmoves
STD_HMOVE_BEGIN
    byte $80, $70, $60, $50, $40, $30, $20, $10, $00, $f0, $e0, $d0, $c0, $b0, $a0, $90
STD_HMOVE_END

CHANNEL_COLORS
    byte RED, BLUE

;-----------------------------------------------------------------------------------
; the CPU reset vectors

    ORG $FFFA

    .word Reset          ; NMI
    .word Reset          ; RESET
    .word Reset          ; IRQ

    END