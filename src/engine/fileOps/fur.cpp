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

void DivEngine::putAssetDirData(SafeWriter* w, std::vector<DivAssetDir>& dir) {
  size_t blockStartSeek, blockEndSeek;

  w->write("ADIR",4);
  blockStartSeek=w->tell();
  w->writeI(0);

  w->writeI(dir.size());

  for (DivAssetDir& i: dir) {
    w->writeString(i.name,false);
    w->writeS(i.entries.size());
    for (int j: i.entries) {
      w->writeC(j);
    }
  }

  blockEndSeek=w->tell();
  w->seek(blockStartSeek,SEEK_SET);
  w->writeI(blockEndSeek-blockStartSeek-4);
  w->seek(0,SEEK_END);
}

DivDataErrors DivEngine::readAssetDirData(SafeReader& reader, std::vector<DivAssetDir>& dir) {
  char magic[4];
  reader.read(magic,4);
  if (memcmp(magic,"ADIR",4)!=0) {
    logV("header is invalid: %c%c%c%c",magic[0],magic[1],magic[2],magic[3]);
    return DIV_DATA_INVALID_HEADER;
  }
  reader.readI(); // reserved

  unsigned int numDirs=reader.readI();

  dir.reserve(numDirs);
  for (unsigned int i=0; i<numDirs; i++) {
    DivAssetDir d;

    d.name=reader.readString();
    unsigned short numEntries=reader.readS();

    d.entries.reserve(numEntries);
    for (unsigned short j=0; j<numEntries; j++) {
      d.entries.push_back(((unsigned char)reader.readC()));
    }

    dir.push_back(d);
  }

  return DIV_DATA_SUCCESS;
}

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
    case DIV_SYSTEM_YM2610:
    case DIV_SYSTEM_YM2610_EXT:
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

bool DivEngine::loadFur(unsigned char* file, size_t len, int variantID) {
  unsigned int insPtr[256];
  unsigned int wavePtr[256];
  unsigned int samplePtr[256];
  unsigned int subSongPtr[256];
  unsigned int sysFlagsPtr[DIV_MAX_CHIPS];
  unsigned int assetDirPtr[3];
  std::vector<unsigned int> patPtr;
  int numberOfSubSongs=0;
  char magic[5];
  memset(magic,0,5);
  SafeReader reader=SafeReader(file,len);
  warnings="";
  assetDirPtr[0]=0;
  assetDirPtr[1]=0;
  assetDirPtr[2]=0;
  try {
    DivSong ds;
    DivSubSong* subSong=ds.subsong[0];

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

    if (ds.version<37) { // compat flags not stored back then
      ds.limitSlides=true;
      ds.linearPitch=1;
      ds.loopModality=0;
    }
    if (ds.version<43) {
      ds.properNoiseLayout=false;
      ds.waveDutyIsVol=false;
    }
    if (ds.version<45) {
      ds.resetMacroOnPorta=true;
      ds.legacyVolumeSlides=true;
      ds.compatibleArpeggio=true;
      ds.noteOffResetsSlides=true;
      ds.targetResetsSlides=true;
    }
    if (ds.version<46) {
      ds.arpNonPorta=true;
      ds.algMacroBehavior=true;
    } else {
      ds.arpNonPorta=false;
      ds.algMacroBehavior=false;
    }
    if (ds.version<49) {
      ds.brokenShortcutSlides=true;
    }
    if (ds.version<50) {
      ds.ignoreDuplicateSlides=false;
    }
    if (ds.version<62) {
      ds.stopPortaOnNoteOff=true;
    }
    if (ds.version<64) {
      ds.brokenDACMode=false;
    }
    if (ds.version<65) {
      ds.oneTickCut=false;
    }
    if (ds.version<66) {
      ds.newInsTriggersInPorta=false;
    }
    if (ds.version<69) {
      ds.arp0Reset=false;
    }
    if (ds.version<71) {
      ds.noSlidesOnFirstTick=false;
      ds.rowResetsArpPos=false;
      ds.ignoreJumpAtEnd=true;
    }
    if (ds.version<72) {
      ds.buggyPortaAfterSlide=true;
      ds.gbInsAffectsEnvelope=false;
    }
    if (ds.version<78) {
      ds.sharedExtStat=false;
    }
    if (ds.version<83) {
      ds.ignoreDACModeOutsideIntendedChannel=true;
      ds.e1e2AlsoTakePriority=false;
    }
    if (ds.version<84) {
      ds.newSegaPCM=false;
    }
    if (ds.version<85) {
      ds.fbPortaPause=true;
    }
    if (ds.version<86) {
      ds.snDutyReset=true;
    }
    if (ds.version<90) {
      ds.pitchMacroIsLinear=false;
    }
    if (ds.version<97) {
      ds.oldOctaveBoundary=true;
    }
    if (ds.version<97) { // actually should be 98 but yky uses this feature ahead of time
      ds.noOPN2Vol=true;
    }
    if (ds.version<99) {
      ds.newVolumeScaling=false;
      ds.volMacroLinger=false;
      ds.brokenOutVol=true;
    }
    if (ds.version<100) {
      ds.e1e2StopOnSameNote=false;
    }
    if (ds.version<101) {
      ds.brokenPortaArp=true;
    }
    if (ds.version<108) {
      ds.snNoLowPeriods=true;
    }
    if (ds.version<110) {
      ds.delayBehavior=1;
    }
    if (ds.version<113) {
      ds.jumpTreatment=1;
    }
    if (ds.version<115) {
      ds.autoSystem=false;
    }
    if (ds.version<117) {
      ds.disableSampleMacro=true;
    }
    if (ds.version<121) {
      ds.brokenOutVol2=false;
    }
    if (ds.version<130) {
      ds.oldArpStrategy=true;
    }
    if (ds.version<138) {
      ds.brokenPortaLegato=true;
    }
    if (ds.version<155) {
      ds.brokenFMOff=true;
    }
    if (ds.version<168) {
      ds.preNoteNoEffect=true;
    }
    if (ds.version<183) {
      ds.oldDPCM=true;
    }
    if (ds.version<184) {
      ds.resetArpPhaseOnNewNote=false;
    }
    if (ds.version<188) {
      ds.ceilVolumeScaling=false;
    }
    if (ds.version<191) {
      ds.oldAlwaysSetVolume=true;
    }
    if (ds.version<200) {
      ds.oldSampleOffset=true;
    }
    ds.isDMF=false;

    reader.readS(); // reserved
    int infoSeek=reader.readI();

    if (!reader.seek(infoSeek,SEEK_SET)) {
      logE("couldn't seek to info header at %d!",infoSeek);
      lastError="couldn't seek to info header!";
      delete[] file;
      return false;
    }

    // read header
    reader.read(magic,4);
    if (strcmp(magic,"INFO")!=0) {
      logE("invalid info header!");
      lastError="invalid info header!";
      delete[] file;
      return false;
    }
    reader.readI();

    subSong->timeBase=reader.readC();
    subSong->speeds.len=2;
    subSong->speeds.val[0]=reader.readC();
    subSong->speeds.val[1]=reader.readC();
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
      lastError="pattern lengrh is negative!";
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
    if (ds.waveLen<0 || ds.waveLen>256) {
      logE("invalid wavetable count!");
      lastError="invalid wavetable count!";
      delete[] file;
      return false;
    }
    if (ds.sampleLen<0 || ds.sampleLen>256) {
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
      logD("- %d: %.2x (%s)",i,sysID,getSystemName(ds.system[i]));
      if (sysID!=0 && systemToFileFur(ds.system[i])==0) {
        logE("unrecognized system ID %.2x",sysID);
        lastError=fmt::sprintf("unrecognized system ID %.2x!",sysID);
        delete[] file;
        return false;
      }
      if (ds.system[i]!=DIV_SYSTEM_NULL) ds.systemLen=i+1;
    }
    int tchans=0;
    for (int i=0; i<ds.systemLen; i++) {
      tchans+=getChannelCount(ds.system[i]);
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
      sysFlagsPtr[i]=reader.readI();
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
      ds.limitSlides=reader.readC();
      ds.linearPitch=reader.readC();
      ds.loopModality=reader.readC();
      if (ds.version>=43) {
        ds.properNoiseLayout=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=43) {
        ds.waveDutyIsVol=reader.readC();
      } else {
        reader.readC();
      }

      if (ds.version>=45) {
        ds.resetMacroOnPorta=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=45) {
        ds.legacyVolumeSlides=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=45) {
        ds.compatibleArpeggio=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=45) {
        ds.noteOffResetsSlides=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=45) {
        ds.targetResetsSlides=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=47) {
        ds.arpNonPorta=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=47) {
        ds.algMacroBehavior=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=49) {
        ds.brokenShortcutSlides=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=50) {
        ds.ignoreDuplicateSlides=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=62) {
        ds.stopPortaOnNoteOff=reader.readC();
        ds.continuousVibrato=reader.readC();
      } else {
        reader.readC();
        reader.readC();
      }
      if (ds.version>=64) {
        ds.brokenDACMode=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=65) {
        ds.oneTickCut=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=66) {
        ds.newInsTriggersInPorta=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=69) {
        ds.arp0Reset=reader.readC();
      } else {
        reader.readC();
      }
    } else {
      for (int i=0; i<20; i++) reader.readC();
    }

    // pointers
    for (int i=0; i<ds.insLen; i++) {
      insPtr[i]=reader.readI();
    }
    for (int i=0; i<ds.waveLen; i++) {
      wavePtr[i]=reader.readI();
    }
    for (int i=0; i<ds.sampleLen; i++) {
      samplePtr[i]=reader.readI();
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
      ds.brokenSpeedSel=reader.readC();
      if (ds.version>=71) {
        ds.noSlidesOnFirstTick=reader.readC();
        ds.rowResetsArpPos=reader.readC();
        ds.ignoreJumpAtEnd=reader.readC();
      } else {
        reader.readC();
        reader.readC();
        reader.readC();
      }
      if (ds.version>=72) {
        ds.buggyPortaAfterSlide=reader.readC();
        ds.gbInsAffectsEnvelope=reader.readC();
      } else {
        reader.readC();
        reader.readC();
      }
      if (ds.version>=78) {
        ds.sharedExtStat=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=83) {
        ds.ignoreDACModeOutsideIntendedChannel=reader.readC();
        ds.e1e2AlsoTakePriority=reader.readC();
      } else {
        reader.readC();
        reader.readC();
      }
      if (ds.version>=84) {
        ds.newSegaPCM=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=85) {
        ds.fbPortaPause=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=86) {
        ds.snDutyReset=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=90) {
        ds.pitchMacroIsLinear=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=94) {
        ds.pitchSlideSpeed=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=97) {
        ds.oldOctaveBoundary=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=98) {
        ds.noOPN2Vol=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=99) {
        ds.newVolumeScaling=reader.readC();
        ds.volMacroLinger=reader.readC();
        ds.brokenOutVol=reader.readC();
      } else {
        reader.readC();
        reader.readC();
        reader.readC();
      }
      if (ds.version>=100) {
        ds.e1e2StopOnSameNote=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=101) {
        ds.brokenPortaArp=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=108) {
        ds.snNoLowPeriods=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=110) {
        ds.delayBehavior=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=113) {
        ds.jumpTreatment=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=115) {
        ds.autoSystem=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=117) {
        ds.disableSampleMacro=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=121) {
        ds.brokenOutVol2=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=130) {
        ds.oldArpStrategy=reader.readC();
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
      numberOfSubSongs=(unsigned char)reader.readC();
      reader.readC(); // reserved
      reader.readC();
      reader.readC();
      // pointers
      for (int i=0; i<numberOfSubSongs; i++) {
        subSongPtr[i]=reader.readI();
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
      ds.brokenPortaLegato=reader.readC();
      if (ds.version>=155) {
        ds.brokenFMOff=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=168) {
        ds.preNoteNoEffect=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=183) {
        ds.oldDPCM=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=184) {
        ds.resetArpPhaseOnNewNote=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=188) {
        ds.ceilVolumeScaling=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=191) {
        ds.oldAlwaysSetVolume=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=200) {
        ds.oldSampleOffset=reader.readC();
      } else {
        reader.readC();
      }
    }

    if (ds.version>=139) {
      subSong->speeds.len=reader.readC();
      for (int i=0; i<16; i++) {
        subSong->speeds.val[i]=reader.readC();
      }

      // grooves
      unsigned char grooveCount=reader.readC();
      ds.grooves.reserve(grooveCount);
      for (int i=0; i<grooveCount; i++) {
        DivGroovePattern gp;
        gp.len=reader.readC();
        for (int j=0; j<16; j++) {
          gp.val[j]=reader.readC();
        }

        ds.grooves.push_back(gp);
      }
    }

    if (ds.version>=156) {
      assetDirPtr[0]=reader.readI();
      assetDirPtr[1]=reader.readI();
      assetDirPtr[2]=reader.readI();
    }

    // read system flags
    if (ds.version>=119) {
      logD("reading chip flags...");
      for (int i=0; i<DIV_MAX_CHIPS; i++) {
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

    // read subsongs
    if (ds.version>=95) {
      ds.subsong.reserve(numberOfSubSongs);
      for (int i=0; i<numberOfSubSongs; i++) {
        ds.subsong.push_back(new DivSubSong);
        if (!reader.seek(subSongPtr[i],SEEK_SET)) {
          logE("couldn't seek to subsong %d!",i+1);
          lastError=fmt::sprintf("couldn't seek to subsong %d!",i+1);
          ds.unload();
          delete[] file;
          return false;
        }

        reader.read(magic,4);
        if (strcmp(magic,"SONG")!=0) {
          logE("%d: invalid subsong header!",i);
          lastError="invalid subsong header!";
          ds.unload();
          delete[] file;
          return false;
        }
        reader.readI();

        subSong=ds.subsong[i+1];
        subSong->timeBase=reader.readC();
        subSong->speeds.len=2;
        subSong->speeds.val[0]=reader.readC();
        subSong->speeds.val[1]=reader.readC();
        subSong->arpLen=reader.readC();
        subSong->hz=reader.readF();

        subSong->patLen=reader.readS();
        subSong->ordersLen=reader.readS();

        subSong->hilightA=reader.readC();
        subSong->hilightB=reader.readC();

        if (ds.version>=96) {
          subSong->virtualTempoN=reader.readS();
          subSong->virtualTempoD=reader.readS();
        } else {
          reader.readI();
        }

        subSong->name=reader.readString();
        subSong->notes=reader.readString();

        logD("reading orders of subsong %d (%d)...",i+1,subSong->ordersLen);
        for (int j=0; j<tchans; j++) {
          for (int k=0; k<subSong->ordersLen; k++) {
            subSong->orders.ord[j][k]=reader.readC();
          }
        }

        for (int i=0; i<tchans; i++) {
          subSong->pat[i].effectCols=reader.readC();
        }

        for (int i=0; i<tchans; i++) {
          if (ds.version<189) {
            subSong->chanShow[i]=reader.readC();
            subSong->chanShowChanOsc[i]=subSong->chanShow[i];
          } else {
            unsigned char tempchar=reader.readC();
            subSong->chanShow[i]=tempchar&1;
            subSong->chanShowChanOsc[i]=tempchar&2;
          }
        }

        for (int i=0; i<tchans; i++) {
          subSong->chanCollapse[i]=reader.readC();
        }

        for (int i=0; i<tchans; i++) {
          subSong->chanName[i]=reader.readString();
        }

        for (int i=0; i<tchans; i++) {
          subSong->chanShortName[i]=reader.readString();
        }

        if (ds.version>=139) {
          subSong->speeds.len=reader.readC();
          for (int i=0; i<16; i++) {
            subSong->speeds.val[i]=reader.readC();
          }
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
            if (note==180) {
              pat->data[j][0]=100;
              pat->data[j][1]=0;
            } else if (note==181) {
              pat->data[j][0]=101;
              pat->data[j][1]=0;
            } else if (note==182) {
              pat->data[j][0]=102;
              pat->data[j][1]=0;
            } else if (note<180) {
              pat->data[j][0]=newFormatNotes[note];
              pat->data[j][1]=newFormatOctaves[note];
            } else {
              pat->data[j][0]=0;
              pat->data[j][1]=0;
            }
          }
          if (mask&2) { // instrument
            pat->data[j][2]=(unsigned char)reader.readC();
          }
          if (mask&4) { // volume
            pat->data[j][3]=(unsigned char)reader.readC();
          }
          for (unsigned char k=0; k<16; k++) {
            if (effectMask&(1<<k)) {
              pat->data[j][4+k]=(unsigned char)reader.readC();
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
          pat->data[j][0]=reader.readS();
          pat->data[j][1]=reader.readS();
          pat->data[j][2]=reader.readS();
          pat->data[j][3]=reader.readS();
          for (int k=0; k<ds.subsong[subs]->pat[chan].effectCols; k++) {
            pat->data[j][4+(k<<1)]=reader.readS();
            pat->data[j][5+(k<<1)]=reader.readS();
          }
          if (pat->data[j][0]==0 && pat->data[j][1]!=0) {
            logD("what? %d:%d:%d note %d octave %d",chan,i,j,pat->data[j][0],pat->data[j][1]);
            pat->data[j][0]=12;
            pat->data[j][1]--;
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
          case DIV_SYSTEM_YM2610:
          case DIV_SYSTEM_YM2610_EXT:
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
          ds.system[i]==DIV_SYSTEM_YM2610_EXT ||
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

    // SrgaPCM slide compat
    if (ds.version<153) {
      for (int i=0; i<ds.systemLen; i++) {
        if (ds.system[i]==DIV_SYSTEM_SEGAPCM || ds.system[i]==DIV_SYSTEM_SEGAPCM_COMPAT) {
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


    if (active) quitDispatch();
    BUSY_BEGIN_SOFT;
    saveLock.lock();
    song.unload();
    song=ds;
    changeSong(0);
    recalcChans();
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

SafeWriter* DivEngine::saveFur(bool notPrimary, bool newPatternFormat) {
  saveLock.lock();
  std::vector<int> subSongPtr;
  std::vector<int> sysFlagsPtr;
  std::vector<int> insPtr;
  std::vector<int> wavePtr;
  std::vector<int> samplePtr;
  std::vector<int> patPtr;
  int assetDirPtr[3];
  size_t ptrSeek, subSongPtrSeek, sysFlagsPtrSeek, blockStartSeek, blockEndSeek, assetDirPtrSeek;
  size_t subSongIndex=0;
  DivSubSong* subSong=song.subsong[subSongIndex];
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
  if (song.wave.size()>256) {
    logE("maximum number of wavetables is 256!");
    lastError="maximum number of wavetables is 256";
    saveLock.unlock();
    return NULL;
  }
  if (song.sample.size()>256) {
    logE("maximum number of samples is 256!");
    lastError="maximum number of samples is 256";
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
    for (int i=0; i<chans; i++) {
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
    for (int i=0; i<chans; i++) {
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
  w->write("INFO",4);
  blockStartSeek=w->tell();
  w->writeI(0);

  w->writeC(subSong->timeBase);
  // these are for compatibility
  w->writeC(subSong->speeds.val[0]);
  w->writeC((subSong->speeds.len>=2)?subSong->speeds.val[1]:subSong->speeds.val[0]);
  w->writeC(subSong->arpLen);
  w->writeF(subSong->hz);
  w->writeS(subSong->patLen);
  w->writeS(subSong->ordersLen);
  w->writeC(subSong->hilightA);
  w->writeC(subSong->hilightB);
  w->writeS(song.insLen);
  w->writeS(song.waveLen);
  w->writeS(song.sampleLen);
  w->writeI(patsToWrite.size());

  for (int i=0; i<DIV_MAX_CHIPS; i++) {
    if (i>=song.systemLen) {
      w->writeC(0);
    } else {
      w->writeC(systemToFileFur(song.system[i]));
    }
  }

  for (int i=0; i<DIV_MAX_CHIPS; i++) {
    w->writeC(song.systemVol[i]*64.0f);
  }

  for (int i=0; i<DIV_MAX_CHIPS; i++) {
    w->writeC(song.systemPan[i]*127.0f);
  }

  // chip flags (we'll seek here later)
  sysFlagsPtrSeek=w->tell();
  for (int i=0; i<DIV_MAX_CHIPS; i++) {
    w->writeI(0);
  }

  // song name
  w->writeString(song.name,false);
  // song author
  w->writeString(song.author,false);

  w->writeF(song.tuning);
  
  // compatibility flags
  w->writeC(song.limitSlides);
  w->writeC(song.linearPitch);
  w->writeC(song.loopModality);
  w->writeC(song.properNoiseLayout);
  w->writeC(song.waveDutyIsVol);
  w->writeC(song.resetMacroOnPorta);
  w->writeC(song.legacyVolumeSlides);
  w->writeC(song.compatibleArpeggio);
  w->writeC(song.noteOffResetsSlides);
  w->writeC(song.targetResetsSlides);
  w->writeC(song.arpNonPorta);
  w->writeC(song.algMacroBehavior);
  w->writeC(song.brokenShortcutSlides);
  w->writeC(song.ignoreDuplicateSlides);
  w->writeC(song.stopPortaOnNoteOff);
  w->writeC(song.continuousVibrato);
  w->writeC(song.brokenDACMode);
  w->writeC(song.oneTickCut);
  w->writeC(song.newInsTriggersInPorta);
  w->writeC(song.arp0Reset);

  ptrSeek=w->tell();
  // instrument pointers (we'll seek here later)
  for (int i=0; i<song.insLen; i++) {
    w->writeI(0);
  }

  // wavetable pointers (we'll seek here later)
  for (int i=0; i<song.waveLen; i++) {
    w->writeI(0);
  }

  // sample pointers (we'll seek here later)
  for (int i=0; i<song.sampleLen; i++) {
    w->writeI(0);
  }

  // pattern pointers (we'll seek here later)
  for (size_t i=0; i<patsToWrite.size(); i++) {
    w->writeI(0);
  }

  for (int i=0; i<chans; i++) {
    for (int j=0; j<subSong->ordersLen; j++) {
      w->writeC(subSong->orders.ord[i][j]);
    }
  }

  for (int i=0; i<chans; i++) {
    w->writeC(subSong->pat[i].effectCols);
  }

  for (int i=0; i<chans; i++) {
    w->writeC(
      (subSong->chanShow[i]?1:0)|
      (subSong->chanShowChanOsc[i]?2:0)
    );
  }

  for (int i=0; i<chans; i++) {
    w->writeC(subSong->chanCollapse[i]);
  }

  for (int i=0; i<chans; i++) {
    w->writeString(subSong->chanName[i],false);
  }

  for (int i=0; i<chans; i++) {
    w->writeString(subSong->chanShortName[i],false);
  }

  w->writeString(song.notes,false);

  w->writeF(song.masterVol);

  // extended compat flags
  w->writeC(song.brokenSpeedSel);
  w->writeC(song.noSlidesOnFirstTick);
  w->writeC(song.rowResetsArpPos);
  w->writeC(song.ignoreJumpAtEnd);
  w->writeC(song.buggyPortaAfterSlide);
  w->writeC(song.gbInsAffectsEnvelope);
  w->writeC(song.sharedExtStat);
  w->writeC(song.ignoreDACModeOutsideIntendedChannel);
  w->writeC(song.e1e2AlsoTakePriority);
  w->writeC(song.newSegaPCM);
  w->writeC(song.fbPortaPause);
  w->writeC(song.snDutyReset);
  w->writeC(song.pitchMacroIsLinear);
  w->writeC(song.pitchSlideSpeed);
  w->writeC(song.oldOctaveBoundary);
  w->writeC(song.noOPN2Vol);
  w->writeC(song.newVolumeScaling);
  w->writeC(song.volMacroLinger);
  w->writeC(song.brokenOutVol);
  w->writeC(song.e1e2StopOnSameNote);
  w->writeC(song.brokenPortaArp);
  w->writeC(song.snNoLowPeriods);
  w->writeC(song.delayBehavior);
  w->writeC(song.jumpTreatment);
  w->writeC(song.autoSystem);
  w->writeC(song.disableSampleMacro);
  w->writeC(song.brokenOutVol2);
  w->writeC(song.oldArpStrategy);

  // first subsong virtual tempo
  w->writeS(subSong->virtualTempoN);
  w->writeS(subSong->virtualTempoD);
  
  // subsong list
  w->writeString(subSong->name,false);
  w->writeString(subSong->notes,false);
  w->writeC((unsigned char)(song.subsong.size()-1));
  w->writeC(0); // reserved
  w->writeC(0);
  w->writeC(0);
  subSongPtrSeek=w->tell();
  // subsong pointers (we'll seek here later)
  for (size_t i=0; i<(song.subsong.size()-1); i++) {
    w->writeI(0);
  }

  // additional metadata
  w->writeString(song.systemName,false);
  w->writeString(song.category,false);
  w->writeString(song.nameJ,false);
  w->writeString(song.authorJ,false);
  w->writeString(song.systemNameJ,false);
  w->writeString(song.categoryJ,false);

  // system output config
  for (int i=0; i<song.systemLen; i++) {
    w->writeF(song.systemVol[i]);
    w->writeF(song.systemPan[i]);
    w->writeF(song.systemPanFR[i]);
  }
  w->writeI(song.patchbay.size());
  for (unsigned int i: song.patchbay) {
    w->writeI(i);
  }
  w->writeC(song.patchbayAuto);

  // even more compat flags
  w->writeC(song.brokenPortaLegato);
  w->writeC(song.brokenFMOff);
  w->writeC(song.preNoteNoEffect);
  w->writeC(song.oldDPCM);
  w->writeC(song.resetArpPhaseOnNewNote);
  w->writeC(song.ceilVolumeScaling);
  w->writeC(song.oldAlwaysSetVolume);
  w->writeC(song.oldSampleOffset);

  // speeds of first song
  w->writeC(subSong->speeds.len);
  for (int i=0; i<16; i++) {
    w->writeC(subSong->speeds.val[i]);
  }

  // groove list
  w->writeC((unsigned char)song.grooves.size());
  for (const DivGroovePattern& i: song.grooves) {
    w->writeC(i.len);
    for (int j=0; j<16; j++) {
      w->writeC(i.val[j]);
    }
  }

  // asset dir pointers (we'll seek here later)
  assetDirPtrSeek=w->tell();
  w->writeI(0);
  w->writeI(0);
  w->writeI(0);

  blockEndSeek=w->tell();
  w->seek(blockStartSeek,SEEK_SET);
  w->writeI(blockEndSeek-blockStartSeek-4);
  w->seek(0,SEEK_END);

  /// SUBSONGS
  subSongPtr.reserve(song.subsong.size() - 1);
  for (subSongIndex=1; subSongIndex<song.subsong.size(); subSongIndex++) {
    subSong=song.subsong[subSongIndex];
    subSongPtr.push_back(w->tell());
    w->write("SONG",4);
    blockStartSeek=w->tell();
    w->writeI(0);

    w->writeC(subSong->timeBase);
    w->writeC(subSong->speeds.val[0]);
    w->writeC((subSong->speeds.len>=2)?subSong->speeds.val[1]:subSong->speeds.val[0]);
    w->writeC(subSong->arpLen);
    w->writeF(subSong->hz);
    w->writeS(subSong->patLen);
    w->writeS(subSong->ordersLen);
    w->writeC(subSong->hilightA);
    w->writeC(subSong->hilightB);
    w->writeS(subSong->virtualTempoN);
    w->writeS(subSong->virtualTempoD);

    w->writeString(subSong->name,false);
    w->writeString(subSong->notes,false);

    for (int i=0; i<chans; i++) {
      for (int j=0; j<subSong->ordersLen; j++) {
        w->writeC(subSong->orders.ord[i][j]);
      }
    }

    for (int i=0; i<chans; i++) {
      w->writeC(subSong->pat[i].effectCols);
    }

    for (int i=0; i<chans; i++) {
      w->writeC(
        (subSong->chanShow[i]?1:0)|
        (subSong->chanShowChanOsc[i]?2:0)
      );
    }

    for (int i=0; i<chans; i++) {
      w->writeC(subSong->chanCollapse[i]);
    }

    for (int i=0; i<chans; i++) {
      w->writeString(subSong->chanName[i],false);
    }

    for (int i=0; i<chans; i++) {
      w->writeString(subSong->chanShortName[i],false);
    }

    // speeds
    w->writeC(subSong->speeds.len);
    for (int i=0; i<16; i++) {
      w->writeC(subSong->speeds.val[i]);
    }

    blockEndSeek=w->tell();
    w->seek(blockStartSeek,SEEK_SET);
    w->writeI(blockEndSeek-blockStartSeek-4);
    w->seek(0,SEEK_END);
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

    if (newPatternFormat) {
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

        if (pat->data[j][0]==100) {
          finalNote=180;
        } else if (pat->data[j][0]==101) { // note release
          finalNote=181;
        } else if (pat->data[j][0]==102) { // macro release
          finalNote=182;
        } else if (pat->data[j][1]==0 && pat->data[j][0]==0) {
          finalNote=255;
        } else {
          int seek=(pat->data[j][0]+(signed char)pat->data[j][1]*12)+60;
          if (seek<0 || seek>=180) {
            finalNote=255;
          } else {
            finalNote=seek;
          }
        }

        if (finalNote!=255) mask|=1; // note
        if (pat->data[j][2]!=-1) mask|=2; // instrument
        if (pat->data[j][3]!=-1) mask|=4; // volume
        for (int k=0; k<song.subsong[i.subsong]->pat[i.chan].effectCols*2; k+=2) {
          if (k==0) {
            if (pat->data[j][4+k]!=-1) mask|=8;
            if (pat->data[j][5+k]!=-1) mask|=16;
          } else if (k<8) {
            if (pat->data[j][4+k]!=-1 || pat->data[j][5+k]!=-1) mask|=32;
          } else {
            if (pat->data[j][4+k]!=-1 || pat->data[j][5+k]!=-1) mask|=64;
          }

          if (pat->data[j][4+k]!=-1) effectMask|=(1<<k);
          if (pat->data[j][5+k]!=-1) effectMask|=(2<<k);
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
          if (mask&2) w->writeC(pat->data[j][2]);
          if (mask&4) w->writeC(pat->data[j][3]);
          if (mask&8) w->writeC(pat->data[j][4]);
          if (mask&16) w->writeC(pat->data[j][5]);
          if (mask&32) {
            if (effectMask&4) w->writeC(pat->data[j][6]);
            if (effectMask&8) w->writeC(pat->data[j][7]);
            if (effectMask&16) w->writeC(pat->data[j][8]);
            if (effectMask&32) w->writeC(pat->data[j][9]);
            if (effectMask&64) w->writeC(pat->data[j][10]);
            if (effectMask&128) w->writeC(pat->data[j][11]);
          }
          if (mask&64) {
            if (effectMask&256) w->writeC(pat->data[j][12]);
            if (effectMask&512) w->writeC(pat->data[j][13]);
            if (effectMask&1024) w->writeC(pat->data[j][14]);
            if (effectMask&2048) w->writeC(pat->data[j][15]);
            if (effectMask&4096) w->writeC(pat->data[j][16]);
            if (effectMask&8192) w->writeC(pat->data[j][17]);
            if (effectMask&16384) w->writeC(pat->data[j][18]);
            if (effectMask&32768) w->writeC(pat->data[j][19]);
          }
        }
      }

      // stop
      w->writeC(0xff);
    } else {
      w->write("PATR",4);
      blockStartSeek=w->tell();
      w->writeI(0);

      w->writeS(i.chan);
      w->writeS(i.pat);
      w->writeS(i.subsong);

      w->writeS(0); // reserved

      for (int j=0; j<song.subsong[i.subsong]->patLen; j++) {
        w->writeS(pat->data[j][0]); // note
        w->writeS(pat->data[j][1]); // octave
        w->writeS(pat->data[j][2]); // instrument
        w->writeS(pat->data[j][3]); // volume
#ifdef TA_BIG_ENDIAN
        for (int k=0; k<song.subsong[i.subsong]->pat[i.chan].effectCols*2; k++) {
          w->writeS(pat->data[j][4+k]);
        }
#else
        w->write(&pat->data[j][4],2*song.subsong[i.subsong]->pat[i.chan].effectCols*2); // effects
#endif
      }

      w->writeString(pat->name,false);
    }

    blockEndSeek=w->tell();
    w->seek(blockStartSeek,SEEK_SET);
    w->writeI(blockEndSeek-blockStartSeek-4);
    w->seek(0,SEEK_END);
  }

  /// POINTERS
  w->seek(ptrSeek,SEEK_SET);

  for (int i=0; i<song.insLen; i++) {
    w->writeI(insPtr[i]);
  }

  // wavetable pointers
  for (int i=0; i<song.waveLen; i++) {
    w->writeI(wavePtr[i]);
  }

  // sample pointers
  for (int i=0; i<song.sampleLen; i++) {
    w->writeI(samplePtr[i]);
  }

  // pattern pointers
  for (int i: patPtr) {
    w->writeI(i);
  }

  // subsong pointers
  w->seek(subSongPtrSeek,SEEK_SET);
  for (size_t i=0; i<(song.subsong.size()-1); i++) {
    w->writeI(subSongPtr[i]);
  }

  // flag pointers
  w->seek(sysFlagsPtrSeek,SEEK_SET);
  for (size_t i=0; i<sysFlagsPtr.size(); i++) {
    w->writeI(sysFlagsPtr[i]);
  }

  // asset dir pointers
  w->seek(assetDirPtrSeek,SEEK_SET);
  for (size_t i=0; i<3; i++) {
    w->writeI(assetDirPtr[i]);
  }

  saveLock.unlock();
  return w;
}

