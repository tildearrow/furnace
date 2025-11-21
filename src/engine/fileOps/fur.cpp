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

#include "fileOpsCommon.h"

short newFormatNotes[180]={
  12, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, // -5
  12, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, // -4
  12, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, // -3
  12, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, // -2
  12, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, // -1
  12, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, //  0
  12, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, //  1
  12, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, //  2
  12, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, //  3
  12, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, //  4
  12, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, //  5
  12, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, //  6
  12, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, //  7
  12, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, //  8
  12, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11  //  9
};

short newFormatOctaves[180]={
  250, 251, 251, 251, 251, 251, 251, 251, 251, 251, 251, 251, // -5
  251, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, // -4
  252, 253, 253, 253, 253, 253, 253, 253, 253, 253, 253, 253, // -3
  253, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, // -2
  254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // -1
  255,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //  0
    0,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1, //  1
    1,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2, //  2
    2,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3, //  3
    3,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4, //  4
    4,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5, //  5
    5,   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,   6, //  6
    6,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7, //  7
    7,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8, //  8
    8,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9, //  9
};

struct PatToWrite {
  unsigned short subsong, chan, pat;
  PatToWrite(unsigned short s, unsigned short c, unsigned short p):
    subsong(s),
    chan(c),
    pat(p) {}
};

void DivEngine::convertOldFlags(unsigned int oldFlags, DivConfig& newFlags, DivSystem sys) {
  newFlags.clear();

  switch (sys) {
    case DIV_SYSTEM_SMS:
      switch (oldFlags&0xff03) {
        case 0x0000:
          newFlags.set("clockSel",0);
          break;
        case 0x0001:
          newFlags.set("clockSel",1);
          break;
        case 0x0002:
          newFlags.set("clockSel",2);
          break;
        case 0x0003:
          newFlags.set("clockSel",3);
          break;
        case 0x0100:
          newFlags.set("clockSel",4);
          break;
        case 0x0101:
          newFlags.set("clockSel",5);
          break;
        case 0x0102:
          newFlags.set("clockSel",6);
          break;
      }
      switch (oldFlags&0xcc) {
        case 0x00:
          newFlags.set("chipType",0);
          break;
        case 0x04:
          newFlags.set("chipType",1);
          break;
        case 0x08:
          newFlags.set("chipType",2);
          break;
        case 0x0c:
          newFlags.set("chipType",3);
          break;
        case 0x40:
          newFlags.set("chipType",4);
          break;
        case 0x44:
          newFlags.set("chipType",5);
          break;
        case 0x48:
          newFlags.set("chipType",6);
          break;
        case 0x4c:
          newFlags.set("chipType",7);
          break;
        case 0x80:
          newFlags.set("chipType",8);
          break;
        case 0x84:
          newFlags.set("chipType",9);
          break;
      }
      if (oldFlags&16) newFlags.set("noPhaseReset",true);
      break;
    case DIV_SYSTEM_GB:
      newFlags.set("chipType",(int)(oldFlags&3));
      if (oldFlags&8) newFlags.set("noAntiClick",true);
      break;
    case DIV_SYSTEM_PCE:
      newFlags.set("clockSel",(int)(oldFlags&1));
      newFlags.set("chipType",(oldFlags&4)?1:0);
      if (oldFlags&8) newFlags.set("noAntiClick",true);
      break;
    case DIV_SYSTEM_NES:
    case DIV_SYSTEM_VRC6:
    case DIV_SYSTEM_FDS:
    case DIV_SYSTEM_MMC5:
    case DIV_SYSTEM_SAA1099:
    case DIV_SYSTEM_OPZ:
      switch (oldFlags) {
        case 0:
          newFlags.set("clockSel",0);
          break;
        case 1:
          newFlags.set("clockSel",1);
          break;
        case 2:
          newFlags.set("clockSel",2);
          break;
      }
      break;
    case DIV_SYSTEM_C64_6581:
    case DIV_SYSTEM_C64_8580:
      switch (oldFlags&15) {
        case 0:
          newFlags.set("clockSel",0);
          break;
        case 1:
          newFlags.set("clockSel",1);
          break;
        case 2:
          newFlags.set("clockSel",2);
          break;
      }
      break;
    case DIV_SYSTEM_YM2610_CRAP:
    case DIV_SYSTEM_YM2610_CRAP_EXT:
    case DIV_SYSTEM_YM2610_FULL:
    case DIV_SYSTEM_YM2610_FULL_EXT:
    case DIV_SYSTEM_YM2610B:
    case DIV_SYSTEM_YM2610B_EXT:
      switch (oldFlags&0xff) {
        case 0:
          newFlags.set("clockSel",0);
          break;
        case 1:
          newFlags.set("clockSel",1);
          break;
      }
      break;
    case DIV_SYSTEM_AY8910:
    case DIV_SYSTEM_AY8930:
      switch (oldFlags&15) {
        case 0:
          newFlags.set("clockSel",0);
          break;
        case 1:
          newFlags.set("clockSel",1);
          break;
        case 2:
          newFlags.set("clockSel",2);
          break;
        case 3:
          newFlags.set("clockSel",3);
          break;
        case 4:
          newFlags.set("clockSel",4);
          break;
        case 5:
          newFlags.set("clockSel",5);
          break;
        case 6:
          newFlags.set("clockSel",6);
          break;
        case 7:
          newFlags.set("clockSel",7);
          break;
        case 8:
          newFlags.set("clockSel",8);
          break;
        case 9:
          newFlags.set("clockSel",9);
          break;
        case 10:
          newFlags.set("clockSel",10);
          break;
        case 11:
          newFlags.set("clockSel",11);
          break;
        case 12:
          newFlags.set("clockSel",12);
          break;
        case 13:
          if (sys==DIV_SYSTEM_AY8910) newFlags.set("clockSel",13);
          break;
        case 14:
          if (sys==DIV_SYSTEM_AY8910) newFlags.set("clockSel",14);
          break;
      }
      if (sys==DIV_SYSTEM_AY8910) switch ((oldFlags>>4)&3) {
        case 0:
          newFlags.set("chipType",0);
          break;
        case 1:
          newFlags.set("chipType",1);
          break;
        case 2:
          newFlags.set("chipType",2);
          break;
        case 3:
          newFlags.set("chipType",3);
          break;
      }
      if (oldFlags&64) newFlags.set("stereo",true);
      if (oldFlags&128) newFlags.set("halfClock",true);
      newFlags.set("stereoSep",(int)((oldFlags>>8)&255));
      break;
    case DIV_SYSTEM_AMIGA:
      if (oldFlags&1) newFlags.set("clockSel",1);
      if (oldFlags&2) newFlags.set("chipType",1);
      if (oldFlags&4) newFlags.set("bypassLimits",true);
      newFlags.set("stereoSep",(int)((oldFlags>>8)&127));
      break;
    case DIV_SYSTEM_YM2151:
      switch (oldFlags&255) {
        case 0:
          newFlags.set("clockSel",0);
          break;
        case 1:
          newFlags.set("clockSel",1);
          break;
        case 2:
          newFlags.set("clockSel",2);
          break;
      }
      break;
    case DIV_SYSTEM_YM2612:
    case DIV_SYSTEM_YM2612_EXT:
    case DIV_SYSTEM_YM2612_DUALPCM:
    case DIV_SYSTEM_YM2612_DUALPCM_EXT:
      switch (oldFlags&0x7fffffff) {
        case 0:
          newFlags.set("clockSel",0);
          break;
        case 1:
          newFlags.set("clockSel",1);
          break;
        case 2:
          newFlags.set("clockSel",2);
          break;
        case 3:
          newFlags.set("clockSel",3);
          break;
        case 4:
          newFlags.set("clockSel",4);
          break;
      }
      if (oldFlags&0x80000000) newFlags.set("ladderEffect",true);
      break;
    case DIV_SYSTEM_TIA:
      newFlags.set("clockSel",(int)(oldFlags&1));
      switch ((oldFlags>>1)&3) {
        case 0:
          newFlags.set("mixingType",0);
          break;
        case 1:
          newFlags.set("mixingType",1);
          break;
        case 2:
          newFlags.set("mixingType",2);
          break;
      }
      break;
    case DIV_SYSTEM_VIC20:
      newFlags.set("clockSel",(int)(oldFlags&1));
      break;
    case DIV_SYSTEM_SNES:
      newFlags.set("volScaleL",(int)(oldFlags&127));
      newFlags.set("volScaleR",(int)((oldFlags>>8)&127));
      break;
    case DIV_SYSTEM_OPLL:
    case DIV_SYSTEM_OPLL_DRUMS:
      switch (oldFlags&15) {
        case 0:
          newFlags.set("clockSel",0);
          break;
        case 1:
          newFlags.set("clockSel",1);
          break;
        case 2:
          newFlags.set("clockSel",2);
          break;
        case 3:
          newFlags.set("clockSel",3);
          break;
      }
      switch (oldFlags>>4) {
        case 0:
          newFlags.set("patchSet",0);
          break;
        case 1:
          newFlags.set("patchSet",1);
          break;
        case 2:
          newFlags.set("patchSet",2);
          break;
        case 3:
          newFlags.set("patchSet",3);
          break;
      }
      break;
    case DIV_SYSTEM_N163:
      switch (oldFlags&15) {
        case 0:
          newFlags.set("clockSel",0);
          break;
        case 1:
          newFlags.set("clockSel",1);
          break;
        case 2:
          newFlags.set("clockSel",2);
          break;
      }
      newFlags.set("channels",(int)((oldFlags>>4)&7));
      if (oldFlags&128) newFlags.set("multiplex",true);
      break;
    case DIV_SYSTEM_YM2203:
    case DIV_SYSTEM_YM2203_EXT:
      switch (oldFlags&31) {
        case 0:
          newFlags.set("clockSel",0);
          break;
        case 1:
          newFlags.set("clockSel",1);
          break;
        case 2:
          newFlags.set("clockSel",2);
          break;
        case 3:
          newFlags.set("clockSel",3);
          break;
        case 4:
          newFlags.set("clockSel",4);
          break;
        case 5:
          newFlags.set("clockSel",5);
          break;
      }
      switch ((oldFlags>>5)&3) {
        case 0:
          newFlags.set("prescale",0);
          break;
        case 1:
          newFlags.set("prescale",1);
          break;
        case 2:
          newFlags.set("prescale",2);
          break;
      }
      break;
    case DIV_SYSTEM_YM2608:
    case DIV_SYSTEM_YM2608_EXT:
      switch (oldFlags&31) {
        case 0:
          newFlags.set("clockSel",0);
          break;
        case 1:
          newFlags.set("clockSel",1);
          break;
      }
      switch ((oldFlags>>5)&3) {
        case 0:
          newFlags.set("prescale",0);
          break;
        case 1:
          newFlags.set("prescale",1);
          break;
        case 2:
          newFlags.set("prescale",2);
          break;
      }
      break;
    case DIV_SYSTEM_OPL:
    case DIV_SYSTEM_OPL2:
    case DIV_SYSTEM_Y8950:
    case DIV_SYSTEM_OPL_DRUMS:
    case DIV_SYSTEM_OPL2_DRUMS:
    case DIV_SYSTEM_Y8950_DRUMS:
    case DIV_SYSTEM_YMZ280B:
      switch (oldFlags&0xff) {
        case 0:
          newFlags.set("clockSel",0);
          break;
        case 1:
          newFlags.set("clockSel",1);
          break;
        case 2:
          newFlags.set("clockSel",2);
          break;
        case 3:
          newFlags.set("clockSel",3);
          break;
        case 4:
          newFlags.set("clockSel",4);
          break;
        case 5:
          newFlags.set("clockSel",5);
          break;
      }
      break;
    case DIV_SYSTEM_OPL3:
    case DIV_SYSTEM_OPL3_DRUMS:
      switch (oldFlags&0xff) {
        case 0:
          newFlags.set("clockSel",0);
          break;
        case 1:
          newFlags.set("clockSel",1);
          break;
        case 2:
          newFlags.set("clockSel",2);
          break;
        case 3:
          newFlags.set("clockSel",3);
          break;
        case 4:
          newFlags.set("clockSel",4);
          break;
      }
      break;
    case DIV_SYSTEM_PCSPKR:
      newFlags.set("speakerType",(int)(oldFlags&3));
      break;
    case DIV_SYSTEM_RF5C68:
      switch (oldFlags&15) {
        case 0:
          newFlags.set("clockSel",0);
          break;
        case 1:
          newFlags.set("clockSel",1);
          break;
        case 2:
          newFlags.set("clockSel",2);
          break;
      }
      switch (oldFlags>>4) {
        case 0:
          newFlags.set("chipType",0);
          break;
        case 1:
          newFlags.set("chipType",1);
          break;
      }
      break;
    case DIV_SYSTEM_VRC7:
      switch (oldFlags&15) {
        case 0:
          newFlags.set("clockSel",0);
          break;
        case 1:
          newFlags.set("clockSel",1);
          break;
        case 2:
          newFlags.set("clockSel",2);
          break;
        case 3:
          newFlags.set("clockSel",3);
          break;
      }
      break;
    case DIV_SYSTEM_SFX_BEEPER:
    case DIV_SYSTEM_SFX_BEEPER_QUADTONE:
      newFlags.set("clockSel",(int)(oldFlags&1));
      break;
    case DIV_SYSTEM_SCC:
    case DIV_SYSTEM_SCC_PLUS:
      switch (oldFlags&63) {
        case 0:
          newFlags.set("clockSel",0);
          break;
        case 1:
          newFlags.set("clockSel",1);
          break;
        case 2:
          newFlags.set("clockSel",2);
          break;
        case 3:
          newFlags.set("clockSel",3);
          break;
      }
      break;
    case DIV_SYSTEM_MSM6295:
      switch (oldFlags&63) {
        case 0:
          newFlags.set("clockSel",0);
          break;
        case 1:
          newFlags.set("clockSel",1);
          break;
        case 2:
          newFlags.set("clockSel",2);
          break;
        case 3:
          newFlags.set("clockSel",3);
          break;
        case 4:
          newFlags.set("clockSel",4);
          break;
        case 5:
          newFlags.set("clockSel",5);
          break;
        case 6:
          newFlags.set("clockSel",6);
          break;
        case 7:
          newFlags.set("clockSel",7);
          break;
        case 8:
          newFlags.set("clockSel",8);
          break;
        case 9:
          newFlags.set("clockSel",9);
          break;
        case 10:
          newFlags.set("clockSel",10);
          break;
        case 11:
          newFlags.set("clockSel",11);
          break;
        case 12:
          newFlags.set("clockSel",12);
          break;
        case 13:
          newFlags.set("clockSel",13);
          break;
        case 14:
          newFlags.set("clockSel",14);
          break;
      }
      if (oldFlags&128) newFlags.set("rateSel",true);
      break;
    case DIV_SYSTEM_MSM6258:
      switch (oldFlags) {
        case 0:
          newFlags.set("clockSel",0);
          break;
        case 1:
          newFlags.set("clockSel",1);
          break;
        case 2:
          newFlags.set("clockSel",2);
          break;
        case 3:
          newFlags.set("clockSel",3);
          break;
      }
      break;
    case DIV_SYSTEM_OPL4:
    case DIV_SYSTEM_OPL4_DRUMS:
      switch (oldFlags&0xff) {
        case 0:
          newFlags.set("clockSel",0);
          break;
        case 1:
          newFlags.set("clockSel",1);
          break;
        case 2:
          newFlags.set("clockSel",2);
          break;
      }
      break;
    case DIV_SYSTEM_X1_010:
      switch (oldFlags&15) {
        case 0:
          newFlags.set("clockSel",0);
          break;
        case 1:
          newFlags.set("clockSel",1);
          break;
      }
      if (oldFlags&16) newFlags.set("stereo",true);
      break;
    case DIV_SYSTEM_SOUND_UNIT:
      newFlags.set("clockSel",(int)(oldFlags&1));
      if (oldFlags&4) newFlags.set("echo",true);
      if (oldFlags&8) newFlags.set("swapEcho",true);
      newFlags.set("sampleMemSize",(int)((oldFlags>>4)&1));
      if (oldFlags&32) newFlags.set("pdm",true);
      newFlags.set("echoDelay",(int)((oldFlags>>8)&63));
      newFlags.set("echoFeedback",(int)((oldFlags>>16)&15));
      newFlags.set("echoResolution",(int)((oldFlags>>20)&15));
      newFlags.set("echoVol",(int)((oldFlags>>24)&255));
      break;
    case DIV_SYSTEM_PCM_DAC:
      if (!oldFlags) oldFlags=0x1f0000|44099;
      newFlags.set("rate",(int)((oldFlags&0xffff)+1));
      newFlags.set("outDepth",(int)((oldFlags>>16)&15));
      if (oldFlags&0x100000) newFlags.set("stereo",true);
      break;
    case DIV_SYSTEM_QSOUND:
      newFlags.set("echoDelay",(int)(oldFlags&0xfff));
      newFlags.set("echoFeedback",(int)((oldFlags>>12)&255));
      break;
    default:
      break;
  }
}

#define READ_ELEMENT_PTRS(_p) { \
  unsigned int numElements=reader.readI(); \
  for (unsigned int i=0; i<numElements; i++) { \
    _p.push_back(reader.readI()); \
  } \
}

#define READ_ELEMENT_UNIQUE(_p) { \
  unsigned int numElements=reader.readI(); \
  if (numElements!=1) { \
    logE("unique element is present more than once!"); \
    lastError="unique element is present more than once!"; \
    delete[] file; \
    return false; \
  } \
  _p=reader.readI(); \
}

bool DivEngine::loadFur(unsigned char* file, size_t len, int variantID) {
  std::vector<unsigned int> insPtr;
  std::vector<unsigned int> wavePtr;
  std::vector<unsigned int> samplePtr;
  std::vector<unsigned int> subSongPtr;
  std::vector<unsigned int> sysFlagsPtr;
  std::vector<unsigned int> assetDirPtr;
  unsigned int compatFlagPtr=0;
  unsigned int commentPtr=0;
  std::vector<unsigned int> groovePtr;
  std::vector<unsigned int> patPtr;
  int tchans=0;
  char magic[5];
  memset(magic,0,5);
  SafeReader reader=SafeReader(file,len);
  warnings="";

  try {
    DivSong ds;
    DivSubSong* subSong=ds.subsong[0];

    /// HEADER
    if (!reader.seek(16,SEEK_SET)) {
      logE("premature end of file!");
      lastError="incomplete file";
      delete[] file;
      return false;
    }
    ds.version=reader.readS();
    logI("module version %d (0x%.2x)",ds.version,ds.version);

    if (variantID!=DIV_FUR_VARIANT_VANILLA) {
      logW("Furnace variant detected: %d",variantID);
      addWarning("this module was created with a downstream version of Furnace. certain features may not be compatible.");
    }

    if (ds.version>DIV_ENGINE_VERSION) {
      logW("this module was created with a more recent version of Furnace!");
      addWarning("this module was created with a more recent version of Furnace!");
    }

    // version-related compat flags
    if (ds.version<37) { // compat flags not stored back then
      ds.compatFlags.limitSlides=true;
      ds.compatFlags.linearPitch=1;
      ds.compatFlags.loopModality=0;
    }
    if (ds.version<43) {
      ds.compatFlags.properNoiseLayout=false;
      ds.compatFlags.waveDutyIsVol=false;
    }
    if (ds.version<45) {
      ds.compatFlags.resetMacroOnPorta=true;
      ds.compatFlags.legacyVolumeSlides=true;
      ds.compatFlags.compatibleArpeggio=true;
      ds.compatFlags.noteOffResetsSlides=true;
      ds.compatFlags.targetResetsSlides=true;
    }
    if (ds.version<46) {
      ds.compatFlags.arpNonPorta=true;
      ds.compatFlags.algMacroBehavior=true;
    } else {
      ds.compatFlags.arpNonPorta=false;
      ds.compatFlags.algMacroBehavior=false;
    }
    if (ds.version<49) {
      ds.compatFlags.brokenShortcutSlides=true;
    }
    if (ds.version<50) {
      ds.compatFlags.ignoreDuplicateSlides=false;
    }
    if (ds.version<62) {
      ds.compatFlags.stopPortaOnNoteOff=true;
    }
    if (ds.version<64) {
      ds.compatFlags.brokenDACMode=false;
    }
    if (ds.version<65) {
      ds.compatFlags.oneTickCut=false;
    }
    if (ds.version<66) {
      ds.compatFlags.newInsTriggersInPorta=false;
    }
    if (ds.version<69) {
      ds.compatFlags.arp0Reset=false;
    }
    if (ds.version<71) {
      ds.compatFlags.noSlidesOnFirstTick=false;
      ds.compatFlags.rowResetsArpPos=false;
      ds.compatFlags.ignoreJumpAtEnd=true;
    }
    if (ds.version<72) {
      ds.compatFlags.buggyPortaAfterSlide=true;
      ds.compatFlags.gbInsAffectsEnvelope=false;
    }
    if (ds.version<78) {
      ds.compatFlags.sharedExtStat=false;
    }
    if (ds.version<83) {
      ds.compatFlags.ignoreDACModeOutsideIntendedChannel=true;
      ds.compatFlags.e1e2AlsoTakePriority=false;
    }
    if (ds.version<84) {
      ds.compatFlags.newSegaPCM=false;
    }
    if (ds.version<85) {
      ds.compatFlags.fbPortaPause=true;
    }
    if (ds.version<86) {
      ds.compatFlags.snDutyReset=true;
    }
    if (ds.version<90) {
      ds.compatFlags.pitchMacroIsLinear=false;
    }
    if (ds.version<97) {
      ds.compatFlags.oldOctaveBoundary=true;
    }
    if (ds.version<97) { // actually should be 98 but yky uses this feature ahead of time
      ds.compatFlags.noOPN2Vol=true;
    }
    if (ds.version<99) {
      ds.compatFlags.newVolumeScaling=false;
      ds.compatFlags.volMacroLinger=false;
      ds.compatFlags.brokenOutVol=true;
    }
    if (ds.version<100) {
      ds.compatFlags.e1e2StopOnSameNote=false;
    }
    if (ds.version<101) {
      ds.compatFlags.brokenPortaArp=true;
    }
    if (ds.version<108) {
      ds.compatFlags.snNoLowPeriods=true;
    }
    if (ds.version<110) {
      ds.compatFlags.delayBehavior=1;
    }
    if (ds.version<113) {
      ds.compatFlags.jumpTreatment=1;
    }
    if (ds.version<115) {
      ds.autoSystem=false;
    }
    if (ds.version<117) {
      ds.compatFlags.disableSampleMacro=true;
    }
    if (ds.version<121) {
      ds.compatFlags.brokenOutVol2=false;
    }
    if (ds.version<130) {
      ds.compatFlags.oldArpStrategy=true;
    }
    if (ds.version<138) {
      ds.compatFlags.brokenPortaLegato=true;
    }
    if (ds.version<155) {
      ds.compatFlags.brokenFMOff=true;
    }
    if (ds.version<168) {
      ds.compatFlags.preNoteNoEffect=true;
    }
    if (ds.version<183) {
      ds.compatFlags.oldDPCM=true;
    }
    if (ds.version<184) {
      ds.compatFlags.resetArpPhaseOnNewNote=false;
    }
    if (ds.version<188) {
      ds.compatFlags.ceilVolumeScaling=false;
    }
    if (ds.version<191) {
      ds.compatFlags.oldAlwaysSetVolume=true;
    }
    if (ds.version<200) {
      ds.compatFlags.oldSampleOffset=true;
    }
    ds.isDMF=false;

    reader.readS(); // reserved
    int infoSeek=reader.readI();

    /// SONG INFO
    if (!reader.seek(infoSeek,SEEK_SET)) {
      logE("couldn't seek to info header at %d!",infoSeek);
      lastError="couldn't seek to info header!";
      delete[] file;
      return false;
    }

    if (ds.version>=240) {
      // read header (NEW)
      reader.read(magic,4);
      if (strcmp(magic,"INF2")!=0) {
        logE("invalid info header! (new)");
        lastError="invalid info header! (new)";
        delete[] file;
        return false;
      }
      reader.readI();

      // clear all sub-songs
      for (DivSubSong* i: ds.subsong) {
        delete i;
      }
      ds.subsong.clear();
      subSong=NULL;

      // song information
      ds.name=reader.readString();
      ds.author=reader.readString();
      ds.systemName=reader.readString();
      ds.category=reader.readString();
      ds.nameJ=reader.readString();
      ds.authorJ=reader.readString();
      ds.systemNameJ=reader.readString();
      ds.categoryJ=reader.readString();
      logI("%s by %s",ds.name.c_str(),ds.author.c_str());
      
      ds.tuning=reader.readF();
      ds.autoSystem=reader.readC();

      // system definition
      ds.masterVol=reader.readF();
      ds.chans=(unsigned short)reader.readS();
      ds.systemLen=(unsigned short)reader.readS();

      if (ds.systemLen<1) {
        logE("zero chips!");
        lastError="zero chips!";
        delete[] file;
        return false;
      }

      // TODO: remove after implementing dynamic stuff
      for (int i=0; i<DIV_MAX_CHIPS; i++) {
        ds.system[i]=DIV_SYSTEM_NULL;
      }
      
      logD("chips: (%d, %d channels)",ds.systemLen,ds.chans);
      for (int i=0; i<ds.systemLen; i++) {
        unsigned short sysID=reader.readS();
        if (sysID>0xff || sysID==0) {
          logE("unrecognized system ID %.4x",sysID);
          lastError=fmt::sprintf("unrecognized system ID %.4x!",sysID);
          delete[] file;
          return false;
        }
        ds.system[i]=systemFromFileFur(sysID);
        if (ds.system[i]==DIV_SYSTEM_NULL) {
          logE("unrecognized system ID %.4x",sysID);
          lastError=fmt::sprintf("unrecognized system ID %.4x!",sysID);
          delete[] file;
          return false;
        }

        // reject compound systems
        const DivSysDef* sysDef=getSystemDef(ds.system[i]);
        if (sysDef==NULL) {
          logE("no definition for system ID %.4x",sysID);
          lastError=fmt::sprintf("no definition for system ID %.4x!",sysID);
          delete[] file;
          return false;
        }

        if (sysDef->isCompound) {
          logE("system ID %.4x is compound",sysID);
          lastError=fmt::sprintf("system ID %.4x is compound!",sysID);
          delete[] file;
          return false;
        }

        ds.systemChans[i]=(unsigned short)reader.readS();
        tchans+=ds.systemChans[i];

        logD("- %d: %.2x (%s, %d channels)",i,sysID,getSystemName(ds.system[i]),ds.systemChans[i]);

        if (ds.systemChans[i]<1) {
          logE("invalid channel count for chip");
          lastError=fmt::sprintf("invalid channel count for chip!");
          delete[] file;
          return false;
        }

        ds.systemVol[i]=reader.readF();
        ds.systemPan[i]=reader.readF();
        ds.systemPanFR[i]=reader.readF();
      }

      if (ds.chans!=tchans) {
        logE("chip channel counts and total channels do not match");
        lastError=fmt::sprintf("chip channel counts and total channels do not match!");
        delete[] file;
        return false;
      }

      // patchbay
      unsigned int conns=reader.readI();
      if (conns>0) ds.patchbay.reserve(conns);
      for (unsigned int i=0; i<conns; i++) {
        ds.patchbay.push_back((unsigned int)reader.readI());
      }
      ds.patchbayAuto=reader.readC();

      // elements
      bool hasElement=true;
      bool seenElement[DIV_ELEMENT_MAX];
      memset(seenElement,0,DIV_ELEMENT_MAX*sizeof(bool));
      logD("elements present in this song:");
      while (hasElement) {
        DivFileElementType elementType=(DivFileElementType)reader.readC();
        if (elementType<DIV_ELEMENT_MAX) {
          if (seenElement[elementType]) {
            logE("duplicate element type!");
            lastError="duplicate element type!";
            delete[] file;
            return false;
          } else {
            seenElement[elementType]=true;
          }
        }
        switch (elementType) {
          case DIV_ELEMENT_SUBSONG:
            logD("- sub-songs");
            READ_ELEMENT_PTRS(subSongPtr);
            break;
          case DIV_ELEMENT_CHIP_FLAGS:
            logD("- chip flags");
            READ_ELEMENT_PTRS(sysFlagsPtr);
            if (sysFlagsPtr.size()!=ds.systemLen) {
              logE("more chip flag pointers than there should be");
              lastError=fmt::sprintf("more chip flag pointers than there should be!");
              delete[] file;
              return false;
            }
            break;
          case DIV_ELEMENT_ASSET_DIR:
            logD("- asset dirs");
            READ_ELEMENT_PTRS(assetDirPtr);
            break;
          case DIV_ELEMENT_INSTRUMENT:
            logD("- instruments");
            READ_ELEMENT_PTRS(insPtr);
            if (insPtr.size()>256) {
              logE("invalid instrument count!");
              lastError="invalid instrument count!";
              delete[] file;
              return false;
            }
            ds.insLen=insPtr.size();
            break;
          case DIV_ELEMENT_WAVETABLE:
            logD("- wavetables");
            READ_ELEMENT_PTRS(wavePtr);
            if (wavePtr.size()>32768) {
              logE("invalid wavetable count!");
              lastError="invalid wavetable count!";
              delete[] file;
              return false;
            }
            ds.waveLen=wavePtr.size();
            break;
          case DIV_ELEMENT_SAMPLE:
            logD("- samples");
            READ_ELEMENT_PTRS(samplePtr);
            if (samplePtr.size()>32768) {
              logE("invalid sample count!");
              lastError="invalid sample count!";
              delete[] file;
              return false;
            }
            ds.sampleLen=samplePtr.size();
            break;
          case DIV_ELEMENT_PATTERN:
            logD("- patterns");
            READ_ELEMENT_PTRS(patPtr);
            break;
          case DIV_ELEMENT_COMPAT_FLAGS:
            logD("- compat flags");
            READ_ELEMENT_UNIQUE(compatFlagPtr);
            break;
          case DIV_ELEMENT_COMMENTS:
            logD("- song comments");
            READ_ELEMENT_UNIQUE(commentPtr);
            break;
          case DIV_ELEMENT_GROOVE:
            logD("- grooves");
            READ_ELEMENT_PTRS(groovePtr);
            break;
          case DIV_ELEMENT_END:
            hasElement=false;
            break;
          default: {
            logD("- UNKNOWN");
            // skip element
            unsigned int numElements=reader.readI();
            for (unsigned int i=0; i<numElements; i++) {
              reader.readI();
            }
            break;
          }
        }
      }

      if (subSongPtr.empty()) {
        logE("song is empty!");
        lastError="song is empty!";
        delete[] file;
        return false;
      }
    } else {
      // read header (OLD)
      reader.read(magic,4);
      if (strcmp(magic,"INFO")!=0) {
        logE("invalid info header! (old)");
        lastError="invalid info header! (old)";
        delete[] file;
        return false;
      }
      reader.readI();

      unsigned char oldTimeBase=reader.readC();
      subSong->speeds.len=2;
      subSong->speeds.val[0]=(unsigned char)reader.readC();
      subSong->speeds.val[1]=(unsigned char)reader.readC();
      subSong->arpLen=reader.readC();
      subSong->hz=reader.readF();

      subSong->patLen=reader.readS();
      subSong->ordersLen=reader.readS();

      subSong->hilightA=reader.readC();
      subSong->hilightB=reader.readC();

      ds.insLen=reader.readS();
      ds.waveLen=reader.readS();
      ds.sampleLen=reader.readS();
      int numberOfPats=reader.readI();

      if (subSong->patLen<0) {
        logE("pattern length is negative!");
        lastError="pattern length is negative!";
        delete[] file;
        return false;
      }
      if (subSong->patLen>DIV_MAX_ROWS) {
        logE("pattern length is too large!");
        lastError="pattern length is too large!";
        delete[] file;
        return false;
      }
      if (subSong->ordersLen<0) {
        logE("song length is negative!");
        lastError="song length is negative!";
        delete[] file;
        return false;
      }
      if (subSong->ordersLen>DIV_MAX_PATTERNS) {
        logE("song is too long!");
        lastError="song is too long!";
        delete[] file;
        return false;
      }
      if (ds.insLen<0 || ds.insLen>256) {
        logE("invalid instrument count!");
        lastError="invalid instrument count!";
        delete[] file;
        return false;
      }
      if (ds.waveLen<0 || ds.waveLen>32768) {
        logE("invalid wavetable count!");
        lastError="invalid wavetable count!";
        delete[] file;
        return false;
      }
      if (ds.sampleLen<0 || ds.sampleLen>32768) {
        logE("invalid sample count!");
        lastError="invalid sample count!";
        delete[] file;
        return false;
      }
      if (numberOfPats<0) {
        logE("invalid pattern count!");
        lastError="invalid pattern count!";
        delete[] file;
        return false;
      }

      logD("systems:");
      ds.systemLen=0;
      for (int i=0; i<DIV_MAX_CHIPS; i++) {
        unsigned char sysID=reader.readC();
        ds.system[i]=systemFromFileFur(sysID);
        ds.systemChans[i]=getChannelCount(ds.system[i]);
        logD("- %d: %.2x (%s, %d channels)",i,sysID,getSystemName(ds.system[i]),ds.systemChans[i]);
        if (sysID!=0 && systemToFileFur(ds.system[i])==0) {
          logE("unrecognized system ID %.2x",sysID);
          lastError=fmt::sprintf("unrecognized system ID %.2x!",sysID);
          delete[] file;
          return false;
        }
        if (ds.system[i]!=DIV_SYSTEM_NULL) ds.systemLen=i+1;
      }

      for (int i=0; i<ds.systemLen; i++) {
        tchans+=ds.systemChans[i];
      }
      if (tchans>DIV_MAX_CHANS) {
        tchans=DIV_MAX_CHANS;
        logW("too many channels!");
      }
      logV("system len: %d",ds.systemLen);
      if (ds.systemLen<1) {
        logE("zero chips!");
        lastError="zero chips!";
        delete[] file;
        return false;
      }

      // system volume
      for (int i=0; i<DIV_MAX_CHIPS; i++) {
        signed char oldSysVol=reader.readC();
        ds.systemVol[i]=(float)oldSysVol/64.0f;
        if (ds.version<59 && ds.system[i]==DIV_SYSTEM_NES) {
          ds.systemVol[i]/=4;
        }
      }

      // system panning
      for (int i=0; i<DIV_MAX_CHIPS; i++) {
        signed char oldSysPan=reader.readC();
        ds.systemPan[i]=(float)oldSysPan/127.0f;
      }

      // system props
      for (int i=0; i<DIV_MAX_CHIPS; i++) {
        sysFlagsPtr.push_back(reader.readI());
      }

      // handle compound systems
      for (int i=0; i<DIV_MAX_CHIPS; i++) {
        if (ds.system[i]==DIV_SYSTEM_GENESIS ||
            ds.system[i]==DIV_SYSTEM_GENESIS_EXT ||
            ds.system[i]==DIV_SYSTEM_ARCADE) {
          for (int j=31; j>i; j--) {
            ds.system[j]=ds.system[j-1];
            ds.systemVol[j]=ds.systemVol[j-1];
            ds.systemPan[j]=ds.systemPan[j-1];
          }
          if (++ds.systemLen>DIV_MAX_CHIPS) ds.systemLen=DIV_MAX_CHIPS;

          if (ds.system[i]==DIV_SYSTEM_GENESIS) {
            ds.system[i]=DIV_SYSTEM_YM2612;
            if (i<31) {
              ds.system[i+1]=DIV_SYSTEM_SMS;
              ds.systemVol[i+1]=ds.systemVol[i]*0.375f;
            }
          }
          if (ds.system[i]==DIV_SYSTEM_GENESIS_EXT) {
            ds.system[i]=DIV_SYSTEM_YM2612_EXT;
            if (i<31) {
              ds.system[i+1]=DIV_SYSTEM_SMS;
              ds.systemVol[i+1]=ds.systemVol[i]*0.375f;
            }
          }
          if (ds.system[i]==DIV_SYSTEM_ARCADE) {
            ds.system[i]=DIV_SYSTEM_YM2151;
            if (i<31) {
              ds.system[i+1]=DIV_SYSTEM_SEGAPCM_COMPAT;
            }
          }
          i++;
        }
      }

      ds.initDefaultSystemChans();
      ds.chans=tchans;

      // flatten 5-channel SegaPCM and Neo Geo CD
      for (int i=0; i<ds.systemLen; i++) {
        if (ds.system[i]==DIV_SYSTEM_SEGAPCM_COMPAT) {
          ds.system[i]=DIV_SYSTEM_SEGAPCM;
        } else if (ds.system[i]==DIV_SYSTEM_YM2610_CRAP) {
          ds.system[i]=DIV_SYSTEM_YM2610_FULL;
        } else if (ds.system[i]==DIV_SYSTEM_YM2610_CRAP_EXT) {
          ds.system[i]=DIV_SYSTEM_YM2610_FULL_EXT;
        }
      }

      ds.name=reader.readString();
      ds.author=reader.readString();
      logI("%s by %s",ds.name.c_str(),ds.author.c_str());

      if (ds.version>=33) {
        ds.tuning=reader.readF();
      } else {
        reader.readI();
      }

      // compatibility flags
      if (ds.version>=37) {
        ds.compatFlags.limitSlides=reader.readC();
        ds.compatFlags.linearPitch=reader.readC();
        ds.compatFlags.loopModality=reader.readC();
        if (ds.version>=43) {
          ds.compatFlags.properNoiseLayout=reader.readC();
        } else {
          reader.readC();
        }
        if (ds.version>=43) {
          ds.compatFlags.waveDutyIsVol=reader.readC();
        } else {
          reader.readC();
        }

        if (ds.version>=45) {
          ds.compatFlags.resetMacroOnPorta=reader.readC();
        } else {
          reader.readC();
        }
        if (ds.version>=45) {
          ds.compatFlags.legacyVolumeSlides=reader.readC();
        } else {
          reader.readC();
        }
        if (ds.version>=45) {
          ds.compatFlags.compatibleArpeggio=reader.readC();
        } else {
          reader.readC();
        }
        if (ds.version>=45) {
          ds.compatFlags.noteOffResetsSlides=reader.readC();
        } else {
          reader.readC();
        }
        if (ds.version>=45) {
          ds.compatFlags.targetResetsSlides=reader.readC();
        } else {
          reader.readC();
        }
        if (ds.version>=47) {
          ds.compatFlags.arpNonPorta=reader.readC();
        } else {
          reader.readC();
        }
        if (ds.version>=47) {
          ds.compatFlags.algMacroBehavior=reader.readC();
        } else {
          reader.readC();
        }
        if (ds.version>=49) {
          ds.compatFlags.brokenShortcutSlides=reader.readC();
        } else {
          reader.readC();
        }
        if (ds.version>=50) {
          ds.compatFlags.ignoreDuplicateSlides=reader.readC();
        } else {
          reader.readC();
        }
        if (ds.version>=62) {
          ds.compatFlags.stopPortaOnNoteOff=reader.readC();
          ds.compatFlags.continuousVibrato=reader.readC();
        } else {
          reader.readC();
          reader.readC();
        }
        if (ds.version>=64) {
          ds.compatFlags.brokenDACMode=reader.readC();
        } else {
          reader.readC();
        }
        if (ds.version>=65) {
          ds.compatFlags.oneTickCut=reader.readC();
        } else {
          reader.readC();
        }
        if (ds.version>=66) {
          ds.compatFlags.newInsTriggersInPorta=reader.readC();
        } else {
          reader.readC();
        }
        if (ds.version>=69) {
          ds.compatFlags.arp0Reset=reader.readC();
        } else {
          reader.readC();
        }
      } else {
        for (int i=0; i<20; i++) reader.readC();
      }

      // pointers
      insPtr.reserve(ds.insLen);
      for (int i=0; i<ds.insLen; i++) {
        insPtr.push_back(reader.readI());
      }
      wavePtr.reserve(ds.waveLen);
      for (int i=0; i<ds.waveLen; i++) {
        wavePtr.push_back(reader.readI());
      }
      samplePtr.reserve(ds.sampleLen);
      for (int i=0; i<ds.sampleLen; i++) {
        samplePtr.push_back(reader.readI());
      }
      patPtr.reserve(numberOfPats);
      for (int i=0; i<numberOfPats; i++) patPtr.push_back(reader.readI());

      logD("reading orders (%d)...",subSong->ordersLen);
      for (int i=0; i<tchans; i++) {
        for (int j=0; j<subSong->ordersLen; j++) {
          subSong->orders.ord[i][j]=reader.readC();
        }
      }

      for (int i=0; i<tchans; i++) {
        subSong->pat[i].effectCols=reader.readC();
        if (subSong->pat[i].effectCols<1 || subSong->pat[i].effectCols>DIV_MAX_EFFECTS) {
          logE("channel %d has zero or too many effect columns! (%d)",i,subSong->pat[i].effectCols);
          lastError=fmt::sprintf("channel %d has too many effect columns! (%d)",i,subSong->pat[i].effectCols);
          delete[] file;
          return false;
        }
      }

      if (ds.version>=39) {
        for (int i=0; i<tchans; i++) {
          if (ds.version<189) {
            subSong->chanShow[i]=reader.readC();
            subSong->chanShowChanOsc[i]=subSong->chanShow[i];
          } else {
            unsigned char tempchar=reader.readC();
            subSong->chanShow[i]=tempchar&1;
            subSong->chanShowChanOsc[i]=(tempchar&2);
          }
        }

        for (int i=0; i<tchans; i++) {
          subSong->chanCollapse[i]=reader.readC();
        }

        if (ds.version<92) {
          for (int i=0; i<tchans; i++) {
            if (subSong->chanCollapse[i]>0) subSong->chanCollapse[i]=3;
          }
        }

        for (int i=0; i<tchans; i++) {
          subSong->chanName[i]=reader.readString();
        }

        for (int i=0; i<tchans; i++) {
          subSong->chanShortName[i]=reader.readString();
        }

        ds.notes=reader.readString();
      }

      if (ds.version>=59) {
        ds.masterVol=reader.readF();
      } else {
        ds.masterVol=2.0f;
      }

      if (ds.version>=70) {
        // extended compat flags
        ds.compatFlags.brokenSpeedSel=reader.readC();
        if (ds.version>=71) {
          ds.compatFlags.noSlidesOnFirstTick=reader.readC();
          ds.compatFlags.rowResetsArpPos=reader.readC();
          ds.compatFlags.ignoreJumpAtEnd=reader.readC();
        } else {
          reader.readC();
          reader.readC();
          reader.readC();
        }
        if (ds.version>=72) {
          ds.compatFlags.buggyPortaAfterSlide=reader.readC();
          ds.compatFlags.gbInsAffectsEnvelope=reader.readC();
        } else {
          reader.readC();
          reader.readC();
        }
        if (ds.version>=78) {
          ds.compatFlags.sharedExtStat=reader.readC();
        } else {
          reader.readC();
        }
        if (ds.version>=83) {
          ds.compatFlags.ignoreDACModeOutsideIntendedChannel=reader.readC();
          ds.compatFlags.e1e2AlsoTakePriority=reader.readC();
        } else {
          reader.readC();
          reader.readC();
        }
        if (ds.version>=84) {
          ds.compatFlags.newSegaPCM=reader.readC();
        } else {
          reader.readC();
        }
        if (ds.version>=85) {
          ds.compatFlags.fbPortaPause=reader.readC();
        } else {
          reader.readC();
        }
        if (ds.version>=86) {
          ds.compatFlags.snDutyReset=reader.readC();
        } else {
          reader.readC();
        }
        if (ds.version>=90) {
          ds.compatFlags.pitchMacroIsLinear=reader.readC();
        } else {
          reader.readC();
        }
        if (ds.version>=94) {
          ds.compatFlags.pitchSlideSpeed=reader.readC();
        } else {
          reader.readC();
        }
        if (ds.version>=97) {
          ds.compatFlags.oldOctaveBoundary=reader.readC();
        } else {
          reader.readC();
        }
        if (ds.version>=98) {
          ds.compatFlags.noOPN2Vol=reader.readC();
        } else {
          reader.readC();
        }
        if (ds.version>=99) {
          ds.compatFlags.newVolumeScaling=reader.readC();
          ds.compatFlags.volMacroLinger=reader.readC();
          ds.compatFlags.brokenOutVol=reader.readC();
        } else {
          reader.readC();
          reader.readC();
          reader.readC();
        }
        if (ds.version>=100) {
          ds.compatFlags.e1e2StopOnSameNote=reader.readC();
        } else {
          reader.readC();
        }
        if (ds.version>=101) {
          ds.compatFlags.brokenPortaArp=reader.readC();
        } else {
          reader.readC();
        }
        if (ds.version>=108) {
          ds.compatFlags.snNoLowPeriods=reader.readC();
        } else {
          reader.readC();
        }
        if (ds.version>=110) {
          ds.compatFlags.delayBehavior=reader.readC();
        } else {
          reader.readC();
        }
        if (ds.version>=113) {
          ds.compatFlags.jumpTreatment=reader.readC();
        } else {
          reader.readC();
        }
        if (ds.version>=115) {
          ds.autoSystem=reader.readC();
        } else {
          reader.readC();
        }
        if (ds.version>=117) {
          ds.compatFlags.disableSampleMacro=reader.readC();
        } else {
          reader.readC();
        }
        if (ds.version>=121) {
          ds.compatFlags.brokenOutVol2=reader.readC();
        } else {
          reader.readC();
        }
        if (ds.version>=130) {
          ds.compatFlags.oldArpStrategy=reader.readC();
        } else {
          reader.readC();
        }
      }

      // first song virtual tempo
      if (ds.version>=96) {
        subSong->virtualTempoN=reader.readS();
        subSong->virtualTempoD=reader.readS();
      } else {
        reader.readI();
      }

      // subsongs
      if (ds.version>=95) {
        subSong->name=reader.readString();
        subSong->notes=reader.readString();
        int numberOfSubSongs=(unsigned char)reader.readC();
        reader.readC(); // reserved
        reader.readC();
        reader.readC();
        // pointers
        for (int i=0; i<numberOfSubSongs; i++) {
          subSongPtr.push_back(reader.readI());
        }
      }

      // additional metadata
      if (ds.version>=103) {
        ds.systemName=reader.readString();
        ds.category=reader.readString();
        ds.nameJ=reader.readString();
        ds.authorJ=reader.readString();
        ds.systemNameJ=reader.readString();
        ds.categoryJ=reader.readString();
      } else {
        ds.systemName=getSongSystemLegacyName(ds,!getConfInt("noMultiSystem",0));
        ds.autoSystem=true;
      }

      // system output config
      if (ds.version>=135) {
        for (int i=0; i<ds.systemLen; i++) {
          ds.systemVol[i]=reader.readF();
          ds.systemPan[i]=reader.readF();
          ds.systemPanFR[i]=reader.readF();
        }

        // patchbay
        unsigned int conns=reader.readI();
        if (conns>0) ds.patchbay.reserve(conns);
        for (unsigned int i=0; i<conns; i++) {
          ds.patchbay.push_back((unsigned int)reader.readI());
        }
      }

      if (ds.version>=136) ds.patchbayAuto=reader.readC();

      if (ds.version>=138) {
        ds.compatFlags.brokenPortaLegato=reader.readC();
        if (ds.version>=155) {
          ds.compatFlags.brokenFMOff=reader.readC();
        } else {
          reader.readC();
        }
        if (ds.version>=168) {
          ds.compatFlags.preNoteNoEffect=reader.readC();
        } else {
          reader.readC();
        }
        if (ds.version>=183) {
          ds.compatFlags.oldDPCM=reader.readC();
        } else {
          reader.readC();
        }
        if (ds.version>=184) {
          ds.compatFlags.resetArpPhaseOnNewNote=reader.readC();
        } else {
          reader.readC();
        }
        if (ds.version>=188) {
          ds.compatFlags.ceilVolumeScaling=reader.readC();
        } else {
          reader.readC();
        }
        if (ds.version>=191) {
          ds.compatFlags.oldAlwaysSetVolume=reader.readC();
        } else {
          reader.readC();
        }
        if (ds.version>=200) {
          ds.compatFlags.oldSampleOffset=reader.readC();
        } else {
          reader.readC();
        }
      }

      if (ds.version>=139) {
        subSong->speeds.len=reader.readC();
        for (int i=0; i<16; i++) {
          subSong->speeds.val[i]=(unsigned char)reader.readC();
        }

        // grooves
        unsigned char grooveCount=reader.readC();
        ds.grooves.reserve(grooveCount);
        for (int i=0; i<grooveCount; i++) {
          DivGroovePattern gp;
          gp.len=(unsigned char)reader.readC();
          for (int j=0; j<16; j++) {
            gp.val[j]=(unsigned char)reader.readC();
          }

          ds.grooves.push_back(gp);
        }
      }

      for (int i=0; i<16; i++) {
        subSong->speeds.val[i]*=(oldTimeBase+1);
      }

      if (ds.version>=156) {
        assetDirPtr.push_back(reader.readI());
        assetDirPtr.push_back(reader.readI());
        assetDirPtr.push_back(reader.readI());
      }
    }

    // read compatibility flags
    if (compatFlagPtr) {
      DivConfig c;
      if (!reader.seek(compatFlagPtr,SEEK_SET)) {
        logE("couldn't seek to compat flags!");
        lastError=fmt::sprintf("couldn't seek to compat flags!");
        ds.unload();
        delete[] file;
        return false;
      }

      if (!song.compatFlags.readData(reader)) {
        logE("invalid compat flag header!");
        lastError="invalid compat flag header!";
        ds.unload();
        delete[] file;
        return false;
      }
    }

    // read song comments
    if (commentPtr) {
      if (!reader.seek(commentPtr,SEEK_SET)) {
        logE("couldn't seek to song comments!");
        lastError=fmt::sprintf("couldn't seek to song comments!");
        ds.unload();
        delete[] file;
        return false;
      }

      reader.read(magic,4);
      if (strcmp(magic,"CMNT")!=0) {
        logE("invalid comment header!");
        lastError="invalid comment header!";
        ds.unload();
        delete[] file;
        return false;
      }
      reader.readI();

      song.notes=reader.readString();
    }

    // read grooves
    ds.grooves.reserve(groovePtr.size());
    for (size_t i=0; i<groovePtr.size(); i++) {
      DivGroovePattern groove;
      if (!reader.seek(groovePtr[i],SEEK_SET)) {
        logE("couldn't seek to groove %d!",i);
        lastError=fmt::sprintf("couldn't seek to groove %d!",i);
        ds.unload();
        delete[] file;
        return false;
      }

      if (!groove.readData(reader)) {
        logE("%d: invalid groove data!",i);
        lastError="invalid groove data!";
        ds.unload();
        delete[] file;
        return false;
      }

      ds.grooves.push_back(groove);
    }

    // read system flags
    if (ds.version>=119) {
      logD("reading chip flags...");
      for (size_t i=0; i<sysFlagsPtr.size(); i++) {
        if (sysFlagsPtr[i]==0) continue;

        if (!reader.seek(sysFlagsPtr[i],SEEK_SET)) {
          logE("couldn't seek to chip %d flags!",i+1);
          lastError=fmt::sprintf("couldn't seek to chip %d flags!",i+1);
          ds.unload();
          delete[] file;
          return false;
        }

        reader.read(magic,4);
        if (strcmp(magic,"FLAG")!=0) {
          logE("%d: invalid flag header!",i);
          lastError="invalid flag header!";
          ds.unload();
          delete[] file;
          return false;
        }
        reader.readI();

        String data=reader.readString();
        ds.systemFlags[i].loadFromMemory(data.c_str());
      }
    } else {
      logD("reading old chip flags...");
      for (int i=0; i<ds.systemLen; i++) {
        convertOldFlags(sysFlagsPtr[i],ds.systemFlags[i],ds.system[i]);
      }
    }

    // read asset directories
    if (ds.version>=156) {
      logD("reading asset directories...");

      if (assetDirPtr.size()>0) {
        if (!reader.seek(assetDirPtr[0],SEEK_SET)) {
          logE("couldn't seek to ins dir!");
          lastError=fmt::sprintf("couldn't read instrument directory");
          ds.unload();
          delete[] file;
          return false;
        }
        if (readAssetDirData(reader,ds.insDir)!=DIV_DATA_SUCCESS) {
          lastError="invalid instrument directory data!";
          ds.unload();
          delete[] file;
          return false;
        }
      }

      if (assetDirPtr.size()>1) {
        if (!reader.seek(assetDirPtr[1],SEEK_SET)) {
          logE("couldn't seek to wave dir!");
          lastError=fmt::sprintf("couldn't read wavetable directory");
          ds.unload();
          delete[] file;
          return false;
        }
        if (readAssetDirData(reader,ds.waveDir)!=DIV_DATA_SUCCESS) {
          lastError="invalid wavetable directory data!";
          ds.unload();
          delete[] file;
          return false;
        }
      }

      if (assetDirPtr.size()>2) {
        if (!reader.seek(assetDirPtr[2],SEEK_SET)) {
          logE("couldn't seek to sample dir!");
          lastError=fmt::sprintf("couldn't read sample directory");
          ds.unload();
          delete[] file;
          return false;
        }
        if (readAssetDirData(reader,ds.sampleDir)!=DIV_DATA_SUCCESS) {
          lastError="invalid sample directory data!";
          ds.unload();
          delete[] file;
          return false;
        }
      }
    }

    // read subsongs
    if (ds.version>=95) {
      ds.subsong.reserve(subSongPtr.size());
      for (size_t i=0; i<subSongPtr.size(); i++) {
        subSong=new DivSubSong;
        ds.subsong.push_back(subSong);
        if (!reader.seek(subSongPtr[i],SEEK_SET)) {
          logE("couldn't seek to subsong %d!",i+1);
          lastError=fmt::sprintf("couldn't seek to subsong %d!",i+1);
          ds.unload();
          delete[] file;
          return false;
        }

        logW("ds.chans: %d",ds.chans);
        if (!subSong->readData(reader,ds.version,ds.chans)) {
          logE("%d: invalid subsong data!",i);
          lastError="invalid subsong data!";
          ds.unload();
          delete[] file;
          return false;
        }
      }
    }

    // read instruments
    ds.ins.reserve(ds.insLen);
    for (int i=0; i<ds.insLen; i++) {
      DivInstrument* ins=new DivInstrument;
      logD("reading instrument %d at %x...",i,insPtr[i]);
      if (!reader.seek(insPtr[i],SEEK_SET)) {
        logE("couldn't seek to instrument %d!",i);
        lastError=fmt::sprintf("couldn't seek to instrument %d!",i);
        ds.unload();
        delete ins;
        delete[] file;
        return false;
      }
      
      if (ins->readInsData(reader,ds.version)!=DIV_DATA_SUCCESS) {
        lastError="invalid instrument header/data!";
        ds.unload();
        delete ins;
        delete[] file;
        return false;
      }

      ds.ins.push_back(ins);
    }

    // read wavetables
    ds.wave.reserve(ds.waveLen);
    for (int i=0; i<ds.waveLen; i++) {
      DivWavetable* wave=new DivWavetable;
      logD("reading wavetable %d at %x...",i,wavePtr[i]);
      if (!reader.seek(wavePtr[i],SEEK_SET)) {
        logE("couldn't seek to wavetable %d!",i);
        lastError=fmt::sprintf("couldn't seek to wavetable %d!",i);
        ds.unload();
        delete wave;
        delete[] file;
        return false;
      }

      if (wave->readWaveData(reader,ds.version)!=DIV_DATA_SUCCESS) {
        lastError="invalid wavetable header/data!";
        ds.unload();
        delete wave;
        delete[] file;
        return false;
      }

      ds.wave.push_back(wave);
    }

    // read samples
    ds.sample.reserve(ds.sampleLen);
    for (int i=0; i<ds.sampleLen; i++) {
      DivSample* sample=new DivSample;

      if (!reader.seek(samplePtr[i],SEEK_SET)) {
        logE("couldn't seek to sample %d!",i);
        lastError=fmt::sprintf("couldn't seek to sample %d!",i);
        ds.unload();
        delete sample;
        delete[] file;
        return false;
      }

      if (sample->readSampleData(reader,ds.version)!=DIV_DATA_SUCCESS) {
        lastError="invalid sample header/data!";
        ds.unload();
        delete sample;
        delete[] file;
        return false;
      }

      ds.sample.push_back(sample);
    }

    // read patterns
    for (unsigned int i: patPtr) {
      bool isNewFormat=false;
      if (!reader.seek(i,SEEK_SET)) {
        logE("couldn't seek to pattern in %x!",i);
        lastError=fmt::sprintf("couldn't seek to pattern in %x!",i);
        ds.unload();
        delete[] file;
        return false;
      }
      reader.read(magic,4);
      logD("reading pattern in %x...",i);
      if (strcmp(magic,"PATR")!=0) {
        if (strcmp(magic,"PATN")!=0 || ds.version<157) {
          logE("%x: invalid pattern header!",i);
          lastError="invalid pattern header!";
          ds.unload();
          delete[] file;
          return false;
        } else {
          isNewFormat=true;
        }
      }
      reader.readI();

      if (isNewFormat) {
        int subs=(unsigned char)reader.readC();
        int chan=(unsigned char)reader.readC();
        int index=reader.readS();

        logD("- %d, %d, %d (new)",subs,chan,index);

        if (chan<0 || chan>=tchans) {
          logE("pattern channel out of range!",i);
          lastError="pattern channel out of range!";
          ds.unload();
          delete[] file;
          return false;
        }
        if (index<0 || index>(DIV_MAX_PATTERNS-1)) {
          logE("pattern index out of range!",i);
          lastError="pattern index out of range!";
          ds.unload();
          delete[] file;
          return false;
        }
        if (subs<0 || subs>=(int)ds.subsong.size()) {
          logE("pattern subsong out of range!",i);
          lastError="pattern subsong out of range!";
          ds.unload();
          delete[] file;
          return false;
        }

        DivPattern* pat=ds.subsong[subs]->pat[chan].getPattern(index,true);
        pat->name=reader.readString();

        // read new pattern
        for (int j=0; j<ds.subsong[subs]->patLen; j++) {
          unsigned char mask=reader.readC();
          unsigned short effectMask=0;

          if (mask==0xff) break;
          if (mask&128) {
            j+=(mask&127)+1;
            continue;
          }

          if (mask&32) {
            effectMask|=(unsigned char)reader.readC();
          }
          if (mask&64) {
            effectMask|=((unsigned short)reader.readC()&0xff)<<8;
          }
          if (mask&8) effectMask|=1;
          if (mask&16) effectMask|=2;

          if (mask&1) { // note
            unsigned char note=reader.readC();
            // TODO: PAT2 format with new off/===/rel values!
            if (note==180) {
              pat->newData[j][0]=DIV_NOTE_OFF;
            } else if (note==181) {
              pat->newData[j][0]=DIV_NOTE_REL;
            } else if (note==182) {
              pat->newData[j][0]=DIV_MACRO_REL;
            } else if (note<180) {
              pat->newData[j][DIV_PAT_NOTE]=note;
            } else {
              pat->newData[j][0]=-1;
            }
          }
          if (mask&2) { // instrument
            pat->newData[j][DIV_PAT_INS]=(unsigned char)reader.readC();
          }
          if (mask&4) { // volume
            pat->newData[j][DIV_PAT_VOL]=(unsigned char)reader.readC();
          }
          for (unsigned char k=0; k<16; k++) {
            if (effectMask&(1<<k)) {
              pat->newData[j][DIV_PAT_FX(0)+k]=(unsigned char)reader.readC();
            }
          }
        }
      } else {
        int chan=reader.readS();
        int index=reader.readS();
        int subs=0;
        if (ds.version>=95) {
          subs=reader.readS();
        } else {
          reader.readS();
        }
        reader.readS();

        logD("- %d, %d, %d (old)",subs,chan,index);

        if (chan<0 || chan>=tchans) {
          logE("pattern channel out of range!",i);
          lastError="pattern channel out of range!";
          ds.unload();
          delete[] file;
          return false;
        }
        if (index<0 || index>(DIV_MAX_PATTERNS-1)) {
          logE("pattern index out of range!",i);
          lastError="pattern index out of range!";
          ds.unload();
          delete[] file;
          return false;
        }
        if (subs<0 || subs>=(int)ds.subsong.size()) {
          logE("pattern subsong out of range!",i);
          lastError="pattern subsong out of range!";
          ds.unload();
          delete[] file;
          return false;
        }

        DivPattern* pat=ds.subsong[subs]->pat[chan].getPattern(index,true);
        for (int j=0; j<ds.subsong[subs]->patLen; j++) {
          short note=reader.readS();
          short octave=reader.readS();
          if (note==0 && octave!=0) {
            logD("what? %d:%d:%d note %d octave %d",chan,i,j,note,octave);
            note=12;
            octave--;
          }
          pat->newData[j][DIV_PAT_NOTE]=splitNoteToNote(note,octave);

          pat->newData[j][DIV_PAT_INS]=reader.readS();
          pat->newData[j][DIV_PAT_VOL]=reader.readS();
          for (int k=0; k<ds.subsong[subs]->pat[chan].effectCols; k++) {
            pat->newData[j][DIV_PAT_FX(k)]=reader.readS();
            pat->newData[j][DIV_PAT_FXVAL(k)]=reader.readS();
          }
        }

        if (ds.version>=51) {
          pat->name=reader.readString();
        }
      }
    }

    if (reader.tell()<reader.size()) {
      if ((reader.tell()+1)!=reader.size()) {
        logW("premature end of song (we are at %x, but size is %x)",reader.tell(),reader.size());
      }
    }

    // convert OPM/NES instrument types
    if (ds.version<117) {
      int opnCount=0;
      int opmCount=0;
      int snCount=0;
      int nesCount=0;
      for (int i=0; i<ds.systemLen; i++) {
        switch (ds.system[i]) {
          case DIV_SYSTEM_NES:
          case DIV_SYSTEM_MMC5:
            nesCount++;
            break;
          case DIV_SYSTEM_SMS:
            snCount++;
            break;
          case DIV_SYSTEM_YM2151:
          case DIV_SYSTEM_OPZ:
            opmCount++;
            break;
          case DIV_SYSTEM_YM2610_FULL:
          case DIV_SYSTEM_YM2610_FULL_EXT:
          case DIV_SYSTEM_YM2610B:
          case DIV_SYSTEM_YM2610B_EXT:
          case DIV_SYSTEM_YM2203:
          case DIV_SYSTEM_YM2203_EXT:
          case DIV_SYSTEM_YM2608:
          case DIV_SYSTEM_YM2608_EXT:
          case DIV_SYSTEM_YM2612:
          case DIV_SYSTEM_YM2612_EXT:
          case DIV_SYSTEM_YM2612_DUALPCM:
          case DIV_SYSTEM_YM2612_DUALPCM_EXT:
            opnCount++;
            break;
          default:
            break;
        }
      }
      if (opmCount>opnCount) {
        for (DivInstrument* i: ds.ins) {
          if (i->type==DIV_INS_FM) i->type=DIV_INS_OPM;
        }
      }
      if (nesCount>snCount) {
        for (DivInstrument* i: ds.ins) {
          if (i->type==DIV_INS_STD) i->type=DIV_INS_NES;
        }
      }
    }

    // ExtCh compat flag
    for (int i=0; i<ds.systemLen; i++) {
      if (ds.system[i]==DIV_SYSTEM_YM2612_EXT ||
          ds.system[i]==DIV_SYSTEM_YM2612_DUALPCM_EXT ||
          ds.system[i]==DIV_SYSTEM_YM2610_FULL_EXT ||
          ds.system[i]==DIV_SYSTEM_YM2610B_EXT ||
          ds.system[i]==DIV_SYSTEM_YM2203_EXT ||
          ds.system[i]==DIV_SYSTEM_YM2608_EXT ||
          ds.system[i]==DIV_SYSTEM_YM2612_CSM ||
          ds.system[i]==DIV_SYSTEM_YM2203_CSM ||
          ds.system[i]==DIV_SYSTEM_YM2608_CSM ||
          ds.system[i]==DIV_SYSTEM_YM2610_CSM ||
          ds.system[i]==DIV_SYSTEM_YM2610B_CSM) {
        if (ds.version<125) {
          ds.systemFlags[i].set("noExtMacros",true);
        }
        if (ds.version<133) {
          ds.systemFlags[i].set("fbAllOps",true);
        }
      }
    }

    // SN noise compat
    if (ds.version<128) {
      for (int i=0; i<ds.systemLen; i++) {
        if (ds.system[i]==DIV_SYSTEM_SMS ||
            ds.system[i]==DIV_SYSTEM_T6W28) {
          ds.systemFlags[i].set("noEasyNoise",true);
        }
      }
    }

    // OPL3 pan compat
    if (ds.version<134) {
      for (int i=0; i<ds.systemLen; i++) {
        if (ds.system[i]==DIV_SYSTEM_OPL3 ||
            ds.system[i]==DIV_SYSTEM_OPL3_DRUMS) {
          ds.systemFlags[i].set("compatPan",true);
        }
      }
    }

    // new YM2612/SN/X1-010 volumes
    if (ds.version<137) {
      for (int i=0; i<ds.systemLen; i++) {
        switch (ds.system[i]) {
          case DIV_SYSTEM_YM2612:
          case DIV_SYSTEM_YM2612_EXT:
          case DIV_SYSTEM_YM2612_DUALPCM:
          case DIV_SYSTEM_YM2612_DUALPCM_EXT:
          case DIV_SYSTEM_YM2612_CSM:
            ds.systemVol[i]/=2.0;
            break;
          case DIV_SYSTEM_SMS:
          case DIV_SYSTEM_T6W28:
          case DIV_SYSTEM_OPLL:
          case DIV_SYSTEM_OPLL_DRUMS:
            ds.systemVol[i]/=1.5;
            break;
          case DIV_SYSTEM_X1_010:
            ds.systemVol[i]/=4.0;
            break;
          default:
            break;
        }
      }
    }

    // Namco C30 noise compat
    if (ds.version<145) {
      for (int i=0; i<ds.systemLen; i++) {
        if (ds.system[i]==DIV_SYSTEM_NAMCO_CUS30) {
          ds.systemFlags[i].set("newNoise",false);
        }
      }
    }

    // SegaPCM slide compat
    if (ds.version<153) {
      for (int i=0; i<ds.systemLen; i++) {
        if (ds.system[i]==DIV_SYSTEM_SEGAPCM) {
          ds.systemFlags[i].set("oldSlides",true);
        }
      }
    }

    // NES PCM compat
    if (ds.version<154) {
      for (int i=0; i<ds.systemLen; i++) {
        if (ds.system[i]==DIV_SYSTEM_NES) {
          ds.systemFlags[i].set("dpcmMode",false);
        }
      }
    }

    // C64 key priority compat
    if (ds.version<160) {
      for (int i=0; i<ds.systemLen; i++) {
        if (ds.system[i]==DIV_SYSTEM_C64_8580 || ds.system[i]==DIV_SYSTEM_C64_6581) {
          ds.systemFlags[i].set("keyPriority",false);
        }
      }
    }

    // Namco 163 pitch compensation compat
    if (ds.version<165) {
      for (int i=0; i<ds.systemLen; i++) {
        if (ds.system[i]==DIV_SYSTEM_N163) {
          ds.systemFlags[i].set("lenCompensate",true);
        }
      }
    }

    // OPM/OPZ slide compat
    if (ds.version<176) {
      for (int i=0; i<ds.systemLen; i++) {
        if (ds.system[i]==DIV_SYSTEM_YM2151 ||
            ds.system[i]==DIV_SYSTEM_OPZ) {
          ds.systemFlags[i].set("brokenPitch",true);
        }
      }
    }

    // C64 1Exy compat
    if (ds.version<186) {
      for (int i=0; i<ds.systemLen; i++) {
        if (ds.system[i]==DIV_SYSTEM_C64_8580 || ds.system[i]==DIV_SYSTEM_C64_6581) {
          ds.systemFlags[i].set("no1EUpdate",true);
        }
      }
    }

    // C64 original reset time and multiply relative
    if (ds.version<187) {
      for (int i=0; i<ds.systemLen; i++) {
        if (ds.system[i]==DIV_SYSTEM_C64_8580 || ds.system[i]==DIV_SYSTEM_C64_6581) {
          ds.systemFlags[i].set("initResetTime",1);
          ds.systemFlags[i].set("multiplyRel",true);
        }
      }
    }

    // OPLL fixedAll compat
    if (ds.version<194) {
      for (int i=0; i<ds.systemLen; i++) {
        if (ds.system[i]==DIV_SYSTEM_OPLL ||
            ds.system[i]==DIV_SYSTEM_OPLL_DRUMS) {
          if (!ds.systemFlags[i].has("fixedAll")) {
            ds.systemFlags[i].set("fixedAll",false);
          }
        }
      }
    }

    // C64 macro race
    if (ds.version<195) {
      for (int i=0; i<ds.systemLen; i++) {
        if (ds.system[i]==DIV_SYSTEM_C64_8580 || ds.system[i]==DIV_SYSTEM_C64_6581) {
          ds.systemFlags[i].set("macroRace",true);
        }
      }
    }

    // VERA old chip revision
    // TIA old tuning
    if (ds.version<213) {
      for (int i=0; i<ds.systemLen; i++) {
        if (ds.system[i]==DIV_SYSTEM_VERA) {
          ds.systemFlags[i].set("chipType",0);
        }
        if (ds.system[i]==DIV_SYSTEM_TIA) {
          ds.systemFlags[i].set("oldPitch",true);
        }
      }
    } else if (ds.version<217) {
      for (int i=0; i<ds.systemLen; i++) {
        if (ds.system[i]==DIV_SYSTEM_VERA) {
          ds.systemFlags[i].set("chipType",1);
        }
      }
    } else if (ds.version<229) {
      for (int i=0; i<ds.systemLen; i++) {
        if (ds.system[i]==DIV_SYSTEM_VERA) {
          ds.systemFlags[i].set("chipType",2);
        }
      }
    }

    // SNES no anti-click
    if (ds.version<220) {
      for (int i=0; i<ds.systemLen; i++) {
        if (ds.system[i]==DIV_SYSTEM_SNES) {
          ds.systemFlags[i].set("antiClick",false);
        }
      }
    }

    // Y8950 broken ADPCM pitch
    if (ds.version<223) {
      for (int i=0; i<ds.systemLen; i++) {
        if (ds.system[i]==DIV_SYSTEM_Y8950 || ds.system[i]==DIV_SYSTEM_Y8950_DRUMS) {
          ds.systemFlags[i].set("compatYPitch",true);
        }
      }
    }

    // YM2612 chip type
    if (ds.version<231) {
      for (int i=0; i<ds.systemLen; i++) {
        if (ds.system[i]==DIV_SYSTEM_YM2612 ||
            ds.system[i]==DIV_SYSTEM_YM2612_EXT ||
            ds.system[i]==DIV_SYSTEM_YM2612_EXT ||
            ds.system[i]==DIV_SYSTEM_YM2612_DUALPCM ||
            ds.system[i]==DIV_SYSTEM_YM2612_DUALPCM_EXT ||
            ds.system[i]==DIV_SYSTEM_YM2612_CSM) {
          if (!ds.systemFlags[i].has("chipType") && !ds.systemFlags[i].has("ladderEffect")) {
            ds.systemFlags[i].set("chipType",0);
          }
        }
      }
    }

    // YM2151 E5xx range was different
    if (ds.version<236) {
      int ch=0;
      // so much nesting
      for (int i=0; i<ds.systemLen; i++) {
        if (ds.system[i]==DIV_SYSTEM_YM2151) {
          // find all E5xx effects and adapt to normal range
          for (int j=ch; j<ch+8; j++) {
            for (size_t k=0; k<ds.subsong.size(); k++) {
              for (int l=0; l<DIV_MAX_PATTERNS; l++) {
                DivPattern* p=ds.subsong[k]->pat[j].data[l];
                if (p==NULL) continue;

                for (int m=0; m<DIV_MAX_ROWS; m++) {
                  for (int n=0; n<ds.subsong[k]->pat[j].effectCols; n++) {
                    if (p->newData[m][DIV_PAT_FX(n)]==0xe5 && p->newData[m][DIV_PAT_FXVAL(n)]!=-1) {
                      int newVal=(2*((p->newData[m][DIV_PAT_FXVAL(n)]&0xff)-0x80))+0x80;
                      if (newVal<0) newVal=0;
                      if (newVal>0xff) newVal=0xff;
                      p->newData[m][DIV_PAT_FXVAL(n)]=newVal;
                    }
                  }
                }
              }
            }
          }
        }
        ch+=ds.systemChans[i];
      }
    }

    // warn on partial pitch linearity
    if (ds.compatFlags.linearPitch>1) {
      ds.compatFlags.linearPitch=1;
    } else if (ds.version<237 && ds.compatFlags.linearPitch!=0) {
      addWarning("this song used partial pitch linearity, which has been removed from Furnace. you may have to adjust your song.");
    }
    ds.recalcChans();

    if (active) quitDispatch();
    BUSY_BEGIN_SOFT;
    saveLock.lock();
    song.unload();
    song=ds;
    hasLoadedSomething=true;
    changeSong(0);
    // removal of legacy sample mode
    if (song.version<239) {
      if (convertLegacySampleMode()) {
        addWarning("Furnace no longer supports legacy sample mode. your song has been converted.");
      }
    }
    saveLock.unlock();
    BUSY_END;
    if (active) {
      initDispatch();
      BUSY_BEGIN;
      renderSamples();
      reset();
      BUSY_END;
    }
  } catch (EndOfFileException& e) {
    logE("premature end of file!");
    lastError="incomplete file";
    delete[] file;
    return false;
  }
  delete[] file;
  return true;
}

SafeWriter* DivEngine::saveFur(bool notPrimary) {
  saveLock.lock();
  std::vector<int> subSongPtr;
  std::vector<int> sysFlagsPtr;
  std::vector<int> insPtr;
  std::vector<int> wavePtr;
  std::vector<int> samplePtr;
  std::vector<int> patPtr;
  std::vector<int> groovePtr;
  int assetDirPtr[3];
  int compatFlagPtr=0;
  int commentPtr=0;
  size_t blockStartSeek, blockEndSeek;
  size_t sng2PtrSeek=0, flagPtrSeek=0, adirPtrSeek=0, ins2PtrSeek=0, wavePtrSeek=0, smp2PtrSeek=0, patnPtrSeek=0, cflgPtrSeek=0;
  size_t cmntPtrSeek=0, grovPtrSeek=0;

  warnings="";

  // fail if values are out of range
  /*
  if (subSong->ordersLen>DIV_MAX_PATTERNS) {
    logE("maximum song length is %d!",DIV_MAX_PATTERNS);
    lastError=fmt::sprintf("maximum song length is %d",DIV_MAX_PATTERNS);
    return NULL;
  }
  if (subSong->patLen>DIV_MAX_ROWS) {
    logE("maximum pattern length is %d!",DIV_MAX_ROWS);
    lastError=fmt::sprintf("maximum pattern length is %d",DIV_MAX_ROWS);
    return NULL;
  }
  */
  if (song.ins.size()>256) {
    logE("maximum number of instruments is 256!");
    lastError="maximum number of instruments is 256";
    saveLock.unlock();
    return NULL;
  }
  if (song.wave.size()>32768) {
    logE("maximum number of wavetables is 32768!");
    lastError="maximum number of wavetables is 32768";
    saveLock.unlock();
    return NULL;
  }
  if (song.sample.size()>32768) {
    logE("maximum number of samples is 32768!");
    lastError="maximum number of samples is 32768";
    saveLock.unlock();
    return NULL;
  }

  if (!notPrimary) {
    song.isDMF=false;
    song.version=DIV_ENGINE_VERSION;
  }

  SafeWriter* w=new SafeWriter;
  w->init();
  /// HEADER
  // write magic
  w->write(DIV_FUR_MAGIC,16);

  // write version
  w->writeS(DIV_ENGINE_VERSION);

  // reserved
  w->writeS(0);

  // song info pointer
  w->writeI(32);

  // reserved
  w->writeI(0);
  w->writeI(0);

  // high short is channel
  // low short is pattern number
  std::vector<PatToWrite> patsToWrite;
  if (getConfInt("saveUnusedPatterns",0)==1) {
    for (int i=0; i<song.chans; i++) {
      for (size_t j=0; j<song.subsong.size(); j++) {
        DivSubSong* subs=song.subsong[j];
        for (int k=0; k<DIV_MAX_PATTERNS; k++) {
          if (subs->pat[i].data[k]==NULL) continue;
          patsToWrite.push_back(PatToWrite(j,i,k));
        }
      }
    }
  } else {
    bool alreadyAdded[DIV_MAX_PATTERNS];
    for (int i=0; i<song.chans; i++) {
      for (size_t j=0; j<song.subsong.size(); j++) {
        DivSubSong* subs=song.subsong[j];
        memset(alreadyAdded,0,DIV_MAX_PATTERNS*sizeof(bool));
        for (int k=0; k<subs->ordersLen; k++) {
          if (alreadyAdded[subs->orders.ord[i][k]]) continue;
          patsToWrite.push_back(PatToWrite(j,i,subs->orders.ord[i][k]));
          alreadyAdded[subs->orders.ord[i][k]]=true;
        }
      }
    }
  }

  /// SONG INFO
  w->write("INF2",4);
  blockStartSeek=w->tell();
  w->writeI(0);

  // song information
  w->writeString(song.name,false);
  w->writeString(song.author,false);
  w->writeString(song.systemName,false);
  w->writeString(song.category,false);
  w->writeString(song.nameJ,false);
  w->writeString(song.authorJ,false);
  w->writeString(song.systemNameJ,false);
  w->writeString(song.categoryJ,false);
  w->writeF(song.tuning);
  w->writeC(song.autoSystem);

  // system definition
  w->writeF(song.masterVol);
  w->writeS(song.chans);
  w->writeS(song.systemLen);

  for (int i=0; i<song.systemLen; i++) {
    w->writeS(systemToFileFur(song.system[i]));
    w->writeS(song.systemChans[i]);
    w->writeF(song.systemVol[i]);
    w->writeF(song.systemPan[i]);
    w->writeF(song.systemPanFR[i]);
  }

  // patchbay
  w->writeI(song.patchbay.size());
  for (unsigned int i: song.patchbay) {
    w->writeI(i);
  }
  w->writeC(song.patchbayAuto);

  /// song elements
  // sub-songs
  if (!song.subsong.empty()) {
    w->writeC(0x01);
    w->writeI(song.subsong.size());
    sng2PtrSeek=w->tell();
    for (size_t i=0; i<song.subsong.size(); i++) {
      w->writeI(0);
    }
  }
  // chip flags
  if (true) {
    w->writeC(0x02);
    w->writeI(song.systemLen);
    flagPtrSeek=w->tell();
    for (int i=0; i<song.systemLen; i++) {
      w->writeI(0);
    }
  }
  // asset directories
  if (true) {
    w->writeC(0x03);
    w->writeI(3);
    adirPtrSeek=w->tell();
    w->writeI(0);
    w->writeI(0);
    w->writeI(0);
  }
  // instruments
  if (!song.ins.empty()) {
    w->writeC(0x04);
    w->writeI(song.ins.size());
    ins2PtrSeek=w->tell();
    for (size_t i=0; i<song.ins.size(); i++) {
      w->writeI(0);
    }
  }
  // wavetables
  if (!song.wave.empty()) {
    w->writeC(0x05);
    w->writeI(song.wave.size());
    wavePtrSeek=w->tell();
    for (size_t i=0; i<song.wave.size(); i++) {
      w->writeI(0);
    }
  }
  // samples
  if (!song.sample.empty()) {
    w->writeC(0x06);
    w->writeI(song.sample.size());
    smp2PtrSeek=w->tell();
    for (size_t i=0; i<song.sample.size(); i++) {
      w->writeI(0);
    }
  }
  // patterns
  if (!patsToWrite.empty()) {
    w->writeC(0x07);
    w->writeI(patsToWrite.size());
    patnPtrSeek=w->tell();
    for (size_t i=0; i<patsToWrite.size(); i++) {
      w->writeI(0);
    }
  }
  // compat flags
  if (!song.compatFlags.areDefaults()) {
    w->writeC(0x08);
    w->writeI(1);
    cflgPtrSeek=w->tell();
    w->writeI(0);
  }
  // song comments
  if (!song.notes.empty()) {
    w->writeC(0x09);
    w->writeI(1);
    cmntPtrSeek=w->tell();
    w->writeI(0);
  }
  // groove patterns
  if (!song.grooves.empty()) {
    w->writeC(0x0a);
    w->writeI(song.grooves.size());
    grovPtrSeek=w->tell();
    for (size_t i=0; i<song.grooves.size(); i++) {
      w->writeI(0);
    }
  }
  
  w->writeC(0);

  blockEndSeek=w->tell();
  w->seek(blockStartSeek,SEEK_SET);
  w->writeI(blockEndSeek-blockStartSeek-4);
  w->seek(0,SEEK_END);

  /// SUBSONGS
  subSongPtr.reserve(song.subsong.size());
  for (size_t i=0; i<song.subsong.size(); i++) {
    subSongPtr.push_back(w->tell());
    song.subsong[i]->putData(w,song.chans);
  }

  /// CHIP FLAGS
  sysFlagsPtr.reserve(song.systemLen);
  for (int i=0; i<song.systemLen; i++) {
    String data=song.systemFlags[i].toString();
    if (data.empty()) {
      sysFlagsPtr.push_back(0);
      continue;
    }

    sysFlagsPtr.push_back(w->tell());
    w->write("FLAG",4);
    blockStartSeek=w->tell();
    w->writeI(0);

    w->writeString(data,false);

    blockEndSeek=w->tell();
    w->seek(blockStartSeek,SEEK_SET);
    w->writeI(blockEndSeek-blockStartSeek-4);
    w->seek(0,SEEK_END);
  }

  /// COMPAT FLAGS
  if (!song.compatFlags.areDefaults()) {
    compatFlagPtr=w->tell();
    song.compatFlags.putData(w);
  }

  /// SONG COMMENTS
  if (!song.notes.empty()) {
    commentPtr=w->tell();
    w->write("CMNT",4);
    blockStartSeek=w->tell();
    w->writeI(0);

    w->writeString(song.notes,false);

    blockEndSeek=w->tell();
    w->seek(blockStartSeek,SEEK_SET);
    w->writeI(blockEndSeek-blockStartSeek-4);
    w->seek(0,SEEK_END);
  }

  /// ASSET DIRECTORIES
  assetDirPtr[0]=w->tell();
  putAssetDirData(w,song.insDir);
  assetDirPtr[1]=w->tell();
  putAssetDirData(w,song.waveDir);
  assetDirPtr[2]=w->tell();
  putAssetDirData(w,song.sampleDir);

  /// INSTRUMENT
  insPtr.reserve(song.insLen);
  for (int i=0; i<song.insLen; i++) {
    DivInstrument* ins=song.ins[i];
    insPtr.push_back(w->tell());
    ins->putInsData2(w,false);
  }

  /// WAVETABLE
  wavePtr.reserve(song.waveLen);
  for (int i=0; i<song.waveLen; i++) {
    DivWavetable* wave=song.wave[i];
    wavePtr.push_back(w->tell());
    wave->putWaveData(w);
  }

  /// SAMPLE
  samplePtr.reserve(song.sampleLen);
  for (int i=0; i<song.sampleLen; i++) {
    DivSample* sample=song.sample[i];
    samplePtr.push_back(w->tell());
    sample->putSampleData(w);
  }

  /// PATTERN
  patPtr.reserve(patsToWrite.size());
  for (PatToWrite& i: patsToWrite) {
    DivPattern* pat=song.subsong[i.subsong]->pat[i.chan].getPattern(i.pat,false);
    patPtr.push_back(w->tell());

    w->write("PATN",4);
    blockStartSeek=w->tell();
    w->writeI(0);

    w->writeC(i.subsong);
    w->writeC(i.chan);
    w->writeS(i.pat);
    w->writeString(pat->name,false);

    unsigned char emptyRows=0;

    for (int j=0; j<song.subsong[i.subsong]->patLen; j++) {
      unsigned char mask=0;
      unsigned char finalNote=255;
      unsigned short effectMask=0;

      if (pat->newData[j][DIV_PAT_NOTE]==DIV_NOTE_OFF) { // note off
        finalNote=180;
      } else if (pat->newData[j][DIV_PAT_NOTE]==DIV_NOTE_REL) { // note release
        finalNote=181;
      } else if (pat->newData[j][DIV_PAT_NOTE]==DIV_MACRO_REL) { // macro release
        finalNote=182;
      } else if (pat->newData[j][DIV_PAT_NOTE]==-1) { // empty
        finalNote=255;
      } else {
        finalNote=pat->newData[j][DIV_PAT_NOTE];
      }

      if (finalNote!=255) mask|=1; // note
      if (pat->newData[j][DIV_PAT_INS]!=-1) mask|=2; // instrument
      if (pat->newData[j][DIV_PAT_VOL]!=-1) mask|=4; // volume
      for (int k=0; k<song.subsong[i.subsong]->pat[i.chan].effectCols*2; k+=2) {
        if (k==0) {
          if (pat->newData[j][DIV_PAT_FX(0)+k]!=-1) mask|=8;
          if (pat->newData[j][DIV_PAT_FXVAL(0)+k]!=-1) mask|=16;
        } else if (k<8) {
          if (pat->newData[j][DIV_PAT_FX(0)+k]!=-1 || pat->newData[j][DIV_PAT_FXVAL(0)+k]!=-1) mask|=32;
        } else {
          if (pat->newData[j][DIV_PAT_FX(0)+k]!=-1 || pat->newData[j][DIV_PAT_FXVAL(0)+k]!=-1) mask|=64;
        }

        if (pat->newData[j][DIV_PAT_FX(0)+k]!=-1) effectMask|=(1<<k);
        if (pat->newData[j][DIV_PAT_FXVAL(0)+k]!=-1) effectMask|=(2<<k);
      }

      if (mask==0) {
        emptyRows++;
        if (emptyRows>127) {
          w->writeC(128|(emptyRows-2));
          emptyRows=0;
        }
      } else {
        if (emptyRows>1) {
          w->writeC(128|(emptyRows-2));
          emptyRows=0;
        } else if (emptyRows) {
          w->writeC(0);
          emptyRows=0;
        }

        w->writeC(mask);

        if (mask&32) w->writeC(effectMask&0xff);
        if (mask&64) w->writeC((effectMask>>8)&0xff);

        if (mask&1) w->writeC(finalNote);
        if (mask&2) w->writeC(pat->newData[j][DIV_PAT_INS]);
        if (mask&4) w->writeC(pat->newData[j][DIV_PAT_VOL]);
        if (mask&8) w->writeC(pat->newData[j][DIV_PAT_FX(0)]);
        if (mask&16) w->writeC(pat->newData[j][DIV_PAT_FXVAL(0)]);
        if (mask&32) {
          if (effectMask&4) w->writeC(pat->newData[j][DIV_PAT_FX(1)]);
          if (effectMask&8) w->writeC(pat->newData[j][DIV_PAT_FXVAL(1)]);
          if (effectMask&16) w->writeC(pat->newData[j][DIV_PAT_FX(2)]);
          if (effectMask&32) w->writeC(pat->newData[j][DIV_PAT_FXVAL(2)]);
          if (effectMask&64) w->writeC(pat->newData[j][DIV_PAT_FX(3)]);
          if (effectMask&128) w->writeC(pat->newData[j][DIV_PAT_FXVAL(3)]);
        }
        if (mask&64) {
          if (effectMask&256) w->writeC(pat->newData[j][DIV_PAT_FX(4)]);
          if (effectMask&512) w->writeC(pat->newData[j][DIV_PAT_FXVAL(4)]);
          if (effectMask&1024) w->writeC(pat->newData[j][DIV_PAT_FX(5)]);
          if (effectMask&2048) w->writeC(pat->newData[j][DIV_PAT_FXVAL(5)]);
          if (effectMask&4096) w->writeC(pat->newData[j][DIV_PAT_FX(6)]);
          if (effectMask&8192) w->writeC(pat->newData[j][DIV_PAT_FXVAL(6)]);
          if (effectMask&16384) w->writeC(pat->newData[j][DIV_PAT_FX(7)]);
          if (effectMask&32768) w->writeC(pat->newData[j][DIV_PAT_FXVAL(7)]);
        }
      }
    }

    // stop
    w->writeC(0xff);

    blockEndSeek=w->tell();
    w->seek(blockStartSeek,SEEK_SET);
    w->writeI(blockEndSeek-blockStartSeek-4);
    w->seek(0,SEEK_END);
  }

  /// POINTERS
  // sub-songs
  if (sng2PtrSeek) {
    w->seek(sng2PtrSeek,SEEK_SET);
    for (size_t i=0; i<song.subsong.size(); i++) {
      w->writeI(subSongPtr[i]);
    }
  }
  // chip flags
  if (flagPtrSeek) {
    w->seek(flagPtrSeek,SEEK_SET);
    for (int i=0; i<song.systemLen; i++) {
      w->writeI(sysFlagsPtr[i]);
    }
  }
  // asset directories
  if (adirPtrSeek) {
    w->seek(adirPtrSeek,SEEK_SET);
    w->writeI(assetDirPtr[0]);
    w->writeI(assetDirPtr[1]);
    w->writeI(assetDirPtr[2]);
  }
  // instruments
  if (ins2PtrSeek) {
    w->seek(ins2PtrSeek,SEEK_SET);
    for (int i: insPtr) {
      w->writeI(i);
    }
  }
  // wavetables
  if (wavePtrSeek) {
    w->seek(wavePtrSeek,SEEK_SET);
    for (int i: wavePtr) {
      w->writeI(i);
    }
  }
  // samples
  if (smp2PtrSeek) {
    w->seek(smp2PtrSeek,SEEK_SET);
    for (int i: samplePtr) {
      w->writeI(i);
    }
  }
  // patterns
  if (patnPtrSeek) {
    w->seek(patnPtrSeek,SEEK_SET);
    for (int i: patPtr) {
      w->writeI(i);
    }
  }
  // compat flags
  if (cflgPtrSeek) {
    w->seek(cflgPtrSeek,SEEK_SET);
    w->writeI(compatFlagPtr);
  }
  // song comments
  if (cmntPtrSeek) {
    w->seek(cmntPtrSeek,SEEK_SET);
    w->writeI(commentPtr);
  }
  // groove patterns
  if (grovPtrSeek) {
    w->seek(grovPtrSeek,SEEK_SET);
    for (int i: groovePtr) {
      w->writeI(i);
    }
  }

  saveLock.unlock();
  return w;
}

