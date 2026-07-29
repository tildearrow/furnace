; baked with Furnace.

songPitch0:
  .db $2f, $21, $28, $23, $3f, $25, $76, $27, $cf, $29, $4b, $2c, $ee, $2e, $b8, $31,
  .db $ad, $34, $cf, $37, $20, $3b, $a4, $3e, $f9, $01, $17, $02, $37, $02, $59, $02,
  .db $7c, $02, $a3, $02, $ca, $02, $f5, $02, $22, $03, $51, $03, $84, $03, $ba, $03,

songPitch1:
  .db $2f, $21, $28, $23, $3f, $25, $76, $27, $cf, $29, $4b, $2c, $ee, $2e, $b8, $31,
  .db $ad, $34, $cf, $37, $20, $3b, $a4, $3e, $f9, $01, $17, $02, $37, $02, $59, $02,
  .db $7c, $02, $a3, $02, $ca, $02, $f5, $02, $22, $03, $51, $03, $84, $03, $ba, $03,

songPitch2:
  .db $08, $22, $0e, $24, $33, $26, $79, $28, $e1, $2a, $6e, $2d, $21, $30, $fe, $32,
  .db $06, $36, $3c, $39, $a4, $3c, $3f, $40, $06, $02, $25, $02, $46, $02, $68, $02,
  .db $8d, $02, $b3, $02, $dd, $02, $08, $03, $36, $03, $68, $03, $9b, $03, $d2, $03,

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

songShiftList0:
  .db $00, $0c, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00,
  .db $0c, $0b,

songSampleStartLow0:
  .db $a0,

songSampleStartHigh0:
  .db $08,

songSampleLoopLow0:
  .db $c3,

songSampleLoopHigh0:
  .db $50,

songInitState0:
  .db $20, $20, $20, $7f, $7f, $00, $7f, $00, $00, $00, $00, $00, $00, $00, $00, $7f,
  .db $7f, $10, $00, $00,

songMacro0:
  .db $00, $00, $00, $7f, $38, $7c, $62, $44, $38, $5b, $38, $80,

songIns0:
  .db $7f, $e0, $7f, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00,
  .db $00,
  .dw songMacro0
  .db $00, $00,

songInsListLow0:
  .db <songIns0

songInsListHigh0:
  .db >songIns0

songCmdStream0:
  .db $46, $43, $53, $00, $08, $00, $00, $00, $0c, $18, $24, $00, $00, $00, $00, $00,
  .db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $40, $7f,
  .db $00, $00, $00, $00, $00, $00, $00, $00,
  .dw (songCmdStream0+64)
  .dw (songCmdStream0+102)
  .dw (songCmdStream0+143)
  .dw (songCmdStream0+187)
  .dw (songCmdStream0+240)
  .dw (songCmdStream0+248)
  .dw (songCmdStream0+256)
  .dw (songCmdStream0+264)
  .db $01, $00, $00, $00, $00, $00, $00, $00, $e0, $d8,
  .dw (songCmdStream0+272)
  .db $61, $f1, $e6, $62, $f1, $6e, $f0, $62, $f1, $6e, $f0, $5f, $f1, $e7, $5d, $f1,
  .db $69, $f0, $5d, $f1, $69, $f0, $61, $f1, $d8,
  .dw (songCmdStream0+272)
  .db $5d, $f1, $da,
  .dd (songCmdStream0+64)
  .db $f0, $e0, $6e, $f2, $6e, $f2, $70, $f1, $e6, $72, $f2, $72, $f2, $6e, $f1, $e7,
  .db $6d, $f2, $6d, $f2, $70, $f1, $6e, $f0, $72, $f0, $77, $f0, $6e, $f0, $72, $f0,
  .db $75, $f0, $6d, $f0, $da,
  .dd (songCmdStream0+102)
  .db $e0, $6b, $f0, $72, $f2, $72, $f2, $74, $f1, $e6, $75, $f2, $75, $f2, $72, $f1,
  .db $e7, $70, $f2, $70, $f2, $74, $f0, $6b, $f0, $6d, $f0, $6e, $f0, $70, $f0, $72,
  .db $f0, $70, $f0, $6e, $f0, $6d, $f0, $da,
  .dd (songCmdStream0+143)
  .db $f0, $e0, $79, $f0, $7a, $f1, $79, $f0, $7a, $f1, $7c, $f0, $e6, $7e, $f1, $7a,
  .db $f1, $77, $f0, $7a, $f1, $77, $f0, $e7, $79, $f1, $79, $f0, $79, $f0, $7c, $f0,
  .db $79, $f1, $79, $f0, $77, $f0, $7e, $f1, $77, $f0, $7e, $f1, $7a, $f0, $77, $f0,
  .db $da,
  .dd (songCmdStream0+187)
  .db $dc, $80, $01, $da,
  .dd (songCmdStream0+240)
  .db $dc, $80, $01, $da,
  .dd (songCmdStream0+248)
  .db $dc, $80, $01, $da,
  .dd (songCmdStream0+256)
  .db $dc, $80, $01, $da,
  .dd (songCmdStream0+264)
  .db $5f, $f1, $6b, $f0, $5f, $f1, $6b, $f0, $d9,

