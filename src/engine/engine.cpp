#include "dataErrors.h"
#include "song.h"
#include <cstdint>
#define _USE_MATH_DEFINES
#include "engine.h"
#include "instrument.h"
#include "safeReader.h"
#include "../ta-log.h"
#include "../fileutils.h"
#include "../utfutils.h"
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
#include <math.h>
#include <zlib.h>
#include <sndfile.h>
#include <fmt/printf.h>

constexpr int MASTER_CLOCK_PREC=(sizeof(void*)==8)?8:0;

void process(void* u, float** in, float** out, int inChans, int outChans, unsigned int size) {
  ((DivEngine*)u)->nextBuf(in,out,inChans,outChans,size);
}

#define DIV_READ_SIZE 131072
#define DIV_DMF_MAGIC ".DelekDefleMask."
#define DIV_FUR_MAGIC "-Furnace module-"

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
    // Furnace-specific systems
    case 0x80:
      return DIV_SYSTEM_AY8910;
    case 0x81:
      return DIV_SYSTEM_AMIGA;
    case 0x82:
      return DIV_SYSTEM_YM2151;
    case 0x83:
      return DIV_SYSTEM_YM2612;
    case 0x84:
      return DIV_SYSTEM_TIA;
    case 0x97:
      return DIV_SYSTEM_SAA1099;
    case 0x9a:
      return DIV_SYSTEM_AY8930;
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
    // Furnace-specific systems
    case DIV_SYSTEM_AY8910:
      return 0x80;
    case DIV_SYSTEM_AMIGA:
      return 0x81;
    case DIV_SYSTEM_YM2151:
      return 0x82;
    case DIV_SYSTEM_YM2612:
      return 0x83;
    case DIV_SYSTEM_TIA:
      return 0x84;
    case DIV_SYSTEM_SAA1099:
      return 0x97;
    case DIV_SYSTEM_AY8930:
      return 0x9a;

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
    // Furnace-specific systems
    case DIV_SYSTEM_AY8910:
    case DIV_SYSTEM_AY8930:
      return 3;
    case DIV_SYSTEM_AMIGA:
      return 4;
    case DIV_SYSTEM_YM2151:
      return 8;
    case DIV_SYSTEM_YM2612:
      return 6;
    case DIV_SYSTEM_TIA:
      return 2;
    case DIV_SYSTEM_SAA1099:
      return 6;
  }
  return 0;
}

int DivEngine::getTotalChannelCount() {
  return chans;
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
    // Furnace-specific systems
    case DIV_SYSTEM_AY8910:
      return "AY-3-8910";
    case DIV_SYSTEM_AMIGA:
      return "Amiga";
    case DIV_SYSTEM_YM2151:
      return "Yamaha YM2151";
    case DIV_SYSTEM_YM2612:
      return "Yamaha YM2612";
    case DIV_SYSTEM_TIA:
      return "Atari 2600";
    case DIV_SYSTEM_SAA1099:
      return "Philips SAA1099";
    case DIV_SYSTEM_AY8930:
      return "Microchip AY8930";
  }
  return "Unknown";
}

const char* DivEngine::getSystemChips(DivSystem sys) {
  switch (sys) {
    case DIV_SYSTEM_NULL:
      return "Unknown";
    case DIV_SYSTEM_YMU759:
      return "Yamaha YMU759";
    case DIV_SYSTEM_GENESIS:
      return "Yamaha YM2612 + TI SN76489";
    case DIV_SYSTEM_SMS:
      return "TI SN76489";
    case DIV_SYSTEM_GB:
      return "Game Boy";
    case DIV_SYSTEM_PCE:
      return "Hudson Soft HuC6280";
    case DIV_SYSTEM_NES:
      return "Ricoh 2A03";
    case DIV_SYSTEM_C64_6581:
      return "SID 6581";
    case DIV_SYSTEM_C64_8580:
      return "SID 8580";
    case DIV_SYSTEM_ARCADE:
      return "Yamaha YM2151 + SegaPCM";
    case DIV_SYSTEM_GENESIS_EXT:
      return "Yamaha YM2612 (extended channel 3) + TI SN76489";
    case DIV_SYSTEM_YM2610:
      return "Yamaha YM2610";
    case DIV_SYSTEM_YM2610_EXT:
      return "Yamaha YM2610 (extended channel 2)";
    // Furnace-specific systems
    case DIV_SYSTEM_AY8910:
      return "AY-3-8910";
    case DIV_SYSTEM_AMIGA:
      return "Paula";
    case DIV_SYSTEM_YM2151:
      return "Yamaha YM2151 standalone";
    case DIV_SYSTEM_YM2612:
      return "Yamaha YM2612 standalone";
    case DIV_SYSTEM_TIA:
      return "Atari TIA";
    case DIV_SYSTEM_SAA1099:
      return "Philips SAA1099";
    case DIV_SYSTEM_AY8930:
      return "Microchip AY8930";
  }
  return "Unknown";
}

const char* DivEngine::getSystemNameJ(DivSystem sys) {
  switch (sys) {
    case DIV_SYSTEM_NULL:
      return "不明";
    case DIV_SYSTEM_YMU759:
      return "";
    case DIV_SYSTEM_GENESIS:
      return "セガメガドライブ";
    case DIV_SYSTEM_SMS:
      return "セガマスターシステム";
    case DIV_SYSTEM_GB:
      return "ゲームボーイ";
    case DIV_SYSTEM_PCE:
      return "PCエンジン";
    case DIV_SYSTEM_NES:
      return "ファミリーコンピュータ";
    case DIV_SYSTEM_C64_6581:
      return "コモドール64 (6581)";
    case DIV_SYSTEM_C64_8580:
      return "コモドール64 (8580)";
    case DIV_SYSTEM_ARCADE:
      return "Arcade";
    case DIV_SYSTEM_GENESIS_EXT:
      return "";
    case DIV_SYSTEM_YM2610:
      return "業務用ネオジオ";
    case DIV_SYSTEM_YM2610_EXT:
      return "";
    // Furnace-specific systems
    case DIV_SYSTEM_AY8910:
      return "";
    case DIV_SYSTEM_AMIGA:
      return "";
    case DIV_SYSTEM_YM2151:
      return "";
    case DIV_SYSTEM_YM2612:
      return "";
    case DIV_SYSTEM_TIA:
      return "";
    case DIV_SYSTEM_SAA1099:
      return "";
    case DIV_SYSTEM_AY8930:
      return "";
  }
  return "不明";
}

bool DivEngine::isFMSystem(DivSystem sys) {
  return (sys==DIV_SYSTEM_GENESIS ||
          sys==DIV_SYSTEM_GENESIS_EXT ||
          sys==DIV_SYSTEM_ARCADE ||
          sys==DIV_SYSTEM_YM2610 ||
          sys==DIV_SYSTEM_YM2610_EXT ||
          sys==DIV_SYSTEM_YMU759 ||
          sys==DIV_SYSTEM_YM2151 ||
          sys==DIV_SYSTEM_YM2612);
}

bool DivEngine::isSTDSystem(DivSystem sys) {
  return (sys!=DIV_SYSTEM_ARCADE &&
          sys!=DIV_SYSTEM_YMU759 &&
          sys!=DIV_SYSTEM_YM2612 &&
          sys!=DIV_SYSTEM_YM2151);
}

const char* chanNames[18][17]={
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
  {"Square 1", "Square 2", "Square 3"},  // AY-3-8910
  {"Channel 1", "Channel 2", "Channel 3", "Channel 4"},  // Amiga
  {"FM 1", "FM 2", "FM 3", "FM 4", "FM 5", "FM 6", "FM 7", "FM 8"}, // YM2151
  {"FM 1", "FM 2", "FM 3", "FM 4", "FM 5", "FM 6"}, // YM2612
  {"Channel 1", "Channel 2"}, // TIA
  {"Square 1", "Square 2", "Square 3", "Square 4", "Square 5", "Square 6"}, // SAA1099
  {"Square 1", "Square 2", "Square 3"},  // AY8930
};

const char* chanShortNames[18][17]={
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
  {"S1", "S2", "S3"},  // AY-3-8910
  {"CH1", "CH2", "CH3", "CH4"},  // Amiga
  {"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8"}, // YM2151
  {"F1", "F2", "F3", "F4", "F5", "F6"}, // YM2612
  {"CH1", "CH2"}, // TIA
  {"S1", "S2", "S3", "S4", "S5", "S6"}, // SAA1099
  {"S1", "S2", "S3"},  // AY8930
};

const int chanTypes[18][17]={
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
  {1, 1, 1},  // AY-3-8910
  {4, 4, 4, 4},  // Amiga
  {0, 0, 0, 0, 0, 0, 0, 0}, // YM2151
  {0, 0, 0, 0, 0, 0}, // YM2612
  {3, 3}, // TIA
  {1, 1, 1, 1, 1, 1}, // SAA1099
  {1, 1, 1},  // AY8930
};

const DivInstrumentType chanPrefType[18][17]={
  {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM}, // YMU759
  {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_STD, DIV_INS_STD, DIV_INS_STD, DIV_INS_STD}, // Genesis
  {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_STD, DIV_INS_STD, DIV_INS_STD, DIV_INS_STD}, // Genesis (extended channel 3)
  {DIV_INS_STD, DIV_INS_STD, DIV_INS_STD, DIV_INS_STD}, // SMS
  {DIV_INS_GB, DIV_INS_GB, DIV_INS_GB, DIV_INS_GB}, // GB
  {DIV_INS_PCE, DIV_INS_PCE, DIV_INS_PCE, DIV_INS_PCE, DIV_INS_PCE, DIV_INS_PCE}, // PCE
  {DIV_INS_STD, DIV_INS_STD, DIV_INS_STD, DIV_INS_STD, DIV_INS_STD}, // NES
  {DIV_INS_C64, DIV_INS_C64, DIV_INS_C64}, // C64
  {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM}, // Arcade
  {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_AY, DIV_INS_AY, DIV_INS_AY, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM}, // YM2610
  {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_AY, DIV_INS_AY, DIV_INS_AY, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM}, // YM2610 (extended channel 2)
  {DIV_INS_AY, DIV_INS_AY, DIV_INS_AY},  // AY-3-8910
  {DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA},  // Amiga
  {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM}, // YM2151
  {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM}, // YM2612
  {DIV_INS_TIA, DIV_INS_TIA}, // TIA
  {DIV_INS_SAA1099, DIV_INS_SAA1099, DIV_INS_SAA1099, DIV_INS_SAA1099, DIV_INS_SAA1099, DIV_INS_SAA1099}, // SAA1099
  {DIV_INS_AY8930, DIV_INS_AY8930, DIV_INS_AY8930},  // AY8930
};

const char* DivEngine::getChannelName(int chan) {
  if (chan<0 || chan>chans) return "??";
  switch (sysOfChan[chan]) {
    case DIV_SYSTEM_NULL: case DIV_SYSTEM_YMU759:
      return chanNames[0][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_GENESIS:
      return chanNames[1][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_GENESIS_EXT:
      return chanNames[2][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_SMS:
      return chanNames[3][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_GB:
      return chanNames[4][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_PCE:
      return chanNames[5][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_NES:
      return chanNames[6][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_C64_6581: case DIV_SYSTEM_C64_8580:
      return chanNames[7][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_ARCADE:
      return chanNames[8][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_YM2610:
      return chanNames[9][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_YM2610_EXT:
      return chanNames[10][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_AY8910:
      return chanNames[11][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_AMIGA:
      return chanNames[12][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_YM2151:
      return chanNames[13][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_YM2612:
      return chanNames[14][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_TIA:
      return chanNames[15][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_SAA1099:
      return chanNames[16][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_AY8930:
      return chanNames[17][dispatchChanOfChan[chan]];
      break;
  }
  return "??";
}

const char* DivEngine::getChannelShortName(int chan) {
  if (chan<0 || chan>chans) return "??";
  switch (sysOfChan[chan]) {
    case DIV_SYSTEM_NULL: case DIV_SYSTEM_YMU759:
      return chanShortNames[0][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_GENESIS:
      return chanShortNames[1][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_GENESIS_EXT:
      return chanShortNames[2][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_SMS:
      return chanShortNames[3][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_GB:
      return chanShortNames[4][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_PCE:
      return chanShortNames[5][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_NES:
      return chanShortNames[6][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_C64_6581: case DIV_SYSTEM_C64_8580:
      return chanShortNames[7][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_ARCADE:
      return chanShortNames[8][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_YM2610:
      return chanShortNames[9][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_YM2610_EXT:
      return chanShortNames[10][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_AY8910:
      return chanShortNames[11][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_AMIGA:
      return chanShortNames[12][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_YM2151:
      return chanShortNames[13][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_YM2612:
      return chanShortNames[14][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_TIA:
      return chanShortNames[15][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_SAA1099:
      return chanShortNames[16][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_AY8930:
      return chanShortNames[17][dispatchChanOfChan[chan]];
      break;
  }
  return "??";
}

int DivEngine::getChannelType(int chan) {
  switch (sysOfChan[chan]) {
    case DIV_SYSTEM_NULL: case DIV_SYSTEM_YMU759:
      return chanTypes[0][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_GENESIS:
      return chanTypes[1][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_GENESIS_EXT:
      return chanTypes[2][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_SMS:
      return chanTypes[3][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_GB:
      return chanTypes[4][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_PCE:
      return chanTypes[5][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_NES:
      return chanTypes[6][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_C64_6581: case DIV_SYSTEM_C64_8580:
      return chanTypes[7][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_ARCADE:
      return chanTypes[8][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_YM2610:
      return chanTypes[9][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_YM2610_EXT:
      return chanTypes[10][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_AY8910:
      return chanTypes[11][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_AMIGA:
      return chanTypes[12][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_YM2151:
      return chanTypes[13][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_YM2612:
      return chanTypes[14][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_TIA:
      return chanTypes[15][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_SAA1099:
      return chanTypes[16][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_AY8930:
      return chanTypes[17][dispatchChanOfChan[chan]];
      break;
  }
  return 1;
}

DivInstrumentType DivEngine::getPreferInsType(int chan) {
  switch (sysOfChan[chan]) {
    case DIV_SYSTEM_NULL: case DIV_SYSTEM_YMU759:
      return chanPrefType[0][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_GENESIS:
      return chanPrefType[1][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_GENESIS_EXT:
      return chanPrefType[2][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_SMS:
      return chanPrefType[3][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_GB:
      return chanPrefType[4][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_PCE:
      return chanPrefType[5][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_NES:
      return chanPrefType[6][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_C64_6581: case DIV_SYSTEM_C64_8580:
      return chanPrefType[7][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_ARCADE:
      return chanPrefType[8][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_YM2610:
      return chanPrefType[9][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_YM2610_EXT:
      return chanPrefType[10][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_AY8910:
      return chanPrefType[11][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_AMIGA:
      return chanPrefType[12][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_YM2151:
      return chanPrefType[13][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_YM2612:
      return chanPrefType[14][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_TIA:
      return chanPrefType[15][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_SAA1099:
      return chanPrefType[16][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_AY8930:
      return chanPrefType[17][dispatchChanOfChan[chan]];
      break;
  }
  return DIV_INS_FM;
}

bool DivEngine::isVGMExportable(DivSystem which) {
  switch (which) {
    case DIV_SYSTEM_GENESIS:
    case DIV_SYSTEM_GENESIS_EXT:
    case DIV_SYSTEM_SMS:
    case DIV_SYSTEM_GB:
    case DIV_SYSTEM_PCE:
    case DIV_SYSTEM_NES:
    case DIV_SYSTEM_ARCADE:
    case DIV_SYSTEM_YM2151:
    case DIV_SYSTEM_YM2612:
    case DIV_SYSTEM_YM2610:
    case DIV_SYSTEM_YM2610_EXT:
    case DIV_SYSTEM_AY8910:
    case DIV_SYSTEM_AY8930:
    case DIV_SYSTEM_SAA1099:
      return true;
    default:
      return false;
  }
  return false;
}

const char* DivEngine::getEffectDesc(unsigned char effect, int chan) {
  switch (effect) {
    case 0x00:
      return "00xy: Arpeggio";
    case 0x01:
      return "01xx: Pitch slide up";
    case 0x02:
      return "02xx: Pitch slide down";
    case 0x03:
      return "03xx: Portamento";
    case 0x04:
      return "04xy: Vibrato";
    case 0x08:
      return "08xy: Set panning";
    case 0x09:
      return "09xx: Set speed 1";
    case 0x0a:
      return "0Axy: Volume slide";
    case 0x0b:
      return "0Bxx: Jump to pattern";
    case 0x0c:
      return "0Cxx: Retrigger";
    case 0x0d:
      return "0Dxx: Jump to next pattern";
    case 0x0f:
      return "0Fxx: Set speed 2";
    case 0xe0:
      return "E0xx: Set arp speed";
    case 0xe1:
      return "E1xy: Note slide up";
    case 0xe2:
      return "E2xy: Note slide down";
    case 0xe3:
      return "E3xx: Set vibrato shape";
    case 0xe4:
      return "E4xx: Set vibrato range";
    case 0xe5:
      return "E5xx: Set pitch";
    case 0xea:
      return "EAxx: Legato";
    case 0xeb:
      return "EBxx: Set sample bank";
    case 0xec:
      return "ECxx: Note cut";
    case 0xed:
      return "EDxx: Note delay";
    case 0xee:
      return "EExx: Send external command";
    case 0xef:
      return "EFxx: Set global tuning";
    default:
      if (chan>=0 && chan<chans) {
        const char* ret=disCont[dispatchOfChan[chan]].dispatch->getEffectName(effect);
        if (ret!=NULL) return ret;
      }
      break;
  }
  return "Invalid effect";
}

#define addWarning(x) \
  if (warnings.empty()) { \
    warnings+=x; \
  } else { \
    warnings+=("\n" x); \
  }

bool DivEngine::loadDMF(unsigned char* file, size_t len) {
  SafeReader reader=SafeReader(file,len);
  warnings="";
  try {
    DivSong ds;

    ds.nullWave.len=32;
    for (int i=0; i<32; i++) {
      ds.nullWave.data[i]=15;
    }

    if (!reader.seek(16,SEEK_SET)) {
      logE("premature end of file!\n");
      lastError="incomplete file";
      delete[] file;
      return false;
    }
    ds.version=(unsigned char)reader.readC();
    logI("module version %d (0x%.2x)\n",ds.version,ds.version);
    if (ds.version>0x18) {
      logW("this version is not supported by Furnace yet!\n");
      lastError="this version is not supported by Furnace yet";
      delete[] file;
      return false;
    }
    unsigned char sys=0;
    ds.systemLen=1;
    if (ds.version<0x09) {
      // V E R S I O N  -> 3 <-
      // AWESOME
      ds.system[0]=DIV_SYSTEM_YMU759;
    } else {
      sys=reader.readC();
      ds.system[0]=systemFromFile(sys);
    }
    if (ds.system[0]==DIV_SYSTEM_NULL) {
      logE("invalid system 0x%.2x!",sys);
      lastError="system not supported. running old version?";
      delete[] file;
      return false;
    }
    
    if (ds.system[0]==DIV_SYSTEM_YMU759 && ds.version<0x10) {
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
        try {
          ds.hz=std::stoi(hz);
        } catch (std::exception& e) {
          logW("invalid custom Hz!\n");
          ds.hz=60;
        }
      }
    }
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

    if (ds.system[0]==DIV_SYSTEM_YMU759) {
      switch (ds.timeBase) {
        case 0:
          ds.hz=248;
          break;
        case 1:
          ds.hz=200;
          break;
        case 2:
          ds.hz=100;
          break;
        case 3:
          ds.hz=50;
          break;
        case 4:
          ds.hz=25;
          break;
        case 5:
          ds.hz=20;
          break;
        default:
          ds.hz=248;
          break;
      }
      ds.customTempo=true;
      ds.timeBase=0;
      addWarning("Yamaha YMU759 emulation is not currently possible!");
    }

    logI("reading pattern matrix (%d)...\n",ds.ordersLen);
    for (int i=0; i<getChannelCount(ds.system[0]); i++) {
      for (int j=0; j<ds.ordersLen; j++) {
        ds.orders.ord[i][j]=reader.readC();
      }
    }

    if (ds.version>0x03) {
      ds.insLen=(unsigned char)reader.readC();
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
      ins->type=ins->mode?DIV_INS_FM:DIV_INS_STD;
      if (ds.system[0]==DIV_SYSTEM_GB) {
        ins->type=DIV_INS_GB;
      }
      if (ds.system[0]==DIV_SYSTEM_C64_8580 || ds.system[0]==DIV_SYSTEM_C64_6581) {
        ins->type=DIV_INS_C64;
      }
      if (ds.system[0]==DIV_SYSTEM_YM2610 || ds.system[0]==DIV_SYSTEM_YM2610_EXT) {
        if (!ins->mode) {
          ins->type=DIV_INS_AY;
        }
        ins->std.dutyMacroHeight=31;
        ins->std.waveMacroHeight=7;
      }
      if (ds.system[0]==DIV_SYSTEM_PCE) {
        ins->type=DIV_INS_PCE;
        ins->std.volMacroHeight=31;
      }

      if (ins->mode) { // FM
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
          if (ds.system[0]!=DIV_SYSTEM_YMU759) ins->fm.ops=4;
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
            if (ds.version<0x11) { // don't know when did this change
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
        if (ds.system[0]!=DIV_SYSTEM_GB || ds.version<0x12) {
          ins->std.volMacroLen=reader.readC();
          for (int j=0; j<ins->std.volMacroLen; j++) {
            if (ds.version<0x0e) {
              ins->std.volMacro[j]=reader.readC();
            } else {
              ins->std.volMacro[j]=reader.readI();
            }
          }
          if (ins->std.volMacroLen>0) {
            ins->std.volMacroOpen=true;
            ins->std.volMacroLoop=reader.readC();
          } else {
            ins->std.volMacroOpen=false;
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
          ins->std.arpMacroOpen=true;
        } else {
          ins->std.arpMacroOpen=false;
        }
        if (ds.version>0x0f) {
          ins->std.arpMacroMode=reader.readC();
        }
        if (!ins->std.arpMacroMode) {
          for (int j=0; j<ins->std.arpMacroLen; j++) {
            ins->std.arpMacro[j]-=12;
          }
        }

        ins->std.dutyMacroLen=reader.readC();
        for (int j=0; j<ins->std.dutyMacroLen; j++) {
          if (ds.version<0x0e) {
            ins->std.dutyMacro[j]=reader.readC();
          } else {
            ins->std.dutyMacro[j]=reader.readI();
          }
          if ((ds.system[0]==DIV_SYSTEM_C64_8580 || ds.system[0]==DIV_SYSTEM_C64_6581) && ins->std.dutyMacro[j]>24) {
            ins->std.dutyMacro[j]=24;
          }
        }
        if (ins->std.dutyMacroLen>0) {
          ins->std.dutyMacroOpen=true;
          ins->std.dutyMacroLoop=reader.readC();
        } else {
          ins->std.dutyMacroOpen=false;
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
          ins->std.waveMacroOpen=true;
          ins->std.waveMacroLoop=reader.readC();
        } else {
          ins->std.waveMacroOpen=false;
        }

        if (ds.system[0]==DIV_SYSTEM_C64_6581 || ds.system[0]==DIV_SYSTEM_C64_8580) {
          ins->c64.triOn=reader.readC();
          ins->c64.sawOn=reader.readC();
          ins->c64.pulseOn=reader.readC();
          ins->c64.noiseOn=reader.readC();

          ins->c64.a=reader.readC();
          ins->c64.d=reader.readC();
          ins->c64.s=reader.readC();
          ins->c64.r=reader.readC();

          ins->c64.duty=(reader.readC()*4095)/100;

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
          ins->c64.cut=(reader.readC()*2047)/100;
          ins->c64.hp=reader.readC();
          ins->c64.bp=reader.readC();
          ins->c64.lp=reader.readC();
          ins->c64.ch3off=reader.readC();
        }

        if (ds.system[0]==DIV_SYSTEM_GB && ds.version>0x11) {
          ins->gb.envVol=reader.readC();
          ins->gb.envDir=reader.readC();
          ins->gb.envLen=reader.readC();
          ins->gb.soundLen=reader.readC();
          ins->std.volMacroOpen=false;

          logD("GB data: vol %d dir %d len %d sl %d\n",ins->gb.envVol,ins->gb.envDir,ins->gb.envLen,ins->gb.soundLen);
        } else if (ds.system[0]==DIV_SYSTEM_GB) {
          // try to convert macro to envelope
          if (ins->std.volMacroLen>0) {
            ins->gb.envVol=ins->std.volMacro[0];
            if (ins->std.volMacro[0]<ins->std.volMacro[1]) {
              ins->gb.envDir=true;
            }
            if (ins->std.volMacro[ins->std.volMacroLen-1]==0) {
              ins->gb.soundLen=ins->std.volMacroLen*2;
            }
          }
          addWarning("Game Boy volume macros converted to envelopes. may not be perfect!");
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
        if (ds.system[0]==DIV_SYSTEM_GB) {
          wave->max=15;
        }
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

    logI("reading patterns (%d channels, %d orders)...\n",getChannelCount(ds.system[0]),ds.ordersLen);
    for (int i=0; i<getChannelCount(ds.system[0]); i++) {
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
          if (ds.system[0]==DIV_SYSTEM_SMS && ds.version<0x0e && pat->data[k][1]>0) {
            // apparently it was up one octave before
            pat->data[k][1]--;
          } else if (ds.system[0]==DIV_SYSTEM_GENESIS && ds.version<0x0e && pat->data[k][1]>0 && i>5) {
            // ditto
            pat->data[k][1]--;
          }
          if (ds.version<0x12) {
            if (ds.system[0]==DIV_SYSTEM_GB && i==3 && pat->data[k][1]>0) {
              // back then noise was 2 octaves lower
              pat->data[k][1]-=2;
            }
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
          if (ds.version<0x12) {
            if (ds.system[0]==DIV_SYSTEM_GB && i==2 && pat->data[k][3]>0) {
              // volume range of GB wave channel was 0-3 rather than 0-F
              pat->data[k][3]=(pat->data[k][3]&3)*5;
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

    ds.sampleLen=(unsigned char)reader.readC();
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
    recalcChans();
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

bool DivEngine::loadFur(unsigned char* file, size_t len) {
  int insPtr[256];
  int wavePtr[256];
  int samplePtr[256];
  std::vector<int> patPtr;
  char magic[5];
  memset(magic,0,5);
  SafeReader reader=SafeReader(file,len);
  warnings="";
  try {
    DivSong ds;

    if (!reader.seek(16,SEEK_SET)) {
      logE("premature end of file!\n");
      lastError="incomplete file";
      delete[] file;
      return false;
    }
    ds.version=reader.readS();
    logI("module version %d (0x%.2x)\n",ds.version,ds.version);

    if (ds.version>DIV_ENGINE_VERSION) {
      logW("this module was created with a more recent version of Furnace!\n");
      addWarning("this module was created with a more recent version of Furnace!");
    }

    reader.readS(); // reserved
    int infoSeek=reader.readI();

    reader.seek(infoSeek,SEEK_SET);

    // read header
    reader.read(magic,4);
    if (strcmp(magic,"INFO")!=0) {
      logE("invalid info header!\n");
      lastError="invalid info header!";
      delete[] file;
      return false;
    }
    reader.readI();

    ds.timeBase=reader.readC();
    ds.speed1=reader.readC();
    ds.speed2=reader.readC();
    ds.arpLen=reader.readC();
    ds.hz=reader.readF();
    ds.pal=(ds.hz>=53);
    if (ds.hz!=50 && ds.hz!=60) ds.customTempo=true;

    ds.patLen=reader.readS();
    ds.ordersLen=reader.readS();

    ds.hilightA=reader.readC();
    ds.hilightB=reader.readC();

    ds.insLen=reader.readS();
    ds.waveLen=reader.readS();
    ds.sampleLen=reader.readS();
    int numberOfPats=reader.readI();

    for (int i=0; i<32; i++) {
      ds.system[i]=systemFromFile(reader.readC());
      if (ds.system[i]!=DIV_SYSTEM_NULL) ds.systemLen=i+1;
    }
    int tchans=0;
    for (int i=0; i<ds.systemLen; i++) {
      tchans+=getChannelCount(ds.system[i]);
    }
    if (tchans>DIV_MAX_CHANS) tchans=DIV_MAX_CHANS;

    // system volume
    for (int i=0; i<32; i++) ds.systemVol[i]=reader.readC();

    // system panning
    for (int i=0; i<32; i++) ds.systemPan[i]=reader.readC();

    // system props
    for (int i=0; i<32; i++) {
      ds.systemFlags[i]=reader.readI();
    }

    ds.name=reader.readString();
    ds.author=reader.readString();
    logI("%s by %s\n",ds.name.c_str(),ds.author.c_str());

    if (ds.version>=33) {
      ds.tuning=reader.readF();
    } else {
      reader.readI();
    }

    // reserved
    for (int i=0; i<20; i++) reader.readC();

    // pointers
    reader.read(insPtr,ds.insLen*4);
    reader.read(wavePtr,ds.waveLen*4);
    reader.read(samplePtr,ds.sampleLen*4);
    for (int i=0; i<numberOfPats; i++) patPtr.push_back(reader.readI());

    for (int i=0; i<tchans; i++) {
      for (int j=0; j<ds.ordersLen; j++) {
        ds.orders.ord[i][j]=reader.readC();
      }
    }

    for (int i=0; i<tchans; i++) {
      ds.pat[i].effectRows=reader.readC();
    }

    // read instruments
    for (int i=0; i<ds.insLen; i++) {
      DivInstrument* ins=new DivInstrument;
      reader.seek(insPtr[i],SEEK_SET);
      
      if (ins->readInsData(reader,ds.version)!=DIV_DATA_SUCCESS) {
        lastError="invalid instrument header/data!";
        delete ins;
        delete[] file;
        return false;
      }

      ds.ins.push_back(ins);
    }

    // read wavetables
    for (int i=0; i<ds.waveLen; i++) {
      DivWavetable* wave=new DivWavetable;
      reader.seek(wavePtr[i],SEEK_SET);

      if (wave->readWaveData(reader,ds.version)!=DIV_DATA_SUCCESS) {
        lastError="invalid wavetable header/data!";
        delete wave;
        delete[] file;
        return false;
      }

      ds.wave.push_back(wave);
    }

    // read samples
    for (int i=0; i<ds.sampleLen; i++) {
      reader.seek(samplePtr[i],SEEK_SET);
      reader.read(magic,4);
      if (strcmp(magic,"SMPL")!=0) {
        logE("%d: invalid sample header!\n",i);
        lastError="invalid sample header!";
        delete[] file;
        return false;
      }
      reader.readI();
      DivSample* sample=new DivSample;

      sample->name=reader.readString();
      sample->length=reader.readI();
      sample->rate=reader.readI();
      sample->vol=reader.readS();
      sample->pitch=reader.readS();
      sample->depth=reader.readC();

      // reserved
      reader.readC();

      if (ds.version>=32) {
        sample->centerRate=reader.readS();
      } else {
        reader.readS();
      }

      if (ds.version>=19) {
        sample->loopStart=reader.readI();
      } else {
        reader.readI();
      }

      sample->data=new short[sample->length];
      reader.read(sample->data,2*sample->length);

      ds.sample.push_back(sample);
    }

    // read patterns
    for (int i: patPtr) {
      reader.seek(i,SEEK_SET);
      reader.read(magic,4);
      if (strcmp(magic,"PATR")!=0) {
        logE("%x: invalid pattern header!\n",i);
        lastError="invalid pattern header!";
        delete[] file;
        return false;
      }
      reader.readI();

      int chan=reader.readS();
      int index=reader.readS();
      reader.readI();

      DivPattern* pat=ds.pat[chan].getPattern(index,true);
      for (int j=0; j<ds.patLen; j++) {
        pat->data[j][0]=reader.readS();
        pat->data[j][1]=reader.readS();
        pat->data[j][2]=reader.readS();
        pat->data[j][3]=reader.readS();
        for (int k=0; k<ds.pat[chan].effectRows; k++) {
          pat->data[j][4+(k<<1)]=reader.readS();
          pat->data[j][5+(k<<1)]=reader.readS();
        }
      }
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
    recalcChans();
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

bool DivEngine::load(unsigned char* f, size_t slen) {
  unsigned char* file;
  size_t len;
  if (slen<16) {
    logE("too small!");
    lastError="file is too small";
    delete[] f;
    return false;
  }
  if (memcmp(f,DIV_DMF_MAGIC,16)!=0 && memcmp(f,DIV_FUR_MAGIC,16)!=0) {
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
  if (memcmp(file,DIV_DMF_MAGIC,16)==0) {
    return loadDMF(file,len); 
  } else if (memcmp(file,DIV_FUR_MAGIC,16)==0) {
    return loadFur(file,len);
  }
  logE("not a valid module!\n");
  lastError="not a compatible song";
  delete[] file;
  return false;
}

SafeWriter* DivEngine::saveFur() {
  int insPtr[256];
  int wavePtr[256];
  int samplePtr[256];
  std::vector<int> patPtr;
  size_t ptrSeek;
  warnings="";

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
  std::vector<int> patsToWrite;
  bool alreadyAdded[256];
  for (int i=0; i<chans; i++) {
    memset(alreadyAdded,0,256*sizeof(bool));
    for (int j=0; j<song.ordersLen; j++) {
      if (alreadyAdded[song.orders.ord[i][j]]) continue;
      patsToWrite.push_back((i<<16)|song.orders.ord[i][j]);
      alreadyAdded[song.orders.ord[i][j]]=true;
    }
  }

  /// SONG INFO
  w->write("INFO",4);
  w->writeI(0);

  w->writeC(song.timeBase);
  w->writeC(song.speed1);
  w->writeC(song.speed2);
  w->writeC(song.arpLen);
  w->writeF(song.hz);
  w->writeS(song.patLen);
  w->writeS(song.ordersLen);
  w->writeC(song.hilightA);
  w->writeC(song.hilightB);
  w->writeS(song.insLen);
  w->writeS(song.waveLen);
  w->writeS(song.sampleLen);
  w->writeI(patsToWrite.size());

  for (int i=0; i<32; i++) {
    if (i>=song.systemLen) {
      w->writeC(0);
    } else {
      w->writeC(systemToFile(song.system[i]));
    }
  }

  for (int i=0; i<32; i++) {
    w->writeC(song.systemVol[i]);
  }

  for (int i=0; i<32; i++) {
    w->writeC(song.systemPan[i]);
  }

  for (int i=0; i<32; i++) {
    w->writeI(song.systemFlags[i]);
  }

  // song name
  w->writeString(song.name,false);
  // song author
  w->writeString(song.author,false);

  w->writeF(song.tuning);
  
  // reserved
  for (int i=0; i<20; i++) {
    w->writeC(0);
  }

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
    for (int j=0; j<song.ordersLen; j++) {
      w->writeC(song.orders.ord[i][j]);
    }
  }

  for (int i=0; i<chans; i++) {
    w->writeC(song.pat[i].effectRows);
  }

  /// INSTRUMENT
  for (int i=0; i<song.insLen; i++) {
    DivInstrument* ins=song.ins[i];
    insPtr[i]=w->tell();
    ins->putInsData(w);
  }

  /// WAVETABLE
  for (int i=0; i<song.waveLen; i++) {
    DivWavetable* wave=song.wave[i];
    wavePtr[i]=w->tell();
    wave->putWaveData(w);
  }

  /// SAMPLE
  for (int i=0; i<song.sampleLen; i++) {
    DivSample* sample=song.sample[i];
    samplePtr[i]=w->tell();
    w->write("SMPL",4);
    w->writeI(0);

    w->writeString(sample->name,false);
    w->writeI(sample->length);
    w->writeI(sample->rate);
    w->writeS(sample->vol);
    w->writeS(sample->pitch);
    w->writeC(sample->depth);
    w->writeC(0);
    w->writeS(sample->centerRate);
    w->writeI(sample->loopStart);

    w->write(sample->data,sample->length*2);
  }

  /// PATTERN
  for (int i: patsToWrite) {
    DivPattern* pat=song.pat[i>>16].getPattern(i&0xffff,false);
    patPtr.push_back(w->tell());
    w->write("PATR",4);
    w->writeI(0);

    w->writeS(i>>16);
    w->writeS(i&0xffff);

    w->writeI(0); // reserved

    for (int j=0; j<song.patLen; j++) {
      w->writeS(pat->data[j][0]); // note
      w->writeS(pat->data[j][1]); // octave
      w->writeS(pat->data[j][2]); // instrument
      w->writeS(pat->data[j][3]); // volume
      w->write(&pat->data[j][4],2*song.pat[i>>16].effectRows*2); // effects
    }
  }

  /// POINTERS
  w->seek(ptrSeek,SEEK_SET);

  for (int i=0; i<song.insLen; i++) {
    w->writeI(insPtr[i]);
  }

  // wavetable pointers (we'll seek here later)
  for (int i=0; i<song.waveLen; i++) {
    w->writeI(wavePtr[i]);
  }

  // sample pointers (we'll seek here later)
  for (int i=0; i<song.sampleLen; i++) {
    w->writeI(samplePtr[i]);
  }

  // pattern pointers (we'll seek here later)
  for (int i: patPtr) {
    w->writeI(i);
  }

  return w;
}

SafeWriter* DivEngine::saveDMF() {
  // fail if more than one system
  if (song.systemLen!=1) {
    logE("cannot save multiple systems in this format!\n");
    lastError="multiple systems not possible on .dmf";
    return NULL;
  }
  // fail if this is an YMU759 song
  if (song.system[0]==DIV_SYSTEM_YMU759) {
    logE("cannot save YMU759 song!\n");
    lastError="YMU759 song saving is not supported";
    return NULL;
  }
  // fail if the system is Furnace-exclusive
  if (systemToFile(song.system[0])&0x80) {
    logE("cannot save Furnace-exclusive system song!\n");
    lastError="this system is not possible on .dmf";
    return NULL;
  }
  warnings="";

  SafeWriter* w=new SafeWriter;
  w->init();
  // write magic
  w->write(DIV_DMF_MAGIC,16);
  // version
  w->writeC(24);
  w->writeC(systemToFile(song.system[0]));

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

  for (int i=0; i<chans; i++) {
    for (int j=0; j<song.ordersLen; j++) {
      w->writeC(song.orders.ord[i][j]);
    }
  }

  if (song.system[0]==DIV_SYSTEM_C64_6581 || song.system[0]==DIV_SYSTEM_C64_8580) {
    addWarning("absolute duty/cutoff macro not available in .dmf!");
    addWarning("duty precision will be lost");
  }

  for (DivInstrument* i: song.ins) {
    if (i->type==DIV_INS_AMIGA) {
      addWarning(".dmf format does not support arbitrary-pitch sample mode");
      break;
    }
  }

  for (DivInstrument* i: song.ins) {
    if (i->type==DIV_INS_FM) {
      addWarning("no FM macros in .dmf format");
      break;
    }
  }

  w->writeC(song.ins.size());
  for (DivInstrument* i: song.ins) {
    w->writeString(i->name,true);

    // safety check
    if (!isFMSystem(song.system[0]) && i->mode) {
      i->mode=0;
    }
    if (!isSTDSystem(song.system[0]) && i->mode==0) {
      i->mode=1;
    }

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
      if (song.system[0]!=DIV_SYSTEM_GB) {
        w->writeC(i->std.volMacroLen);
        w->write(i->std.volMacro,4*i->std.volMacroLen);
        if (i->std.volMacroLen>0) {
          w->writeC(i->std.volMacroLoop);
        }
      }

      w->writeC(i->std.arpMacroLen);
      if (i->std.arpMacroMode) {
        w->write(i->std.arpMacro,4*i->std.arpMacroLen);
      } else {
        for (int j=0; j<i->std.arpMacroLen; j++) {
          w->writeI(i->std.arpMacro[j]+12);
        }
      }
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

      if (song.system[0]==DIV_SYSTEM_C64_6581 || song.system[0]==DIV_SYSTEM_C64_8580) {
        w->writeC(i->c64.triOn);
        w->writeC(i->c64.sawOn);
        w->writeC(i->c64.pulseOn);
        w->writeC(i->c64.noiseOn);

        w->writeC(i->c64.a);
        w->writeC(i->c64.d);
        w->writeC(i->c64.s);
        w->writeC(i->c64.r);

        logW("duty and cutoff precision will be lost!\n");
        w->writeC((i->c64.duty*100)/4095);

        w->writeC(i->c64.ringMod);
        w->writeC(i->c64.oscSync);

        w->writeC(i->c64.toFilter);
        w->writeC(i->c64.volIsCutoff);
        w->writeC(i->c64.initFilter);

        w->writeC(i->c64.res);
        w->writeC((i->c64.cut*100)/2047);
        w->writeC(i->c64.hp);
        w->writeC(i->c64.bp);
        w->writeC(i->c64.lp);
        w->writeC(i->c64.ch3off);
      }

      if (song.system[0]==DIV_SYSTEM_GB) {
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

  for (int i=0; i<getChannelCount(song.system[0]); i++) {
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

  if (song.sample.size()>0) {
    addWarning("samples' rates will be rounded to nearest compatible value");
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

void DivEngine::performVGMWrite(SafeWriter* w, DivSystem sys, DivRegWrite& write, int streamOff, double* loopTimer, double* loopFreq, int* loopSample, bool isSecond) {
  if (write.addr==0xffffffff) { // Furnace fake reset
    switch (sys) {
      case DIV_SYSTEM_GENESIS:
      case DIV_SYSTEM_GENESIS_EXT:
      case DIV_SYSTEM_YM2612:
        for (int i=0; i<3; i++) { // set SL and RR to highest
          w->writeC(isSecond?0xa2:0x52);
          w->writeC(0x80+i);
          w->writeC(0xff);
          w->writeC(isSecond?0xa2:0x52);
          w->writeC(0x84+i);
          w->writeC(0xff);
          w->writeC(isSecond?0xa2:0x52);
          w->writeC(0x88+i);
          w->writeC(0xff);
          w->writeC(isSecond?0xa2:0x52);
          w->writeC(0x8c+i);
          w->writeC(0xff);

          w->writeC(isSecond?0xa3:0x53);
          w->writeC(0x80+i);
          w->writeC(0xff);
          w->writeC(isSecond?0xa3:0x53);
          w->writeC(0x84+i);
          w->writeC(0xff);
          w->writeC(isSecond?0xa3:0x53);
          w->writeC(0x88+i);
          w->writeC(0xff);
          w->writeC(isSecond?0xa3:0x53);
          w->writeC(0x8c+i);
          w->writeC(0xff);
        }
        for (int i=0; i<3; i++) { // note off
          w->writeC(isSecond?0xa2:0x52);
          w->writeC(0x28);
          w->writeC(i);
          w->writeC(isSecond?0xa2:0x52);
          w->writeC(0x28);
          w->writeC(4+i);
        }
        w->writeC(isSecond?0xa2:0x52); // disable DAC
        w->writeC(0x2b);
        w->writeC(0);
        if (sys!=DIV_SYSTEM_YM2612) {
          for (int i=0; i<4; i++) {
            w->writeC(isSecond?0x30:0x50);
            w->writeC(0x90|(i<<5)|15);
          }
        }
        break;
      case DIV_SYSTEM_SMS:
        for (int i=0; i<4; i++) {
          w->writeC(isSecond?0x30:0x50);
          w->writeC(0x90|(i<<5)|15);
        }
        break;
      case DIV_SYSTEM_GB:
        // square 1
        w->writeC(0xb3);
        w->writeC(isSecond?0x82:2);
        w->writeC(0);
        w->writeC(0xb3);
        w->writeC(isSecond?0x84:4);
        w->writeC(0x80);

        // square 2
        w->writeC(0xb3);
        w->writeC(isSecond?0x87:7);
        w->writeC(0);
        w->writeC(0xb3);
        w->writeC(isSecond?0x89:9);
        w->writeC(0x80);

        // wave
        w->writeC(0xb3);
        w->writeC(isSecond?0x8c:0x0c);
        w->writeC(0);
        w->writeC(0xb3);
        w->writeC(isSecond?0x8e:0x0e);
        w->writeC(0x80);

        // noise
        w->writeC(0xb3);
        w->writeC(isSecond?0x91:0x11);
        w->writeC(0);
        w->writeC(0xb3);
        w->writeC(isSecond?0x93:0x13);
        w->writeC(0x80);
        break;
      case DIV_SYSTEM_PCE:
        for (int i=0; i<6; i++) {
          w->writeC(0xb9);
          w->writeC(isSecond?0x80:0);
          w->writeC(i);
          w->writeC(0xb9);
          w->writeC(isSecond?0x84:4);
          w->writeC(0);
        }
        break;
      case DIV_SYSTEM_NES:
        w->writeC(0xb4);
        w->writeC(isSecond?0x95:0x15);
        w->writeC(0);
        break;
      case DIV_SYSTEM_ARCADE:
      case DIV_SYSTEM_YM2151:
        for (int i=0; i<8; i++) {
          w->writeC(isSecond?0xa4:0x54);
          w->writeC(0xe0+i);
          w->writeC(0xff);
          w->writeC(isSecond?0xa4:0x54);
          w->writeC(0xe8+i);
          w->writeC(0xff);
          w->writeC(isSecond?0xa4:0x54);
          w->writeC(0xf0+i);
          w->writeC(0xff);
          w->writeC(isSecond?0xa4:0x54);
          w->writeC(0xf8+i);
          w->writeC(0xff);

          w->writeC(isSecond?0xa4:0x54);
          w->writeC(0x08);
          w->writeC(i);
        }
        if (sys==DIV_SYSTEM_ARCADE) {
          for (int i=0; i<5; i++) {
            w->writeC(0xc0);
            w->writeS((isSecond?0x8086:0x86)+(i<<3));
            w->writeC(3);
          }
        }
        break;
      case DIV_SYSTEM_YM2610:
      case DIV_SYSTEM_YM2610_EXT:
        for (int i=0; i<2; i++) { // set SL and RR to highest
          w->writeC(isSecond?0xa8:0x58);
          w->writeC(0x81+i);
          w->writeC(0xff);
          w->writeC(isSecond?0xa8:0x58);
          w->writeC(0x85+i);
          w->writeC(0xff);
          w->writeC(isSecond?0xa8:0x58);
          w->writeC(0x89+i);
          w->writeC(0xff);
          w->writeC(isSecond?0xa8:0x58);
          w->writeC(0x8d+i);
          w->writeC(0xff);

          w->writeC(isSecond?0xa9:0x59);
          w->writeC(0x81+i);
          w->writeC(0xff);
          w->writeC(isSecond?0xa9:0x59);
          w->writeC(0x85+i);
          w->writeC(0xff);
          w->writeC(isSecond?0xa9:0x59);
          w->writeC(0x89+i);
          w->writeC(0xff);
          w->writeC(isSecond?0xa9:0x59);
          w->writeC(0x8d+i);
          w->writeC(0xff);
        }
        for (int i=0; i<2; i++) { // note off
          w->writeC(isSecond?0xa8:0x58);
          w->writeC(0x28);
          w->writeC(1+i);
          w->writeC(isSecond?0xa8:0x58);
          w->writeC(0x28);
          w->writeC(5+i);
        }
        
        // reset AY
        w->writeC(isSecond?0xa8:0x58);
        w->writeC(7);
        w->writeC(0x3f);

        w->writeC(isSecond?0xa8:0x58);
        w->writeC(8);
        w->writeC(0);

        w->writeC(isSecond?0xa8:0x58);
        w->writeC(9);
        w->writeC(0);

        w->writeC(isSecond?0xa8:0x58);
        w->writeC(10);
        w->writeC(0);

        // reset sample
        w->writeC(isSecond?0xa9:0x59);
        w->writeC(0);
        w->writeC(0xbf);
        break;
      case DIV_SYSTEM_AY8910:
        w->writeC(0xa0);
        w->writeC(isSecond?0x87:7);
        w->writeC(0x3f);

        w->writeC(0xa0);
        w->writeC(isSecond?0x88:8);
        w->writeC(0);

        w->writeC(0xa0);
        w->writeC(isSecond?0x89:9);
        w->writeC(0);

        w->writeC(0xa0);
        w->writeC(isSecond?0x8a:10);
        w->writeC(0);
        break;
      case DIV_SYSTEM_AY8930:
        w->writeC(0xa0);
        w->writeC(isSecond?0x8d:0x0d);
        w->writeC(0);
        w->writeC(0xa0);
        w->writeC(isSecond?0x8d:0x0d);
        w->writeC(0xa0);
        break;
      case DIV_SYSTEM_SAA1099:
        w->writeC(0xbd);
        w->writeC(isSecond?0x9c:0x1c);
        w->writeC(0x02);
        w->writeC(0xbd);
        w->writeC(isSecond?0x94:0x14);
        w->writeC(0);
        w->writeC(0xbd);
        w->writeC(isSecond?0x95:0x15);
        w->writeC(0);

        for (int i=0; i<6; i++) {
          w->writeC(0xbd);
          w->writeC((isSecond?0x80:0)+i);
          w->writeC(0);
        }
        break;
      default:
        break;
    }
  }
  if (write.addr>=0xffff0000) { // Furnace special command
    unsigned char streamID=streamOff+((write.addr&0xff00)>>8);
    switch (write.addr&0xff) {
      case 0: // play sample
        if (write.val<song.sampleLen) {
          DivSample* sample=song.sample[write.val];
          w->writeC(0x95);
          w->writeC(streamID);
          w->writeS(write.val); // sample number
          w->writeC((sample->loopStart==0)); // flags
          if (sample->loopStart>0) {
            loopTimer[streamID]=sample->rendLength;
            loopSample[streamID]=write.val;
          }
        }
        break;
      case 1: // set sample freq
        w->writeC(0x92);
        w->writeC(streamID);
        w->writeI(write.val);
        loopFreq[streamID]=write.val;
        break;
      case 2: // stop sample
        w->writeC(0x94);
        w->writeC(streamID);
        loopSample[streamID]=-1;
        break;
    }
    return;
  }
  switch (sys) {
    case DIV_SYSTEM_GENESIS:
    case DIV_SYSTEM_GENESIS_EXT:
    case DIV_SYSTEM_YM2612:
      switch (write.addr>>8) {
        case 0: // port 0
          w->writeC(isSecond?0xa2:0x52);
          w->writeC(write.addr&0xff);
          w->writeC(write.val);
          break;
        case 1: // port 1
          w->writeC(isSecond?0xa3:0x53);
          w->writeC(write.addr&0xff);
          w->writeC(write.val);
          break;
        case 2: // PSG
          w->writeC(isSecond?0x30:0x50);
          w->writeC(write.val);
          break;
      }
      break;
    case DIV_SYSTEM_SMS:
      w->writeC(isSecond?0x30:0x50);
      w->writeC(write.val);
      break;
    case DIV_SYSTEM_GB:
      w->writeC(0xb3);
      w->writeC((isSecond?0x80:0)|((write.addr-16)&0xff));
      w->writeC(write.val);
      break;
    case DIV_SYSTEM_PCE:
      w->writeC(0xb9);
      w->writeC((isSecond?0x80:0)|(write.addr&0xff));
      w->writeC(write.val);
      break;
    case DIV_SYSTEM_NES:
      w->writeC(0xb4);
      w->writeC((isSecond?0x80:0)|(write.addr&0xff));
      w->writeC(write.val);
      break;
    case DIV_SYSTEM_ARCADE:
    case DIV_SYSTEM_YM2151:
      switch (write.addr>>16) {
        case 0: // YM2151
          w->writeC(isSecond?0xa4:0x54);
          w->writeC(write.addr&0xff);
          w->writeC(write.val);
          break;
        case 1: // SegaPCM
          w->writeC(0xc0);
          w->writeS((isSecond?0x8000:0)|(write.addr&0xffff));
          w->writeC(write.val);
          break;
      }
      break;
    case DIV_SYSTEM_YM2610:
    case DIV_SYSTEM_YM2610_EXT:
      switch (write.addr>>8) {
        case 0: // port 0
          w->writeC(isSecond?0xa8:0x58);
          w->writeC(write.addr&0xff);
          w->writeC(write.val);
          break;
        case 1: // port 1
          w->writeC(isSecond?0xa9:0x59);
          w->writeC(write.addr&0xff);
          w->writeC(write.val);
          break;
      }
      break;
    case DIV_SYSTEM_AY8910:
    case DIV_SYSTEM_AY8930:
      w->writeC(0xa0);
      w->writeC((isSecond?0x80:0)|(write.addr&0xff));
      w->writeC(write.val);
      break;
    case DIV_SYSTEM_SAA1099:
      w->writeC(0xbd);
      w->writeC((isSecond?0x80:0)|(write.addr&0xff));
      w->writeC(write.val);
      break;
    default:
      logW("write not handled!\n");
      break;
  }
}

void DivEngine::walkSong(int& loopOrder, int& loopRow, int& loopEnd) {
  loopOrder=0;
  loopRow=0;
  loopEnd=-1;
  int nextOrder=-1;
  int nextRow=0;
  int effectVal=0;
  DivPattern* pat[DIV_MAX_CHANS];
  for (int i=0; i<song.ordersLen; i++) {
    for (int j=0; j<chans; j++) {
      pat[j]=song.pat[j].getPattern(song.orders.ord[j][i],false);
    }
    for (int j=nextRow; j<song.patLen; j++) {
      nextRow=0;
      for (int k=0; k<chans; k++) {
        for (int l=0; l<song.pat[k].effectRows; l++) {
          effectVal=pat[k]->data[j][5+(l<<1)];
          if (effectVal<0) effectVal=0;
          if (pat[k]->data[j][4+(l<<1)]==0x0d) {
            if (nextOrder==-1 && i<song.ordersLen-1) {
              nextOrder=i+1;
              nextRow=effectVal;
            }
          } else if (pat[k]->data[j][4+(l<<1)]==0x0b) {
            if (nextOrder==-1) {
              nextOrder=effectVal;
              nextRow=0;
            }
          }
        }
      }
      if (nextOrder!=-1) {
        if (nextOrder<=i) {
          loopOrder=nextOrder;
          loopRow=nextRow;
          loopEnd=i;
          return;
        }
        i=nextOrder-1;
        nextOrder=-1;
        break;
      }
    }
  }
}

SafeWriter* DivEngine::saveVGM(bool* sysToExport, bool loop) {
  stop();
  setOrder(0);
  isBusy.lock();
  // determine loop point
  int loopOrder=0;
  int loopRow=0;
  int loopEnd=0;
  walkSong(loopOrder,loopRow,loopEnd);
  logI("loop point: %d %d\n",loopOrder,loopRow);

  curOrder=0;
  freelance=false;
  playing=false;
  extValuePresent=false;
  remainingLoops=-1;

  // play the song ourselves
  bool done=false;
  int writeCount=0;

  int gd3Off=0;

  int hasSN=0;
  int snNoiseConfig=9;
  int snNoiseSize=16;
  int snFlags=0;
  int hasOPLL=0;
  int hasOPN2=0;
  int hasOPM=0;
  int hasSegaPCM=0;
  int segaPCMOffset=0xf8000d;
  int hasRFC=0;
  int hasOPN=0;
  int hasOPNA=0;
  int hasOPNB=0;
  int hasOPL2=0;
  int hasOPL=0;
  int hasY8950=0;
  int hasOPL3=0;
  int hasOPL4=0;
  int hasOPX=0;
  int hasZ280=0;
  int hasRFC1=0;
  int hasPWM=0;
  int hasAY=0;
  int ayConfig=0;
  int ayFlags=0;
  int hasGB=0;
  int hasNES=0;
  int hasMultiPCM=0;
  int hasuPD7759=0;
  int hasOKIM6258=0;
  int hasK054539=0;
  int hasOKIM6295=0;
  int hasK051649=0;
  int hasPCE=0;
  int hasNamco=0;
  int hasK053260=0;
  int hasPOKEY=0;
  int hasQSound=0;
  int hasSCSP=0;
  int hasSwan=0;
  int hasVSU=0;
  int hasSAA=0;
  int hasES5503=0;
  int hasES5505=0;
  int hasX1=0;
  int hasC352=0;
  int hasGA20=0;

  int howManyChips=0;

  int loopPos=-1;
  int loopTick=-1;

  SafeWriter* w=new SafeWriter;
  w->init();

  // write header
  w->write("Vgm ",4);
  w->writeI(0); // will be written later
  w->writeI(0x171); // VGM 1.71

  bool willExport[32];
  bool isSecond[32];
  int streamIDs[32];
  double loopTimer[DIV_MAX_CHANS];
  double loopFreq[DIV_MAX_CHANS];
  int loopSample[DIV_MAX_CHANS];

  for (int i=0; i<DIV_MAX_CHANS; i++) {
    loopTimer[i]=0;
    loopFreq[i]=0;
    loopSample[i]=-1;
  }

  bool writeDACSamples=false;
  bool writeNESSamples=false;
  bool writePCESamples=false;
  bool writeADPCM=false;
  bool writeSegaPCM=false;

  for (int i=0; i<song.systemLen; i++) {
    willExport[i]=false;
    isSecond[i]=false;
    streamIDs[i]=0;
    if (sysToExport!=NULL) {
      if (!sysToExport[i]) continue;
    }
    switch (song.system[i]) {
      case DIV_SYSTEM_GENESIS:
      case DIV_SYSTEM_GENESIS_EXT:
        writeDACSamples=true;
        if (!hasOPN2) {
          hasOPN2=7670454;
          willExport[i]=true;
        } else if (!(hasOPN2&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=true;
          hasOPN2|=0x40000000;
          howManyChips++;
        }
        if (!hasSN) {
          hasSN=3579545;
          willExport[i]=true;
        } else if (!(hasSN&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=true;
          hasSN|=0x40000000;
          howManyChips++;
        }
        break;
      case DIV_SYSTEM_SMS:
        if (!hasSN) {
          hasSN=3579545;
          willExport[i]=true;
        } else if (!(hasSN&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=true;
          hasSN|=0x40000000;
          howManyChips++;
        }
        break;
      case DIV_SYSTEM_GB:
        if (!hasGB) {
          hasGB=4194304;
          willExport[i]=true;
        } else if (!(hasGB&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=true;
          hasGB|=0x40000000;
          howManyChips++;
        }
        break;
      case DIV_SYSTEM_PCE:
        if (!hasPCE) {
          hasPCE=3579545;
          willExport[i]=true;
          writePCESamples=true;
        } else if (!(hasPCE&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=true;
          hasPCE|=0x40000000;
          howManyChips++;
        }
        break;
      case DIV_SYSTEM_NES:
        if (!hasNES) {
          hasNES=1789773;
          willExport[i]=true;
          writeNESSamples=true;
        } else if (!(hasNES&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=true;
          hasNES|=0x40000000;
          howManyChips++;
        }
        break;
      case DIV_SYSTEM_ARCADE:
        if (!hasOPM) {
          hasOPM=3579545;
          willExport[i]=true;
        } else if (!(hasOPM&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=true;
          hasOPM|=0x40000000;
          howManyChips++;
        }
        if (!hasSegaPCM) {
          hasSegaPCM=4000000;
          willExport[i]=true;
          writeSegaPCM=true;
        } else if (!(hasSegaPCM&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=true;
          hasSegaPCM|=0x40000000;
          howManyChips++;
        }
        break;
      case DIV_SYSTEM_YM2610:
      case DIV_SYSTEM_YM2610_EXT:
        if (!hasOPNB) {
          hasOPNB=8000000;
          willExport[i]=true;
          writeADPCM=true;
        } else if (!(hasOPNB&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=true;
          hasOPNB|=0x40000000;
          howManyChips++;
        }
        break;
      case DIV_SYSTEM_AY8910:
      case DIV_SYSTEM_AY8930:
        if (!hasAY) {
          hasAY=1789773;
          ayConfig=(song.system[i]==DIV_SYSTEM_AY8930)?3:0;
          ayFlags=1;
          willExport[i]=true;
        } else if (!(hasAY&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=true;
          hasAY|=0x40000000;
          howManyChips++;
        }
        break;
      case DIV_SYSTEM_SAA1099:
        if (!hasSAA) {
          hasSAA=8000000;
          willExport[i]=true;
        } else if (!(hasSAA&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=true;
          hasSAA|=0x40000000;
          howManyChips++;
        }
        break;
      case DIV_SYSTEM_YM2612:
        if (!hasOPN2) {
          hasOPN2=7670454;
          willExport[i]=true;
          writeDACSamples=true;
        } else if (!(hasOPN2&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=true;
          hasOPN2|=0x40000000;
          howManyChips++;
        }
        break;
      case DIV_SYSTEM_YM2151:
        if (!hasOPM) {
          hasOPM=3579545;
          willExport[i]=true;
        } else if (!(hasOPM&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=true;
          hasOPM|=0x40000000;
          howManyChips++;
        }
        break;
      default:
        break;
    }
    if (willExport[i]) {
      disCont[i].dispatch->toggleRegisterDump(true);
    }
  }

  //bool wantsExtraHeader=false;
  /*for (int i=0; i<song.systemLen; i++) {
    if (isSecond[i]) {
      wantsExtraHeader=true;
      break;
    }
  }*/

  // write chips and stuff
  w->writeI(hasSN);
  w->writeI(hasOPLL);
  w->writeI(0);
  w->writeI(0); // length. will be written later
  w->writeI(0); // loop. will be written later
  w->writeI(0); // loop length. why is this necessary?
  w->writeI(0); // tick rate
  w->writeS(snNoiseConfig);
  w->writeC(snNoiseSize);
  w->writeC(snFlags);
  w->writeI(hasOPN2);
  w->writeI(hasOPM);
  w->writeI(0); // data pointer. will be written later
  w->writeI(hasSegaPCM);
  w->writeI(segaPCMOffset);
  w->writeI(hasRFC);
  w->writeI(hasOPN);
  w->writeI(hasOPNA);
  w->writeI(hasOPNB);
  w->writeI(hasOPL2);
  w->writeI(hasOPL);
  w->writeI(hasY8950);
  w->writeI(hasOPL3);
  w->writeI(hasOPL4);
  w->writeI(hasOPX);
  w->writeI(hasZ280);
  w->writeI(hasRFC1);
  w->writeI(hasPWM);
  w->writeI(hasAY);
  w->writeC(ayConfig);
  w->writeC(ayFlags);
  w->writeC(ayFlags); // OPN
  w->writeC(ayFlags); // OPNA
  w->writeC(0); // volume
  w->writeC(0); // reserved
  w->writeC(0); // loop count
  w->writeC(0); // loop modifier
  w->writeI(hasGB);
  w->writeI(hasNES);
  w->writeI(hasMultiPCM);
  w->writeI(hasuPD7759);
  w->writeI(hasOKIM6258);
  w->writeC(0); // flags
  w->writeC(0); // K flags
  w->writeC(0); // C140 chip type
  w->writeC(0); // reserved
  w->writeI(hasOKIM6295);
  w->writeI(hasK051649);
  w->writeI(hasK054539);
  w->writeI(hasPCE);
  w->writeI(hasNamco);
  w->writeI(hasK053260);
  w->writeI(hasPOKEY);
  w->writeI(hasQSound);
  w->writeI(hasSCSP);
  w->writeI(0); // extra header
  w->writeI(hasSwan);
  w->writeI(hasVSU);
  w->writeI(hasSAA);
  w->writeI(hasES5503);
  w->writeI(hasES5505);
  w->writeC(0); // 5503 chans
  w->writeC(0); // 5505 chans
  w->writeC(0); // C352 clock divider
  w->writeC(0); // reserved
  w->writeI(hasX1);
  w->writeI(hasC352);
  w->writeI(hasGA20);
  for (int i=0; i<7; i++) { // reserved
    w->writeI(0);
  }

  /* TODO
  unsigned int exHeaderOff=w->tell();
  if (wantsExtraHeader) {
    w->writeI(4);
    w->writeI(4);

    // write clocks
    w->writeC(howManyChips);
  }*/

  unsigned int songOff=w->tell();

  // write samples
  unsigned int sampleSeek=0;
  for (int i=0; i<song.sampleLen; i++) {
    DivSample* sample=song.sample[i];
    logI("setting seek to %d\n",sampleSeek);
    sample->rendOffContiguous=sampleSeek;
    sampleSeek+=sample->rendLength;
  }

  if (writeDACSamples) for (int i=0; i<song.sampleLen; i++) {
    DivSample* sample=song.sample[i];
    w->writeC(0x67);
    w->writeC(0x66);
    w->writeC(0);
    w->writeI(sample->rendLength);
    if (sample->depth==8) {
      for (unsigned int j=0; j<sample->rendLength; j++) {
        w->writeC((unsigned char)sample->rendData[j]+0x80);
      }
    } else {
      for (unsigned int j=0; j<sample->rendLength; j++) {
        w->writeC(((unsigned short)sample->rendData[j]+0x8000)>>8);
      }
    }
  }

  if (writeNESSamples) for (int i=0; i<song.sampleLen; i++) {
    DivSample* sample=song.sample[i];
    w->writeC(0x67);
    w->writeC(0x66);
    w->writeC(7);
    w->writeI(sample->rendLength);
    if (sample->depth==8) {
      for (unsigned int j=0; j<sample->rendLength; j++) {
        w->writeC(((unsigned char)sample->rendData[j]+0x80)>>1);
      }
    } else {
      for (unsigned int j=0; j<sample->rendLength; j++) {
        w->writeC(((unsigned short)sample->rendData[j]+0x8000)>>9);
      }
    }
  }

  if (writePCESamples) for (int i=0; i<song.sampleLen; i++) {
    DivSample* sample=song.sample[i];
    w->writeC(0x67);
    w->writeC(0x66);
    w->writeC(5);
    w->writeI(sample->rendLength);
    if (sample->depth==8) {
      for (unsigned int j=0; j<sample->rendLength; j++) {
        w->writeC(((unsigned char)sample->rendData[j]+0x80)>>3);
      }
    } else {
      for (unsigned int j=0; j<sample->rendLength; j++) {
        w->writeC(((unsigned short)sample->rendData[j]+0x8000)>>11);
      }
    }
  }

  if (writeSegaPCM) {
    unsigned char* pcmMem=new unsigned char[16777216];

    size_t memPos=0;
    for (int i=0; i<song.sampleLen; i++) {
      DivSample* sample=song.sample[i];
      if ((memPos&0xff0000)!=((memPos+sample->rendLength)&0xff0000)) {
        memPos=(memPos+0xffff)&0xff0000;
      }
      if (memPos>=16777216) break;
      sample->rendOffP=memPos;
      unsigned int alignedSize=(sample->rendLength+0xff)&(~0xff);
      unsigned int readPos=0;
      if (alignedSize>65536) alignedSize=65536;
      if (sample->depth==8) {
        for (unsigned int j=0; j<alignedSize; j++) {
          if (readPos>=sample->rendLength) {
            if (sample->loopStart>=0 && sample->loopStart<(int)sample->rendLength) {
              readPos=sample->loopStart;
              pcmMem[memPos++]=((unsigned char)sample->rendData[readPos]+0x80);
            } else {
              pcmMem[memPos++]=0x80;
            }
          } else {
            pcmMem[memPos++]=((unsigned char)sample->rendData[readPos]+0x80);
          }
          readPos++;
          if (memPos>=16777216) break;
        }
        sample->loopOffP=readPos-sample->loopStart;
      } else {
        for (unsigned int j=0; j<alignedSize; j++) {
          if (readPos>=sample->rendLength) {
            if (sample->loopStart>=0 && sample->loopStart<(int)sample->rendLength) {
              readPos=sample->loopStart;
              pcmMem[memPos++]=(((unsigned short)sample->rendData[readPos]+0x8000)>>8);
            } else {
              pcmMem[memPos++]=0x80;
            }
          } else {
            pcmMem[memPos++]=(((unsigned short)sample->rendData[readPos]+0x8000)>>8);
          }
          readPos++;
          if (memPos>=16777216) break;
        }
        sample->loopOffP=readPos-sample->loopStart;
      }
      if (memPos>=16777216) break;
    }

    w->writeC(0x67);
    w->writeC(0x66);
    w->writeC(0x80);
    w->writeI(memPos+8);
    w->writeI(memPos);
    w->writeI(0);
    w->write(pcmMem,memPos);

    delete[] pcmMem;
  }

  if (writeADPCM && adpcmMemLen>0) {
    w->writeC(0x67);
    w->writeC(0x66);
    w->writeC(0x82);
    w->writeI(adpcmMemLen+8);
    w->writeI(adpcmMemLen);
    w->writeI(0);
    w->write(adpcmMem,adpcmMemLen);
  }

  // initialize streams
  int streamID=0;
  for (int i=0; i<song.systemLen; i++) {
    if (!willExport[i]) continue;
    streamIDs[i]=streamID;
    switch (song.system[i]) {
      case DIV_SYSTEM_GENESIS:
      case DIV_SYSTEM_GENESIS_EXT:
        w->writeC(0x90);
        w->writeC(streamID);
        w->writeC(0x02);
        w->writeC(0); // port
        w->writeC(0x2a); // DAC

        w->writeC(0x91);
        w->writeC(streamID);
        w->writeC(0);
        w->writeC(1);
        w->writeC(0);

        w->writeC(0x92);
        w->writeC(streamID);
        w->writeI(32000); // default
        streamID++;
        break;
      case DIV_SYSTEM_NES:
        w->writeC(0x90);
        w->writeC(streamID);
        w->writeC(20);
        w->writeC(0); // port
        w->writeC(0x11); // DAC

        w->writeC(0x91);
        w->writeC(streamID);
        w->writeC(7);
        w->writeC(1);
        w->writeC(0);

        w->writeC(0x92);
        w->writeC(streamID);
        w->writeI(32000); // default
        streamID++;
        break;
      case DIV_SYSTEM_PCE:
        for (int j=0; j<6; j++) {
          w->writeC(0x90);
          w->writeC(streamID);
          w->writeC(27);
          w->writeC(j); // port
          w->writeC(0x06); // select+DAC

          w->writeC(0x91);
          w->writeC(streamID);
          w->writeC(5);
          w->writeC(1);
          w->writeC(0);

          w->writeC(0x92);
          w->writeC(streamID);
          w->writeI(16000); // default
          streamID++;
        }
        break;
      default:
        break;
    }
  }

  // write song data
  playSub(false);
  size_t tickCount=0;
  bool writeLoop=false;
  while (!done) {
    if (loopPos==-1) {
      if (loopOrder==curOrder && loopRow==curRow && ticks==1) {
        writeLoop=true;
      }
    }
    if (nextTick()) {
      done=true;
      if (!loop) {
        for (int i=0; i<song.systemLen; i++) {
          disCont[i].dispatch->getRegisterWrites().clear();
        }
        break;
      }
      // stop all streams
      for (int i=0; i<streamID; i++) {
        w->writeC(0x94);
        w->writeC(i);
        loopSample[i]=-1;
      }
    }
    // get register dumps
    for (int i=0; i<song.systemLen; i++) {
      std::vector<DivRegWrite>& writes=disCont[i].dispatch->getRegisterWrites();
      for (DivRegWrite& j: writes) {
        performVGMWrite(w,song.system[i],j,streamIDs[i],loopTimer,loopFreq,loopSample,isSecond[i]);
        writeCount++;
      }
      writes.clear();
    }
    // check whether we need to loop
    int totalWait=cycles>>MASTER_CLOCK_PREC;
    for (int i=0; i<streamID; i++) {
      if (loopSample[i]>=0) {
        loopTimer[i]-=(loopFreq[i]/44100.0)*(double)totalWait;
      }
    }
    bool haveNegatives=false;
    for (int i=0; i<streamID; i++) {
      if (loopSample[i]>=0) {
        if (loopTimer[i]<0) {
          haveNegatives=true;
        }
      }
    }
    while (haveNegatives) {
      // finish all negatives
      int nextToTouch=-1;
      for (int i=0; i<streamID; i++) {
        if (loopSample[i]>=0) {
          if (loopTimer[i]<0) {
            if (nextToTouch>=0) {
              if (loopTimer[nextToTouch]>loopTimer[i]) nextToTouch=i;
            } else {
              nextToTouch=i;
            }
          }
        }
      }
      if (nextToTouch>=0) {
        double waitTime=totalWait+(loopTimer[nextToTouch]*(44100.0/loopFreq[nextToTouch]));
        if (waitTime>0) {
          w->writeC(0x61);
          w->writeS(waitTime);
          printf("wait is: %f\n",waitTime);
          totalWait-=waitTime;
          tickCount+=waitTime;
        }
        if (loopSample[nextToTouch]<song.sampleLen) {
          DivSample* sample=song.sample[loopSample[nextToTouch]];
          // insert loop
          if (sample->loopStart<(int)sample->rendLength) {
            w->writeC(0x93);
            w->writeC(nextToTouch);
            w->writeI(sample->rendOffContiguous+sample->loopStart);
            w->writeC(0x81);
            w->writeI(sample->rendLength-sample->loopStart);
          }
        }
        loopSample[nextToTouch]=-1;
      } else {
        haveNegatives=false;
      }
    }
    // write wait
    if (totalWait>0) {
      w->writeC(0x61);
      w->writeS(totalWait);
      tickCount+=totalWait;
    }
    if (writeLoop) {
      writeLoop=false;
      loopPos=w->tell();
      loopTick=tickCount;
    }
  }
  // end of song
  w->writeC(0x66);

  for (int i=0; i<song.systemLen; i++) {
    disCont[i].dispatch->toggleRegisterDump(false);
  }

  // write GD3 tag
  gd3Off=w->tell();
  w->write("Gd3 ",4);
  w->writeI(0x100);
  w->writeI(0); // length. will be written later

  WString ws;
  ws=utf8To16(song.name.c_str());
  w->writeWString(ws,false); // name
  w->writeS(0); // japanese name
  w->writeS(0); // game name
  w->writeS(0); // japanese game name
  if (song.systemLen>1) {
    ws=L"Multiple Systems";
  } else {
    ws=utf8To16(getSystemName(song.system[0]));
  }
  w->writeWString(ws,false); // system name
  if (song.systemLen>1) {
    ws=L"複数システム";
  } else {
    ws=utf8To16(getSystemNameJ(song.system[0]));
  }
  w->writeWString(ws,false); // japanese system name
  ws=utf8To16(song.author.c_str());
  w->writeWString(ws,false); // author name
  w->writeS(0); // japanese author name
  w->writeS(0); // date
  w->writeWString(L"Furnace Tracker",false); // ripper
  w->writeS(0); // notes

  int gd3Len=w->tell()-gd3Off-12;

  w->seek(gd3Off+8,SEEK_SET);
  w->writeI(gd3Len);

  // finish file
  size_t len=w->size()-4;
  w->seek(4,SEEK_SET);
  w->writeI(len);
  w->seek(0x14,SEEK_SET);
  w->writeI(gd3Off-0x14);
  w->writeI(tickCount);
  if (loop) {
    w->writeI(loopPos-0x1c);
    w->writeI(tickCount-loopTick-1);
  } else {
    w->writeI(0);
    w->writeI(0);
  }
  w->seek(0x34,SEEK_SET);
  w->writeI(songOff-0x34);
  /*if (wantsExtraHeader) {
    w->seek(0xbc,SEEK_SET);
    w->writeI(exHeaderOff-0xbc);
  }*/

  remainingLoops=-1;
  playing=false;
  freelance=false;
  extValuePresent=false;

  logI("%d register writes total.\n",writeCount);

  isBusy.unlock();
  return w;
}

void _runExportThread(DivEngine* caller) {
  caller->runExportThread();
}

bool DivEngine::isExporting() {
  return exporting;
}

#define EXPORT_BUFSIZE 2048

void DivEngine::runExportThread() {
  switch (exportMode) {
    case DIV_EXPORT_MODE_ONE: {
      SNDFILE* sf;
      SF_INFO si;
      si.samplerate=got.rate;
      si.channels=2;
      si.format=SF_FORMAT_WAV|SF_FORMAT_PCM_16;

      sf=sf_open(exportPath.c_str(),SFM_WRITE,&si);
      if (sf==NULL) {
        logE("could not open file for writing! (%s)\n",sf_strerror(NULL));
        exporting=false;
        return;
      }

      float* outBuf[3];
      outBuf[0]=new float[EXPORT_BUFSIZE];
      outBuf[1]=new float[EXPORT_BUFSIZE];
      outBuf[2]=new float[EXPORT_BUFSIZE*2];

      // take control of audio output
      deinitAudioBackend();
      playSub(false);

      logI("rendering to file...\n");

      while (playing) {
        nextBuf(NULL,outBuf,0,2,EXPORT_BUFSIZE);
        for (int i=0; i<EXPORT_BUFSIZE; i++) {
          outBuf[2][i<<1]=MAX(-1.0f,MIN(1.0f,outBuf[0][i]));
          outBuf[2][1+(i<<1)]=MAX(-1.0f,MIN(1.0f,outBuf[1][i]));
        }
        if (totalProcessed>EXPORT_BUFSIZE) {
          logE("error: total processed is bigger than export bufsize! %d>%d\n",totalProcessed,EXPORT_BUFSIZE);
        }
        if (sf_writef_float(sf,outBuf[2],totalProcessed)!=(int)totalProcessed) {
          logE("error: failed to write entire buffer!\n");
          break;
        }
      }

      delete[] outBuf[0];
      delete[] outBuf[1];
      delete[] outBuf[2];

      if (sf_close(sf)!=0) {
        logE("could not close audio file!\n");
      }
      exporting=false;

      if (initAudioBackend()) {
        for (int i=0; i<song.systemLen; i++) {
          disCont[i].setRates(got.rate);
          disCont[i].setQuality(lowQuality);
        }
        if (!output->setRun(true)) {
          logE("error while activating audio!\n");
        }
      }
      logI("done!\n");
      break;
    }
    case DIV_EXPORT_MODE_MANY_SYS: {
      SNDFILE* sf[32];
      SF_INFO si[32];
      String fname[32];
      for (int i=0; i<song.systemLen; i++) {
        sf[i]=NULL;
        si[i].samplerate=got.rate;
        if (disCont[i].dispatch->isStereo()) {
          si[i].channels=2;
        } else {
          si[i].channels=1;
        }
        si[i].format=SF_FORMAT_WAV|SF_FORMAT_PCM_16;
      }

      for (int i=0; i<song.systemLen; i++) {
        fname[i]=fmt::sprintf("%s_s%d.wav",exportPath,i+1);
        logI("- %s\n",fname[i].c_str());
        sf[i]=sf_open(fname[i].c_str(),SFM_WRITE,&si[i]);
        if (sf[i]==NULL) {
          logE("could not open file for writing! (%s)\n",sf_strerror(NULL));
          for (int j=0; j<i; j++) {
            sf_close(sf[i]);
          }
          return;
        }
      }

      float* outBuf[2];
      outBuf[0]=new float[EXPORT_BUFSIZE];
      outBuf[1]=new float[EXPORT_BUFSIZE];
      short* sysBuf=new short[EXPORT_BUFSIZE*2];

      // take control of audio output
      deinitAudioBackend();
      playSub(false);

      logI("rendering to files...\n");

      while (playing) {
        nextBuf(NULL,outBuf,0,2,EXPORT_BUFSIZE);
        for (int i=0; i<song.systemLen; i++) {
          for (int j=0; j<EXPORT_BUFSIZE; j++) {
            if (!disCont[i].dispatch->isStereo()) {
              sysBuf[j]=disCont[i].bbOut[0][j];
            } else {
              sysBuf[j<<1]=disCont[i].bbOut[0][j];
              sysBuf[1+(j<<1)]=disCont[i].bbOut[1][j];
            }
          }
          if (totalProcessed>EXPORT_BUFSIZE) {
            logE("error: total processed is bigger than export bufsize! (%d) %d>%d\n",i,totalProcessed,EXPORT_BUFSIZE);
          }
          if (sf_writef_short(sf[i],sysBuf,totalProcessed)!=(int)totalProcessed) {
            logE("error: failed to write entire buffer! (%d)\n",i);
            break;
          }
        }
      }

      delete[] outBuf[0];
      delete[] outBuf[1];
      delete[] sysBuf;

      for (int i=0; i<song.systemLen; i++) {
        if (sf_close(sf[i])!=0) {
          logE("could not close audio file!\n");
        }
      }
      exporting=false;

      if (initAudioBackend()) {
        for (int i=0; i<song.systemLen; i++) {
          disCont[i].setRates(got.rate);
          disCont[i].setQuality(lowQuality);
        }
        if (!output->setRun(true)) {
          logE("error while activating audio!\n");
        }
      }
      logI("done!\n");
      break;
    }
    case DIV_EXPORT_MODE_MANY_CHAN: {
      // take control of audio output
      deinitAudioBackend();

      float* outBuf[3];
      outBuf[0]=new float[EXPORT_BUFSIZE];
      outBuf[1]=new float[EXPORT_BUFSIZE];
      outBuf[2]=new float[EXPORT_BUFSIZE*2];
      int loopCount=remainingLoops;

      logI("rendering to files...\n");
      
      for (int i=0; i<chans; i++) {
        SNDFILE* sf;
        SF_INFO si;
        String fname=fmt::sprintf("%s_c%d.wav",exportPath,i+1);
        logI("- %s\n",fname.c_str());
        si.samplerate=got.rate;
        si.channels=2;
        si.format=SF_FORMAT_WAV|SF_FORMAT_PCM_16;

        sf=sf_open(fname.c_str(),SFM_WRITE,&si);
        if (sf==NULL) {
          logE("could not open file for writing! (%s)\n",sf_strerror(NULL));
          break;
        }

        for (int j=0; j<chans; j++) {
          bool mute=(j!=i);
          isMuted[j]=mute;
          if (disCont[dispatchOfChan[j]].dispatch!=NULL) {
            disCont[dispatchOfChan[j]].dispatch->muteChannel(dispatchChanOfChan[j],isMuted[j]);
          }
        }
        
        curOrder=0;
        remainingLoops=loopCount;
        playSub(false);

        while (playing) {
          nextBuf(NULL,outBuf,0,2,EXPORT_BUFSIZE);
          for (int j=0; j<EXPORT_BUFSIZE; j++) {
            outBuf[2][j<<1]=MAX(-1.0f,MIN(1.0f,outBuf[0][j]));
            outBuf[2][1+(j<<1)]=MAX(-1.0f,MIN(1.0f,outBuf[1][j]));
          }
          if (totalProcessed>EXPORT_BUFSIZE) {
            logE("error: total processed is bigger than export bufsize! %d>%d\n",totalProcessed,EXPORT_BUFSIZE);
          }
          if (sf_writef_float(sf,outBuf[2],totalProcessed)!=(int)totalProcessed) {
            logE("error: failed to write entire buffer!\n");
            break;
          }
        }

        if (sf_close(sf)!=0) {
          logE("could not close audio file!\n");
        }
      }
      exporting=false;

      delete[] outBuf[0];
      delete[] outBuf[1];
      delete[] outBuf[2];

      for (int i=0; i<chans; i++) {
        isMuted[i]=false;
        if (disCont[dispatchOfChan[i]].dispatch!=NULL) {
          disCont[dispatchOfChan[i]].dispatch->muteChannel(dispatchChanOfChan[i],false);
        }
      }

      if (initAudioBackend()) {
        for (int i=0; i<song.systemLen; i++) {
          disCont[i].setRates(got.rate);
          disCont[i].setQuality(lowQuality);
        }
        if (!output->setRun(true)) {
          logE("error while activating audio!\n");
        }
      }
      logI("done!\n");
      break;
    }
  }
}

bool DivEngine::saveAudio(const char* path, int loops, DivAudioExportModes mode) {
  exportPath=path;
  exportMode=mode;
  exporting=true;
  stop();
  setOrder(0);
  remainingLoops=loops;
  exportThread=new std::thread(_runExportThread,this);
  return true;
}

void DivEngine::waitAudioFile() {
  if (exportThread!=NULL) {
    exportThread->join();
  }
}

bool DivEngine::haltAudioFile() {
  stop();
  return true;
}

void DivEngine::notifyInsChange(int ins) {
  isBusy.lock();
  for (int i=0; i<song.systemLen; i++) {
    disCont[i].dispatch->notifyInsChange(ins);
  }
  isBusy.unlock();
}

void DivEngine::notifyWaveChange(int wave) {
  isBusy.lock();
  for (int i=0; i<song.systemLen; i++) {
    disCont[i].dispatch->notifyWaveChange(wave);
  }
  isBusy.unlock();
}

#ifdef _WIN32
#define CONFIG_FILE "\\furnace.cfg"
#else
#define CONFIG_FILE "/furnace.cfg"
#endif

bool DivEngine::saveConf() {
  configFile=configPath+String(CONFIG_FILE);
  FILE* f=ps_fopen(configFile.c_str(),"wb");
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
  FILE* f=ps_fopen(configFile.c_str(),"rb");
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
    if (adpcmLen>1048576) adpcmLen=1048576;
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

  // step 3: allocate ADPCM samples
  if (adpcmMem==NULL) adpcmMem=new unsigned char[16777216];

  size_t memPos=0;
  for (int i=0; i<song.sampleLen; i++) {
    DivSample* s=song.sample[i];
    if ((memPos&0xf00000)!=((memPos+s->adpcmRendLength)&0xf00000)) {
      memPos=(memPos+0xfffff)&0xf00000;
    }
    if (memPos>=16777216) {
      logW("out of ADPCM memory for sample %d!\n",i);
      break;
    }
    if (memPos+s->adpcmRendLength>=16777216) {
      memcpy(adpcmMem+memPos,s->adpcmRendData,16777216-memPos);
      logW("out of ADPCM memory for sample %d!\n",i);
    } else {
      memcpy(adpcmMem+memPos,s->adpcmRendData,s->adpcmRendLength);
    }
    s->rendOff=memPos;
    memPos+=s->adpcmRendLength;
  }
  adpcmMemLen=memPos+256;
}

void DivEngine::createNew() {
  DivSystem sys=song.system[0];
  quitDispatch();
  isBusy.lock();
  song.unload();
  song=DivSong();
  song.system[0]=sys;
  recalcChans();
  renderSamples();
  isBusy.unlock();
  initDispatch();
  isBusy.lock();
  reset();
  isBusy.unlock();
}

void DivEngine::changeSystem(int index, DivSystem which) {
  quitDispatch();
  isBusy.lock();
  song.system[index]=which;
  recalcChans();
  isBusy.unlock();
  initDispatch();
  isBusy.lock();
  renderSamples();
  reset();
  isBusy.unlock();
}

bool DivEngine::addSystem(DivSystem which) {
  if (song.systemLen>32) {
    lastError="cannot add more than 32";
    return false;
  }
  // this was DIV_MAX_CHANS but I am setting it to 63 for now due to an ImGui limitation
  if (chans+getChannelCount(which)>63) {
    lastError="max number of total channels is 63";
    return false;
  }
  quitDispatch();
  isBusy.lock();
  song.system[song.systemLen++]=which;
  recalcChans();
  isBusy.unlock();
  initDispatch();
  isBusy.lock();
  renderSamples();
  reset();
  isBusy.unlock();
  return true;
}

bool DivEngine::removeSystem(int index) {
  if (song.systemLen<=1) {
    lastError="cannot remove the last one";
    return false;
  }
  if (index<0 || index>=song.systemLen) {
    lastError="invalid index";
    return false;
  }
  quitDispatch();
  isBusy.lock();
  song.system[index]=DIV_SYSTEM_NULL;
  song.systemLen--;
  for (int i=index; i<song.systemLen; i++) {
    song.system[i]=song.system[i+1];
  }
  recalcChans();
  isBusy.unlock();
  initDispatch();
  isBusy.lock();
  renderSamples();
  reset();
  isBusy.unlock();
  return true;
}

String DivEngine::getLastError() {
  return lastError;
}

String DivEngine::getWarnings() {
  return warnings;
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

DivChannelState* DivEngine::getChanState(int ch) {
  if (ch<0 || ch>=chans) return NULL;
  return &chan[ch];
}

void* DivEngine::getDispatchChanState(int ch) {
  if (ch<0 || ch>=chans) return NULL;
  return disCont[dispatchOfChan[ch]].dispatch->getChanState(dispatchChanOfChan[ch]);
}

void DivEngine::playSub(bool preserveDrift) {
  reset();
  if (preserveDrift && curOrder==0) return;
  bool oldRepeatPattern=repeatPattern;
  repeatPattern=false;
  int goal=curOrder;
  curOrder=0;
  curRow=0;
  int prevDrift;
  prevDrift=clockDrift;
  clockDrift=0;
  cycles=0;
  if (preserveDrift) {
    endOfSong=false;
  } else {
    ticks=1;
    totalTicks=0;
    totalSeconds=0;
    totalTicksR=0;
  }
  speedAB=false;
  playing=true;
  for (int i=0; i<song.systemLen; i++) disCont[i].dispatch->setSkipRegisterWrites(true);
  while (curOrder<goal) {
    if (nextTick(preserveDrift)) break;
  }
  for (int i=0; i<song.systemLen; i++) disCont[i].dispatch->setSkipRegisterWrites(false);
  if (goal>0) {
    for (int i=0; i<song.systemLen; i++) disCont[i].dispatch->forceIns();
  }
  repeatPattern=oldRepeatPattern;
  if (preserveDrift) {
    clockDrift=prevDrift;
  } else {
    clockDrift=0;
    cycles=0;
  }
  if (!preserveDrift) {
    ticks=1;
  }
}

int DivEngine::calcBaseFreq(double clock, double divider, int note, bool period) {
  double base=(period?(song.tuning*0.0625):song.tuning)*pow(2.0,(float)(note+3)/12.0);
  return period?
         round((clock/base)/divider):
         base*(divider/clock);
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
  remainingLoops=-1;
  isBusy.unlock();
}

void DivEngine::recalcChans() {
  chans=0;
  int chanIndex=0;
  for (int i=0; i<song.systemLen; i++) {
    int chanCount=getChannelCount(song.system[i]);
    chans+=chanCount;
    for (int j=0; j<chanCount; j++) {
      sysOfChan[chanIndex]=song.system[i];
      dispatchOfChan[chanIndex]=i;
      dispatchChanOfChan[chanIndex]=j;
      chanIndex++;
    }
  }
}

void DivEngine::reset() {
  for (int i=0; i<DIV_MAX_CHANS; i++) {
    chan[i]=DivChannelState();
    if (i<chans) chan[i].volMax=(disCont[dispatchOfChan[i]].dispatch->dispatch(DivCommand(DIV_CMD_GET_VOLMAX,dispatchChanOfChan[i]))<<8)|0xff;
    chan[i].volume=chan[i].volMax;
  }
  extValue=0;
  extValuePresent=0;
  speed1=song.speed1;
  speed2=song.speed2;
  nextSpeed=speed1;
  divider=60;
  if (song.customTempo) {
    divider=song.hz;
  } else {
    if (song.pal) {
      divider=60;
    } else {
      divider=50;
    }
  }
  globalPitch=0;
  for (int i=0; i<song.systemLen; i++) {
    disCont[i].dispatch->reset();
    disCont[i].clear();
  }
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
  switch (song.system[0]) {
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

void DivEngine::previewSample(int sample, int note) {
  isBusy.lock();
  if (sample<0 || sample>=(int)song.sample.size()) {
    sPreview.sample=-1;
    sPreview.pos=0;
    isBusy.unlock();
    return;
  }
  blip_clear(samp_bb);
  double rate=song.sample[sample]->rate;
  if (note>=0) {
    rate=(song.tuning*pow(2.0,(double)(note+3)/12.0));
    if (rate<=0) rate=song.sample[sample]->rate;
  }
  blip_set_rates(samp_bb,rate,got.rate);
  samp_prevSample=0;
  sPreview.pos=0;
  sPreview.sample=sample;
  sPreview.wave=-1;
  isBusy.unlock();
}

void DivEngine::stopSamplePreview() {
  isBusy.lock();
  sPreview.sample=-1;
  sPreview.pos=0;
  isBusy.unlock();
}

void DivEngine::previewWave(int wave, int note) {
  isBusy.lock();
  if (wave<0 || wave>=(int)song.wave.size()) {
    sPreview.wave=-1;
    sPreview.pos=0;
    isBusy.unlock();
    return;
  }
  if (song.wave[wave]->len<=0) {
    isBusy.unlock();
    return;
  }
  blip_clear(samp_bb);
  blip_set_rates(samp_bb,song.wave[wave]->len*((song.tuning*0.0625)*pow(2.0,(double)(note+3)/12.0)),got.rate);
  samp_prevSample=0;
  sPreview.pos=0;
  sPreview.sample=-1;
  sPreview.wave=wave;
  isBusy.unlock();
}

void DivEngine::stopWavePreview() {
  isBusy.lock();
  sPreview.wave=-1;
  sPreview.pos=0;
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

int DivEngine::getCurHz() {
  return divider;
}

int DivEngine::getTotalSeconds() {
  return totalSeconds;
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
      if (disCont[dispatchOfChan[i]].dispatch!=NULL) {
        disCont[dispatchOfChan[i]].dispatch->muteChannel(dispatchChanOfChan[i],isMuted[i]);
      }
    }
  } else {
    for (int i=0; i<chans; i++) {
      isMuted[i]=false;
      if (disCont[dispatchOfChan[i]].dispatch!=NULL) {
        disCont[dispatchOfChan[i]].dispatch->muteChannel(dispatchChanOfChan[i],isMuted[i]);
      }
    }
  }
  isBusy.unlock();
}

void DivEngine::muteChannel(int chan, bool mute) {
  isBusy.lock();
  isMuted[chan]=mute;
  if (disCont[dispatchOfChan[chan]].dispatch!=NULL) {
    disCont[dispatchOfChan[chan]].dispatch->muteChannel(dispatchChanOfChan[chan],isMuted[chan]);
  }
  isBusy.unlock();
}

int DivEngine::addInstrument(int refChan) {
  isBusy.lock();
  DivInstrument* ins=new DivInstrument;
  int insCount=(int)song.ins.size();
  ins->name=fmt::sprintf("Instrument %d",insCount);
  ins->type=getPreferInsType(refChan);
  song.ins.push_back(ins);
  song.insLen=insCount+1;
  isBusy.unlock();
  return insCount;
}

bool DivEngine::addInstrumentFromFile(const char *path) {
  warnings="";
  FILE* f=ps_fopen(path,"rb");
  if (f==NULL) {
    lastError=strerror(errno);
    return false;
  }
  unsigned char* buf;
  ssize_t len;
  if (fseek(f,0,SEEK_END)!=0) {
    lastError=strerror(errno);
    fclose(f);
    return false;
  }
  len=ftell(f);
  if (len<0) {
    lastError=strerror(errno);
    fclose(f);
    return false;
  }
  if (len==0) {
    lastError=strerror(errno);
    fclose(f);
    return false;
  }
  if (fseek(f,0,SEEK_SET)!=0) {
    lastError=strerror(errno);
    fclose(f);
    return false;
  }
  buf=new unsigned char[len];
  if (fread(buf,1,len,f)!=(size_t)len) {
    logW("did not read entire instrument file buffer!\n");
    lastError="did not read entire instrument file!";
    delete[] buf;
    return false;
  }
  fclose(f);

  SafeReader reader=SafeReader(buf,len);

  unsigned char magic[16];
  bool isFurnaceInstr=false;
  try {
    reader.read(magic,16);
    if (memcmp("-Furnace instr.-",magic,16)==0) {
      isFurnaceInstr=true;
    }
  } catch (EndOfFileException e) {
    reader.seek(0,SEEK_SET);
  }

  DivInstrument* ins=new DivInstrument;
  if (isFurnaceInstr) {
    try {
      short version=reader.readS();
      reader.readS(); // reserved

      if (version>DIV_ENGINE_VERSION) {
        warnings="this instrument is made with a more recent version of Furnace!";
      }

      unsigned int dataPtr=reader.readI();
      reader.seek(dataPtr,SEEK_SET);

      if (ins->readInsData(reader,version)!=DIV_DATA_SUCCESS) {
        lastError="invalid instrument header/data!";
        delete ins;
        delete[] buf;
        return false;
      }
    } catch (EndOfFileException e) {
      lastError="premature end of file";
      logE("premature end of file!\n");
      delete ins;
      delete[] buf;
      return false;
    }
  } else { // read as .dmp
    try {
      reader.seek(0,SEEK_SET);
      unsigned char version=reader.readC();

      if (version>11) {
        lastError="unknown sample version!";
        delete ins;
        delete[] buf;
        return false;
      }

      unsigned char sys=reader.readC();

      switch (sys) {
        case 1: // YMU759
          ins->type=DIV_INS_FM;
          break;
        case 2: // Genesis
          ins->type=DIV_INS_FM;
          break;
        case 3: // SMS
          ins->type=DIV_INS_STD;
          break;
        case 4: // Game Boy
          ins->type=DIV_INS_GB;
          break;
        case 5: // PC Engine
          ins->type=DIV_INS_PCE;
          break;
        case 6: // NES
          ins->type=DIV_INS_STD;
          break;
        case 7: case 0x17: // C64
          ins->type=DIV_INS_C64;
          break;
        case 8: // Arcade
          ins->type=DIV_INS_FM;
          break;
        default:
          lastError="unknown instrument type!";
          delete ins;
          delete[] buf;
          return false;
          break;
      }

      bool mode=reader.readC();
      if (mode==0 && ins->type==DIV_INS_FM) {
        ins->type=DIV_INS_STD;
      }

      if (mode) { // FM
        if (version<0x0a) ins->fm.ops=reader.readC();
        ins->fm.fms=reader.readC();
        ins->fm.fb=reader.readC();
        ins->fm.alg=reader.readC();
        if (sys!=1) ins->fm.ams=reader.readC();

        for (int j=0; j<ins->fm.ops; j++) {
          ins->fm.op[j].mult=reader.readC();
          ins->fm.op[j].tl=reader.readC();
          ins->fm.op[j].ar=reader.readC();
          ins->fm.op[j].dr=reader.readC();
          ins->fm.op[j].sl=reader.readC();
          ins->fm.op[j].rr=reader.readC();
          ins->fm.op[j].am=reader.readC();
          if (sys==1) { // YMU759
            ins->fm.op[j].ws=reader.readC();
            ins->fm.op[j].ksl=reader.readC();
            ins->fm.op[j].vib=reader.readC();
            ins->fm.op[j].egt=reader.readC();
            ins->fm.op[j].sus=reader.readC();
            ins->fm.op[j].ksr=reader.readC();
            ins->fm.op[j].dvb=reader.readC();
            ins->fm.op[j].dam=reader.readC();
          } else {
            ins->fm.op[j].rs=reader.readC();
            ins->fm.op[j].dt=reader.readC();
            ins->fm.op[j].dt2=ins->fm.op[j].dt>>4;
            ins->fm.op[j].dt&=15;
            ins->fm.op[j].d2r=reader.readC();
            ins->fm.op[j].ssgEnv=reader.readC();
          }
        }
      } else { // STD
        if (ins->type!=DIV_INS_GB) {
          ins->std.volMacroLen=reader.readC();
          for (int i=0; i<ins->std.volMacroLen; i++) {
            ins->std.volMacro[i]=reader.readI();
          }
          if (ins->std.volMacroLen>0) {
            ins->std.volMacroOpen=true;
            ins->std.volMacroLoop=reader.readC();
          } else {
            ins->std.volMacroOpen=false;
          }
        }

        ins->std.arpMacroLen=reader.readC();
        for (int i=0; i<ins->std.arpMacroLen; i++) {
          ins->std.arpMacro[i]=reader.readI();
        }
        if (ins->std.arpMacroLen>0) {
          ins->std.arpMacroOpen=true;
          ins->std.arpMacroLoop=reader.readC();
        } else {
          ins->std.arpMacroOpen=false;
        }
        ins->std.arpMacroMode=reader.readC();

        ins->std.dutyMacroLen=reader.readC();
        for (int i=0; i<ins->std.dutyMacroLen; i++) {
          ins->std.dutyMacro[i]=reader.readI();
        }
        if (ins->std.dutyMacroLen>0) {
          ins->std.dutyMacroOpen=true;
          ins->std.dutyMacroLoop=reader.readC();
        } else {
          ins->std.dutyMacroOpen=false;
        }

        ins->std.waveMacroLen=reader.readC();
        for (int i=0; i<ins->std.waveMacroLen; i++) {
          ins->std.waveMacro[i]=reader.readI();
        }
        if (ins->std.waveMacroLen>0) {
          ins->std.waveMacroOpen=true;
          ins->std.waveMacroLoop=reader.readC();
        } else {
          ins->std.waveMacroOpen=false;
        }

        if (ins->type==DIV_INS_C64) {
          ins->c64.triOn=reader.readC();
          ins->c64.sawOn=reader.readC();
          ins->c64.pulseOn=reader.readC();
          ins->c64.noiseOn=reader.readC();

          ins->c64.a=reader.readC();
          ins->c64.d=reader.readC();
          ins->c64.s=reader.readC();
          ins->c64.r=reader.readC();

          ins->c64.duty=(reader.readC()*4095)/100;

          ins->c64.ringMod=reader.readC();
          ins->c64.oscSync=reader.readC();
          ins->c64.toFilter=reader.readC();
          if (version<0x07) { // TODO: UNSURE
            ins->c64.volIsCutoff=reader.readI();
          } else {
            ins->c64.volIsCutoff=reader.readC();
          }
          ins->c64.initFilter=reader.readC();

          ins->c64.res=reader.readC();
          ins->c64.cut=(reader.readC()*2047)/100;
          ins->c64.hp=reader.readC();
          ins->c64.bp=reader.readC();
          ins->c64.lp=reader.readC();
          ins->c64.ch3off=reader.readC();
        }
        if (ins->type==DIV_INS_GB) {
          ins->gb.envVol=reader.readC();
          ins->gb.envDir=reader.readC();
          ins->gb.envLen=reader.readC();
          ins->gb.soundLen=reader.readC();
        }
      }
    } catch (EndOfFileException e) {
      lastError="premature end of file";
      logE("premature end of file!\n");
      delete ins;
      delete[] buf;
      return false;
    }
  }

  isBusy.lock();
  int insCount=(int)song.ins.size();
  song.ins.push_back(ins);
  song.insLen=insCount+1;
  isBusy.unlock();
  return true;
}

void DivEngine::delInstrument(int index) {
  isBusy.lock();
  if (index>=0 && index<(int)song.ins.size()) {
    for (int i=0; i<song.systemLen; i++) {
      disCont[i].dispatch->notifyInsDeletion(song.ins[index]);
    }
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

bool DivEngine::addWaveFromFile(const char* path) {
  FILE* f=ps_fopen(path,"rb");
  if (f==NULL) {
    return false;
  }
  unsigned char* buf;
  ssize_t len;
  if (fseek(f,0,SEEK_END)!=0) {
    fclose(f);
    return false;
  }
  len=ftell(f);
  if (len<0) {
    fclose(f);
    return false;
  }
  if (len==0) {
    fclose(f);
    return false;
  }
  if (fseek(f,0,SEEK_SET)!=0) {
    fclose(f);
    return false;
  }
  buf=new unsigned char[len];
  if (fread(buf,1,len,f)!=(size_t)len) {
    logW("did not read entire wavetable file buffer!\n");
    delete[] buf;
    return false;
  }
  fclose(f);

  SafeReader reader=SafeReader(buf,len);

  unsigned char magic[16];
  bool isFurnaceTable=false;
  try {
    reader.read(magic,16);
    if (memcmp("-Furnace waveta-",magic,16)==0) {
      isFurnaceTable=true;
    }
  } catch (EndOfFileException e) {
    reader.seek(0,SEEK_SET);
  }

  DivWavetable* wave=new DivWavetable;
  try {
    if (isFurnaceTable) {
      reader.seek(16,SEEK_SET);
      short version=reader.readS();
      reader.readS(); // reserved
      reader.seek(20,SEEK_SET);
      if (wave->readWaveData(reader,version)!=DIV_DATA_SUCCESS) {
        lastError="invalid wavetable header/data!";
        delete wave;
        delete[] buf;
        return false;
      }
    } else {
      try {
        // read as .dmw
        reader.seek(0,SEEK_SET);
        int len=reader.readI();
        wave->max=(unsigned char)reader.readC();
        if (reader.size()==(size_t)(len+5)) {
          // read as .dmw
          logI("reading .dmw...\n");
          if (len>256) len=256;
          for (int i=0; i<len; i++) {
            wave->data[i]=(unsigned char)reader.readC();
          }
        } else {
          // read as binary
          logI("reading binary...\n");
          len=reader.size();
          if (len>256) len=256;
          reader.seek(0,SEEK_SET);
          for (int i=0; i<len; i++) {
            wave->data[i]=(unsigned char)reader.readC();
            if (wave->max<wave->data[i]) wave->max=wave->data[i];
          }
          wave->len=len;
        }
      } catch (EndOfFileException e) {
        // read as binary
        len=reader.size();
        logI("reading binary for being too small...\n");
        if (len>256) len=256;
        reader.seek(0,SEEK_SET);
        for (int i=0; i<len; i++) {
          wave->data[i]=(unsigned char)reader.readC();
          if (wave->max<wave->data[i]) wave->max=wave->data[i];
        }
        wave->len=len;
      }
    }
  } catch (EndOfFileException e) {
    delete wave;
    delete[] buf;
    return false;
  }
  
  isBusy.lock();
  int waveCount=(int)song.wave.size();
  song.wave.push_back(wave);
  song.waveLen=waveCount+1;
  isBusy.unlock();
  return true;
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
      if (((si.format&SF_FORMAT_SUBMASK)==SF_FORMAT_PCM_U8)) {
        averaged+=buf[i+j]-0x80;
      } else {
        averaged+=buf[i+j];
      }
    }
    averaged/=si.channels;
    sample->data[index++]=averaged;
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
  unsigned char order[DIV_MAX_CHANS];
  if (song.ordersLen>=0x7e) return;
  isBusy.lock();
  if (duplicate) {
    for (int i=0; i<DIV_MAX_CHANS; i++) {
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
    for (int i=0; i<DIV_MAX_CHANS; i++) {
      song.orders.ord[i][song.ordersLen]=order[i];
    }
    song.ordersLen++;
  } else { // after current order
    for (int i=0; i<DIV_MAX_CHANS; i++) {
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
  for (int i=0; i<DIV_MAX_CHANS; i++) {
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
  for (int i=0; i<DIV_MAX_CHANS; i++) {
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
  for (int i=0; i<DIV_MAX_CHANS; i++) {
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

bool DivEngine::moveInsUp(int which) {
  if (which<1 || which>=(int)song.ins.size()) return false;
  isBusy.lock();
  DivInstrument* prev=song.ins[which];
  song.ins[which]=song.ins[which-1];
  song.ins[which-1]=prev;
  isBusy.unlock();
  return true;
}

bool DivEngine::moveWaveUp(int which) {
  if (which<1 || which>=(int)song.wave.size()) return false;
  isBusy.lock();
  DivWavetable* prev=song.wave[which];
  song.wave[which]=song.wave[which-1];
  song.wave[which-1]=prev;
  isBusy.unlock();
  return true;
}

bool DivEngine::moveSampleUp(int which) {
  if (which<1 || which>=(int)song.sample.size()) return false;
  isBusy.lock();
  DivSample* prev=song.sample[which];
  song.sample[which]=song.sample[which-1];
  song.sample[which-1]=prev;
  isBusy.unlock();
  return true;
}

bool DivEngine::moveInsDown(int which) {
  if (which<0 || which>=((int)song.ins.size())-1) return false;
  isBusy.lock();
  DivInstrument* prev=song.ins[which];
  song.ins[which]=song.ins[which+1];
  song.ins[which+1]=prev;
  isBusy.unlock();
  return true;
}

bool DivEngine::moveWaveDown(int which) {
  if (which<0 || which>=((int)song.wave.size())-1) return false;
  isBusy.lock();
  DivWavetable* prev=song.wave[which];
  song.wave[which]=song.wave[which+1];
  song.wave[which+1]=prev;
  isBusy.unlock();
  return true;
}

bool DivEngine::moveSampleDown(int which) {
  if (which<0 || which>=((int)song.sample.size())-1) return false;
  isBusy.lock();
  DivSample* prev=song.sample[which];
  song.sample[which]=song.sample[which+1];
  song.sample[which+1]=prev;
  isBusy.unlock();
  return true;
}

void DivEngine::noteOn(int chan, int ins, int note, int vol) {
  if (chan<0 || chan>=chans) return;
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
  if (chan<0 || chan>=chans) return;
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

void DivEngine::setSysFlags(int system, unsigned int flags) {
  isBusy.lock();
  song.systemFlags[system]=flags;
  disCont[system].dispatch->setFlags(song.systemFlags[system]);
  disCont[system].setRates(got.rate);
  isBusy.unlock();
}

void DivEngine::setSongRate(int hz, bool pal) {
  isBusy.lock();
  song.pal=!pal;
  song.hz=hz;
  song.customTempo=(song.hz!=50 && song.hz!=60);
  divider=60;
  if (song.customTempo) {
    divider=song.hz;
  } else {
    if (song.pal) {
      divider=60;
    } else {
      divider=50;
    }
  }
  isBusy.unlock();
}

void DivEngine::setAudio(DivAudioEngines which) {
  audioEngine=which;
}

void DivEngine::setView(DivStatusView which) {
  view=which;
}

bool DivEngine::getMetronome() {
  return metronome;
}

void DivEngine::setMetronome(bool enable) {
  metronome=enable;
  metroAmp=0;
}

void DivEngine::setConsoleMode(bool enable) {
  consoleMode=enable;
}

void DivEngine::switchMaster() {
  deinitAudioBackend();
  if (initAudioBackend()) {
    for (int i=0; i<song.systemLen; i++) {
      disCont[i].setRates(got.rate);
      disCont[i].setQuality(lowQuality);
    }
    if (!output->setRun(true)) {
      logE("error while activating audio!\n");
    }
  }
}

void DivEngine::initDispatch() {
  isBusy.lock();
  for (int i=0; i<song.systemLen; i++) {
    disCont[i].init(song.system[i],this,getChannelCount(song.system[i]),got.rate,song.systemFlags[i]);
    disCont[i].setRates(got.rate);
    disCont[i].setQuality(lowQuality);
  }
  recalcChans();
  isBusy.unlock();
}

void DivEngine::quitDispatch() {
  isBusy.lock();
  for (int i=0; i<song.systemLen; i++) {
    disCont[i].quit();
  }
  cycles=0;
  clockDrift=0;
  chans=0;
  playing=false;
  speedAB=false;
  endOfSong=false;
  ticks=0;
  curRow=0;
  curOrder=0;
  nextSpeed=3;
  changeOrd=-1;
  changePos=0;
  totalTicks=0;
  totalSeconds=0;
  totalTicksR=0;
  totalCmds=0;
  lastCmds=0;
  cmdsPerSecond=0;
  for (int i=0; i<DIV_MAX_CHANS; i++) {
    isMuted[i]=0;
  }
  isBusy.unlock();
}

#define CHECK_CONFIG_DIR_MAC() \
  configPath+="/Library/Application Support/Furnace"; \
  if (stat(configPath.c_str(),&st)<0) { \
    logI("creating config dir...\n"); \
    if (mkdir(configPath.c_str(),0755)<0) { \
      logW("could not make config dir! (%s)\n",strerror(errno)); \
      configPath="."; \
    } \
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

bool DivEngine::initAudioBackend() {
  // load values
  if (audioEngine==DIV_AUDIO_NULL) {
    if (getConfString("audioEngine","SDL")=="JACK") {
      audioEngine=DIV_AUDIO_JACK;
    } else {
      audioEngine=DIV_AUDIO_SDL;
    }
  }

  lowQuality=getConfInt("audioQuality",0);

  switch (audioEngine) {
    case DIV_AUDIO_JACK:
#ifndef HAVE_JACK
      logE("Furnace was not compiled with JACK support!\n");
      setConf("audioEngine","SDL");
      saveConf();
      output=new TAAudioSDL;
#else
      output=new TAAudioJACK;
#endif
      break;
    case DIV_AUDIO_SDL:
      output=new TAAudioSDL;
      break;
    case DIV_AUDIO_DUMMY:
      output=new TAAudio;
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

  if (!output->init(want,got)) {
    logE("error while initializing audio!\n");
    delete output;
    output=NULL;
    return false;
  }

  return true;
}

bool DivEngine::deinitAudioBackend() {
  if (output!=NULL) {
    output->quit();
    delete output;
    output=NULL;
  }
  return true;
}

#ifdef _WIN32
#include "winStuff.h"
#endif

bool DivEngine::init() {
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
#ifdef __APPLE__
      CHECK_CONFIG_DIR_MAC();
#else
      CHECK_CONFIG_DIR();
#endif
    }
  } else {
    configPath=home;
#ifdef __APPLE__
    CHECK_CONFIG_DIR_MAC();
#else
    CHECK_CONFIG_DIR();
#endif
  }
#endif
  logD("config path: %s\n",configPath.c_str());

  loadConf();

  // init the rest of engine
  if (!initAudioBackend()) return false;

  samp_bb=blip_new(32768);
  if (samp_bb==NULL) {
    logE("not enough memory!\n");
    return false;
  }

  samp_bbOut=new short[got.bufsize];

  samp_bbIn=new short[32768];
  samp_bbInLen=32768;
  
  blip_set_rates(samp_bb,44100,got.rate);

  for (int i=0; i<64; i++) {
    vibTable[i]=127*sin(((double)i/64.0)*(2*M_PI));
  }

  for (int i=0; i<DIV_MAX_CHANS; i++) {
    isMuted[i]=0;
  }

  oscBuf[0]=new float[32768];
  oscBuf[1]=new float[32768];

  initDispatch();
  reset();
  active=true;

  if (!output->setRun(true)) {
    logE("error while activating!\n");
    return false;
  }
  return true;
}

bool DivEngine::quit() {
  deinitAudioBackend();
  quitDispatch();
  logI("saving config.\n");
  saveConf();
  active=false;
  delete[] oscBuf[0];
  delete[] oscBuf[1];
  return true;
}
