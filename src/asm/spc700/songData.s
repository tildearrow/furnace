; baked with Furnace.

songPitch0:
  .db $7d, $21, $7b, $23, $97, $25, $d3, $27, $31, $2a, $b4, $2c, $5c, $2f, $2d, $32,
  .db $29, $35, $52, $38, $ab, $3b, $38, $3f, $fe, $01, $1c, $02, $3c, $02, $5e, $02,
  .db $83, $02, $a8, $02, $d1, $02, $fc, $02, $29, $03, $59, $03, $8d, $03, $c2, $03,

songPitch1:
  .db $7d, $21, $7b, $23, $97, $25, $d3, $27, $31, $2a, $b4, $2c, $5c, $2f, $2d, $32,
  .db $29, $35, $52, $38, $ab, $3b, $38, $3f, $fe, $01, $1c, $02, $3c, $02, $5e, $02,
  .db $83, $02, $a8, $02, $d1, $02, $fc, $02, $29, $03, $59, $03, $8d, $03, $c2, $03,

songPitch2:
  .db $66, $21, $62, $23, $7d, $25, $b8, $27, $14, $2a, $95, $2c, $3b, $2f, $0a, $32,
  .db $04, $35, $2b, $38, $82, $3b, $0c, $3f, $fc, $01, $1b, $02, $3b, $02, $5c, $02,
  .db $81, $02, $a6, $02, $cf, $02, $fa, $02, $27, $03, $57, $03, $8a, $03, $c0, $03,

songPitch3:
  .db $09, $20, $f0, $21, $f5, $23, $18, $26, $5c, $28, $c3, $2a, $4e, $2d, $ff, $2f,
  .db $da, $32, $e0, $35, $14, $39, $79, $3c, $e7, $01, $05, $02, $23, $02, $44, $02,
  .db $67, $02, $8b, $02, $b1, $02, $db, $02, $06, $03, $34, $03, $65, $03, $99, $03,

songPitch4:
  .db $7d, $21, $7b, $23, $97, $25, $d3, $27, $31, $2a, $b4, $2c, $5c, $2f, $2d, $32,
  .db $29, $35, $52, $38, $ab, $3b, $38, $3f, $fe, $01, $1c, $02, $3c, $02, $5e, $02,
  .db $83, $02, $a8, $02, $d1, $02, $fc, $02, $29, $03, $59, $03, $8d, $03, $c2, $03,

songPitch5:
  .db $7d, $21, $7b, $23, $97, $25, $d3, $27, $31, $2a, $b4, $2c, $5c, $2f, $2d, $32,
  .db $29, $35, $52, $38, $ab, $3b, $38, $3f, $fe, $01, $1c, $02, $3c, $02, $5e, $02,
  .db $83, $02, $a8, $02, $d1, $02, $fc, $02, $29, $03, $59, $03, $8d, $03, $c2, $03,

songPitch6:
  .db $2d, $32, $28, $35, $52, $38, $ab, $3b, $37, $3f, $f9, $42, $f5, $46, $2d, $4b,
  .db $a5, $4f, $62, $54, $66, $59, $b7, $5e, $fb, $02, $2a, $03, $59, $03, $8c, $03,
  .db $c2, $03, $fc, $03, $38, $04, $78, $04, $bd, $04, $04, $05, $51, $05, $a2, $05,

songPitchListLow0:
  .db $00,
  .db <songPitch0
  .db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00,
  .db <songPitch1
  .db <songPitch2
  .db <songPitch3
  .db <songPitch4
  .db <songPitch5
  .db <songPitch6

songPitchListHigh0:
  .db $00,
  .db >songPitch0
  .db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00,
  .db >songPitch1
  .db >songPitch2
  .db >songPitch3
  .db >songPitch4
  .db >songPitch5
  .db >songPitch6

songShiftList0:
  .db $00, $0c, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00,
  .db $0c, $0a, $0a, $0a, $0a, $0b,

songSampleStartLow0:
  .db $a0, $0c, $71, $6a, $94,

songSampleStartHigh0:
  .db $08, $09, $0d, $1a, $1d,

songSampleLoopLow0:
  .db $e8, $70, $46, $f2, $7e,

songSampleLoopHigh0:
  .db $08, $0d, $1a, $1c, $1e,

songInitState0:
  .db $7f, $7f, $00, $44, $53, $69, $cc, $09, $f7, $00, $00, $d4, $f9, $00, $05, $7f,
  .db $7f, $10, $00, $00,

songMacro0:
  .db $00, $00, $00, $2c, $7f, $5a, $2f, $80,

songMacro1:
  .db $01, $00, $00, $92, $fb, $92, $ff, $00, $80,

songIns0:
  .db $ff, $e0, $7f, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00,
  .db $00,
  .dw songMacro0
  .dw songMacro1
  .db $00, $00,

songMacro2:
  .db $00, $00, $00, $7f, $80,

songMacro3:
  .db $01, $00, $00, $09, $05, $01, $00, $80,

songIns1:
  .db $ff, $e0, $7f, $00, $00, $01, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00,
  .db $00,
  .dw songMacro2
  .dw songMacro3
  .db $00, $00,

songMacro4:
  .db $00, $00, $00, $7f, $7f, $7f, $7d, $7b, $79, $77, $73, $6f, $6a, $67, $62, $5e,
  .db $5a, $57, $53, $4e, $4c, $4b, $4a, $48, $46, $44, $42, $80,

songIns2:
  .db $ff, $e0, $7f, $00, $00, $02, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00,
  .db $00,
  .dw songMacro4
  .db $00, $00,

songMacro5:
  .db $00, $00, $00, $7f, $80,

songMacro6:
  .db $01, $00, $00, $02, $80,

songIns3:
  .db $8f, $ce, $7f, $2d, $00, $03, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00,
  .db $00,
  .dw songMacro5
  .dw songMacro6
  .db $00, $00,

songMacro7:
  .db $00, $00, $00, $7f, $76, $76, $76, $76, $75, $75, $75, $3d, $19, $07, $05, $04,
  .db $01, $00, $00, $00, $00, $80,

songMacro8:
  .db $01, $00, $00, $1e, $04, $03, $00, $92, $fa, $92, $f9, $92, $f8, $92, $f8, $92,
  .db $f8, $92, $f6, $92, $f5, $92, $f5, $92, $f4, $80,

songIns4:
  .db $ff, $e0, $7f, $00, $00, $03, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00,
  .db $00,
  .dw songMacro7
  .dw songMacro8
  .db $00, $00,

songMacro9:
  .db $00, $00, $00, $5c, $80,

songIns5:
  .db $9f, $c0, $7f, $2e, $00, $04, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00,
  .db $00,
  .dw songMacro9
  .db $00, $00,

songInsListLow0:
  .db <songIns0
  .db <songIns1
  .db <songIns2
  .db <songIns3
  .db <songIns4
  .db <songIns5

songInsListHigh0:
  .db >songIns0
  .db >songIns1
  .db >songIns2
  .db >songIns3
  .db >songIns4
  .db >songIns5

songCmdStream0:
  .db $46, $43, $53, $00, $08, $00, $00, $00, $0b, $16, $21, $2c, $02, $14, $6e, $42,
  .db $84, $09, $58, $37, $4d, $63, $79, $b0, $04, $01, $02, $05, $00, $03, $2a, $3a,
  .db $7a, $6a, $00, $00, $84, $00, $00, $00,
  .dw (songCmdStream0+64)
  .dw (songCmdStream0+319)
  .dw (songCmdStream0+514)
  .dw (songCmdStream0+714)
  .dw (songCmdStream0+876)
  .dw (songCmdStream0+895)
  .dw (songCmdStream0+917)
  .dw (songCmdStream0+929)
  .db $03, $02, $01, $01, $02, $02, $05, $04, $e5, $e9, $5e, $ec, $00, $d8,
  .dw (songCmdStream0+1177)
  .db $62, $f2, $5d, $f0, $5b, $f0, $60, $f0, $65, $f0, $64, $f0, $60, $f0, $5d, $f0,
  .db $5b, $f1, $62, $f1, $65, $f1, $60, $f3, $59, $f0, $5b, $f0, $60, $f0, $67, $f1,
  .db $60, $f0, $59, $f1, $59, $f1, $60, $f1, $65, $f3, $60, $f0, $63, $f1, $60, $f0,
  .db $67, $f0, $60, $f0, $5e, $d8,
  .dw (songCmdStream0+1177)
  .db $5c, $f2, $60, $f0, $62, $f1, $64, $f0, $62, $f0, $5c, $f1, $5b, $f1, $5b, $f1,
  .db $59, $f0, $5b, $f0, $5d, $f2, $58, $f0, $5d, $f1, $60, $d8,
  .dw (songCmdStream0+1291)
  .db $f1, $5d, $f0, $60, $f0, $62, $f2, $60, $f0, $67, $f0, $60, $d8,
  .dw (songCmdStream0+1105)
  .db $62, $f1, $69, $f1, $6e, $f1, $62, $f3, $5d, $f0, $60, $f0, $65, $f0, $64, $f0,
  .db $62, $f0, $5d, $f0, $d8,
  .dw (songCmdStream0+1187)
  .db $60, $f2, $60, $f1, $5b, $f0, $60, $f2, $5b, $f0, $60, $f1, $64, $f1, $60, $f1,
  .db $59, $f1, $59, $f1, $64, $f0, $65, $f0, $59, $f2, $59, $f0, $5b, $f1, $5d, $f1,
  .db $54, $f1, $59, $f1, $60, $f1, $65, $f1, $63, $f3, $62, $f1, $60, $f1, $5b, $f0,
  .db $5d, $d8,
  .dw (songCmdStream0+1105)
  .db $5c, $f1, $63, $f1, $68, $f1, $5c, $f3, $5c, $f0, $5a, $f0, $5c, $f1, $5c, $f1,
  .db $d8,
  .dw (songCmdStream0+1187)
  .db $5d, $f1, $5d, $f1, $5b, $f0, $5d, $f0, $5d, $f2, $5d, $f0, $60, $f0, $5d, $f0,
  .db $64, $d8,
  .dw (songCmdStream0+1291)
  .db $f3, $5d, $f1, $60, $f0, $62, $f0, $60, $f0, $5d, $f0, $62, $f1, $5d, $f1, $62,
  .db $f1, $67, $f1, $66, $f1, $64, $f1, $62, $f1, $5d, $f1, $da,
  .dd (songCmdStream0+64)
  .db $e2, $e7, $5d, $ec, $01, $d8,
  .dw (songCmdStream0+1323)
  .db $f2, $b6, $fc, $67, $f7, $69, $f3, $71, $f0, $70, $f0, $6c, $f0, $69, $f0, $67,
  .db $f0, $64, $f0, $5e, $f1, $62, $f1, $69, $f1, $67, $f3, $64, $f1, $67, $f1, $64,
  .db $f3, $62, $f3, $63, $fd, $60, $f0, $5d, $d8,
  .dw (songCmdStream0+1323)
  .db $f7, $b6, $f3, $67, $f1, $69, $f1, $6c, $f1, $68, $f2, $b6, $f0, $69, $f1, $6c,
  .db $f1, $6e, $f3, $70, $f1, $6e, $f1, $6c, $f3, $64, $f1, $67, $f1, $64, $f7, $62,
  .db $f3, $6e, $f2, $66, $f2, $5d, $f1, $d8,
  .dw (songCmdStream0+1155)
  .db $59, $f1, $5b, $f1, $5d, $f1, $62, $f6, $5b, $f1, $69, $f2, $64, $f0, $65, $f3,
  .db $5d, $f1, $65, $f3, $67, $f2, $67, $f3, $62, $f3, $5d, $f3, $54, $f0, $67, $f2,
  .db $67, $f2, $67, $f2, $60, $f3, $64, $f2, $60, $f1, $60, $f3, $62, $f3, $5e, $f0,
  .db $60, $fb, $d8,
  .dw (songCmdStream0+1155)
  .db $60, $f2, $60, $f2, $5e, $fa, $6c, $f1, $62, $f7, $60, $f6, $64, $ff, $5b, $f0,
  .db $5d, $f1, $5b, $f0, $5d, $f2, $5d, $f2, $5d, $f1, $5b, $f0, $5d, $f1, $5d, $f1,
  .db $5a, $f0, $5d, $f2, $5d, $f2, $5a, $f0, $5d, $f2, $62, $f3, $da,
  .dd (songCmdStream0+319)
  .db $e2, $e7, $6e, $ec, $01, $d8,
  .dw (songCmdStream0+1244)
  .db $6e, $f1, $70, $f1, $6e, $f7, $6c, $f3, $60, $f1, $65, $f3, $6c, $f3, $67, $f3,
  .db $69, $f7, $64, $f6, $64, $f1, $6e, $d8,
  .dw (songCmdStream0+1244)
  .db $b6, $f1, $64, $f1, $62, $f3, $6e, $f1, $70, $f1, $71, $f3, $73, $f1, $71, $f1,
  .db $70, $f3, $69, $f1, $6c, $f1, $69, $f3, $67, $fc, $69, $f2, $64, $f3, $59, $f1,
  .db $60, $f1, $65, $f1, $69, $f6, $59, $f1, $62, $f1, $65, $f1, $67, $f6, $5d, $f1,
  .db $60, $f1, $64, $f1, $67, $f6, $5b, $f1, $5d, $f1, $62, $f1, $65, $fe, $5e, $fb,
  .db $5b, $fb, $5e, $f0, $5e, $f3, $64, $f2, $64, $f1, $5e, $f2, $65, $f3, $5b, $f3,
  .db $64, $f2, $64, $f2, $64, $f3, $64, $f0, $65, $f2, $60, $f1, $63, $f2, $63, $f2,
  .db $63, $f7, $63, $f3, $62, $f1, $64, $f1, $65, $f1, $69, $f6, $5b, $f1, $60, $f1,
  .db $64, $f1, $67, $f6, $60, $f1, $62, $f1, $64, $f1, $67, $f6, $63, $f2, $63, $f2,
  .db $63, $f3, $62, $f3, $69, $f1, $65, $f7, $65, $f6, $60, $dd, $c6, $60, $f2, $64,
  .db $f2, $64, $f2, $64, $f2, $64, $f1, $62, $f2, $62, $f2, $62, $f3, $60, $f3, $5d,
  .db $f1, $da,
  .dd (songCmdStream0+514)
  .db $e7, $ec, $01, $f7, $e2, $d8,
  .dw (songCmdStream0+1279)
  .db $f3, $70, $f3, $6e, $f1, $65, $f3, $6e, $fa, $6a, $f3, $60, $f7, $6a, $f3, $69,
  .db $f1, $67, $f0, $65, $f2, $62, $f1, $64, $f1, $65, $f1, $d8,
  .dw (songCmdStream0+1279)
  .db $f2, $b6, $f0, $71, $f1, $73, $f1, $75, $f3, $76, $f1, $75, $f1, $73, $f3, $6c,
  .db $f1, $70, $f1, $6e, $fa, $69, $f3, $67, $f2, $62, $fc, $b6, $dc, $aa, $02, $65,
  .db $f3, $62, $f7, $69, $f3, $60, $f2, $60, $f2, $60, $f2, $64, $f0, $60, $f2, $59,
  .db $f0, $58, $f1, $60, $f1, $60, $f3, $60, $f6, $69, $f2, $69, $f2, $67, $f7, $65,
  .db $f3, $5e, $f1, $6e, $f1, $6c, $f1, $6e, $f6, $64, $f1, $70, $f1, $71, $f1, $73,
  .db $f8, $70, $f1, $71, $f1, $70, $f6, $65, $f2, $65, $f2, $65, $f7, $65, $fa, $d8,
  .dw (songCmdStream0+1208)
  .db $f3, $67, $f2, $67, $f2, $67, $f2, $67, $f2, $67, $f1, $66, $f2, $66, $f2, $66,
  .db $f3, $64, $f3, $b6, $f1, $da,
  .dd (songCmdStream0+714)
  .db $dc, $3e, $05, $e3, $e8, $cf, $ff, $80, $d8,
  .dw (songCmdStream0+980)
  .db $dc, $76, $01, $da,
  .dd (songCmdStream0+876)
  .db $dc, $3e, $05, $c5, $fa, $f0, $e3, $e8, $cf, $00, $ff, $d8,
  .dw (songCmdStream0+980)
  .db $dc, $6b, $01, $da,
  .dd (songCmdStream0+895)
  .db $e0, $d8,
  .dw (songCmdStream0+941)
  .db $d8,
  .dw (songCmdStream0+941)
  .db $da,
  .dd (songCmdStream0+917)
  .db $e4, $d8,
  .dw (songCmdStream0+948)
  .db $d8,
  .dw (songCmdStream0+948)
  .db $da,
  .dd (songCmdStream0+929)
  .db $d8,
  .dw (songCmdStream0+958)
  .db $d8,
  .dw (songCmdStream0+958)
  .db $d9, $d8,
  .dw (songCmdStream0+968)
  .db $d8,
  .dw (songCmdStream0+968)
  .db $d8,
  .dw (songCmdStream0+968)
  .db $d9, $d8,
  .dw (songCmdStream0+1040)
  .db $d8,
  .dw (songCmdStream0+1040)
  .db $d8,
  .dw (songCmdStream0+1040)
  .db $d9, $e6, $6a, $ec, $01, $d8,
  .dw (songCmdStream0+1093)
  .db $6a, $d8,
  .dw (songCmdStream0+1093)
  .db $d9, $d8,
  .dw (songCmdStream0+1266)
  .db $69, $f1, $67, $f1, $60, $f8, $5d, $f1, $64, $f1, $65, $f8, $64, $d8,
  .dw (songCmdStream0+1314)
  .db $f7, $59, $d8,
  .dw (songCmdStream0+1314)
  .db $f3, $59, $f1, $5b, $f1, $59, $f1, $5d, $dc, $34, $01, $d8,
  .dw (songCmdStream0+1266)
  .db $64, $f1, $65, $f1, $67, $f8, $6a, $f1, $69, $f1, $65, $f8, $62, $f1, $65, $f1,
  .db $6c, $f7, $d8,
  .dw (songCmdStream0+1208)
  .db $d9, $56, $ec, $00, $f2, $56, $ec, $00, $f0, $d8,
  .dw (songCmdStream0+1258)
  .db $f1, $56, $ec, $00, $f2, $d8,
  .dw (songCmdStream0+1258)
  .db $f0, $56, $ec, $00, $f1, $56, $ec, $00, $f1, $d8,
  .dw (songCmdStream0+1302)
  .db $f3, $56, $ec, $00, $f0, $56, $ec, $00, $f0, $e1, $67, $ec, $01, $f4, $b5, $f9,
  .db $e0, $56, $ec, $00, $f2, $d9, $f0, $6e, $d8,
  .dw (songCmdStream0+1125)
  .db $69, $f0, $6c, $d8,
  .dw (songCmdStream0+1125)
  .db $d9, $f0, $5e, $d8,
  .dw (songCmdStream0+1228)
  .db $5e, $d8,
  .dw (songCmdStream0+1177)
  .db $5d, $f3, $5d, $f0, $5b, $f0, $5d, $f1, $5d, $f1, $d9, $f0, $75, $f0, $6a, $f0,
  .db $6e, $f0, $73, $f0, $6a, $f0, $6e, $f0, $71, $f0, $6a, $f0, $6e, $f0, $70, $f0,
  .db $67, $f0, $6c, $f0, $6e, $f0, $65, $f0, $d9, $5d, $d8,
  .dw (songCmdStream0+1323)
  .db $f6, $5d, $f1, $5e, $f1, $62, $f1, $64, $f6, $5b, $f1, $5d, $f1, $60, $f1, $64,
  .db $f6, $d9, $d8,
  .dw (songCmdStream0+1228)
  .db $5d, $f1, $64, $f1, $69, $f1, $d9, $5b, $f1, $62, $f1, $65, $f1, $5b, $f2, $56,
  .db $f0, $59, $f0, $5b, $f0, $62, $f1, $59, $f0, $5b, $f0, $d9, $6a, $f1, $69, $f3,
  .db $65, $f1, $62, $f1, $67, $fa, $69, $f1, $64, $f1, $60, $f1, $5d, $f1, $62, $d9,
  .db $f1, $65, $f1, $6a, $f1, $5e, $f3, $5e, $f0, $6a, $f0, $69, $f1, $5e, $f1, $d9,
  .db $f1, $70, $f1, $71, $f1, $69, $f3, $76, $f1, $75, $f3, $6c, $f1, $d9, $d8,
  .dw (songCmdStream0+1302)
  .db $f0, $56, $ec, $00, $d9, $62, $f1, $60, $f1, $62, $f8, $64, $f1, $65, $f1, $67,
  .db $f8, $d9, $78, $fa, $71, $f1, $70, $f1, $71, $f1, $73, $f1, $71, $d9, $f1, $5d,
  .db $f1, $62, $f1, $62, $f1, $62, $f1, $62, $d9, $e1, $67, $ec, $01, $f4, $b5, $f5,
  .db $e0, $56, $ec, $00, $d9, $f1, $62, $f1, $60, $f3, $59, $f1, $59, $d9, $f1, $5e,
  .db $f1, $62, $f1, $65, $d9,

