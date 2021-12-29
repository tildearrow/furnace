#include "engine.h"
#include "instrument.h"
#include "safeReader.h"
#include "../ta-log.h"
#include "../audio/sdl.h"
#include <cstddef>
#include <stdexcept>
#ifndef _WIN32
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#endif
#ifdef HAVE_JACK
#include "../audio/jack.h"
#endif
#include "platform/genesis.h"
#include "platform/genesisext.h"
#include "platform/sms.h"
#include "platform/gb.h"
#include "platform/pce.h"
#include "platform/nes.h"
#include "platform/c64.h"
#include "platform/arcade.h"
#include "platform/ym2610.h"
#include "platform/ym2610ext.h"
#include "platform/dummy.h"
#include <math.h>
#include <zlib.h>
#include <sndfile.h>
#include <fmt/printf.h>

void process(void* u, float** in, float** out, int inChans, int outChans, unsigned int size) {
  ((DivEngine*)u)->nextBuf(in,out,inChans,outChans,size);
}

#define DIV_READ_SIZE 131072
#define DIV_DMF_MAGIC ".DelekDefleMask."

struct InflateBlock {
  unsigned char* buf;
  size_t len;
  size_t blockSize;
  InflateBlock(size_t s) {
    buf=new unsigned char[s];
    len=s;
    blockSize=0;
  }
  ~InflateBlock() {
    delete[] buf;
    len=0;
  }
};

DivSystem systemFromFile(unsigned char val) {
  switch (val) {
    case 0x01:
      return DIV_SYSTEM_YMU759;
    case 0x02:
      return DIV_SYSTEM_GENESIS;
    case 0x03:
      return DIV_SYSTEM_SMS;
    case 0x04:
      return DIV_SYSTEM_GB;
    case 0x05:
      return DIV_SYSTEM_PCE;
    case 0x06:
      return DIV_SYSTEM_NES;
    case 0x07:
      return DIV_SYSTEM_C64_8580;
    case 0x08:
      return DIV_SYSTEM_ARCADE;
    case 0x09:
      return DIV_SYSTEM_YM2610;
    case 0x42:
      return DIV_SYSTEM_GENESIS_EXT;
    case 0x47:
      return DIV_SYSTEM_C64_6581;
    case 0x49:
      return DIV_SYSTEM_YM2610_EXT;
  }
  return DIV_SYSTEM_NULL;
}

unsigned char systemToFile(DivSystem val) {
  switch (val) {
    case DIV_SYSTEM_YMU759:
      return 0x01;
    case DIV_SYSTEM_GENESIS:
      return 0x02;
    case DIV_SYSTEM_SMS:
      return 0x03;
    case DIV_SYSTEM_GB:
      return 0x04;
    case DIV_SYSTEM_PCE:
      return 0x05;
    case DIV_SYSTEM_NES:
      return 0x06;
    case DIV_SYSTEM_C64_8580:
      return 0x07;
    case DIV_SYSTEM_ARCADE:
      return 0x08;
    case DIV_SYSTEM_YM2610:
      return 0x09;
    case DIV_SYSTEM_GENESIS_EXT:
      return 0x42;
    case DIV_SYSTEM_C64_6581:
      return 0x47;
    case DIV_SYSTEM_YM2610_EXT:
      return 0x49;
    case DIV_SYSTEM_NULL:
      return 0;
  }
  return 0;
}

int DivEngine::getChannelCount(DivSystem sys) {
  switch (sys) {
    case DIV_SYSTEM_NULL:
      return 0;
    case DIV_SYSTEM_YMU759:
      return 17;
    case DIV_SYSTEM_GENESIS:
      return 10;
    case DIV_SYSTEM_SMS:
    case DIV_SYSTEM_GB:
      return 4;
    case DIV_SYSTEM_PCE:
      return 6;
    case DIV_SYSTEM_NES:
      return 5;
    case DIV_SYSTEM_C64_6581:
    case DIV_SYSTEM_C64_8580:
      return 3;
    case DIV_SYSTEM_ARCADE:
    case DIV_SYSTEM_GENESIS_EXT:
    case DIV_SYSTEM_YM2610:
      return 13;      
    case DIV_SYSTEM_YM2610_EXT:
      return 16;
  }
  return 0;
}

const char* DivEngine::getSystemName(DivSystem sys) {
  switch (sys) {
    case DIV_SYSTEM_NULL:
      return "Unknown";
    case DIV_SYSTEM_YMU759:
      return "Yamaha YMU759";
    case DIV_SYSTEM_GENESIS:
      return "Sega Genesis/Mega Drive";
    case DIV_SYSTEM_SMS:
      return "Sega Master System";
    case DIV_SYSTEM_GB:
      return "Game Boy";
    case DIV_SYSTEM_PCE:
      return "PC Engine/TurboGrafx-16";
    case DIV_SYSTEM_NES:
      return "NES";
    case DIV_SYSTEM_C64_6581:
      return "Commodore 64 with 6581";
    case DIV_SYSTEM_C64_8580:
      return "Commodore 64 with 8580";
    case DIV_SYSTEM_ARCADE:
      return "Arcade";
    case DIV_SYSTEM_GENESIS_EXT:
      return "Sega Genesis Extended Channel 3";
    case DIV_SYSTEM_YM2610:
      return "Neo Geo";
    case DIV_SYSTEM_YM2610_EXT:
      return "Neo Geo Extended Channel 2";
  }
  return "Unknown";
}

bool DivEngine::isFMSystem(DivSystem sys) {
  return (sys==DIV_SYSTEM_GENESIS ||
          sys==DIV_SYSTEM_GENESIS_EXT ||
          sys==DIV_SYSTEM_ARCADE ||
          sys==DIV_SYSTEM_YM2610 ||
          sys==DIV_SYSTEM_YM2610_EXT ||
          sys==DIV_SYSTEM_YMU759);
}

bool DivEngine::isSTDSystem(DivSystem sys) {
  return (sys!=DIV_SYSTEM_ARCADE && sys!=DIV_SYSTEM_YMU759);
}

int DivEngine::getWaveRes(DivSystem sys) {
  switch (sys) {
    case DIV_SYSTEM_GB:
      return 15;
    case DIV_SYSTEM_PCE:
      return 31;
    default:
      return 31;
  }
  return 31;
}

const char* chanNames[11][17]={
  {"Channel 1", "Channel 2", "Channel 3", "Channel 4", "Channel 5", "Channel 6", "Channel 7", "Channel 8", "Channel 9", "Channel 10", "Channel 11", "Channel 12", "Channel 13", "Channel 14", "Channel 15", "Channel 16", "PCM"}, // YMU759
  {"FM 1", "FM 2", "FM 3", "FM 4", "FM 5", "FM 6", "Square 1", "Square 2", "Square 3", "Noise"}, // Genesis
  {"FM 1", "FM 2", "FM 3 OP1", "FM 3 OP2", "FM 3 OP3", "FM 3 OP4", "FM 4", "FM 5", "FM 6", "Square 1", "Square 2", "Square 3", "Noise"}, // Genesis (extended channel 3)
  {"Square 1", "Square 2", "Square 3", "Noise"}, // SMS
  {"Pulse 1", "Pulse 2", "Wavetable", "Noise"}, // GB
  {"Channel 1", "Channel 2", "Channel 3", "Channel 4", "Channel 5", "Channel 6"}, // PCE
  {"Pulse 1", "Pulse 2", "Triangle", "Noise", "PCM"}, // NES
  {"Channel 1", "Channel 2", "Channel 3"}, // C64
  {"FM 1", "FM 2", "FM 3", "FM 4", "FM 5", "FM 6", "FM 7", "FM 8", "Sample 1", "Sample 2", "Sample 3", "Sample 4", "Sample 5"}, // Arcade
  {"FM 1", "FM 2", "FM 3", "FM 4", "Square 1", "Square 2", "Square 3", "Sample 1", "Sample 2", "Sample 3", "Sample 4", "Sample 5", "Sample 6"}, // YM2610
  {"FM 1", "FM 2 OP1", "FM 2 OP2", "FM 2 OP3", "FM 2 OP4", "FM 3", "FM 4", "Square 1", "Square 2", "Square 3", "Sample 1", "Sample 2", "Sample 3", "Sample 4", "Sample 5", "Sample 6"}, // YM2610 (extended channel 2)
};

const char* chanShortNames[11][17]={
  {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "PCM"}, // YMU759
  {"F1", "F2", "F3", "F4", "F5", "F6", "S1", "S2", "S3", "NO"}, // Genesis
  {"F1", "F2", "O1", "O2", "O3", "O4", "F4", "F5", "F6", "S1", "S2", "S3", "S4"}, // Genesis (extended channel 3)
  {"S1", "S2", "S3", "NO"}, // SMS
  {"S1", "S2", "WA", "NO"}, // GB
  {"CH1", "CH2", "CH3", "CH4", "CH5", "CH6"}, // PCE
  {"S1", "S2", "TR", "NO", "PCM"}, // NES
  {"CH1", "CH2", "CH3"}, // C64
  {"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "P1", "P2", "P3", "P4", "P5"}, // Arcade
  {"F1", "F2", "F3", "F4", "S1", "S2", "S3", "P1", "P2", "P3", "P4", "P5", "P6"}, // YM2610
  {"F1", "O1", "O2", "O3", "O4", "F3", "F4", "S1", "S2", "S3", "P1", "P2", "P3", "P4", "P5", "P6"}, // YM2610 (extended channel 2)
};

const int chanTypes[11][17]={
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4}, // YMU759
  {0, 0, 0, 0, 0, 0, 1, 1, 1, 2}, // Genesis
  {0, 0, 5, 5, 5, 5, 0, 0, 0, 1, 1, 1, 2}, // Genesis (extended channel 3)
  {1, 1, 1, 2}, // SMS
  {1, 1, 3, 2}, // GB
  {3, 3, 3, 3, 3, 3}, // PCE
  {1, 1, 3, 2, 4}, // NES
  {2, 2, 2}, // C64
  {0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4}, // Arcade
  {0, 0, 0, 0, 1, 1, 1, 4, 4, 4, 4, 4, 4}, // YM2610
  {0, 5, 5, 5, 5, 0, 0, 1, 1, 1, 4, 4, 4, 4, 4, 4}, // YM2610 (extended channel 2)
};

const char* DivEngine::getChannelName(int chan) {
  switch (song.system) {
    case DIV_SYSTEM_NULL: case DIV_SYSTEM_YMU759:
      return chanNames[0][chan];
      break;
    case DIV_SYSTEM_GENESIS:
      return chanNames[1][chan];
      break;
    case DIV_SYSTEM_GENESIS_EXT:
      return chanNames[2][chan];
      break;
    case DIV_SYSTEM_SMS:
      return chanNames[3][chan];
      break;
    case DIV_SYSTEM_GB:
      return chanNames[4][chan];
      break;
    case DIV_SYSTEM_PCE:
      return chanNames[5][chan];
      break;
    case DIV_SYSTEM_NES:
      return chanNames[6][chan];
      break;
    case DIV_SYSTEM_C64_6581: case DIV_SYSTEM_C64_8580:
      return chanNames[7][chan];
      break;
    case DIV_SYSTEM_ARCADE:
      return chanNames[8][chan];
      break;
    case DIV_SYSTEM_YM2610:
      return chanNames[9][chan];
      break;
    case DIV_SYSTEM_YM2610_EXT:
      return chanNames[10][chan];
      break;
  }
  return "??";
}

const char* DivEngine::getChannelShortName(int chan) {
  switch (song.system) {
    case DIV_SYSTEM_NULL: case DIV_SYSTEM_YMU759:
      return chanShortNames[0][chan];
      break;
    case DIV_SYSTEM_GENESIS:
      return chanShortNames[1][chan];
      break;
    case DIV_SYSTEM_GENESIS_EXT:
      return chanShortNames[2][chan];
      break;
    case DIV_SYSTEM_SMS:
      return chanShortNames[3][chan];
      break;
    case DIV_SYSTEM_GB:
      return chanShortNames[4][chan];
      break;
    case DIV_SYSTEM_PCE:
      return chanShortNames[5][chan];
      break;
    case DIV_SYSTEM_NES:
      return chanShortNames[6][chan];
      break;
    case DIV_SYSTEM_C64_6581: case DIV_SYSTEM_C64_8580:
      return chanShortNames[7][chan];
      break;
    case DIV_SYSTEM_ARCADE:
      return chanShortNames[8][chan];
      break;
    case DIV_SYSTEM_YM2610:
      return chanShortNames[9][chan];
      break;
    case DIV_SYSTEM_YM2610_EXT:
      return chanShortNames[10][chan];
      break;
  }
  return "??";
}

int DivEngine::getChannelType(int chan) {
  switch (song.system) {
    case DIV_SYSTEM_NULL: case DIV_SYSTEM_YMU759:
      return chanTypes[0][chan];
      break;
    case DIV_SYSTEM_GENESIS:
      return chanTypes[1][chan];
      break;
    case DIV_SYSTEM_GENESIS_EXT:
      return chanTypes[2][chan];
      break;
    case DIV_SYSTEM_SMS:
      return chanTypes[3][chan];
      break;
    case DIV_SYSTEM_GB:
      return chanTypes[4][chan];
      break;
    case DIV_SYSTEM_PCE:
      return chanTypes[5][chan];
      break;
    case DIV_SYSTEM_NES:
      return chanTypes[6][chan];
      break;
    case DIV_SYSTEM_C64_6581: case DIV_SYSTEM_C64_8580:
      return chanTypes[7][chan];
      break;
    case DIV_SYSTEM_ARCADE:
      return chanTypes[8][chan];
      break;
    case DIV_SYSTEM_YM2610:
      return chanTypes[9][chan];
      break;
    case DIV_SYSTEM_YM2610_EXT:
      return chanTypes[10][chan];
      break;
  }
  return 1;
}

int DivEngine::getMaxVolume() {
  switch (song.system) {
    case DIV_SYSTEM_PCE:
      return 31;
    default:
      return 15;
  }
  return 127;
}

int DivEngine::getMaxDuty() {
  switch (song.system) {
    case DIV_SYSTEM_YM2610: case DIV_SYSTEM_YM2610_EXT:
      return 31;
    case DIV_SYSTEM_C64_6581: case DIV_SYSTEM_C64_8580:
      return 8;
    case DIV_SYSTEM_PCE:
      return 0;
    default:
      return 3;
  }
  return 3;
}

int DivEngine::getMaxWave() {
  switch (song.system) {
    case DIV_SYSTEM_PCE: case DIV_SYSTEM_GB:
      return 63;
    case DIV_SYSTEM_YM2610: case DIV_SYSTEM_YM2610_EXT:
      return 7;
    case DIV_SYSTEM_C64_6581: case DIV_SYSTEM_C64_8580:
      return 8;
    default:
      return 0;
  }
  return 0;
}

bool DivEngine::load(unsigned char* f, size_t slen) {
  unsigned char* file;
  size_t len;
  if (slen<16) {
    logE("too small!");
    lastError="file is too small";
    delete[] f;
    return false;
  }
  if (memcmp(f,DIV_DMF_MAGIC,16)!=0) {
    logD("loading as zlib...\n");
    // try zlib
    z_stream zl;
    memset(&zl,0,sizeof(z_stream));

    zl.avail_in=slen;
    zl.next_in=(Bytef*)f;
    zl.zalloc=NULL;
    zl.zfree=NULL;
    zl.opaque=NULL;

    int nextErr;
    nextErr=inflateInit(&zl);
    if (nextErr!=Z_OK) {
      if (zl.msg==NULL) {
        logE("zlib error: unknown! %d\n",nextErr);
      } else {
        logE("zlib error: %s\n",zl.msg);
      }
      inflateEnd(&zl);
      delete[] f;
      lastError="not a .dmf song";
      return false;
    }

    std::vector<InflateBlock*> blocks;
    while (true) {
      InflateBlock* ib=new InflateBlock(DIV_READ_SIZE);
      zl.next_out=ib->buf;
      zl.avail_out=ib->len;

      nextErr=inflate(&zl,Z_SYNC_FLUSH);
      if (nextErr!=Z_OK && nextErr!=Z_STREAM_END) {
        if (zl.msg==NULL) {
          logE("zlib error: unknown error! %d\n",nextErr);
          lastError="unknown decompression error";
        } else {
          logE("zlib inflate: %s\n",zl.msg);
          lastError=fmt::sprintf("decompression error: %s",zl.msg);
        }
        for (InflateBlock* i: blocks) delete i;
        blocks.clear();
        delete ib;
        inflateEnd(&zl);
        delete[] f;
        return false;
      }
      ib->blockSize=ib->len-zl.avail_out;
      blocks.push_back(ib);
      if (nextErr==Z_STREAM_END) {
        break;
      }
    }
    nextErr=inflateEnd(&zl);
    if (nextErr!=Z_OK) {
      if (zl.msg==NULL) {
        logE("zlib end error: unknown error! %d\n",nextErr);
        lastError="unknown decompression finish error";
      } else {
        logE("zlib end: %s\n",zl.msg);
        lastError=fmt::sprintf("decompression finish error: %s",zl.msg);
      }
      for (InflateBlock* i: blocks) delete i;
      blocks.clear();
      delete[] f;
      return false;
    }

    size_t finalSize=0;
    size_t curSeek=0;
    for (InflateBlock* i: blocks) {
      finalSize+=i->blockSize;
    }
    if (finalSize<1) {
      logE("compressed too small!\n");
      lastError="file too small";
      for (InflateBlock* i: blocks) delete i;
      blocks.clear();
      delete[] f;
      return false;
    }
    file=new unsigned char[finalSize];
    for (InflateBlock* i: blocks) {
      memcpy(&file[curSeek],i->buf,i->blockSize);
      curSeek+=i->blockSize;
      delete i;
    }
    blocks.clear();
    len=finalSize;
    delete[] f;
  } else {
    logD("loading as uncompressed\n");
    file=(unsigned char*)f;
    len=slen;
  }
  if (memcmp(file,DIV_DMF_MAGIC,16)!=0) {
    logE("not a valid module!\n");
    lastError="not a .dmf song";
    delete[] file;
    return false;
  }
  SafeReader reader=SafeReader(file,len);
  try {
    DivSong ds;

    ds.nullWave.len=32;
    for (int i=0; i<32; i++) {
      ds.nullWave.data[i]=15;
    }

    if (!reader.seek(16,SEEK_SET)) {
      logE("premature end of file!");
      lastError="incomplete file";
      delete[] file;
      return false;
    }
    ds.version=reader.readC();
    logI("module version %d (0x%.2x)\n",ds.version,ds.version);
    if (ds.version>0x18) {
      logW("this version is not supported by Furnace yet!\n");
      lastError="this version is not supported by Furnace yet";
      delete[] file;
      return false;
    }
    unsigned char sys=0;
    if (ds.version<0x09) {
      // V E R S I O N  -> 3 <-
      // AWESOME
      ds.system=DIV_SYSTEM_YMU759;
    } else {
      sys=reader.readC();
      ds.system=systemFromFile(sys);
    }
    if (ds.system==DIV_SYSTEM_NULL) {
      logE("invalid system 0x%.2x!",sys);
      lastError="system not supported. running old version?";
      delete[] file;
      return false;
    }
    
    if (ds.system==DIV_SYSTEM_YMU759 && ds.version<0x10) { // TODO
      ds.vendor=reader.readString((unsigned char)reader.readC());
      ds.carrier=reader.readString((unsigned char)reader.readC());
      ds.category=reader.readString((unsigned char)reader.readC());
      ds.name=reader.readString((unsigned char)reader.readC());
      ds.author=reader.readString((unsigned char)reader.readC());
      ds.writer=reader.readString((unsigned char)reader.readC());
      ds.composer=reader.readString((unsigned char)reader.readC());
      ds.arranger=reader.readString((unsigned char)reader.readC());
      ds.copyright=reader.readString((unsigned char)reader.readC());
      ds.manGroup=reader.readString((unsigned char)reader.readC());
      ds.manInfo=reader.readString((unsigned char)reader.readC());
      ds.createdDate=reader.readString((unsigned char)reader.readC());
      ds.revisionDate=reader.readString((unsigned char)reader.readC());
      logI("%s by %s\n",ds.name.c_str(),ds.author.c_str());
      logI("has YMU-specific data:\n");
      logI("- carrier: %s\n",ds.carrier.c_str());
      logI("- category: %s\n",ds.category.c_str());
      logI("- vendor: %s\n",ds.vendor.c_str());
      logI("- writer: %s\n",ds.writer.c_str());
      logI("- composer: %s\n",ds.composer.c_str());
      logI("- arranger: %s\n",ds.arranger.c_str());
      logI("- copyright: %s\n",ds.copyright.c_str());
      logI("- management group: %s\n",ds.manGroup.c_str());
      logI("- management info: %s\n",ds.manInfo.c_str());
      logI("- created on: %s\n",ds.createdDate.c_str());
      logI("- revision date: %s\n",ds.revisionDate.c_str());
    } else {
      ds.name=reader.readString((unsigned char)reader.readC());
      ds.author=reader.readString((unsigned char)reader.readC());
      logI("%s by %s\n",ds.name.c_str(),ds.author.c_str());
    }

    logI("reading module data...\n");
    if (ds.version>0x0c) {
      ds.hilightA=reader.readC();
      ds.hilightB=reader.readC();
    }

    ds.timeBase=reader.readC();
    ds.speed1=reader.readC();
    if (ds.version>0x03) {
      ds.speed2=reader.readC();
      ds.pal=reader.readC();
      ds.hz=(ds.pal)?60:50;
      ds.customTempo=reader.readC();
    } else {
      ds.speed2=ds.speed1;
    }
    if (ds.version>0x0a) {
      String hz=reader.readString(3);
      if (ds.customTempo) {
        ds.hz=std::stoi(hz);
      }
    }
    // TODO
    if (ds.version>0x17) {
      ds.patLen=reader.readI();
    } else {
      ds.patLen=(unsigned char)reader.readC();
    }
    ds.ordersLen=(unsigned char)reader.readC();

    if (ds.version<20 && ds.version>3) {
      ds.arpLen=reader.readC();
    } else {
      ds.arpLen=1;
    }

    logI("reading pattern matrix (%d)...\n",ds.ordersLen);
    for (int i=0; i<getChannelCount(ds.system); i++) {
      for (int j=0; j<ds.ordersLen; j++) {
        ds.orders.ord[i][j]=reader.readC();
      }
    }

    if (ds.version>0x03) {
      ds.insLen=reader.readC();
    } else {
      ds.insLen=16;
    }
    logI("reading instruments (%d)...\n",ds.insLen);
    for (int i=0; i<ds.insLen; i++) {
      DivInstrument* ins=new DivInstrument;
      if (ds.version>0x03) {
        ins->name=reader.readString((unsigned char)reader.readC());
      }
      logD("%d name: %s\n",i,ins->name.c_str());
      if (ds.version<0x0b) {
        // instruments in ancient versions were all FM or STD.
        ins->mode=1;
      } else {
        ins->mode=reader.readC();
      }

      if (ins->mode) { // FM
        if (!isFMSystem(ds.system)) {
          logE("FM instrument in non-FM system. oopsie?\n");
          lastError="FM instrument in non-FM system";
          delete[] file;
          return false;
        }
        ins->fm.alg=reader.readC();
        if (ds.version<0x13) {
          reader.readC();
        }
        ins->fm.fb=reader.readC();
        if (ds.version<0x13) {
          reader.readC();
        }
        ins->fm.fms=reader.readC();
        if (ds.version<0x13) {
          reader.readC();
          ins->fm.ops=2+reader.readC()*2;
          if (ds.system!=DIV_SYSTEM_YMU759) ins->fm.ops=4;
        } else {
          ins->fm.ops=4;
        }
        if (ins->fm.ops!=2 && ins->fm.ops!=4) {
          logE("invalid op count %d. did we read it wrong?\n",ins->fm.ops);
          lastError="file is corrupt or unreadable at operators";
          delete[] file;
          return false;
        }
        ins->fm.ams=reader.readC();

        for (int j=0; j<ins->fm.ops; j++) {
          ins->fm.op[j].am=reader.readC();
          ins->fm.op[j].ar=reader.readC();
          if (ds.version<0x13) {
            ins->fm.op[j].dam=reader.readC();
          }
          ins->fm.op[j].dr=reader.readC();
          if (ds.version<0x13) {
            ins->fm.op[j].dvb=reader.readC();
            ins->fm.op[j].egt=reader.readC();
            ins->fm.op[j].ksl=reader.readC();
            if (ds.version<0x11) { // TODO: don't know when did this change
              ins->fm.op[j].ksr=reader.readC();
            }
          }
          ins->fm.op[j].mult=reader.readC();
          ins->fm.op[j].rr=reader.readC();
          ins->fm.op[j].sl=reader.readC();
          if (ds.version<0x13) {
            ins->fm.op[j].sus=reader.readC();
          }
          ins->fm.op[j].tl=reader.readC();
          if (ds.version<0x13) {
            ins->fm.op[j].vib=reader.readC();
            ins->fm.op[j].ws=reader.readC();
          } else {
            ins->fm.op[j].dt2=reader.readC();
          }
          if (ds.version>0x03) {
            ins->fm.op[j].rs=reader.readC();
            ins->fm.op[j].dt=reader.readC();
            ins->fm.op[j].d2r=reader.readC();
            ins->fm.op[j].ssgEnv=reader.readC();
          }

          logD("OP%d: AM %d AR %d DAM %d DR %d DVB %d EGT %d KSL %d MULT %d RR %d SL %d SUS %d TL %d VIB %d WS %d RS %d DT %d D2R %d SSG-EG %d\n",j,
               ins->fm.op[j].am,
               ins->fm.op[j].ar,
               ins->fm.op[j].dam,
               ins->fm.op[j].dr,
               ins->fm.op[j].dvb,
               ins->fm.op[j].egt,
               ins->fm.op[j].ksl,
               ins->fm.op[j].mult,
               ins->fm.op[j].rr,
               ins->fm.op[j].sl,
               ins->fm.op[j].sus,
               ins->fm.op[j].tl,
               ins->fm.op[j].vib,
               ins->fm.op[j].ws,
               ins->fm.op[j].rs,
               ins->fm.op[j].dt,
               ins->fm.op[j].d2r,
               ins->fm.op[j].ssgEnv
               );
        }
      } else { // STD
        if (ds.system!=DIV_SYSTEM_GB || ds.version<0x12) {
          ins->std.volMacroLen=reader.readC();
          for (int j=0; j<ins->std.volMacroLen; j++) {
            if (ds.version<0x0e) {
              ins->std.volMacro[j]=reader.readC();
            } else {
              ins->std.volMacro[j]=reader.readI();
            }
          }
          if (ins->std.volMacroLen>0) {
            ins->std.volMacroLoop=reader.readC();
          }
        }

        ins->std.arpMacroLen=reader.readC();
        for (int j=0; j<ins->std.arpMacroLen; j++) {
          if (ds.version<0x0e) {
            ins->std.arpMacro[j]=reader.readC();
          } else {
            ins->std.arpMacro[j]=reader.readI();
          }
        }
        if (ins->std.arpMacroLen>0) {
          ins->std.arpMacroLoop=reader.readC();
        }
        if (ds.version>0x0f) { // TODO
          ins->std.arpMacroMode=reader.readC();
        }

        ins->std.dutyMacroLen=reader.readC();
        for (int j=0; j<ins->std.dutyMacroLen; j++) {
          if (ds.version<0x0e) {
            ins->std.dutyMacro[j]=reader.readC();
          } else {
            ins->std.dutyMacro[j]=reader.readI();
          }
        }
        if (ins->std.dutyMacroLen>0) {
          ins->std.dutyMacroLoop=reader.readC();
        }

        ins->std.waveMacroLen=reader.readC();
        for (int j=0; j<ins->std.waveMacroLen; j++) {
          if (ds.version<0x0e) {
            ins->std.waveMacro[j]=reader.readC();
          } else {
            ins->std.waveMacro[j]=reader.readI();
          }
        }
        if (ins->std.waveMacroLen>0) {
          ins->std.waveMacroLoop=reader.readC();
        }

        if (ds.system==DIV_SYSTEM_C64_6581 || ds.system==DIV_SYSTEM_C64_8580) {
          ins->c64.triOn=reader.readC();
          ins->c64.sawOn=reader.readC();
          ins->c64.pulseOn=reader.readC();
          ins->c64.noiseOn=reader.readC();

          ins->c64.a=reader.readC();
          ins->c64.d=reader.readC();
          ins->c64.s=reader.readC();
          ins->c64.r=reader.readC();

          ins->c64.duty=reader.readC();

          ins->c64.ringMod=reader.readC();
          ins->c64.oscSync=reader.readC();
          ins->c64.toFilter=reader.readC();
          if (ds.version<0x11) {
            ins->c64.volIsCutoff=reader.readI();
          } else {
            ins->c64.volIsCutoff=reader.readC();
          }
          ins->c64.initFilter=reader.readC();

          ins->c64.res=reader.readC();
          ins->c64.cut=reader.readC();
          ins->c64.hp=reader.readC();
          ins->c64.bp=reader.readC();
          ins->c64.lp=reader.readC();
          ins->c64.ch3off=reader.readC();
        }

        if (ds.system==DIV_SYSTEM_GB && ds.version>0x11) {
          ins->gb.envVol=reader.readC();
          ins->gb.envDir=reader.readC();
          ins->gb.envLen=reader.readC();
          ins->gb.soundLen=reader.readC();

          logD("GB data: vol %d dir %d len %d sl %d\n",ins->gb.envVol,ins->gb.envDir,ins->gb.envLen,ins->gb.soundLen);
        }
      }

      ds.ins.push_back(ins);
    }

    if (ds.version>0x0b) {
      ds.waveLen=(unsigned char)reader.readC();
      logI("reading wavetables (%d)...\n",ds.waveLen);
      for (int i=0; i<ds.waveLen; i++) {
        DivWavetable* wave=new DivWavetable;
        wave->len=(unsigned char)reader.readI();
        if (wave->len>32) {
          logE("invalid wave length %d. are we doing something wrong?\n",wave->len);
          lastError="file is corrupt or unreadable at wavetables";
          delete[] file;
          return false;
        }
        logD("%d length %d\n",i,wave->len);
        for (int j=0; j<wave->len; j++) {
          if (ds.version<0x0e) {
            wave->data[j]=reader.readC();
          } else {
            wave->data[j]=reader.readI();
          }
        }
        ds.wave.push_back(wave);
      }
    }

    logI("reading patterns (%d channels, %d orders)...\n",getChannelCount(ds.system),ds.ordersLen);
    for (int i=0; i<getChannelCount(ds.system); i++) {
      DivChannelData& chan=ds.pat[i];
      if (ds.version<0x0a) {
        chan.effectRows=1;
      } else {
        chan.effectRows=reader.readC();
      }
      logD("%d fx rows: %d\n",i,chan.effectRows);
      if (chan.effectRows>4 || chan.effectRows<1) {
        logE("invalid effect row count %d. are you sure everything is ok?\n",chan.effectRows);
        lastError="file is corrupt or unreadable at effect rows";
        delete[] file;
        return false;
      }
      for (int j=0; j<ds.ordersLen; j++) {
        DivPattern* pat=chan.getPattern(ds.orders.ord[i][j],true);
        for (int k=0; k<ds.patLen; k++) {
          // note
          pat->data[k][0]=reader.readS();
          // octave
          pat->data[k][1]=reader.readS();
          if (ds.system==DIV_SYSTEM_SMS && ds.version<0x0e && pat->data[k][1]>0) {
            // apparently it was up one octave before
            pat->data[k][1]--;
          } else if (ds.system==DIV_SYSTEM_GENESIS && ds.version<0x0e && pat->data[k][1]>0 && i>5) {
            // ditto
            pat->data[k][1]--;
          }
          // volume
          pat->data[k][3]=reader.readS();
          if (ds.version<0x0a) {
            // back then volume was stored as 00-ff instead of 00-7f/0-f
            if (i>5) {
              pat->data[k][3]>>=4;
            } else {
              pat->data[k][3]>>=1;
            }
          }
          for (int l=0; l<chan.effectRows; l++) {
            // effect
            pat->data[k][4+(l<<1)]=reader.readS();
            pat->data[k][5+(l<<1)]=reader.readS();

            if (ds.version<0x14) {
              if (pat->data[k][4+(l<<1)]==0xe5 && pat->data[k][5+(l<<1)]!=-1) {
                pat->data[k][5+(l<<1)]=128+((pat->data[k][5+(l<<1)]-128)/4);
              }
            }
          }
          // instrument
          pat->data[k][2]=reader.readS();
        }
      }
    }

    ds.sampleLen=reader.readC();
    logI("reading samples (%d)...\n",ds.sampleLen);
    if (ds.version<0x0b && ds.sampleLen>0) { // TODO what is this for?
      reader.readC();
    }
    for (int i=0; i<ds.sampleLen; i++) {
      DivSample* sample=new DivSample;
      sample->length=reader.readI();
      if (sample->length<0) {
        logE("invalid sample length %d. are we doing something wrong?\n",sample->length);
        lastError="file is corrupt or unreadable at samples";
        delete[] file;
        return false;
      }
      if (ds.version>0x16) {
        sample->name=reader.readString((unsigned char)reader.readC());
      } else {
        sample->name="";
      }
      logD("%d name %s (%d)\n",i,sample->name.c_str(),sample->length);
      if (ds.version<0x0b) {
        sample->rate=22050;
        sample->pitch=0;
        sample->vol=0;
      } else {
        sample->rate=fileToDivRate(reader.readC());
        sample->pitch=reader.readC();
        sample->vol=reader.readC();
      }
      if (ds.version>0x15) {
        sample->depth=reader.readC();
      } else {
        sample->depth=16;
      }
      if (sample->length>0) {
        if (ds.version<0x0b) {
          sample->data=new short[1+(sample->length/2)];
          reader.read(sample->data,sample->length);
          sample->length/=2;
        } else {
          sample->data=new short[sample->length];
          reader.read(sample->data,sample->length*2);
        }
      }
      ds.sample.push_back(sample);
    }

    if (reader.tell()<reader.size()) {
      if ((reader.tell()+1)!=reader.size()) {
        logW("premature end of song (we are at %x, but size is %x)\n",reader.tell(),reader.size());
      }
    }

    if (active) quitDispatch();
    isBusy.lock();
    song.unload();
    song=ds;
    chans=getChannelCount(song.system);
    renderSamples();
    isBusy.unlock();
    if (active) {
      initDispatch();
      syncReset();
    }
  } catch (EndOfFileException e) {
    logE("premature end of file!\n");
    lastError="incomplete file";
    delete[] file;
    return false;
  }
  delete[] file;
  return true;
}

SafeWriter* DivEngine::save() {
  SafeWriter* w=new SafeWriter;
  w->init();
  // write magic
  w->write(DIV_DMF_MAGIC,16);
  // version
  w->writeC(24);
  w->writeC(systemToFile(song.system));

  // song info
  w->writeString(song.name,true);
  w->writeString(song.author,true);
  w->writeC(song.hilightA);
  w->writeC(song.hilightB);
  
  w->writeC(song.timeBase);
  w->writeC(song.speed1);
  w->writeC(song.speed2);
  w->writeC(song.pal);
  w->writeC(song.customTempo);
  char customHz[4];
  memset(customHz,0,4);
  snprintf(customHz,4,"%d",song.hz);
  w->write(customHz,3);
  w->writeI(song.patLen);
  w->writeC(song.ordersLen);

  for (int i=0; i<getChannelCount(song.system); i++) {
    for (int j=0; j<song.ordersLen; j++) {
      w->writeC(song.orders.ord[i][j]);
    }
  }

  w->writeC(song.ins.size());
  for (DivInstrument* i: song.ins) {
    w->writeString(i->name,true);
    w->writeC(i->mode);
    if (i->mode) { // FM
      w->writeC(i->fm.alg);
      w->writeC(i->fm.fb);
      w->writeC(i->fm.fms);
      w->writeC(i->fm.ams);

      for (int j=0; j<4; j++) {
        DivInstrumentFM::Operator& op=i->fm.op[j];
        w->writeC(op.am);
        w->writeC(op.ar);
        w->writeC(op.dr);
        w->writeC(op.mult);
        w->writeC(op.rr);
        w->writeC(op.sl);
        w->writeC(op.tl);
        w->writeC(op.dt2);
        w->writeC(op.rs);
        w->writeC(op.dt);
        w->writeC(op.d2r);
        w->writeC(op.ssgEnv);
      }
    } else { // STD
      if (song.system!=DIV_SYSTEM_GB) {
        w->writeC(i->std.volMacroLen);
        w->write(i->std.volMacro,4*i->std.volMacroLen);
        if (i->std.volMacroLen>0) {
          w->writeC(i->std.volMacroLoop);
        }
      }

      w->writeC(i->std.arpMacroLen);
      w->write(i->std.arpMacro,4*i->std.arpMacroLen);
      if (i->std.arpMacroLen>0) {
        w->writeC(i->std.arpMacroLoop);
      }
      w->writeC(i->std.arpMacroMode);

      w->writeC(i->std.dutyMacroLen);
      w->write(i->std.dutyMacro,4*i->std.dutyMacroLen);
      if (i->std.dutyMacroLen>0) {
        w->writeC(i->std.dutyMacroLoop);
      }

      w->writeC(i->std.waveMacroLen);
      w->write(i->std.waveMacro,4*i->std.waveMacroLen);
      if (i->std.waveMacroLen>0) {
        w->writeC(i->std.waveMacroLoop);
      }

      if (song.system==DIV_SYSTEM_C64_6581 || song.system==DIV_SYSTEM_C64_8580) {
        w->writeC(i->c64.triOn);
        w->writeC(i->c64.sawOn);
        w->writeC(i->c64.pulseOn);
        w->writeC(i->c64.noiseOn);

        w->writeC(i->c64.a);
        w->writeC(i->c64.d);
        w->writeC(i->c64.s);
        w->writeC(i->c64.r);

        w->writeC(i->c64.duty);

        w->writeC(i->c64.ringMod);
        w->writeC(i->c64.oscSync);

        w->writeC(i->c64.toFilter);
        w->writeC(i->c64.volIsCutoff);
        w->writeC(i->c64.initFilter);

        w->writeC(i->c64.res);
        w->writeC(i->c64.cut);
        w->writeC(i->c64.hp);
        w->writeC(i->c64.bp);
        w->writeC(i->c64.lp);
        w->writeC(i->c64.ch3off);
      }

      if (song.system==DIV_SYSTEM_GB) {
        w->writeC(i->gb.envVol);
        w->writeC(i->gb.envDir);
        w->writeC(i->gb.envLen);
        w->writeC(i->gb.soundLen);
      }
    }
  }

  w->writeC(song.wave.size());
  for (DivWavetable* i: song.wave) {
    w->writeI(i->len);
    w->write(i->data,4*i->len);
  }

  for (int i=0; i<getChannelCount(song.system); i++) {
    w->writeC(song.pat[i].effectRows);

    for (int j=0; j<song.ordersLen; j++) {
      DivPattern* pat=song.pat[i].getPattern(song.orders.ord[i][j],false);
      for (int k=0; k<song.patLen; k++) {
        w->writeS(pat->data[k][0]); // note
        w->writeS(pat->data[k][1]); // octave
        w->writeS(pat->data[k][3]); // volume
        w->write(&pat->data[k][4],2*song.pat[i].effectRows*2); // effects
        w->writeS(pat->data[k][2]); // instrument
      }
    }
  }

  w->writeC(song.sample.size());
  for (DivSample* i: song.sample) {
    w->writeI(i->length);
    w->writeString(i->name,true);
    w->writeC(divToFileRate(i->rate));
    w->writeC(i->pitch);
    w->writeC(i->vol);
    w->writeC(i->depth);
    w->write(i->data,2*i->length);
  }

  return w;
}

#ifdef _WIN32
#define CONFIG_FILE "\\furnace.cfg"
#else
#define CONFIG_FILE "/furnace.cfg"
#endif

bool DivEngine::saveConf() {
  configFile=configPath+String(CONFIG_FILE);
  FILE* f=fopen(configFile.c_str(),"wb");
  if (f==NULL) {
    logW("could not write config file! %s\n",strerror(errno));
    return false;
  }
  for (auto& i: conf) {
    String toWrite=fmt::sprintf("%s=%s\n",i.first,i.second);
    if (fwrite(toWrite.c_str(),1,toWrite.size(),f)!=toWrite.size()) {
      logW("could not write config file! %s\n",strerror(errno));
      fclose(f);
      return false;
    }
  }
  fclose(f);
  return true;
}

bool DivEngine::loadConf() {
  char line[4096];
  configFile=configPath+String(CONFIG_FILE);
  FILE* f=fopen(configFile.c_str(),"rb");
  if (f==NULL) {
    logI("creating default config.\n");
    return saveConf();
  }
  logI("loading config.\n");
  while (!feof(f)) {
    String key="";
    String value="";
    bool keyOrValue=false;
    if (fgets(line,4095,f)==NULL) {
      break;
    }
    for (char* i=line; *i; i++) {
      if (*i=='\n') continue;
      if (keyOrValue) {
        value+=*i;
      } else {
        if (*i=='=') {
          keyOrValue=true;
        } else {
          key+=*i;
        }
      }
    }
    if (keyOrValue) {
      conf[key]=value;
    }
  }
  fclose(f);
  return true;
}

bool DivEngine::getConfBool(String key, bool fallback) {
  try {
    String val=conf.at(key);
    if (val=="true") {
      return true;
    } else if (val=="false") {
      return false;
    }
  } catch (std::out_of_range& e) {
  }
  return fallback;
}

int DivEngine::getConfInt(String key, int fallback) {
  try {
    String val=conf.at(key);
    int ret=std::stoi(val);
    return ret;
  } catch (std::out_of_range& e) {
  } catch (std::invalid_argument& e) {
  }
  return fallback;
}

float DivEngine::getConfFloat(String key, float fallback) {
  try {
    String val=conf.at(key);
    float ret=std::stof(val);
    return ret;
  } catch (std::out_of_range& e) {
  } catch (std::invalid_argument& e) {
  }
  return fallback;
}

double DivEngine::getConfDouble(String key, double fallback) {
  try {
    String val=conf.at(key);
    double ret=std::stod(val);
    return ret;
  } catch (std::out_of_range& e) {
  } catch (std::invalid_argument& e) {
  }
  return fallback;
}

String DivEngine::getConfString(String key, String fallback) {
  try {
    String val=conf.at(key);
    return val;
  } catch (std::out_of_range& e) {
  }
  return fallback;
}

void DivEngine::setConf(String key, bool value) {
  if (value) {
    conf[key]="true";
  } else {
    conf[key]="false";
  }
}

void DivEngine::setConf(String key, int value) {
  conf[key]=fmt::sprintf("%d",value);
}

void DivEngine::setConf(String key, float value) {
  conf[key]=fmt::sprintf("%f",value);
}

void DivEngine::setConf(String key, double value) {
  conf[key]=fmt::sprintf("%f",value);
}

void DivEngine::setConf(String key, String value) {
  conf[key]=value;
}

// ADPCM code attribution: https://wiki.neogeodev.org/index.php?title=ADPCM_codecs

static short adSteps[49]={ 
	16, 17, 19, 21, 23, 25, 28, 31, 34, 37,
	41, 45, 50, 55, 60, 66, 73, 80, 88, 97,
	107, 118, 130, 143, 157, 173, 190, 209, 230, 253,
	279, 307, 337, 371, 408, 449, 494, 544, 598, 658,
	724, 796, 876, 963, 1060, 1166, 1282, 1411, 1552
};

static int adStepSeek[16]={
  -1, -1, -1, -1, 2, 5, 7, 9, -1, -1, -1, -1, 2, 5, 7, 9
};

static double samplePitches[11]={
  0.1666666666, 0.2, 0.25, 0.333333333, 0.5,
  1,
  2, 3, 4, 5, 6
};

void DivEngine::renderSamplesP() {
  isBusy.lock();
  renderSamples();
  isBusy.unlock();
}

void DivEngine::renderSamples() {
  sPreview.sample=-1;
  sPreview.pos=0;
  if (jediTable==NULL) {
    jediTable=new int[16*49];
    for (int step=0; step<49; step++) {
      for (int nib=0; nib<16; nib++) {
        int value=(2*(nib&0x07)+1)*adSteps[step]/8;
        jediTable[step*16+nib]=((nib&0x08)!=0)?-value:value;
      }
    }
  }

  for (int i=0; i<song.sampleLen; i++) {
    DivSample* s=song.sample[i];
    if (s->rendLength!=0) {
      delete[] s->rendData;
      delete[] s->adpcmRendData;
    }
    s->rendLength=(double)s->length/samplePitches[s->pitch];
    if (s->rendLength==0) {
      s->adpcmRendLength=0;
      continue;
    }
    s->rendData=new short[s->rendLength];
    size_t adpcmLen=((s->rendLength>>1)+255)&0xffffff00;
    s->adpcmRendLength=adpcmLen;
    s->adpcmRendData=new unsigned char[adpcmLen];
    memset(s->adpcmRendData,0,adpcmLen);

    // step 1: render to PCM
    unsigned int k=0;
    float mult=(float)(s->vol)/50.0f;
    for (double j=0; j<s->length; j+=samplePitches[s->pitch]) {
      if (k>=s->rendLength) {
        break;
      }
      if (s->depth==8) {
        float next=(float)(s->data[(unsigned int)j]-0x80)*mult;
        s->rendData[k++]=fmin(fmax(next,-128),127);
      } else {
        float next=(float)s->data[(unsigned int)j]*mult;
        s->rendData[k++]=fmin(fmax(next,-32768),32767);
      }
    }

    // step 2: render to ADPCM
    int acc=0;
    int decstep=0;
    int diff=0;
    int step=0;
    int predsample=0;
    int index=0;
    int prevsample=0;
    int previndex=0;
    for (unsigned int j=0; j<s->adpcmRendLength*2; j++) {
      unsigned char encoded=0;
      int tempstep=0;

      predsample=prevsample;
      index=previndex;
      step=adSteps[index];

      short sample=(j<s->rendLength)?((s->depth==16)?(s->rendData[j]>>4):(s->rendData[j]<<4)):0;
      if (sample>0x7d0) sample=0x7d0;
      if (sample<-0x7d0) sample=-0x7d0;
      diff=sample-predsample;
      if (diff>=0) {
        encoded=0;
      } else {
        encoded=8;
        diff=-diff;
      }

      tempstep=step;
      if (diff>=tempstep) {
        encoded|=4;
        diff-=tempstep;
      }
      tempstep>>=1;
      if (diff>=tempstep) {
        encoded|=2;
        diff-=tempstep;
      }
      tempstep>>=1;
      if (diff>=tempstep) encoded|=1;

      acc+=jediTable[decstep+encoded];
      if (acc>0x7ff || acc<-0x800) {
        logW("clipping! %d\n",acc);
      }
      acc&=0xfff;
      if (acc&0x800) acc|=~0xfff;
      decstep+=adStepSeek[encoded&7]*16;
      if (decstep<0) decstep=0;
      if (decstep>48*16) decstep=48*16;
      predsample=(short)acc;

      index+=adStepSeek[encoded];
      if (index<0) index=0;
      if (index>48) index=48;

      prevsample=predsample;
      previndex=index;

      if (j&1) {
        s->adpcmRendData[j>>1]|=encoded;
      } else {
        s->adpcmRendData[j>>1]=encoded<<4;
      }
    }
  }

  // step 3: allocate the samples if needed
  if (song.system==DIV_SYSTEM_YM2610 || song.system==DIV_SYSTEM_YM2610_EXT) {
    if (adpcmMem==NULL) adpcmMem=new unsigned char[16777216];

    size_t memPos=0;
    for (int i=0; i<song.sampleLen; i++) {
      DivSample* s=song.sample[i];
      if ((memPos&0xf00000)!=((memPos+s->adpcmRendLength)&0xf00000)) {
        memPos=(memPos+0xfffff)&0xf00000;
      }
      memcpy(adpcmMem+memPos,s->adpcmRendData,s->adpcmRendLength);
      s->rendOff=memPos;
      memPos+=s->adpcmRendLength;
    }
  }
}

void DivEngine::createNew() {
  DivSystem sys=song.system;
  quitDispatch();
  isBusy.lock();
  song.unload();
  song=DivSong();
  song.system=sys;
  chans=getChannelCount(song.system);
  renderSamples();
  isBusy.unlock();
  initDispatch();
  isBusy.lock();
  reset();
  isBusy.unlock();
}

void DivEngine::changeSystem(DivSystem which) {
  quitDispatch();
  isBusy.lock();
  song.system=which;
  chans=getChannelCount(song.system);
  // instrument safety check
  for (DivInstrument* i: song.ins) {
    if (!isFMSystem(song.system) && i->mode) {
      i->mode=false;
    }
    if (!isSTDSystem(song.system) && !i->mode) {
      i->mode=true;
    }
  }
  isBusy.unlock();
  initDispatch();
  isBusy.lock();
  renderSamples();
  reset();
  isBusy.unlock();
}

String DivEngine::getLastError() {
  return lastError;
}

DivInstrument* DivEngine::getIns(int index) {
  if (index<0 || index>=song.insLen) return &song.nullIns;
  return song.ins[index];
}

DivWavetable* DivEngine::getWave(int index) {
  if (index<0 || index>=song.waveLen) {
    if (song.waveLen>0) {
      return song.wave[0];
    } else {
      return &song.nullWave;
    }
  }
  return song.wave[index];
}

void DivEngine::setLoops(int loops) {
  remainingLoops=loops;
}

void DivEngine::playSub(bool preserveDrift) {
  reset();
  if (preserveDrift && curOrder==0) return;
  bool oldRepeatPattern=repeatPattern;
  repeatPattern=false;
  int goal=curOrder;
  curOrder=0;
  curRow=0;
  int prevDrift=clockDrift;
  clockDrift=0;
  cycles=0;
  if (preserveDrift) {
    endOfSong=false;
  } else {
    ticks=1;
    totalTicks=0;
  }
  speedAB=false;
  playing=true;
  dispatch->setSkipRegisterWrites(true);
  while (curOrder<goal) {
    if (nextTick(preserveDrift)) break;
  }
  dispatch->setSkipRegisterWrites(false);
  if (goal>0) {
    dispatch->forceIns();
  }
  repeatPattern=oldRepeatPattern;
  if (preserveDrift) {
    clockDrift=prevDrift;
  } else {
    clockDrift=0;
  }
  if (!preserveDrift) {
    cycles=0;
    ticks=1;
  }
}

int DivEngine::calcFreq(int base, int pitch, bool period) {
  return period?
          int(base*pow(2,-(double)pitch/(12.0*128.0))/(98.0+globalPitch*6.0)*98.0):
          (int(base*pow(2,(double)pitch/(12.0*128.0))*(98+globalPitch*6))/98);
}

void DivEngine::play() {
  isBusy.lock();
  freelance=false;
  playSub(false);
  isBusy.unlock();
}

void DivEngine::stop() {
  isBusy.lock();
  freelance=false;
  playing=false;
  extValuePresent=false;
  isBusy.unlock();
}

void DivEngine::reset() {
  for (int i=0; i<17; i++) {
    chan[i]=DivChannelState();
    chan[i].volMax=(dispatch->dispatch(DivCommand(DIV_CMD_GET_VOLMAX,i))<<8)|0xff;
    chan[i].volume=chan[i].volMax;
  }
  extValue=0;
  extValuePresent=0;
  speed1=song.speed1;
  speed2=song.speed2;
  globalPitch=0;
  dispatch->reset();
}

void DivEngine::syncReset() {
  isBusy.lock();
  reset();
  isBusy.unlock();
}

const int sampleRates[6]={
  4000, 8000, 11025, 16000, 22050, 32000
};

int DivEngine::fileToDivRate(int frate) {
  if (frate<0) frate=0;
  if (frate>5) frate=5;
  return sampleRates[frate];
}

int DivEngine::divToFileRate(int drate) {
  if (drate>26000) {
    return 5;
  } else if (drate>18000) {
    return 4;
  } else if (drate>14000) {
    return 3;
  } else if (drate>9500) {
    return 2;
  } else if (drate>6000) {
    return 1;
  } else {
    return 0;
  }
  return 4;
}

int DivEngine::getEffectiveSampleRate(int rate) {
  if (rate<1) return 0;
  switch (song.system) {
    case DIV_SYSTEM_YMU759:
      return 8000;
    case DIV_SYSTEM_GENESIS: case DIV_SYSTEM_GENESIS_EXT:
      return 1278409/(1280000/rate);
    case DIV_SYSTEM_PCE:
      return 1789773/(1789773/rate);
    case DIV_SYSTEM_ARCADE:
      return (31250*MIN(255,(rate*255/31250)))/255;
    case DIV_SYSTEM_YM2610: case DIV_SYSTEM_YM2610_EXT:
      return 18518;
    default:
      break;
  }
  return rate;
}

void DivEngine::previewSample(int sample) {
  isBusy.lock();
  if (sample<0 || sample>=(int)song.sample.size()) {
    sPreview.sample=-1;
    sPreview.pos=0;
    isBusy.unlock();
    return;
  }
  blip_clear(bb[2]);
  blip_set_rates(bb[2],song.sample[sample]->rate,got.rate);
  prevSample[2]=0;
  sPreview.pos=0;
  sPreview.sample=sample;
  isBusy.unlock();
}

String DivEngine::getConfigPath() {
  return configPath;
}

int DivEngine::getMaxVolumeChan(int ch) {
  return chan[ch].volMax>>8;
}

unsigned char DivEngine::getOrder() {
  return curOrder;
}

int DivEngine::getRow() {
  return curRow;
}

unsigned char DivEngine::getSpeed1() {
  return speed1;
}

unsigned char DivEngine::getSpeed2() {
  return speed2;
}

int DivEngine::getHz() {
  if (song.customTempo) {
    return song.hz;
  } else if (song.pal) {
    return 60;
  } else {
    return 50;
  }
  return 60;
}

int DivEngine::getTotalTicks() {
  return totalTicks;
}

bool DivEngine::getRepeatPattern() {
  return repeatPattern;
}

void DivEngine::setRepeatPattern(bool value) {
  isBusy.lock();
  repeatPattern=value;
  isBusy.unlock();
}

bool DivEngine::hasExtValue() {
  return extValuePresent;
}

unsigned char DivEngine::getExtValue() {
  return extValue;
}

bool DivEngine::isPlaying() {
  return (playing && !freelance);
}

bool DivEngine::isChannelMuted(int chan) {
  return isMuted[chan];
}

void DivEngine::toggleMute(int chan) {
  muteChannel(chan,!isMuted[chan]);
}

void DivEngine::toggleSolo(int chan) {
  bool solo=false;
  for (int i=0; i<chans; i++) {
    if (i==chan) {
      solo=true;
      continue;
    } else {
      if (!isMuted[i]) {
        solo=false;
        break;
      }
    }
  }
  isBusy.lock();
  if (!solo) {
    for (int i=0; i<chans; i++) {
      isMuted[i]=(i!=chan);
      if (dispatch!=NULL) {
        dispatch->muteChannel(i,isMuted[i]);
      }
    }
  } else {
    for (int i=0; i<chans; i++) {
      isMuted[i]=false;
      if (dispatch!=NULL) {
        dispatch->muteChannel(i,isMuted[i]);
      }
    }
  }
  isBusy.unlock();
}

void DivEngine::muteChannel(int chan, bool mute) {
  isBusy.lock();
  isMuted[chan]=mute;
  if (dispatch!=NULL) {
    dispatch->muteChannel(chan,isMuted[chan]);
  }
  isBusy.unlock();
}

int DivEngine::addInstrument() {
  isBusy.lock();
  DivInstrument* ins=new DivInstrument;
  int insCount=(int)song.ins.size();
  ins->name=fmt::sprintf("Instrument %d",insCount);
  ins->mode=isFMSystem(song.system);
  song.ins.push_back(ins);
  song.insLen=insCount+1;
  isBusy.unlock();
  return insCount;
}

void DivEngine::delInstrument(int index) {
  isBusy.lock();
  if (index>=0 && index<(int)song.ins.size()) {
    delete song.ins[index];
    song.ins.erase(song.ins.begin()+index);
    song.insLen=song.ins.size();
  }
  isBusy.unlock();
}

int DivEngine::addWave() {
  isBusy.lock();
  DivWavetable* wave=new DivWavetable;
  int waveCount=(int)song.wave.size();
  song.wave.push_back(wave);
  song.waveLen=waveCount+1;
  isBusy.unlock();
  return waveCount;
}

void DivEngine::delWave(int index) {
  isBusy.lock();
  if (index>=0 && index<(int)song.wave.size()) {
    delete song.wave[index];
    song.wave.erase(song.wave.begin()+index);
    song.waveLen=song.wave.size();
  }
  isBusy.unlock();
}

int DivEngine::addSample() {
  isBusy.lock();
  DivSample* sample=new DivSample;
  int sampleCount=(int)song.sample.size();
  sample->name=fmt::sprintf("Sample %d",sampleCount);
  song.sample.push_back(sample);
  song.sampleLen=sampleCount+1;
  renderSamples();
  isBusy.unlock();
  return sampleCount;
}

bool DivEngine::addSampleFromFile(const char* path) {
  isBusy.lock();
  SF_INFO si;
  SNDFILE* f=sf_open(path,SFM_READ,&si);
  if (f==NULL) {
    isBusy.unlock();
    return false;
  }
  if (si.frames>1000000) {
    sf_close(f);
    isBusy.unlock();
    return false;
  }
  short* buf=new short[si.channels*si.frames];
  if (sf_readf_short(f,buf,si.frames)!=si.frames) {
    logW("sample read size mismatch!\n");
  }
  sf_close(f);
  DivSample* sample=new DivSample;
  int sampleCount=(int)song.sample.size();
  const char* sName=strrchr(path,'/');
  if (sName==NULL) {
    sName=path;
  } else {
    sName++;
  }
  sample->name=sName;

  int index=0;
  sample->length=si.frames;
  sample->data=new short[si.frames];
  sample->depth=16;
  sample->vol=50;
  sample->pitch=5;
  for (int i=0; i<si.frames*si.channels; i+=si.channels) {
    int averaged=0;
    for (int j=0; j<si.channels; j++) {
      averaged+=buf[i+j];
    }
    averaged/=si.channels;
    sample->data[index++]=averaged^(((si.format&SF_FORMAT_SUBMASK)==SF_FORMAT_PCM_U8)?0x8000:0);
  }
  delete[] buf;
  sample->rate=si.samplerate;
  if (sample->rate<4000) sample->rate=4000;
  if (sample->rate>32000) sample->rate=32000;

  song.sample.push_back(sample);
  song.sampleLen=sampleCount+1;
  renderSamples();
  isBusy.unlock();
  return sampleCount;
}

void DivEngine::delSample(int index) {
  isBusy.lock();
  if (index>=0 && index<(int)song.sample.size()) {
    delete song.sample[index];
    song.sample.erase(song.sample.begin()+index);
    song.sampleLen=song.sample.size();
    renderSamples();
  }
  isBusy.unlock();
}

void DivEngine::addOrder(bool duplicate, bool where) {
  unsigned char order[32];
  if (song.ordersLen>=0x7e) return;
  isBusy.lock();
  if (duplicate) {
    for (int i=0; i<32; i++) {
      order[i]=song.orders.ord[i][curOrder];
    }
  } else {
    bool used[256];
    for (int i=0; i<chans; i++) {
      memset(used,0,sizeof(bool)*256);
      for (int j=0; j<song.ordersLen; j++) {
        used[song.orders.ord[i][j]]=true;
      }
      order[i]=0x7e;
      for (int j=0; j<256; j++) {
        if (!used[j]) {
          order[i]=j;
          break;
        }
      }
    }
  }
  if (where) { // at the end
    for (int i=0; i<32; i++) {
      song.orders.ord[i][song.ordersLen]=order[i];
    }
    song.ordersLen++;
  } else { // after current order
    for (int i=0; i<32; i++) {
      for (int j=song.ordersLen; j>curOrder; j--) {
        song.orders.ord[i][j]=song.orders.ord[i][j-1];
      }
      song.orders.ord[i][curOrder+1]=order[i];
    }
    song.ordersLen++;
    curOrder++;
    if (playing && !freelance) {
      playSub(false);
    }
  }
  isBusy.unlock();
}

void DivEngine::deleteOrder() {
  if (song.ordersLen<=1) return;
  isBusy.lock();
  for (int i=0; i<32; i++) {
    for (int j=curOrder; j<song.ordersLen; j++) {
      song.orders.ord[i][j]=song.orders.ord[i][j+1];
    }
  }
  song.ordersLen--;
  if (curOrder>=song.ordersLen) curOrder=song.ordersLen-1;
  if (playing && !freelance) {
    playSub(false);
  }
  isBusy.unlock();
}

void DivEngine::moveOrderUp() {
  isBusy.lock();
  if (curOrder<1) {
    isBusy.unlock();
    return;
  }
  for (int i=0; i<32; i++) {
    song.orders.ord[i][curOrder]^=song.orders.ord[i][curOrder-1];
    song.orders.ord[i][curOrder-1]^=song.orders.ord[i][curOrder];
    song.orders.ord[i][curOrder]^=song.orders.ord[i][curOrder-1];
  }
  curOrder--;
  if (playing && !freelance) {
    playSub(false);
  }
  isBusy.unlock();
}

void DivEngine::moveOrderDown() {
  isBusy.lock();
  if (curOrder>=song.ordersLen-1) {
    isBusy.unlock();
    return;
  }
  for (int i=0; i<32; i++) {
    song.orders.ord[i][curOrder]^=song.orders.ord[i][curOrder+1];
    song.orders.ord[i][curOrder+1]^=song.orders.ord[i][curOrder];
    song.orders.ord[i][curOrder]^=song.orders.ord[i][curOrder+1];
  }
  curOrder++;
  if (playing && !freelance) {
    playSub(false);
  }
  isBusy.unlock();
}

void DivEngine::noteOn(int chan, int ins, int note, int vol) {
  isBusy.lock();
  pendingNotes.push(DivNoteEvent(chan,ins,note,vol,true));
  if (!playing) {
    reset();
    freelance=true;
    playing=true;
  }
  isBusy.unlock();
}

void DivEngine::noteOff(int chan) {
  isBusy.lock();
  pendingNotes.push(DivNoteEvent(chan,-1,-1,-1,false));
  if (!playing) {
    reset();
    freelance=true;
    playing=true;
  }
  isBusy.unlock();
}

void DivEngine::setOrder(unsigned char order) {
  isBusy.lock();
  curOrder=order;
  if (order>=song.ordersLen) curOrder=0;
  if (playing && !freelance) {
    playSub(false);
  }
  isBusy.unlock();
}

void DivEngine::setSongRate(int hz, bool pal) {
  isBusy.lock();
  song.pal=!pal;
  song.hz=hz;
  song.customTempo=(song.hz!=50 && song.hz!=60);
  dispatch->setPAL((!song.pal) || (song.customTempo!=0 && song.hz<53));
  blip_set_rates(bb[0],dispatch->rate,got.rate);
  blip_set_rates(bb[1],dispatch->rate,got.rate);
  isBusy.unlock();
}

void DivEngine::setAudio(DivAudioEngines which) {
  audioEngine=which;
}

void DivEngine::setView(DivStatusView which) {
  view=which;
}

void DivEngine::setConsoleMode(bool enable) {
  consoleMode=enable;
}

void DivEngine::initDispatch() {
  if (dispatch!=NULL) return;
  isBusy.lock();
  switch (song.system) {
    case DIV_SYSTEM_GENESIS:
      dispatch=new DivPlatformGenesis;
      break;
    case DIV_SYSTEM_GENESIS_EXT:
      dispatch=new DivPlatformGenesisExt;
      break;
    case DIV_SYSTEM_SMS:
      dispatch=new DivPlatformSMS;
      break;
    case DIV_SYSTEM_GB:
      dispatch=new DivPlatformGB;
      break;
    case DIV_SYSTEM_PCE:
      dispatch=new DivPlatformPCE;
      break;
    case DIV_SYSTEM_NES:
      dispatch=new DivPlatformNES;
      break;
    case DIV_SYSTEM_C64_6581:
      dispatch=new DivPlatformC64;
      ((DivPlatformC64*)dispatch)->setChipModel(true);
      break;
    case DIV_SYSTEM_C64_8580:
      dispatch=new DivPlatformC64;
      ((DivPlatformC64*)dispatch)->setChipModel(false);
      break;
    case DIV_SYSTEM_ARCADE:
      dispatch=new DivPlatformArcade;
      ((DivPlatformArcade*)dispatch)->setYMFM(true);
      break;
    case DIV_SYSTEM_YM2610:
      dispatch=new DivPlatformYM2610;
      break;
    case DIV_SYSTEM_YM2610_EXT:
      dispatch=new DivPlatformYM2610Ext;
      break;
    default:
      logW("this system is not supported yet! using dummy platform.\n");
      dispatch=new DivPlatformDummy;
      break;
  }
  dispatch->init(this,getChannelCount(song.system),got.rate,(!song.pal) || (song.customTempo!=0 && song.hz<53));
  chans=getChannelCount(song.system);

  blip_set_rates(bb[0],dispatch->rate,got.rate);
  blip_set_rates(bb[1],dispatch->rate,got.rate);
  isBusy.unlock();
}

void DivEngine::quitDispatch() {
  if (dispatch==NULL) return;
  isBusy.lock();
  dispatch->quit();
  delete dispatch;
  dispatch=NULL;
  chans=0;
  playing=false;
  speedAB=false;
  endOfSong=false;
  ticks=0;
  cycles=0;
  curRow=0;
  curOrder=0;
  nextSpeed=3;
  clockDrift=0;
  changeOrd=-1;
  changePos=0;
  totalTicks=0;
  totalCmds=0;
  lastCmds=0;
  cmdsPerSecond=0;
  for (int i=0; i<17; i++) {
    isMuted[i]=0;
  }
  isBusy.unlock();
}

#define CHECK_CONFIG_DIR() \
  configPath+="/.config"; \
  if (stat(configPath.c_str(),&st)<0) { \
    logI("creating user config dir...\n"); \
    if (mkdir(configPath.c_str(),0755)<0) { \
      logW("could not make user config dir! (%s)\n",strerror(errno)); \
      configPath="."; \
    } \
  } \
  if (configPath!=".") { \
    configPath+="/furnace"; \
    if (stat(configPath.c_str(),&st)<0) { \
      logI("creating config dir...\n"); \
      if (mkdir(configPath.c_str(),0755)<0) { \
        logW("could not make config dir! (%s)\n",strerror(errno)); \
        configPath="."; \
      } \
    } \
  }

#ifdef _WIN32
#include "winStuff.h"
#endif

bool DivEngine::init(String outName) {
  // init config
#ifdef _WIN32
  configPath=getWinConfigPath();
#else
  struct stat st;
  char* home=getenv("HOME");
  if (home==NULL) {
    int uid=getuid();
    struct passwd* entry=getpwuid(uid);
    if (entry==NULL) {
      logW("unable to determine config directory! (%s)\n",strerror(errno));
      configPath=".";
    } else {
      configPath=entry->pw_dir;
      CHECK_CONFIG_DIR();
    }
  } else {
    configPath=home;
    CHECK_CONFIG_DIR();
  }
#endif
  logD("config path: %s\n",configPath.c_str());

  loadConf();

  // load values
  if (getConfString("audioEngine","SDL")=="JACK") {
    audioEngine=DIV_AUDIO_JACK;
  } else {
    audioEngine=DIV_AUDIO_SDL;
  }

  // init the rest of engine
  SNDFILE* outFile=NULL;
  SF_INFO outInfo;
  if (outName!="") {
    // init out file
    got.bufsize=2048;
    got.rate=44100;

    outInfo.samplerate=got.rate;
    outInfo.channels=2;
    outInfo.format=SF_FORMAT_WAV|SF_FORMAT_PCM_16;

    outFile=sf_open(outName.c_str(),SFM_WRITE,&outInfo);
    if (outFile==NULL) {
      logE("could not open file for writing!\n");
      return false;
    }
  } else {
    switch (audioEngine) {
      case DIV_AUDIO_JACK:
  #ifndef HAVE_JACK
        logE("Furnace was not compiled with JACK support!\n");
        setConf("audioEngine","SDL");
        saveConf();
        return false;
  #else
        output=new TAAudioJACK;
  #endif
        break;
      case DIV_AUDIO_SDL:
        output=new TAAudioSDL;
        break;
      default:
        logE("invalid audio engine!\n");
        return false;
    }
    want.bufsize=getConfInt("audioBufSize",1024);
    want.rate=getConfInt("audioRate",44100);
    want.fragments=2;
    want.inChans=0;
    want.outChans=2;
    want.outFormat=TA_AUDIO_FORMAT_F32;
    want.name="Furnace";
  
    output->setCallback(process,this);
  
    logI("initializing audio.\n");
    if (!output->init(want,got)) {
      logE("error while initializing audio!\n");
      return false;
    }
  }

  bb[0]=blip_new(32768);
  if (bb[0]==NULL) {
    logE("not enough memory!\n");
    return false;
  }

  bb[1]=blip_new(32768);
  if (bb[1]==NULL) {
    logE("not enough memory!\n");
    return false;
  }

  bb[2]=blip_new(32768);
  if (bb[2]==NULL) {
    logE("not enough memory!\n");
    return false;
  }
  
  bbOut[0]=new short[got.bufsize];
  bbOut[1]=new short[got.bufsize];
  bbOut[2]=new short[got.bufsize];

  bbIn[0]=new short[32768];
  bbIn[1]=new short[32768];
  bbIn[2]=new short[32768];
  bbInLen=32768;
  
  blip_set_rates(bb[2],44100,got.rate);

  for (int i=0; i<64; i++) {
    vibTable[i]=127*sin(((double)i/64.0)*(2*M_PI));
  }

  for (int i=0; i<17; i++) {
    isMuted[i]=0;
  }

  initDispatch();
  reset();
  active=true;

  if (outName!="") {
    short* ilBuffer=new short[got.bufsize*2];
    // render to file
    remainingLoops=1;
    play();
    while (remainingLoops) {
      nextBuf(NULL,NULL,0,2,got.bufsize);

      if (dispatch->isStereo()) {
        for (size_t i=0; i<got.bufsize; i++) {
          ilBuffer[i<<1]=bbOut[0][i];
          ilBuffer[1+(i<<1)]=bbOut[1][i];
        }
      } else {
        for (size_t i=0; i<got.bufsize; i++) {
          ilBuffer[i<<1]=bbOut[0][i];
          ilBuffer[1+(i<<1)]=bbOut[0][i];
        }
      }

      if (!remainingLoops) {
        sf_writef_short(outFile,ilBuffer,totalProcessed);
      } else {
        sf_writef_short(outFile,ilBuffer,got.bufsize);
      }
    }
    delete[] ilBuffer;
    sf_close(outFile);
    return true;
  } else {
    if (!output->setRun(true)) {
      logE("error while activating!\n");
      return false;
    }
  }
  return true;
}

bool DivEngine::quit() {
  output->quit();
  quitDispatch();
  logI("saving config.\n");
  saveConf();
  active=false;
  return true;
}
