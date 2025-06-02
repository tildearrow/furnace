; this is an example file.

; constants
FCS_MAX_CHAN=8 ; maximum number of channels (up to 127, but see below!)
FCS_MAX_STACK=16 ; stack depth per channel (FCS_MAX_STACK*FCS_MAX_CHAN<256)

; player constants - change if necessary
fcsAddrBase=$20 ; player state address base
fcsZeroPage=$10 ; player state address base for zero-page-mandatory variables
fcsGlobalStack=$200 ; player stack (best placed in a page)
fcsPtr=$8000 ; pointer to command stream
fcsVolMax=fcsVolMaxExample ; pointer to max channel volume array

; command call table
; - see src/engine/dispatch.h for a list of commands to be potentially handled
;   - do not implement HINT commands - the player will never send these
;   - no need to implement commands not pertaining to the target system
; - a zero pointer means "don't handle"
fcsCmdTableLow=fcsCmdTableExample
fcsCmdTableHigh=fcsCmdTableExample

; "dummy" implementation - example only!

fcsDummyFunc:
  rts

fcsVolMaxExample:
  .db $7f, $00
  .db $7f, $00
  .db $7f, $00
  .db $7f, $00
  .db $7f, $00
  .db $7f, $00
  .db $7f, $00
  .db $7f, $00

fcsCmdTableExample:
  .dsb 256, 0
