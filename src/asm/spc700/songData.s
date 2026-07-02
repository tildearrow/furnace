; baked with Furnace.

songIns0:
  .db $f7, $70, $7f, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00,
  .db $00, $00, $00,

songInsListLow0:
  .db <songIns0

songInsListHigh0:
  .db >songIns0

songPitch0:
  .db $2f, $21, $28, $23, $3f, $25, $76, $27, $cf, $29, $4b, $2c, $ee, $2e, $b8, $31,
  .db $ad, $34, $cf, $37, $20, $3b, $a4, $3e, $f9, $01, $17, $02, $37, $02, $59, $02,
  .db $7c, $02, $a3, $02, $ca, $02, $f5, $02, $22, $03, $51, $03, $84, $03, $ba, $03,

songPitch1:
  .db $2f, $21, $28, $23, $3f, $25, $76, $27, $cf, $29, $4b, $2c, $ee, $2e, $b8, $31,
  .db $ad, $34, $cf, $37, $20, $3b, $a4, $3e,

songPitch2:
  .db $08, $22, $0e, $24, $33, $26, $79, $28, $e1, $2a, $6e, $2d, $21, $30, $fe, $32,
  .db $06, $36, $3c, $39, $a4, $3c, $3f, $40,

songPitchListLow0:
  .db $00,
  .db <songPitch0
  .db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00,
  .db <songPitch1
  .db <songPitch2

songPitchListHigh0:
  .db $00,
  .db >songPitch0
  .db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00,
  .db >songPitch1
  .db >songPitch2

songChipData0:
  .db $20, $20, $00, $7f, $7f, $00, $7f, $00, $00, $00, $00, $00, $00, $00, $00, $7f,
  .db $7f, $10, $00, $00,

