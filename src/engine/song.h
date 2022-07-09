/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2022 tildearrow and contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef _SONG_H
#define _SONG_H
#include <stdio.h>
#include <vector>

#define DIV_MAX_CHANS 128

#include "../ta-utils.h"
#include "orders.h"
#include "instrument.h"
#include "pattern.h"
#include "wavetable.h"
#include "sample.h"

enum DivSystem {
  DIV_SYSTEM_NULL=0,
  DIV_SYSTEM_YMU759,
  DIV_SYSTEM_GENESIS, // ** COMPOUND SYSTEM - DO NOT USE! **
  DIV_SYSTEM_GENESIS_EXT, // ** COMPOUND SYSTEM - DO NOT USE! **
  DIV_SYSTEM_SMS,
  DIV_SYSTEM_SMS_OPLL, // ** COMPOUND SYSTEM - DO NOT USE! **
  DIV_SYSTEM_GB,
  DIV_SYSTEM_PCE,
  DIV_SYSTEM_NES,
  DIV_SYSTEM_NES_VRC7, // ** COMPOUND SYSTEM - DO NOT USE! **
  DIV_SYSTEM_NES_FDS, // ** COMPOUND SYSTEM - DO NOT USE! **
  DIV_SYSTEM_C64_6581,
  DIV_SYSTEM_C64_8580,
  DIV_SYSTEM_ARCADE, // ** COMPOUND SYSTEM - DO NOT USE! **
  DIV_SYSTEM_YM2610,
  DIV_SYSTEM_YM2610_EXT,
  
  DIV_SYSTEM_AY8910,
  DIV_SYSTEM_AMIGA,
  DIV_SYSTEM_YM2151,
  DIV_SYSTEM_YM2612,
  DIV_SYSTEM_TIA,
  DIV_SYSTEM_SAA1099,
  DIV_SYSTEM_AY8930,
  DIV_SYSTEM_VIC20,
  DIV_SYSTEM_PET,
  DIV_SYSTEM_SNES,
  DIV_SYSTEM_VRC6,
  DIV_SYSTEM_OPLL,
  DIV_SYSTEM_FDS,
  DIV_SYSTEM_MMC5,
  DIV_SYSTEM_N163,
  DIV_SYSTEM_OPN,
  DIV_SYSTEM_OPN_EXT,
  DIV_SYSTEM_PC98,
  DIV_SYSTEM_PC98_EXT,
  DIV_SYSTEM_OPL,
  DIV_SYSTEM_OPL2,
  DIV_SYSTEM_OPL3,
  DIV_SYSTEM_MULTIPCM,
  DIV_SYSTEM_PCSPKR,
  DIV_SYSTEM_POKEY,
  DIV_SYSTEM_RF5C68,
  DIV_SYSTEM_SWAN,
  DIV_SYSTEM_OPZ,
  DIV_SYSTEM_POKEMINI,
  DIV_SYSTEM_SEGAPCM,
  DIV_SYSTEM_VBOY,
  DIV_SYSTEM_VRC7,
  DIV_SYSTEM_YM2610B,
  DIV_SYSTEM_SFX_BEEPER,
  DIV_SYSTEM_YM2612_EXT,
  DIV_SYSTEM_SCC,
  DIV_SYSTEM_OPL_DRUMS,
  DIV_SYSTEM_OPL2_DRUMS,
  DIV_SYSTEM_OPL3_DRUMS,
  DIV_SYSTEM_YM2610_FULL,
  DIV_SYSTEM_YM2610_FULL_EXT,
  DIV_SYSTEM_OPLL_DRUMS,
  DIV_SYSTEM_LYNX,
  DIV_SYSTEM_QSOUND,
  DIV_SYSTEM_VERA,
  DIV_SYSTEM_YM2610B_EXT,
  DIV_SYSTEM_SEGAPCM_COMPAT,
  DIV_SYSTEM_X1_010,
  DIV_SYSTEM_BUBSYS_WSG,
  DIV_SYSTEM_OPL4,
  DIV_SYSTEM_OPL4_DRUMS,
  DIV_SYSTEM_ES5506,
  DIV_SYSTEM_Y8950,
  DIV_SYSTEM_Y8950_DRUMS,
  DIV_SYSTEM_SCC_PLUS,
  DIV_SYSTEM_SOUND_UNIT,
  DIV_SYSTEM_MSM6295,
  DIV_SYSTEM_MSM6258,
  DIV_SYSTEM_YMZ280B,
  DIV_SYSTEM_NAMCO,
  DIV_SYSTEM_NAMCO_15XX,
  DIV_SYSTEM_NAMCO_CUS30,
  DIV_SYSTEM_YM2612_FRAC,
  DIV_SYSTEM_YM2612_FRAC_EXT,
  DIV_SYSTEM_RESERVED_8,
  DIV_SYSTEM_T6W28,
  DIV_SYSTEM_PCM_DAC,
  DIV_SYSTEM_DUMMY
};

struct DivSubSong {
  String name, notes;
  unsigned char hilightA, hilightB;
  unsigned char timeBase, speed1, speed2, arpLen;
  short virtualTempoN, virtualTempoD;
  bool pal;
  bool customTempo;
  float hz;
  int patLen, ordersLen;

  DivOrders orders;
  DivChannelData pat[DIV_MAX_CHANS];

  bool chanShow[DIV_MAX_CHANS];
  unsigned char chanCollapse[DIV_MAX_CHANS];
  String chanName[DIV_MAX_CHANS];
  String chanShortName[DIV_MAX_CHANS];

  void clearData();

  DivSubSong(): 
    hilightA(4),
    hilightB(16),
    timeBase(0),
    speed1(6),
    speed2(6),
    arpLen(1),
    virtualTempoN(150),
    virtualTempoD(150),
    pal(true),
    customTempo(false),
    hz(60.0),
    patLen(64),
    ordersLen(1) {
    for (int i=0; i<DIV_MAX_CHANS; i++) {
      chanShow[i]=true;
      chanCollapse[i]=0;
    }
  }
};

struct DivSong {
  // version number used for saving the song.
  // Furnace will save using the latest possible version,
  // known version numbers:
  // - 26: v1.1.3
  //   - changes height of FDS wave to 6-bit (it was 4-bit before)
  // - 25: v1.1
  //   - adds pattern names (in a rather odd way)
  //   - introduces SMS+OPLL system
  // - 24: v0.12/0.13/1.0
  //   - current format version
  //   - changes pattern length from char to int, probably to allow for size 256
  // - 23: ???
  //   - what happened here?
  // - 20: v11.1 (?)
  //   - E5xx effect range is now ±1 semitone
  // - 19: v11
  //   - introduces Arcade system
  //   - changes to the FM instrument format due to YMU759 being dropped
  // - 18: v10
  //   - radically changes STD instrument for Game Boy
  // - 17: v9
  //   - changes C64 volIsCutoff flag from int to char for unknown reasons
  // - 16: v8 (?)
  //   - introduces C64 system
  // - 15: v7 (?)
  // - 14: v6 (?)
  //   - introduces NES system
  //   - changes macro and wave values from char to int
  // - 13: v5.1
  //   - introduces PC Engine system in later version (how?)
  //   - stores highlight in file
  // - 12: v5 (?)
  //   - introduces Game Boy system
  //   - introduces wavetables
  // - 11: ???
  //   - introduces Sega Master System
  //   - custom Hz support
  //   - instrument type (FM/STD) present
  //   - prior to this version the instrument type depended on the system
  // - 10: ???
  //   - introduces multiple effect columns
  // - 9: v3.9
  //   - introduces Genesis system
  //   - introduces system number
  //   - patterns now stored in current known format
  // - 8: ???
  //   - only used in the Medivo YMU cover
  // - 7: ???
  //   - only present in a later version of First.dmf
  //   - pattern format changes: empty field is 0xFF instead of 0x80
  //   - instrument now stored in pattern
  // - 5: BETA 3
  //   - adds arpeggio tick
  // - 4: BETA 2
  //   - possibly adds instrument number (stored in channel)?
  //   - cannot confirm as I don't have any version 4 modules
  // - 3: BETA 1
  //   - possibly the first version that could save
  //   - basic format, no system number, 16 instruments, one speed, YMU759-only
  //   - patterns were stored in a different format (chars instead of shorts) and no instrument
  //   - if somebody manages to find a version 2 or even 1 module, please tell me as it will be worth more than a luxury vehicle
  unsigned short version;
  bool isDMF;

  // system
  DivSystem system[32];
  unsigned char systemLen;
  signed char systemVol[32];
  signed char systemPan[32];
  // interpretation of these flags varies depending on system.
  // - most systems:
  //   - bit 0: PAL
  // - NES:
  //   - bit 0-1: system type
  //     - 0: NTSC
  //     - 1: PAL
  //     - 2: Dendy
  // - SMS/SN76489:
  //   - bit 0-1, 8-15: clock rate
  //     - 0000: 3.58MHz (NTSC)
  //     - 0001: 3.55MHz (PAL)
  //     - 0002: 4MHz (Other)
  //     - 0003: 1.79MHz (half NTSC)
  //     - 0100: 3MHz
  //     - 0101: 2MHz
  //     - 0102: 447KHz (NTSC / 8)
  //   - bit 2-3, 6-7: chip type
  //     - 00: Sega VDP (16-bit noise)
  //     - 04: real SN76489 (15-bit noise)
  //     - 08: real SN76489 with Atari-like short noise buzz (15-bit noise)
  //     - 0c: Game Gear (16-bit noise, stereo)
  //     - 40: real SN76489A (17-bit noise)
  //     - 44: real SN76496 (17-bit noise)
  //     - 48: NCR 8496 (16-bit noise)
  //     - 4c: Tandy PSSJ-3 (16-bit noise)
  //     - 80: real SN94624 (15-bit noise)
  //     - 84: real SN76494 (17-bit noise)
  //   - bit 4: disable noise phase reset
  // - YM2612/YM3438:
  //   - bit 0-30: clock rate
  //     - 0: Genesis NTSC (7.67MHz)
  //     - 1: Genesis PAL (7.61MHz)
  //     - 2: FM Towns (8MHz)
  //     - 3: AtGames Genesis (6.13MHz)
  //     - 4: Sega System 32 (8.06MHz)
  //   - bit 31: DAC distortion
  //     - 0: disable
  //     - 1: enable
  // - YM2151:
  //   - bit 0-7: clock rate
  //     - 0: 3.58MHz (NTSC)
  //     - 1: 3.55MHz (PAL)
  //     - 2: 4MHz
  // - YM2610(B):
  //   - bit 0-7: clock rate
  //     - 0: 8MHz (Neo Geo MVS)
  //     - 1: 8.06MHz (Neo Geo AES)
  // - AY-3-8910/AY8930:
  //   - bit 0-3: clock rate
  //     - 0: 1.79MHz (MSX NTSC)
  //     - 1: 1.77MHz (ZX Spectrum, MSX PAL, etc.)
  //     - 2: 1.75MHz (ZX Spectrum)
  //     - 3: 2MHz (Atari ST)
  //     - 4: 1.5MHz (Vectrex)
  //     - 5: 1MHz (Amstrad CPC)
  //     - 6: 0.89MHz (Sunsoft 5B)
  //     - 7: 1.67MHz
  //     - 8: 0.83MHz (Sunsoft 5B on PAL)
  //     - 9: 1.10MHz (Gamate/VIC-20 PAL)
  //     - 10: 2.097152MHz (Game Boy)
  //     - 11: 3.58MHz (Darky)
  //     - 12: 3.6MHz (Darky)
  //     - 13: 1.25MHz
  //     - 14: 1.536MHz
  //   - bit 4-5: chip type (ignored on AY8930)
  //     - 0: AY-3-8910 or similar
  //     - 1: YM2149
  //     - 2: Sunsoft 5B
  //     - 3: AY-3-8914
  //   - bit 6: stereo (ignored on Sunsoft 5B)
  //     - 0: mono
  //     - 1: stereo ABC
  //   - bit 7: clock divider pin (YM2149, AY8930)
  //     - 0: high (disable divider)
  //     - 1: low (internally divided to half)
  // - SAA1099:
  //   - bit 0-1: clock rate
  //     - 0: 8MHz (SAM Coupé)
  //     - 1: 7.15MHz (Game Blaster, NTSC)
  //     - 2: 7.09MHz (PAL)
  // - Amiga:
  //   - bit 0: clock rate
  //     - 0: 7.15MHz (NTSC)
  //     - 1: 7.09MHz (PAL)
  //   - bit 1: model
  //     - 0: Amiga 500
  //     - 1: Amiga 1200
  //   - bit 8-14: stereo separation
  //     - 0 is 0% while 127 is 100%
  // - PC Speaker:
  //   - bit 0-1: speaker type
  //     - 0: unfiltered
  //     - 1: cone
  //     - 2: piezo
  //     - 3: real (TODO)
  // - QSound:
  //   - bit 12-20: echo feedback
  //     - Valid values are 0-255
  //   - bit 0-11: echo delay length
  //     - Valid values are 0-2725
  //     - 0 is max length, 2725 is min length
  // - OPLL:
  //   - bit 0-3: clock rate
  //     - 0: NTSC (3.58MHz)
  //     - 1: PAL (3.55MHz)
  //     - 2: Other (4MHz)
  //     - 3: half NTSC (1.79MHz)
  //   - bit 4-7: patch set
  //     - 0: YM2413
  //     - 1: YMF281
  //     - 2: YM2423
  //     - 3: VRC7
  //     - 4: custom (TODO)
  // - X1-010:
  //   - bit 0-3: clock rate
  //     - 0: 16MHz (Seta 1)
  //     - 1: 16.67MHz (Seta 2)
  //   - bit 4: stereo
  //     - 0: mono
  //     - 1: stereo
  // - YM2203:
  //   - bit 0-4: clock rate
  //     - 0: 3.58MHz (NTSC)
  //     - 1: 3.55MHz (PAL)
  //     - 2: 4MHz
  //     - 3: 3MHz
  //     - 4: 3.9936MHz (PC-88, PC-98)
  //     - 5: 1.5MHz
  //   - bit 5-6: output rate
  //     - 0: FM: clock / 72, SSG: clock / 16
  //     - 1: FM: clock / 36, SSG: clock / 8
  //     - 2: FM: clock / 24, SSG: clock / 4
  // - YM2608:
  //   - bit 0-4: clock rate
  //     - 0: 8MHz
  //     - 1: 7.987MHz (PC-88, PC-98)
  //   - bit 5-6: output rate
  //     - 0: FM: clock / 144, SSG: clock / 32
  //     - 1: FM: clock / 72, SSG: clock / 16
  //     - 2: FM: clock / 48, SSG: clock / 8
  // - YM3526, YM3812, Y8950:
  //   - bit 0-7: clock rate
  //     - 0: 3.58MHz (NTSC)
  //     - 1: 3.55MHz (PAL)
  //     - 2: 4MHz
  //     - 3: 3MHz
  //     - 4: 3.9936MHz (PC-88, PC-98)
  //     - 5: 3.5MHz
  // - YMF262:
  //   - bit 0-7: clock rate
  //     - 0: 14.32MHz (NTSC)
  //     - 1: 14.19MHz (PAL)
  //     - 2: 14MHz
  //     - 3: 16MHz
  //     - 4: 15MHz
  // - YMF289B: (TODO)
  //   - bit 0-7: clock rate
  //     - 0: 33.8688MHz
  //     - 1: 28.64MHz (NTSC)
  //     - 2: 28.38MHz (PAL)
  // - MSM6295:
  //   - bit 0-6: clock rate
  //     - 0: 1MHz
  //     - 1: 1.056MHz
  //     - 2: 4MHz
  //     - 3: 4.224MHz
  //     - 4: 3.58MHz (NTSC)
  //     - 5: 1.79MHz (Half NTSC)
  //     - 6: 1.023MHz
  //     - 7: 0.895MHz (Quarter NTSC)
  //     - 8: 2MHz
  //     - 9: 2.112MHz
  //     - 10: 0.875MHz
  //     - 11: 0.9375MHz
  //     - 12: 1.5MHz
  //     - 13: 3MHz
  //     - 14: 1.193MHz
  //   - bit 7: Output rate
  //     - 0: clock / 132
  //     - 1: clock / 165
  // - SCC/+:
  //   - bit 0-6: clock rate
  //     - 0: 1.79MHz (MSX NTSC)
  //     - 1: 1.77MHz (PAL)
  //     - 2: 1.5MHz
  //     - 3: 2MHz
  // - YMZ280B:
  //   - bit 0-7: clock rate
  //     - 0: 16.9344MHz
  //     - 1: 14.32MHz (NTSC)
  //     - 2: 14.19MHz (PAL)
  //     - 3: 16MHz
  //     - 4: 16.67MHz
  //     - 5: 14MHz
  unsigned int systemFlags[32];

  // song information
  String name, author;

  // legacy song information
  // those will be stored in .fur and mapped to VGM as:
  // category -> game name
  // writer -> ripper
  // createdDate -> year
  String carrier, composer, vendor, category, writer, arranger, copyright, manGroup, manInfo, createdDate, revisionDate;

  // more VGM specific stuff
  String nameJ, authorJ, categoryJ;

  // other things
  String notes;

  // module details
  int insLen, waveLen, sampleLen;
  float masterVol;
  float tuning;

  // compatibility flags
  bool limitSlides;
  // linear pitch
  // 0: not linear
  // 1: only pitch changes (04xy/E5xx) linear
  // 2: full linear
  unsigned char linearPitch;
  unsigned char pitchSlideSpeed;
  // loop behavior
  // 0: reset on loop
  // 1: fake reset on loop
  // 2: don't do anything on loop
  unsigned char loopModality;
  bool properNoiseLayout;
  bool waveDutyIsVol;
  bool resetMacroOnPorta;
  bool legacyVolumeSlides;
  bool compatibleArpeggio;
  bool noteOffResetsSlides;
  bool targetResetsSlides;
  bool arpNonPorta;
  bool algMacroBehavior;
  bool brokenShortcutSlides;
  bool ignoreDuplicateSlides;
  bool stopPortaOnNoteOff;
  bool continuousVibrato;
  bool brokenDACMode;
  bool oneTickCut;
  bool newInsTriggersInPorta;
  bool arp0Reset;
  bool brokenSpeedSel;
  bool noSlidesOnFirstTick;
  bool rowResetsArpPos;
  bool ignoreJumpAtEnd;
  bool buggyPortaAfterSlide;
  bool gbInsAffectsEnvelope;
  bool sharedExtStat;
  bool ignoreDACModeOutsideIntendedChannel;
  bool e1e2AlsoTakePriority;
  bool newSegaPCM;
  bool fbPortaPause;
  bool snDutyReset;
  bool pitchMacroIsLinear;
  bool oldOctaveBoundary;
  bool noOPN2Vol;
  bool newVolumeScaling;
  bool volMacroLinger;
  bool brokenOutVol;
  bool e1e2StopOnSameNote;
  bool brokenPortaArp;

  std::vector<DivInstrument*> ins;
  std::vector<DivWavetable*> wave;
  std::vector<DivSample*> sample;

  std::vector<DivSubSong*> subsong;

  DivInstrument nullIns, nullInsOPLL, nullInsOPL, nullInsOPLDrums, nullInsQSound;
  DivWavetable nullWave;
  DivSample nullSample;

  /**
   * clear orders and patterns.
   */
  void clearSongData();

  /**
   * clear instruments.
   */
  void clearInstruments();

  /**
   * clear wavetables.
   */
  void clearWavetables();

  /**
   * clear samples.
   */
  void clearSamples();

  /**
   * unloads the song, freeing all memory associated with it.
   * use before destroying the object.
   */
  void unload();

  DivSong():
    version(0),
    isDMF(false),
    systemLen(2),
    name(""),
    author(""),
    carrier(""),
    composer(""),
    vendor(""),
    category(""),
    writer(""),
    arranger(""),
    copyright(""),
    manGroup(""),
    manInfo(""),
    createdDate(""),
    revisionDate(""),
    insLen(0),
    waveLen(0),
    sampleLen(0),
    masterVol(1.0f),
    tuning(440.0f),
    limitSlides(false),
    linearPitch(2),
    pitchSlideSpeed(4),
    loopModality(0),
    properNoiseLayout(false),
    waveDutyIsVol(false),
    resetMacroOnPorta(false),
    legacyVolumeSlides(false),
    compatibleArpeggio(false),
    noteOffResetsSlides(true),
    targetResetsSlides(true),
    arpNonPorta(false),
    algMacroBehavior(false),
    brokenShortcutSlides(false),
    ignoreDuplicateSlides(false),
    stopPortaOnNoteOff(false),
    continuousVibrato(false),
    brokenDACMode(false),
    oneTickCut(false),
    newInsTriggersInPorta(true),
    arp0Reset(true),
    brokenSpeedSel(false),
    noSlidesOnFirstTick(false),
    rowResetsArpPos(false),
    ignoreJumpAtEnd(false),
    buggyPortaAfterSlide(false),
    gbInsAffectsEnvelope(true),
    sharedExtStat(true),
    ignoreDACModeOutsideIntendedChannel(false),
    e1e2AlsoTakePriority(false),
    newSegaPCM(true),
    fbPortaPause(false),
    snDutyReset(false),
    pitchMacroIsLinear(true),
    oldOctaveBoundary(false),
    noOPN2Vol(false),
    newVolumeScaling(true),
    volMacroLinger(true),
    brokenOutVol(false),
    e1e2StopOnSameNote(false),
    brokenPortaArp(false) {
    for (int i=0; i<32; i++) {
      system[i]=DIV_SYSTEM_NULL;
      systemVol[i]=64;
      systemPan[i]=0;
      systemFlags[i]=0;
    }
    subsong.push_back(new DivSubSong);
    system[0]=DIV_SYSTEM_YM2612;
    system[1]=DIV_SYSTEM_SMS;

    // OPLL default instrument contest winner - piano_guitar_idk by Weeppiko
    nullInsOPLL.type=DIV_INS_OPLL;
    nullInsOPLL.fm.opllPreset=0;
    nullInsOPLL.fm.alg=0;
    nullInsOPLL.fm.fb=7;
    nullInsOPLL.fm.fms=1;
    nullInsOPLL.fm.ams=0;
    nullInsOPLL.fm.op[0].ar=15;
    nullInsOPLL.fm.op[0].dr=5;
    nullInsOPLL.fm.op[0].sl=3;
    nullInsOPLL.fm.op[0].rr=3;
    nullInsOPLL.fm.op[0].tl=40;
    nullInsOPLL.fm.op[0].ksl=0;
    nullInsOPLL.fm.op[0].mult=5;
    nullInsOPLL.fm.op[0].am=0;
    nullInsOPLL.fm.op[0].vib=1;
    nullInsOPLL.fm.op[0].ksr=0;
    nullInsOPLL.fm.op[0].ssgEnv=8;
    nullInsOPLL.fm.op[1].ar=15;
    nullInsOPLL.fm.op[1].dr=1;
    nullInsOPLL.fm.op[1].sl=11;
    nullInsOPLL.fm.op[1].rr=6;
    nullInsOPLL.fm.op[1].tl=0;
    nullInsOPLL.fm.op[1].ksl=0;
    nullInsOPLL.fm.op[1].mult=1;
    nullInsOPLL.fm.op[1].am=0;
    nullInsOPLL.fm.op[1].vib=0;
    nullInsOPLL.fm.op[1].ksr=0;
    nullInsOPLL.fm.op[1].ssgEnv=8;
    nullInsOPLL.name="This is a bug! Report!";

    nullInsOPL.type=DIV_INS_OPL;
    nullInsOPL.fm.alg=0;
    nullInsOPL.fm.fb=7;
    nullInsOPL.fm.op[0].dr=2;
    nullInsOPL.fm.op[0].rr=7;
    nullInsOPL.fm.op[0].tl=22;
    nullInsOPL.fm.op[0].ksl=1;
    nullInsOPL.fm.op[0].mult=3;
    nullInsOPL.fm.op[1].tl=0;
    nullInsOPL.fm.op[1].dr=3;
    nullInsOPL.fm.op[1].rr=12;
    nullInsOPL.fm.op[1].mult=1;
    nullInsOPL.name="This is a bug! Report!";
    nullInsOPL.fm.kickFreq=(1<<10)|576;
    nullInsOPL.fm.snareHatFreq=(1<<10)|672;
    nullInsOPL.fm.tomTopFreq=896;

    nullInsOPLDrums=nullInsOPL;
    nullInsOPLDrums.type=DIV_INS_OPL_DRUMS;
    nullInsOPLDrums.fm.fixedDrums=true;

    for (int i=0; i<4; i++) {
      nullInsOPLDrums.fm.op[i].am=0;
      nullInsOPLDrums.fm.op[i].vib=0;
      nullInsOPLDrums.fm.op[i].ksr=0;
      nullInsOPLDrums.fm.op[i].sus=0;
      nullInsOPLDrums.fm.op[i].ws=0;
      nullInsOPLDrums.fm.op[i].ksl=0;
      nullInsOPLDrums.fm.op[i].tl=0;
    }

    // snare
    nullInsOPLDrums.fm.op[0].ar=13;
    nullInsOPLDrums.fm.op[0].dr=8;
    nullInsOPLDrums.fm.op[0].sl=4;
    nullInsOPLDrums.fm.op[0].rr=8;
    nullInsOPLDrums.fm.op[0].mult=1;

    // tom
    nullInsOPLDrums.fm.op[1].ar=15;
    nullInsOPLDrums.fm.op[1].dr=8;
    nullInsOPLDrums.fm.op[1].sl=5;
    nullInsOPLDrums.fm.op[1].rr=9;
    nullInsOPLDrums.fm.op[1].mult=5;

    // top
    nullInsOPLDrums.fm.op[2].ar=10;
    nullInsOPLDrums.fm.op[2].dr=10;
    nullInsOPLDrums.fm.op[2].sl=5;
    nullInsOPLDrums.fm.op[2].rr=5;
    nullInsOPLDrums.fm.op[2].mult=1;
    nullInsOPLDrums.fm.op[2].ksr=1;

    // hi-hat
    nullInsOPLDrums.fm.op[3].ar=12;
    nullInsOPLDrums.fm.op[3].dr=8;
    nullInsOPLDrums.fm.op[3].sl=10;
    nullInsOPLDrums.fm.op[3].rr=7;
    nullInsOPLDrums.fm.op[3].mult=2;

    nullInsQSound.std.panLMacro.mode=true;
  }
};

#endif
