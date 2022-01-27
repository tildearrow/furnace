#include "debug.h"
#include "imgui.h"
#include "../engine/platform/genesis.h"
#include "../engine/platform/genesisext.h"
#include "../engine/platform/sms.h"
#include "../engine/platform/gb.h"
#include "../engine/platform/pce.h"
#include "../engine/platform/nes.h"
#include "../engine/platform/c64.h"
#include "../engine/platform/arcade.h"
#include "../engine/platform/ym2610.h"
#include "../engine/platform/ym2610ext.h"
#include "../engine/platform/ay.h"
#include "../engine/platform/ay8930.h"
#include "../engine/platform/tia.h"
#include "../engine/platform/saa.h"
#include "../engine/platform/amiga.h"
#include "../engine/platform/dummy.h"

void putDispatchChan(void* data, int chanNum, int type) {
  ImVec4 colorOn=ImVec4(1.0f,1.0f,0.0f,1.0f);
  ImVec4 colorOff=ImVec4(0.3f,0.3f,0.3f,1.0f);
  switch (type) {
    case DIV_SYSTEM_GENESIS: {
      DivPlatformGenesis::Channel* ch=(DivPlatformGenesis::Channel*)data;
      ImGui::Text("> Genesis");
      ImGui::Text("- freqHL: %.2x%.2x",ch->freqH,ch->freqL);
      ImGui::Text("* freq: %d",ch->freq);
      ImGui::Text(" - base: %d",ch->baseFreq);
      ImGui::Text(" - pitch: %d",ch->pitch);
      ImGui::Text("- note: %d",ch->note);
      ImGui::Text("- ins: %d",ch->ins);
      ImGui::Text("- vol: %.2x",ch->vol);
      ImGui::Text("- outVol: %.2x",ch->outVol);
      ImGui::Text("- pan: %x",ch->pan);
      ImGui::TextColored(ch->active?colorOn:colorOff,">> Active");
      ImGui::TextColored(ch->insChanged?colorOn:colorOff,">> InsChanged");
      ImGui::TextColored(ch->freqChanged?colorOn:colorOff,">> FreqChanged");
      ImGui::TextColored(ch->keyOn?colorOn:colorOff,">> KeyOn");
      ImGui::TextColored(ch->keyOff?colorOn:colorOff,">> KeyOff");
      ImGui::TextColored(ch->portaPause?colorOn:colorOff,">> PortaPause");
      ImGui::TextColored(ch->furnaceDac?colorOn:colorOff,">> FurnaceDAC");
      ImGui::TextColored(ch->inPorta?colorOn:colorOff,">> InPorta");
      break;
    }
    case DIV_SYSTEM_SMS: {
      DivPlatformSMS::Channel* ch=(DivPlatformSMS::Channel*)data;
      ImGui::Text("> SMS");
      ImGui::Text("* freq: %d",ch->freq);
      ImGui::Text(" - base: %d",ch->baseFreq);
      ImGui::Text(" - pitch: %d",ch->pitch);
      ImGui::Text("- note: %d",ch->note);
      ImGui::Text("- ins: %d",ch->ins);
      ImGui::Text("- vol: %.2x",ch->vol);
      ImGui::Text("- outVol: %.2x",ch->outVol);
      ImGui::TextColored(ch->active?colorOn:colorOff,">> Active");
      ImGui::TextColored(ch->insChanged?colorOn:colorOff,">> InsChanged");
      ImGui::TextColored(ch->freqChanged?colorOn:colorOff,">> FreqChanged");
      ImGui::TextColored(ch->keyOn?colorOn:colorOff,">> KeyOn");
      ImGui::TextColored(ch->keyOff?colorOn:colorOff,">> KeyOff");
      break;
    }
    case DIV_SYSTEM_GB: {
      DivPlatformGB::Channel* ch=(DivPlatformGB::Channel*)data;
      ImGui::Text("> GameBoy");
      ImGui::Text("* freq: %d",ch->freq);
      ImGui::Text(" - base: %d",ch->baseFreq);
      ImGui::Text(" - pitch: %d",ch->pitch);
      ImGui::Text("- note: %d",ch->note);
      ImGui::Text("- ins: %d",ch->ins);
      ImGui::Text("- duty: %d",ch->duty);
      ImGui::Text("- sweep: %.2x",ch->sweep);
      ImGui::Text("- vol: %.2x",ch->vol);
      ImGui::Text("- outVol: %.2x",ch->outVol);
      ImGui::Text("- wave: %d",ch->wave);
      ImGui::TextColored(ch->active?colorOn:colorOff,">> Active");
      ImGui::TextColored(ch->insChanged?colorOn:colorOff,">> InsChanged");
      ImGui::TextColored(ch->freqChanged?colorOn:colorOff,">> FreqChanged");
      ImGui::TextColored(ch->sweepChanged?colorOn:colorOff,">> SweepChanged");
      ImGui::TextColored(ch->keyOn?colorOn:colorOff,">> KeyOn");
      ImGui::TextColored(ch->keyOff?colorOn:colorOff,">> KeyOff");
      ImGui::TextColored(ch->inPorta?colorOn:colorOff,">> InPorta");
      break;
    }
    case DIV_SYSTEM_PCE: {
      DivPlatformPCE::Channel* ch=(DivPlatformPCE::Channel*)data;
      ImGui::Text("> PCEngine");
      ImGui::Text("* freq: %d",ch->freq);
      ImGui::Text(" - base: %d",ch->baseFreq);
      ImGui::Text(" - pitch: %d",ch->pitch);
      ImGui::Text("- note: %d",ch->note);
      ImGui::Text("* DAC:");
      ImGui::Text(" - period: %d",ch->dacPeriod);
      ImGui::Text(" - rate: %d",ch->dacRate);
      ImGui::Text(" - pos: %d",ch->dacPos);
      ImGui::Text(" - sample: %d",ch->dacSample);
      ImGui::Text("- ins: %d",ch->ins);
      ImGui::Text("- pan: %.2x",ch->pan);
      ImGui::Text("- vol: %.2x",ch->vol);
      ImGui::Text("- outVol: %.2x",ch->outVol);
      ImGui::Text("- wave: %d",ch->wave);
      ImGui::TextColored(ch->active?colorOn:colorOff,">> Active");
      ImGui::TextColored(ch->insChanged?colorOn:colorOff,">> InsChanged");
      ImGui::TextColored(ch->freqChanged?colorOn:colorOff,">> FreqChanged");
      ImGui::TextColored(ch->keyOn?colorOn:colorOff,">> KeyOn");
      ImGui::TextColored(ch->keyOff?colorOn:colorOff,">> KeyOff");
      ImGui::TextColored(ch->inPorta?colorOn:colorOff,">> InPorta");
      ImGui::TextColored(ch->noise?colorOn:colorOff,">> Noise");
      ImGui::TextColored(ch->pcm?colorOn:colorOff,">> DAC");
      ImGui::TextColored(ch->furnaceDac?colorOn:colorOff,">> FurnaceDAC");
      break;
    }
    case DIV_SYSTEM_NES: {
      DivPlatformNES::Channel* ch=(DivPlatformNES::Channel*)data;
      ImGui::Text("> NES");
      ImGui::Text("* freq: %d",ch->freq);
      ImGui::Text(" - base: %d",ch->baseFreq);
      ImGui::Text(" - pitch: %d",ch->pitch);
      ImGui::Text(" - prev: %d",ch->prevFreq);
      ImGui::Text("- note: %d",ch->note);
      ImGui::Text("- ins: %d",ch->ins);
      ImGui::Text("- duty: %d",ch->duty);
      ImGui::Text("- sweep: %.2x",ch->sweep);
      ImGui::Text("- vol: %.2x",ch->vol);
      ImGui::Text("- outVol: %.2x",ch->outVol);
      ImGui::Text("- wave: %d",ch->wave);
      ImGui::TextColored(ch->active?colorOn:colorOff,">> Active");
      ImGui::TextColored(ch->insChanged?colorOn:colorOff,">> InsChanged");
      ImGui::TextColored(ch->freqChanged?colorOn:colorOff,">> FreqChanged");
      ImGui::TextColored(ch->sweepChanged?colorOn:colorOff,">> SweepChanged");
      ImGui::TextColored(ch->keyOn?colorOn:colorOff,">> KeyOn");
      ImGui::TextColored(ch->keyOff?colorOn:colorOff,">> KeyOff");
      ImGui::TextColored(ch->inPorta?colorOn:colorOff,">> InPorta");
      ImGui::TextColored(ch->furnaceDac?colorOn:colorOff,">> FurnaceDAC");
      break;
    }
    case DIV_SYSTEM_C64_6581: case DIV_SYSTEM_C64_8580: {
      DivPlatformC64::Channel* ch=(DivPlatformC64::Channel*)data;
      ImGui::Text("> C64");
      ImGui::Text("* freq: %d",ch->freq);
      ImGui::Text(" - base: %d",ch->baseFreq);
      ImGui::Text(" - pitch: %d",ch->pitch);
      ImGui::Text(" - prev: %d",ch->prevFreq);
      ImGui::Text("- testWhen: %d",ch->testWhen);
      ImGui::Text("- note: %d",ch->note);
      ImGui::Text("- ins: %d",ch->ins);
      ImGui::Text("- duty: %d",ch->duty);
      ImGui::Text("- sweep: %.2x",ch->sweep);
      ImGui::Text("- wave: %.1x",ch->wave);
      ImGui::Text("- ADSR: %.1x %.1x %.1x %.1x",ch->attack,ch->decay,ch->sustain,ch->release);
      ImGui::Text("- vol: %.2x",ch->vol);
      ImGui::Text("- outVol: %.2x",ch->outVol);
      ImGui::TextColored(ch->active?colorOn:colorOff,">> Active");
      ImGui::TextColored(ch->insChanged?colorOn:colorOff,">> InsChanged");
      ImGui::TextColored(ch->freqChanged?colorOn:colorOff,">> FreqChanged");
      ImGui::TextColored(ch->sweepChanged?colorOn:colorOff,">> SweepChanged");
      ImGui::TextColored(ch->keyOn?colorOn:colorOff,">> KeyOn");
      ImGui::TextColored(ch->keyOff?colorOn:colorOff,">> KeyOff");
      ImGui::TextColored(ch->inPorta?colorOn:colorOff,">> InPorta");
      ImGui::TextColored(ch->filter?colorOn:colorOff,">> Filter");
      ImGui::TextColored(ch->resetMask?colorOn:colorOff,">> ResetMask");
      ImGui::TextColored(ch->resetFilter?colorOn:colorOff,">> ResetFilter");
      ImGui::TextColored(ch->resetDuty?colorOn:colorOff,">> ResetDuty");
      ImGui::TextColored(ch->ring?colorOn:colorOff,">> Ring");
      ImGui::TextColored(ch->sync?colorOn:colorOff,">> Sync");
      break;
    }
    case DIV_SYSTEM_ARCADE: {
      DivPlatformArcade::Channel* ch=(DivPlatformArcade::Channel*)data;
      ImGui::Text("> Arcade");
      ImGui::Text("- freqHL: %.2x%.2x",ch->freqH,ch->freqL);
      ImGui::Text("* freq: %d",ch->freq);
      ImGui::Text(" - base: %d",ch->baseFreq);
      ImGui::Text(" - pitch: %d",ch->pitch);
      //ImGui::Text("- note: %d",ch->note);
      ImGui::Text("- ins: %d",ch->ins);
      ImGui::Text("- KOnCycles: %d",ch->konCycles);
      ImGui::Text("- vol: %.2x",ch->vol);
       //ImGui::Text("- outVol: %.2x",ch->outVol);
      ImGui::Text("- chVolL: %.2x",ch->chVolL);
      ImGui::Text("- chVolR: %.2x",ch->chVolR);
      ImGui::Text("* PCM:");
      ImGui::Text(" - sample: %d",ch->pcm.sample);
      ImGui::Text(" - pos: %d",ch->pcm.pos>>8);
      ImGui::Text(" - subPos: %d",ch->pcm.pos&0xff);
      ImGui::Text(" - len: %d",ch->pcm.len);
      ImGui::Text(" - freq: %.2x",ch->pcm.freq);
      ImGui::TextColored(ch->active?colorOn:colorOff,">> Active");
      ImGui::TextColored(ch->insChanged?colorOn:colorOff,">> InsChanged");
      ImGui::TextColored(ch->freqChanged?colorOn:colorOff,">> FreqChanged");
      ImGui::TextColored(ch->keyOn?colorOn:colorOff,">> KeyOn");
      ImGui::TextColored(ch->keyOff?colorOn:colorOff,">> KeyOff");
      ImGui::TextColored(ch->portaPause?colorOn:colorOff,">> PortaPause");
      ImGui::TextColored(ch->furnacePCM?colorOn:colorOff,">> FurnacePCM");
      //ImGui::TextColored(ch->inPorta?colorOn:colorOff,">> InPorta");
      break;
    }
    default:
      ImGui::Text("Unknown system! Help!");
      break;
  }
}