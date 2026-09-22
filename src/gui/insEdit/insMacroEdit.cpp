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
#include "../guiConst.h"
#include "../intConst.h"
#include "../plot_nolerp.h"
#include "misc/cpp/imgui_stdlib.h"
#include "IconsFontAwesome4.h"
#include "furIcons.h"

const char* macroTypeLabels[4]={
  ICON_FA_BAR_CHART "##IMacroType",
  ICON_FUR_ADSR "##IMacroType",
  ICON_FUR_TRI "##IMacroType",
  ICON_FA_SIGN_OUT "##IMacroType"
};

const char* macroLFOShapes[4]={
  _N("Triangle"),
  _N("Saw"),
  _N("Square"),
  _N("How did you even")
};

String macroHover(int id, float val, void* u) {
  return fmt::sprintf("%d: %d",id,(int)val);
}

String macroHoverLoop(int id, float val, void* u) {
  if (val>1) return _("Release");
  if (val>0) return _("Loop");
  return "";
}

String macroHoverBit30(int id, float val, void* u) {
  if (val>0) return _("Fixed");
  return _("Relative");
}

String genericGuide(float value) {
  return fmt::sprintf("%d",(int)value);
}

inline int deBit30(const int val) {
  if ((val&0xc0000000)==0x40000000 || (val&0xc0000000)==0x80000000) return val^0x40000000;
  return val;
}

inline bool enBit30(const int val) {
  if ((val&0xc0000000)==0x40000000 || (val&0xc0000000)==0x80000000) return true;
  return false;
}

#define MACRO_VZOOM i.ins->temp.vZoom[i.macro->macroType]
#define MACRO_VSCROLL i.ins->temp.vScroll[i.macro->macroType]

#define ADSR_LENGTH_HINT(_x,_range) \
  if (_x<1) { \
    ImGui::SetTooltip(_("forever")); \
  } else { \
    const int lHint=((((_range)<<8)|0xff)+(_x)-1)/(_x); \
    ImGui::SetTooltip(ngettext("~%d tick","~%d ticks",lHint),lHint); \
  }

#define ADSR_LENGTH_HINT_REL(_x,_range) \
  if (_x<1) { \
    ImGui::SetTooltip(_("instant")); \
  } else { \
    const int lHint=((((_range)<<8)|0xff)+(_x)-1)/(_x); \
    ImGui::SetTooltip(ngettext("~%d tick","~%d ticks",lHint),lHint); \
  }

void FurnaceGUI::drawMacroEdit(FurnaceGUIMacroDesc& i, int totalFit, float availableWidth, int index) {
  static float asFloat[256];
  static int asInt[256];
  static float loopIndicator[256];
  static float bit30Indicator[256];
  static bool doHighlight[256];

  const auto updateRangeInputs=[&]() {
    // Update range inputs to match the current instrument, if the instrument has changed.
    // It's a lambda because it needs to be called later on, or else the values might not have been initialized yet.
    // FIXME: are there any other cases that may modify these values, other than "current instrument changed"?
    if (insEditMacroInsChanged) {
      insEditMacroEnvBottom=i.macro->val[0];
      insEditMacroEnvTop=i.macro->val[1];
      insEditMacroInsChanged=false;
    }
  };

  if ((i.macro->open&6)==0) {
    for (int j=0; j<256; j++) {
      bit30Indicator[j]=0;
      if (j+macroDragScroll>=i.macro->len) {
        asFloat[j]=0;
        asInt[j]=0;
      } else {
        asFloat[j]=deBit30(i.macro->val[j+macroDragScroll]);
        asInt[j]=deBit30(i.macro->val[j+macroDragScroll]);
        if (i.bit30) bit30Indicator[j]=enBit30(i.macro->val[j+macroDragScroll]);
      }
      if (j+macroDragScroll>=i.macro->len || (j+macroDragScroll>i.macro->rel && i.macro->loop<i.macro->rel)) {
        loopIndicator[j]=0;
      } else {
        loopIndicator[j]=((i.macro->loop!=255 && (j+macroDragScroll)>=i.macro->loop))|((i.macro->rel!=255 && (j+macroDragScroll)==i.macro->rel)<<1);
      }
    }
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,ImVec2(0.0f,0.0f));
    if (MACRO_VZOOM<1) {
      if (i.macro->macroType==DIV_MACRO_ARP || i.isArp) {
        MACRO_VZOOM=24;
        MACRO_VSCROLL=120-12;
      }
      else if ((i.macro->macroType == DIV_MACRO_PITCH || i.isPitch) || (i.macro->macroType == DIV_MACRO_EX7 && i.isPitch)) {
        MACRO_VZOOM=128;
        MACRO_VSCROLL=2048-64;
      } else {
        MACRO_VZOOM=i.max-i.min;
        MACRO_VSCROLL=0;
      }
    }
    if (MACRO_VZOOM>(i.max-i.min)) {
      MACRO_VZOOM=i.max-i.min;
    }

    memset(doHighlight,0,256*sizeof(bool));
    if (e->isRunning()) for (int j=0; j<e->getTotalChannelCount(); j++) {
      DivChannelState* chanState=e->getChanState(j);
      if (chanState==NULL) continue;

      if (chanState->keyOff) continue;
      if (chanState->lastIns!=curIns) continue;

      DivMacroInt* macroInt=e->getMacroInt(j);
      if (macroInt==NULL) continue;

      DivMacroStruct* macroStruct=macroInt->structByType(i.macro->macroType);
      if (macroStruct==NULL) continue;

      if (macroStruct->lastPos>i.macro->len) continue;
      if (macroStruct->lastPos<macroDragScroll) continue;
      if (macroStruct->lastPos>255) continue;
      if (!macroStruct->actualHad) continue;

      doHighlight[macroStruct->lastPos-macroDragScroll]=true;
    }

    if (i.isBitfield) {
      PlotBitfield("##IMacro",asInt,totalFit,0,i.bitfieldBits,i.max,ImVec2(availableWidth,(i.macro->open&1)?(i.height*dpiScale):(32.0f*dpiScale)),sizeof(float),doHighlight,uiColors[GUI_COLOR_MACRO_HIGHLIGHT],i.color,i.hoverFunc,i.hoverFuncUser);
    } else {
      PlotCustom("##IMacro",asFloat,totalFit,macroDragScroll,NULL,i.min+MACRO_VSCROLL,i.min+MACRO_VSCROLL+MACRO_VZOOM,ImVec2(availableWidth,(i.macro->open&1)?(i.height*dpiScale):(32.0f*dpiScale)),sizeof(float),i.color,i.macro->len-macroDragScroll,i.hoverFunc,i.hoverFuncUser,i.blockMode,(i.macro->open&1)?genericGuide:NULL,doHighlight,uiColors[GUI_COLOR_MACRO_HIGHLIGHT]);
    }
    if ((i.macro->open&1) && (ImGui::IsItemClicked(ImGuiMouseButton_Left) || ImGui::IsItemClicked(ImGuiMouseButton_Right))) {
      ImGui::InhibitInertialScroll();
      macroDragStart=ImGui::GetItemRectMin();
      macroDragAreaSize=ImVec2(availableWidth,i.height*dpiScale);
      if (i.isBitfield) {
        macroDragMin=i.min;
        macroDragMax=i.max;
      } else {
        macroDragMin=i.min+MACRO_VSCROLL;
        macroDragMax=i.min+MACRO_VSCROLL+MACRO_VZOOM;
      }
      macroDragBitMode=i.isBitfield;
      macroDragInitialValueSet=false;
      macroDragInitialValue=false;
      macroDragLen=totalFit;
      macroDragActive=true;
      macroDragBit30=i.bit30;
      macroDragSettingBit30=false;
      macroDragTarget=i.macro->val;
      macroDragChar=false;
      macroDragLineMode=(i.isBitfield)?false:ImGui::IsItemClicked(ImGuiMouseButton_Right);
      macroDragLineInitial=ImVec2(0,0);
      lastMacroDesc=i;
      processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y);
    }
    if ((i.macro->open&1)) {
      if (ImGui::IsItemHovered()) {
        if (ctrlWheeling) {
          if (ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift)) {
            MACRO_VZOOM+=wheelY*(1+(MACRO_VZOOM>>4));
            if (MACRO_VZOOM<1) MACRO_VZOOM=1;
            if (MACRO_VZOOM>(i.max-i.min)) MACRO_VZOOM=i.max-i.min;
            if ((MACRO_VSCROLL+MACRO_VZOOM)>(i.max-i.min)) {
              MACRO_VSCROLL=(i.max-i.min)-MACRO_VZOOM;
            }
          } else if (settings.autoMacroStepSize==0) {
            macroPointSize+=wheelY;
            if (macroPointSize<1) macroPointSize=1;
            if (macroPointSize>256) macroPointSize=256;
          }
        } else if ((ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift)) && wheelY!=0) {
          MACRO_VSCROLL+=wheelY*(1+(MACRO_VZOOM>>4));
          if (MACRO_VSCROLL<0) MACRO_VSCROLL=0;
          if (MACRO_VSCROLL>((i.max-i.min)-MACRO_VZOOM)) MACRO_VSCROLL=(i.max-i.min)-MACRO_VZOOM;
        }
      }

      // slider
      if (!i.isBitfield) {
        if (settings.oldMacroVSlider) {
          ImGui::SameLine(0.0f);
          if (ImGui::VSliderInt("##IMacroVScroll",ImVec2(20.0f*dpiScale,i.height*dpiScale),&MACRO_VSCROLL,0,(i.max-i.min)-MACRO_VZOOM,"",ImGuiSliderFlags_NoInput)) {
            if (MACRO_VSCROLL<0) MACRO_VSCROLL=0;
            if (MACRO_VSCROLL>((i.max-i.min)-MACRO_VZOOM)) MACRO_VSCROLL=(i.max-i.min)-MACRO_VZOOM;
          }
          if (ImGui::IsItemHovered() && ctrlWheeling) {
            MACRO_VSCROLL+=wheelY*(1+(MACRO_VZOOM>>4));
            if (MACRO_VSCROLL<0) MACRO_VSCROLL=0;
            if (MACRO_VSCROLL>((i.max-i.min)-MACRO_VZOOM)) MACRO_VSCROLL=(i.max-i.min)-MACRO_VZOOM;
          }
        } else {
          ImS64 scrollV=(i.max-i.min-MACRO_VZOOM)-MACRO_VSCROLL;
          ImS64 availV=MACRO_VZOOM;
          ImS64 contentsV=(i.max-i.min);

          ImGui::SameLine(0.0f);
          ImGui::SetCursorPosX(ImGui::GetCursorPosX()-ImGui::GetStyle().ItemSpacing.x);
          ImRect scrollbarPos=ImRect(ImGui::GetCursorScreenPos(),ImGui::GetCursorScreenPos());
          scrollbarPos.Max.x+=ImGui::GetStyle().ScrollbarSize;
          scrollbarPos.Max.y+=i.height*dpiScale;
          ImGui::Dummy(ImVec2(ImGui::GetStyle().ScrollbarSize,i.height*dpiScale));
          if (ImGui::IsItemHovered() && ctrlWheeling) {
            MACRO_VSCROLL+=wheelY*(1+(MACRO_VZOOM>>4));
            if (MACRO_VSCROLL<0) MACRO_VSCROLL=0;
            if (MACRO_VSCROLL>((i.max-i.min)-MACRO_VZOOM)) MACRO_VSCROLL=(i.max-i.min)-MACRO_VZOOM;
          }

          ImGuiID scrollbarID=ImGui::GetID("##IMacroVScroll");
          ImGui::KeepAliveID(scrollbarID);
          if (ImGui::ScrollbarEx(scrollbarPos,scrollbarID,ImGuiAxis_Y,&scrollV,availV,contentsV,0)) {
            MACRO_VSCROLL=(i.max-i.min-MACRO_VZOOM)-scrollV;
          }
        }
      }

      // bit 30 area
      if (i.bit30) {
        PlotCustom("##IMacroBit30",bit30Indicator,totalFit,macroDragScroll,NULL,0,1,ImVec2(availableWidth,12.0f*dpiScale),sizeof(float),i.color,i.macro->len-macroDragScroll,&macroHoverBit30);
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
          macroDragTarget=i.macro->val;
          macroDragChar=false;
          macroDragLineMode=false;
          macroDragLineInitial=ImVec2(0,0);
          lastMacroDesc=i;
          processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y);
        }
      }

      // loop area
      PlotCustom("##IMacroLoop",loopIndicator,totalFit,macroDragScroll,NULL,0,2,ImVec2(availableWidth,12.0f*dpiScale),sizeof(float),i.color,i.macro->len-macroDragScroll,&macroHoverLoop);
      if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        ImGui::InhibitInertialScroll();
        macroLoopDragStart=ImGui::GetItemRectMin();
        macroLoopDragAreaSize=ImVec2(availableWidth,12.0f*dpiScale);
        macroLoopDragLen=totalFit;
        if (ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift)) {
          macroLoopDragTarget=&i.macro->rel;
        } else {
          macroLoopDragTarget=&i.macro->loop;
        }
        macroLoopDragActive=true;
        processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y);
      }
      if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        ImGui::InhibitInertialScroll();
        if (ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift)) {
          i.macro->rel=255;
        } else {
          i.macro->loop=255;
        }
      }
      ImGui::SetNextItemWidth(availableWidth);
      String& mmlStr=mmlString[index];
      if (ImGui::InputText("##IMacroMML",&mmlStr)) {
        decodeMMLStr(mmlStr,i.macro->val,i.macro->len,i.macro->loop,i.min,(i.isBitfield)?((1<<(i.isBitfield?(i.max):0))-1):i.max,i.macro->rel,i.bit30);
      }
      if (!ImGui::IsItemActive()) {
        encodeMMLStr(mmlStr,i.macro->val,i.macro->len,i.macro->loop,i.macro->rel,false,i.bit30);
      }
    }
    ImGui::PopStyleVar();
  } else {
    const auto adjustParam=[](int& value, int oldBot, int oldTop, int newBot, int newTop) {
      // Warning! It must be true that oldBot<=oldTop and newBot<=newTop.
      double oldAmp=fabs((double)oldTop-oldBot);
      double newAmp=fabs((double)newTop-newBot);
      if (oldAmp<1.0) oldAmp=1.0;
      double normalized=(double)(value-oldBot)/oldAmp;
      value=(normalized*newAmp)+newBot;
      value=CLAMP(value,newBot,newTop); // make sure it's in the range
    };

    const int actualMax=i.isBitfield?((1<<i.max)-1):i.max;
    if (i.macro->open&2) {
      const bool compact=(availableWidth<300.0f*dpiScale);
      bool adsrAdjust=false; // set to true if the range has been changed, so values can be readjusted
      if (ImGui::BeginTable("MacroADSR",compact?2:4)) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.3);
        if (!compact) {
          ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthFixed);
          ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthStretch,0.3);
        }
        //ImGui::TableSetupColumn("c4",ImGuiTableColumnFlags_WidthStretch,0.4);

        int oldBot=i.macro->val[0];
        int oldTop=i.macro->val[1];
        updateRangeInputs();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::Text(_("Bottom"));
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::InputInt("##MABottom",&insEditMacroEnvBottom,1,16)) {}
        if (ImGui::IsItemDeactivated()) { PARAMETER
          i.macro->val[0]=CLAMP(insEditMacroEnvBottom,i.min,actualMax);
          insEditMacroEnvTop=i.macro->val[0];
          adsrAdjust=true;
        }

        if (compact) ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::Text(_("Top"));
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::InputInt("##MATop",&insEditMacroEnvTop,1,16)) {}
        if (ImGui::IsItemDeactivated()) { PARAMETER
          i.macro->val[1]=CLAMP(insEditMacroEnvTop,i.min,actualMax);
          insEditMacroEnvTop=i.macro->val[1];
          adsrAdjust=true;
        }

        const int oldAdsrBottom=MIN(oldBot,oldTop);
        const int oldAdsrTop=MAX(oldBot,oldTop);
        const int oldAdsrRange=abs(oldAdsrTop-oldAdsrBottom);
        const int oldAdsrParamMax=(oldAdsrRange<<8)|0xff;

        const int adsrBottom=MIN(i.macro->val[0],i.macro->val[1]);
        const int adsrTop=MAX(i.macro->val[0],i.macro->val[1]);
        const int adsrRange=abs(adsrTop-adsrBottom);
        const int adsrParamMax=(adsrRange<<8)|0xff;

        // if the range has changed, we must adjust all parameters to make sure they're in range.
        if (adsrAdjust) {
          adjustParam(i.macro->val[2],0,oldAdsrParamMax,0,adsrParamMax); // attack
          adjustParam(i.macro->val[4],0,oldAdsrParamMax,0,adsrParamMax); // decay
          adjustParam(i.macro->val[7],0,oldAdsrParamMax,0,adsrParamMax); // sustain decay
          adjustParam(i.macro->val[8],0,oldAdsrParamMax,0,adsrParamMax); // release
          adjustParam(i.macro->val[5],oldAdsrBottom,oldAdsrTop,adsrBottom,adsrTop); // sustain level
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::Text(_("Attack"));
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (CWSliderInt("##MAAR",&i.macro->val[2],0,adsrParamMax,"%d",ImGuiSliderFlags_Logarithmic)) { PARAMETER
          if (i.macro->val[2]<0) i.macro->val[2]=0;
          if (i.macro->val[2]>adsrParamMax) i.macro->val[2]=adsrParamMax;
        } rightClickable
        if (ImGui::IsItemHovered()) {
          ADSR_LENGTH_HINT(i.macro->val[2],adsrRange);
        }

        if (compact) {
          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::AlignTextToFramePadding();
          ImGui::Text(_("Hold"));
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (CWSliderInt("##MAHT",&i.macro->val[3],0,255)) { PARAMETER
            if (i.macro->val[3]<0) i.macro->val[3]=0;
            if (i.macro->val[3]>255) i.macro->val[3]=255;
          } rightClickable
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(ngettext("%d tick","%d ticks",i.macro->val[3]),i.macro->val[3]);
          }

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::AlignTextToFramePadding();
          ImGui::Text(_("Decay"));
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (CWSliderInt("##MADR",&i.macro->val[4],0,adsrParamMax,"%d",ImGuiSliderFlags_Logarithmic)) { PARAMETER
            if (i.macro->val[4]<0) i.macro->val[4]=0;
            if (i.macro->val[4]>adsrParamMax) i.macro->val[4]=adsrParamMax;
          } rightClickable
          if (ImGui::IsItemHovered()) {
            ADSR_LENGTH_HINT(i.macro->val[4],adsrTop-i.macro->val[5]);
          }

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::AlignTextToFramePadding();
          ImGui::Text(_("Sustain"));
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (CWSliderInt("##MASL",&i.macro->val[5],adsrBottom,adsrTop)) { PARAMETER
            if (i.macro->val[5]<adsrBottom) i.macro->val[5]=adsrBottom;
            if (i.macro->val[5]>adsrTop) i.macro->val[5]=adsrTop;
          } rightClickable

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::AlignTextToFramePadding();
          ImGui::Text(_("SusTime"));
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (CWSliderInt("##MAST",&i.macro->val[6],0,255)) { PARAMETER
            if (i.macro->val[6]<0) i.macro->val[6]=0;
            if (i.macro->val[6]>255) i.macro->val[6]=255;
          } rightClickable
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(ngettext("%d tick","%d ticks",i.macro->val[6]),i.macro->val[6]);
          }

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::AlignTextToFramePadding();
          ImGui::Text(_("SusDecay"));
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (CWSliderInt("##MASR",&i.macro->val[7],0,adsrParamMax,"%d",ImGuiSliderFlags_Logarithmic)) { PARAMETER
            if (i.macro->val[7]<0) i.macro->val[7]=0;
            if (i.macro->val[7]>adsrParamMax) i.macro->val[7]=adsrParamMax;
          } rightClickable
          if (ImGui::IsItemHovered()) {
            ADSR_LENGTH_HINT(i.macro->val[7],i.macro->val[5]-adsrBottom);
          }

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::AlignTextToFramePadding();
          ImGui::Text(_("Release"));
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (CWSliderInt("##MARR",&i.macro->val[8],0,adsrParamMax,"%d",ImGuiSliderFlags_Logarithmic)) { PARAMETER
            if (i.macro->val[8]<0) i.macro->val[8]=0;
            if (i.macro->val[8]>adsrParamMax) i.macro->val[8]=adsrParamMax;
          } rightClickable
          if (ImGui::IsItemHovered()) {
            ADSR_LENGTH_HINT_REL(i.macro->val[8],i.macro->val[5]-adsrBottom);
          }
        } else {
          ImGui::TableNextColumn();
          ImGui::AlignTextToFramePadding();
          ImGui::Text(_("Sustain"));
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (CWSliderInt("##MASL",&i.macro->val[5],adsrBottom,adsrTop)) { PARAMETER
            if (i.macro->val[5]<adsrBottom) i.macro->val[5]=adsrBottom;
            if (i.macro->val[5]>adsrTop) i.macro->val[5]=adsrTop;
          } rightClickable

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::AlignTextToFramePadding();
          ImGui::Text(_("Hold"));
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (CWSliderInt("##MAHT",&i.macro->val[3],0,255)) { PARAMETER
            if (i.macro->val[3]<0) i.macro->val[3]=0;
            if (i.macro->val[3]>255) i.macro->val[3]=255;
          } rightClickable
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(ngettext("%d tick","%d ticks",i.macro->val[3]),i.macro->val[3]);
          }

          ImGui::TableNextColumn();
          ImGui::AlignTextToFramePadding();
          ImGui::Text(_("SusTime"));
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (CWSliderInt("##MAST",&i.macro->val[6],0,255)) { PARAMETER
            if (i.macro->val[6]<0) i.macro->val[6]=0;
            if (i.macro->val[6]>255) i.macro->val[6]=255;
          } rightClickable
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(ngettext("%d tick","%d ticks",i.macro->val[6]),i.macro->val[6]);
          }

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::AlignTextToFramePadding();
          ImGui::Text(_("Decay"));
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (CWSliderInt("##MADR",&i.macro->val[4],0,adsrParamMax,"%d",ImGuiSliderFlags_Logarithmic)) { PARAMETER
            if (i.macro->val[4]<0) i.macro->val[4]=0;
            if (i.macro->val[4]>adsrParamMax) i.macro->val[4]=adsrParamMax;
          } rightClickable
          if (ImGui::IsItemHovered()) {
            ADSR_LENGTH_HINT(i.macro->val[4],adsrTop-i.macro->val[5]);
          }

          ImGui::TableNextColumn();
          ImGui::AlignTextToFramePadding();
          ImGui::Text(_("SusDecay"));
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (CWSliderInt("##MASR",&i.macro->val[7],0,adsrParamMax,"%d",ImGuiSliderFlags_Logarithmic)) { PARAMETER
            if (i.macro->val[7]<0) i.macro->val[7]=0;
            if (i.macro->val[7]>adsrParamMax) i.macro->val[7]=adsrParamMax;
          } rightClickable
          if (ImGui::IsItemHovered()) {
            ADSR_LENGTH_HINT(i.macro->val[7],i.macro->val[5]-adsrBottom);
          }

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::TableNextColumn();

          ImGui::TableNextColumn();
          ImGui::AlignTextToFramePadding();
          ImGui::Text(_("Release"));
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (CWSliderInt("##MARR",&i.macro->val[8],0,adsrParamMax,"%d",ImGuiSliderFlags_Logarithmic)) { PARAMETER
            if (i.macro->val[8]<0) i.macro->val[8]=0;
            if (i.macro->val[8]>adsrParamMax) i.macro->val[8]=adsrParamMax;
          } rightClickable
          if (ImGui::IsItemHovered()) {
            ADSR_LENGTH_HINT_REL(i.macro->val[8],i.macro->val[5]-adsrBottom);
          }
        }

        ImGui::EndTable();
      }
    }
    if (i.macro->open&4) {
      const bool compact=(availableWidth<300.0f*dpiScale);
      bool lfoAdjust=false; // set to true if the range has been changed, so values can be readjusted
      if (ImGui::BeginTable("MacroLFO",compact?2:4)) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.3);
        if (!compact) {
          ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthFixed);
          ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthStretch,0.3);
        }

        const int oldLfoBottom=i.macro->val[0];
        const int oldLfoTop=i.macro->val[1];
        updateRangeInputs();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::Text(_("Bottom"));
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::InputInt("##MABottom",&insEditMacroEnvBottom,1,16)) {}
        if (ImGui::IsItemDeactivated()) { PARAMETER
          i.macro->val[0]=CLAMP(insEditMacroEnvTop,i.min,actualMax);
          insEditMacroEnvTop=i.macro->val[0];
          lfoAdjust=true;
        }

        if (compact) ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::Text(_("Top"));
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::InputInt("##MATop",&insEditMacroEnvTop,1,16)) {}
        if (ImGui::IsItemDeactivated()) { PARAMETER
          i.macro->val[1]=CLAMP(insEditMacroEnvTop,i.min,actualMax);
          insEditMacroEnvTop=i.macro->val[1];
          lfoAdjust=true;
        }

        const auto calcParamMax=[](int bottom, int top, int shape) {
          int range=abs(top-bottom);
          return (shape==2)?65536:((range<<8)|0xff);
        };

        const int oldLfoShape=i.macro->val[12];
        const int oldLfoParamMax=calcParamMax(oldLfoBottom,oldLfoTop,oldLfoShape);

        const int lfoBottom=i.macro->val[0];
        const int lfoTop=i.macro->val[1];
        const int lfoShape=i.macro->val[12];
        int lfoParamMax=calcParamMax(lfoBottom,lfoTop,lfoShape);

        if (lfoAdjust) {
          adjustParam(i.macro->val[11],0,oldLfoParamMax>>1,0,lfoParamMax>>1); // speed
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::Text(_("Speed"));
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (CWSliderInt("##MLSpeed",&i.macro->val[11],0,lfoParamMax>>1)) { PARAMETER
          if (i.macro->val[11]<0) i.macro->val[11]=0;
          if (i.macro->val[11]>(lfoParamMax>>1)) i.macro->val[11]=lfoParamMax>>1;
        } rightClickable
        if (ImGui::IsItemHovered()) {
          if (i.macro->val[11]<1) {
            ImGui::SetTooltip(_("halted"));
          } else {
            const int lHint=(lfoParamMax/i.macro->val[11])*((lfoShape==0)?2:1);
            ImGui::SetTooltip(ngettext("~%d tick","~%d ticks",lHint),lHint);
          }
        }

        if (compact) ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::Text(_("Phase"));
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (CWSliderInt("##MLPhase",&i.macro->val[13],0,1023)) { PARAMETER
          if (i.macro->val[13]<0) i.macro->val[13]=0;
          if (i.macro->val[13]>1023) i.macro->val[13]=1023;
        } rightClickable

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::Text(_("Shape"));
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (CWSliderInt("##MLShape",&i.macro->val[12],0,2,macroLFOShapes[i.macro->val[12]&3])) { PARAMETER
          if (i.macro->val[12]<0) i.macro->val[12]=0;
          if (i.macro->val[12]>2) i.macro->val[12]=2;

          // an adjust is requested here in case the user has changed the LFO shape (square uses an accumulator from 0 to 65535 and disregards the range)
          lfoAdjust=true;
          lfoParamMax=calcParamMax(lfoBottom,lfoTop,i.macro->val[12]);
        } rightClickable

        // adjust again with the potential new lfoParamMax
        if (lfoAdjust) {
          adjustParam(i.macro->val[11],0,oldLfoParamMax>>1,0,lfoParamMax>>1); // speed
        }

        ImGui::EndTable();
      }
    }
  }
}

#define BUTTON_TO_SET_MODE(buttonType) \
  if (buttonType(macroTypeLabels[(i.macro->open>>1)&3])) { \
    unsigned char prevOpen=i.macro->open; \
    if (i.macro->open>=4) { \
      i.macro->open&=(~6); \
    } else { \
      i.macro->open+=2; \
    } \
\
    /* check whether macro type is now ADSR/LFO or sequence */ \
    if (((prevOpen&6)?1:0)!=((i.macro->open&6)?1:0)) { \
      /* swap memory */ \
      /* this way the macro isn't corrupted if the user decides to go */ \
      /* back to sequence mode */ \
      i.macro->len^=i.ins->temp.lenMemory[i.macro->macroType]; \
      i.ins->temp.lenMemory[i.macro->macroType]^=i.macro->len; \
      i.macro->len^=i.ins->temp.lenMemory[i.macro->macroType]; \
\
      for (int j=0; j<16; j++) { \
        i.macro->val[j]^=i.ins->temp.typeMemory[i.macro->macroType][j]; \
        i.ins->temp.typeMemory[i.macro->macroType][j]^=i.macro->val[j]; \
        i.macro->val[j]^=i.ins->temp.typeMemory[i.macro->macroType][j]; \
      } \
\
      /* if ADSR/LFO, populate min/max */ \
      if (i.macro->open&6) { \
        const int actualMax=i.isBitfield?((1<<i.max)-1):i.max; \
        if (i.macro->val[0]==0 && i.macro->val[1]==0) { \
          i.macro->val[0]=i.min; \
          i.macro->val[1]=actualMax; \
        } \
        i.macro->val[0]=CLAMP(i.macro->val[0],i.min,actualMax); \
        i.macro->val[1]=CLAMP(i.macro->val[1],i.min,actualMax); \
      } \
    } \
    PARAMETER; \
  } \
  if (ImGui::IsItemHovered()) { \
    switch (i.macro->open&6) { \
      case 0: \
        ImGui::SetTooltip(_("Macro type: Sequence")); \
        break; \
      case 2: \
        ImGui::SetTooltip(_("Macro type: ADSR")); \
        break; \
      case 4: \
        ImGui::SetTooltip(_("Macro type: LFO")); \
        break; \
      default: \
        ImGui::SetTooltip(_("Macro type: What's going on here?")); \
        break; \
    } \
  } \
  if (i.macro->open&6) { \
    i.macro->len=16; \
  }

#define BUTTON_TO_SET_PROPS(_x) \
  pushToggleColors(_x.macro->speed!=1 || _x.macro->delay); \
  ImGui::Button(ICON_FA_ELLIPSIS_H "##IMacroSet"); \
  popToggleColors(); \
  if (ImGui::IsItemHovered()) { \
    ImGui::SetTooltip(_("Delay/Step Length")); \
  } \
  if (ImGui::BeginPopupContextItem("IMacroSetP",ImGuiPopupFlags_MouseButtonLeft)) { \
    if (ImGui::InputScalar(_("Step Length (ticks)##IMacroSpeed"),ImGuiDataType_U8,&_x.macro->speed,&_ONE,&_THREE)) { \
      if (_x.macro->speed<1) _x.macro->speed=1; \
      MARK_MODIFIED; \
    } \
    if (ImGui::InputScalar(_("Delay##IMacroDelay"),ImGuiDataType_U8,&_x.macro->delay,&_ONE,&_THREE)) { \
      MARK_MODIFIED; \
    } \
    ImGui::EndPopup(); \
  }

#define BUTTON_TO_SET_RELEASE(buttonType) \
  pushToggleColors(i.macro->open&8); \
  if (buttonType(ICON_FA_BOLT "##IMacroRelMode")) { \
    i.macro->open^=8; \
  } \
  if (ImGui::IsItemHovered()) { \
    if (i.macro->open&8) { \
      ImGui::SetTooltip(_("Release mode: Active (jump to release pos)")); \
    } else { \
      ImGui::SetTooltip(_("Release mode: Passive (delayed release)")); \
    } \
  } \
  popToggleColors(); \

#define BUTTON_FOR_MACRO_MENU(buttonType) \
  if (mobileUI) { \
    if (buttonType(ICON_FA_PAGELINES "##IMacroMenu")) { \
      lastMacroDesc=i; \
      displayMacroMenu=true; \
    } \
  }

void FurnaceGUI::drawMacros(std::vector<FurnaceGUIMacroDesc>& macros, FurnaceGUIMacroEditState& state, DivInstrument* ins) {
  int index=0;
  int maxMacroLen=0;
  float reservedSpace=(settings.oldMacroVSlider)?(20.0f*dpiScale+ImGui::GetStyle().ItemSpacing.x):ImGui::GetStyle().ScrollbarSize;

  for (FurnaceGUIMacroDesc& m: macros) {
    m.ins=ins;
    if (m.macro->len>maxMacroLen) maxMacroLen=m.macro->len;
  }

  switch (settings.macroLayout) {
    case 0: {
      if (ImGui::BeginTable("MacroSpace",2)) {
        float precalcWidth=0.0f;
        for (FurnaceGUIMacroDesc& i: macros) {
          float next=ImGui::CalcTextSize(i.displayName).x+ImGui::GetStyle().ItemInnerSpacing.x*2.0f+ImGui::CalcTextSize(ICON_FA_CHEVRON_UP).x+ImGui::GetStyle().ItemSpacing.x*2.0f;
          if (next>precalcWidth) precalcWidth=next;
        }
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed,MAX(120.0f*dpiScale,precalcWidth));
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.0);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        float lenAvail=ImGui::GetContentRegionAvail().x;
        //ImGui::Dummy(ImVec2(120.0f*dpiScale,dpiScale));
        if (settings.autoMacroStepSize==0) {
          ImGui::SetNextItemWidth(120.0f*dpiScale);
          if (ImGui::InputInt("##MacroPointSize",&macroPointSize,1,4)) {
            if (macroPointSize<1) macroPointSize=1;
            if (macroPointSize>256) macroPointSize=256;
          }
        }
        ImGui::TableNextColumn();
        float availableWidth=ImGui::GetContentRegionAvail().x-reservedSpace;
        int totalFit=MIN(255,availableWidth/MAX(1,macroPointSize*dpiScale));
        int scrollMax=0;
        if (settings.autoMacroStepSize!=0) totalFit=1;
        for (FurnaceGUIMacroDesc& i: macros) {
          if (i.macro->len>scrollMax) scrollMax=i.macro->len;
          if (settings.autoMacroStepSize==1) {
            if ((i.macro->open&6)==0 && totalFit<i.macro->len) totalFit=i.macro->len;
          } else if (settings.autoMacroStepSize==2) {
            if ((i.macro->open&6)==0 && totalFit<maxMacroLen) totalFit=maxMacroLen;
          }
        }
        scrollMax-=totalFit;
        if (scrollMax<0) scrollMax=0;
        if (macroDragScroll>scrollMax) {
          macroDragScroll=scrollMax;
        }
        ImGui::BeginDisabled(scrollMax<1);
        ImGui::SetNextItemWidth(availableWidth);
        if (CWSliderInt("##MacroScrollTop",&macroDragScroll,0,scrollMax,"")) {
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
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("index: %.2X"),i.macro->macroType);
          }
          ImGui::SameLine();
          if (ImGui::SmallButton((i.macro->open&1)?(ICON_FA_CHEVRON_UP "##IMacroOpen"):(ICON_FA_CHEVRON_DOWN "##IMacroOpen"))) {
            i.macro->open^=1;
          }
          if (i.macro->open&1) {
            if ((i.macro->open&6)==0) {
              ImGui::SetNextItemWidth(lenAvail);
              int macroLen=i.macro->len;
              if (ImGui::InputScalar("##IMacroLen",ImGuiDataType_U8,&macroLen,&_ONE,&_THREE)) { MARK_MODIFIED
                if (macroLen<0) macroLen=0;
                if (macroLen>255) macroLen=255;
                i.macro->len=macroLen;
              }
            }
            BUTTON_TO_SET_MODE(ImGui::Button);
            ImGui::SameLine();
            BUTTON_TO_SET_PROPS(i);
            if ((i.macro->open&6)==0) {
              ImGui::SameLine();
              BUTTON_TO_SET_RELEASE(ImGui::Button);
            }
            BUTTON_FOR_MACRO_MENU(ImGui::Button);
            // do not change this!
            // anything other than a checkbox will look ugly!
            // if you really need more than two macro modes please tell me.
            if (i.modeName!=NULL) {
              bool modeVal=i.macro->mode;
              String modeName=fmt::sprintf("%s##IMacroMode",i.modeName);
              if (ImGui::Checkbox(modeName.c_str(),&modeVal)) {
                i.macro->mode=modeVal;
                i.ins->temp.vZoom[i.macro->macroType]=-1;
                i.ins->temp.vScroll[i.macro->macroType]=-1;
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
        if (CWSliderInt("##MacroScrollBottom",&macroDragScroll,0,scrollMax,"")) {
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
          int totalFit=i.macro->len;
          if (totalFit<1) totalFit=1;

          ImGui::PushID(index);

          ImGui::TextUnformatted(i.displayName);
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("index: %.2X"),i.macro->macroType);
          }
          ImGui::SameLine();
          if (ImGui::SmallButton((i.macro->open&1)?(ICON_FA_CHEVRON_UP "##IMacroOpen"):(ICON_FA_CHEVRON_DOWN "##IMacroOpen"))) {
            i.macro->open^=1;
          }

          if (i.macro->open&1) {
            ImGui::SameLine();
            BUTTON_TO_SET_MODE(ImGui::SmallButton);
          }

          drawMacroEdit(i,totalFit,availableWidth,index);

          if (i.macro->open&1) {
            if ((i.macro->open&6)==0) {
              ImGui::Text(_("Length"));
              ImGui::SameLine();
              ImGui::SetNextItemWidth(120.0f*dpiScale);
              int macroLen=i.macro->len;
              if (ImGui::InputScalar("##IMacroLen",ImGuiDataType_U8,&macroLen,&_ONE,&_THREE)) { MARK_MODIFIED
                if (macroLen<0) macroLen=0;
                if (macroLen>255) macroLen=255;
                i.macro->len=macroLen;
              }
              ImGui::SameLine();
            }
            BUTTON_TO_SET_PROPS(i);
            if ((i.macro->open&6)==0) {
              ImGui::SameLine();
              BUTTON_TO_SET_RELEASE(ImGui::Button);
            }
            BUTTON_FOR_MACRO_MENU(ImGui::Button);
            if (i.modeName!=NULL) {
              bool modeVal=i.macro->mode;
              String modeName=fmt::sprintf("%s##IMacroMode",i.modeName);
              ImGui::SameLine();
              if (ImGui::Checkbox(modeName.c_str(),&modeVal)) {
                i.macro->mode=modeVal;
                i.ins->temp.vZoom[i.macro->macroType]=-1;
                i.ins->temp.vScroll[i.macro->macroType]=-1;
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
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.2f);
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.8f);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        for (size_t i=0; i<macros.size(); i++) {
          // include macro len if non-zero, making particularly clear at-a-glance which macros
          // have non-zero len (i.e. are active). and calculate how big we need to be to leave some
          // extra space so the column doesn't change size when len is changed under typical
          // circumstances (really don't want to move buttons while mouse is being clicked or held).
          char buf[256];

          if (macros[i].macro->len>0) {
            snprintf(buf,255,"%s [%d]###%s_%d",macros[i].displayName,macros[i].macro->len,macros[i].displayName,(int)i);
          } else {
            snprintf(buf,255,"%s###%s_%d",macros[i].displayName,macros[i].displayName,(int)i);
          }

          if (ImGui::Selectable(buf,state.selectedMacro==(int)i)) {
            state.selectedMacro=i;
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("index: %.2X"),macros[i].macro->macroType);
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
          m.macro->open|=1;

          float availableWidth=ImGui::GetContentRegionAvail().x-reservedSpace;
          int totalFit=MIN(255,availableWidth/MAX(1,macroPointSize*dpiScale));
          int scrollMax=0;
          for (FurnaceGUIMacroDesc& i: macros) {
            if (i.macro->len>scrollMax) scrollMax=i.macro->len;
          }
          if (settings.autoMacroStepSize==1) totalFit=MAX(1,m.macro->len);
          else if (settings.autoMacroStepSize==2) totalFit=MAX(1,maxMacroLen);
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

          if (settings.autoMacroStepSize==0) {
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
          }

          m.height=ImGui::GetContentRegionAvail().y-ImGui::GetFontSize()-ImGui::GetFrameHeightWithSpacing()-(m.bit30?28.0f:12.0f)*dpiScale-ImGui::GetStyle().ItemSpacing.y*3.0f;
          if (m.height<10.0f*dpiScale) m.height=10.0f*dpiScale;
          m.height/=dpiScale;
          drawMacroEdit(m,totalFit,availableWidth,index);

          if (m.macro->open&1) {
            bool showLen=((m.macro->open&6)==0);
            int colCount=showLen ? 4 : 3;
            float availX=ImGui::GetContentRegionAvail().x;

            // fairly arbitrary scaling logic
            bool shortLabels=(availX<600.0f*dpiScale);
            float scalarItemWidth=MIN((availX-90.0f*dpiScale)/colCount, 120.0f*dpiScale);
            if (ImGui::BeginTable("##MacroMetaData",colCount)) {
              if (showLen) ImGui::TableSetupColumn("len",ImGuiTableColumnFlags_WidthStretch,0.0);
              ImGui::TableSetupColumn("stepLen",ImGuiTableColumnFlags_WidthStretch,0.0);
              ImGui::TableSetupColumn("delay",ImGuiTableColumnFlags_WidthStretch,0.0);
              ImGui::TableSetupColumn("buttons",ImGuiTableColumnFlags_WidthFixed,0.0);

              ImGui::TableNextRow();
              if (showLen) {
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text(shortLabels ? _("Len##macroEditLengthShortLabel") : _("Length"));
                if (shortLabels && ImGui::IsItemHovered()) ImGui::SetTooltip("%s", _("Length"));
                ImGui::SameLine();
                ImGui::SetNextItemWidth(scalarItemWidth);
                int macroLen=m.macro->len;
                if (ImGui::InputScalar("##IMacroLen",ImGuiDataType_U8,&macroLen,&_ONE,&_THREE)) { MARK_MODIFIED
                  if (macroLen<0) macroLen=0;
                  if (macroLen>255) macroLen=255;
                  m.macro->len=macroLen;
                }
              }
              ImGui::TableNextColumn();
              ImGui::AlignTextToFramePadding();
              ImGui::Text(shortLabels ? _("SLen##macroEditStepLenShortLabel") : _("StepLen"));
              if (shortLabels && ImGui::IsItemHovered()) ImGui::SetTooltip("%s", _("StepLen"));
              ImGui::SameLine();
              ImGui::SetNextItemWidth(scalarItemWidth);
              if (ImGui::InputScalar("##IMacroSpeed",ImGuiDataType_U8,&m.macro->speed,&_ONE,&_THREE)) {
                if (m.macro->speed<1) m.macro->speed=1;
                MARK_MODIFIED;
              }
              ImGui::TableNextColumn();
              ImGui::AlignTextToFramePadding();
              ImGui::Text(shortLabels ? _("Del##macroEditDelayShortLabel") : _("Delay"));
              if (shortLabels && ImGui::IsItemHovered()) ImGui::SetTooltip("%s", _("Delay"));
              ImGui::SameLine();
              ImGui::SetNextItemWidth(scalarItemWidth);
              if (ImGui::InputScalar("##IMacroDelay",ImGuiDataType_U8,&m.macro->delay,&_ONE,&_THREE)) {
                MARK_MODIFIED;
              }
              ImGui::TableNextColumn();
              {
                FurnaceGUIMacroDesc& i=m;
                BUTTON_TO_SET_MODE(ImGui::Button);
                if ((i.macro->open&6)==0) {
                  ImGui::SameLine();
                  BUTTON_TO_SET_RELEASE(ImGui::Button);
                }
                BUTTON_FOR_MACRO_MENU(ImGui::Button);
              }
              if (m.modeName!=NULL) {
                bool modeVal=m.macro->mode;
                String modeName=fmt::sprintf("%s##IMacroMode",m.modeName);
                ImGui::SameLine();
                if (ImGui::Checkbox(modeName.c_str(),&modeVal)) {
                  m.macro->mode=modeVal;
                  m.ins->temp.vZoom[m.macro->macroType]=-1;
                  m.ins->temp.vScroll[m.macro->macroType]=-1;
                }
              }
              ImGui::EndTable();
            }
          } else {
            ImGui::Text(_("The heck? No, this isn't even working correctly..."));
          }
        } else {
          ImGui::Text(_("The only problem with that selectedMacro is that it's a bug..."));
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

void FurnaceGUI::handleMacroMenu(DivInstrument* ins) {
  if (displayMacroMenu) {
    displayMacroMenu=false;
    if (lastMacroDesc.macro!=NULL) {
      ImGui::OpenPopup("macroMenu");
    }
  }
  if (ImGui::BeginPopup("macroMenu",ImGuiWindowFlags_NoMove|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoSavedSettings)) {
    if (ImGui::MenuItem(_("copy"))) {
      String mmlStr;
      encodeMMLStr(mmlStr,lastMacroDesc.macro->val,lastMacroDesc.macro->len,lastMacroDesc.macro->loop,lastMacroDesc.macro->rel);
      SDL_SetClipboardText(mmlStr.c_str());
    }
    if (ImGui::MenuItem(_("paste"))) {
      String mmlStr;
      char* clipText=SDL_GetClipboardText();
      if (clipText!=NULL) {
        if (clipText[0]) {
          mmlStr=clipText;
        }
        SDL_free(clipText);
      }
      if (!mmlStr.empty()) {
        decodeMMLStr(mmlStr,lastMacroDesc.macro->val,lastMacroDesc.macro->len,lastMacroDesc.macro->loop,lastMacroDesc.min,(lastMacroDesc.isBitfield)?((1<<(lastMacroDesc.isBitfield?lastMacroDesc.max:0))-1):lastMacroDesc.max,lastMacroDesc.macro->rel);
      }
    }
    ImGui::Separator();
    if (ImGui::MenuItem(_("clear"))) {
      lastMacroDesc.macro->len=0;
      lastMacroDesc.macro->loop=255;
      lastMacroDesc.macro->rel=255;
      for (int i=0; i<256; i++) {
        lastMacroDesc.macro->val[i]=0;
      }
    }
    if (ImGui::MenuItem(_("clear contents"))) {
      for (int i=0; i<256; i++) {
        lastMacroDesc.macro->val[i]=0;
      }
    }
    ImGui::Separator();
    if (ImGui::BeginMenu(_("offset..."))) {
      ImGui::InputInt(_("X"),&macroOffX,1,10);
      ImGui::InputInt(_("Y"),&macroOffY,1,10);
      if (ImGui::Button(_("offset"))) {
        int oldData[256];
        memset(oldData,0,256*sizeof(int));
        memcpy(oldData,lastMacroDesc.macro->val,lastMacroDesc.macro->len*sizeof(int));

        for (int i=0; i<lastMacroDesc.macro->len; i++) {
          int val=0;
          bool bit30=false;
          if ((i-macroOffX)>=0 && (i-macroOffX)<lastMacroDesc.macro->len) {
            bit30=enBit30(oldData[i-macroOffX]);
            val=deBit30(oldData[i-macroOffX])+macroOffY;
            if (val<lastMacroDesc.min) val=lastMacroDesc.min;
            if (val>lastMacroDesc.max) val=lastMacroDesc.max;
          }
          lastMacroDesc.macro->val[i]=val^(bit30?0x40000000:0);
        }

        if (lastMacroDesc.macro->loop<lastMacroDesc.macro->len) {
          lastMacroDesc.macro->loop+=macroOffX;
        } else {
          lastMacroDesc.macro->loop=255;
        }
        if ((lastMacroDesc.macro->rel+macroOffX)>=0 && (lastMacroDesc.macro->rel+macroOffX)<lastMacroDesc.macro->len) {
          lastMacroDesc.macro->rel+=macroOffX;
        } else {
          lastMacroDesc.macro->rel=255;
        }

        ImGui::CloseCurrentPopup();
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu(_("scale..."))) {
      if (ImGui::InputFloat(_("X"),&macroScaleX,1.0f,10.0f,"%.2f%%")) {
        if (macroScaleX<0.1) macroScaleX=0.1;
        if (macroScaleX>12800.0) macroScaleX=12800.0;
      }
      ImGui::InputFloat(_("Y"),&macroScaleY,1.0f,10.0f,"%.2f%%");
      if (ImGui::Button(_("scale"))) {
        int oldData[256];
        memset(oldData,0,256*sizeof(int));
        memcpy(oldData,lastMacroDesc.macro->val,lastMacroDesc.macro->len*sizeof(int));

        unsigned char oldLen=lastMacroDesc.macro->len;
        lastMacroDesc.macro->len=MIN(255,((double)lastMacroDesc.macro->len*(macroScaleX/100.0)));

        for (int i=0; i<lastMacroDesc.macro->len; i++) {
          int val=0;
          bool bit30=false;
          double posX=round((double)i*(100.0/macroScaleX)-0.01);
          if (posX>=0 && posX<oldLen) {
            val=round((double)deBit30(oldData[(int)posX])*(macroScaleY/100.0));
            bit30=enBit30(oldData[(int)posX]);
            if (val<lastMacroDesc.min) val=lastMacroDesc.min;
            if (val>lastMacroDesc.max) val=lastMacroDesc.max;
          }
          lastMacroDesc.macro->val[i]=val^(bit30?0x40000000:0);
        }

        ImGui::CloseCurrentPopup();
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu(_("randomize..."))) {
      if (macroRandMin<lastMacroDesc.min) macroRandMin=lastMacroDesc.min;
      if (macroRandMin>lastMacroDesc.max) macroRandMin=lastMacroDesc.max;
      if (macroRandMax<lastMacroDesc.min) macroRandMax=lastMacroDesc.min;
      if (macroRandMax>lastMacroDesc.max) macroRandMax=lastMacroDesc.max;
      ImGui::InputInt(_("Min"),&macroRandMin,1,10);
      ImGui::InputInt(_("Max"),&macroRandMax,1,10);
      if (ImGui::Button(_("randomize"))) {
        for (int i=0; i<lastMacroDesc.macro->len; i++) {
          int val=0;
          if (macroRandMax<=macroRandMin) {
            val=macroRandMin;
          } else {
            val=macroRandMin+(rand()%(macroRandMax-macroRandMin+1));
          }
          lastMacroDesc.macro->val[i]=val;
        }

        ImGui::CloseCurrentPopup();
      }
      ImGui::EndMenu();
    }
    
    ImGui::EndPopup();
  }
}
