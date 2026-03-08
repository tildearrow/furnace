/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2026 tildearrow and contributors
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

#include "../engine/chipUtils.h"
#include "../engine/platform/gbaminmod.h"
#include "gui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <imgui.h>

bool FurnaceGUI::drawSysConf(int chan, int sysPos, DivSystem type, DivConfig& flags, bool modifyOnChange, bool fromMenu) {
  bool altered=false;
  bool mustRender=false;
  bool restart=modifyOnChange;
  bool supportsCustomRate=true;
  bool supportsChannelCount=(chan>=0);

  switch (type) {
    case DIV_SYSTEM_YM2612:
    case DIV_SYSTEM_YM2612_EXT: 
    case DIV_SYSTEM_YM2612_DUALPCM:
    case DIV_SYSTEM_YM2612_DUALPCM_EXT:
    case DIV_SYSTEM_YM2612_CSM: {
      int clockSel=flags.getInt("clockSel",0);
      int chipType=1;
      if (flags.has("chipType")) {
        chipType=flags.getInt("chipType",1);
      } else if (flags.has("ladderEffect")) {
        chipType=flags.getBool("ladderEffect",0)?1:0;
      }
      int interruptSimCycles=flags.getInt("interruptSimCycles",0);
      bool noExtMacros=flags.getBool("noExtMacros",false);
      bool fbAllOps=flags.getBool("fbAllOps",false);
      bool msw=flags.getBool("msw",false);

      ImGui::Text(_("Clock rate:"));
      ImGui::Indent();
      if (ImGui::RadioButton("NTSC (7.67MHz)",clockSel==0)) {
        clockSel=0;
        altered=true;
      }
      if (ImGui::RadioButton("PAL (7.61MHz)",clockSel==1)) {
        clockSel=1;
        altered=true;
      }
      if (ImGui::RadioButton("FM Towns (8MHz)",clockSel==2)) {
        clockSel=2;
        altered=true;
      }
      if (ImGui::RadioButton("AtGames Genesis (6.13MHz)",clockSel==3)) {
        clockSel=3;
        altered=true;
      }
      if (ImGui::RadioButton("Sega System 32 (8.05MHz)",clockSel==4)) {
        clockSel=4;
        altered=true;
      }
      ImGui::Unindent();

      ImGui::Text(_("Chip type:"));
      ImGui::Indent();
      if (ImGui::RadioButton(_("YM2612 (9-bit DAC with distortion)"),chipType==1)) {
        chipType=1;
        altered=true;
      }
      if (ImGui::RadioButton(_("YM3438 (9-bit DAC)"),chipType==0)) {
        chipType=0;
        altered=true;
      }
      if (ImGui::RadioButton(_("YMF276 (external DAC)"),chipType==2)) {
        chipType=2;
        altered=true;
      }
      ImGui::Unindent();

      if (type==DIV_SYSTEM_YM2612_EXT || type==DIV_SYSTEM_YM2612_DUALPCM_EXT || type==DIV_SYSTEM_YM2612_CSM) {
        if (ImGui::Checkbox(_("Disable ExtCh FM macros (compatibility)"),&noExtMacros)) {
          altered=true;
        }
        if (ImGui::Checkbox(_("Ins change in ExtCh operator 2-4 affects FB (compatibility)"),&fbAllOps)) {
          altered=true;
        }
      }

      if (msw || settings.mswEnabled) {
        if (ImGui::Checkbox(_("Modified sine wave (joke)"),&msw)) {
          altered=true;
        }
      }

      ImGui::Text(_("DAC interrupt simulation:"));
      if (CWSliderInt(_("cycles##InterruptSim"),&interruptSimCycles,0,256)) {
        if (interruptSimCycles<0) interruptSimCycles=0;
        if (interruptSimCycles>256) interruptSimCycles=256;
        altered=true;
      } rightClickable
      
      if (altered) {
        e->lockSave([&]() {
          flags.set("clockSel",clockSel);
          flags.set("chipType",chipType);
          flags.set("noExtMacros",noExtMacros);
          flags.set("fbAllOps",fbAllOps);
          flags.set("msw",msw);
          flags.set("interruptSimCycles",interruptSimCycles);
        });
      }
      break;
    }
    case DIV_SYSTEM_SMS: {
      int clockSel=flags.getInt("clockSel",0);
      int chipType=flags.getInt("chipType",0);
      bool noPhaseReset=flags.getBool("noPhaseReset",false);
      bool noEasyNoise=flags.getBool("noEasyNoise",false);

      ImGui::Text(_("Clock rate:"));
      ImGui::Indent();
      if (ImGui::RadioButton(_("3.58MHz (NTSC)"),clockSel==0)) {
        clockSel=0;
        altered=true;
      }
      if (ImGui::RadioButton(_("3.55MHz (PAL)"),clockSel==1)) {
        clockSel=1;
        altered=true;
      }
      if (ImGui::RadioButton(_("4MHz (BBC Micro)"),clockSel==2)) {
        clockSel=2;
        altered=true;
      }
      if (ImGui::RadioButton(_("1.79MHz (Half NTSC)"),clockSel==3)) {
        clockSel=3;
        altered=true;
      }
      if (ImGui::RadioButton(_("3MHz (Exed Exes)"),clockSel==4)) {
        clockSel=4;
        altered=true;
      }
      if (ImGui::RadioButton(_("2MHz (Sega System 1)"),clockSel==5)) {
        clockSel=5;
        altered=true;
      }
      if (ImGui::RadioButton(_("447KHz (TI-99/4A)"),clockSel==6)) {
        clockSel=6;
        altered=true;
      }
      ImGui::Unindent();
      ImGui::Text(_("Chip type:"));
      ImGui::Indent();
      if (ImGui::RadioButton(_("Sega VDP/Master System"),chipType==0)) {
        chipType=0;
        altered=true;
      }
      if (ImGui::RadioButton(_("TI SN76489"),chipType==1)) {
        chipType=1;
        altered=true;
      }
      if (ImGui::RadioButton(_("TI SN76489 with Atari-like short noise"),chipType==2)) {
        chipType=2;
        altered=true;
      }
      if (ImGui::RadioButton(_("Game Gear"),chipType==3)) {
        chipType=3;
        altered=true;
      }
      if (ImGui::RadioButton(_("TI SN76489A"),chipType==4)) {
        chipType=4;
        altered=true;
      }
      if (ImGui::RadioButton(_("TI SN76496"),chipType==5)) {
        chipType=5;
        altered=true;
      }
      if (ImGui::RadioButton(_("NCR 8496"),chipType==6)) {
        chipType=6;
        altered=true;
      }
      if (ImGui::RadioButton(_("Tandy PSSJ 3-voice sound"),chipType==7)) {
        chipType=7;
        altered=true;
      }
      if (ImGui::RadioButton(_("TI SN94624"),chipType==8)) {
        chipType=8;
        altered=true;
      }
      if (ImGui::RadioButton(_("TI SN76494"),chipType==9)) {
        chipType=9;
        altered=true;
      }
      ImGui::Unindent();

      if (ImGui::Checkbox(_("Disable noise period change phase reset"),&noPhaseReset)) {
        altered=true;
      }

      if (ImGui::Checkbox(_("Disable easy period to note mapping on upper octaves"),&noEasyNoise)) {
        altered=true;
      }

      if (altered) {
        e->lockSave([&]() {
          flags.set("clockSel",clockSel);
          flags.set("chipType",chipType);
          flags.set("noPhaseReset",noPhaseReset);
          flags.set("noEasyNoise",noEasyNoise);
        });
      }
      break;
    }
    case DIV_SYSTEM_PCE: {
      bool clockSel=flags.getInt("clockSel",0);
      int chipType=flags.getInt("chipType",0);
      bool noAntiClick=flags.getBool("noAntiClick",false);

      if (ImGui::Checkbox(_("Pseudo-PAL"),&clockSel)) {
        altered=true;
      }
      if (ImGui::Checkbox(_("Disable anti-click"),&noAntiClick)) {
        altered=true;
      }
      ImGui::Text(_("Chip revision:"));
      ImGui::Indent();
      if (ImGui::RadioButton(_("HuC6280 (original)"),chipType==0)) {
        chipType=0;
        altered=true;
      }
      if (ImGui::RadioButton(_("HuC6280A (SuperGrafx)"),chipType==1)) {
        chipType=1;
        altered=true;
      }
      ImGui::Unindent();

      if (altered) {
        e->lockSave([&]() {
          flags.set("clockSel",(int)clockSel);
          flags.set("chipType",chipType);
          flags.set("noAntiClick",noAntiClick);
        });
      }
      break;
    }
    case DIV_SYSTEM_SOUND_UNIT: {
      int clockSel=flags.getInt("clockSel",0);
      bool echo=flags.getBool("echo",false);
      bool swapEcho=flags.getBool("swapEcho",false);
      int sampleMemSize=flags.getInt("sampleMemSize",0);
      bool pdm=flags.getBool("pdm",false);
      int echoDelay=flags.getInt("echoDelay",0);
      int echoFeedback=flags.getInt("echoFeedback",0);
      int echoResolution=flags.getInt("echoResolution",0);
      int echoVol=(signed char)flags.getInt("echoVol",0);

      ImGui::Text(_("CPU rate:"));
      ImGui::Indent();
      if (ImGui::RadioButton(_("6.18MHz (NTSC)"),clockSel==0)) {
        clockSel=0;
        altered=true;
      }
      if (ImGui::RadioButton(_("5.95MHz (PAL)"),clockSel==1)) {
        clockSel=1;
        altered=true;
      }
      ImGui::Unindent();
      ImGui::Text(_("Sample memory:"));
      ImGui::Indent();
      if (ImGui::RadioButton(_("8K (rev A/B/E)"),sampleMemSize==0)) {
        sampleMemSize=0;
        altered=true;
      }
      if (ImGui::RadioButton(_("64K (rev D/F)"),sampleMemSize==1)) {
        sampleMemSize=1;
        altered=true;
      }
      ImGui::Unindent();
      ImGui::Text(_("DAC resolution:"));
      ImGui::Indent();
      if (ImGui::RadioButton(_("16-bit (rev A/B/D/F)"),pdm==0)) {
        pdm=false;
        altered=true;
      }
      if (ImGui::RadioButton(_("8-bit + TDM (rev C/E)"),pdm==1)) {
        pdm=true;
        altered=true;
      }
      ImGui::Unindent();
      if (ImGui::Checkbox(_("Enable echo"),&echo)) {
        altered=true;
      }
      if (ImGui::Checkbox(_("Swap echo channels"),&swapEcho)) {
        altered=true;
      }
      ImGui::Text(_("Echo delay:"));
      if (CWSliderInt("##EchoBufSize",&echoDelay,0,63)) {
        if (echoDelay<0) echoDelay=0;
        if (echoDelay>63) echoDelay=63;
        altered=true;
      } rightClickable
      ImGui::Text(_("Echo resolution:"));
      if (CWSliderInt("##EchoResolution",&echoResolution,0,15)) {
        if (echoResolution<0) echoResolution=0;
        if (echoResolution>15) echoResolution=15;
        altered=true;
      } rightClickable
      ImGui::Text(_("Echo feedback:"));
      if (CWSliderInt("##EchoFeedback",&echoFeedback,0,15)) {
        if (echoFeedback<0) echoFeedback=0;
        if (echoFeedback>15) echoFeedback=15;
        altered=true;
      } rightClickable
      ImGui::Text(_("Echo volume:"));
      if (CWSliderInt("##EchoVolume",&echoVol,-128,127)) {
        if (echoVol<-128) echoVol=-128;
        if (echoVol>127) echoVol=127;
        altered=true;
      } rightClickable

      if (altered) {
        e->lockSave([&]() {
          flags.set("clockSel",clockSel);
          flags.set("echo",echo);
          flags.set("swapEcho",swapEcho);
          flags.set("sampleMemSize",sampleMemSize);
          flags.set("pdm",pdm);
          flags.set("echoDelay",echoDelay);
          flags.set("echoFeedback",echoFeedback);
          flags.set("echoResolution",echoResolution);
          flags.set("echoVol",(unsigned char)echoVol);
        });
      }
      break;
    }
    case DIV_SYSTEM_GB: {
      int chipType=flags.getInt("chipType",0);
      bool noAntiClick=flags.getBool("noAntiClick",false);
      bool invertWave=flags.getBool("invertWave",true);
      bool enoughAlready=flags.getBool("enoughAlready",false);

      if (ImGui::Checkbox(_("Disable anti-click"),&noAntiClick)) {
        altered=true;
      }
      ImGui::Text(_("Chip revision:"));
      ImGui::Indent();
      if (ImGui::RadioButton(_("Original (DMG)"),chipType==0)) {
        chipType=0;
        altered=true;
      }
      if (ImGui::RadioButton(_("Game Boy Color (rev C)"),chipType==1)) {
        chipType=1;
        altered=true;
      }
      if (ImGui::RadioButton(_("Game Boy Color (rev E)"),chipType==2)) {
        chipType=2;
        altered=true;
      }
      if (ImGui::RadioButton(_("Game Boy Advance"),chipType==3)) {
        chipType=3;
        altered=true;
      }
      ImGui::Unindent();
      ImGui::Text(_("Wave channel orientation:"));
      if (chipType==3) {
        ImGui::Indent();
        if (ImGui::RadioButton(_("Normal"),!invertWave)) {
          invertWave=false;
          altered=true;
        }
        if (ImGui::RadioButton(_("Inverted"),invertWave)) {
          invertWave=true;
          altered=true;
        }
        ImGui::Unindent();
      } else {
        ImGui::Indent();
        if (ImGui::RadioButton(_("Exact data (inverted)"),!invertWave)) {
          invertWave=false;
          altered=true;
        }
        if (ImGui::RadioButton(_("Exact output (normal)"),invertWave)) {
          invertWave=true;
          altered=true;
        }
        ImGui::Unindent();
      }
      if (enoughAlready) {
        if (ImGui::Checkbox(_("Pretty please one more compat flag when I use arpeggio and my sound length"),&enoughAlready)) {
          altered=true;
        }
      }

      if (altered) {
        e->lockSave([&]() {
          flags.set("chipType",chipType);
          flags.set("noAntiClick",noAntiClick);
          flags.set("invertWave",invertWave);
          flags.set("enoughAlready",enoughAlready);
        });
      }
      break;
    }
    case DIV_SYSTEM_GBA_DMA: {
      int dacDepth=flags.getInt("dacDepth",9);

      ImGui::Text(_("DAC bit depth (reduces output rate):"));
      if (CWSliderInt("##DACDepth",&dacDepth,6,9)) {
        if (dacDepth<6) dacDepth=6;
        if (dacDepth>9) dacDepth=9;
        altered=true;
      }

      if (altered) {
        e->lockSave([&]() {
          flags.set("dacDepth",dacDepth);
        });
      }
      break;
    }
    case DIV_SYSTEM_GBA_MINMOD: {
      supportsCustomRate=false;
      int volScale=flags.getInt("volScale",4096);
      int mixBufs=flags.getInt("mixBufs",15);
      int dacDepth=flags.getInt("dacDepth",9);
      int channels=flags.getInt("channels",16);
      int sampRate=flags.getInt("sampRate",21845);
      ImGui::Text(_("Volume scale:"));
      if (CWSliderInt("##VolScale",&volScale,0,32768)) {
        if (volScale<0) volScale=0;
        if (volScale>32768) volScale=32768;
        altered=true;
      } rightClickable
      ImGui::Text(_("Mix buffers (allows longer echo delay):"));
      if (CWSliderInt("##MixBufs",&mixBufs,2,15)) {
        if (mixBufs<2) mixBufs=2;
        if (mixBufs>16) mixBufs=16;
        altered=true;
      } rightClickable
      ImGui::Text(_("DAC bit depth (reduces output rate):"));
      if (CWSliderInt("##DACDepth",&dacDepth,6,9)) {
        if (dacDepth<6) dacDepth=6;
        if (dacDepth>9) dacDepth=9;
        altered=true;
      } rightClickable
      ImGui::Text(_("Channel limit:"));
      if (CWSliderInt("##Channels",&channels,1,16)) {
        if (channels<1) channels=1;
        if (channels>16) channels=16;
        altered=true;
      } rightClickable
      ImGui::Text(_("Sample rate:"));
      if (CWSliderInt("##SampRate",&sampRate,256,65536)) {
        if (sampRate<1) sampRate=21845;
        if (sampRate>65536) sampRate=65536;
        altered=true;
      } rightClickable
      if (chan>=0) {
        DivPlatformGBAMinMod* dispatch=(DivPlatformGBAMinMod*)e->getDispatch(chan);
        if (dispatch!=NULL) {
          float maxCPU=dispatch->maxCPU*100;
          ImGui::Text(_("Actual sample rate: %d Hz"), dispatch->chipClock);
          if (maxCPU>90) ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_WARNING]);
          ImGui::Text(_("Max mixer CPU usage: %.0f%%"),maxCPU);
          if (maxCPU>90) ImGui::PopStyleColor();
          FurnaceGUI::popWarningColor();
        }
      }
      if (altered) {
        e->lockSave([&]() {
          flags.set("volScale",volScale);
          flags.set("mixBufs",mixBufs);
          flags.set("dacDepth",dacDepth);
          flags.set("channels",channels);
          flags.set("sampRate",sampRate);
        });
      }
      break;
    }
    case DIV_SYSTEM_OPLL:
    case DIV_SYSTEM_OPLL_DRUMS:
    case DIV_SYSTEM_VRC7: {
      int clockSel=flags.getInt("clockSel",0);
      int patchSet=flags.getInt("patchSet",0);
      bool noTopHatFreq=flags.getBool("noTopHatFreq",false);
      bool fixedAll=flags.getBool("fixedAll",true);

      ImGui::Text(_("Clock rate:"));
      ImGui::Indent();
      if (ImGui::RadioButton(_("NTSC (3.58MHz)"),clockSel==0)) {
        clockSel=0;
        altered=true;
      }
      if (ImGui::RadioButton(_("PAL (3.55MHz)"),clockSel==1)) {
        clockSel=1;
        altered=true;
      }
      if (ImGui::RadioButton(_("Arcade (4MHz)"),clockSel==2)) {
        clockSel=2;
        altered=true;
      }
      if (ImGui::RadioButton(_("Half NTSC (1.79MHz)"),clockSel==3)) {
        clockSel=3;
        altered=true;
      }
      ImGui::Unindent();
      if (type!=DIV_SYSTEM_VRC7) {
        ImGui::Text(_("Patch set:"));
        ImGui::Indent();
        if (ImGui::RadioButton("Yamaha YM2413",patchSet==0)) {
          patchSet=0;
          altered=true;
        }
        if (ImGui::RadioButton("Yamaha YMF281",patchSet==1)) {
          patchSet=1;
          altered=true;
        }
        if (ImGui::RadioButton("Yamaha YM2423",patchSet==2)) {
          patchSet=2;
          altered=true;
        }
        if (ImGui::RadioButton("Konami VRC7",patchSet==3)) {
          patchSet=3;
          altered=true;
        }
        ImGui::Unindent();
      }

      if (type==DIV_SYSTEM_OPLL_DRUMS) {
        if (ImGui::Checkbox(_("Ignore top/hi-hat frequency changes"),&noTopHatFreq)) {
          altered=true;
        }
        if (ImGui::Checkbox(_("Apply fixed frequency to all drums at once"),&fixedAll)) {
          altered=true;
        }
      }

      if (altered) {
        e->lockSave([&]() {
          flags.set("clockSel",clockSel);
          if (type!=DIV_SYSTEM_VRC7) {
            flags.set("patchSet",patchSet);
          }
          flags.set("noTopHatFreq",noTopHatFreq);
          flags.set("fixedAll",fixedAll);
        });
      }
      break;
    }
    case DIV_SYSTEM_YM2151: {
      int clockSel=flags.getInt("clockSel",0);
      bool brokenPitch=flags.getBool("brokenPitch",false);

      ImGui::Indent();
      if (ImGui::RadioButton(_("NTSC/X16 (3.58MHz)"),clockSel==0)) {
        clockSel=0;
        altered=true;
      }
      if (ImGui::RadioButton(_("PAL (3.55MHz)"),clockSel==1)) {
        clockSel=1;
        altered=true;
      }
      if (ImGui::RadioButton(_("X1/X68000 (4MHz)"),clockSel==2)) {
        clockSel=2;
        altered=true;
      }
      ImGui::Unindent();

      if (ImGui::Checkbox(_("Broken pitch macro/slides (compatibility)"),&brokenPitch)) {
        altered=true;
      }

      if (altered) {
        e->lockSave([&]() {
          flags.set("clockSel",clockSel);
          flags.set("brokenPitch",brokenPitch);
        });
      }
      break;
    }
    case DIV_SYSTEM_OPZ: {
      bool clockSel=flags.getInt("clockSel",0);
      bool brokenPitch=flags.getBool("brokenPitch",false);

      if (ImGui::Checkbox(_("Pseudo-PAL"),&clockSel)) {
        altered=true;
      }

      if (ImGui::Checkbox(_("Broken pitch macro/slides (compatibility)"),&brokenPitch)) {
        altered=true;
      }

      if (altered) {
        e->lockSave([&]() {
          flags.set("clockSel",(int)clockSel);
          flags.set("brokenPitch",brokenPitch);
        });
      }
      break;
    }
    case DIV_SYSTEM_NES:
    case DIV_SYSTEM_5E01: {
      int clockSel=flags.getInt("clockSel",0);
      bool dpcmMode=flags.getBool("dpcmMode",true);
      bool resetSweep=flags.getBool("resetSweep",false);

      ImGui::Text(_("Clock rate:"));

      ImGui::Indent();
      if (ImGui::RadioButton(_("NTSC (1.79MHz)"),clockSel==0)) {
        clockSel=0;
        altered=true;
      }
      if (ImGui::RadioButton(_("PAL (1.67MHz)"),clockSel==1)) {
        clockSel=1;
        altered=true;
      }
      if (ImGui::RadioButton(_("Dendy (1.77MHz)"),clockSel==2)) {
        clockSel=2;
        altered=true;
      }
      ImGui::Unindent();

      ImGui::Text(_("DPCM channel mode:"));

      ImGui::Indent();
      if (ImGui::RadioButton(_("DPCM (muffled samples; low CPU usage)"),dpcmMode)) {
        dpcmMode=true;
        altered=true;
      }
      if (ImGui::RadioButton(_("PCM (crisp samples; high CPU usage)"),!dpcmMode)) {
        dpcmMode=false;
        altered=true;
      }
      ImGui::Unindent();

      if (ImGui::Checkbox(_("Reset sweep on new note"),&resetSweep)) {
        altered=true;
      }

      if (altered) {
        e->lockSave([&]() {
          flags.set("clockSel",clockSel);
          flags.set("dpcmMode",dpcmMode);
          flags.set("resetSweep",resetSweep);
        });
      }
      break;
    }
    case DIV_SYSTEM_VRC6:
    case DIV_SYSTEM_FDS:
    case DIV_SYSTEM_MMC5: {
      int clockSel=flags.getInt("clockSel",0);

      ImGui::Text(_("Clock rate:"));

      ImGui::Indent();
      if (ImGui::RadioButton(_("NTSC (1.79MHz)"),clockSel==0)) {
        clockSel=0;
        altered=true;
      }
      if (ImGui::RadioButton(_("PAL (1.67MHz)"),clockSel==1)) {
        clockSel=1;
        altered=true;
      }
      if (ImGui::RadioButton(_("Dendy (1.77MHz)"),clockSel==2)) {
        clockSel=2;
        altered=true;
      }
      ImGui::Unindent();

      if (altered) {
        e->lockSave([&]() {
          flags.set("clockSel",clockSel);
        });
      }
      break;
    }
    case DIV_SYSTEM_C64_8580:
    case DIV_SYSTEM_C64_6581:
    case DIV_SYSTEM_C64_PCM: {
      int clockSel=flags.getInt("clockSel",0);
      bool keyPriority=flags.getBool("keyPriority",true);
      bool no1EUpdate=flags.getBool("no1EUpdate",false);
      bool multiplyRel=flags.getBool("multiplyRel",false);
      bool macroRace=flags.getBool("macroRace",false);
      int testAttack=flags.getInt("testAttack",0);
      int testDecay=flags.getInt("testDecay",0);
      int testSustain=flags.getInt("testSustain",0);
      int testRelease=flags.getInt("testRelease",0);
      int initResetTime=flags.getInt("initResetTime",2);

      ImGui::Text(_("Clock rate:"));

      ImGui::Indent();
      if (ImGui::RadioButton(_("NTSC (1.02MHz)"),clockSel==0)) {
        clockSel=0;
        altered=true;
      }
      if (ImGui::RadioButton(_("PAL (0.99MHz)"),clockSel==1)) {
        clockSel=1;
        altered=true;
      }
      if (ImGui::RadioButton(_("SSI 2001 (0.89MHz)"),clockSel==2)) {
        clockSel=2;
        altered=true;
      }
      ImGui::Unindent();

      ImGui::Text(_("Global parameter priority:"));

      ImGui::Indent();
      if (ImGui::RadioButton(_("Left to right"),!keyPriority)) {
        keyPriority=false;
        altered=true;
      }
      if (ImGui::RadioButton(_("Last used channel"),keyPriority)) {
        keyPriority=true;
        altered=true;
      }
      ImGui::Unindent();

      ImGui::Text(_("Hard reset envelope:"));

      if (CWSliderInt(_("Attack"),&testAttack,0,15)) {
        if (testAttack<0) testAttack=0;
        if (testAttack>15) testAttack=15;
        altered=true;
      }
      if (CWSliderInt(_("Decay"),&testDecay,0,15)) {
        if (testDecay<0) testDecay=0;
        if (testDecay>15) testDecay=15;
        altered=true;
      }
      if (CWSliderInt(_("Sustain"),&testSustain,0,15)) {
        if (testSustain<0) testSustain=0;
        if (testSustain>15) testSustain=15;
        altered=true;
      }
      if (CWSliderInt(_("Release"),&testRelease,0,15)) {
        if (testRelease<0) testRelease=0;
        if (testRelease>15) testRelease=15;
        altered=true;
      }

      ImGui::Text(_("Envelope reset time:"));

      pushWarningColor(initResetTime<1 || initResetTime>4);
      if (CWSliderInt("##InitReset",&initResetTime,0,16)) {
        if (initResetTime<0) initResetTime=0;
        if (initResetTime>16) initResetTime=16;
        altered=true;
      }
      popWarningColor();
      
      ImGui::Text(_("- 0 disables envelope reset. not recommended!\n- 1 may trigger SID envelope bugs.\n- values that are too high may result in notes being skipped."));

      if (ImGui::Checkbox(_("Disable 1Exy env update (compatibility)"),&no1EUpdate)) {
        altered=true;
      }

      if (multiplyRel) {
        if (ImGui::Checkbox(_("Relative duty and cutoff macros are coarse (compatibility)"),&multiplyRel)) {
          altered=true;
        }
      }

      if (ImGui::Checkbox(_("Cutoff macro race conditions (compatibility)"),&macroRace)) {
        altered=true;
      }


      if (altered) {
        e->lockSave([&]() {
          flags.set("clockSel",clockSel);
          flags.set("keyPriority",keyPriority);
          flags.set("no1EUpdate",no1EUpdate);
          flags.set("multiplyRel",multiplyRel);
          flags.set("macroRace",macroRace);
          flags.set("testAttack",testAttack);
          flags.set("testDecay",testDecay);
          flags.set("testSustain",testSustain);
          flags.set("testRelease",testRelease);
          flags.set("initResetTime",initResetTime);
        });
      }
      break;
    }
    case DIV_SYSTEM_YM2610_CSM:
    case DIV_SYSTEM_YM2610_FULL:
    case DIV_SYSTEM_YM2610_FULL_EXT:
    case DIV_SYSTEM_YM2610B:
    case DIV_SYSTEM_YM2610B_EXT:
    case DIV_SYSTEM_YM2610B_CSM: {
      int clockSel=flags.getInt("clockSel",0);
      bool noExtMacros=flags.getBool("noExtMacros",false);
      bool fbAllOps=flags.getBool("fbAllOps",false);
      int ssgVol=flags.getInt("ssgVol",128);
      int fmVol=flags.getInt("fmVol",256);
      bool hasSharedAdpcmBus=flags.getBool("hasSharedAdpcmBus",false);

      ImGui::Indent();
      if (ImGui::RadioButton(_("8MHz (Neo Geo MVS)"),clockSel==0)) {
        clockSel=0;
        altered=true;
      }
      if (ImGui::RadioButton(_("8.06MHz (Neo Geo AES)"),clockSel==1)) {
        clockSel=1;
        altered=true;
      }
      ImGui::Unindent();

      if (type==DIV_SYSTEM_YM2610_FULL_EXT || type==DIV_SYSTEM_YM2610B_EXT || type==DIV_SYSTEM_YM2610_CSM || type==DIV_SYSTEM_YM2610B_CSM) {
        if (ImGui::Checkbox(_("Disable ExtCh FM macros (compatibility)"),&noExtMacros)) {
          altered=true;
        }
        if (ImGui::Checkbox(_("Ins change in ExtCh operator 2-4 affects FB (compatibility)"),&fbAllOps)) {
          altered=true;
        }
      }

      if (CWSliderInt(_("SSG Volume"),&ssgVol,0,256)) {
        if (ssgVol<0) ssgVol=0;
        if (ssgVol>256) ssgVol=256;
        altered=true;
      } rightClickable

      if (CWSliderInt(_("FM/ADPCM Volume"),&fmVol,0,256)) {
        if (fmVol<0) fmVol=0;
        if (fmVol>256) fmVol=256;
        altered=true;
      } rightClickable

      if (ImGui::Checkbox(_("Use shared ADPCM memory space"),&hasSharedAdpcmBus)) {
        altered=true;
        mustRender=true;
      }

      if (altered) {
        e->lockSave([&]() {
          flags.set("clockSel",clockSel);
          flags.set("noExtMacros",noExtMacros);
          flags.set("fbAllOps",fbAllOps);
          flags.set("ssgVol",ssgVol);
          flags.set("fmVol",fmVol);
          flags.set("hasSharedAdpcmBus",hasSharedAdpcmBus);
        });
      }
      break;
    }
    case DIV_SYSTEM_AY8910:
    case DIV_SYSTEM_AY8930: {
      int clockSel=flags.getInt("clockSel",0);
      int chipType=flags.getInt("chipType",0);
      bool halfClock=flags.getBool("halfClock",false);
      bool stereo=flags.getBool("stereo",false);
      int stereoSep=flags.getInt("stereoSep",0);

      ImGui::Text(_("Clock rate:"));
      ImGui::Indent();
      if (ImGui::RadioButton(_("1.79MHz (ZX Spectrum NTSC/MSX)"),clockSel==0)) {
        clockSel=0;
        altered=true;
      }
      if (ImGui::RadioButton(_("1.77MHz (ZX Spectrum PAL)"),clockSel==1)) {
        clockSel=1;
        altered=true;
      }
      if (ImGui::RadioButton(_("0.83MHz (Pre-divided Sunsoft 5B on PAL)"),clockSel==8)) {
        clockSel=8;
        altered=true;
      }
      if (ImGui::RadioButton(_("0.89MHz (Pre-divided Sunsoft 5B)"),clockSel==6)) {
        clockSel=6;
        altered=true;
      }
      if (ImGui::RadioButton(_("1MHz (Amstrad CPC)"),clockSel==5)) {
        clockSel=5;
        altered=true;
      }
      if (ImGui::RadioButton(_("1.10MHz (Gamate/VIC-20 PAL)"),clockSel==9)) {
        clockSel=9;
        altered=true;
      }
      if (ImGui::RadioButton(_("1.25MHz (Mag Max)"),clockSel==13)) {
        clockSel=13;
        altered=true;
      }
      if (ImGui::RadioButton(_("1.5MHz (Vectrex)"),clockSel==4)) {
        clockSel=4;
        altered=true;
      }
      if (ImGui::RadioButton(_("1.536MHz (Kyugo)"),clockSel==14)) {
        clockSel=14;
        altered=true;
      }
      if (ImGui::RadioButton(_("1.67MHz (?)"),clockSel==7)) {
        clockSel=7;
        altered=true;
      }
      if (ImGui::RadioButton(_("1.75MHz (ZX Spectrum 48K)"),clockSel==2)) {
        clockSel=2;
        altered=true;
      }
      if (ImGui::RadioButton(_("1.99MHz (PC-88)"),clockSel==15)) {
        clockSel=15;
        altered=true;
      }
      if (ImGui::RadioButton(_("2MHz (Atari ST/Sharp X1)"),clockSel==3)) {
        clockSel=3;
        altered=true;
      }
      if (ImGui::RadioButton(_("2^21Hz (Game Boy)"),clockSel==10)) {
        clockSel=10;
        altered=true;
      }
      if (ImGui::RadioButton(_("3.58MHz (Darky)"),clockSel==11)) {
        clockSel=11;
        altered=true;
      }
      if (ImGui::RadioButton(_("3.6MHz (Darky)"),clockSel==12)) {
        clockSel=12;
        altered=true;
      }
      ImGui::Unindent();
      if (type==DIV_SYSTEM_AY8910) {
        ImGui::Text(_("Chip type:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("AY-3-8910"),chipType==0)) {
          chipType=0;
          altered=true;
        }
        if (ImGui::RadioButton(_("YM2149(F)"),chipType==1)) {
          chipType=1;
          altered=true;
        }
        if (ImGui::RadioButton(_("Sunsoft 5B"),chipType==2)) {
          chipType=2;
          altered=true;
        }
        pushWarningColor(chipType==3 && settings.ayCore==1);
        if (ImGui::RadioButton(_("AY-3-8914"),chipType==3)) {
          chipType=3;
          altered=true;
        }
        popWarningColor();
        ImGui::Unindent();
        if (ImGui::IsItemHovered()) {
          if (ImGui::BeginTooltip()) {
            ImGui::TextUnformatted(_("note: AY-3-8914 is not supported by the VGM format!"));
            if (settings.ayCore==1) {
              ImGui::TextUnformatted(_("AtomicSSG will not emulate AY-3-8914. falling back to MAME!"));
            }
            ImGui::EndTooltip();
          }
        }
      }
      ImGui::BeginDisabled(type==DIV_SYSTEM_AY8910 && chipType==2);
      if (ImGui::Checkbox(_("Stereo##_AY_STEREO"),&stereo)) {
        altered=true;
      }
      if (stereo) {
        int sep=256-(stereoSep&255);
        if (CWSliderInt(_("Separation"),&sep,1,256)) {
          if (sep<1) sep=1;
          if (sep>256) sep=256;
          stereoSep=256-sep;
          altered=true;
        }
      }
      ImGui::EndDisabled();
      ImGui::BeginDisabled(type==DIV_SYSTEM_AY8910 && chipType!=1);
      if (ImGui::Checkbox(_("Half Clock divider##_AY_CLKSEL"),&halfClock)) {
        altered=true;
      }
      ImGui::EndDisabled();

      if (altered) {
        e->lockSave([&]() {
          flags.set("clockSel",clockSel);
          if (type==DIV_SYSTEM_AY8910) {
            flags.set("chipType",chipType);
          }
          flags.set("halfClock",halfClock);
          flags.set("stereo",stereo);
          flags.set("stereoSep",stereoSep);
        });
      }
      break;
    }
    case DIV_SYSTEM_SAA1099: {
      int clockSel=flags.getInt("clockSel",0);

      ImGui::Text(_("Clock rate:"));
      ImGui::Indent();
      if (ImGui::RadioButton(_("SAM Coupé (8MHz)"),clockSel==0)) {
        clockSel=0;
        altered=true;
      }
      if (ImGui::RadioButton(_("NTSC (7.15MHz)"),clockSel==1)) {
        clockSel=1;
        altered=true;
      }
      if (ImGui::RadioButton(_("PAL (7.09MHz)"),clockSel==2)) {
        clockSel=2;
        altered=true;
      }
      ImGui::Unindent();

      if (altered) {
        e->lockSave([&]() {
          flags.set("clockSel",clockSel);
        });
      }
      break;
    }
    case DIV_SYSTEM_AMIGA: {
      bool clockSel=flags.getInt("clockSel",0);
      int chipType=flags.getInt("chipType",0);
      int chipMem=flags.getInt("chipMem",21);
      int stereoSep=flags.getInt("stereoSep",0);
      bool bypassLimits=flags.getBool("bypassLimits",false);

      ImGui::Text(_("Stereo separation:"));
      if (CWSliderInt("##StereoSep",&stereoSep,0,127)) {
        if (stereoSep<0) stereoSep=0;
        if (stereoSep>127) stereoSep=127;
        altered=true;
      } rightClickable

      ImGui::Text(_("Model:"));
      ImGui::Indent();
      if (ImGui::RadioButton(_("Amiga 500 (OCS)"),chipType==0)) {
        chipType=0;
        altered=true;
      }
      if (ImGui::RadioButton(_("Amiga 1200 (AGA)"),chipType==1)) {
        chipType=1;
        altered=true;
      }
      ImGui::Unindent();

      ImGui::Text(_("Chip memory:"));
      ImGui::Indent();
      if (ImGui::RadioButton(_("2MB (ECS/AGA max)"),chipMem==21)) {
        chipMem=21;
        altered=true;
        mustRender=true;
      }
      if (ImGui::RadioButton("1MB",chipMem==20)) {
        chipMem=20;
        altered=true;
        mustRender=true;
      }
      if (ImGui::RadioButton(_("512KB (OCS max)"),chipMem==19)) {
        chipMem=19;
        altered=true;
        mustRender=true;
      }
      if (ImGui::RadioButton("256KB",chipMem==18)) {
        chipMem=18;
        altered=true;
        mustRender=true;
      }
      ImGui::Unindent();


      if (ImGui::Checkbox(_("PAL"),&clockSel)) {
        altered=true;
      }
      if (ImGui::Checkbox(_("Bypass frequency limits"),&bypassLimits)) {
        altered=true;
      }

      if (altered) {
        e->lockSave([&]() {
          flags.set("clockSel",(int)clockSel);
          flags.set("chipType",chipType);
          flags.set("chipMem",chipMem);
          flags.set("stereoSep",stereoSep);
          flags.set("bypassLimits",bypassLimits);
        });
      }
      break;
    }
    case DIV_SYSTEM_TIA: {
      bool clockSel=flags.getInt("clockSel",0);
      int mixingType=flags.getInt("mixingType",0);
      bool softwarePitch=flags.getBool("softwarePitch",false);
      bool oldPitch=flags.getBool("oldPitch",false);

      ImGui::BeginDisabled(oldPitch);
      if (ImGui::Checkbox(_("Software pitch driver"),&softwarePitch)) {
        altered=true;
      }
      ImGui::EndDisabled();
      if (ImGui::Checkbox(_("Old pitch table (compatibility)"),&oldPitch)) {
        if (oldPitch) softwarePitch=false;
        altered=true;
      }

      ImGui::Text(_("Mixing mode:"));
      ImGui::Indent();
      if (ImGui::RadioButton(_("Mono"),mixingType==0)) {
        mixingType=0;
        altered=true;
      }
      if (ImGui::RadioButton(_("Mono (no distortion)"),mixingType==1)) {
        mixingType=1;
        altered=true;
      }
      if (ImGui::RadioButton(_("Stereo"),mixingType==2)) {
        mixingType=2;
        altered=true;
      }
      ImGui::Unindent();

      if (ImGui::Checkbox(_("PAL"),&clockSel)) {
        altered=true;
      }

      if (altered) {
        e->lockSave([&]() {
          flags.set("clockSel",(int)clockSel);
          flags.set("mixingType",mixingType);
          flags.set("softwarePitch",softwarePitch);
          flags.set("oldPitch",oldPitch);
        });
      }
      break;
    }
    case DIV_SYSTEM_PCSPKR: {
      int clockSel=flags.getInt("clockSel",0);
      int speakerType=flags.getInt("speakerType",0);
      bool resetPhase=flags.getBool("resetPhase",false);

      ImGui::Text(_("Clock rate:"));
      ImGui::Indent();
      if (ImGui::RadioButton(_("1.19MHz (PC)"),clockSel==0)) {
        clockSel=0;
        altered=true;
      }
      if (ImGui::RadioButton(_("1.99MHz (PC-98)"),clockSel==1)) {
        clockSel=1;
        altered=true;
      }
      if (ImGui::RadioButton(_("2.46MHz (PC-98)"),clockSel==2)) {
        clockSel=2;
        altered=true;
      }
      ImGui::Unindent();

      ImGui::Text(_("Speaker type:"));
      ImGui::Indent();
      if (ImGui::RadioButton(_("Unfiltered"),speakerType==0)) {
        speakerType=0;
        altered=true;
      }
      if (ImGui::RadioButton(_("Cone"),speakerType==1)) {
        speakerType=1;
        altered=true;
      }
      if (ImGui::RadioButton(_("Piezo"),speakerType==2)) {
        speakerType=2;
        altered=true;
      }
      if (ImGui::RadioButton(_("Use system beeper (Linux only!)"),speakerType==3)) {
        speakerType=3;
        altered=true;
      }
      ImGui::Unindent();

      if (ImGui::Checkbox(_("Reset phase on frequency change"),&resetPhase)) {
        altered=true;
      }

      if (altered) {
        e->lockSave([&]() {
          flags.set("clockSel",clockSel);
          flags.set("speakerType",speakerType);
          flags.set("resetPhase",resetPhase);
        });
      }
      break;
    }
    case DIV_SYSTEM_QSOUND: {
      int echoDelay=flags.getInt("echoDelay",0);
      int echoFeedback=flags.getInt("echoFeedback",0)&255;

      ImGui::Text(_("Echo delay:"));
      int echoBufSize1=2725-echoDelay;
      if (CWSliderInt("##EchoBufSize",&echoBufSize1,0,2725)) {
        if (echoBufSize1<0) echoBufSize1=0;
        if (echoBufSize1>2725) echoBufSize1=2725;
        echoDelay=2725-echoBufSize1;
        altered=true;
      } rightClickable
      ImGui::Text(_("Echo feedback:"));
      if (CWSliderInt("##EchoFeedback",&echoFeedback,0,255)) {
        if (echoFeedback<0) echoFeedback=0;
        if (echoFeedback>255) echoFeedback=255;
        altered=true;
      } rightClickable

      if (altered) {
        e->lockSave([&]() {
          flags.set("echoDelay",echoDelay);
          flags.set("echoFeedback",echoFeedback);
        });
      }

      supportsCustomRate=false;
      break;
    }
    case DIV_SYSTEM_X1_010: {
      int clockSel=flags.getInt("clockSel",0);
      bool stereo=flags.getBool("stereo",false);
      bool isBanked=flags.getBool("isBanked",false);

      ImGui::Text(_("Clock rate:"));
      ImGui::Indent();
      if (ImGui::RadioButton(_("16MHz (Seta 1)"),clockSel==0)) {
        clockSel=0;
        altered=true;
      }
      if (ImGui::RadioButton(_("16.67MHz (Seta 2)"),clockSel==1)) {
        clockSel=1;
        altered=true;
      }
      if (ImGui::RadioButton(_("14.32MHz (NTSC)"),clockSel==2)) {
        clockSel=2;
        altered=true;
      }
      ImGui::Unindent();

      if (ImGui::Checkbox(_("Stereo"),&stereo)) {
        altered=true;
      }

      if (ImGui::Checkbox(_("Bankswitched (Seta 2)"),&isBanked)) {
        altered=true;
        mustRender=true;
      }

      if (altered) {
        e->lockSave([&]() {
          flags.set("clockSel",clockSel);
          flags.set("stereo",stereo);
          flags.set("isBanked",isBanked);
        });
      }
      break;
    }
    case DIV_SYSTEM_N163: {
      int clockSel=flags.getInt("clockSel",0);
      int channels=flags.getInt("channels",7)+1;
      bool multiplex=flags.getBool("multiplex",false);
      bool lenCompensate=flags.getBool("lenCompensate",false);
      bool posLatch=flags.getBool("posLatch",true);

      ImGui::Text(_("Clock rate:"));
      ImGui::Indent();
      if (ImGui::RadioButton(_("NTSC (1.79MHz)"),clockSel==0)) {
        clockSel=0;
        altered=true;
      }
      if (ImGui::RadioButton(_("PAL (1.67MHz)"),clockSel==1)) {
        clockSel=1;
        altered=true;
      }
      if (ImGui::RadioButton(_("Dendy (1.77MHz)"),clockSel==2)) {
        clockSel=2;
        altered=true;
      }
      ImGui::Unindent();
      if (chan>=0) {
        if (channels!=e->song.systemChans[chan]) {
          pushWarningColor(true);
          ImGui::Text(_("the legacy channel limit is not equal to the channel count!\neither set the channel count to %d, or click one of the following buttons:"),channels);
          if (ImGui::Button(_("Fix channel count"))) {
            if (e->setSystemChans(chan,channels,preserveChanPos)) {
              MARK_MODIFIED;
              recalcTimestamps=true;
              if (e->song.autoSystem) {
                autoDetectSystem();
              }
              updateWindowTitle();
              updateROMExportAvail();
              altered=true;
            }
          }
          if (ImGui::Button(_("Give me more channels"))) {
            channels=e->song.systemChans[chan];
            altered=true;
          }
          popWarningColor();
        }
      }
      if (ImGui::Checkbox(_("Disable hissing"),&multiplex)) {
        altered=true;
      }
      if (ImGui::Checkbox(_("Scale frequency to wave length"),&lenCompensate)) {
        altered=true;
      }
      if (ImGui::Checkbox(_("Waveform position latch"),&posLatch)) {
        altered=true;
      }
      ImGui::SetItemTooltip(_("when enabled, a waveform position effect will lock the position, preventing instrument changes from changing it.\nuse a wave position effect with value FE or FF to unlock it."));

      if (altered) {
        e->lockSave([&]() {
          flags.set("clockSel",clockSel);
          flags.set("channels",channels-1);
          flags.set("multiplex",multiplex);
          flags.set("lenCompensate",lenCompensate);
          flags.set("posLatch",posLatch);
        });
      }
      break;
    }
    case DIV_SYSTEM_ES5506: {
      int channels=flags.getInt("channels",0x1f)+1;
      int volScale=flags.getInt("volScale",4095);
      bool amigaVol=flags.getBool("amigaVol",false);
      bool amigaPitch=flags.getBool("amigaPitch",false);

      int minChans=5;
      if (chan>=0) {
        minChans=e->song.systemChans[chan];
        if (minChans>32) minChans=32;
      }

      DivDispatch* dispatch=e->getDispatch(chan);
      double masterClock=16000000.0;
      if (dispatch!=NULL) {
        masterClock=dispatch->chipClock;
      }
      String outRateLabel=fmt::sprintf("/%d (%.0fHz)",channels,round(masterClock/(16.0*(double)channels)));

      ImGui::Text(_("Output rate divider:"));
      pushWarningColor(channels<minChans);
      if (CWSliderInt("##OTTO_InitialChannelLimit",&channels,minChans,32,outRateLabel.c_str())) {
        if (channels<5) channels=5;
        if (channels>32) channels=32;
        altered=true;
      } rightClickable
      if (ImGui::IsItemHovered() && channels<minChans) {
        ImGui::SetTooltip(_("divider too low! certain channels will be disabled."));
      }
      popWarningColor();

      ImGui::Text(_("Volume scale:"));

      if (CWSliderInt("##VolScaleO",&volScale,0,4095)) {
        if (volScale<0) volScale=0;
        if (volScale>4095) volScale=4095;
        altered=true;
      } rightClickable

      if (ImGui::Checkbox(_("Amiga channel volumes (64)"),&amigaVol)) {
        altered=true;
      }
      pushWarningColor(amigaPitch && e->song.compatFlags.linearPitch);
      if (ImGui::Checkbox(_("Amiga-like pitch (non-linear pitch only)"),&amigaPitch)) {
        altered=true;
      }
      if (amigaPitch && e->song.compatFlags.linearPitch) {
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("pitch linearity is set to linear. this won't do anything!");
        }
      }
      popWarningColor();

      if (altered) {
        e->lockSave([&]() {
          flags.set("channels",channels-1);
          flags.set("volScale",volScale);
          flags.set("amigaVol",amigaVol);
          flags.set("amigaPitch",amigaPitch);
        });
      }
      break;
    }
    case DIV_SYSTEM_YM2203:
    case DIV_SYSTEM_YM2203_EXT:
    case DIV_SYSTEM_YM2203_CSM: {
      int clockSel=flags.getInt("clockSel",0);
      int prescale=flags.getInt("prescale",0);
      bool noExtMacros=flags.getBool("noExtMacros",false);
      bool fbAllOps=flags.getBool("fbAllOps",false);
      int ssgVol=flags.getInt("ssgVol",128);
      int fmVol=flags.getInt("fmVol",256);

      ImGui::Text(_("Clock rate:"));
      ImGui::Indent();
      if (ImGui::RadioButton(_("3.58MHz (NTSC)"),clockSel==0)) {
        clockSel=0;
        altered=true;
      }
      if (ImGui::RadioButton(_("3.54MHz (PAL)"),clockSel==1)) {
        clockSel=1;
        altered=true;
      }
      if (ImGui::RadioButton(_("4MHz"),clockSel==2)) {
        clockSel=2;
        altered=true;
      }
      if (ImGui::RadioButton(_("3MHz"),clockSel==3)) {
        clockSel=3;
        altered=true;
      }
      if (ImGui::RadioButton(_("3.9936MHz (PC-88/PC-98)"),clockSel==4)) {
        clockSel=4;
        altered=true;
      }
      if (ImGui::RadioButton(_("1.5MHz"),clockSel==5)) {
        clockSel=5;
        altered=true;
      }
      ImGui::Unindent();
      ImGui::Text(_("Output rate:"));
      ImGui::Indent();
      if (ImGui::RadioButton(_("FM: clock / 72, SSG: clock / 16"),prescale==0)) {
        prescale=0;
        altered=true;
      }
      if (ImGui::RadioButton(_("FM: clock / 36, SSG: clock / 8"),prescale==1)) {
        prescale=1;
        altered=true;
      }
      if (ImGui::RadioButton(_("FM: clock / 24, SSG: clock / 4"),prescale==2)) {
        prescale=2;
        altered=true;
      }
      ImGui::Unindent();

      if (CWSliderInt(_("SSG Volume"),&ssgVol,0,256)) {
        if (ssgVol<0) ssgVol=0;
        if (ssgVol>256) ssgVol=256;
        altered=true;
      } rightClickable

      if (CWSliderInt(_("FM Volume"),&fmVol,0,256)) {
        if (fmVol<0) fmVol=0;
        if (fmVol>256) fmVol=256;
        altered=true;
      } rightClickable

      if (type==DIV_SYSTEM_YM2203_EXT || type==DIV_SYSTEM_YM2203_CSM) {
        if (ImGui::Checkbox(_("Disable ExtCh FM macros (compatibility)"),&noExtMacros)) {
          altered=true;
        }
        if (ImGui::Checkbox(_("Ins change in ExtCh operator 2-4 affects FB (compatibility)"),&fbAllOps)) {
          altered=true;
        }
      }

      if (altered) {
        e->lockSave([&]() {
          flags.set("clockSel",clockSel);
          flags.set("prescale",prescale);
          flags.set("noExtMacros",noExtMacros);
          flags.set("fbAllOps",fbAllOps);
          flags.set("ssgVol",ssgVol);
          flags.set("fmVol",fmVol);
        });
      }
      break;
    }
    case DIV_SYSTEM_YM2608:
    case DIV_SYSTEM_YM2608_EXT:
    case DIV_SYSTEM_YM2608_CSM: {
      int clockSel=flags.getInt("clockSel",0);
      int prescale=flags.getInt("prescale",0);
      bool noExtMacros=flags.getBool("noExtMacros",false);
      bool fbAllOps=flags.getBool("fbAllOps",false);
      bool memROM=flags.getBool("memROM",false);
      bool memParallel=flags.getBool("memParallel",true);
      int ssgVol=flags.getInt("ssgVol",128);
      int fmVol=flags.getInt("fmVol",256);

      ImGui::Text(_("Clock rate:"));
      ImGui::Indent();
      if (ImGui::RadioButton(_("8MHz (Arcade)"),clockSel==0)) {
        clockSel=0;
        altered=true;
      }
      if (ImGui::RadioButton(_("7.987MHz (PC-88/PC-98)"),clockSel==1)) {
        clockSel=1;
        altered=true;
      }
      ImGui::Unindent();
      ImGui::Text(_("Output rate:"));
      ImGui::Indent();
      if (ImGui::RadioButton(_("FM: clock / 144, SSG: clock / 32"),prescale==0)) {
        prescale=0;
        altered=true;
      }
      if (ImGui::RadioButton(_("FM: clock / 72, SSG: clock / 16"),prescale==1)) {
        prescale=1;
        altered=true;
      }
      if (ImGui::RadioButton(_("FM: clock / 48, SSG: clock / 8"),prescale==2)) {
        prescale=2;
        altered=true;
      }
      ImGui::Unindent();

      if (CWSliderInt(_("SSG Volume"),&ssgVol,0,256)) {
        if (ssgVol<0) ssgVol=0;
        if (ssgVol>256) ssgVol=256;
        altered=true;
      } rightClickable

      if (CWSliderInt(_("FM/ADPCM Volume"),&fmVol,0,256)) {
        if (fmVol<0) fmVol=0;
        if (fmVol>256) fmVol=256;
        altered=true;
      } rightClickable

      ImGui::TextUnformatted(_("Memory type:"));
      ImGui::Indent();
      if (ImGui::RadioButton(_("Parallel RAM (8-bit)"),(!memROM && memParallel))) {
        memROM=false;
        memParallel=true;
        altered=true;
        mustRender=true;
      }
      /*
      if (ImGui::RadioButton(_("Serial RAM (1-bit)"),(!memROM && !memParallel))) {
        memROM=false;
        memParallel=false;
        altered=true;
        mustRender=true;
      }*/
      if (ImGui::RadioButton(_("ROM"),(memROM && !memParallel))) {
        memROM=true;
        memParallel=false;
        altered=true;
        mustRender=true;
      }
      if (!memROM && !memParallel) {
        ImGui::Text(_("I have not implemented serial memory yet!"));
      }
      if (memROM && memParallel) {
        ImGui::Text(_("Congratulations! You found the invalid option!"));
      }
      ImGui::Unindent();

      if (type==DIV_SYSTEM_YM2608_EXT || type==DIV_SYSTEM_YM2608_CSM) {
        if (ImGui::Checkbox(_("Disable ExtCh FM macros (compatibility)"),&noExtMacros)) {
          altered=true;
        }
        if (ImGui::Checkbox(_("Ins change in ExtCh operator 2-4 affects FB (compatibility)"),&fbAllOps)) {
          altered=true;
        }
      }

      if (altered) {
        e->lockSave([&]() {
          flags.set("clockSel",clockSel);
          flags.set("prescale",prescale);
          flags.set("noExtMacros",noExtMacros);
          flags.set("fbAllOps",fbAllOps);
          flags.set("memROM",memROM);
          flags.set("memParallel",memParallel);
          flags.set("ssgVol",ssgVol);
          flags.set("fmVol",fmVol);
        });
      }
      break;
    }
    case DIV_SYSTEM_RF5C68: {
      int clockSel=flags.getInt("clockSel",0);
      int chipType=flags.getInt("chipType",0);

      ImGui::Text(_("Clock rate:"));
      ImGui::Indent();
      if (ImGui::RadioButton(_("8MHz (FM Towns)"),clockSel==0)) {
        clockSel=0;
        altered=true;
      }
      if (ImGui::RadioButton(_("10MHz (Sega System 18)"),clockSel==1)) {
        clockSel=1;
        altered=true;
      }
      if (ImGui::RadioButton(_("12.5MHz (Sega CD/System 32)"),clockSel==2)) {
        clockSel=2;
        altered=true;
      }
      ImGui::Unindent();
      ImGui::Text(_("Chip type:"));
      ImGui::Indent();
      if (ImGui::RadioButton(_("RF5C68 (10-bit output)"),chipType==0)) {
        chipType=0;
        altered=true;
      }
      if (ImGui::RadioButton(_("RF5C164 (16-bit output)"),chipType==1)) {
        chipType=1;
        altered=true;
      }
      ImGui::Unindent();

      if (altered) {
        e->lockSave([&]() {
          flags.set("clockSel",clockSel);
          flags.set("chipType",chipType);
        });
      }
      break;
    }
    case DIV_SYSTEM_MSM6258: {
      int clockSel=flags.getInt("clockSel",0);

      ImGui::Text(_("Clock rate:"));
      ImGui::Indent();
      if (ImGui::RadioButton("4MHz",clockSel==0)) {
        clockSel=0;
        altered=true;
      }
      if (ImGui::RadioButton("4.096MHz",clockSel==1)) {
        clockSel=1;
        altered=true;
      }
      if (ImGui::RadioButton("8MHz (X68000)",clockSel==2)) {
        clockSel=2;
        altered=true;
      }
      if (ImGui::RadioButton("8.192MHz",clockSel==3)) {
        clockSel=3;
        altered=true;
      }
      ImGui::Unindent();
      
      int chipClock=flags.getInt("customClock",0);
      if (!chipClock) {
        switch (clockSel) {
          case 0:
            chipClock=4000000;
            break;
          case 1:
            chipClock=4096000;
            break;
          case 2:
            chipClock=8000000;
            break;
          case 3:
            chipClock=8192000;
            break;
        }
      }

      ImGui::Text(_("Sample rate table:"));
      if (ImGui::BeginTable("6258Rate",3)) {
        ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
        ImGui::TableNextColumn();
        ImGui::Text(_("divider \\ clock"));
        ImGui::TableNextColumn();
        ImGui::Text(_("full"));
        ImGui::TableNextColumn();
        ImGui::Text(_("half"));

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,ImGui::GetColorU32(ImGuiCol_TableHeaderBg));
        ImGui::Text("/512");
        ImGui::TableNextColumn();
        ImGui::Text("%dHz",chipClock/512);
        ImGui::TableNextColumn();
        ImGui::Text("%dHz",chipClock/1024);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,ImGui::GetColorU32(ImGuiCol_TableHeaderBg));
        ImGui::Text("/768");
        ImGui::TableNextColumn();
        ImGui::Text("%dHz",chipClock/768);
        ImGui::TableNextColumn();
        ImGui::Text("%dHz",chipClock/1536);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,ImGui::GetColorU32(ImGuiCol_TableHeaderBg));
        ImGui::Text("/1024");
        ImGui::TableNextColumn();
        ImGui::Text("%dHz",chipClock/1024);
        ImGui::TableNextColumn();
        ImGui::Text("%dHz",chipClock/2048);

        ImGui::EndTable();
      }

      if (altered) {
        e->lockSave([&]() {
          flags.set("clockSel",clockSel);
        });
      }
      break;
    }
    case DIV_SYSTEM_MSM6295: {
      int clockSel=flags.getInt("clockSel",0);
      bool rateSel=flags.getBool("rateSel",false);
      bool isBanked=flags.getBool("isBanked",false);

      ImGui::Text(_("Clock rate:"));
      ImGui::Indent();
      if (ImGui::RadioButton("0.875MHz",clockSel==10)) {
        clockSel=10;
        altered=true;
      }
      if (ImGui::RadioButton("0.89MHz",clockSel==7)) {
        clockSel=7;
        altered=true;
      }
      if (ImGui::RadioButton("0.9375MHz",clockSel==11)) {
        clockSel=11;
        altered=true;
      }
      if (ImGui::RadioButton("1MHz",clockSel==0)) {
        clockSel=0;
        altered=true;
      }
      if (ImGui::RadioButton("1.02MHz",clockSel==6)) {
        clockSel=6;
        altered=true;
      }
      if (ImGui::RadioButton("1.056MHz",clockSel==1)) {
        clockSel=1;
        altered=true;
      }
      if (ImGui::RadioButton("1.193MHz (Atari)",clockSel==14)) {
        clockSel=14;
        altered=true;
      }
      if (ImGui::RadioButton("1.5MHz",clockSel==12)) {
        clockSel=12;
        altered=true;
      }
      if (ImGui::RadioButton("1.79MHz",clockSel==5)) {
        clockSel=5;
        altered=true;
      }
      if (ImGui::RadioButton("2MHz",clockSel==8)) {
        clockSel=8;
        altered=true;
      }
      if (ImGui::RadioButton("2.112MHz",clockSel==9)) {
        clockSel=9;
        altered=true;
      }
      if (ImGui::RadioButton("3MHz",clockSel==13)) {
        clockSel=13;
        altered=true;
      }
      if (ImGui::RadioButton("3.2MHz",clockSel==15)) {
        clockSel=15;
        altered=true;
      }
      if (ImGui::RadioButton("3.58MHz",clockSel==4)) {
        clockSel=4;
        altered=true;
      }
      if (ImGui::RadioButton("4MHz",clockSel==2)) {
        clockSel=2;
        altered=true;
      }
      if (ImGui::RadioButton("4.224MHz",clockSel==3)) {
        clockSel=3;
        altered=true;
      }
      ImGui::Unindent();
      ImGui::Text(_("Output rate:"));
      ImGui::Indent();
      if (ImGui::RadioButton(_("clock / 132"),rateSel==0)) {
        rateSel=false;
        altered=true;
      }
      if (ImGui::RadioButton(_("clock / 165"),rateSel==1)) {
        rateSel=true;
        altered=true;
      }
      ImGui::Unindent();

      if (ImGui::Checkbox(_("Bankswitched (NMK112)"),&isBanked)) {
        altered=true;
        mustRender=true;
      }

      if (altered) {
        e->lockSave([&]() {
          flags.set("clockSel",clockSel);
          flags.set("rateSel",rateSel);
          flags.set("isBanked",isBanked);
        });
      }
      break;
    }
    case DIV_SYSTEM_SCC:
    case DIV_SYSTEM_SCC_PLUS: {
      int clockSel=flags.getInt("clockSel",0);

      ImGui::Text(_("Clock rate:"));
      ImGui::Indent();
      if (ImGui::RadioButton(_("1.79MHz (NTSC/MSX)"),clockSel==0)) {
        clockSel=0;
        altered=true;
      }
      if (ImGui::RadioButton(_("1.77MHz (PAL)"),clockSel==1)) {
        clockSel=1;
        altered=true;
      }
      if (ImGui::RadioButton(_("1.5MHz (Arcade)"),clockSel==2)) {
        clockSel=2;
        altered=true;
      }
      if (ImGui::RadioButton(_("2MHz"),clockSel==3)) {
        clockSel=3;
        altered=true;
      }
      ImGui::Unindent();

      if (altered) {
        e->lockSave([&]() {
          flags.set("clockSel",clockSel);
        });
      }
      break;
    }
    case DIV_SYSTEM_LYNX: {
      bool tuned=flags.getBool("tuned",false);
      if (ImGui::Checkbox(_("Consistent frequency across all duties"),&tuned)) {
        altered=true;
        e->lockSave([&]() {
          flags.set("tuned",tuned);
        });
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(_("note: only works for an initial LFSR value of 0!"));
      }
      break;
    }
    case DIV_SYSTEM_OPL:
    case DIV_SYSTEM_OPL_DRUMS:
    case DIV_SYSTEM_OPL2:
    case DIV_SYSTEM_OPL2_DRUMS:
    case DIV_SYSTEM_Y8950:
    case DIV_SYSTEM_Y8950_DRUMS: {
      int clockSel=flags.getInt("clockSel",0);
      bool compatYPitch=flags.getBool("compatYPitch",false);

      ImGui::Text(_("Clock rate:"));
      ImGui::Indent();
      if (ImGui::RadioButton(_("3.58MHz (NTSC)"),clockSel==0)) {
        clockSel=0;
        altered=true;
      }
      if (ImGui::RadioButton(_("3.54MHz (PAL)"),clockSel==1)) {
        clockSel=1;
        altered=true;
      }
      if (ImGui::RadioButton(_("4MHz"),clockSel==2)) {
        clockSel=2;
        altered=true;
      }
      if (ImGui::RadioButton(_("3MHz"),clockSel==3)) {
        clockSel=3;
        altered=true;
      }
      if (ImGui::RadioButton(_("3.9936MHz (PC-88/PC-98)"),clockSel==4)) {
        clockSel=4;
        altered=true;
      }
      if (ImGui::RadioButton(_("3.5MHz"),clockSel==5)) {
        clockSel=5;
        altered=true;
      }
      ImGui::Unindent();

      if ((type==DIV_SYSTEM_Y8950 || type==DIV_SYSTEM_Y8950_DRUMS) && compatYPitch) {
        if (ImGui::Checkbox(_("ADPCM channel one octave up (compatibility)"),&compatYPitch)) {
          altered=true;
        }
      }

      if (altered) {
        e->lockSave([&]() {
          flags.set("clockSel",clockSel);
          flags.set("compatYPitch",compatYPitch);
        });
      }
      break;
    }
    case DIV_SYSTEM_OPL3:
    case DIV_SYSTEM_OPL3_DRUMS: {
      int clockSel=flags.getInt("clockSel",0);
      int chipType=flags.getInt("chipType",0);
      bool compatPan=flags.getBool("compatPan",false);

      ImGui::Text(_("Clock rate:"));
      ImGui::Indent();
      if (ImGui::RadioButton(_("14.32MHz (NTSC)"),clockSel==0)) {
        clockSel=0;
        altered=true;
      }
      if (ImGui::RadioButton(_("14.19MHz (PAL)"),clockSel==1)) {
        clockSel=1;
        altered=true;
      }
      if (ImGui::RadioButton(_("14MHz"),clockSel==2)) {
        clockSel=2;
        altered=true;
      }
      if (ImGui::RadioButton(_("16MHz"),clockSel==3)) {
        clockSel=3;
        altered=true;
      }
      if (ImGui::RadioButton(_("15MHz"),clockSel==4)) {
        clockSel=4;
        altered=true;
      }
      if (ImGui::RadioButton(_("33.8688MHz (OPL3-L)"),clockSel==5)) {
        clockSel=5;
        altered=true;
      }
      ImGui::Text(_("Chip type:"));
      if (ImGui::RadioButton(_("OPL3 (YMF262)"),chipType==0)) {
        chipType=0;
        altered=true;
      }
      if (ImGui::RadioButton(_("OPL3-L (YMF289B)"),chipType==1)) {
        chipType=1;
        altered=true;
      }
      ImGui::Unindent();

      if (ImGui::Checkbox(_("Compatible panning (0800)"),&compatPan)) {
        altered=true;
      }

      if (altered) {
        e->lockSave([&]() {
          flags.set("clockSel",clockSel);
          flags.set("chipType",chipType);
          flags.set("compatPan",compatPan);
        });
      }
      break;
    }
    case DIV_SYSTEM_YMZ280B: {
      int clockSel=flags.getInt("clockSel",0);

      ImGui::Text(_("Clock rate:"));
      ImGui::Indent();
      if (ImGui::RadioButton("16.9344MHz",clockSel==0)) {
        clockSel=0;
        altered=true;
      }
      if (ImGui::RadioButton("14.32MHz (NTSC)",clockSel==1)) {
        clockSel=1;
        altered=true;
      }
      if (ImGui::RadioButton("14.19MHz (PAL)",clockSel==2)) {
        clockSel=2;
        altered=true;
      }
      if (ImGui::RadioButton("16MHz",clockSel==3)) {
        clockSel=3;
        altered=true;
      }
      if (ImGui::RadioButton("16.67MHz",clockSel==4)) {
        clockSel=4;
        altered=true;
      }
      if (ImGui::RadioButton("14MHz",clockSel==5)) {
        clockSel=5;
        altered=true;
      }
      ImGui::Unindent();

      if (altered) {
        e->lockSave([&]() {
          flags.set("clockSel",clockSel);
        });
      }
      break;
    }
    case DIV_SYSTEM_PCM_DAC: {
      supportsCustomRate=false;
      // default to 44100Hz 16-bit stereo
      int sampRate=flags.getInt("rate",44100);
      int bitDepth=flags.getInt("outDepth",15)+1;
      int interpolation=flags.getInt("interpolation",0);
      int volMax=flags.getInt("volMax",255);
      float volMult=flags.getFloat("volMult",1.0f);
      bool stereo=flags.getBool("stereo",false);

      ImGui::Text(_("Output rate:"));
      if (CWSliderInt("##SampRate",&sampRate,1000,96000)) {
        if (sampRate<1000) sampRate=1000;
        if (sampRate>96000) sampRate=96000;
        altered=true;
      } rightClickable
      ImGui::Text(_("Output bit depth:"));
      if (CWSliderInt("##BitDepth",&bitDepth,1,16)) {
        if (bitDepth<1) bitDepth=1;
        if (bitDepth>16) bitDepth=16;
        altered=true;
      } rightClickable
      ImGui::Text(_("Maximum volume:"));
      if (CWSliderInt("##VolMax",&volMax,1,255)) {
        if (volMax<1) volMax=1;
        if (volMax>255) volMax=255;
        altered=true;
      } rightClickable
      if (ImGui::Checkbox(_("Stereo"),&stereo)) {
        altered=true;
      }

      ImGui::Text(_("Volume multiplier:"));
      if (CWSliderFloat("##VolMult",&volMult,0.0f,1.0f)) {
        if (volMult<0.0f) volMult=0.0f;
        if (volMult>1.0f) volMult=1.0f;
        altered=true;
      } rightClickable

      ImGui::Text(_("Interpolation:"));
      ImGui::Indent();
      if (ImGui::RadioButton(_("None"),interpolation==0)) {
        interpolation=0;
        altered=true;
      }
      if (ImGui::RadioButton(_("Linear"),interpolation==1)) {
        interpolation=1;
        altered=true;
      }
      if (ImGui::RadioButton(_("Cubic"),interpolation==2)) {
        interpolation=2;
        altered=true;
      }
      if (ImGui::RadioButton(_("Sinc"),interpolation==3)) {
        interpolation=3;
        altered=true;
      }
      ImGui::Unindent();

      if (altered) {
        e->lockSave([&]() {
          flags.set("rate",sampRate);
          flags.set("outDepth",bitDepth-1);
          flags.set("stereo",stereo);
          flags.set("interpolation",interpolation);
          flags.set("volMax",volMax);
          flags.set("volMult",volMult);
        });
      }
      break;
    }
    case DIV_SYSTEM_SNES: {
      char temp[64];
      int vsL=127-(flags.getInt("volScaleL",0)&127);
      int vsR=127-(flags.getInt("volScaleR",0)&127);
      bool echo=flags.getBool("echo",false);
      int echoVolL=flags.getInt("echoVolL",127);
      int echoVolR=flags.getInt("echoVolR",127);
      int echoDelay=flags.getInt("echoDelay",0)&15;
      int echoFeedback=flags.getInt("echoFeedback",0);
      int echoMask=flags.getInt("echoMask",0);

      int echoFilter[8];
      echoFilter[0]=flags.getInt("echoFilter0",127);
      echoFilter[1]=flags.getInt("echoFilter1",0);
      echoFilter[2]=flags.getInt("echoFilter2",0);
      echoFilter[3]=flags.getInt("echoFilter3",0);
      echoFilter[4]=flags.getInt("echoFilter4",0);
      echoFilter[5]=flags.getInt("echoFilter5",0);
      echoFilter[6]=flags.getInt("echoFilter6",0);
      echoFilter[7]=flags.getInt("echoFilter7",0);

      bool interpolationOff=flags.getBool("interpolationOff",false);
      bool antiClick=flags.getBool("antiClick",true);

      ImGui::Text(_("Volume scale:"));
      if (CWSliderInt(_("Left##VolScaleL"),&vsL,0,127)) {
        if (vsL<0) vsL=0;
        if (vsL>127) vsL=127;
        altered=true;
      } rightClickable
      if (CWSliderInt(_("Right##VolScaleL"),&vsR,0,127)) {
        if (vsR<0) vsR=0;
        if (vsR>127) vsR=127;
        altered=true;
      } rightClickable

      if (ImGui::Checkbox(_("Enable echo"),&echo)) {
        altered=true;
      }
      
      ImGui::Text(_("Initial echo state:"));
      for (int i=0; i<8; i++) {
        bool echoChan=(bool)(echoMask&(1<<i));
        snprintf(temp,63,"%d##EON%d",i+1,i);
        if (ImGui::Checkbox(temp,&echoChan)) {
          echoMask&=~(1<<i);
          if (echoChan) {
            echoMask|=1<<i;
          }
          altered=true;
        }
        if (i<7) {
          if (fromMenu) {
            ImGui::SameLine();
          } else {
            sameLineMaybe();
          }
        }
      }

      if (CWSliderInt(_("Delay##EchoDelay"),&echoDelay,0,15)) {
        if (echoDelay<0) echoDelay=0;
        if (echoDelay>15) echoDelay=15;
        altered=true;
      } rightClickable

      if (CWSliderInt(_("Feedback##EchoFeedback"),&echoFeedback,-128,127)) {
        if (echoFeedback<-128) echoFeedback=-128;
        if (echoFeedback>127) echoFeedback=127;
        altered=true;
      } rightClickable

      ImGui::Text(_("Echo volume:"));
      if (CWSliderInt(_("Left##EchoVolL"),&echoVolL,-128,127)) {
        if (echoVolL<-128) echoVolL=-128;
        if (echoVolL>127) echoVolL=127;
        altered=true;
      } rightClickable
      if (CWSliderInt(_("Right##EchoVolL"),&echoVolR,-128,127)) {
        if (echoVolR<-128) echoVolR=-128;
        if (echoVolR>127) echoVolR=127;
        altered=true;
      } rightClickable

      ImGui::Text(_("Echo filter:"));
      for (int i=0; i<8; i++) {
        snprintf(temp,63,"%d##FIR%d",i+1,i);
        if (CWSliderInt(temp,&echoFilter[i],-128,127)) {
          if (echoFilter[i]<-128) echoFilter[i]=-128;
          if (echoFilter[i]>127) echoFilter[i]=127;
          altered=true;
        } rightClickable
      }

      if (ImGui::Button(snesFilterHex?_("Hex##SNESFHex"):_("Dec##SNESFHex"))) {
        snesFilterHex=!snesFilterHex;
      }
      ImGui::SameLine();
      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x); // wavetable text input size found here
      if (ImGui::InputText("##MMLWave",&mmlStringSNES[sysPos])) {
        int actualData[256];
        int discardIt=0;
        memset(actualData,0,256*sizeof(int));
        decodeMMLStrW(mmlStringSNES[sysPos],actualData,discardIt,snesFilterHex?0:-128,snesFilterHex?255:127,snesFilterHex);
        if (snesFilterHex) {
          for (int i=0; i<8; i++) {
            if (actualData[i]>=128) actualData[i]-=256;
          }
        }
        memcpy(echoFilter,actualData,8*sizeof(int));
        altered=true;
      }
      if (!ImGui::IsItemActive()) {
        int actualData[8];
        for (int i=0; i<8; i++) {
          if (echoFilter[i]<0 && snesFilterHex) {
            actualData[i]=echoFilter[i]+256;
          } else {
            actualData[i]=echoFilter[i];
          }
        }
        encodeMMLStr(mmlStringSNES[sysPos],actualData,8,-1,-1,snesFilterHex);
      }

      int filterSum=(
        echoFilter[0]+
        echoFilter[1]+
        echoFilter[2]+
        echoFilter[3]+
        echoFilter[4]+
        echoFilter[5]+
        echoFilter[6]+
        echoFilter[7]
      );

      ImGui::PushStyleColor(ImGuiCol_Text,(filterSum<-128 || filterSum>127)?uiColors[GUI_COLOR_LOGLEVEL_WARNING]:uiColors[GUI_COLOR_TEXT]);
      ImGui::Text(_("sum: %d"),filterSum);
      ImGui::PopStyleColor();

      if (ImGui::Checkbox(_("Disable Gaussian interpolation"),&interpolationOff)) {
        altered=true;
      }

      if (ImGui::Checkbox(_("Anti-click"),&antiClick)) {
        altered=true;
      }

      if (altered) {
        e->lockSave([&]() {
          flags.set("volScaleL",127-vsL);
          flags.set("volScaleR",127-vsR);
          flags.set("echo",echo);
          flags.set("echoVolL",echoVolL);
          flags.set("echoVolR",echoVolR);
          flags.set("echoDelay",echoDelay);
          flags.set("echoFeedback",echoFeedback);
          flags.set("echoFilter0",echoFilter[0]);
          flags.set("echoFilter1",echoFilter[1]);
          flags.set("echoFilter2",echoFilter[2]);
          flags.set("echoFilter3",echoFilter[3]);
          flags.set("echoFilter4",echoFilter[4]);
          flags.set("echoFilter5",echoFilter[5]);
          flags.set("echoFilter6",echoFilter[6]);
          flags.set("echoFilter7",echoFilter[7]);
          flags.set("echoMask",echoMask);
          flags.set("interpolationOff",interpolationOff);
          flags.set("antiClick",antiClick);
        });
      }

      supportsCustomRate=false;

      break;
    }
    case DIV_SYSTEM_MSM5232: {
      int detune=flags.getInt("detune",0);
      int vibSpeed=flags.getInt("vibSpeed",0);
      float vibDepth=flags.getFloat("vibDepth",0.0f);
      bool groupEnv[2];
      int groupVol[8];
      float capValue[8];
      char temp[64];
      groupEnv[0]=flags.getBool("groupEnv0",true);
      groupEnv[1]=flags.getBool("groupEnv1",true);
      groupVol[0]=flags.getInt("partVolume0",255);
      groupVol[1]=flags.getInt("partVolume1",255);
      groupVol[2]=flags.getInt("partVolume2",255);
      groupVol[3]=flags.getInt("partVolume3",255);
      groupVol[4]=flags.getInt("partVolume4",255);
      groupVol[5]=flags.getInt("partVolume5",255);
      groupVol[6]=flags.getInt("partVolume6",255);
      groupVol[7]=flags.getInt("partVolume7",255);
      capValue[0]=flags.getFloat("capValue0",390.0f);
      capValue[1]=flags.getFloat("capValue1",390.0f);
      capValue[2]=flags.getFloat("capValue2",390.0f);
      capValue[3]=flags.getFloat("capValue3",390.0f);
      capValue[4]=flags.getFloat("capValue4",390.0f);
      capValue[5]=flags.getFloat("capValue5",390.0f);
      capValue[6]=flags.getFloat("capValue6",390.0f);
      capValue[7]=flags.getFloat("capValue7",390.0f);

      if (CWSliderInt(_("Detune"),&detune,-127,127)) {
        if (detune<-127) detune=-127;
        if (detune>127) detune=127;
        altered=true;
      } rightClickable

      ImGui::Text(_("Capacitor values (nF):"));
       for (int i=0; i<8; i++) {
        snprintf(temp,63,"%d##CAPV%d",i+1,i);
        if (CWSliderFloat(temp,&capValue[i],1.0f,1000.0f)) {
          if (capValue[i]<0) capValue[i]=0;
          if (capValue[i]>1000) capValue[i]=1000;
          altered=true;
        } rightClickable
      }
      
      ImGui::Text(_("Initial part volume (channel 1-4):"));
      for (int i=0; i<4; i++) {
        snprintf(temp,63,"%d'##GRPV%d",2<<i,i);
        if (CWSliderInt(temp,&groupVol[i],0,255)) {
          if (groupVol[i]<0) groupVol[i]=0;
          if (groupVol[i]>255) groupVol[i]=255;
          altered=true;
        } rightClickable
      }

      ImGui::Text(_("Initial part volume (channel 5-8):"));
      for (int i=4; i<8; i++) {
        snprintf(temp,63,"%d'##GRPV%d",2<<(i-4),i);
        if (CWSliderInt(temp,&groupVol[i],0,255)) {
          if (groupVol[i]<0) groupVol[i]=0;
          if (groupVol[i]>255) groupVol[i]=255;
          altered=true;
        } rightClickable
      }

      ImGui::Text(_("Envelope mode (channel 1-4):"));
      ImGui::Indent();
      if (ImGui::RadioButton(_("Capacitor (attack/decay)##EM00"),groupEnv[0])) {
        groupEnv[0]=true;
        altered=true;
      }
      if (ImGui::RadioButton(_("External (volume macro)##EM01"),!groupEnv[0])) {
        groupEnv[0]=false;
        altered=true;
      }
      ImGui::Unindent();

      ImGui::Text(_("Envelope mode (channel 5-8):"));
      ImGui::Indent();
      if (ImGui::RadioButton(_("Capacitor (attack/decay)##EM10"),groupEnv[1])) {
        groupEnv[1]=true;
        altered=true;
      }
      if (ImGui::RadioButton(_("External (volume macro)##EM11"),!groupEnv[1])) {
        groupEnv[1]=false;
        altered=true;
      }
      ImGui::Unindent();

      ImGui::Text(_("Global vibrato:"));

      if (CWSliderInt(_("Speed"),&vibSpeed,0,256)) {
        if (vibSpeed<0) vibSpeed=0;
        if (vibSpeed>256) vibSpeed=256;
        altered=true;
      } rightClickable
      if (CWSliderFloat(_("Depth"),&vibDepth,0.0f,256.0f)) {
        if (vibDepth<0) vibDepth=0;
        if (vibDepth>256) vibDepth=256;
        altered=true;
      } rightClickable

      if (altered) {
        flags.set("detune",detune);
        flags.set("vibSpeed",vibSpeed);
        flags.set("vibDepth",vibDepth);

        flags.set("capValue0",capValue[0]);
        flags.set("capValue1",capValue[1]);
        flags.set("capValue2",capValue[2]);
        flags.set("capValue3",capValue[3]);
        flags.set("capValue4",capValue[4]);
        flags.set("capValue5",capValue[5]);
        flags.set("capValue6",capValue[6]);
        flags.set("capValue7",capValue[7]);

        flags.set("partVolume0",groupVol[0]);
        flags.set("partVolume1",groupVol[1]);
        flags.set("partVolume2",groupVol[2]);
        flags.set("partVolume3",groupVol[3]);
        flags.set("partVolume4",groupVol[4]);
        flags.set("partVolume5",groupVol[5]);
        flags.set("partVolume6",groupVol[6]);
        flags.set("partVolume7",groupVol[7]);

        flags.set("groupEnv0",groupEnv[0]);
        flags.set("groupEnv1",groupEnv[1]);
      }
      break;
    }
    case DIV_SYSTEM_T6W28: {
      bool noEasyNoise=flags.getBool("noEasyNoise",false);

      if (ImGui::Checkbox(_("Disable easy period to note mapping on upper octaves"),&noEasyNoise)) {
        altered=true;
      }

      if (altered) {
        e->lockSave([&]() {
          flags.set("noEasyNoise",noEasyNoise);
        });
      }
      break;
    }
    case DIV_SYSTEM_K007232: {
      bool stereo=flags.getBool("stereo",false);

      if (ImGui::Checkbox(_("Stereo"),&stereo)) {
        altered=true;
      }

      if (altered) {
        e->lockSave([&]() {
          flags.set("stereo",stereo);
        });
      }
      break;
    }
    case DIV_SYSTEM_NAMCO:
    case DIV_SYSTEM_NAMCO_15XX: {
      bool romMode=flags.getBool("romMode",false);

      ImGui::Text(_("Waveform storage mode:"));
      ImGui::Indent();
      if (ImGui::RadioButton(_("RAM"),!romMode)) {
        romMode=false;
        altered=true;
      }
      if (ImGui::RadioButton(_("ROM (up to 8 waves)"),romMode)) {
        romMode=true;
        altered=true;
      }
      ImGui::Unindent();

      if (altered) {
        e->lockSave([&]() {
          flags.set("romMode",romMode);
        });
      }
      break;
    }
    case DIV_SYSTEM_NAMCO_CUS30: {
      bool newNoise=flags.getBool("newNoise",true);

      if (InvCheckbox(_("Compatible noise frequencies"),&newNoise)) {
        altered=true;
      }

      if (altered) {
        e->lockSave([&]() {
          flags.set("newNoise",newNoise);
        });
      }
      break;
    }
    case DIV_SYSTEM_SEGAPCM: {
      bool oldSlides=flags.getBool("oldSlides",false);

      if (ImGui::Checkbox(_("Legacy slides and pitch (compatibility)"),&oldSlides)) {
        altered=true;
      }

      if (altered) {
        e->lockSave([&]() {
          flags.set("oldSlides",oldSlides);
        });
      }
      break;
    }
    case DIV_SYSTEM_SUPERVISION: {
      bool swapDuty=flags.getInt("swapDuty",true);

      if (ImGui::Checkbox(_("Swap noise duty cycles"),&swapDuty)) {
        altered=true;
      }

      bool sqStereo=flags.getInt("sqStereo",false);

      if (ImGui::Checkbox(_("Stereo pulse waves"),&sqStereo)) {
        altered=true;
      }

      if (altered) {
        e->lockSave([&]() {
          flags.set("swapDuty",(int)swapDuty);
        });
        e->lockSave([&]() {
          flags.set("sqStereo",(int)sqStereo);
        });
      }
      break;
    }
    case DIV_SYSTEM_SM8521: {
    /* bool noAntiClick=flags.getBool("noAntiClick",false);

      if (ImGui::Checkbox(_("Disable anti-click"),&noAntiClick)) {
        altered=true;
      }

      if (altered) {
        e->lockSave([&]() {
          flags.set("noAntiClick",noAntiClick);
        });
      }*/
      break;
    }
    case DIV_SYSTEM_K053260: {
      int clockSel=flags.getInt("clockSel",0);

      ImGui::Text(_("Clock rate:"));
      ImGui::Indent();
      if (ImGui::RadioButton(_("3.58MHz (NTSC)"),clockSel==0)) {
        clockSel=0;
        altered=true;
      }
      if (ImGui::RadioButton(_("4MHz"),clockSel==1)) {
        clockSel=1;
        altered=true;
      }
      ImGui::Unindent();

      if (altered) {
        e->lockSave([&]() {
          flags.set("clockSel",clockSel);
        });
      }
      break;
    }
    case DIV_SYSTEM_TED: {
      int clockSel=flags.getInt("clockSel",0);
      bool keyPriority=flags.getBool("keyPriority",true);

      ImGui::Text(_("Clock rate:"));

      ImGui::Indent();
      if (ImGui::RadioButton(_("NTSC (1.79MHz)"),clockSel==0)) {
        clockSel=0;
        altered=true;
      }
      if (ImGui::RadioButton(_("PAL (1.77MHz)"),clockSel==1)) {
        clockSel=1;
        altered=true;
      }
      ImGui::Unindent();

      ImGui::Text(_("Global parameter priority:"));

      ImGui::Indent();
      if (ImGui::RadioButton(_("Left to right"),!keyPriority)) {
        keyPriority=false;
        altered=true;
      }
      if (ImGui::RadioButton(_("Last used channel"),keyPriority)) {
        keyPriority=true;
        altered=true;
      }
      ImGui::Unindent();

      if (altered) {
        e->lockSave([&]() {
          flags.set("clockSel",clockSel);
          flags.set("keyPriority",keyPriority);
        });
      }
      break;
    }
    case DIV_SYSTEM_C140: {
      int bankType=flags.getInt("bankType",0);

      ImGui::Text(_("Banking style:"));
      ImGui::Indent();
      if (ImGui::RadioButton(_("Namco System 2 (2MB)"),bankType==0)) {
        bankType=0;
        altered=true;
        mustRender=true;
      }
      if (ImGui::RadioButton(_("Namco System 21 (4MB)"),bankType==1)) {
        bankType=1;
        altered=true;
        mustRender=true;
      }
      if (ImGui::RadioButton(_("Raw (16MB; no VGM export!)"),bankType==2)) {
        bankType=2;
        altered=true;
        mustRender=true;
      }
      ImGui::Unindent();

      if (altered) {
        e->lockSave([&]() {
          flags.set("bankType",bankType);
        });
      }
      break;
    }
    case DIV_SYSTEM_VBOY: {
      bool romMode=flags.getBool("romMode",false);
      bool noAntiClick=flags.getBool("noAntiClick",false);
      bool screwThis=flags.getBool("screwThis",false);

      ImGui::Text(_("Waveform storage mode:"));
      ImGui::Indent();
      if (ImGui::RadioButton(_("Static (up to 5 waves)"),romMode)) {
        romMode=true;
        altered=true;
      }
      if (ImGui::RadioButton(_("Dynamic (phase reset on wave change!)"),!romMode)) {
        romMode=false;
        altered=true;
      }
      ImGui::Unindent();

      if (!romMode) {
        if (ImGui::Checkbox(_("Disable anti-phase-reset"),&noAntiClick)) {
          altered=true;
        }
        if (ImGui::Checkbox(_("I don't care about hardware"),&screwThis)) {
          altered=true;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("Virtual Boy hardware requires all channels to be disabled before writing to wave memory.\nif the clicks that arise from this annoy you, use this option.\nnote that your song won't play on hardware if you do so!"));
        }
      }

      if (altered) {
        e->lockSave([&]() {
          flags.set("romMode",romMode);
          flags.set("noAntiClick",noAntiClick);
          flags.set("screwThis",screwThis);
        });
      }
      break;
    }
    case DIV_SYSTEM_SFX_BEEPER_QUADTONE: {
      bool sysPal=flags.getInt("clockSel",0);
      bool noHiss=flags.getBool("noHiss",false);
      if (ImGui::Checkbox(_("PAL"),&sysPal)) {
        altered=true;
      }
      if (ImGui::Checkbox(_("Disable hissing"),&noHiss)) {
        altered=true;
      }
      if (altered) {
        e->lockSave([&]() {
          flags.set("clockSel",(int)sysPal);
          flags.set("noHiss",noHiss);
        });
      }
      break;
    }
    case DIV_SYSTEM_NDS: {
      int chipType=flags.getInt("chipType",0);

      ImGui::Text(_("Model:"));
      ImGui::Indent();
      if (ImGui::RadioButton(_("DS (4MB RAM)"),chipType==0)) {
        chipType=0;
        altered=true;
      }
      if (ImGui::RadioButton(_("DSi (16MB RAM)"),chipType==1)) {
        chipType=1;
        altered=true;
      }
      ImGui::Unindent();

      if (altered) {
        e->lockSave([&]() {
          flags.set("chipType",chipType);
        });
      }
      break;
    }
    case DIV_SYSTEM_VERA: {
      int chipType=flags.getInt("chipType",3);

      ImGui::Text(_("Chip revision:"));
      ImGui::Indent();
      if (ImGui::RadioButton(_("Initial release"),chipType==0)) {
        chipType=0;
        altered=true;
      }
      if (ImGui::RadioButton(_("V 47.0.2 (different volume curve)"),chipType==1)) {
        chipType=1;
        altered=true;
      }
      if (ImGui::RadioButton(_("V 48.0.1 (Tri/Saw PW XOR)"),chipType==2)) {
        chipType=2;
        altered=true;
      }
      if (ImGui::RadioButton(_("X16 Emu R49 (Noise freq fix)"),chipType==3)) {
        chipType=3;
        altered=true;
      }
      ImGui::Unindent();

      if (altered) {
        e->lockSave([&]() {
          flags.set("chipType",chipType);
        });
      }
      break;
    }
    case DIV_SYSTEM_OPL4:
    case DIV_SYSTEM_OPL4_DRUMS: {
      int clockSel=flags.getInt("clockSel",0);
      int ramSize=flags.getInt("ramSize",0);
      int fmMixL=flags.getInt("fmMixL",4);
      int fmMixR=flags.getInt("fmMixR",4);
      int pcmMixL=flags.getInt("pcmMixL",7);
      int pcmMixR=flags.getInt("pcmMixR",7);

      ImGui::Text(_("Clock rate:"));
      ImGui::Indent();
      if (ImGui::RadioButton(_("33.8688MHz"),clockSel==0)) {
        clockSel=0;
        altered=true;
      }
      if (ImGui::RadioButton(_("28.64MHz (NTSC)"),clockSel==1)) {
        clockSel=1;
        altered=true;
      }
      if (ImGui::RadioButton(_("28.38MHz (PAL)"),clockSel==2)) {
        clockSel=2;
        altered=true;
      }
      ImGui::Unindent();

      ImGui::Text(_("RAM size:"));
      ImGui::Indent();
      if (ImGui::RadioButton(_("4MB"),ramSize==0)) {
        ramSize=0;
        altered=true;
        mustRender=true;
      }
      if (ImGui::RadioButton(_("2MB"),ramSize==1)) {
        ramSize=1;
        altered=true;
        mustRender=true;
      }
      if (ImGui::RadioButton(_("1MB"),ramSize==2)) {
        ramSize=2;
        altered=true;
        mustRender=true;
      }
      if (ImGui::RadioButton(_("640KB"),ramSize==3)) {
        ramSize=3;
        altered=true;
        mustRender=true;
      }
      if (ImGui::RadioButton(_("512KB"),ramSize==4)) {
        ramSize=4;
        altered=true;
        mustRender=true;
      }
      if (ImGui::RadioButton(_("256KB"),ramSize==5)) {
        ramSize=5;
        altered=true;
        mustRender=true;
      }
      if (ImGui::RadioButton(_("128KB"),ramSize==6)) {
        ramSize=6;
        altered=true;
        mustRender=true;
      }
      ImGui::Unindent();

      ImGui::Text("FM volume:");
      if (CWSliderInt(_("Left##FMMixL"),&fmMixL,0,7)) {
        if (fmMixL<0) fmMixL=0;
        if (fmMixL>7) fmMixL=7;
        altered=true;
      } rightClickable
      if (CWSliderInt(_("Right##FMMixR"),&fmMixR,0,7)) {
        if (fmMixR<0) fmMixR=0;
        if (fmMixR>7) fmMixR=7;
        altered=true;
      } rightClickable

      ImGui::Text("PCM volume:");
      if (CWSliderInt(_("Left##PCMMixL"),&pcmMixL,0,7)) {
        if (pcmMixL<0) pcmMixL=0;
        if (pcmMixL>7) pcmMixL=7;
        altered=true;
      } rightClickable
      if (CWSliderInt(_("Right##PCMMixR"),&pcmMixR,0,7)) {
        if (pcmMixR<0) pcmMixR=0;
        if (pcmMixR>7) pcmMixR=7;
        altered=true;
      } rightClickable

      if (altered) {
        e->lockSave([&]() {
          flags.set("clockSel",clockSel);
          flags.set("ramSize",ramSize);
          flags.set("fmMixL",fmMixL);
          flags.set("fmMixR",fmMixR);
          flags.set("pcmMixL",pcmMixL);
          flags.set("pcmMixR",pcmMixR);
        });
      }
      break;
    }
    case DIV_SYSTEM_VIC20: {
      bool sysPal=flags.getInt("clockSel",0);
      bool filterOff=flags.getBool("filterOff",false);

      if (ImGui::Checkbox(_("PAL"),&sysPal)) {
        altered=true;
      }
      if (ImGui::Checkbox(_("Disable filtering"),&filterOff)) {
        altered=true;
      }

      if (altered) {
        e->lockSave([&]() {
          flags.set("clockSel",(int)sysPal);
          flags.set("filterOff",filterOff);
        });
      }
      break;
    }
    case DIV_SYSTEM_SWAN: {
      bool stereo=flags.getBool("stereo",true);
      if (ImGui::Checkbox(_("Headphone output##_SWAN_STEREO"),&stereo)) {
        altered=true;
      }

      if (altered) {
        e->lockSave([&]() {
          flags.set("stereo",stereo);
        });
      }
      break;
    }
    case DIV_SYSTEM_BUBSYS_WSG:
    case DIV_SYSTEM_PET:
    case DIV_SYSTEM_GA20:
    case DIV_SYSTEM_PV1000:
    case DIV_SYSTEM_C219:
    case DIV_SYSTEM_BIFURCATOR:
    case DIV_SYSTEM_POWERNOISE:
    case DIV_SYSTEM_UPD1771C:
    case DIV_SYSTEM_MULTIPCM:
      break;
    case DIV_SYSTEM_YMU759:
    case DIV_SYSTEM_ESFM:
      supportsCustomRate=false;
      ImGui::Text(_("nothing to configure"));
      break;
    case DIV_SYSTEM_SID3: {
      bool quarterClock=flags.getBool("quarterClock",false);
      if (ImGui::Checkbox(_("Quarter clock speed"),&quarterClock)) {
        altered=true;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(_("Decreases clock speed and CPU audio load by 4 times.\nCan be used if your CPU is too slow for the chip."
        "\nDoes not affect clock speed during export!\n\n"

        "Warning! Filters may become unstable at high cutoff and resonance\nif this option or lower clock speed are used!\n"
        "Also filters' timbre may be different near these values.\n\n"

        "Default clock speed is 1MHz (1000000Hz)."));
      }

      if (altered) {
        e->lockSave([&]() {
          flags.set("quarterClock",(int)quarterClock);
        });
      }
      break;
    }
    default: {
      bool sysPal=flags.getInt("clockSel",0);

      if (ImGui::Checkbox(_("PAL"),&sysPal)) {
        altered=true;
      }

      if (altered) {
        e->lockSave([&]() {
          flags.set("clockSel",(int)sysPal);
        });
      }
      break;
    }
  }

  bool separatedYet=false;
  const DivSysDef* sysDef=e->getSystemDef(type);
  if (sysDef!=NULL) {
    if (sysDef->minChans==sysDef->maxChans && sysDef->channels==sysDef->minChans) {
      supportsChannelCount=false;
    }
    if (!supportsChannelCount) {
      if (e->song.systemChans[chan]!=sysDef->channels) {
        ImGui::Separator();
        separatedYet=true;

        ImGui::TextUnformatted(_("irregular channel count detected!"));
        if (ImGui::Button(_("click here to fix it."))) {
          if (e->setSystemChans(chan,sysDef->channels,preserveChanPos)) {
            MARK_MODIFIED;
            recalcTimestamps=true;
            if (e->song.autoSystem) {
              autoDetectSystem();
            }
            updateWindowTitle();
            updateROMExportAvail();

            if (type==DIV_SYSTEM_N163) {
              e->lockSave([&]() {
                flags.set("channels",e->song.systemChans[chan]-1);
              });
              altered=true;
            }
          } else {
            showError(e->getLastError());
          }
        }
      }
    }
  }
  if (supportsChannelCount) {
    ImGui::Separator();
    separatedYet=true;
    int chCount=e->song.systemChans[chan];
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(_("Channels"));
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120.0f*dpiScale);
    if (ImGui::InputInt("##ChCount",&chCount,0,0)) {
      if (sysDef!=NULL) {
        if (chCount<sysDef->minChans) chCount=sysDef->minChans;
        if (chCount>sysDef->maxChans) chCount=sysDef->maxChans;
      }
    }
    if (ImGui::IsItemDeactivatedAfterEdit() && chCount!=e->song.systemChans[chan]) {
      if (e->setSystemChans(chan,chCount,preserveChanPos)) {
        MARK_MODIFIED;
        recalcTimestamps=true;
        if (e->song.autoSystem) {
          autoDetectSystem();
        }
        updateWindowTitle();
        updateROMExportAvail();

        if (type==DIV_SYSTEM_N163) {
          e->lockSave([&]() {
            flags.set("channels",e->song.systemChans[chan]-1);
          });
          altered=true;
        }
      } else {
        showError(e->getLastError());
      }
    }
    if (sysDef!=NULL) {
      ImGui::SameLine();
      ImGui::Text("(%d - %d)",sysDef->minChans,sysDef->maxChans);
    }
  }

  if (supportsCustomRate) {
    if (!separatedYet) ImGui::Separator();
    int customClock=flags.getInt("customClock",0);
    bool usingCustomClock=customClock>=MIN_CUSTOM_CLOCK;

    if (ImGui::Checkbox(_("Custom clock rate"),&usingCustomClock)) {
      if (usingCustomClock) {
        customClock=MIN_CUSTOM_CLOCK;
      } else {
        customClock=0;
      }
      altered=true;
    }
    ImGui::Indent();
    if (ImGui::InputInt("Hz",&customClock,100,10000)) {
      if (customClock<MIN_CUSTOM_CLOCK) customClock=0;
      if (customClock>MAX_CUSTOM_CLOCK) customClock=MAX_CUSTOM_CLOCK;
      altered=true;
    }
    ImGui::Unindent();

    if (altered) {
      e->lockSave([&]() {
        flags.set("customClock",customClock);
      });
    }
  }

  if (altered) {
    if (chan>=0) {
      e->updateSysFlags(chan,restart,mustRender);
      if (e->song.autoSystem) {
        autoDetectSystem();
      }
      updateWindowTitle();
    }
    MARK_MODIFIED;
  }

  return altered;
}
