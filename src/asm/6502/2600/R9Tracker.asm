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
audio_song_order    ds 1  ; what index are we at in song
audio_row_idx       ds 1  ; where are we in the current order
audio_pattern_idx   ds 2  ; which pattern is playing on each channel
audio_waveform_idx  ds 2  ; where are we in waveform on each channel
audio_timer         ds 2  ; time left on next action on each channel

tmp_pattern_ptr     ds 2
tmp_waveform_ptr    ds 2


vis_song            ds 1
vis_order           ds 1
vis_row             ds 1

vis_frequency       ds 2
vis_amplitude       ds 2
vis_waveform        ds 2
vis_gradient        ds 2

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
; TODO: fire = pause / play
; TODO: l/r  = fwd / reverse
; TODO: u/d  = choose track
;


;
; -- audio tracker
;

audio_tracker           
            ldx #NUM_AUDIO_CHANNELS - 1
audio_loop 
            ldy audio_timer,x
            beq _audio_next_note
            dey
            sty audio_timer,x
            jmp _audio_next_channel
_audio_next_note
            ldy audio_pattern_idx,x 
            lda PAT_TABLE_START,y
_audio_next_note_t
            sta tmp_pattern_ptr
            lda PAT_TABLE_START+1,y
            sta tmp_pattern_ptr + 1
            ldy audio_row_idx
            lda (tmp_pattern_ptr),y
_audio_next_note_ty
            tay                       ; y is now waveform ptr
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
            lda #255
            sta audio_timer,x
            lda #255
            sta audio_waveform_idx,x
_audio_next_channel
            dex
            bpl audio_loop

            ; update track - check if both waveforms done
            lda audio_waveform_idx
            and audio_waveform_idx+1
            cmp #255
            bne audio_end            
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
            jmp audio_tracker; if not 255 loop back 
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
            jmp audio_tracker;  loop back 

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

            ldx #NUM_AUDIO_CHANNELS - 1
vis_calc_loop
            lda CHANNEL_COLORS,x
            ora vis_amplitude,x
            sta vis_gradient,x
            dex
            bpl vis_calc_loop

;---------------------
; end vblank

            jsr waitOnVBlank ; SL 34
            sta WSYNC ; SL 35
            lda #1
            sta CTRLPF ; reflect playfield
            lda #BLACK
            sta COLUBK
            lda #0
            sta GRP0
            lda #WHITE
            sta COLUP0

            ldx #100
header_loop
            sta WSYNC
            dex
            bpl header_loop 

            DISPLAY_COUNTER vis_song
            DISPLAY_COUNTER vis_order
            DISPLAY_COUNTER vis_row

   
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
            asl
            lda SONG_TABLE_START,y
            sta audio_song_ptr
            iny
            lda SONG_TABLE_START,y
            sta audio_song_ptr + 1
            ldy #0
            sty audio_row_idx
            sty audio_waveform_idx
            sty audio_waveform_idx + 1
            lda (audio_song_ptr),y
            sta audio_pattern_idx
            iny
            lda (audio_song_ptr),y
            sta audio_pattern_idx+1
            iny
            sty audio_song_order
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

      MAC DISPLAY_COUNTER
            sta WSYNC
            lda #WHITE
            sta COLUBK
            sta WSYNC
            lda {1}
            sec
._counter_resp_loop
            sbc #15
            bcs ._counter_resp_loop
            tay
            lda LOOKUP_STD_HMOVE,y
            sta HMP0
            sta RESP0
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
      ENDM

;-----------------------------------------------------------------------------------
; the CPU reset vectors

    ORG $FFFA

    .word Reset          ; NMI
    .word Reset          ; RESET
    .word Reset          ; IRQ

    END