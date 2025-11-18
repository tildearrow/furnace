/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2025 tildearrow and contributors
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
#include "../pch.h"

#include "defines.h"
#include "../timeutils.h"
#include "../ta-utils.h"
#include "config.h"
#include "assetDir.h"
#include "orders.h"
#include "instrument.h"
#include "pattern.h"
#include "wavetable.h"
#include "sample.h"
#include "sysDef.h"

enum DivEffectType: unsigned short {
  DIV_EFFECT_NULL=0,
  DIV_EFFECT_DUMMY,
  DIV_EFFECT_EXTERNAL,
  DIV_EFFECT_VOLUME,
  DIV_EFFECT_FILTER
};

enum DivFileElementType: unsigned char {
  DIV_ELEMENT_END=0,
  DIV_ELEMENT_SUBSONG,
  DIV_ELEMENT_CHIP_FLAGS,
  DIV_ELEMENT_ASSET_DIR,
  DIV_ELEMENT_INSTRUMENT,
  DIV_ELEMENT_WAVETABLE,
  DIV_ELEMENT_SAMPLE,
  DIV_ELEMENT_PATTERN,
  DIV_ELEMENT_COMPAT_FLAGS,
  DIV_ELEMENT_COMMENTS,
  DIV_ELEMENT_GROOVE,

  DIV_ELEMENT_MAX
};

struct DivGroovePattern {
  unsigned short val[16];
  unsigned short len;
  bool readData(SafeReader& reader);
  void putData(SafeWriter* w);
  DivGroovePattern():
    len(1) {
    for (int i=0; i<16; i++) {
      val[i]=6;
    }
  }
};

struct DivSongTimestamps {
  // song duration (in seconds and microseconds)
  TimeMicros totalTime;
  uint64_t totalTicks;
  int totalRows;

  // loop region (order/row positions)
  struct Position {
    int order, row;
    Position():
      order(0), row(0) {}
  } loopStart, loopEnd;
  // set to true if a 0Bxx effect is found and it defines a loop region
  bool isLoopDefined;
  // set to false if FFxx is found
  bool isLoopable;

  // timestamp of a row
  // DO NOT ACCESS DIRECTLY! use the functions instead.
  // if seconds is -1, it means this row is not touched at all.
  TimeMicros* orders[DIV_MAX_PATTERNS];
  TimeMicros loopStartTime;

  // the furthest row that the playhead goes through in an order.
  unsigned char maxRow[DIV_MAX_PATTERNS];

  // call this function to get the timestamp of a row.
  TimeMicros getTimes(int order, int row);

  DivSongTimestamps();
  ~DivSongTimestamps();
};


struct DivSubSong {
  String name, notes;
  unsigned char hilightA, hilightB;
  unsigned char effectDivider, arpLen;
  DivGroovePattern speeds;
  short virtualTempoN, virtualTempoD;
  float hz;
  int patLen, ordersLen;

  DivOrders orders;
  DivChannelData pat[DIV_MAX_CHANS];

  bool chanShow[DIV_MAX_CHANS];
  bool chanShowChanOsc[DIV_MAX_CHANS];
  unsigned char chanCollapse[DIV_MAX_CHANS];
  String chanName[DIV_MAX_CHANS];
  String chanShortName[DIV_MAX_CHANS];

  // song timestamps
  DivSongTimestamps ts;

  /**
   * calculate timestamps (loop position, song length and more).
   */
  void calcTimestamps(int chans, std::vector<DivGroovePattern>& grooves, int jumpTreatment, int ignoreJumpAtEnd, int brokenSpeedSel, int delayBehavior, int firstPat=0);

  /**
   * read sub-song data.
   */
  bool readData(SafeReader& reader, int version, int chans);

  /**
   * write sub-song block to a SafeWriter.
   */
  void putData(SafeWriter* w, int chans);

  void clearData();
  void removeUnusedPatterns();
  void optimizePatterns();
  void rearrangePatterns();
  void sortOrders();
  void makePatUnique();

  DivSubSong(): 
    hilightA(4),
    hilightB(16),
    effectDivider(0),
    arpLen(1),
    virtualTempoN(150),
    virtualTempoD(150),
    hz(60.0),
    patLen(64),
    ordersLen(1) {
    for (int i=0; i<DIV_MAX_CHANS; i++) {
      chanShow[i]=true;
      chanShowChanOsc[i]=true;
      chanCollapse[i]=0;
    }
  }
};

struct DivEffectStorage {
  DivEffectType id;
  unsigned short slot, storageVer;
  float dryWet;
  unsigned char* storage;
  size_t storageLen;
  DivEffectStorage():
    id(DIV_EFFECT_NULL),
    slot(0),
    storageVer(0),
    dryWet(1.0f),
    storage(NULL),
    storageLen(0) {}
};

struct DivCompatFlags {
  bool limitSlides;
  // linear pitch
  unsigned char linearPitch;
  unsigned char pitchSlideSpeed;
  // loop behavior
  // 0: reset on loop
  // 1: fake reset on loop
  // 2: don't do anything on loop
  unsigned char loopModality;
  // cut/delay effect behavior
  // 0: strict (don't allow value higher than or equal to speed)
  // 1: broken (don't allow value higher than speed)
  // 2: lax (allow value higher than speed)
  unsigned char delayBehavior;
  // 0B/0D treatment
  // 0: normal (0B/0D accepted)
  // 1: old Furnace (first one accepted)
  // 2: DefleMask (0D takes priority over 0B)
  unsigned char jumpTreatment;
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
  bool brokenOutVol2;
  bool e1e2StopOnSameNote;
  bool brokenPortaArp;
  bool snNoLowPeriods;
  bool disableSampleMacro;
  bool oldArpStrategy;
  bool brokenPortaLegato;
  bool brokenFMOff;
  bool preNoteNoEffect;
  bool oldDPCM;
  bool resetArpPhaseOnNewNote;
  bool ceilVolumeScaling;
  bool oldAlwaysSetVolume;
  bool oldSampleOffset;
  // new flags as of dev240
  bool oldCenterRate;

  void setDefaults();
  bool areDefaults();
  bool readData(SafeReader& reader);
  void putData(SafeWriter* w);

  bool operator==(const DivCompatFlags& other) {
    return (memcmp(this,&other,sizeof(DivCompatFlags))==0);
  }
  bool operator!=(const DivCompatFlags& other) {
    return (memcmp(this,&other,sizeof(DivCompatFlags))!=0);
  }

  DivCompatFlags() {
    memset(this,0,sizeof(DivCompatFlags));
    setDefaults();
  }
};

struct DivSong {
  unsigned short version;
  bool isDMF;

  // system
  int chans;
  DivSystem system[DIV_MAX_CHIPS];
  unsigned short systemChans[DIV_MAX_CHIPS];
  unsigned char systemLen;
  float systemVol[DIV_MAX_CHIPS];
  float systemPan[DIV_MAX_CHIPS];
  float systemPanFR[DIV_MAX_CHIPS];
  DivConfig systemFlags[DIV_MAX_CHIPS];

  // song information
  String name, author, systemName;

  // legacy song information
  // those will be stored in .fur and mapped to VGM as:
  // category -> game name
  // writer -> ripper
  // createdDate -> year
  String carrier, composer, vendor, category, writer, arranger, copyright, manGroup, manInfo, createdDate, revisionDate;

  // more VGM specific stuff
  String nameJ, authorJ, categoryJ, systemNameJ;

  // other things
  String notes;

  // module details
  int insLen, waveLen, sampleLen;
  float masterVol;
  float tuning;

  bool autoSystem;
  bool patchbayAuto;

  // compatibility flags
  DivCompatFlags compatFlags;

  std::vector<DivInstrument*> ins;
  std::vector<DivWavetable*> wave;
  std::vector<DivSample*> sample;

  std::vector<DivSubSong*> subsong;
  std::vector<unsigned int> patchbay;
  std::vector<DivGroovePattern> grooves;

  std::vector<DivAssetDir> insDir;
  std::vector<DivAssetDir> waveDir;
  std::vector<DivAssetDir> sampleDir;

  std::vector<DivEffectStorage> effects;

  /**
   * INTERNAL STATE - do not modify.
   */
  // default/"null" instruments (when instrument is none/-1)
  DivInstrument nullIns, nullInsOPLL, nullInsOPL, nullInsOPLDrums, nullInsQSound, nullInsESFM;
  // default assets, returned by getWave()/getSample() in DivEngine
  DivWavetable nullWave;
  DivSample nullSample;

  // channel information arrays.
  // chip of a channel
  DivSystem sysOfChan[DIV_MAX_CHANS];
  // dispatch (chip index) of a channel
  int dispatchOfChan[DIV_MAX_CHANS];
  // tracker channel to chip channel mapping
  // -1 means "nowhere".
  int dispatchChanOfChan[DIV_MAX_CHANS];
  // the first channel of a chip, indexed per channel
  int dispatchFirstChan[DIV_MAX_CHANS];
  // cached DivChanDef for each channel.
  DivChanDef chanDef[DIV_MAX_CHANS];

  std::vector<DivInstrumentType> possibleInsTypes;

  /**
   * find data past 0Bxx effects and place that into new sub-songs.
   */
  void findSubSongs();

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
   * set systemChans[] to default values.
   * call recalcChans() afterwards.
   */
  void initDefaultSystemChans();

  /**
   * recalculate channel count and internal state.
   * call after editing system[] or systemChans[].
   */
  void recalcChans();

  /**
   * unloads the song, freeing all memory associated with it.
   * use before destroying the object.
   */
  void unload();

  DivSong():
    version(0),
    isDMF(false),
    chans(0),
    systemLen(2),
    name(""),
    author(""),
    systemName(""),
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
    autoSystem(true),
    patchbayAuto(true) {
    memset(dispatchFirstChan,0,DIV_MAX_CHANS*sizeof(int));
    memset(dispatchChanOfChan,0,DIV_MAX_CHANS*sizeof(int));
    memset(dispatchOfChan,0,DIV_MAX_CHANS*sizeof(int));
    memset(sysOfChan,0,DIV_MAX_CHANS*sizeof(int));

    for (int i=0; i<DIV_MAX_CHIPS; i++) {
      system[i]=DIV_SYSTEM_NULL;
      systemChans[i]=0;
      systemVol[i]=1.0;
      systemPan[i]=0.0;
      systemPanFR[i]=0.0;
    }
    subsong.push_back(new DivSubSong);
    system[0]=DIV_SYSTEM_YM2612;
    system[1]=DIV_SYSTEM_SMS;
    systemChans[0]=6;
    systemChans[1]=4;

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

    // ESFM default instrument - port of OPN default instrument
    nullInsESFM.esfm.noise=0;
    nullInsESFM.esfm.op[0].outLvl=0;
    nullInsESFM.esfm.op[0].modIn=4;
    nullInsESFM.esfm.op[0].dt=2;
    nullInsESFM.fm.op[0].tl=42;
    nullInsESFM.fm.op[0].ar=15;
    nullInsESFM.fm.op[0].dr=3;
    nullInsESFM.fm.op[0].sl=15;
    nullInsESFM.fm.op[0].rr=3;
    nullInsESFM.fm.op[0].mult=5;

    nullInsESFM.esfm.op[1].outLvl=0;
    nullInsESFM.esfm.op[1].modIn=7;
    nullInsESFM.esfm.op[1].dt=-3;
    nullInsESFM.fm.op[1].tl=18;
    nullInsESFM.fm.op[1].ar=15;
    nullInsESFM.fm.op[1].dr=3;
    nullInsESFM.fm.op[1].sl=15;
    nullInsESFM.fm.op[1].rr=4;
    nullInsESFM.fm.op[1].mult=1;

    nullInsESFM.esfm.op[2].outLvl=0;
    nullInsESFM.esfm.op[2].modIn=7;
    nullInsESFM.esfm.op[2].dt=2;
    nullInsESFM.fm.op[2].tl=48;
    nullInsESFM.fm.op[2].ar=15;
    nullInsESFM.fm.op[2].dr=2;
    nullInsESFM.fm.op[2].sl=11;
    nullInsESFM.fm.op[2].rr=1;
    nullInsESFM.fm.op[2].mult=1;
    nullInsESFM.fm.op[2].sus=1;

    nullInsESFM.esfm.op[3].outLvl=7;
    nullInsESFM.esfm.op[3].modIn=7;
    nullInsESFM.esfm.op[3].dt=-3;
    nullInsESFM.fm.op[3].tl=0;
    nullInsESFM.fm.op[3].ar=15;
    nullInsESFM.fm.op[3].dr=3;
    nullInsESFM.fm.op[3].sl=15;
    nullInsESFM.fm.op[3].rr=9;
    nullInsESFM.fm.op[3].mult=1;

    recalcChans();
  }
};

#endif
