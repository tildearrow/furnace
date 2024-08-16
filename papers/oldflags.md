# converting from old flags

prior to format version 119, chip flags were stored as a 32-bit integer.
this section will help you understand the old flag format.

chips which aren't on this list don't have any flags or have been added after version 119.

## 0x02: Genesis (COMPOUND) and 0x42: Genesis extended (COMPOUND)

- bit 31: ladderEffect (bool)
- bit 0-30: clockSel (int)
  - 0: NTSC
  - 1: PAL
  - 2: 8MHz
  - 3: Firecore (`COLOR_NTSC*12/7`)
  - 4: System 32 (`COLOR_NTSC*9/4`)
  - only 0 and 1 apply to the SN part as well.

## 0x03: SMS (SN76489)

- flags AND 0xff03: clockSel (int)
  - 0x0000: NTSC (becomes 0)
  - 0x0001: PAL (becomes 1)
  - 0x0002: 4MHz (becomes 2)
  - 0x0003: half NTSC (becomes 3)
  - 0x0100: 3MHz (becomes 4)
  - 0x0101: 2MHz (becomes 5)
  - 0x0102: eighth NTSC (becomes 6)
- flags AND 0xcc: chipType (int)
  - 0x00: Sega PSG (becomes 0)
  - 0x04: TI SN76489 (becomes 1)
  - 0x08: SN with Atari-like short noise (becomes 2)
  - 0x0c: Game Gear (becomes 3)
  - 0x40: TI SN76489A (becomes 4)
  - 0x44: TI SN76496 (becomes 5)
  - 0x48: NCR 8496 (becomes 6)
  - 0x4c: Tandy PSSJ 3-voice sound (becomes 7)
  - 0x80: TI SN94624 (becomes 8)
  - 0x84: TI SN76494 (becomes 9)
- bit 4: noPhaseReset (bool)

## 0x04: Game Boy

- bits 0-1: chipType (int)
  - 0: DMG (rev B)
  - 1: CGB (rev C)
  - 2: CGB (rev E)
  - 3: AGB
- bit 3: noAntiClick (bool)

## 0x05: PC Engine

- bit 0: clockSel (int)
  - 0: NTSC
  - 1: pseudo-PAL
- bit 2: chipType (int)
  - 0: HuC6280
  - 1: HuC6280A
- bit 3: noAntiClick (bool)

## 0x06: NES, 0x88: VRC6, 0x8a: FDS and 0x8b: MMC5

- flags: clockSel (int)
  - 0: NTSC (2A03)
  - 1: PAL (2A07)
  - 2: Dendy

## 0x07: C64 (8580) and 0x47: C64 (6581)

- bit 0-3: clockSel (int)
  - 0: NTSC
  - 1: PAL
  - 2: SSI 2001

## 0x08: Arcade (YM2151+SegaPCM; COMPOUND)

- bit 0-7: clockSel (int)
  - 0: NTSC
  - 1: PAL
  - 2: 4MHz
  - this clock only applies to the YM2151.

## 0x09: Neo Geo CD (YM2610), 0xa5: Neo Geo (YM2610), 0xa6: Neo Geo extended (YM2610), 0x49: Neo Geo CD extended, 0x9e: YM2610B and 0xde: YM2610B extended

- bit 0-7: clockSel (int)
  - 0: 8MHz
  - 1: 8.06MHz (Neo Geo AES)

## 0x80: AY-3-8910

- bit 0-3: clockSel (int)
  - 0: NTSC
  - 1: PAL
  - 2: ZX Spectrum 48K (1.75MHz)
  - 3: 2MHz
  - 4: 1.5MHz
  - 5: 1MHz
  - 6: Sunsoft 5B
  - 7: PAL NES
  - 8: Sunsoft 5B on PAL NES
  - 9: 1.10MHz
  - 10: 2^21MHz
  - 11: double NTSC
  - 12: 3.6MHz
  - 13: 1.25MHz
  - 14: 1.536MHz
- bit 4-5: chipType (int)
  - 0: AY-3-8910
  - 1: YM2149(F)
  - 2: Sunsoft 5B
  - 3: AY-3-8914
- bit 6: stereo (bool)
- bit 7: halfClock (bool)
- bit 8-15: stereoSep (int)

## 0x81: Amiga

- bit 0: clockSel (int)
  - 0: NTSC
  - 1: PAL
- bit 1: chipType (int)
  - 0: Amiga 500
  - 1: Amiga 1200
- bit 2: bypassLimits (bool)
- bit 8-14: stereoSep (int)

## 0x82: YM2151 alone

- bit 0-7: clockSel (int)
  - 0: NTSC
  - 1: PAL
  - 2: 4MHz

## 0x83: YM2612 alone, 0xa0: YM2612 extended, 0xbd: YM2612 extra features extended and 0xbe: YM2612 extra features

- bit 31: ladderEffect (bool)
- bit 0-30: clockSel (int)
  - 0: NTSC
  - 1: PAL
  - 2: 8MHz
  - 3: Firecore (`COLOR_NTSC*12/7`)
  - 4: System 32 (`COLOR_NTSC*9/4`)

## 0x84: TIA

- bit 0: clockSel (int)
  - 0: NTSC
  - 1: PAL
- bit 1-2: mixingType (int)
  - 0: mono
  - 1: mono (no distortion)
  - 2: stereo

## 0x85: VIC-20

- bit 0: clockSel (int)
  - 0: NTSC
  - 1: PAL

## 0x87: SNES

- bit 0-6: volScaleL (int)
- bit 8-14: volScaleR (int)

## 0x89: OPLL (YM2413) and 0xa7: OPLL drums (YM2413)

- bit 0-3: clockSel (int)
  - 0: NTSC
  - 1: PAL
  - 2: 4MHz
  - 3: half NTSC
- bit 4-31: patchSet (int)
  - 0: YM2413
  - 1: YMF281
  - 2: YM2423
  - 3: VRC7

## 0x8c: Namco 163

- bit 0-3: clockSel (int)
  - 0: NTSC (2A03)
  - 1: PAL (2A07)
  - 2: Dendy
- bit 4-6: channels (int)
- bit 7: multiplex (bool)

## 0x8d: YM2203 and 0xb6: YM2203 extended

- bit 0-4: clockSel (int)
  - 0: NTSC
  - 1: PAL
  - 2: 4MHz
  - 3: 3MHz
  - 4: 3.99MHz
  - 5: 1.5MHz
- bit 5-6: prescale (int)
  - 0: /6
  - 1: /3
  - 2: /2

## 0x8e: YM2608 and 0xb7: YM2608 extended

- bit 0-4: clockSel (int)
  - 0: 8MHz
  - 1: 7.98MHz
- bit 5-6: prescale (int)
  - 0: /6
  - 1: /3
  - 2: /2

## 0x8f: OPL (YM3526), 0xa2: OPL drums (YM3526), 0x90: OPL2 (YM3812), 0xa3: OPL2 drums (YM3812), 0xb2: Yamaha Y8950 and 0xb3: Yamaha Y8950 drums

- bit 0-7: clockSel (int)
  - 0: NTSC
  - 1: PAL
  - 2: 4MHz
  - 3: 3MHz
  - 4: 3.99MHz
  - 5: 3.5MHz

## 0x91: OPL3 (YMF262) and 0xa4: OPL3 drums (YMF262)

- bit 0-7: clockSel (int)
  - 0: NTSC
  - 1: PAL
  - 2: 14MHz
  - 3: 16MHz
  - 4: 15MHz

## 0x93: Intel 8253 (beeper)

- bit 0-1: speakerType (int)
  - 0: unfiltered
  - 1: cone
  - 2: piezo
  - 3: system

## 0x95: RF5C68

- bit 0-3: clockSel (int)
  - 0: 8MHz
  - 1: 10MHz
  - 2: 12.5MHz
- bit 4-31: chipType (int)
  - 0: RF5C68
  - 1: RF5C164

## 0x97: Philips SAA1099

- flags: clockSel (int)
  - 0: 8MHz
  - 1: NTSC
  - 2: PAL

## 0x98: OPZ (YM2414)

- flags: clockSel (int)
  - 0: NTSC
  - 1: pseudo-PAL
  - 2: 4MHz

## 0x9a: AY8930

- bit 0-3: clockSel (int)
  - 0: NTSC
  - 1: PAL
  - 2: ZX Spectrum 48K (1.75MHz)
  - 3: 2MHz
  - 4: 1.5MHz
  - 5: 1MHz
  - 6: Sunsoft 5B
  - 7: PAL NES
  - 8: Sunsoft 5B on PAL NES
  - 9: 1.10MHz
  - 10: 2^21MHz
  - 11: double NTSC
  - 12: 3.6MHz
- bit 6: stereo (bool)
- bit 7: halfClock (bool)
- bit 8-15: stereoSep (int)

## 0x9d: VRC7

- bit 0-3: clockSel (int)
  - 0: NTSC
  - 1: PAL
  - 2: 4MHz
  - 3: half NTSC

## 0x9f: ZX Spectrum (beeper)

- bit 0-1: clockSel (int)
  - 0: NTSC
  - 1: PAL

## 0xa1: Konami SCC and 0xb4: Konami SCC+

- bit 0-6: clockSel (int)
  - 0: NTSC
  - 1: PAL
  - 2: 1.5MHz
  - 3: 2MHz

## 0xaa: MSM6295

- bit 0-6: clockSel (int)
  - 0: 1MHz
  - 1: 1.056MHz
  - 2: 4MHz
  - 3: 4.224MHz
  - 4: NTSC
  - 5: half NTSC
  - 6: 2/7 NTSC
  - 7: quarter NTSC
  - 8: 2MHz
  - 9: 2.112MHz
  - 10: 875KHz
  - 11: 937.5KHz
  - 12: 1.5MHz
  - 13: 3MHz
  - 14: 1/3 NTSC
- bit 7: rateSel (bool)

## 0xab: MSM6258

- flags: clockSel (int)
  - 0: 4MHz
  - 1: 4.096MHz
  - 2: 8MHz
  - 3: 8.192MHz

## 0xae: OPL4 (YMF278B) and 0xaf: OPL4 drums (YMF278B)

- bit 0-7: clockSel (int)
  - 0: NTSC
  - 1: PAL
  - 2: 33.8688MHz

## 0xb0: Seta/Allumer X1-010

- bit 0-3: clockSel (int)
  - 0: 16MHz
  - 1: 16.67MHz
- bit 4: stereo (bool)

## 0xb1: Ensoniq ES5506

- bit 0-4: channels (int)

## 0xb5: tildearrow Sound Unit

- bit 0: clockSel (int)
  - 0: NTSC
  - 1: PAL
- bit 2: echo (bool)
- bit 3: swapEcho (bool)
- bit 4: sampleMemSize (int)
  - 0: 8K
  - 1: 64K
- bit 5: pdm (bool)
- bit 8-13: echoDelay (int)
- bit 16-19: echoFeedback (int)
- bit 20-23: echoResolution (int)
- bit 24-31: echoVol (int)

## 0xb8: YMZ280B

- bit 0-7: clockSel (int)
  - 0: 16.9344MHz
  - 1: NTSC
  - 2: PAL
  - 3: 16MHz
  - 4: 16.67MHz
  - 5: 14MHz

## 0xc0: PCM DAC

- bit 0-15: rate (int)
  - add +1 to this value
- bit 16-19: outDepth (int)
- bit 20: stereo (bool)

## 0xe0: QSound

- bit 0-11: echoDelay (int)
- bit 12-19: echoFeedback (int)
