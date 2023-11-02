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
#include "../engine/platform/msm6295.h"
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
  ImGui::Text("I will finish later...");
}
void putDispatchChan(void* data, int chanNum, int type) {
  ImGui::Text("I will finish later...");
}
