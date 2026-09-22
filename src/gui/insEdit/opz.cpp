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

#include "insEditCommon.h"

void FurnaceGUI::insEditOPZ(DivInstrument* ins) {
  std::vector<FurnaceGUIMacroDesc> macroList;

  char label[32];
  int opCount=4;
  if (ins->type==DIV_INS_OPLL) opCount=2;
  if (ins->type==DIV_INS_OPL) opCount=(ins->fm.ops==4)?4:2;

  insTabFM(ins);

  if (ins->type!=DIV_INS_ESFM) {
    if (ImGui::BeginTabItem(_("FM Macros"))) {
      if (ins->type==DIV_INS_OPLL) {
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_SUS),&ins->std.algMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_FB),&ins->std.fbMacro,0,7,96,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_DC),&ins->std.fmsMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_DM),&ins->std.amsMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
      } else {
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_FB),&ins->std.fbMacro,0,7,96,uiColors[GUI_COLOR_MACRO_OTHER]));
        if (ins->type!=DIV_INS_OPL && ins->type!=DIV_INS_OPL_DRUMS) {
          macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_ALG),&ins->std.algMacro,0,7,96,uiColors[GUI_COLOR_MACRO_OTHER]));
          if (ins->type==DIV_INS_OPZ) {
            macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_FMS),&ins->std.fmsMacro,0,7,96,uiColors[GUI_COLOR_MACRO_OTHER]));
            macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_AMS),&ins->std.amsMacro,0,3,48,uiColors[GUI_COLOR_MACRO_OTHER]));
            macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_FMS2),&ins->std.ex9Macro,0,7,96,uiColors[GUI_COLOR_MACRO_OTHER]));
            macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_AMS2),&ins->std.ex10Macro,0,3,48,uiColors[GUI_COLOR_MACRO_OTHER]));
          } else {
            macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_FMS),&ins->std.fmsMacro,0,7,96,uiColors[GUI_COLOR_MACRO_OTHER]));
            macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_AMS),&ins->std.amsMacro,0,3,48,uiColors[GUI_COLOR_MACRO_OTHER]));
          }
        } else {
          macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_ALG),&ins->std.algMacro,0,4,96,uiColors[GUI_COLOR_MACRO_OTHER]));
        }
      }

      if (ins->type==DIV_INS_FM) {
        macroList.push_back(FurnaceGUIMacroDesc(_("LFO Speed"),&ins->std.ex3Macro,0,8,96,uiColors[GUI_COLOR_MACRO_OTHER]));
      }
      if (ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM) {
        macroList.push_back(FurnaceGUIMacroDesc(_("AM Depth"),&ins->std.ex1Macro,0,127,128,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc(_("PM Depth"),&ins->std.ex2Macro,0,127,128,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc(_("LFO Speed"),&ins->std.ex3Macro,0,255,128,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc(_("LFO Shape"),&ins->std.waveMacro,0,3,48,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,macroLFOWaves));
      }
      if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPM || ins->type==DIV_INS_OPZ) {
        macroList.push_back(FurnaceGUIMacroDesc(_("OpMask"),&ins->std.ex4Macro,0,4,128,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,fmOperatorBits));
      }
      if (ins->type==DIV_INS_OPZ) {
        macroList.push_back(FurnaceGUIMacroDesc(_("AM Depth 2"),&ins->std.ex5Macro,0,127,128,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc(_("PM Depth 2"),&ins->std.ex6Macro,0,127,128,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc(_("LFO2 Speed"),&ins->std.ex7Macro,0,255,128,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc(_("LFO2 Shape"),&ins->std.ex8Macro,0,3,48,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,macroLFOWaves));
      }
      drawMacros(macroList,macroEditStateFM,ins);
      ImGui::EndTabItem();
    }
  }
  for (int i=0; i<opCount; i++) {
    if (ins->type==DIV_INS_OPL_DRUMS) {
      if (i>0) break;
      snprintf(label,31,_("Operator Macros"));
    } else {
      snprintf(label,31,_("OP%d Macros"),i+1);
    }
    if (ImGui::BeginTabItem(label)) {
      ImGui::PushID(i);
      int ordi=(opCount==4 && ins->type!=DIV_INS_ESFM)?orderedOps[i]:i;
      int maxTl=127;
      if (ins->type==DIV_INS_OPLL) {
        if (i==1) {
          maxTl=15;
        } else {
          maxTl=63;
        }
      }
      if (ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_ESFM) {
        maxTl=63;
      }
      int maxArDr=(ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM)?31:15;

      if (ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS) {
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_TL),&ins->std.opMacros[ordi].tlMacro,0,maxTl,128,uiColors[GUI_COLOR_MACRO_VOLUME]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_AR),&ins->std.opMacros[ordi].arMacro,0,maxArDr,64,uiColors[GUI_COLOR_MACRO_ENVELOPE]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_DR),&ins->std.opMacros[ordi].drMacro,0,maxArDr,64,uiColors[GUI_COLOR_MACRO_ENVELOPE]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_SL),&ins->std.opMacros[ordi].slMacro,0,15,64,uiColors[GUI_COLOR_MACRO_ENVELOPE]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_RR),&ins->std.opMacros[ordi].rrMacro,0,15,64,uiColors[GUI_COLOR_MACRO_ENVELOPE]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_KSL),&ins->std.opMacros[ordi].kslMacro,0,3,32,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_MULT),&ins->std.opMacros[ordi].multMacro,0,15,64,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_WS),&ins->std.opMacros[ordi].wsMacro,0,7,64,uiColors[GUI_COLOR_MACRO_OTHER]));

        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_AM),&ins->std.opMacros[ordi].amMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_VIB),&ins->std.opMacros[ordi].vibMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_KSR),&ins->std.opMacros[ordi].ksrMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_SUS),&ins->std.opMacros[ordi].susMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
      } else if (ins->type==DIV_INS_OPLL) {
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_TL),&ins->std.opMacros[ordi].tlMacro,0,maxTl,128,uiColors[GUI_COLOR_MACRO_VOLUME]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_AR),&ins->std.opMacros[ordi].arMacro,0,maxArDr,64,uiColors[GUI_COLOR_MACRO_ENVELOPE]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_DR),&ins->std.opMacros[ordi].drMacro,0,maxArDr,64,uiColors[GUI_COLOR_MACRO_ENVELOPE]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_SL),&ins->std.opMacros[ordi].slMacro,0,15,64,uiColors[GUI_COLOR_MACRO_ENVELOPE]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_RR),&ins->std.opMacros[ordi].rrMacro,0,15,64,uiColors[GUI_COLOR_MACRO_ENVELOPE]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_KSL),&ins->std.opMacros[ordi].kslMacro,0,3,32,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_MULT),&ins->std.opMacros[ordi].multMacro,0,15,64,uiColors[GUI_COLOR_MACRO_OTHER]));

        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_AM),&ins->std.opMacros[ordi].amMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_VIB),&ins->std.opMacros[ordi].vibMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_KSR),&ins->std.opMacros[ordi].ksrMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_EGS),&ins->std.opMacros[ordi].egtMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
      } else if (ins->type==DIV_INS_ESFM) {
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_TL),&ins->std.opMacros[ordi].tlMacro,0,maxTl,128,uiColors[GUI_COLOR_MACRO_VOLUME]));
        if (ins->esfm.op[ordi].fixed) {
          macroList.push_back(FurnaceGUIMacroDesc(_("Block"),&ins->std.opMacros[ordi].ssgMacro,0,7,64,uiColors[GUI_COLOR_MACRO_PITCH],true));
          macroList.push_back(FurnaceGUIMacroDesc(_("FreqNum"),&ins->std.opMacros[ordi].dtMacro,0,1023,160,uiColors[GUI_COLOR_MACRO_PITCH]));
        } else {
          macroList.push_back(FurnaceGUIMacroDesc(_("Op. Arpeggio"),&ins->std.opMacros[ordi].ssgMacro,-120,120,160,uiColors[GUI_COLOR_MACRO_PITCH],true,NULL,macroHoverNote,false,NULL,true,ins->std.opMacros[ordi].ssgMacro.val,true));
          macroList.push_back(FurnaceGUIMacroDesc(_("Op. Pitch"),&ins->std.opMacros[ordi].dtMacro,-2048,2047,160,uiColors[GUI_COLOR_MACRO_PITCH],true,macroRelativeMode,NULL,false,NULL,false,NULL,false,true));
        }
        macroList.push_back(FurnaceGUIMacroDesc(ESFM_NAME(ESFM_DELAY),&ins->std.opMacros[ordi].dt2Macro,0,7,64,uiColors[GUI_COLOR_MACRO_ENVELOPE]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_AR),&ins->std.opMacros[ordi].arMacro,0,maxArDr,64,uiColors[GUI_COLOR_MACRO_ENVELOPE]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_DR),&ins->std.opMacros[ordi].drMacro,0,maxArDr,64,uiColors[GUI_COLOR_MACRO_ENVELOPE]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_SL),&ins->std.opMacros[ordi].slMacro,0,15,64,uiColors[GUI_COLOR_MACRO_ENVELOPE]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_RR),&ins->std.opMacros[ordi].rrMacro,0,15,64,uiColors[GUI_COLOR_MACRO_ENVELOPE]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_KSL),&ins->std.opMacros[ordi].kslMacro,0,3,32,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_MULT),&ins->std.opMacros[ordi].multMacro,0,15,64,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_WS),&ins->std.opMacros[ordi].wsMacro,0,7,64,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc(ESFM_NAME(ESFM_OUTLVL),&ins->std.opMacros[ordi].egtMacro,0,7,64,uiColors[GUI_COLOR_MACRO_VOLUME]));
        macroList.push_back(FurnaceGUIMacroDesc(ESFM_NAME(ESFM_MODIN),&ins->std.opMacros[ordi].d2rMacro,0,7,64,uiColors[GUI_COLOR_MACRO_VOLUME]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_AM),&ins->std.opMacros[ordi].amMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_VIB),&ins->std.opMacros[ordi].vibMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_DAM),&ins->std.opMacros[ordi].damMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_DVB),&ins->std.opMacros[ordi].dvbMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_KSR),&ins->std.opMacros[ordi].ksrMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_SUS),&ins->std.opMacros[ordi].susMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
        macroList.push_back(FurnaceGUIMacroDesc(_("Op. Panning"),&ins->std.opMacros[ordi].rsMacro,0,2,40,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,panBits));
      } else if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPM) {
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_TL),&ins->std.opMacros[ordi].tlMacro,0,maxTl,128,uiColors[GUI_COLOR_MACRO_VOLUME]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_AR),&ins->std.opMacros[ordi].arMacro,0,maxArDr,64,uiColors[GUI_COLOR_MACRO_ENVELOPE]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_DR),&ins->std.opMacros[ordi].drMacro,0,maxArDr,64,uiColors[GUI_COLOR_MACRO_ENVELOPE]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_D2R),&ins->std.opMacros[ordi].d2rMacro,0,31,64,uiColors[GUI_COLOR_MACRO_ENVELOPE]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_RR),&ins->std.opMacros[ordi].rrMacro,0,15,64,uiColors[GUI_COLOR_MACRO_ENVELOPE]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_SL),&ins->std.opMacros[ordi].slMacro,0,15,64,uiColors[GUI_COLOR_MACRO_ENVELOPE]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_RS),&ins->std.opMacros[ordi].rsMacro,0,3,32,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_MULT),&ins->std.opMacros[ordi].multMacro,0,15,64,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_DT),&ins->std.opMacros[ordi].dtMacro,0,7,64,uiColors[GUI_COLOR_MACRO_PITCH]));
        if (ins->type==DIV_INS_OPM) {
          macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_DT2),&ins->std.opMacros[ordi].dt2Macro,0,3,32,uiColors[GUI_COLOR_MACRO_PITCH]));
        }
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_AM),&ins->std.opMacros[ordi].amMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));

        if (ins->type==DIV_INS_FM) {
          macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_SSG),&ins->std.opMacros[ordi].ssgMacro,0,4,64,uiColors[GUI_COLOR_MACRO_ENVELOPE],false,NULL,NULL,true,ssgEnvBits));
        }
      } else if (ins->type==DIV_INS_OPZ) {
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_TL),&ins->std.opMacros[ordi].tlMacro,0,maxTl,128,uiColors[GUI_COLOR_MACRO_VOLUME]));
        if (ins->fm.op[ordi].egt) {
          if (!ins->fm.op[ordi].sus) {
            macroList.push_back(FurnaceGUIMacroDesc(_("Block"),&ins->std.opMacros[ordi].ssgMacro,0,7,64,uiColors[GUI_COLOR_MACRO_PITCH],true));
            macroList.push_back(FurnaceGUIMacroDesc(_("FreqNum"),&ins->std.opMacros[ordi].susMacro,0,255,160,uiColors[GUI_COLOR_MACRO_PITCH]));
          } else {
            macroList.push_back(FurnaceGUIMacroDesc(_("Op. Arpeggio"),&ins->std.opMacros[ordi].ssgMacro,-120,120,160,uiColors[GUI_COLOR_MACRO_PITCH],true,NULL,macroHoverNote,false,NULL,true,ins->std.opMacros[ordi].ssgMacro.val,true));
            macroList.push_back(FurnaceGUIMacroDesc(_("Op. Pitch"),&ins->std.opMacros[ordi].susMacro,-2048,2047,160,uiColors[GUI_COLOR_MACRO_PITCH],true,macroRelativeMode,NULL,false,NULL,false,NULL,false,true));
          }
        }
        
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_AR),&ins->std.opMacros[ordi].arMacro,0,maxArDr,64,uiColors[GUI_COLOR_MACRO_ENVELOPE]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_DR),&ins->std.opMacros[ordi].drMacro,0,maxArDr,64,uiColors[GUI_COLOR_MACRO_ENVELOPE]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_D2R),&ins->std.opMacros[ordi].d2rMacro,0,31,64,uiColors[GUI_COLOR_MACRO_ENVELOPE]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_RR),&ins->std.opMacros[ordi].rrMacro,0,15,64,uiColors[GUI_COLOR_MACRO_ENVELOPE]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_SL),&ins->std.opMacros[ordi].slMacro,0,15,64,uiColors[GUI_COLOR_MACRO_ENVELOPE]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_RS),&ins->std.opMacros[ordi].rsMacro,0,3,32,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_MULT),&ins->std.opMacros[ordi].multMacro,0,15,64,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_DT),&ins->std.opMacros[ordi].dtMacro,0,7,64,uiColors[GUI_COLOR_MACRO_PITCH]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_DT2),&ins->std.opMacros[ordi].dt2Macro,0,3,32,uiColors[GUI_COLOR_MACRO_PITCH]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_AM),&ins->std.opMacros[ordi].amMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
      }
      drawMacros(macroList,macroEditStateOP[ordi],ins);
      ImGui::PopID();
      ImGui::EndTabItem();
    }
  }

  if (ImGui::BeginTabItem(_("Macros"))) {
    macroList.push_back(FurnaceGUIMacroDesc(_("Volume"),&ins->std.volMacro,0,127,160,uiColors[GUI_COLOR_MACRO_VOLUME]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Arpeggio"),&ins->std.arpMacro,-120,120,160,uiColors[GUI_COLOR_MACRO_PITCH],true,NULL,macroHoverNote,false,NULL,true,ins->std.arpMacro.val));
    macroList.push_back(FurnaceGUIMacroDesc(_("Noise Freq"),&ins->std.dutyMacro,0,32,160,uiColors[GUI_COLOR_MACRO_NOISE]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Panning"),&ins->std.panLMacro,0,2,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,panBits));
    macroList.push_back(FurnaceGUIMacroDesc(_("Pitch"),&ins->std.pitchMacro,-2048,2047,160,uiColors[GUI_COLOR_MACRO_PITCH],true,macroRelativeMode));
    macroList.push_back(FurnaceGUIMacroDesc(_("Phase Reset"),&ins->std.phaseResetMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));

    drawMacros(macroList,macroEditStateMacros,ins);
    ImGui::EndTabItem();
  }
}
