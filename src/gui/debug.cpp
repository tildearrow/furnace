/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2023 tildearrow and contributors
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

#include "debug.h"
#include "imgui.h"
#include "../engine/platform/fmsharedbase.h"
#include "../engine/platform/genesis.h"
#include "../engine/platform/genesisext.h"
#include "../engine/platform/sms.h"
#include "../engine/platform/gb.h"
#include "../engine/platform/pce.h"
#include "../engine/platform/nes.h"
#include "../engine/platform/c64.h"
#include "../engine/platform/arcade.h"
#include "../engine/platform/segapcm.h"
#include "../engine/platform/ym2203.h"
#include "../engine/platform/ym2203ext.h"
#include "../engine/platform/ym2608.h"
#include "../engine/platform/ym2608ext.h"
#include "../engine/platform/ym2610.h"
#include "../engine/platform/ym2610ext.h"
#include "../engine/platform/ym2610b.h"
#include "../engine/platform/ym2610bext.h"
#include "../engine/platform/ay.h"
#include "../engine/platform/ay8930.h"
#include "../engine/platform/tia.h"
#include "../engine/platform/saa.h"
#include "../engine/platform/amiga.h"
#include "../engine/platform/qsound.h"
#include "../engine/platform/x1_010.h"
#include "../engine/platform/n163.h"
#include "../engine/platform/vrc6.h"
#include "../engine/platform/es5506.h"
#include "../engine/platform/lynx.h"
#include "../engine/platform/pcmdac.h"
#include "../engine/platform/k007232.h"
#include "../engine/platform/ga20.h"
#include "../engine/platform/sm8521.h"
#include "../engine/platform/pv1000.h"
#include "../engine/platform/k053260.h"
#include "../engine/platform/c140.h"
#include "../engine/platform/c219.h"
#include "../engine/platform/dummy.h"

#define COMMON_CHIP_DEBUG \
  ImGui::Text("- rate: %d",ch->rate); \
  ImGui::Text("- chipClock: %d",ch->chipClock);

#define FM_CHIP_DEBUG \
  COMMON_CHIP_DEBUG; \
  ImGui::Text("- delay: %d",ch->delay);

#define FM_OPN_CHIP_DEBUG \
  FM_CHIP_DEBUG; \
  ImGui::Text("- fmFreqBase: %.f",ch->fmFreqBase); \
  ImGui::Text("- fmDivBase: %d",ch->fmDivBase); \
  ImGui::Text("- ayDiv: %d",ch->ayDiv); \
  ImGui::Text("- extChanOffs: %d",ch->extChanOffs); \
  ImGui::Text("- psgChanOffs: %d",ch->psgChanOffs); \
  ImGui::Text("- adpcmAChanOffs: %d",ch->adpcmAChanOffs); \
  ImGui::Text("- adpcmBChanOffs: %d",ch->adpcmBChanOffs); \

#define COMMON_CHIP_DEBUG_BOOL \
  ImGui::TextColored(ch->skipRegisterWrites?colorOn:colorOff,">> SkipRegisterWrites"); \
  ImGui::TextColored(ch->dumpWrites?colorOn:colorOff,">> DumpWrites");

#define FM_CHIP_DEBUG_BOOL \
  COMMON_CHIP_DEBUG_BOOL; \
  ImGui::TextColored(ch->lastBusy?colorOn:colorOff,">> LastBusy"); \

#define FM_OPN_CHIP_DEBUG_BOOL \
  FM_CHIP_DEBUG_BOOL; \
  ImGui::TextColored(ch->extSys?colorOn:colorOff,">> ExtSys"); \

#define GENESIS_CHIP_DEBUG \
  DivPlatformGenesis* ch=(DivPlatformGenesis*)data; \
  ImGui::Text("> YM2612"); \
  FM_OPN_CHIP_DEBUG; \
  ImGui::Text("- lfoValue: %d",ch->lfoValue); \
  ImGui::Text("- softPCMTimer: %d",ch->softPCMTimer); \
  FM_OPN_CHIP_DEBUG_BOOL; \
  ImGui::TextColored(ch->extMode?colorOn:colorOff,">> ExtMode"); \
  ImGui::TextColored(ch->softPCM?colorOn:colorOff,">> SoftPCM"); \
  ImGui::TextColored(ch->useYMFM?colorOn:colorOff,">> UseYMFM"); \

#define OPNB_CHIP_DEBUG \
  FM_OPN_CHIP_DEBUG; \
  ImGui::Text("- sampleBank: %d",ch->sampleBank); \
  ImGui::Text("- writeADPCMAOff: %d",ch->writeADPCMAOff); \
  ImGui::Text("- writeADPCMAOn: %d",ch->writeADPCMAOn); \
  ImGui::Text("- globalADPCMAVolume: %d",ch->globalADPCMAVolume); \
  FM_OPN_CHIP_DEBUG_BOOL; \
  ImGui::TextColored(ch->extMode?colorOn:colorOff,">> ExtMode");

#define SMS_CHIP_DEBUG \
  DivPlatformSMS* sms=(DivPlatformSMS*)data; \
  ImGui::Text("> SMS"); \
  ImGui::Text("- rate: %d",sms->rate); \
  ImGui::Text("- chipClock: %d",sms->chipClock); \
  ImGui::Text("- lastPan: %d",sms->lastPan); \
  ImGui::Text("- oldValue: %d",sms->oldValue); \
  ImGui::Text("- snNoiseMode: %d",sms->snNoiseMode); \
  ImGui::Text("- divider: %d",sms->divider); \
  ImGui::Text("- toneDivider: %.f",sms->toneDivider); \
  ImGui::Text("- noiseDivider: %.f",sms->noiseDivider); \
  ImGui::TextColored(sms->skipRegisterWrites?colorOn:colorOff,">> SkipRegisterWrites"); \
  ImGui::TextColored(sms->dumpWrites?colorOn:colorOff,">> DumpWrites"); \
  ImGui::TextColored(sms->updateSNMode?colorOn:colorOff,">> UpdateSNMode"); \
  ImGui::TextColored(sms->resetPhase?colorOn:colorOff,">> ResetPhase"); \
  ImGui::TextColored(sms->isRealSN?colorOn:colorOff,">> IsRealSN"); \
  ImGui::TextColored(sms->stereo?colorOn:colorOff,">> Stereo"); \
  ImGui::TextColored(sms->nuked?colorOn:colorOff,">> Nuked");


#define COMMON_CHAN_DEBUG \
  ImGui::Text("* freq: %d",ch->freq); \
  ImGui::Text(" - base: %d",ch->baseFreq); \
  ImGui::Text(" - pitch: %d",ch->pitch); \
  ImGui::Text(" - pitch2: %d",ch->pitch2); \
  ImGui::Text("- note: %d",ch->note); \
  ImGui::Text("- ins: %d",ch->ins); \
  ImGui::Text("- vol: %.2x",ch->vol); \
  ImGui::Text("- outVol: %.2x",ch->outVol);

#define COMMON_CHAN_DEBUG_BOOL \
  ImGui::TextColored(ch->active?colorOn:colorOff,">> Active"); \
  ImGui::TextColored(ch->insChanged?colorOn:colorOff,">> InsChanged"); \
  ImGui::TextColored(ch->freqChanged?colorOn:colorOff,">> FreqChanged"); \
  ImGui::TextColored(ch->keyOn?colorOn:colorOff,">> KeyOn"); \
  ImGui::TextColored(ch->keyOff?colorOn:colorOff,">> KeyOff"); \
  ImGui::TextColored(ch->portaPause?colorOn:colorOff,">> PortaPause"); \
  ImGui::TextColored(ch->inPorta?colorOn:colorOff,">> InPorta");

#define GENESIS_CHAN_DEBUG \
  DivPlatformGenesis::Channel* ch=(DivPlatformGenesis::Channel*)data; \
  ImGui::Text("> YM2612"); \
  ImGui::Text("- freqHL: %.2x%.2x",ch->freqH,ch->freqL); \
  COMMON_CHAN_DEBUG; \
  ImGui::Text("- portaPauseFreq: %d",ch->portaPauseFreq); \
  ImGui::Text("* DAC:"); \
  ImGui::Text(" - period: %d",ch->dacPeriod); \
  ImGui::Text(" - rate: %d",ch->dacRate); \
  ImGui::Text(" - pos: %d",ch->dacPos); \
  ImGui::Text(" - sample: %d",ch->dacSample); \
  ImGui::Text(" - delay: %d",ch->dacDelay); \
  ImGui::Text(" - output: %d",ch->dacOutput); \
  ImGui::Text("- pan: %x",ch->pan); \
  ImGui::Text("- opMask: %x",ch->opMask); \
  ImGui::Text("- sampleBank: %d",ch->sampleBank); \
  COMMON_CHAN_DEBUG_BOOL; \
  ImGui::TextColored(ch->furnaceDac?colorOn:colorOff,">> FurnaceDAC"); \
  ImGui::TextColored(ch->hardReset?colorOn:colorOff,">> hardReset"); \
  ImGui::TextColored(ch->opMaskChanged?colorOn:colorOff,">> opMaskChanged"); \
  ImGui::TextColored(ch->dacMode?colorOn:colorOff,">> DACMode"); \
  ImGui::TextColored(ch->dacDirection?colorOn:colorOff,">> DACDirection");

#define GENESIS_OPCHAN_DEBUG \
  DivPlatformOPN::OPNOpChannelStereo* ch=(DivPlatformOPN::OPNOpChannelStereo*)data; \
  ImGui::Text("> YM2612 (per operator)"); \
  ImGui::Text("- freqHL: %.2x%.2x",ch->freqH,ch->freqL); \
  COMMON_CHAN_DEBUG; \
  ImGui::Text("- portaPauseFreq: %d",ch->portaPauseFreq); \
  ImGui::Text("- pan: %x",ch->pan); \
  COMMON_CHAN_DEBUG_BOOL; \
  ImGui::TextColored(ch->mask?colorOn:colorOff,">> Mask");

#define SMS_CHAN_DEBUG \
  DivPlatformSMS::Channel* ch=(DivPlatformSMS::Channel*)data; \
  ImGui::Text("> SMS"); \
  COMMON_CHAN_DEBUG; \
  COMMON_CHAN_DEBUG_BOOL;

#define OPN_CHAN_DEBUG \
  DivPlatformOPN::OPNChannel* ch=(DivPlatformOPN::OPNChannel*)data; \
  ImGui::Text("> YM2203"); \
  ImGui::Text("- freqHL: %.2x%.2x",ch->freqH,ch->freqL); \
  COMMON_CHAN_DEBUG; \
  ImGui::Text("- portaPauseFreq: %d",ch->portaPauseFreq); \
  ImGui::Text("* PSG:"); \
  ImGui::Text(" - psgMode: %d",ch->psgMode); \
  ImGui::Text(" - autoEnvNum: %d",ch->autoEnvNum); \
  ImGui::Text(" - autoEnvDen: %d",ch->autoEnvDen); \
  ImGui::Text("- sample: %d",ch->sample); \
  ImGui::Text("- opMask: %x",ch->opMask); \
  COMMON_CHAN_DEBUG_BOOL; \
  ImGui::TextColored(ch->hardReset?colorOn:colorOff,">> hardReset"); \
  ImGui::TextColored(ch->opMaskChanged?colorOn:colorOff,">> opMaskChanged"); \
  ImGui::TextColored(ch->furnacePCM?colorOn:colorOff,">> FurnacePCM");

#define OPN_OPCHAN_DEBUG \
  DivPlatformOPN::OPNOpChannel* ch=(DivPlatformOPN::OPNOpChannel*)data; \
  ImGui::Text("> YM2203 (per operator)"); \
  ImGui::Text("- freqHL: %.2x%.2x",ch->freqH,ch->freqL); \
  COMMON_CHAN_DEBUG; \
  ImGui::Text("- portaPauseFreq: %d",ch->portaPauseFreq); \
  COMMON_CHAN_DEBUG_BOOL; \
  ImGui::TextColored(ch->mask?colorOn:colorOff,">> Mask");

#define OPNB_CHAN_DEBUG \
  ImGui::Text("- freqHL: %.2x%.2x",ch->freqH,ch->freqL); \
  COMMON_CHAN_DEBUG; \
  ImGui::Text("- portaPauseFreq: %d",ch->portaPauseFreq); \
  ImGui::Text("* PSG:"); \
  ImGui::Text(" - psgMode: %d",ch->psgMode); \
  ImGui::Text(" - autoEnvNum: %d",ch->autoEnvNum); \
  ImGui::Text(" - autoEnvDen: %d",ch->autoEnvDen); \
  ImGui::Text("- sample: %d",ch->sample); \
  ImGui::Text("- pan: %x",ch->pan); \
  ImGui::Text("- opMask: %x",ch->opMask); \
  ImGui::Text("- macroVolMul: %x",ch->macroVolMul); \
  COMMON_CHAN_DEBUG_BOOL; \
  ImGui::TextColored(ch->hardReset?colorOn:colorOff,">> hardReset"); \
  ImGui::TextColored(ch->opMaskChanged?colorOn:colorOff,">> opMaskChanged"); \
  ImGui::TextColored(ch->furnacePCM?colorOn:colorOff,">> FurnacePCM");

#define OPNB_OPCHAN_DEBUG \
  ImGui::Text("- freqHL: %.2x%.2x",ch->freqH,ch->freqL); \
  COMMON_CHAN_DEBUG; \
  ImGui::Text("- portaPauseFreq: %d",ch->portaPauseFreq); \
  ImGui::Text("- pan: %x",ch->pan); \
  COMMON_CHAN_DEBUG_BOOL; \
  ImGui::TextColored(ch->mask?colorOn:colorOff,">> Mask");

void putDispatchChip(void* data, int type) {
  ImVec4 colorOn=ImVec4(1.0f,1.0f,0.0f,1.0f);
  ImVec4 colorOff=ImVec4(0.3f,0.3f,0.3f,1.0f);
  switch (type) {
    case DIV_SYSTEM_YM2612:
    case DIV_SYSTEM_YM2612_EXT:
    case DIV_SYSTEM_YM2612_DUALPCM:
    case DIV_SYSTEM_YM2612_DUALPCM_EXT: {
      GENESIS_CHIP_DEBUG;
      break;
    }
    case DIV_SYSTEM_GENESIS:
    case DIV_SYSTEM_GENESIS_EXT: {
      GENESIS_CHIP_DEBUG;
      SMS_CHIP_DEBUG;
      break;
    }
    case DIV_SYSTEM_SMS: {
      SMS_CHIP_DEBUG;
      break;
    }
    case DIV_SYSTEM_YM2203:
    case DIV_SYSTEM_YM2203_EXT: {
      DivPlatformYM2203* ch=(DivPlatformYM2203*)data;
      ImGui::Text("> YM2203");
      FM_OPN_CHIP_DEBUG;
      ImGui::Text("- sampleBank: %d",ch->sampleBank);
      ImGui::Text("- prescale: %d",ch->prescale);
      FM_OPN_CHIP_DEBUG_BOOL;
      ImGui::TextColored(ch->extMode?colorOn:colorOff,">> ExtMode");
      break;
    }
    case DIV_SYSTEM_YM2608:
    case DIV_SYSTEM_YM2608_EXT: {
      DivPlatformYM2608* ch=(DivPlatformYM2608*)data;
      ImGui::Text("> YM2608");
      FM_OPN_CHIP_DEBUG;
      ImGui::Text("- sampleBank: %d",ch->sampleBank);
      ImGui::Text("- writeRSSOff: %d",ch->writeRSSOff);
      ImGui::Text("- writeRSSOn: %d",ch->writeRSSOn);
      ImGui::Text("- globalRSSVolume: %d",ch->globalRSSVolume);
      ImGui::Text("- prescale: %d",ch->prescale);
      FM_OPN_CHIP_DEBUG_BOOL;
      ImGui::TextColored(ch->extMode?colorOn:colorOff,">> ExtMode");
      break;
    }
    case DIV_SYSTEM_YM2610:
    case DIV_SYSTEM_YM2610_EXT:
    case DIV_SYSTEM_YM2610_FULL:
    case DIV_SYSTEM_YM2610_FULL_EXT: {
      DivPlatformYM2610* ch=(DivPlatformYM2610*)data;
      ImGui::Text("> YM2610");
      OPNB_CHIP_DEBUG;
      break;
    }
    case DIV_SYSTEM_YM2610B:
    case DIV_SYSTEM_YM2610B_EXT: {
      DivPlatformYM2610B* ch=(DivPlatformYM2610B*)data;
      ImGui::Text("> YM2610B");
      OPNB_CHIP_DEBUG;
      break;
    }
    case DIV_SYSTEM_GB: {
      DivPlatformGB* ch=(DivPlatformGB*)data;
      ImGui::Text("> GameBoy");
      COMMON_CHIP_DEBUG;
      ImGui::Text("- lastPan: %d",ch->lastPan);
      ImGui::Text("- antiClickPeriodCount: %d",ch->antiClickPeriodCount);
      ImGui::Text("- antiClickWavePos: %d",ch->antiClickWavePos);
      COMMON_CHIP_DEBUG_BOOL;
      ImGui::TextColored(ch->antiClickEnabled?colorOn:colorOff,">> AntiClickEnabled");
      break;
    }
    case DIV_SYSTEM_PCE: {
      DivPlatformPCE* ch=(DivPlatformPCE*)data;
      ImGui::Text("> PCEngine");
      COMMON_CHIP_DEBUG;
      ImGui::Text("- lastPan: %d",ch->lastPan);
      ImGui::Text("- cycles: %d",ch->cycles);
      ImGui::Text("- curChan: %d",ch->curChan);
      ImGui::Text("- delay: %d",ch->delay);
      ImGui::Text("- sampleBank: %d",ch->sampleBank);
      ImGui::Text("- lfoMode: %d",ch->lfoMode);
      ImGui::Text("- lfoSpeed: %d",ch->lfoSpeed);
      COMMON_CHIP_DEBUG_BOOL;
      ImGui::TextColored(ch->antiClickEnabled?colorOn:colorOff,">> AntiClickEnabled");
      break;
    }
    case DIV_SYSTEM_NES: {
      DivPlatformNES* ch=(DivPlatformNES*)data;
      ImGui::Text("> NES");
      COMMON_CHIP_DEBUG;
      ImGui::Text("* DAC:");
      ImGui::Text(" - Period: %d",ch->dacPeriod);
      ImGui::Text(" - Rate: %d",ch->dacRate);
      ImGui::Text(" - Pos: %d",ch->dacPos);
      ImGui::Text(" - AntiClick: %d",ch->dacAntiClick);
      ImGui::Text(" - Sample: %d",ch->dacSample);
      ImGui::Text("- dpcmBank: %d",ch->dpcmBank);
      ImGui::Text("- sampleBank: %d",ch->sampleBank);
      ImGui::Text("- writeOscBuf: %d",ch->writeOscBuf);
      ImGui::Text("- apuType: %d",ch->apuType);
      COMMON_CHIP_DEBUG_BOOL;
      ImGui::TextColored(ch->dpcmMode?colorOn:colorOff,">> DPCMMode");
      ImGui::TextColored(ch->dacAntiClickOn?colorOn:colorOff,">> DACAntiClickOn");
      ImGui::TextColored(ch->useNP?colorOn:colorOff,">> UseNP");
      ImGui::TextColored(ch->goingToLoop?colorOn:colorOff,">> GoingToLoop");
      break;
    }
    case DIV_SYSTEM_C64_6581: case DIV_SYSTEM_C64_8580: {
      DivPlatformC64* ch=(DivPlatformC64*)data;
      ImGui::Text("> C64");
      COMMON_CHIP_DEBUG;
      ImGui::Text("- filtControl: %d",ch->filtControl);
      ImGui::Text("- filtRes: %d",ch->filtRes);
      ImGui::Text("- vol: %d",ch->vol);
      ImGui::Text("- writeOscBuf: %d",ch->writeOscBuf);
      ImGui::Text("- filtCut: %d",ch->filtCut);
      ImGui::Text("- resetTime: %d",ch->resetTime);
      COMMON_CHIP_DEBUG_BOOL;
      break;
    }
    case DIV_SYSTEM_ARCADE:
    case DIV_SYSTEM_YM2151: {
      DivPlatformArcade* ch=(DivPlatformArcade*)data;
      ImGui::Text("> YM2151");
      FM_CHIP_DEBUG;
      ImGui::Text("- baseFreqOff: %d",ch->baseFreqOff);
      ImGui::Text("- amDepth: %d",ch->amDepth);
      ImGui::Text("- pmDepth: %d",ch->pmDepth);
      FM_CHIP_DEBUG_BOOL;
      ImGui::TextColored(ch->useYMFM?colorOn:colorOff,">> UseYMFM");
      break;
    }
    case DIV_SYSTEM_SEGAPCM:
    case DIV_SYSTEM_SEGAPCM_COMPAT: {
      DivPlatformSegaPCM* ch=(DivPlatformSegaPCM*)data;
      ImGui::Text("> SegaPCM");
      COMMON_CHIP_DEBUG;
      ImGui::Text("- delay: %d",ch->delay);
      ImGui::Text("- pcmL: %d",ch->pcmL);
      ImGui::Text("- pcmR: %d",ch->pcmR);
      ImGui::Text("- pcmCycles: %d",ch->pcmCycles);
      ImGui::Text("- sampleBank: %d",ch->sampleBank);
      COMMON_CHIP_DEBUG_BOOL;
      break;
    }
    case DIV_SYSTEM_AY8910: {
      DivPlatformAY8910* ch=(DivPlatformAY8910*)data;
      ImGui::Text("> AY-3-8910");
      COMMON_CHIP_DEBUG;
      ImGui::Text("- sampleBank: %d",ch->sampleBank);
      ImGui::Text("- stereoSep: %d",ch->stereoSep);
      ImGui::Text("- delay: %d",ch->delay);
      ImGui::Text("- extClock: %d",ch->extClock);
      ImGui::Text("- extDiv: %d",ch->extDiv);
      ImGui::Text("- portAVal: %d",ch->portAVal);
      ImGui::Text("- portBVal: %d",ch->portBVal);
      ImGui::Text("* envelope:");
      ImGui::Text(" - mode: %d",ch->ayEnvMode);
      ImGui::Text(" - period: %d",ch->ayEnvPeriod);
      ImGui::Text(" * slide: %d",ch->ayEnvSlide);
      ImGui::Text("  - slideLow: %d",ch->ayEnvSlideLow);
      COMMON_CHIP_DEBUG_BOOL;
      ImGui::TextColored(ch->extMode?colorOn:colorOff,">> ExtMode");
      ImGui::TextColored(ch->stereo?colorOn:colorOff,">> Stereo");
      ImGui::TextColored(ch->sunsoft?colorOn:colorOff,">> Sunsoft");
      ImGui::TextColored(ch->intellivision?colorOn:colorOff,">> Intellivision");
      ImGui::TextColored(ch->clockSel?colorOn:colorOff,">> ClockSel");
      ImGui::TextColored(ch->ioPortA?colorOn:colorOff,">> IoPortA");
      ImGui::TextColored(ch->ioPortB?colorOn:colorOff,">> IoPortB");
      break;
    }
    case DIV_SYSTEM_AY8930: {
      DivPlatformAY8930* ch=(DivPlatformAY8930*)data;
      ImGui::Text("> AY8930");
      COMMON_CHIP_DEBUG;
      ImGui::Text("* noise:");
      ImGui::Text(" - and: %d",ch->ayNoiseAnd);
      ImGui::Text(" - or: %d",ch->ayNoiseOr);
      ImGui::Text("- sampleBank: %d",ch->sampleBank);
      ImGui::Text("- stereoSep: %d",ch->stereoSep);
      ImGui::Text("- delay: %d",ch->delay);
      ImGui::Text("- portAVal: %d",ch->portAVal);
      ImGui::Text("- portBVal: %d",ch->portBVal);
      COMMON_CHIP_DEBUG_BOOL;
      ImGui::TextColored(ch->bank?colorOn:colorOff,">> Bank");
      ImGui::TextColored(ch->extMode?colorOn:colorOff,">> ExtMode");
      ImGui::TextColored(ch->stereo?colorOn:colorOff,">> Stereo");
      ImGui::TextColored(ch->clockSel?colorOn:colorOff,">> ClockSel");
      ImGui::TextColored(ch->ioPortA?colorOn:colorOff,">> IoPortA");
      ImGui::TextColored(ch->ioPortB?colorOn:colorOff,">> IoPortB");
      break;
    }
    case DIV_SYSTEM_QSOUND: {
      DivPlatformQSound* ch=(DivPlatformQSound*)data;
      ImGui::Text("> QSound");
      COMMON_CHIP_DEBUG;
      ImGui::Text("* echo:");
      ImGui::Text(" - delay: %d",ch->echoDelay);
      ImGui::Text(" - feedback: %d",ch->echoFeedback);
      COMMON_CHIP_DEBUG_BOOL;
      break;
    }
    case DIV_SYSTEM_X1_010: {
      DivPlatformX1_010* ch=(DivPlatformX1_010*)data;
      ImGui::Text("> X1-010");
      COMMON_CHIP_DEBUG;
      ImGui::Text("- sampleBank: %d",ch->sampleBank);
      ImGui::Text("- bankSlot: [%d,%d,%d,%d,%d,%d,%d,%d]",ch->bankSlot[0],ch->bankSlot[1],ch->bankSlot[2],ch->bankSlot[3],ch->bankSlot[4],ch->bankSlot[5],ch->bankSlot[6],ch->bankSlot[7]);
      COMMON_CHIP_DEBUG_BOOL;
      ImGui::TextColored(ch->stereo?colorOn:colorOff,">> Stereo");
      ImGui::TextColored(ch->isBanked?colorOn:colorOff,">> IsBanked");
      break;
    }
    case DIV_SYSTEM_N163: {
      DivPlatformN163* ch=(DivPlatformN163*)data;
      ImGui::Text("> N163");
      COMMON_CHIP_DEBUG;
      ImGui::Text("- initChanMax: %d",ch->initChanMax);
      ImGui::Text("- chanMax: %d",ch->chanMax);
      ImGui::Text("- loadWave: %d",ch->loadWave);
      ImGui::Text("- loadPos: %d",ch->loadPos);
      COMMON_CHIP_DEBUG_BOOL;
      ImGui::TextColored(ch->multiplex?colorOn:colorOff,">> Multiplex");
      break;
    }
    case DIV_SYSTEM_VRC6: {
      DivPlatformVRC6* ch=(DivPlatformVRC6*)data;
      ImGui::Text("> VRC6");
      COMMON_CHIP_DEBUG;
      ImGui::Text("- sampleBank: %.2x",ch->sampleBank);
      ImGui::Text("- writeOscBuf: %.2x",ch->writeOscBuf);
      COMMON_CHIP_DEBUG_BOOL;
      break;
    }
    case DIV_SYSTEM_LYNX: {
      DivPlatformLynx* ch=(DivPlatformLynx*)data;
      ImGui::Text("> Lynx");
      COMMON_CHIP_DEBUG;
      COMMON_CHIP_DEBUG_BOOL;
      break;
    }
    case DIV_SYSTEM_PCM_DAC: {
      DivPlatformPCMDAC* ch=(DivPlatformPCMDAC*)data;
      ImGui::Text("> PCM DAC");
      COMMON_CHIP_DEBUG;
      ImGui::Text("- outDepth: %d",ch->outDepth);
      COMMON_CHIP_DEBUG_BOOL;
      ImGui::TextColored(ch->outStereo?colorOn:colorOff,">> OutStereo");
      break;
    }
    case DIV_SYSTEM_ES5506: {
      DivPlatformES5506* ch=(DivPlatformES5506*)data;
      ImGui::Text("> ES5506");
      COMMON_CHIP_DEBUG;
      ImGui::Text("- cycle: %d",ch->cycle);
      ImGui::Text("- curPage: %d",ch->curPage);
      ImGui::Text("- volScale: %d",ch->volScale);
      ImGui::Text("- maskedVal: %.2x",ch->maskedVal);
      ImGui::Text("- irqv: %.2x",ch->irqv);
      ImGui::Text("- curCR: %.8x",ch->curCR);
      ImGui::Text("- initChanMax: %d",ch->initChanMax);
      ImGui::Text("- chanMax: %d",ch->chanMax);
      COMMON_CHIP_DEBUG_BOOL;
      ImGui::TextColored(ch->isMasked?colorOn:colorOff,">> IsMasked");
      ImGui::TextColored(ch->isReaded?colorOn:colorOff,">> isReaded");
      ImGui::TextColored(ch->irqTrigger?colorOn:colorOff,">> IrqTrigger");
      break;
    }
    case DIV_SYSTEM_K007232: {
      DivPlatformK007232* ch=(DivPlatformK007232*)data;
      ImGui::Text("> K007232");
      COMMON_CHIP_DEBUG;
      ImGui::Text("- delay: %.2x",ch->delay);
      ImGui::Text("- lastLoop: %.2x",ch->lastLoop);
      ImGui::Text("- lastVolume: %.2x",ch->lastVolume);
      COMMON_CHIP_DEBUG_BOOL;
      ImGui::TextColored(ch->stereo?colorOn:colorOff,">> Stereo");
      break;
    }
    case DIV_SYSTEM_GA20: {
      DivPlatformGA20* ch=(DivPlatformGA20*)data;
      ImGui::Text("> GA20");
      COMMON_CHIP_DEBUG;
      ImGui::Text("- delay: %.2x",ch->delay);
      COMMON_CHIP_DEBUG_BOOL;
      break;
    }
    case DIV_SYSTEM_SM8521: {
      DivPlatformSM8521* ch=(DivPlatformSM8521*)data;
      ImGui::Text("> SM8521");
      COMMON_CHIP_DEBUG;
      COMMON_CHIP_DEBUG_BOOL;
      ImGui::TextColored(ch->antiClickEnabled?colorOn:colorOff,">> AntiClickEnabled");
      break;
    }
    case DIV_SYSTEM_PV1000: {
      DivPlatformPV1000* ch=(DivPlatformPV1000*)data;
      ImGui::Text("> PV1000");
      COMMON_CHIP_DEBUG;
      COMMON_CHIP_DEBUG_BOOL;
      break;
    }
    case DIV_SYSTEM_K053260: {
      DivPlatformK053260* ch=(DivPlatformK053260*)data;
      ImGui::Text("> K053260");
      COMMON_CHIP_DEBUG;
      COMMON_CHIP_DEBUG_BOOL;
      break;
    }
    case DIV_SYSTEM_C140: {
      DivPlatformC140* ch=(DivPlatformC140*)data;
      ImGui::Text("> C140");
      COMMON_CHIP_DEBUG;
      COMMON_CHIP_DEBUG_BOOL;
      break;
    }
    case DIV_SYSTEM_C219: {
      DivPlatformC219* ch=(DivPlatformC219*)data;
      ImGui::Text("> C219");
      COMMON_CHIP_DEBUG;
      COMMON_CHIP_DEBUG_BOOL;
      break;
    }
    default:
      ImGui::Text("Unimplemented chip! Help!");
      break;
  }
}
void putDispatchChan(void* data, int chanNum, int type) {
  ImVec4 colorOn=ImVec4(1.0f,1.0f,0.0f,1.0f);
  ImVec4 colorOff=ImVec4(0.3f,0.3f,0.3f,1.0f);
  switch (type) {
    case DIV_SYSTEM_GENESIS: {
      if (chanNum>5) {
        SMS_CHAN_DEBUG;
      } else {
        GENESIS_CHAN_DEBUG;
      }
      break;
    }
    case DIV_SYSTEM_GENESIS_EXT: {
      if (chanNum>8) {
        SMS_CHAN_DEBUG;
      } else if (chanNum>=2 && chanNum<=5) {
        DivPlatformOPN::OPNOpChannelStereo* ch=(DivPlatformOPN::OPNOpChannelStereo*)data;
        ImGui::Text("> YM2612 (per operator)");
        OPNB_OPCHAN_DEBUG;
      } else {
        GENESIS_CHAN_DEBUG;
      }
      break;
    }
    case DIV_SYSTEM_YM2612:
    case DIV_SYSTEM_YM2612_DUALPCM: {
      GENESIS_CHAN_DEBUG;
      break;
    }
    case DIV_SYSTEM_YM2612_EXT:
    case DIV_SYSTEM_YM2612_DUALPCM_EXT: {
      if (chanNum>=2 && chanNum<=5) {
        DivPlatformOPN::OPNOpChannelStereo* ch=(DivPlatformOPN::OPNOpChannelStereo*)data;
        ImGui::Text("> YM2612 (per operator)");
        OPNB_OPCHAN_DEBUG;
      } else {
        GENESIS_CHAN_DEBUG;
      }
      break;
    }
    case DIV_SYSTEM_SMS: {
      SMS_CHAN_DEBUG;
      break;
    }
    case DIV_SYSTEM_YM2203: {
      OPN_CHAN_DEBUG;
      break;
    }
    case DIV_SYSTEM_YM2203_EXT: {
      if (chanNum>=2 && chanNum<=5) {
        OPN_OPCHAN_DEBUG;
      } else {
        OPN_CHAN_DEBUG;
      }
      break;
    }
    case DIV_SYSTEM_YM2608: {
      DivPlatformOPN::OPNChannelStereo* ch=(DivPlatformOPN::OPNChannelStereo*)data;
      ImGui::Text("> YM2608");
      OPNB_CHAN_DEBUG;
      break;
    }
    case DIV_SYSTEM_YM2608_EXT: {
      if (chanNum>=2 && chanNum<=5) {
        DivPlatformOPN::OPNOpChannelStereo* ch=(DivPlatformOPN::OPNOpChannelStereo*)data;
        ImGui::Text("> YM2608 (per operator)");
        OPNB_OPCHAN_DEBUG;
      } else {
        DivPlatformOPN::OPNChannelStereo* ch=(DivPlatformOPN::OPNChannelStereo*)data;
        ImGui::Text("> YM2608");
        OPNB_CHAN_DEBUG;
      }
      break;
    }
    case DIV_SYSTEM_YM2610:
    case DIV_SYSTEM_YM2610_FULL: {
      DivPlatformOPN::OPNChannelStereo* ch=(DivPlatformOPN::OPNChannelStereo*)data;
      ImGui::Text("> YM2610");
      OPNB_CHAN_DEBUG;
      break;
    }
    case DIV_SYSTEM_YM2610B: {
      DivPlatformOPN::OPNChannelStereo* ch=(DivPlatformOPN::OPNChannelStereo*)data;
      ImGui::Text("> YM2610B");
      OPNB_CHAN_DEBUG;
      break;
    }
    case DIV_SYSTEM_YM2610_EXT:
    case DIV_SYSTEM_YM2610_FULL_EXT: {
      if (chanNum>=1 && chanNum<=4) {
        DivPlatformOPN::OPNOpChannelStereo* ch=(DivPlatformOPN::OPNOpChannelStereo*)data;
        ImGui::Text("> YM2610 (per operator)");
        OPNB_OPCHAN_DEBUG;
      } else {
        DivPlatformOPN::OPNChannelStereo* ch=(DivPlatformOPN::OPNChannelStereo*)data;
        ImGui::Text("> YM2610");
        OPNB_CHAN_DEBUG;
      }
      break;
    }
    case DIV_SYSTEM_YM2610B_EXT: {
      if (chanNum>=2 && chanNum<=5) {
        DivPlatformOPN::OPNOpChannelStereo* ch=(DivPlatformOPN::OPNOpChannelStereo*)data;
        ImGui::Text("> YM2610B (per operator)");
        OPNB_OPCHAN_DEBUG;
      } else {
        DivPlatformOPN::OPNChannelStereo* ch=(DivPlatformOPN::OPNChannelStereo*)data;
        ImGui::Text("> YM2610B");
        OPNB_CHAN_DEBUG;
      }
      break;
    }
    case DIV_SYSTEM_GB: {
      DivPlatformGB::Channel* ch=(DivPlatformGB::Channel*)data;
      ImGui::Text("> GameBoy");
      COMMON_CHAN_DEBUG;
      ImGui::Text("- duty: %d",ch->duty);
      ImGui::Text("- sweep: %.2x",ch->sweep);
      ImGui::Text("- wave: %d",ch->wave);
      COMMON_CHAN_DEBUG_BOOL;
      ImGui::TextColored(ch->sweepChanged?colorOn:colorOff,">> SweepChanged");
      break;
    }
    case DIV_SYSTEM_PCE: {
      DivPlatformPCE::Channel* ch=(DivPlatformPCE::Channel*)data;
      ImGui::Text("> PCEngine");
      COMMON_CHAN_DEBUG;
      ImGui::Text("* DAC:");
      ImGui::Text(" - period: %d",ch->dacPeriod);
      ImGui::Text(" - rate: %d",ch->dacRate);
      ImGui::Text(" - pos: %d",ch->dacPos);
      ImGui::Text(" - out: %d",ch->dacOut);
      ImGui::Text(" - sample: %d",ch->dacSample);
      ImGui::Text("- pan: %.2x",ch->pan);
      ImGui::Text("- wave: %d",ch->wave);
      ImGui::Text("- macroVolMul: %d",ch->macroVolMul);
      COMMON_CHAN_DEBUG_BOOL;
      ImGui::TextColored(ch->noise?colorOn:colorOff,">> Noise");
      ImGui::TextColored(ch->pcm?colorOn:colorOff,">> DAC");
      ImGui::TextColored(ch->furnaceDac?colorOn:colorOff,">> FurnaceDAC");
      break;
    }
    case DIV_SYSTEM_NES: {
      DivPlatformNES::Channel* ch=(DivPlatformNES::Channel*)data;
      ImGui::Text("> NES");
      COMMON_CHAN_DEBUG;
      ImGui::Text(" - prev: %d",ch->prevFreq);
      ImGui::Text("- duty: %d",ch->duty);
      ImGui::Text("- sweep: %.2x",ch->sweep);
      COMMON_CHAN_DEBUG_BOOL;
      ImGui::TextColored(ch->sweepChanged?colorOn:colorOff,">> SweepChanged");
      ImGui::TextColored(ch->furnaceDac?colorOn:colorOff,">> FurnaceDAC");
      break;
    }
    case DIV_SYSTEM_C64_6581: case DIV_SYSTEM_C64_8580: {
      DivPlatformC64::Channel* ch=(DivPlatformC64::Channel*)data;
      ImGui::Text("> C64");
      COMMON_CHAN_DEBUG;
      ImGui::Text("- prevFreq: %d",ch->prevFreq);
      ImGui::Text("- testWhen: %d",ch->testWhen);
      ImGui::Text("- duty: %d",ch->duty);
      ImGui::Text("- sweep: %.2x",ch->sweep);
      ImGui::Text("- wave: %.1x",ch->wave);
      ImGui::Text("- ADSR: %.1x %.1x %.1x %.1x",ch->attack,ch->decay,ch->sustain,ch->release);
      COMMON_CHAN_DEBUG_BOOL;
      ImGui::TextColored(ch->sweepChanged?colorOn:colorOff,">> SweepChanged");
      ImGui::TextColored(ch->filter?colorOn:colorOff,">> Filter");
      ImGui::TextColored(ch->resetMask?colorOn:colorOff,">> ResetMask");
      ImGui::TextColored(ch->resetFilter?colorOn:colorOff,">> ResetFilter");
      ImGui::TextColored(ch->resetDuty?colorOn:colorOff,">> ResetDuty");
      ImGui::TextColored(ch->ring?colorOn:colorOff,">> Ring");
      ImGui::TextColored(ch->sync?colorOn:colorOff,">> Sync");
      break;
    }
    case DIV_SYSTEM_ARCADE:
    case DIV_SYSTEM_YM2151: {
      DivPlatformArcade::Channel* ch=(DivPlatformArcade::Channel*)data;
      ImGui::Text("> YM2151");
      ImGui::Text("- freqHL: %.2x%.2x",ch->freqH,ch->freqL);
      COMMON_CHAN_DEBUG;
      ImGui::Text("- KOnCycles: %d",ch->konCycles);
      ImGui::Text("- chVolL: %.2x",ch->chVolL);
      ImGui::Text("- chVolR: %.2x",ch->chVolR);
      COMMON_CHAN_DEBUG_BOOL;
      break;
    }
    case DIV_SYSTEM_SEGAPCM:
    case DIV_SYSTEM_SEGAPCM_COMPAT: {
      DivPlatformSegaPCM::Channel* ch=(DivPlatformSegaPCM::Channel*)data;
      ImGui::Text("> SegaPCM");
      COMMON_CHAN_DEBUG;
      ImGui::Text("* PCM:");
      ImGui::Text(" - sample: %d",ch->pcm.sample);
      ImGui::Text(" - pos: %d",ch->pcm.pos);
      ImGui::Text(" - len: %d",ch->pcm.len);
      ImGui::Text(" - freq: %d",ch->pcm.freq);
      ImGui::Text("- chVolL: %.2x",ch->chVolL);
      ImGui::Text("- chVolR: %.2x",ch->chVolR);
      ImGui::Text("- chPanL: %.2x",ch->chPanL);
      ImGui::Text("- chPanR: %.2x",ch->chPanR);
      ImGui::Text("- macroVolMul: %.2x",ch->macroVolMul);
      COMMON_CHAN_DEBUG_BOOL;
      ImGui::TextColored(ch->furnacePCM?colorOn:colorOff,">> FurnacePCM");
      ImGui::TextColored(ch->isNewSegaPCM?colorOn:colorOff,">> IsNewSegaPCM");
      break;
    }
    case DIV_SYSTEM_AY8910: {
      DivPlatformAY8910::Channel* ch=(DivPlatformAY8910::Channel*)data;
      ImGui::Text("> AY-3-8910");
      COMMON_CHAN_DEBUG;
      ImGui::Text("* DAC:");
      ImGui::Text(" - sample: %d",ch->dac.sample);
      ImGui::Text(" - rate: %d",ch->dac.rate);
      ImGui::Text(" - period: %d",ch->dac.period);
      ImGui::Text(" - pos: %d",ch->dac.pos);
      ImGui::Text(" - out: %d",ch->dac.out);
      ImGui::Text("- autoEnvNum: %.2x",ch->autoEnvNum);
      ImGui::Text("- autoEnvDen: %.2x",ch->autoEnvDen);
      COMMON_CHAN_DEBUG_BOOL;
      ImGui::TextColored(ch->dac.furnaceDAC?colorOn:colorOff,">> furnaceDAC");
      break;
    }
    case DIV_SYSTEM_AY8930: {
      DivPlatformAY8930::Channel* ch=(DivPlatformAY8930::Channel*)data;
      ImGui::Text("> AY8930");
      COMMON_CHAN_DEBUG;
      ImGui::Text("- duty: %d",ch->duty);
      ImGui::Text("* DAC:");
      ImGui::Text(" - sample: %d",ch->dac.sample);
      ImGui::Text(" - rate: %d",ch->dac.rate);
      ImGui::Text(" - period: %d",ch->dac.period);
      ImGui::Text(" - pos: %d",ch->dac.pos);
      ImGui::Text(" - out: %d",ch->dac.out);
      ImGui::Text("- autoEnvNum: %.2x",ch->autoEnvNum);
      ImGui::Text("- autoEnvDen: %.2x",ch->autoEnvDen);
      COMMON_CHAN_DEBUG_BOOL;
      ImGui::TextColored(ch->dac.furnaceDAC?colorOn:colorOff,">> furnaceDAC");
      break;
    }
    case DIV_SYSTEM_QSOUND: {
      DivPlatformQSound::Channel* ch=(DivPlatformQSound::Channel*)data;
      ImGui::Text("> QSound");
      COMMON_CHAN_DEBUG;
      ImGui::Text("- sample: %d",ch->sample);
      ImGui::Text("- echo: %d",ch->echo);
      ImGui::Text("- panning: %d",ch->panning);
      ImGui::Text("- resVol: %.2x",ch->resVol);
      COMMON_CHAN_DEBUG_BOOL;
      ImGui::TextColored(ch->useWave?colorOn:colorOff,">> UseWave");
      ImGui::TextColored(ch->surround?colorOn:colorOff,">> Surround");
      ImGui::TextColored(ch->isNewQSound?colorOn:colorOff,">> IsNewQSound");
      break;
    }
    case DIV_SYSTEM_X1_010: {
      DivPlatformX1_010::Channel* ch=(DivPlatformX1_010::Channel*)data;
      ImGui::Text("> X1-010");
      COMMON_CHAN_DEBUG;
      ImGui::Text("- wave: %d",ch->wave);
      ImGui::Text("- sample: %d",ch->sample);
      ImGui::Text("- pan: %d",ch->pan);
      ImGui::Text("* envelope:");
      ImGui::Text(" - shape: %d",ch->env.shape);
      ImGui::Text(" - period: %.2x",ch->env.period);
      ImGui::Text(" - slide: %.2x",ch->env.slide);
      ImGui::Text(" - slidefrac: %.2x",ch->env.slidefrac);
      ImGui::Text(" - autoEnvNum: %.2x",ch->autoEnvNum);
      ImGui::Text(" - autoEnvDen: %.2x",ch->autoEnvDen);
      ImGui::Text("- WaveBank: %d",ch->waveBank);
      ImGui::Text("- bankSlot: %d",ch->bankSlot);
      ImGui::Text("- Lvol: %.2x",ch->lvol);
      ImGui::Text("- Rvol: %.2x",ch->rvol);
      COMMON_CHAN_DEBUG_BOOL;
      ImGui::TextColored(ch->envChanged?colorOn:colorOff,">> EnvChanged");
      ImGui::TextColored(ch->furnacePCM?colorOn:colorOff,">> FurnacePCM");
      ImGui::TextColored(ch->pcm?colorOn:colorOff,">> PCM");
      ImGui::TextColored(ch->env.flag.envEnable?colorOn:colorOff,">> EnvEnable");
      ImGui::TextColored(ch->env.flag.envOneshot?colorOn:colorOff,">> EnvOneshot");
      ImGui::TextColored(ch->env.flag.envSplit?colorOn:colorOff,">> EnvSplit");
      ImGui::TextColored(ch->env.flag.envHinvR?colorOn:colorOff,">> EnvHinvR");
      ImGui::TextColored(ch->env.flag.envVinvR?colorOn:colorOff,">> EnvVinvR");
      ImGui::TextColored(ch->env.flag.envHinvL?colorOn:colorOff,">> EnvHinvL");
      ImGui::TextColored(ch->env.flag.envVinvL?colorOn:colorOff,">> EnvVinvL");
      break;
    }
    case DIV_SYSTEM_N163: {
      DivPlatformN163::Channel* ch=(DivPlatformN163::Channel*)data;
      ImGui::Text("> N163");
      COMMON_CHAN_DEBUG;
      ImGui::Text("- wave: %d",ch->wave);
      ImGui::Text("- wavepos: %d",ch->wavePos);
      ImGui::Text("- wavelen: %d",ch->waveLen);
      ImGui::Text("- wavemode: %d",ch->waveMode);
      ImGui::Text("- resVol: %.2x",ch->resVol);
      COMMON_CHAN_DEBUG_BOOL;
      ImGui::TextColored(ch->volumeChanged?colorOn:colorOff,">> VolumeChanged");
      ImGui::TextColored(ch->waveChanged?colorOn:colorOff,">> WaveChanged");
      ImGui::TextColored(ch->waveUpdated?colorOn:colorOff,">> WaveUpdated");
      break;
    }
    case DIV_SYSTEM_VRC6: {
      DivPlatformVRC6::Channel* ch=(DivPlatformVRC6::Channel*)data;
      ImGui::Text("> VRC6");
      COMMON_CHAN_DEBUG;
      ImGui::Text("* DAC:");
      ImGui::Text(" - period: %d",ch->dacPeriod);
      ImGui::Text(" - rate: %d",ch->dacRate);
      ImGui::Text(" - out: %d",ch->dacOut);
      ImGui::Text(" - pos: %d",ch->dacPos);
      ImGui::Text(" - sample: %d",ch->dacSample);
      ImGui::Text("- duty: %d",ch->duty);
      COMMON_CHAN_DEBUG_BOOL;
      ImGui::TextColored(ch->pcm?colorOn:colorOff,">> DAC");
      ImGui::TextColored(ch->furnaceDac?colorOn:colorOff,">> FurnaceDAC");
      break;
    }
    case DIV_SYSTEM_ES5506: {
      DivPlatformES5506::Channel* ch=(DivPlatformES5506::Channel*)data;
      ImGui::Text("> ES5506");
      COMMON_CHAN_DEBUG;
      ImGui::Text("- nextFreq: %d",ch->nextFreq);
      ImGui::Text("- nextNote: %d",ch->nextNote);
      ImGui::Text("- currNote: %d",ch->currNote);
      ImGui::Text("- wave: %d",ch->wave);
      ImGui::Text("- VolMacroMax: %d",ch->volMacroMax);
      ImGui::Text("- PanMacroMax: %d",ch->panMacroMax);
      ImGui::Text("* PCM:");
      ImGui::Text(" * index: %d",ch->pcm.index);
      ImGui::Text("  - next: %d",ch->pcm.next);
      ImGui::Text(" - note: %d",ch->pcm.note);
      ImGui::Text(" * freqOffs: %.6f",ch->pcm.freqOffs);
      ImGui::Text("  - next: %.6f",ch->pcm.nextFreqOffs);
      ImGui::Text(" - bank: %.2x",ch->pcm.bank);
      ImGui::Text(" - start: %.8x",ch->pcm.start);
      ImGui::Text(" - end: %.8x",ch->pcm.end);
      ImGui::Text(" - length: %.8x",ch->pcm.length);
      ImGui::Text(" - loopStart: %.8x",ch->pcm.loopStart);
      ImGui::Text(" - loopEnd: %.8x",ch->pcm.loopEnd);
      ImGui::Text(" - loopMode: %d",ch->pcm.loopMode);
      ImGui::Text(" - nextPos: %d",ch->pcm.nextPos);
      ImGui::Text("* Filter:");
      ImGui::Text(" - Mode: %d",ch->filter.mode);
      ImGui::Text(" - K1: %.4x",ch->filter.k1);
      ImGui::Text(" - K2: %.4x",ch->filter.k2);
      ImGui::Text("* Envelope:");
      ImGui::Text(" - EnvCount: %.3x",ch->envelope.ecount);
      ImGui::Text(" - LVRamp: %d",ch->envelope.lVRamp);
      ImGui::Text(" - RVRamp: %d",ch->envelope.rVRamp);
      ImGui::Text(" - K1Ramp: %d",ch->envelope.k1Ramp);
      ImGui::Text(" - K2Ramp: %d",ch->envelope.k2Ramp);
      ImGui::Text(" - K1Offs: %d",ch->k1Offs);
      ImGui::Text(" - K2Offs: %d",ch->k2Offs);
      ImGui::Text(" - K1Slide: %d",ch->k1Slide);
      ImGui::Text(" - K2Slide: %d",ch->k2Slide);
      ImGui::Text(" - K1Prev: %.4x",ch->k1Prev);
      ImGui::Text(" - K2Prev: %.4x",ch->k2Prev);
      ImGui::Text("* Overwrite:");
      ImGui::Text(" * Filter:");
      ImGui::Text("  - Mode: %d",ch->overwrite.filter.mode);
      ImGui::Text("  - K1: %.4x",ch->overwrite.filter.k1);
      ImGui::Text("  - K2: %.4x",ch->overwrite.filter.k2);
      ImGui::Text(" * Envelope:");
      ImGui::Text("  - EnvCount: %.3x",ch->overwrite.envelope.ecount);
      ImGui::Text("  - LVRamp: %d",ch->overwrite.envelope.lVRamp);
      ImGui::Text("  - RVRamp: %d",ch->overwrite.envelope.rVRamp);
      ImGui::Text("  - K1Ramp: %d",ch->overwrite.envelope.k1Ramp);
      ImGui::Text("  - K2Ramp: %d",ch->overwrite.envelope.k2Ramp);
      ImGui::Text("- CA: %.2x",ch->ca);
      ImGui::Text("- LVol: %.2x",ch->lVol);
      ImGui::Text("- RVol: %.2x",ch->rVol);
      ImGui::Text("- outLVol: %.2x",ch->outLVol);
      ImGui::Text("- outRVol: %.2x",ch->outRVol);
      ImGui::Text("- ResLVol: %.2x",ch->resLVol);
      ImGui::Text("- ResRVol: %.2x",ch->resRVol);
      ImGui::Text("- oscOut: %d",ch->oscOut);
      COMMON_CHAN_DEBUG_BOOL;
      ImGui::TextColored(ch->volChanged.lVol?colorOn:colorOff,">> LVolChanged");
      ImGui::TextColored(ch->volChanged.rVol?colorOn:colorOff,">> RVolChanged");
      ImGui::TextColored(ch->filterChanged.mode?colorOn:colorOff,">> FilterModeChanged");
      ImGui::TextColored(ch->filterChanged.k1?colorOn:colorOff,">> FilterK1Changed");
      ImGui::TextColored(ch->filterChanged.k2?colorOn:colorOff,">> FilterK2Changed");
      ImGui::TextColored(ch->envChanged.ecount?colorOn:colorOff,">> EnvECountChanged");
      ImGui::TextColored(ch->envChanged.lVRamp?colorOn:colorOff,">> EnvLVRampChanged");
      ImGui::TextColored(ch->envChanged.rVRamp?colorOn:colorOff,">> EnvRVRampChanged");
      ImGui::TextColored(ch->envChanged.k1Ramp?colorOn:colorOff,">> EnvK1RampChanged");
      ImGui::TextColored(ch->envChanged.k2Ramp?colorOn:colorOff,">> EnvK2RampChanged");
      ImGui::TextColored(ch->pcmChanged.index?colorOn:colorOff,">> PCMIndexChanged");
      ImGui::TextColored(ch->pcmChanged.slice?colorOn:colorOff,">> PCMSliceChanged");
      ImGui::TextColored(ch->pcmChanged.position?colorOn:colorOff,">> PCMPositionChanged");
      ImGui::TextColored(ch->pcmChanged.loopBank?colorOn:colorOff,">> PCMLoopBankChanged");
      ImGui::TextColored(ch->isReverseLoop?colorOn:colorOff,">> IsReverseLoop");
      ImGui::TextColored(ch->pcm.isNoteMap?colorOn:colorOff,">> PCMIsNoteMap");
      ImGui::TextColored(ch->pcm.pause?colorOn:colorOff,">> PCMPause");
      ImGui::TextColored(ch->pcm.direction?colorOn:colorOff,">> PCMDirection");
      ImGui::TextColored(ch->pcm.setPos?colorOn:colorOff,">> PCMSetPos");
      ImGui::TextColored(ch->envelope.k1Slow?colorOn:colorOff,">> EnvK1Slow");
      ImGui::TextColored(ch->envelope.k2Slow?colorOn:colorOff,">> EnvK2Slow");
      ImGui::TextColored(ch->overwrite.envelope.k1Slow?colorOn:colorOff,">> EnvK1SlowOverwrite");
      ImGui::TextColored(ch->overwrite.envelope.k2Slow?colorOn:colorOff,">> EnvK2SlowOverwrite");
      ImGui::TextColored(ch->overwrite.state.mode?colorOn:colorOff,">> FilterModeOverwrited");
      ImGui::TextColored(ch->overwrite.state.k1?colorOn:colorOff,">> FilterK1Overwrited");
      ImGui::TextColored(ch->overwrite.state.k2?colorOn:colorOff,">> FilterK2Overwrited");
      ImGui::TextColored(ch->overwrite.state.ecount?colorOn:colorOff,">> EnvECountOverwrited");
      ImGui::TextColored(ch->overwrite.state.lVRamp?colorOn:colorOff,">> EnvLVRampOverwrited");
      ImGui::TextColored(ch->overwrite.state.rVRamp?colorOn:colorOff,">> EnvRVRampOverwrited");
      ImGui::TextColored(ch->overwrite.state.k1Ramp?colorOn:colorOff,">> EnvK1RampOverwrited");
      ImGui::TextColored(ch->overwrite.state.k2Ramp?colorOn:colorOff,">> EnvK2RampOverwrited");
      break;
    }
    case DIV_SYSTEM_LYNX: {
      DivPlatformLynx::Channel* ch=(DivPlatformLynx::Channel*)data;
      ImGui::Text("> Lynx");
      COMMON_CHAN_DEBUG;
      ImGui::Text("* FreqDiv:");
      ImGui::Text(" - clockDivider: %d",ch->fd.clockDivider);
      ImGui::Text(" - backup: %d",ch->fd.backup);
      ImGui::Text("- actualNote: %d",ch->actualNote);
      ImGui::Text("* Sample:");
      ImGui::Text(" - sample: %d",ch->sample);
      ImGui::Text(" - pos: %d",ch->samplePos);
      ImGui::Text(" - accum: %d",ch->sampleAccum);
      ImGui::Text(" * freq: %d",ch->sampleFreq);
      ImGui::Text("  - base: %d",ch->sampleBaseFreq);
      ImGui::Text("* duty:");
      ImGui::Text(" - int_feedback7: %d",ch->duty.int_feedback7);
      ImGui::Text(" - feedback: %d",ch->duty.feedback);
      ImGui::Text("- pan: %.2x",ch->pan);
      ImGui::Text("- macroVolMul: %.2x",ch->macroVolMul);
      COMMON_CHAN_DEBUG_BOOL;
      ImGui::TextColored(ch->pcm?colorOn:colorOff,">> DAC");
      break;
    }
    case DIV_SYSTEM_PCM_DAC: {
      DivPlatformPCMDAC::Channel* ch=(DivPlatformPCMDAC::Channel*)data;
      ImGui::Text("> PCM DAC");
      COMMON_CHAN_DEBUG;
      ImGui::Text("* Sample: %d",ch->sample);
      ImGui::Text(" - dir: %d",ch->audDir);
      ImGui::Text(" - loc: %d",ch->audLoc);
      ImGui::Text(" - len: %d",ch->audLen);
      ImGui::Text(" * pos: %d",ch->audPos);
      ImGui::Text("  - sub: %d",ch->audSub);
      ImGui::Text("- wave: %d",ch->wave);
      ImGui::Text("- panL: %.2x",ch->panL);
      ImGui::Text("- panR: %.2x",ch->panR);
      ImGui::Text("- envVol: %.2x",ch->envVol);
      COMMON_CHAN_DEBUG_BOOL;
      ImGui::TextColored(ch->useWave?colorOn:colorOff,">> UseWave");
      ImGui::TextColored(ch->setPos?colorOn:colorOff,">> SetPos");
      break;
    }
    case DIV_SYSTEM_K007232: {
      DivPlatformK007232::Channel* ch=(DivPlatformK007232::Channel*)data;
      ImGui::Text("> K007232");
      COMMON_CHAN_DEBUG;
      ImGui::Text("- prevFreq: %d",ch->prevFreq);
      ImGui::Text("* Sample: %d",ch->sample);
      ImGui::Text(" - pos: %d",ch->audPos);
      ImGui::Text(" - prevBank: %d",ch->prevBank);
      ImGui::Text("* panning: %d",ch->panning);
      ImGui::Text(" - prev: %d",ch->prevPan);
      ImGui::Text("- resVol: %.2x",ch->resVol);
      ImGui::Text("- lvol: %.2x",ch->lvol);
      ImGui::Text("- rvol: %.2x",ch->rvol);
      ImGui::Text("- macroVolMul: %.2x",ch->macroVolMul);
      COMMON_CHAN_DEBUG_BOOL;
      ImGui::TextColored(ch->volumeChanged?colorOn:colorOff,">> VolumeChanged");
      ImGui::TextColored(ch->setPos?colorOn:colorOff,">> SetPos");
      break;
    }
    case DIV_SYSTEM_GA20: {
      DivPlatformGA20::Channel* ch=(DivPlatformGA20::Channel*)data;
      ImGui::Text("> GA20");
      COMMON_CHAN_DEBUG;
      ImGui::Text("- prevFreq: %d",ch->prevFreq);
      ImGui::Text("* Sample: %d",ch->sample);
      ImGui::Text(" - pos: %d",ch->audPos);
      ImGui::Text("- resVol: %.2x",ch->resVol);
      ImGui::Text("- macroVolMul: %.2x",ch->macroVolMul);
      COMMON_CHAN_DEBUG_BOOL;
      ImGui::TextColored(ch->volumeChanged?colorOn:colorOff,">> VolumeChanged");
      ImGui::TextColored(ch->setPos?colorOn:colorOff,">> SetPos");
      break;
    }
    case DIV_SYSTEM_SM8521: {
      DivPlatformSM8521::Channel* ch=(DivPlatformSM8521::Channel*)data;
      ImGui::Text("> SM8521");
      COMMON_CHAN_DEBUG;
      ImGui::Text("* AntiClick:");
      ImGui::Text(" - periodCount: %d",ch->antiClickPeriodCount);
      ImGui::Text(" - wavePos: %d",ch->antiClickWavePos);
      ImGui::Text("- wave: %d",ch->wave);
      COMMON_CHAN_DEBUG_BOOL;
      ImGui::TextColored(ch->volumeChanged?colorOn:colorOff,">> VolumeChanged");
      break;
    }
    case DIV_SYSTEM_PV1000: {
      DivPlatformPV1000::Channel* ch=(DivPlatformPV1000::Channel*)data;
      ImGui::Text("> PV1000");
      COMMON_CHAN_DEBUG;
      COMMON_CHAN_DEBUG_BOOL;
      break;
    }
    case DIV_SYSTEM_K053260: {
      DivPlatformK053260::Channel* ch=(DivPlatformK053260::Channel*)data;
      ImGui::Text("> K053260");
      COMMON_CHAN_DEBUG;
      ImGui::Text("* Sample: %d",ch->sample);
      ImGui::Text(" - pos: %d",ch->audPos);
      ImGui::Text("- panning: %d",ch->panning);
      ImGui::Text("- macroVolMul: %.2x",ch->macroVolMul);
      COMMON_CHAN_DEBUG_BOOL;
      ImGui::TextColored(ch->setPos?colorOn:colorOff,">> SetPos");
      ImGui::TextColored(ch->reverse?colorOn:colorOff,">> Reverse");
      break;
    }
    case DIV_SYSTEM_C140: {
      DivPlatformC140::Channel* ch=(DivPlatformC140::Channel*)data;
      ImGui::Text("> C140");
      COMMON_CHAN_DEBUG;
      ImGui::Text("* Sample: %d",ch->sample);
      ImGui::Text(" - pos: %d",ch->audPos);
      ImGui::Text("- chPanL: %.2x",ch->chPanL);
      ImGui::Text("- chPanR: %.2x",ch->chPanR);
      ImGui::Text("- chVolL: %.2x",ch->chVolL);
      ImGui::Text("- chVolR: %.2x",ch->chVolR);
      ImGui::Text("- macroVolMul: %.2x",ch->macroVolMul);
      ImGui::Text("- macroPanMul: %.2x",ch->macroPanMul);
      COMMON_CHAN_DEBUG_BOOL;
      ImGui::TextColored(ch->volChangedL?colorOn:colorOff,">> VolChangedL");
      ImGui::TextColored(ch->volChangedR?colorOn:colorOff,">> VolChangedR");
      ImGui::TextColored(ch->setPos?colorOn:colorOff,">> SetPos");
      break;
    }
    case DIV_SYSTEM_C219: {
      DivPlatformC219::Channel* ch=(DivPlatformC219::Channel*)data;
      ImGui::Text("> C219");
      COMMON_CHAN_DEBUG;
      ImGui::Text("* Sample: %d",ch->sample);
      ImGui::Text(" - pos: %d",ch->audPos);
      ImGui::Text("- chPanL: %.2x",ch->chPanL);
      ImGui::Text("- chPanR: %.2x",ch->chPanR);
      ImGui::Text("- chVolL: %.2x",ch->chVolL);
      ImGui::Text("- chVolR: %.2x",ch->chVolR);
      ImGui::Text("- macroVolMul: %.2x",ch->macroVolMul);
      ImGui::Text("- macroPanMul: %.2x",ch->macroPanMul);
      COMMON_CHAN_DEBUG_BOOL;
      ImGui::TextColored(ch->volChangedL?colorOn:colorOff,">> VolChangedL");
      ImGui::TextColored(ch->volChangedR?colorOn:colorOff,">> VolChangedR");
      ImGui::TextColored(ch->setPos?colorOn:colorOff,">> SetPos");
      ImGui::TextColored(ch->noise?colorOn:colorOff,">> Noise");
      ImGui::TextColored(ch->invLout?colorOn:colorOff,">> InvLout");
      ImGui::TextColored(ch->invSign?colorOn:colorOff,">> InvSign");
      break;
    }
    default:
      ImGui::Text("Unimplemented chip! Help!");
      break;
  }
}
