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
  .db $0a, $23, $20, $25, $55, $27, $ab, $29, $26, $2c, $c6, $2e, $8e, $31, $80, $34,
  .db $9f, $37, $ee, $3a, $6f, $3e, $25, $42, $16, $02, $35, $02, $56, $02, $7b, $02,
  .db $a0, $02, $c8, $02, $f2, $02, $1f, $03, $4f, $03, $81, $03, $b6, $03, $ef, $03,

songPitch3:
  .db $9f, $37, $ee, $3a, $6f, $3e, $25, $42, $14, $46, $3f, $4a, $a9, $4e, $57, $53,
  .db $4b, $58, $8b, $5d, $1b, $63, $00, $69, $4f, $03, $81, $03, $b6, $03, $ef, $03,
  .db $2b, $04, $6a, $04, $ae, $04, $f4, $04, $40, $05, $90, $05, $e5, $05, $3e, $06,

songPitch4:
  .db $81, $34, $a0, $37, $ef, $3a, $70, $3e, $27, $42, $16, $46, $41, $4a, $ab, $4e,
  .db $59, $53, $4d, $58, $8d, $5d, $1e, $63, $1f, $03, $4f, $03, $81, $03, $b7, $03,
  .db $ef, $03, $2b, $04, $6a, $04, $ae, $04, $f4, $04, $40, $05, $91, $05, $e4, $05,

songPitch5:
  .db $13, $21, $0a, $23, $20, $25, $55, $27, $ac, $29, $26, $2c, $c6, $2e, $8e, $31,
  .db $80, $34, $9f, $37, $ee, $3a, $6f, $3e, $f7, $01, $16, $02, $35, $02, $57, $02,
  .db $7a, $02, $a0, $02, $c8, $02, $f2, $02, $1f, $03, $4f, $03, $81, $03, $b7, $03,

songPitch6:
  .db $12, $21, $0a, $23, $1f, $25, $54, $27, $ab, $29, $25, $2c, $c5, $2e, $8d, $31,
  .db $7f, $34, $9f, $37, $ed, $3a, $6e, $3e, $f8, $01, $15, $02, $35, $02, $57, $02,
  .db $7a, $02, $a0, $02, $c8, $02, $f2, $02, $20, $03, $4e, $03, $81, $03, $b7, $03,

songPitch7:
  .db $4e, $32, $4c, $35, $77, $38, $d2, $3b, $61, $3f, $26, $43, $24, $47, $5f, $4b,
  .db $da, $4f, $9a, $54, $a2, $59, $f6, $5e, $fe, $02, $2b, $03, $5b, $03, $8f, $03,
  .db $c5, $03, $fe, $03, $3b, $04, $7b, $04, $c0, $04, $08, $05, $54, $05, $a6, $05,

songPitch8:
  .db $9b, $3b, $27, $3f, $e8, $42, $e2, $46, $19, $4b, $91, $4f, $4c, $54, $4f, $59,
  .db $9e, $5e, $3f, $64, $35, $6a, $86, $70, $8c, $03, $c1, $03, $fa, $03, $37, $04,
  .db $78, $04, $bb, $04, $03, $05, $4f, $05, $a1, $05, $f6, $05, $51, $06, $b0, $06,

songPitch9:
  .db $c3, $2a, $4d, $2d, $ff, $2f, $da, $32, $e0, $35, $14, $39, $79, $3c, $11, $40,
  .db $e1, $43, $ea, $47, $31, $4c, $b8, $50, $8a, $02, $b2, $02, $db, $02, $06, $03,
  .db $34, $03, $65, $03, $98, $03, $d0, $03, $09, $04, $47, $04, $87, $04, $cd, $04,

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
  .db <songPitch7
  .db <songPitch8
  .db <songPitch9

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
  .db >songPitch7
  .db >songPitch8
  .db >songPitch9

songShiftList0:
  .db $00, $0c, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00,
  .db $0c, $0a, $0b, $0d, $0b, $0c, $0b, $0b, $0b,

songSampleStartLow0:
  .db $a0, $7e, $6a, $e5, $1d, $29, $6f, $a9,

songSampleStartHigh0:
  .db $08, $15, $1a, $1d, $22, $3a, $3f, $47,

songSampleLoopLow0:
  .db $7d, $69, $e4, $d5, $2b, $54, $89, $55,

songSampleLoopHigh0:
  .db $15, $1a, $1d, $21, $23, $3f, $46, $4a,

songInitState0:
  .db $40, $40, $00, $c0, $3f, $40, $7f, $00, $00, $00, $00, $00, $00, $00, $03, $7f,
  .db $7f, $10, $f8, $00,

songIns0:
  .db $ff, $e0, $7f, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00,
  .db $00, $00, $00,

songIns1:
  .db $ff, $e0, $7f, $00, $00, $01, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00,
  .db $00, $00, $00,

songIns2:
  .db $ff, $e0, $7f, $00, $00, $02, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00,
  .db $00, $00, $00,

songIns3:
  .db $8f, $12, $7f, $01, $00, $03, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00,
  .db $00, $00, $00,

songMacro0:
  .db $05, $00, $00, $02, $80,

songIns4:
  .db $ff, $eb, $7f, $00, $00, $04, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00,
  .db $00,
  .dw songMacro0
  .db $00, $00,

songMacro1:
  .db $05, $00, $00, $02, $80,

songIns5:
  .db $ff, $ec, $7f, $00, $00, $05, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00,
  .db $00,
  .dw songMacro1
  .db $00, $00,

songMacro2:
  .db $05, $00, $00, $02, $80,

songIns6:
  .db $ff, $ee, $7f, $00, $00, $06, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00,
  .db $00,
  .dw songMacro2
  .db $00, $00,

songMacro3:
  .db $05, $00, $00, $12, $80,

songIns7:
  .db $ff, $ed, $7f, $00, $00, $07, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00,
  .db $00,
  .dw songMacro3
  .db $00, $00,

songInsListLow0:
  .db <songIns0
  .db <songIns1
  .db <songIns2
  .db <songIns3
  .db <songIns4
  .db <songIns5
  .db <songIns6
  .db <songIns7

songInsListHigh0:
  .db >songIns0
  .db >songIns1
  .db >songIns2
  .db >songIns3
  .db >songIns4
  .db >songIns5
  .db >songIns6
  .db >songIns7

songCmdStream0:
  .db $46, $43, $53, $00, $08, $00, $00, $00, $06, $0c, $03, $12, $60, $18, $c0, $09,
  .db $24, $42, $48, $9c, $1e, $30, $78, $90, $01, $02, $04, $00, $05, $03, $7f, $2f,
  .db $5f, $16, $3a, $7a, $00, $00, $00, $00,
  .dw (songCmdStream0+64)
  .dw (songCmdStream0+145)
  .dw (songCmdStream0+297)
  .dw (songCmdStream0+575)
  .dw (songCmdStream0+685)
  .dw (songCmdStream0+734)
  .dw (songCmdStream0+785)
  .dw (songCmdStream0+861)
  .db $07, $08, $02, $02, $01, $01, $01, $02, $e3,
  .dw (songCmdStream0+1389)
  .db $05, $f8, $5f, $f5, $5f, $f5, $5f, $f1,
  .dw (songCmdStream0+1389)
  .db $05, $dd, $36, $e0, $c7, $4f, $63, $f0, $e3, $e6, $5f, $f5, $5f, $f1,
  .dw (songCmdStream0+1148)
  .db $04, $f5, $5f, $f5, $5f,
  .dw (songCmdStream0+1588)
  .db $06, $fd, $e0, $e7, $63, $f7, $eb, $63, $f2, $e6, $63, $f0, $c7, $4f, $63, $f0,
  .db $e6, $63, $f1, $e3, $e6, $5f, $f1,
  .dw (songCmdStream0+1148)
  .db $04,
  .dw (songCmdStream0+1352)
  .db $05,
  .dw (songCmdStream0+1274)
  .db $04, $5f,
  .dw (songCmdStream0+1352)
  .db $05, $5f,
  .dw (songCmdStream0+1352)
  .db $05,
  .dd (songCmdStream0+91)
  .db $00, $e1, $e7, $cf, $ff, $c0,
  .dw (songCmdStream0+1124)
  .db $04,
  .dw (songCmdStream0+1459)
  .db $05,
  .dw (songCmdStream0+1124)
  .db $04, $e0, $e7,
  .dw (songCmdStream0+1408)
  .db $05, $63, $f0,
  .dw (songCmdStream0+1297)
  .db $05,
  .dw (songCmdStream0+1134)
  .db $04,
  .dw (songCmdStream0+1330)
  .db $05,
  .dw (songCmdStream0+1797)
  .db $07,
  .dw (songCmdStream0+1134)
  .db $04,
  .dw (songCmdStream0+1484)
  .db $05, $cf, $fe, $ff,
  .dw (songCmdStream0+1523)
  .db $05,
  .dw (songCmdStream0+1523)
  .db $05,
  .dw (songCmdStream0+1779)
  .db $06,
  .dw (songCmdStream0+1451)
  .db $05,
  .dw (songCmdStream0+1779)
  .db $06,
  .dw (songCmdStream0+1365)
  .db $05,
  .dw (songCmdStream0+915)
  .db $03, $e9, $7c, $f0, $e6,
  .dw (songCmdStream0+1312)
  .db $05,
  .dw (songCmdStream0+1594)
  .db $06, $e0, $e7, $cf, $fe, $ff, $63, $f0, $e6,
  .dw (songCmdStream0+1744)
  .db $06,
  .dw (songCmdStream0+1654)
  .db $06,
  .dw (songCmdStream0+915)
  .db $03, $e0, $c7, $4f, $cf, $fe, $ff, $63, $f0, $e1, $e6, $cf, $ff, $c0,
  .dw (songCmdStream0+1678)
  .db $06, $7c, $f0, $e7, $7c, $f0, $e9,
  .dw (songCmdStream0+1428)
  .db $05, $e0, $e6,
  .dw (songCmdStream0+1408)
  .db $05,
  .dw (songCmdStream0+1660)
  .db $06, $c7, $4f, $63, $f0, $e6,
  .dw (songCmdStream0+1490)
  .db $05,
  .dw (songCmdStream0+1684)
  .db $06,
  .dw (songCmdStream0+1490)
  .db $05,
  .dw (songCmdStream0+1816)
  .db $07, $e7, $cf, $ff, $c0,
  .dw (songCmdStream0+1428)
  .db $05,
  .dd (songCmdStream0+166)
  .db $00, $e5, $e6,
  .dw (songCmdStream0+1698)
  .db $06, $f2, $61, $f2, $b5, $f7,
  .dw (songCmdStream0+1626)
  .db $06, $f0, $5f, $f3, $5c, $f1, $b5, $f0, $5c, $f1, $5f, $f1, $61, $f0, $b5, $f0,
  .dw (songCmdStream0+1466)
  .db $05, $f0, $b5, $f0, $5a, $fc, $5a, $f0, $b5, $f0, $5a, $f1, $5c, $f3, $5c, $f2,
  .db $b5, $f2, $5d,
  .dw (songCmdStream0+1805)
  .db $07, $64, $f1,
  .dw (songCmdStream0+1698)
  .db $06, $f7, $61, $f2, $b5, $f2,
  .dw (songCmdStream0+1626)
  .db $06, $f3, $5c, $f3, $b5, $f0, $5c, $f0, $5f, $f2, $b5, $f7, $61, $f3,
  .dw (songCmdStream0+1466)
  .db $05, $f2, $b5, $f2, $5a, $f3, $5a,
  .dw (songCmdStream0+1823)
  .db $07, $f0,
  .dw (songCmdStream0+1828)
  .db $07, $5f, $f3, $5c, $f1, $5f, $f0, $5c, $f2, $b5, $f7, $5c, $f0, $64, $f0, $5f,
  .db $f0, $b5, $f0, $5c, $f0,
  .dw (songCmdStream0+935)
  .db $03, $5c,
  .dw (songCmdStream0+935)
  .db $03, $5d, $f3, $5d, $f2, $b5, $f2, $5d, $f0, $5d, $f0, $b5, $f0, $5d, $f3, $64,
  .db $f0, $b5, $f0, $64,
  .dw (songCmdStream0+1805)
  .db $07, $5d, $f3, $5d, $f0, $b5, $f1, $5d, $f1, $64, $f0, $b5, $f1, $66, $f3, $63,
  .db $f0, $b5, $f0, $5c, $f3, $5c, $f0, $b5, $f1,
  .dw (songCmdStream0+1828)
  .db $07, $5c, $f3, $5f, $f0, $b5, $f0, $5c, $f0, $63, $f5,
  .dw (songCmdStream0+1828)
  .db $07, $5c, $f1,
  .dw (songCmdStream0+1828)
  .db $07, $5c, $f0,
  .dw (songCmdStream0+1828)
  .db $07, $5a, $f3, $63, $f0, $b5, $f0, $5d, $f1, $5d, $f0, $b5, $f3, $5d, $f3, $b5,
  .db $f0, $64, $f0, $b5, $f0, $68, $f1, $5d, $f1, $b5, $f1, $5d, $f5, $64, $f0, $b5,
  .db $f3, $66, $f5, $5d, $f1, $5c, $f3, $5c, $f2, $b5, $f2, $5f, $f3, $5f,
  .dw (songCmdStream0+1823)
  .db $07, $f1, $5f, $f2, $b5, $f7, $60, $f1, $63, $f5, $c7, $4f, $68, $f0, $b5, $f0,
  .db $e6, $68, $f5, $68, $f2, $b5, $f2, $68, $f2, $b5, $f7, $66, $f2, $b5, $f2, $64,
  .db $f5,
  .dd (songCmdStream0+411)
  .db $00, $e2, $ea, $7b, $f6, $74, $f6, $7b, $f6, $74, $f6, $b5, $f8, $e4, $ea,
  .dw (songCmdStream0+1166)
  .db $04, $dd, $54,
  .dw (songCmdStream0+1166)
  .db $04, $f1, $b8, $06, $c7, $36,
  .dw (songCmdStream0+1438)
  .db $05, $73, $f2, $74, $f0, $b5, $f2, $72, $f7, $b5, $f2, $6d, $f7, $b5, $f2, $70,
  .db $f5, $72, $f5, $6f, $f5, $6d, $f0, $b5, $f0, $6d, $f1, $b5, $f1, $6a, $f2, $6b,
  .db $f2, $b5, $f0, $6d, $fa, $b5, $f1,
  .dw (songCmdStream0+1438)
  .db $05,
  .dw (songCmdStream0+1636)
  .db $06, $f7, $b5, $f2, $74, $f5, $72, $f0, $b5, $f3, $77, $f5, $75, $f0, $b5, $f3,
  .db $74, $f3, $b5, $f0, $72, $f0, $b5, $f0, $72, $f5, $74, $f0, $b5, $f0, $74, $fa,
  .dd (songCmdStream0+585)
  .db $00, $e2, $c7, $32, $cf, $7e, $ff, $70, $f6, $69, $f6, $70, $f6, $69, $f6, $e7,
  .db $cf, $fe, $ff,
  .dw (songCmdStream0+1546)
  .db $06, $ff, $7b, $f8, $7c, $f1, $e7,
  .dw (songCmdStream0+1546)
  .db $06, $dd, $a8, $77, $f5, $79, $f6, $79, $f6, $7c, $f6, $79, $f4, $7b, $f4,
  .dd (songCmdStream0+699)
  .db $00, $e2, $c7, $32, $cf, $ff, $80, $6b, $f6, $64, $f6, $6d, $f4, $6b, $dc, $20,
  .db $01, $e7, $cf, $7e, $ff,
  .dw (songCmdStream0+1560)
  .db $06, $ff, $c7, $12, $79, $fd, $e7,
  .dw (songCmdStream0+1560)
  .db $06, $f6, $cf, $fe, $ff, $69, $f6, $68, $f6, $66, $f6, $68, $f4, $6c, $f4,
  .dd (songCmdStream0+750)
  .db $00, $e2, $e7, $61, $f6, $5a, $f6, $61, $f6, $64, $f6, $e7, $cf, $ff, $80,
  .dw (songCmdStream0+1574)
  .db $06, $ff, $c7, $1f, $77, $fd, $e7,
  .dw (songCmdStream0+1574)
  .db $06, $f6, $e4, $c7, $14, $cf, $ff, $80, $74, $f4, $77, $f4, $cf, $7e, $ff, $76,
  .db $f4, $72, $f4, $cf, $ff, $80, $74, $f4, $c9, $79, $04, $c0, $80, $78, $dd, $07,
  .db $c9, $79, $00, $ca, $79, $dd, $59, $cf, $7e, $ff, $7c, $f4, $7b, $f4,
  .dd (songCmdStream0+795)
  .db $00, $f8, $b8, $07, $c7, $7c,
  .dw (songCmdStream0+1787)
  .db $06, $f6,
  .dw (songCmdStream0+1787)
  .db $06, $fb, $f8,
  .dw (songCmdStream0+1611)
  .db $06, $f4, $cf, $fe, $ff, $5f, $dd, $e4,
  .dw (songCmdStream0+1611)
  .db $06, $fe, $cf, $fe, $ff, $5f, $dd, $a8,
  .dw (songCmdStream0+1787)
  .db $06, $dd, $e4,
  .dw (songCmdStream0+1833)
  .db $07, $66, $f4, $cf, $fe, $ff, $63, $f4,
  .dd (songCmdStream0+874)
  .db $00,
  .dw (songCmdStream0+1678)
  .db $06,
  .dw (songCmdStream0+1159)
  .db $04, $e9, $7c, $f0, $e6,
  .dw (songCmdStream0+1159)
  .db $04,
  .dw (songCmdStream0+1750)
  .db $06,
  .dw (songCmdStream0+1159)
  .db $04, $d9, $f0, $61, $f0, $61, $f0, $b5, $f0, $64, $f1, $64, $f2, $b5, $f2, $61,
  .db $f1, $61, $f2, $b5, $f2, $68, $f1, $66, $f1, $5f, $f1, $5d, $f1, $5d, $f2, $b5,
  .db $f2, $64, $f1, $64, $f2, $b5, $f2, $5d, $f1, $5d, $f2, $b5, $f2, $5d, $f2, $b5,
  .db $f7, $66, $f1, $64, $f0, $b5, $f0, $61, $f0,
  .dw (songCmdStream0+1765)
  .db $06, $63, $f1, $63, $f2, $b5, $f2,
  .dw (songCmdStream0+1765)
  .db $06, $5f,
  .dw (songCmdStream0+1823)
  .db $07, $f1, $66,
  .dw (songCmdStream0+1805)
  .db $07, $5c, $f1, $5c,
  .dw (songCmdStream0+1823)
  .db $07,
  .dw (songCmdStream0+1823)
  .db $07,
  .dw (songCmdStream0+1805)
  .db $07, $63, $f1, $63,
  .dw (songCmdStream0+1823)
  .db $07, $f1, $66, $f0, $63, $dd, $0f, $b5, $f2, $5d, $f1, $5d, $f0, $b5, $f0, $61,
  .db $f2, $b5, $f2, $61, $f1, $b5, $f0, $61, $f1, $5d, $f2, $b5, $f2, $64, $f1, $5d,
  .db $f1,
  .dw (songCmdStream0+1765)
  .db $06, $63, $f3, $5f, $f0, $b5, $f0, $66, $f0, $b5, $f0, $5f, $f1, $68, $f0, $b5,
  .db $f0, $5f, $f5, $5f, $f2, $b5, $f2, $61, $f9,
  .dw (songCmdStream0+1828)
  .db $07, $64, $f2, $b5, $f2, $63, $f0, $b5, $f0, $61, $f0, $b5, $f0, $5f, $f0, $61,
  .db $f1, $5f, $f0, $5d, $f0, $5c, $f0, $5a, $f0, $5c, $f0, $5f, $f0, $64, $f0, $64,
  .db $f0, $d9,
  .dw (songCmdStream0+1284)
  .db $05,
  .dw (songCmdStream0+1459)
  .db $05,
  .dw (songCmdStream0+1284)
  .db $05, $d9, $63, $f0,
  .dw (songCmdStream0+1297)
  .db $05, $63, $f0,
  .dw (songCmdStream0+1297)
  .db $05,
  .dw (songCmdStream0+1744)
  .db $06, $d9,
  .dw (songCmdStream0+1274)
  .db $04, $e6, $5f,
  .dw (songCmdStream0+1352)
  .db $05, $e6, $5f, $d9,
  .dw (songCmdStream0+1312)
  .db $05,
  .dw (songCmdStream0+1375)
  .db $05, $d9, $6d, $f8, $74, $f0, $b5, $f0, $c7, $1f, $6d, $f1, $ea, $77, $f5, $75,
  .db $f0, $c7, $1f, $77,
  .dw (songCmdStream0+1734)
  .db $06, $f3, $ea, $74, $fa, $ea, $72, $f1, $70, $f1, $72, $f1, $70, $f5, $6f, $f1,
  .db $6d, $f1, $c7, $1f, $6f, $f1, $ea, $6b, $f0, $b5, $f0, $c7, $1f, $6d, $f0, $c7,
  .db $14, $6b, $f0, $ea, $6d, $dd, $3c, $70, $f0, $c7, $1f, $6d, $f3, $ea, $72, $f1,
  .db $c7, $1f, $70, $f0, $b5,
  .dw (songCmdStream0+1734)
  .db $06, $f0, $ea, $6f, $f1, $c7, $1f, $72, $f1, $ea, $6b, $f1, $68, $f2, $b5, $f7,
  .db $6b, $f1, $c7, $1f, $68, $f2, $b5, $f7, $ea, $6d, $f0, $b5, $f0, $6d, $fe, $b5,
  .db $d9, $5f,
  .dw (songCmdStream0+1352)
  .db $05, $e6, $5f,
  .dw (songCmdStream0+1356)
  .db $05, $d9,
  .dw (songCmdStream0+1365)
  .db $05, $7c, $f0, $e9,
  .dw (songCmdStream0+1772)
  .db $06,
  .dw (songCmdStream0+1399)
  .db $05, $d9,
  .dw (songCmdStream0+1811)
  .db $07,
  .dw (songCmdStream0+1330)
  .db $05, $e9, $7c, $f0, $e0, $e6, $cf, $fe, $ff, $d9, $7c, $f0,
  .dw (songCmdStream0+1816)
  .db $07,
  .dw (songCmdStream0+1708)
  .db $06,
  .dw (songCmdStream0+1375)
  .db $05,
  .dw (songCmdStream0+1500)
  .db $05, $e9, $7c, $f0, $d9, $e6, $7c, $f0, $e7,
  .dw (songCmdStream0+1535)
  .db $05,
  .dw (songCmdStream0+1535)
  .db $05, $7c, $f0, $e6, $7c, $f0,
  .dw (songCmdStream0+1484)
  .db $05,
  .dw (songCmdStream0+1811)
  .db $07, $d9,
  .dw (songCmdStream0+1356)
  .db $05, $d9,
  .dw (songCmdStream0+1421)
  .db $05, $e6, $5f,
  .dw (songCmdStream0+1421)
  .db $05, $d9,
  .dw (songCmdStream0+1399)
  .db $05, $e7, $7c, $f0,
  .dw (songCmdStream0+1772)
  .db $06, $d9,
  .dw (songCmdStream0+1728)
  .db $06, $7c, $f0,
  .dw (songCmdStream0+1500)
  .db $05,
  .dw (songCmdStream0+1654)
  .db $06, $7c, $f0, $d9,
  .dw (songCmdStream0+1648)
  .db $06,
  .dw (songCmdStream0+1718)
  .db $06,
  .dw (songCmdStream0+1648)
  .db $06, $d9,
  .dw (songCmdStream0+1507)
  .db $05, $7c, $f0,
  .dw (songCmdStream0+1601)
  .db $06, $d9, $cf, $fe, $ff, $63, $f0, $e1, $cf, $ff, $c0,
  .dw (songCmdStream0+1451)
  .db $05, $d9,
  .dw (songCmdStream0+1588)
  .db $06,
  .dw (songCmdStream0+1667)
  .db $06, $d9,
  .dw (songCmdStream0+1451)
  .db $05, $e1, $cf, $ff, $c0, $7c, $f0, $d9,
  .dw (songCmdStream0+1636)
  .db $06, $f1, $74, $f5, $72, $f0, $b5, $f1, $6d, $f9, $d9, $7c, $f0, $e0, $e6,
  .dw (songCmdStream0+1514)
  .db $05, $d9, $e7, $7c, $f0,
  .dw (songCmdStream0+1507)
  .db $05, $d9, $5a, $f5, $5a, $f0, $b5, $f0, $5a, $f0, $b5, $f0, $5a, $f2, $b5, $f2,
  .db $5a, $f1, $5a, $d9, $e0, $eb,
  .dw (songCmdStream0+1514)
  .db $05, $d9, $63, $f0, $e1,
  .dw (songCmdStream0+1684)
  .db $06, $63, $f0, $e1, $d9,
  .dw (songCmdStream0+1601)
  .db $06, $e7, $7c, $f0, $d9, $7c, $f0,
  .dw (songCmdStream0+1594)
  .db $06, $e7, $d9, $cf, $fe, $ff, $63, $f0,
  .dw (songCmdStream0+1660)
  .db $06, $d9,
  .dw (songCmdStream0+1744)
  .db $06, $e0, $eb, $cf, $fe, $ff, $63, $f0, $e6, $d9, $7c, $f0, $e9, $7c, $f0, $e0,
  .db $eb,
  .dw (songCmdStream0+1758)
  .db $06, $d9, $7b, $f4, $7c, $f4, $7e, $f4, $7e, $f4, $79, $f4, $7e, $f4, $79, $d9,
  .db $70, $f4, $74, $f4, $7b, $f4, $77, $f4, $74, $f4, $77, $f4, $70, $d9, $6b, $f4,
  .db $6d, $f4, $74, $f4, $72, $f4, $69, $f4, $6f, $f4, $6d, $d9,
  .dw (songCmdStream0+1667)
  .db $06, $e6, $5f, $d9,
  .dw (songCmdStream0+1708)
  .db $06, $e6, $7c, $f0, $d9, $e9,
  .dw (songCmdStream0+1772)
  .db $06, $7c, $f0, $e6, $7c, $f0, $d9,
  .dw (songCmdStream0+1833)
  .db $07, $5f, $f6, $cf, $9e, $ff, $64, $f4, $cf, $ff, $a0, $63, $d9, $5c,
  .dw (songCmdStream0+1805)
  .db $07, $61, $fc, $5f, $f0, $b5, $d9, $6c, $f2, $6d, $f0, $b5, $f2, $70, $f7, $b5,
  .db $f2, $72, $d9, $5f,
  .dw (songCmdStream0+1718)
  .db $06, $5f, $d9, $7c, $f0,
  .dw (songCmdStream0+1728)
  .db $06, $d9,
  .dw (songCmdStream0+1811)
  .db $07,
  .dw (songCmdStream0+1797)
  .db $07, $d9, $f3, $e8, $5f, $f0, $e6, $5f, $f1, $e8, $5f, $f1, $d9, $7c, $f0,
  .dw (songCmdStream0+1750)
  .db $06, $d9, $cf, $ff, $c0, $7c, $f0, $e7, $7c, $f0, $e0, $e6, $cf, $fe, $ff, $d9,
  .db $61, $f5, $61, $f0, $b5, $f0, $61, $f2, $b5, $d9, $e6, $7c, $f0, $e7, $7c, $f0,
  .db $e9, $7c, $f0, $d9, $f5, $5f, $f5, $5f, $f5, $5f, $f1, $5f, $f1, $d9, $e0, $e6,
  .dw (songCmdStream0+1758)
  .db $06, $d9, $f0, $ea, $74, $f5, $72, $f0, $c7, $1f, $74, $d9,
  .dw (songCmdStream0+1779)
  .db $06, $7c, $f0, $d9,
  .dw (songCmdStream0+1797)
  .db $07, $e1, $cf, $ff, $c0, $d9, $cf, $fe, $ff,
  .dw (songCmdStream0+1779)
  .db $06, $d9, $5f, $f1, $5f, $f2, $b5, $f2, $d9, $7c, $f0, $e6, $7c, $f0, $e7, $d9,
  .db $63, $f0, $e1, $e7, $cf, $ff, $c0, $d9, $cf, $9e, $ff, $64, $f6, $cf, $ff, $a0,
  .db $5f, $d9, $e0, $e6, $cf, $fe, $ff, $63, $f0, $d9, $f1, $5f, $f0, $b5, $f0, $d9,
  .db $e1,
  .dw (songCmdStream0+1816)
  .db $07, $d9, $e7, $cf, $ff, $c0, $7c, $f0, $d9, $f2, $b5, $f2, $5c, $d9, $5c, $f0,
  .db $b5, $f0, $d9, $cf, $9e, $ff, $64, $fb, $cf, $ff, $a0, $d9,

