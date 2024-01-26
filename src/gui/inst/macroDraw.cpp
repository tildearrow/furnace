/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2024 tildearrow and contributors
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

#include "macroDraw.h"

#include "../gui.h"
#include "../../ta-log.h"
#include "../imgui_internal.h"
#include "../../engine/macroInt.h"
#include "../misc/cpp/imgui_stdlib.h"
#include "../guiConst.h"
#include "../intConst.h"
#include <fmt/printf.h>
#include <imgui.h>
#include "../plot_nolerp.h"

class FurnaceGUI;

const char* macroTypeLabels[4]={
  ICON_FA_BAR_CHART "##IMacroType",
  ICON_FUR_ADSR "##IMacroType",
  ICON_FUR_TRI "##IMacroType",
  ICON_FA_SIGN_OUT "##IMacroType"
};

const char* macroLFOShapes[4]={
  "Triangle", "Saw", "Square", "How did you even"
};


void FurnaceGUI::drawMacroEdit(FurnaceGUIMacroDesc& i, int totalFit, float availableWidth, int index) {
  static float asFloat[256];
  static int asInt[256];
  static float loopIndicator[256];
  static float bit30Indicator[256];
  static bool doHighlight[256];

  if ((i.get_macro()->open&6)==0) {
    for (int j=0; j<256; j++) {
      bit30Indicator[j]=0;
      if (j+macroDragScroll>=i.get_macro()->len) {
        asFloat[j]=0;
        asInt[j]=0;
      } else {
        asFloat[j]=deBit30(i.get_macro()->val[j+macroDragScroll]);
        asInt[j]=deBit30(i.get_macro()->val[j+macroDragScroll])+i.bitOffset;
        if (i.bit30) bit30Indicator[j]=enBit30(i.get_macro()->val[j+macroDragScroll]);
      }
      if (j+macroDragScroll>=i.get_macro()->len || (j+macroDragScroll>i.get_macro()->rel && i.get_macro()->loop<i.get_macro()->rel)) {
        loopIndicator[j]=0;
      } else {
        loopIndicator[j]=((i.get_macro()->loop!=255 && (j+macroDragScroll)>=i.get_macro()->loop))|((i.get_macro()->rel!=255 && (j+macroDragScroll)==i.get_macro()->rel)<<1);
      }
    }
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,ImVec2(0.0f,0.0f));

    if (i.get_macro()->vZoom<1) {
      if (i.get_macro()->macroType==DIV_MACRO_ARP || i.is_arp) {
        i.get_macro()->vZoom=24;
        i.get_macro()->vScroll=120-12;
      } else if (i.get_macro()->macroType==DIV_MACRO_PITCH || i.is_pitch) {
        i.get_macro()->vZoom=128;
        i.get_macro()->vScroll=2048-64;
      } else {
        i.get_macro()->vZoom=i.max-i.min;
        i.get_macro()->vScroll=0;
      }
    }
    if (i.get_macro()->vZoom>(i.max-i.min)) {
      i.get_macro()->vZoom=i.max-i.min;
    }

    memset(doHighlight,0,256*sizeof(bool));
    if (e->isRunning()) for (int j=0; j<e->getTotalChannelCount(); j++) {
      DivChannelState* chanState=e->getChanState(j);
      if (chanState==NULL) continue;

      if (chanState->keyOff) continue;
      if (chanState->lastIns!=curIns) continue;

      DivMacroInt* macroInt=e->getMacroInt(j);
      if (macroInt==NULL) continue;

      DivMacroStruct* macroStruct=macroInt->structByType(i.get_macro()->macroType + (i.oper == 0xff ? 0 : (i.oper * 32)));
      if (macroStruct==NULL) continue;

      if (macroStruct->lastPos>i.get_macro()->len) continue;
      if (macroStruct->lastPos<macroDragScroll) continue;
      if (macroStruct->lastPos>255) continue;
      if (!macroStruct->actualHad) continue;

      doHighlight[macroStruct->lastPos-macroDragScroll]=true;
    }

    if (i.isBitfield) {
      PlotBitfield("##IMacro",asInt,totalFit,0,i.bitfieldBits,i.max,ImVec2(availableWidth,(i.get_macro()->open&1)?(i.height*dpiScale):(32.0f*dpiScale)),sizeof(float),doHighlight);
    } else {
      PlotCustom("##IMacro",asFloat,totalFit,macroDragScroll,NULL,i.min+i.get_macro()->vScroll,i.min+i.get_macro()->vScroll+i.get_macro()->vZoom,ImVec2(availableWidth,(i.get_macro()->open&1)?(i.height*dpiScale):(32.0f*dpiScale)),sizeof(float),i.color,i.get_macro()->len-macroDragScroll,i.hoverFunc,i.hoverFuncUser,i.blockMode,(i.get_macro()->open&1)?genericGuide:NULL,doHighlight);
    }
    if ((i.get_macro()->open&1) && (ImGui::IsItemClicked(ImGuiMouseButton_Left) || ImGui::IsItemClicked(ImGuiMouseButton_Right))) {
      ImGui::InhibitInertialScroll();
      macroDragStart=ImGui::GetItemRectMin();
      macroDragAreaSize=ImVec2(availableWidth,i.height*dpiScale);
      if (i.isBitfield) {
        macroDragMin=i.min;
        macroDragMax=i.max;
      } else {
        macroDragMin=i.min+i.get_macro()->vScroll;
        macroDragMax=i.min+i.get_macro()->vScroll+i.get_macro()->vZoom;
      }
      macroDragBitOff=i.bitOffset;
      macroDragBitMode=i.isBitfield;
      macroDragInitialValueSet=false;
      macroDragInitialValue=false;
      macroDragLen=totalFit;
      macroDragActive=true;
      macroDragBit30=i.bit30;
      macroDragSettingBit30=false;
      macroDragTarget=i.get_macro()->val;
      macroDragChar=false;
      macroDragLineMode=(i.isBitfield)?false:ImGui::IsItemClicked(ImGuiMouseButton_Right);
      macroDragLineInitial=ImVec2(0,0);
      lastMacroDesc=i;
      processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y);
    }
    if ((i.get_macro()->open&1)) {
      if (ImGui::IsItemHovered()) {
        if (ctrlWheeling) {
          if (ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift)) {
            i.get_macro()->vZoom+=wheelY*(1+(i.get_macro()->vZoom>>4));
            if (i.get_macro()->vZoom<1) i.get_macro()->vZoom=1;
            if (i.get_macro()->vZoom>(i.max-i.min)) i.get_macro()->vZoom=i.max-i.min;
            if ((i.get_macro()->vScroll+i.get_macro()->vZoom)>(i.max-i.min)) {
              i.get_macro()->vScroll=(i.max-i.min)-i.get_macro()->vZoom;
            }
          } else {
            macroPointSize+=wheelY;
            if (macroPointSize<1) macroPointSize=1;
            if (macroPointSize>256) macroPointSize=256;
          }
        } else if ((ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift)) && wheelY!=0) {
          i.get_macro()->vScroll+=wheelY*(1+(i.get_macro()->vZoom>>4));
          if (i.get_macro()->vScroll<0) i.get_macro()->vScroll=0;
          if (i.get_macro()->vScroll>((i.max-i.min)-i.get_macro()->vZoom)) i.get_macro()->vScroll=(i.max-i.min)-i.get_macro()->vZoom;
        }
      }

      // slider
      if (!i.isBitfield) {
        if (settings.oldMacroVSlider) {
          ImGui::SameLine(0.0f);
          if (ImGui::VSliderInt("##IMacroVScroll",ImVec2(20.0f*dpiScale,i.height*dpiScale),&i.get_macro()->vScroll,0,(i.max-i.min)-i.get_macro()->vZoom,"",ImGuiSliderFlags_NoInput)) {
            if (i.get_macro()->vScroll<0) i.get_macro()->vScroll=0;
            if (i.get_macro()->vScroll>((i.max-i.min)-i.get_macro()->vZoom)) i.get_macro()->vScroll=(i.max-i.min)-i.get_macro()->vZoom;
          }
          if (ImGui::IsItemHovered() && ctrlWheeling) {
            i.get_macro()->vScroll+=wheelY*(1+(i.get_macro()->vZoom>>4));
            if (i.get_macro()->vScroll<0) i.get_macro()->vScroll=0;
            if (i.get_macro()->vScroll>((i.max-i.min)-i.get_macro()->vZoom)) i.get_macro()->vScroll=(i.max-i.min)-i.get_macro()->vZoom;
          }
        } else {
          ImS64 scrollV=(i.max-i.min-i.get_macro()->vZoom)-i.get_macro()->vScroll;
          ImS64 availV=i.get_macro()->vZoom;
          ImS64 contentsV=(i.max-i.min);

          ImGui::SameLine(0.0f);
          ImGui::SetCursorPosX(ImGui::GetCursorPosX()-ImGui::GetStyle().ItemSpacing.x);
          ImRect scrollbarPos=ImRect(ImGui::GetCursorScreenPos(),ImGui::GetCursorScreenPos());
          scrollbarPos.Max.x+=ImGui::GetStyle().ScrollbarSize;
          scrollbarPos.Max.y+=i.height*dpiScale;
          ImGui::Dummy(ImVec2(ImGui::GetStyle().ScrollbarSize,i.height*dpiScale));
          if (ImGui::IsItemHovered() && ctrlWheeling) {
            i.get_macro()->vScroll+=wheelY*(1+(i.get_macro()->vZoom>>4));
            if (i.get_macro()->vScroll<0) i.get_macro()->vScroll=0;
            if (i.get_macro()->vScroll>((i.max-i.min)-i.get_macro()->vZoom)) i.get_macro()->vScroll=(i.max-i.min)-i.get_macro()->vZoom;
          }

          ImGuiID scrollbarID=ImGui::GetID("##IMacroVScroll");
          ImGui::KeepAliveID(scrollbarID);
          if (ImGui::ScrollbarEx(scrollbarPos,scrollbarID,ImGuiAxis_Y,&scrollV,availV,contentsV,0)) {
            i.get_macro()->vScroll=(i.max-i.min-i.get_macro()->vZoom)-scrollV;
          }
        }
      }

      // bit 30 area
      if (i.bit30) {
        PlotCustom("##IMacroBit30",bit30Indicator,totalFit,macroDragScroll,NULL,0,1,ImVec2(availableWidth,12.0f*dpiScale),sizeof(float),i.color,i.get_macro()->len-macroDragScroll,&macroHoverBit30);
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
          ImGui::InhibitInertialScroll();
          macroDragStart=ImGui::GetItemRectMin();
          macroDragAreaSize=ImVec2(availableWidth,12.0f*dpiScale);
          macroDragInitialValueSet=false;
          macroDragInitialValue=false;
          macroDragLen=totalFit;
          macroDragActive=true;
          macroDragBit30=i.bit30;
          macroDragSettingBit30=true;
          macroDragTarget=i.get_macro()->val;
          macroDragChar=false;
          macroDragLineMode=false;
          macroDragLineInitial=ImVec2(0,0);
          lastMacroDesc=i;
          processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y);
        }
      }

      // loop area
      PlotCustom("##IMacroLoop",loopIndicator,totalFit,macroDragScroll,NULL,0,2,ImVec2(availableWidth,12.0f*dpiScale),sizeof(float),i.color,i.get_macro()->len-macroDragScroll,&macroHoverLoop);
      if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        ImGui::InhibitInertialScroll();
        macroLoopDragStart=ImGui::GetItemRectMin();
        macroLoopDragAreaSize=ImVec2(availableWidth,12.0f*dpiScale);
        macroLoopDragLen=totalFit;
        if (ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift)) {
          macroLoopDragTarget=&i.get_macro()->rel;
        } else {
          macroLoopDragTarget=&i.get_macro()->loop;
        }
        macroLoopDragActive=true;
        processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y);
      }
      if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        ImGui::InhibitInertialScroll();
        if (ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift)) {
          i.get_macro()->rel=255;
        } else {
          i.get_macro()->loop=255;
        }
      }
      ImGui::SetNextItemWidth(availableWidth);
      String& mmlStr=mmlString[index];
      if (ImGui::InputText("##IMacroMML",&mmlStr)) {
        decodeMMLStr(mmlStr,i.get_macro()->val,i.get_macro()->len,i.get_macro()->loop,i.min,(i.isBitfield)?((1<<(i.isBitfield?i.max:0))-1):i.max,i.get_macro()->rel,i.bit30);
      }
      if (!ImGui::IsItemActive()) {
        encodeMMLStr(mmlStr,i.get_macro()->val,i.get_macro()->len,i.get_macro()->loop,i.get_macro()->rel,false,i.bit30);
      }
    }
    ImGui::PopStyleVar();
  } else {
    if (i.get_macro()->open&2) {
      if (ImGui::BeginTable("MacroADSR",4)) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.3);
        ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthStretch,0.3);
        //ImGui::TableSetupColumn("c4",ImGuiTableColumnFlags_WidthStretch,0.4);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Bottom");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::InputInt("##MABottom",&i.get_macro()->val[0],1,16)) { PARAMETER
          if (i.get_macro()->val[0]<i.min) i.get_macro()->val[0]=i.min;
          if (i.get_macro()->val[0]>i.max) i.get_macro()->val[0]=i.max;
        }

        ImGui::TableNextColumn();
        ImGui::Text("Top");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::InputInt("##MATop",&i.get_macro()->val[1],1,16)) { PARAMETER
          if (i.get_macro()->val[1]<i.min) i.get_macro()->val[1]=i.min;
          if (i.get_macro()->val[1]>i.max) i.get_macro()->val[1]=i.max;
        }

        /*ImGui::TableNextColumn();
        ImGui::Text("the envelope goes here");*/

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Attack");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (CWSliderInt("##MAAR",&i.get_macro()->val[2],0,255)) { PARAMETER
          if (i.get_macro()->val[2]<0) i.get_macro()->val[2]=0;
          if (i.get_macro()->val[2]>255) i.get_macro()->val[2]=255;
        } rightClickable

        ImGui::TableNextColumn();
        ImGui::Text("Sustain");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (CWSliderInt("##MASL",&i.get_macro()->val[5],0,255)) { PARAMETER
          if (i.get_macro()->val[5]<0) i.get_macro()->val[5]=0;
          if (i.get_macro()->val[5]>255) i.get_macro()->val[5]=255;
        } rightClickable

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Hold");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (CWSliderInt("##MAHT",&i.get_macro()->val[3],0,255)) { PARAMETER
          if (i.get_macro()->val[3]<0) i.get_macro()->val[3]=0;
          if (i.get_macro()->val[3]>255) i.get_macro()->val[3]=255;
        } rightClickable

        ImGui::TableNextColumn();
        ImGui::Text("SusTime");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (CWSliderInt("##MAST",&i.get_macro()->val[6],0,255)) { PARAMETER
          if (i.get_macro()->val[6]<0) i.get_macro()->val[6]=0;
          if (i.get_macro()->val[6]>255) i.get_macro()->val[6]=255;
        } rightClickable

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Decay");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (CWSliderInt("##MADR",&i.get_macro()->val[4],0,255)) { PARAMETER
          if (i.get_macro()->val[4]<0) i.get_macro()->val[4]=0;
          if (i.get_macro()->val[4]>255) i.get_macro()->val[4]=255;
        } rightClickable

        ImGui::TableNextColumn();
        ImGui::Text("SusDecay");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (CWSliderInt("##MASR",&i.get_macro()->val[7],0,255)) { PARAMETER
          if (i.get_macro()->val[7]<0) i.get_macro()->val[7]=0;
          if (i.get_macro()->val[7]>255) i.get_macro()->val[7]=255;
        } rightClickable

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Release");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (CWSliderInt("##MARR",&i.get_macro()->val[8],0,255)) { PARAMETER
          if (i.get_macro()->val[8]<0) i.get_macro()->val[8]=0;
          if (i.get_macro()->val[8]>255) i.get_macro()->val[8]=255;
        } rightClickable

        ImGui::EndTable();
      }
    }
    if (i.get_macro()->open&4) {
      if (ImGui::BeginTable("MacroLFO",4)) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.3);
        ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthStretch,0.3);
        //ImGui::TableSetupColumn("c4",ImGuiTableColumnFlags_WidthStretch,0.4);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Bottom");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::InputInt("##MABottom",&i.get_macro()->val[0],1,16)) { PARAMETER
          if (i.get_macro()->val[0]<i.min) i.get_macro()->val[0]=i.min;
          if (i.get_macro()->val[0]>i.max) i.get_macro()->val[0]=i.max;
        }

        ImGui::TableNextColumn();
        ImGui::Text("Top");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::InputInt("##MATop",&i.get_macro()->val[1],1,16)) { PARAMETER
          if (i.get_macro()->val[1]<i.min) i.get_macro()->val[1]=i.min;
          if (i.get_macro()->val[1]>i.max) i.get_macro()->val[1]=i.max;
        }

        /*ImGui::TableNextColumn();
        ImGui::Text("the envelope goes here");*/

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Speed");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (CWSliderInt("##MLSpeed",&i.get_macro()->val[11],0,255)) { PARAMETER
          if (i.get_macro()->val[11]<0) i.get_macro()->val[11]=0;
          if (i.get_macro()->val[11]>255) i.get_macro()->val[11]=255;
        } rightClickable

        ImGui::TableNextColumn();
        ImGui::Text("Phase");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (CWSliderInt("##MLPhase",&i.get_macro()->val[13],0,1023)) { PARAMETER
          if (i.get_macro()->val[13]<0) i.get_macro()->val[13]=0;
          if (i.get_macro()->val[13]>1023) i.get_macro()->val[13]=1023;
        } rightClickable

        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Shape");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (CWSliderInt("##MLShape",&i.get_macro()->val[12],0,2,macroLFOShapes[i.get_macro()->val[12]&3])) { PARAMETER
          if (i.get_macro()->val[12]<0) i.get_macro()->val[12]=0;
          if (i.get_macro()->val[12]>2) i.get_macro()->val[12]=2;
        } rightClickable

        ImGui::EndTable();
      }
    }
  }
}

#define BUTTON_TO_SET_MODE(buttonType) \
  if (buttonType(macroTypeLabels[(i.get_macro()->open>>1)&3])) { \
    unsigned char prevOpen=i.get_macro()->open; \
    if (i.get_macro()->open>=4) { \
      i.get_macro()->open&=(~6); \
    } else { \
      i.get_macro()->open+=2; \
    } \
\
    /* check whether macro type is now ADSR/LFO or sequence */ \
    if (((prevOpen&6)?1:0)!=((i.get_macro()->open&6)?1:0)) { \
      /* swap memory */ \
      /* this way the macro isn't corrupted if the user decides to go */ \
      /* back to sequence mode */ \
      i.get_macro()->len^=i.get_macro()->lenMemory; \
      i.get_macro()->lenMemory^=i.get_macro()->len; \
      i.get_macro()->len^=i.get_macro()->lenMemory; \
\
      for (int j=0; j<16; j++) { \
        i.get_macro()->val[j]^=i.get_macro()->typeMemory[j]; \
        i.get_macro()->typeMemory[j]^=i.get_macro()->val[j]; \
        i.get_macro()->val[j]^=i.get_macro()->typeMemory[j]; \
      } \
\
      /* if ADSR/LFO, populate min/max */ \
      if (i.get_macro()->open&6) { \
        if (i.get_macro()->val[0]==0 && i.get_macro()->val[1]==0) { \
          i.get_macro()->val[0]=i.min; \
          i.get_macro()->val[1]=i.max; \
        } \
        i.get_macro()->val[0]=CLAMP(i.get_macro()->val[0],i.min,i.max); \
        i.get_macro()->val[1]=CLAMP(i.get_macro()->val[1],i.min,i.max); \
      } \
    } \
    PARAMETER; \
  } \
  if (ImGui::IsItemHovered()) { \
    switch (i.get_macro()->open&6) { \
      case 0: \
        ImGui::SetTooltip("Macro type: Sequence"); \
        break; \
      case 2: \
        ImGui::SetTooltip("Macro type: ADSR"); \
        break; \
      case 4: \
        ImGui::SetTooltip("Macro type: LFO"); \
        break; \
      default: \
        ImGui::SetTooltip("Macro type: What's going on here?"); \
        break; \
    } \
  } \
  if (i.get_macro()->open&6) { \
    i.get_macro()->len=16; \
  }

#define BUTTON_TO_SET_PROPS(_x) \
  pushToggleColors(_x.get_macro()->speed!=1 || _x.get_macro()->delay); \
  ImGui::Button(ICON_FA_ELLIPSIS_H "##IMacroSet"); \
  popToggleColors(); \
  if (ImGui::IsItemHovered()) { \
    ImGui::SetTooltip("Delay/Step Length"); \
  } \
  if (ImGui::BeginPopupContextItem("IMacroSetP",ImGuiPopupFlags_MouseButtonLeft)) { \
    if (ImGui::InputScalar("Step Length (ticks)##IMacroSpeed",ImGuiDataType_U8,&_x.get_macro()->speed,&_ONE,&_THREE)) { \
      if (_x.get_macro()->speed<1) _x.get_macro()->speed=1; \
      MARK_MODIFIED; \
    } \
    if (ImGui::InputScalar("Delay##IMacroDelay",ImGuiDataType_U8,&_x.get_macro()->delay,&_ONE,&_THREE)) { \
      MARK_MODIFIED; \
    } \
    ImGui::EndPopup(); \
  }

#define BUTTON_TO_SET_RELEASE(buttonType) \
  pushToggleColors(i.get_macro()->open&8); \
  if (buttonType(ICON_FA_BOLT "##IMacroRelMode")) { \
    i.get_macro()->open^=8; \
  } \
  if (ImGui::IsItemHovered()) { \
    if (i.get_macro()->open&8) { \
      ImGui::SetTooltip("Release mode: Active (jump to release pos)"); \
    } else { \
      ImGui::SetTooltip("Release mode: Passive (delayed release)"); \
    } \
  } \
  popToggleColors(); \

void FurnaceGUI::drawMacros(std::vector<FurnaceGUIMacroDesc>& macros, FurnaceGUIMacroEditState& state) {
  int index=0;
  float reservedSpace=(settings.oldMacroVSlider)?(20.0f*dpiScale+ImGui::GetStyle().ItemSpacing.x):ImGui::GetStyle().ScrollbarSize;

  float max_macro_len = 0.0;

  for(int i = 0; i < (int)macros.size(); i++) //calculate table column width (the one where macro names go) to eliminate one-frame glitch
  {
    float current_width = ImGui::CalcTextSize(macros[i].displayName).x;

    if(current_width > max_macro_len)
    {
      max_macro_len = current_width;
    }
  }

  max_macro_len += 25.0f * dpiScale; //for the arrow that expands/collapses the macro, located to the right of macro name

  if(max_macro_len < 120.0f*dpiScale) max_macro_len = 120.0f*dpiScale;

  switch (settings.macroLayout) {
    case 0: {
      if (ImGui::BeginTable("MacroSpace",2)) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed,max_macro_len);
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.0);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        float lenAvail=ImGui::GetContentRegionAvail().x;
        //ImGui::Dummy(ImVec2(120.0f*dpiScale,dpiScale));
        ImGui::SetNextItemWidth(120.0f*dpiScale);
        if (ImGui::InputInt("##MacroPointSize",&macroPointSize,1,4)) {
          if (macroPointSize<1) macroPointSize=1;
          if (macroPointSize>256) macroPointSize=256;
        }
        ImGui::TableNextColumn();
        float availableWidth=ImGui::GetContentRegionAvail().x-reservedSpace;
        int totalFit=MIN(255,availableWidth/MAX(1,macroPointSize*dpiScale));
        int scrollMax=0;
        for (FurnaceGUIMacroDesc& i: macros) {
          if (i.get_macro()->len>scrollMax) scrollMax=i.get_macro()->len;
        }
        scrollMax-=totalFit;
        if (scrollMax<0) scrollMax=0;
        if (macroDragScroll>scrollMax) {
          macroDragScroll=scrollMax;
        }
        ImGui::BeginDisabled(scrollMax<1);
        ImGui::SetNextItemWidth(availableWidth);
        if (CWSliderInt("##MacroScroll",&macroDragScroll,0,scrollMax,"")) {
          if (macroDragScroll<0) macroDragScroll=0;
          if (macroDragScroll>scrollMax) macroDragScroll=scrollMax;
        }
        ImGui::EndDisabled();

        // draw macros
        for (FurnaceGUIMacroDesc& i: macros) {
          ImGui::PushID(index);
          ImGui::TableNextRow();

          // description
          ImGui::TableNextColumn();
          ImGui::Text("%s",i.displayName);
          ImGui::SameLine();
          if (ImGui::SmallButton((i.get_macro()->open&1)?(ICON_FA_CHEVRON_UP "##IMacroOpen"):(ICON_FA_CHEVRON_DOWN "##IMacroOpen"))) {
            i.get_macro()->open^=1;
          }
          if (i.get_macro()->open&1) {
            if ((i.get_macro()->open&6)==0) {
              ImGui::SetNextItemWidth(lenAvail);
              int macroLen=i.get_macro()->len;
              if (ImGui::InputScalar("##IMacroLen",ImGuiDataType_U8,&macroLen,&_ONE,&_THREE)) { MARK_MODIFIED
                if (macroLen<0) macroLen=0;
                if (macroLen>255) macroLen=255;
                i.get_macro()->len=macroLen;
              }
            }
            BUTTON_TO_SET_MODE(ImGui::Button);
            ImGui::SameLine();
            BUTTON_TO_SET_PROPS(i);
            if ((i.get_macro()->open&6)==0) {
              ImGui::SameLine();
              BUTTON_TO_SET_RELEASE(ImGui::Button);
            }
            // do not change this!
            // anything other than a checkbox will look ugly!
            // if you really need more than two macro modes please tell me.
            if (i.modeName!=NULL) {
              bool modeVal=i.get_macro()->mode;
              String modeName=fmt::sprintf("%s##IMacroMode",i.modeName);
              if (ImGui::Checkbox(modeName.c_str(),&modeVal)) {
                i.get_macro()->mode=modeVal;
              }
            }
          }

          // macro area
          ImGui::TableNextColumn();
          drawMacroEdit(i,totalFit,availableWidth,index);
          ImGui::PopID();
          index++;
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        ImGui::BeginDisabled(scrollMax<1);
        ImGui::SetNextItemWidth(availableWidth);
        if (CWSliderInt("##MacroScroll",&macroDragScroll,0,scrollMax,"")) {
          if (macroDragScroll<0) macroDragScroll=0;
          if (macroDragScroll>scrollMax) macroDragScroll=scrollMax;
        }
        ImGui::EndDisabled();
        ImGui::EndTable();
      }
      break;
    }
    case 1: {
      ImGui::Text("Tabs");
      break;
    }
    case 2: {
      int columns=round(ImGui::GetContentRegionAvail().x/(400.0*dpiScale));
      int curColumn=0;
      if (columns<1) columns=1;
      if (ImGui::BeginTable("MacroGrid",columns,ImGuiTableFlags_BordersInner)) {
        for (FurnaceGUIMacroDesc& i: macros) {
          if (curColumn==0) ImGui::TableNextRow();
          ImGui::TableNextColumn();

          if (++curColumn>=columns) curColumn=0;
          
          float availableWidth=ImGui::GetContentRegionAvail().x-reservedSpace;
          int totalFit=i.get_macro()->len;
          if (totalFit<1) totalFit=1;

          ImGui::PushID(index);

          ImGui::TextUnformatted(i.displayName);
          ImGui::SameLine();
          if (ImGui::SmallButton((i.get_macro()->open&1)?(ICON_FA_CHEVRON_UP "##IMacroOpen"):(ICON_FA_CHEVRON_DOWN "##IMacroOpen"))) {
            i.get_macro()->open^=1;
          }

          if (i.get_macro()->open&1) {
            ImGui::SameLine();
            BUTTON_TO_SET_MODE(ImGui::SmallButton);
          }

          drawMacroEdit(i,totalFit,availableWidth,index);

          if (i.get_macro()->open&1) {
            if ((i.get_macro()->open&6)==0) {
              ImGui::Text("Length");
              ImGui::SameLine();
              ImGui::SetNextItemWidth(120.0f*dpiScale);
              int macroLen=i.get_macro()->len;
              if (ImGui::InputScalar("##IMacroLen",ImGuiDataType_U8,&macroLen,&_ONE,&_THREE)) { MARK_MODIFIED
                if (macroLen<0) macroLen=0;
                if (macroLen>255) macroLen=255;
                i.get_macro()->len=macroLen;
              }
              ImGui::SameLine();
            }
            BUTTON_TO_SET_PROPS(i);
            if ((i.get_macro()->open&6)==0) {
              ImGui::SameLine();
              BUTTON_TO_SET_RELEASE(ImGui::Button);
            }
            if (i.modeName!=NULL) {
              bool modeVal=i.get_macro()->mode;
              String modeName=fmt::sprintf("%s##IMacroMode",i.modeName);
              ImGui::SameLine();
              if (ImGui::Checkbox(modeName.c_str(),&modeVal)) {
                i.get_macro()->mode=modeVal;
              }
            }
          }

          ImGui::PopID();
          index++;
        }
        ImGui::EndTable();
      }
      break;
    }
    case 3: {
      if (ImGui::BeginTable("MacroList",2,ImGuiTableFlags_Borders)) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        for (size_t i=0; i<macros.size(); i++) {
          if (ImGui::Selectable(macros[i].displayName,state.selectedMacro==(int)i)) {
            state.selectedMacro=i;
          }
        }

        ImGui::TableNextColumn();
        float availableWidth=ImGui::GetContentRegionAvail().x-reservedSpace;
        int totalFit=MIN(255,availableWidth/MAX(1,macroPointSize*dpiScale));
        if (macroDragScroll>255-totalFit) {
          macroDragScroll=255-totalFit;
        }

        if (state.selectedMacro<0 || state.selectedMacro>=(int)macros.size()) {
          state.selectedMacro=0;
        }

        if (state.selectedMacro>=0 && state.selectedMacro<(int)macros.size()) {
          FurnaceGUIMacroDesc& m=macros[state.selectedMacro];
          m.get_macro()->open|=1;

          float availableWidth=ImGui::GetContentRegionAvail().x-reservedSpace;
          int totalFit=MIN(255,availableWidth/MAX(1,macroPointSize*dpiScale));
          int scrollMax=0;
          for (FurnaceGUIMacroDesc& i: macros) {
            if (i.get_macro()->len>scrollMax) scrollMax=i.get_macro()->len;
          }
          scrollMax-=totalFit;
          if (scrollMax<0) scrollMax=0;
          if (macroDragScroll>scrollMax) {
            macroDragScroll=scrollMax;
          }
          ImGui::BeginDisabled(scrollMax<1);
          ImGui::SetNextItemWidth(availableWidth);
          if (CWSliderInt("##MacroScroll",&macroDragScroll,0,scrollMax,"")) {
            if (macroDragScroll<0) macroDragScroll=0;
            if (macroDragScroll>scrollMax) macroDragScroll=scrollMax;
          }
          ImGui::EndDisabled();

          ImGui::SameLine();
          ImGui::Button(ICON_FA_SEARCH_PLUS "##MacroZoomB");
          if (ImGui::BeginPopupContextItem("MacroZoomP",ImGuiPopupFlags_MouseButtonLeft)) {
            ImGui::SetNextItemWidth(120.0f*dpiScale);
            if (ImGui::InputInt("##MacroPointSize",&macroPointSize,1,4)) {
              if (macroPointSize<1) macroPointSize=1;
              if (macroPointSize>256) macroPointSize=256;
            }
            ImGui::EndPopup();
          }

          m.height=ImGui::GetContentRegionAvail().y-ImGui::GetFontSize()-ImGui::GetFrameHeightWithSpacing()-(m.bit30?28.0f:12.0f)*dpiScale-ImGui::GetStyle().ItemSpacing.y*3.0f;
          if (m.height<10.0f*dpiScale) m.height=10.0f*dpiScale;
          m.height/=dpiScale;
          drawMacroEdit(m,totalFit,availableWidth,index);

          if (m.get_macro()->open&1) {
            if ((m.get_macro()->open&6)==0) {
              ImGui::Text("Length");
              ImGui::SameLine();
              ImGui::SetNextItemWidth(120.0f*dpiScale);
              int macroLen=m.get_macro()->len;
              if (ImGui::InputScalar("##IMacroLen",ImGuiDataType_U8,&macroLen,&_ONE,&_THREE)) { MARK_MODIFIED
                if (macroLen<0) macroLen=0;
                if (macroLen>255) macroLen=255;
                m.get_macro()->len=macroLen;
              }
              ImGui::SameLine();
            }
            ImGui::Text("StepLen");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(120.0f*dpiScale);
            if (ImGui::InputScalar("##IMacroSpeed",ImGuiDataType_U8,&m.get_macro()->speed,&_ONE,&_THREE)) {
              if (m.get_macro()->speed<1) m.get_macro()->speed=1;
              MARK_MODIFIED;
            }
            ImGui::SameLine();
            ImGui::Text("Delay");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(120.0f*dpiScale);
            if (ImGui::InputScalar("##IMacroDelay",ImGuiDataType_U8,&m.get_macro()->delay,&_ONE,&_THREE)) {
              MARK_MODIFIED;
            }
            ImGui::SameLine();
            {
              FurnaceGUIMacroDesc& i=m;
              BUTTON_TO_SET_MODE(ImGui::Button);
              if ((i.get_macro()->open&6)==0) {
                ImGui::SameLine();
                BUTTON_TO_SET_RELEASE(ImGui::Button);
              }
            }
            if (m.modeName!=NULL) {
              bool modeVal=m.get_macro()->mode;
              String modeName=fmt::sprintf("%s##IMacroMode",m.modeName);
              ImGui::SameLine();
              if (ImGui::Checkbox(modeName.c_str(),&modeVal)) {
                m.get_macro()->mode=modeVal;
              }
            }
          } else {
            ImGui::Text("The heck? No, this isn't even working correctly...");
          }
        } else {
          ImGui::Text("The only problem with that selectedMacro is that it's a bug...");
        }

        // goes here
        ImGui::EndTable();
      }
      break;
    }
    case 4: {
      ImGui::Text("Single (combo box)");
      break;
    }
  }
}