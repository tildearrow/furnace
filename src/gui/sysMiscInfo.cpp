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
 
#include "gui.h"
#include "guiConst.h"
#include "misc/cpp/imgui_stdlib.h"
#include <fmt/printf.h>
#include <imgui.h>

const char* FurnaceGUI::getSystemPartNumber(DivSystem sys, DivConfig& flags) {
  switch (sys) {
    case DIV_SYSTEM_YMU759:
      return "YMU759";
      break;
    case DIV_SYSTEM_SMS:{
      int chipType=flags.getInt("chipType",0);
      if (chipType==4) {
        return "SN76489A";
      } else if (chipType==5) {
        return "SN76496";
      } else if (chipType==6) {
        return "8496";
      } else if (chipType==7) {
        return "PSSJ";//not part number
      } else if (chipType==8) {
        return "SN94624";
      } else if (chipType==9) {
        return "SN76494";
      } else {
        return "SN76489";
      }
      break;
    }
    case DIV_SYSTEM_PCE:{
      int chipType=flags.getInt("chipType",0);
      if (chipType==1) {
        return "HuC6280A";
      } else {
        return "HuC6280";
      }
    }
    case DIV_SYSTEM_NES:
      return "2A03";
      break;
    case DIV_SYSTEM_C64_6581:
      return "6581";
      break;
    case DIV_SYSTEM_C64_8580:
      return "8580";
      break;
    case DIV_SYSTEM_Y8950:
    case DIV_SYSTEM_Y8950_DRUMS:
      return "Y8950";
      break;
    case DIV_SYSTEM_AY8910:{
      int chipType=flags.getInt("chipType",0);
        if (chipType==1) {
          return "YM2149(F)";
        } else if (chipType==2) {
          return "5B";
        } else if (chipType==3) {
          return "AY-3-8914";
        } else {
          return "AY-3-8910";
        }
      break;
    }
    case DIV_SYSTEM_YM2151:
      return "YM2151";
      break;
    case DIV_SYSTEM_YM2612:
    case DIV_SYSTEM_YM2612_CSM:
    case DIV_SYSTEM_YM2612_DUALPCM:
    case DIV_SYSTEM_YM2612_DUALPCM_EXT:
    case DIV_SYSTEM_YM2612_EXT:{
      int chipType=0;
      if (flags.has("chipType")) {
        chipType=flags.getInt("chipType",0);
      } else {
        chipType=flags.getBool("ladderEffect",0)?1:0;
      }
      if (chipType==0) {
        return "YM3438";
      } else if (chipType==2) {
        return "YMF276";
      } else {
        return "YM2612";
      }
      break;
    }
    case DIV_SYSTEM_TIA:
      return "TIA";
      break;
    case DIV_SYSTEM_SAA1099:
      return "SAA1099";
      break;
    case DIV_SYSTEM_AY8930:
      return "AY8930";
      break;
    case DIV_SYSTEM_VIC20:
      return "VIC";
      break;
    case DIV_SYSTEM_PET:
      return "PET";
      break;
    case DIV_SYSTEM_VRC6:
      return "VRC6";
      break;
    case DIV_SYSTEM_FDS:
      return "FDS";
      break;
    case DIV_SYSTEM_MMC5:
      return "MMC5";
      break;
    case DIV_SYSTEM_N163:
      return "N163";
      break;
    case DIV_SYSTEM_YM2203:
    case DIV_SYSTEM_YM2203_EXT:
    case DIV_SYSTEM_YM2203_CSM:
      return "YM2203";
      break;
    case DIV_SYSTEM_YM2608:
    case DIV_SYSTEM_YM2608_CSM:
    case DIV_SYSTEM_YM2608_EXT:
      return "YM2608";
      break;
    case DIV_SYSTEM_OPLL:
    case DIV_SYSTEM_OPLL_DRUMS:{
      int patchSet=flags.getInt("patchSet",0);
        if (patchSet==1) {
          return "YMF281";
        } else if (patchSet==2) {
          return "YM2423";
        } else if (patchSet==3) {
          return "VRC7";
        } else {
          return "YM2413";
        }
      break;
    }
    case DIV_SYSTEM_OPL2:
    case DIV_SYSTEM_OPL2_DRUMS:
      return "YM3812";
      break;
    case DIV_SYSTEM_OPL3:
    case DIV_SYSTEM_OPL3_DRUMS:{
      int chipType=flags.getInt("chipType",0);
      if (chipType==1) {
        return "YMF289B";
      } else {
        return "YMF262";
      }
      break;
    }
    case DIV_SYSTEM_OPL4:
    case DIV_SYSTEM_OPL4_DRUMS:
      return "YMF278";
      break;
    case DIV_SYSTEM_MULTIPCM:
      return "YMW258-F";
      break;
    case DIV_SYSTEM_RF5C68:{
      int chipType=flags.getInt("chipType",0);
      if (chipType==1) {
        return "RF5C164";
      } else {
        return "RF5C68";
      }
      break;
    }
    case DIV_SYSTEM_OPZ:
      return "YM2414";
      break;
    case DIV_SYSTEM_SEGAPCM:
    case DIV_SYSTEM_SEGAPCM_COMPAT:
      return "SegaPCM";
      break;
    case DIV_SYSTEM_VRC7:
      return "VRC7";
      break;
    case DIV_SYSTEM_YM2610B:
    case DIV_SYSTEM_YM2610B_CSM:
    case DIV_SYSTEM_YM2610B_EXT:
      return "YM2610B";
      break;
    case DIV_SYSTEM_SFX_BEEPER:
    case DIV_SYSTEM_SFX_BEEPER_QUADTONE:
      return "ZXS Beeper";
      break;
    case DIV_SYSTEM_SCC:
      return "SCC";
      break;
    case DIV_SYSTEM_YM2610:
    case DIV_SYSTEM_YM2610_CSM:
    case DIV_SYSTEM_YM2610_EXT:
    case DIV_SYSTEM_YM2610_FULL:
    case DIV_SYSTEM_YM2610_FULL_EXT:
      return "YM2610";
      break;
    case DIV_SYSTEM_OPL:
    case DIV_SYSTEM_OPL_DRUMS:
      return "YM3526";
      break;
    case DIV_SYSTEM_QSOUND:
      return "QSound";
      break;
    case DIV_SYSTEM_X1_010:
      return "X1-010";
      break;
    case DIV_SYSTEM_BUBSYS_WSG:
      return "Konami WSG";
      break;
    case DIV_SYSTEM_ES5506:
      return "ES5506";
      break;
    case DIV_SYSTEM_SCC_PLUS:
      return "SCC+";
      break;
    case DIV_SYSTEM_SOUND_UNIT:
      return "TSU";
      break;
    case DIV_SYSTEM_MSM6295:
      return "MSM6295";
      break;
    case DIV_SYSTEM_MSM6258:
      return "MSM6258";
      break;
    case DIV_SYSTEM_YMZ280B:
      return "YMZ280B";
      break;
    case DIV_SYSTEM_NAMCO_15XX:
      return "C15";
      break;
    case DIV_SYSTEM_NAMCO_CUS30:
      return "C30";
      break;
    case DIV_SYSTEM_MSM5232:
      return "MSM5232";
      break;
    case DIV_SYSTEM_K007232:
      return "K007232";
      break;
    case DIV_SYSTEM_GA20:
      return "GA20";
      break;
    case DIV_SYSTEM_PCM_DAC:
      return "DAC";
      break;
    case DIV_SYSTEM_SM8521:
      return "SM8521";
      break;
    case DIV_SYSTEM_PV1000:
      return "PV-1000";
      break;
    case DIV_SYSTEM_K053260:
      return "K053260";
      break;
    case DIV_SYSTEM_TED:
      return "TED";
      break;
    case DIV_SYSTEM_C140:
      return "C140";
      break;
    case DIV_SYSTEM_C219:
      return "C219";
      break;
    case DIV_SYSTEM_ESFM:
      return "ES1xxx";
      break;
    case DIV_SYSTEM_SUPERVISION:
      return "Watara Supervision";
      break;
    case DIV_SYSTEM_UPD1771C:
      return "μPD1771C-017";
      break;
    default:
      return FurnaceGUI::getSystemName(sys);
      break;
  }
}

float FurnaceGUI::drawSystemChannelInfo(const DivSysDef* whichDef, int keyHitOffset, float tooltipWidth, int chanCount) {
  if (whichDef==NULL) return 0;
  if (chanCount<1) chanCount=whichDef->channels;

  ImDrawList* dl=ImGui::GetWindowDrawList();
  const ImVec2 p=ImGui::GetCursorScreenPos();
  if (tooltipWidth<=0.0f) tooltipWidth=ImGui::GetContentRegionAvail().x;
  ImVec2 sep=ImGui::GetStyle().ItemSpacing;
  sep.x*=0.5f;
  ImVec2 ledSize=ImVec2(
    (tooltipWidth-sep.x*(chanCount-1))/(float)chanCount,
    settings.iconSize*dpiScale
  );
  if (ledSize.x<8.0f*dpiScale) ledSize.x=8.0f*dpiScale;
  float x=p.x, y=p.y;
  for (int i=0; i<chanCount; i++) {
    if (x+ledSize.x-0.125>tooltipWidth+p.x) {
      x=p.x;
      y+=ledSize.y+sep.y;
    }
    ImVec4 color=uiColors[GUI_COLOR_CHANNEL_BG];
    if (i<whichDef->channels) color=uiColors[whichDef->chanTypes[i]+GUI_COLOR_CHANNEL_FM];
    if (keyHitOffset>=0) {
      if (e->isChannelMuted(keyHitOffset+i)) {
        color=uiColors[GUI_COLOR_CHANNEL_MUTED];
        color.x*=MIN(1.0f,0.125f+keyHit1[keyHitOffset+i]*0.875f);
        color.y*=MIN(1.0f,0.125f+keyHit1[keyHitOffset+i]*0.875f);
        color.z*=MIN(1.0f,0.125f+keyHit1[keyHitOffset+i]*0.875f);
      } else {
        color.x*=MIN(1.0f,0.125f+keyHit1[keyHitOffset+i]*0.875f);
        color.y*=MIN(1.0f,0.125f+keyHit1[keyHitOffset+i]*0.875f);
        color.z*=MIN(1.0f,0.125f+keyHit1[keyHitOffset+i]*0.875f);
      }
    }
    dl->AddRectFilled(ImVec2(x,y),ImVec2(x+ledSize.x,y+ledSize.y),ImGui::GetColorU32(color),ledSize.y);
    x+=ledSize.x+sep.x;
  }
  ImGui::Dummy(ImVec2(tooltipWidth,(y-p.y)+ledSize.y));
  return (y-p.y)+ledSize.y;
}

void FurnaceGUI::drawSystemChannelInfoText(const DivSysDef* whichDef) {
  String info="";
  // same order as chanNames
  // helper: FM|PU|NO|WA|SA | SQ|TR|SW|OP|DR|SL|WV|CH
  unsigned char chanCount[CHANNEL_TYPE_MAX];
  memset(chanCount,0,CHANNEL_TYPE_MAX);
  // count channel types
  for (int i=0; i<whichDef->channels; i++) {
    switch (whichDef->chanInsType[i][0]) {
      case DIV_INS_STD: // square
      case DIV_INS_BEEPER:
      case DIV_INS_TED:
      case DIV_INS_VIC:
      case DIV_INS_T6W28:
      case DIV_INS_PV1000:
        if (whichDef->id==0xfd) { // dummy
          chanCount[CHANNEL_TYPE_OTHER]++;
          break;
        }
        if (whichDef->id==0x9f) { // zx sfx
          chanCount[CHANNEL_TYPE_PULSE]++;
          break;
        }
        if (whichDef->chanTypes[i]==DIV_CH_NOISE) { // sn/t6w noise
          chanCount[CHANNEL_TYPE_NOISE]++;
        } else { // DIV_CH_PULSE, any sqr chan
          chanCount[CHANNEL_TYPE_SQUARE]++;
        }
        break;
      case DIV_INS_NES:
        if (whichDef->chanTypes[i]==DIV_CH_WAVE) {
          chanCount[whichDef->id==0xf1?CHANNEL_TYPE_WAVE:CHANNEL_TYPE_TRIANGLE]++; // triangle, wave for 5E01
        } else {
          chanCount[whichDef->chanTypes[i]]++;
        }
        break;
      case DIV_INS_AY:
      case DIV_INS_AY8930:
        chanCount[CHANNEL_TYPE_PSG]++;
        break;
      case DIV_INS_OPL_DRUMS:
      case DIV_INS_OPL:
      case DIV_INS_OPLL:
        if (whichDef->chanTypes[i]==DIV_CH_OP) {
          chanCount[CHANNEL_TYPE_FM]++; // opl3 4op
          break;
        }
        if (whichDef->chanTypes[i]==DIV_CH_NOISE) {
          chanCount[CHANNEL_TYPE_DRUMS]++; // drums
        } else {
          chanCount[whichDef->chanTypes[i]]++;
        }
        break;
      case DIV_INS_FM:
        if (whichDef->chanTypes[i]==DIV_CH_OP) {
          chanCount[CHANNEL_TYPE_OPERATOR]++; // ext. ops
        } else if (whichDef->chanTypes[i]==DIV_CH_NOISE) {
          break; // csm timer
        } else {
          chanCount[whichDef->chanTypes[i]]++;
        }
        break;
      case DIV_INS_ADPCMA:
      case DIV_INS_ADPCMB:
        chanCount[CHANNEL_TYPE_SAMPLE]++;
        break;
      case DIV_INS_VRC6_SAW:
        chanCount[CHANNEL_TYPE_SAW]++;
        break;
      case DIV_INS_POWERNOISE_SLOPE:
        chanCount[CHANNEL_TYPE_SLOPE]++;
        break;
      case DIV_INS_QSOUND:
        chanCount[CHANNEL_TYPE_SAMPLE]++;
        break;
      case DIV_INS_NDS:
        if (whichDef->chanTypes[i]!=DIV_CH_PCM) { // the psg chans can also play samples??
          chanCount[CHANNEL_TYPE_SAMPLE]++;
        }
        chanCount[whichDef->chanTypes[i]]++;
        break;
      case DIV_INS_VERA:
        if (whichDef->chanTypes[i]==DIV_CH_PULSE) {
          chanCount[CHANNEL_TYPE_WAVE]++;
        } else { // sample chan
          chanCount[CHANNEL_TYPE_SAMPLE]++;
        }
        break;
      case DIV_INS_DAVE:
        if (whichDef->chanTypes[i]==DIV_CH_WAVE) {
          chanCount[CHANNEL_TYPE_OTHER]++;
        } else {
          chanCount[whichDef->chanTypes[i]]++;
        }
        break;
      case DIV_INS_SWAN:
        if (whichDef->chanTypes[i]!=DIV_CH_WAVE) {
          chanCount[CHANNEL_TYPE_WAVETABLE]++;
        }
        chanCount[whichDef->chanTypes[i]]++;
        break;
      case DIV_INS_SID3:
        if (whichDef->chanTypes[i]!=DIV_CH_WAVE) {
          chanCount[CHANNEL_TYPE_OTHER]++;
        } else {
          chanCount[CHANNEL_TYPE_WAVE]++;
        }
        break;
      case DIV_INS_C64: // uncategorizable (by me)
      case DIV_INS_TIA:
      case DIV_INS_PET:
      case DIV_INS_SU:
      case DIV_INS_POKEY:
      case DIV_INS_MIKEY:
      case DIV_INS_BIFURCATOR:
      case DIV_INS_SID2:
        chanCount[CHANNEL_TYPE_OTHER]++;
        break;
      default:
        chanCount[whichDef->chanTypes[i]]++;
        break;
    }
  }
  // generate string
  for (int j=0; j<CHANNEL_TYPE_MAX; j++) {
    unsigned char i=chanNamesHierarchy[j];
    if (chanCount[i]==0) continue;
    if (info.length()!=0) {
      info+=", ";
    }
    if (i==CHANNEL_TYPE_OTHER) {
      if (chanCount[i]>1) {
        info+=fmt::sprintf("%d %s",chanCount[i],chanNames[CHANNEL_TYPE_OTHER+1]);
      } else {
        info+=fmt::sprintf("%d %s",chanCount[i],chanNames[CHANNEL_TYPE_OTHER]);
      }
      continue;
    }
    info+=fmt::sprintf("%d × %s",chanCount[i],chanNames[i]);
  }
  ImGui::Text("%s",info.c_str());
}
