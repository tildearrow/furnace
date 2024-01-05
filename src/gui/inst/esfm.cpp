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

#include "fmEnvUtil.h"
#include "stringsUtil.h"
#include "macroDraw.h"
#include "fmPublicVars.h"
#include "publicVars.h"
#include "fm.h"
#include "../intConst.h"

extern "C" {
#include "../../extern/Nuked-OPLL/opll.h"
}

class FurnaceGUI;

void FurnaceGUI::drawInsESFM(DivInstrument* ins)
{
  opCount=4;
  opsAreMutable=false;

  if (ImGui::BeginTabItem("FM")) 
  {
    DivInstrumentFM& fmOrigin=ins->fm;

    isPresentCount=0;
    memset(isPresent,0,4*sizeof(bool));

    if (!isPresent[0] && !isPresent[1] && !isPresent[2] && !isPresent[3]) 
    {
      isPresent[0]=true;
    }
    for (int i=0; i<4; i++) 
    {
      if (isPresent[i]) isPresentCount++;
    }
    presentWhich=0;
    for (int i=0; i<4; i++) 
    {
      if (isPresent[i]) 
      {
        presentWhich=i;
        break;
      }
    }

    if (ImGui::BeginTable("fmDetails",3,ImGuiTableFlags_SizingStretchProp)) 
    {
      ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.50f);
      ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.15f);
      ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.35f);

      ImGui::TableNextRow();

      ImGui::TableNextColumn();
      P(CWSliderScalar(ESFM_LONG_NAME(ESFM_NOISE),ImGuiDataType_U8,&ins->esfm.noise,&_ZERO,&_THREE,esfmNoiseModeNames[ins->esfm.noise&3])); rightClickable
      ImGui::TextUnformatted(esfmNoiseModeDescriptions[ins->esfm.noise&3]);
      ImGui::TableNextColumn();
      ImGui::TableNextColumn();
      if (fmPreviewOn) 
      {
        drawFMPreview(ImVec2(ImGui::GetContentRegionAvail().x,48.0*dpiScale));
        if (!fmPreviewPaused) 
        {
          renderFMPreview(ins,1);
          WAKE_UP;
        }
      } 
      else 
      {
        drawESFMAlgorithm(ins->esfm, ImVec2(ImGui::GetContentRegionAvail().x,48.0*dpiScale));
      }
      kvsConfig(ins);
      ImGui::EndTable();
    }

    bool willDisplayOps=true;

    ImGui::BeginDisabled(!willDisplayOps);
    if (settings.fmLayout==0) 
    {
      int numCols=19;
      if (ImGui::BeginTable("FMOperators",numCols,ImGuiTableFlags_SizingStretchProp|ImGuiTableFlags_BordersH|ImGuiTableFlags_BordersOuterV)) 
      {
        // configure columns
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed); // op name
        ImGui::TableSetupColumn("c0e0",ImGuiTableColumnFlags_WidthStretch,0.05f); // outLvl
        ImGui::TableSetupColumn("c0e1",ImGuiTableColumnFlags_WidthFixed); // -separator-
        ImGui::TableSetupColumn("c0e2",ImGuiTableColumnFlags_WidthStretch,0.05f); // delay
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.05f); // ar
        ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.05f); // dr
        ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthStretch,0.05f); // sl
        ImGui::TableSetupColumn("c5",ImGuiTableColumnFlags_WidthStretch,0.05f); // rr
        ImGui::TableSetupColumn("c6",ImGuiTableColumnFlags_WidthFixed); // -separator-
        ImGui::TableSetupColumn("c7",ImGuiTableColumnFlags_WidthStretch,0.05f); // tl
        ImGui::TableSetupColumn("c8",ImGuiTableColumnFlags_WidthStretch,0.05f); // rs/ksl
        ImGui::TableSetupColumn("c8e0",ImGuiTableColumnFlags_WidthStretch,0.05f); // outLvl
        ImGui::TableSetupColumn("c9",ImGuiTableColumnFlags_WidthStretch,0.05f); // mult
        ImGui::TableSetupColumn("c9e",ImGuiTableColumnFlags_WidthStretch,0.05f); // ct
        ImGui::TableSetupColumn("c10",ImGuiTableColumnFlags_WidthStretch,0.05f); // dt
        ImGui::TableSetupColumn("c15",ImGuiTableColumnFlags_WidthFixed); // am
        ImGui::TableSetupColumn("c12",ImGuiTableColumnFlags_WidthFixed); // -separator-
        ImGui::TableSetupColumn("c13",ImGuiTableColumnFlags_WidthStretch,0.2f); // ssg/waveform
        ImGui::TableSetupColumn("c14",ImGuiTableColumnFlags_WidthStretch,0.3f); // env

        // header
        ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        CENTER_TEXT(ESFM_SHORT_NAME(ESFM_MODIN));
        ImGui::TextUnformatted(ESFM_SHORT_NAME(ESFM_MODIN));
        TOOLTIP_TEXT(ESFM_LONG_NAME(ESFM_MODIN));
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        CENTER_TEXT(ESFM_SHORT_NAME(ESFM_DELAY));
        ImGui::TextUnformatted(ESFM_SHORT_NAME(ESFM_DELAY));
        TOOLTIP_TEXT(ESFM_LONG_NAME(ESFM_DELAY));
        ImGui::TableNextColumn();
        CENTER_TEXT(FM_SHORT_NAME(FM_AR));
        ImGui::TextUnformatted(FM_SHORT_NAME(FM_AR));
        TOOLTIP_TEXT(FM_NAME(FM_AR));
        ImGui::TableNextColumn();
        CENTER_TEXT(FM_SHORT_NAME(FM_DR));
        ImGui::TextUnformatted(FM_SHORT_NAME(FM_DR));
        TOOLTIP_TEXT(FM_NAME(FM_DR));
        if (settings.susPosition==0) 
        {
          ImGui::TableNextColumn();
          CENTER_TEXT(FM_SHORT_NAME(FM_SL));
          ImGui::TextUnformatted(FM_SHORT_NAME(FM_SL));
          TOOLTIP_TEXT(FM_NAME(FM_SL));
        }
        ImGui::TableNextColumn();
        CENTER_TEXT(FM_SHORT_NAME(FM_RR));
        ImGui::TextUnformatted(FM_SHORT_NAME(FM_RR));
        TOOLTIP_TEXT(FM_NAME(FM_RR));
        if (settings.susPosition==1) 
        {
          ImGui::TableNextColumn();
          CENTER_TEXT(FM_SHORT_NAME(FM_SL));
          ImGui::TextUnformatted(FM_SHORT_NAME(FM_SL));
          TOOLTIP_TEXT(FM_NAME(FM_SL));
        }
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        CENTER_TEXT(FM_SHORT_NAME(FM_TL));
        ImGui::TextUnformatted(FM_SHORT_NAME(FM_TL));
        TOOLTIP_TEXT(FM_NAME(FM_TL));
        ImGui::TableNextColumn();
        CENTER_TEXT(FM_SHORT_NAME(FM_KSL));
        ImGui::TextUnformatted(FM_SHORT_NAME(FM_KSL));
        TOOLTIP_TEXT(FM_NAME(FM_KSL));
        ImGui::TableNextColumn();
        CENTER_TEXT(ESFM_SHORT_NAME(ESFM_OUTLVL));
        ImGui::TextUnformatted(ESFM_SHORT_NAME(ESFM_OUTLVL));
        TOOLTIP_TEXT(ESFM_LONG_NAME(ESFM_OUTLVL));
        ImGui::TableNextColumn();
        CENTER_TEXT(FM_SHORT_NAME(FM_MULT));
        ImGui::TextUnformatted(FM_SHORT_NAME(FM_MULT));
        TOOLTIP_TEXT(FM_NAME(FM_MULT));
        ImGui::TableNextColumn();
        CENTER_TEXT(ESFM_SHORT_NAME(ESFM_CT));
        ImGui::TextUnformatted(ESFM_SHORT_NAME(ESFM_CT));
        TOOLTIP_TEXT(ESFM_LONG_NAME(ESFM_CT));
        ImGui::TableNextColumn();
        CENTER_TEXT(ESFM_SHORT_NAME(ESFM_DT));
        ImGui::TextUnformatted(ESFM_SHORT_NAME(ESFM_DT));
        TOOLTIP_TEXT(ESFM_LONG_NAME(ESFM_DT));
        ImGui::TableNextColumn();
        CENTER_TEXT("Other");
        ImGui::TextUnformatted("Other");
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        CENTER_TEXT(FM_NAME(FM_WS));
        ImGui::TextUnformatted(FM_NAME(FM_WS));
        ImGui::TableNextColumn();
        CENTER_TEXT("Envelope");
        ImGui::TextUnformatted("Envelope");

        float sliderHeight=32.0f*dpiScale;

        for (int i=0; i<opCount; i++) 
        {
          DivInstrumentFM::Operator& op=fmOrigin.op[i];
          DivInstrumentESFM::Operator& opE=ins->esfm.op[i];

          ImGui::TableNextRow();
          ImGui::TableNextColumn();

          // push colors
          if (settings.separateFMColors) 
          {
            bool mod=true;

            // this is the same as the KVS heuristic in platform/esfm.h
            if (opE.outLvl==7) mod=false;
            else if (opE.outLvl>0) 
            {
              if (i==3) mod=false;
              else 
              {
                DivInstrumentESFM::Operator& opENext=ins->esfm.op[i+1];
                if (opENext.modIn==0) mod=false;
                else if ((opE.outLvl-opENext.modIn)>=2) mod=false;
              }
            }

            if (mod) 
            {
              pushAccentColors(
                uiColors[GUI_COLOR_FM_PRIMARY_MOD],
                uiColors[GUI_COLOR_FM_SECONDARY_MOD],
                uiColors[GUI_COLOR_FM_BORDER_MOD],
                uiColors[GUI_COLOR_FM_BORDER_SHADOW_MOD]
              );
            } 
            else 
            {
              pushAccentColors(
                uiColors[GUI_COLOR_FM_PRIMARY_CAR],
                uiColors[GUI_COLOR_FM_SECONDARY_CAR],
                uiColors[GUI_COLOR_FM_BORDER_CAR],
                uiColors[GUI_COLOR_FM_BORDER_SHADOW_CAR]
              );
            }
          }

          if (i==0) 
          {
            sliderHeight=(ImGui::GetContentRegionAvail().y/opCount)-ImGui::GetStyle().ItemSpacing.y;
            float sliderMinHeightESFM=ImGui::GetFrameHeight()*5.0+ImGui::GetStyle().ItemSpacing.y*4.0;
            sliderHeight=sliderMinHeightESFM;
          }

          ImGui::PushID(fmt::sprintf("op%d",i).c_str());
          String opNameLabel;
          opNameLabel=fmt::sprintf("OP%d",i+1);
          ImGui::TextUnformatted(opNameLabel.c_str());
          // drag point
          OP_DRAG_POINT;

          int maxTl=63;
          int maxArDr=15;
          bool ksrOn=op.ksr;
          bool vibOn=op.vib;
          bool susOn=op.sus;
          bool fixedOn=opE.fixed;

          ImGui::TableNextColumn();
          CENTER_VSLIDER;
          P(CWVSliderScalar("##MODIN",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&opE.modIn,&_ZERO,&_SEVEN)); rightClickable
          ImGui::TableNextColumn();
          ImGui::Dummy(ImVec2(4.0f*dpiScale,2.0f*dpiScale));
          ImGui::TableNextColumn();
          opE.delay&=7;
          CENTER_VSLIDER;
          P(CWVSliderScalar("##DELAY",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&opE.delay,&_ZERO,&_SEVEN)); rightClickable

          ImGui::TableNextColumn();
          op.ar&=maxArDr;
          CENTER_VSLIDER;
          P(CWVSliderScalar("##AR",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.ar,&maxArDr,&_ZERO)); rightClickable

          ImGui::TableNextColumn();
          op.dr&=maxArDr;
          CENTER_VSLIDER;
          P(CWVSliderScalar("##DR",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.dr,&maxArDr,&_ZERO)); rightClickable

          if (settings.susPosition==0)
          {
            ImGui::TableNextColumn();
            op.sl&=15;
            CENTER_VSLIDER;
            P(CWVSliderScalar("##SL",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.sl,&_FIFTEEN,&_ZERO)); rightClickable
          }

          ImGui::TableNextColumn();
          op.rr&=15;
          CENTER_VSLIDER;
          P(CWVSliderScalar("##RR",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.rr,&_FIFTEEN,&_ZERO)); rightClickable

          if (settings.susPosition==1) 
          {
            ImGui::TableNextColumn();
            op.sl&=15;
            CENTER_VSLIDER;
            P(CWVSliderScalar("##SL",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.sl,&_FIFTEEN,&_ZERO)); rightClickable
          }

          ImGui::TableNextColumn();
          ImGui::Dummy(ImVec2(4.0f*dpiScale,2.0f*dpiScale));

          ImGui::TableNextColumn();
          op.tl&=maxTl;
          CENTER_VSLIDER;
          P(CWVSliderScalar("##TL",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.tl,&maxTl,&_ZERO)); rightClickable

          ImGui::TableNextColumn();
          CENTER_VSLIDER;
          
          int ksl=ins->type==DIV_INS_OPLL?op.ksl:kslMap[op.ksl&3];
          if (CWVSliderInt("##KSL",ImVec2(20.0f*dpiScale,sliderHeight),&ksl,0,3)) 
          {
            op.ksl=(ins->type==DIV_INS_OPLL?ksl:kslMap[ksl&3]);
            PARAMETER;
          } rightClickable

          ImGui::TableNextColumn();
          CENTER_VSLIDER;
          P(CWVSliderScalar("##OUTLVL",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&opE.outLvl,&_ZERO,&_SEVEN)); rightClickable

          ImGui::TableNextColumn();
          CENTER_VSLIDER;
          P(CWVSliderScalar("##MULT",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.mult,&_ZERO,&_FIFTEEN)); rightClickable

          ImGui::TableNextColumn();
          CENTER_VSLIDER;
          P(CWVSliderScalar("##CT",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_S8,&opE.ct,&_MINUS_TWENTY_FOUR,&_TWENTY_FOUR)); rightClickable
          if (ImGui::IsItemHovered()) 
          {
            ImGui::SetTooltip("Detune in semitones");
          }

          ImGui::TableNextColumn();
          CENTER_VSLIDER;
          P(CWVSliderScalar("##DT",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_S8,&opE.dt,&_MINUS_ONE_HUNDRED_TWENTY_EIGHT,&_ONE_HUNDRED_TWENTY_SEVEN)); rightClickable
          if (ImGui::IsItemHovered()) 
          {
            ImGui::SetTooltip("Detune in fractions of semitone.\n128 = +1 semitone, -128 = -1 semitone.");
          }

          ImGui::TableNextColumn();
          bool amOn=op.am;
          bool leftOn=opE.left;
          bool rightOn=opE.right;

          ImGui::SetCursorPosY(ImGui::GetCursorPosY()+0.5*(sliderHeight-ImGui::GetFrameHeight()*5.0-ImGui::GetStyle().ItemSpacing.y*4.0));
          ImVec2 curPosBeforeDummy = ImGui::GetCursorPos();
          ImGui::Dummy(ImVec2(ImGui::GetFrameHeightWithSpacing()*2.0f+ImGui::CalcTextSize(FM_SHORT_NAME(FM_DAM)).x*2.0f,1.0f));
          ImGui::SetCursorPos(curPosBeforeDummy);

          if (ImGui::BeginTable("panCheckboxes",(fixedOn)?3:2,ImGuiTableFlags_SizingStretchProp)) 
          {
            ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,1.0);
            ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,1.0);
            if (fixedOn) {
              ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,1.2);
            }

            float yCoordBeforeTablePadding=ImGui::GetCursorPosY();
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::SetCursorPosY(yCoordBeforeTablePadding);
            if (ImGui::Checkbox(ESFM_SHORT_NAME(ESFM_LEFT),&leftOn)) 
            { PARAMETER
              opE.left=leftOn;
            }
            if (ImGui::IsItemHovered()) 
            {
              ImGui::SetTooltip("If operator outputs sound, enable left channel output.");
            }
            ImGui::TableNextColumn();
            ImGui::SetCursorPosY(yCoordBeforeTablePadding);
            if (ImGui::Checkbox(ESFM_SHORT_NAME(ESFM_RIGHT),&rightOn)) 
            { PARAMETER
              opE.right=rightOn;
            }
            if (ImGui::IsItemHovered()) 
            {
              ImGui::SetTooltip("If operator outputs sound, enable right channel output.");
            }
            if (fixedOn) 
            {
              ImGui::TableNextColumn();
              ImGui::SetCursorPosY(yCoordBeforeTablePadding);
              if (ImGui::Checkbox(FM_SHORT_NAME(FM_AM),&amOn)) 
              { PARAMETER
                op.am=amOn;
              }
            }
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if (ImGui::Checkbox(FM_SHORT_NAME(FM_KSR),&ksrOn)) 
            { PARAMETER
              op.ksr=ksrOn;
            }
            ImGui::TableNextColumn();
            if (ImGui::Checkbox(FM_SHORT_NAME(FM_SUS),&susOn)) 
            { PARAMETER
              op.sus=susOn;
            }
            if (fixedOn) 
            {
              bool damOn=op.dam;
              ImGui::TableNextColumn();
              if (ImGui::Checkbox(FM_SHORT_NAME(FM_DAM),&damOn)) 
              { PARAMETER
                op.dam=damOn;
              }
            }
            ImGui::EndTable();
          }
          ImGui::SetCursorPosY(ImGui::GetCursorPosY()-0.5*ImGui::GetStyle().ItemSpacing.y);
          if (ImGui::Checkbox(ESFM_NAME(ESFM_FIXED),&fixedOn)) 
          { PARAMETER
            opE.fixed=fixedOn;

            ins->std.get_op_macro(i)->op_get_macro(DIV_MACRO_OP_SSG, true)->vZoom = -1;
            ins->std.get_op_macro(i)->op_get_macro(DIV_MACRO_OP_DT, true)->vZoom = -1;
          }
          
          if (fixedOn) 
          {
            int block=(opE.ct>>2)&7;
            int freqNum=((opE.ct&3)<<8)|((unsigned char)opE.dt);
            if (ImGui::InputInt("Block",&block,1,1)) 
            {
              if (block<0) block=0;
              if (block>7) block=7;
              opE.ct=(opE.ct&(~(7<<2)))|(block<<2);
            }
            if (ImGui::InputInt("FreqNum",&freqNum,1,16)) 
            {
              if (freqNum<0) freqNum=0;
              if (freqNum>1023) freqNum=1023;
              opE.dt=freqNum&0xff;
              opE.ct=(opE.ct&(~3))|(freqNum>>8);
            }
          } 
          else 
          {
            if (ImGui::BeginTable("amVibCheckboxes",2,ImGuiTableFlags_SizingStretchSame)) 
            {
              ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.0);
              ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.0);

              float yCoordBeforeTablePadding=ImGui::GetCursorPosY();
              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              ImGui::SetCursorPosY(yCoordBeforeTablePadding);
              if (ImGui::Checkbox(FM_SHORT_NAME(FM_AM),&amOn)) 
              { PARAMETER
                op.am=amOn;
              }
              ImGui::TableNextColumn();
              ImGui::SetCursorPosY(yCoordBeforeTablePadding);
              if (ImGui::Checkbox(FM_SHORT_NAME(FM_VIB),&vibOn)) 
              { PARAMETER
                op.vib=vibOn;
              }
              bool damOn=op.dam;
              bool dvbOn=op.dvb;
              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              if (ImGui::Checkbox(FM_SHORT_NAME(FM_DAM),&damOn)) 
              { PARAMETER
                op.dam=damOn;
              }
              ImGui::TableNextColumn();
              if (ImGui::Checkbox(FM_SHORT_NAME(FM_DVB),&dvbOn)) 
              { PARAMETER
                op.dvb=dvbOn;
              }
              ImGui::EndTable();
            }
          }

          ImGui::TableNextColumn();
          ImGui::Dummy(ImVec2(4.0f*dpiScale,2.0f*dpiScale));
          ImGui::TableNextColumn();

          drawWaveform(op.ws&7,ins->type==DIV_INS_OPZ,ImVec2(ImGui::GetContentRegionAvail().x,sliderHeight-ImGui::GetFrameHeightWithSpacing()*((fixedOn)?3.0f:1.0f)));
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          P(CWSliderScalar("##WS",ImGuiDataType_U8,&op.ws,&_ZERO,&_SEVEN,(settings.oplStandardWaveNames?oplWaveformsStandard[op.ws&7]:oplWaveforms[op.ws&7]))); rightClickable
          if ( fixedOn) 
          {
            if (ImGui::Checkbox(FM_SHORT_NAME(FM_VIB),&vibOn)) { PARAMETER
              op.vib=vibOn;
            }
            bool dvbOn=op.dvb;
            if (ImGui::Checkbox(FM_SHORT_NAME(FM_DVB),&dvbOn)) { PARAMETER
              op.dvb=dvbOn;
            }
          }

          ImGui::TableNextColumn();
          drawFMEnv(op.tl&maxTl,op.ar&maxArDr,op.dr&maxArDr,((op.rr&15)*2),op.rr&15,op.sl&15,op.sus,op.ssgEnv&8,fmOrigin.alg,maxTl,maxArDr,15,ImVec2(ImGui::GetContentRegionAvail().x,sliderHeight),ins->type);

          if (settings.separateFMColors) 
          {
            popAccentColors();
          }

          ImGui::PopID();
        }

        ImGui::EndTable();
      }
    } 
    
    else if (settings.fmLayout>=4 && settings.fmLayout<=6) 
    { // alternate
      int columns=2;
      switch (settings.fmLayout) 
      {
        case 4: // 2x2
          columns=2;
          break;
        case 5: // 1x4
          columns=1;
          break;
        case 6: // 4x1
          columns=opCount;
          break;
      }
      char tempID[1024];
      ImVec2 oldPadding=ImGui::GetStyle().CellPadding;
      ImGui::PushStyleVar(ImGuiStyleVar_CellPadding,ImVec2(8.0f*dpiScale,4.0f*dpiScale));
      if (ImGui::BeginTable("AltFMOperators",columns,ImGuiTableFlags_SizingStretchSame|ImGuiTableFlags_BordersInner)) 
      {
        for (int i=0; i<opCount; i++) 
        {
          DivInstrumentFM::Operator& op=fmOrigin.op[i];
          DivInstrumentESFM::Operator& opE=ins->esfm.op[i];
          if ((settings.fmLayout!=6 && ((i+1)&1)) || i==0 || settings.fmLayout==5) ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::PushID(fmt::sprintf("op%d",i).c_str());

          // push colors
          if (settings.separateFMColors) 
          {
            bool mod=true;
            // this is the same as the KVS heuristic in platform/esfm.h
            if (opE.outLvl==7) mod=false;
            else if (opE.outLvl>0) 
            {
              if (i==3) mod=false;
              else 
              {
                DivInstrumentESFM::Operator& opENext=ins->esfm.op[i+1];
                if (opENext.modIn==0) mod=false;
                else if ((opE.outLvl-opENext.modIn)>=2) mod=false;
              }
            }

            if (mod) 
            {
              pushAccentColors(
                uiColors[GUI_COLOR_FM_PRIMARY_MOD],
                uiColors[GUI_COLOR_FM_SECONDARY_MOD],
                uiColors[GUI_COLOR_FM_BORDER_MOD],
                uiColors[GUI_COLOR_FM_BORDER_SHADOW_MOD]
              );
            } 
            else 
            {
              pushAccentColors(
                uiColors[GUI_COLOR_FM_PRIMARY_CAR],
                uiColors[GUI_COLOR_FM_SECONDARY_CAR],
                uiColors[GUI_COLOR_FM_BORDER_CAR],
                uiColors[GUI_COLOR_FM_BORDER_SHADOW_CAR]
              );
            }
          }

          ImGui::Dummy(ImVec2(dpiScale,dpiScale));
          snprintf(tempID,1024,"Operator %d",i+1);

          float nextCursorPosX=ImGui::GetCursorPosX()+0.5*(ImGui::GetContentRegionAvail().x-ImGui::CalcTextSize(tempID).x-(opsAreMutable?(ImGui::GetStyle().FramePadding.x*2.0f):0.0f));
          OP_DRAG_POINT;
          ImGui::SameLine();
          ImGui::SetCursorPosX(nextCursorPosX);
          ImGui::TextUnformatted(tempID);

          float sliderHeight=200.0f*dpiScale;
          float waveWidth=140.0*dpiScale*((ins->type==DIV_INS_ESFM)?0.85f:1.0f);
          float waveHeight=sliderHeight-ImGui::GetFrameHeightWithSpacing()*(5.0f);

          int maxTl=63;
          int maxArDr=15;

          bool ksrOn=op.ksr;
          bool vibOn=op.vib;
          bool susOn=op.sus; // yawn

          ImGui::PushStyleVar(ImGuiStyleVar_CellPadding,oldPadding);
          if (ImGui::BeginTable("opParams",4,ImGuiTableFlags_BordersInnerV)) 
          {
            ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed,waveWidth);
            ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthFixed);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            float textY=ImGui::GetCursorPosY();
            CENTER_TEXT_20(ESFM_SHORT_NAME(ESFM_DELAY));
            ImGui::TextUnformatted(ESFM_SHORT_NAME(ESFM_DELAY));
            TOOLTIP_TEXT(ESFM_LONG_NAME(ESFM_DELAY));
            ImGui::TableNextColumn();
            ImGui::Text("Waveform");
            ImGui::TableNextColumn();
            ImGui::Text("Envelope");
            ImGui::TableNextColumn();

            // A/D/S/R
            ImGui::TableNextColumn();

            opE.delay&=7;
            P(CWVSliderScalar("##DELAY",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&opE.delay,&_ZERO,&_SEVEN)); rightClickable
            ImGui::SameLine();

            op.ar&=maxArDr;
            float textX_AR=ImGui::GetCursorPosX();
            P(CWVSliderScalar("##AR",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.ar,&maxArDr,&_ZERO)); rightClickable

            ImGui::SameLine();
            op.dr&=maxArDr;
            float textX_DR=ImGui::GetCursorPosX();
            P(CWVSliderScalar("##DR",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.dr,&maxArDr,&_ZERO)); rightClickable

            float textX_SL=0.0f;
            if (settings.susPosition==0) 
            {
              ImGui::SameLine();
              op.sl&=15;
              textX_SL=ImGui::GetCursorPosX();
              P(CWVSliderScalar("##SL",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.sl,&_FIFTEEN,&_ZERO)); rightClickable
            }

            ImGui::SameLine();
            op.rr&=15;
            float textX_RR=ImGui::GetCursorPosX();
            P(CWVSliderScalar("##RR",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.rr,&_FIFTEEN,&_ZERO)); rightClickable

            if (settings.susPosition==1) 
            {
              ImGui::SameLine();
              op.sl&=15;
              textX_SL=ImGui::GetCursorPosX();
              P(CWVSliderScalar("##SL",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.sl,&_FIFTEEN,&_ZERO)); rightClickable
            }

            ImVec2 prevCurPos=ImGui::GetCursorPos();

            // labels
            ImGui::SetCursorPos(ImVec2(textX_AR,textY));
            CENTER_TEXT_20(FM_SHORT_NAME(FM_AR));
            ImGui::TextUnformatted(FM_SHORT_NAME(FM_AR));
            TOOLTIP_TEXT(FM_NAME(FM_AR));

            ImGui::SetCursorPos(ImVec2(textX_DR,textY));
            CENTER_TEXT_20(FM_SHORT_NAME(FM_DR));
            ImGui::TextUnformatted(FM_SHORT_NAME(FM_DR));
            TOOLTIP_TEXT(FM_NAME(FM_DR));

            ImGui::SetCursorPos(ImVec2(textX_SL,textY));
            CENTER_TEXT_20(FM_SHORT_NAME(FM_SL));
            ImGui::TextUnformatted(FM_SHORT_NAME(FM_SL));
            TOOLTIP_TEXT(FM_NAME(FM_SL));

            ImGui::SetCursorPos(ImVec2(textX_RR,textY));
            CENTER_TEXT_20(FM_SHORT_NAME(FM_RR));
            ImGui::TextUnformatted(FM_SHORT_NAME(FM_RR));
            TOOLTIP_TEXT(FM_NAME(FM_RR));

            ImGui::SetCursorPos(prevCurPos);
            
            ImGui::TableNextColumn();

            // waveform
            drawWaveform(op.ws&7,ins->type==DIV_INS_OPZ,ImVec2(waveWidth,waveHeight));
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            P(CWSliderScalar("##WS",ImGuiDataType_U8,&op.ws,&_ZERO,&_SEVEN,(ins->type==DIV_INS_OPZ)?opzWaveforms[op.ws&7]:(settings.oplStandardWaveNames?oplWaveformsStandard[op.ws&7]:oplWaveforms[op.ws&7]))); rightClickable

            // params
            ImGui::Separator();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_MULT));
            P(CWSliderScalar("##MULT",ImGuiDataType_U8,&op.mult,&_ZERO,&_FIFTEEN,tempID)); rightClickable

            if (opE.fixed) 
            {
              int block=(opE.ct>>2)&7;
              int freqNum=((opE.ct&3)<<8)|((unsigned char)opE.dt);
              ImGui::Text("Blk");
              if (ImGui::IsItemHovered()) 
              {
                ImGui::SetTooltip("Block");
              }
              ImGui::SameLine();
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
              //ImVec2 cursorAlign=ImGui::GetCursorPos();
              if (ImGui::InputInt("##Block",&block,1,1)) 
              {
                if (block<0) block=0;
                if (block>7) block=7;
                opE.ct=(opE.ct&(~(7<<2)))|(block<<2);
              }

              ImGui::Text("F");
              if (ImGui::IsItemHovered()) 
              {
                ImGui::SetTooltip("Frequency (F-Num)");
              }
              ImGui::SameLine();
              //ImGui::SetCursorPos(ImVec2(cursorAlign.x,ImGui::GetCursorPosY()));
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
              if (ImGui::InputInt("##FreqNum",&freqNum,1,16)) 
              {
                if (freqNum<0) freqNum=0;
                if (freqNum>1023) freqNum=1023;
                opE.dt=freqNum&0xff;
                opE.ct=(opE.ct&(~3))|(freqNum>>8);
              }
            } 
            
            else 
            {
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
              snprintf(tempID,1024,"%s: %%d",ESFM_NAME(ESFM_CT));
              P(CWSliderScalar("##CT",ImGuiDataType_S8,&opE.ct,&_MINUS_TWENTY_FOUR,&_TWENTY_FOUR,tempID)); rightClickable
              if (ImGui::IsItemHovered()) 
              {
                ImGui::SetTooltip("Detune in semitones");
              }

              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
              snprintf(tempID,1024,"%s: %%d",ESFM_NAME(ESFM_DT));
              P(CWSliderScalar("##DT",ImGuiDataType_S8,&opE.dt,&_MINUS_ONE_HUNDRED_TWENTY_EIGHT,&_ONE_HUNDRED_TWENTY_SEVEN,tempID)); rightClickable
              if (ImGui::IsItemHovered()) 
              {
                ImGui::SetTooltip("Detune in fractions of semitone.\n128 = +1 semitone, -128 = -1 semitone.");
              }
            }

            if (ImGui::BeginTable("panCheckboxes",2,ImGuiTableFlags_SizingStretchSame)) 
            {
              ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.0f);
              ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.0f);

              float yPosOutsideTablePadding=ImGui::GetCursorPosY();
              bool leftOn=opE.left;
              bool rightOn=opE.right;
              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              ImGui::SetCursorPosY(yPosOutsideTablePadding);
              if (ImGui::Checkbox(ESFM_SHORT_NAME(ESFM_LEFT),&leftOn)) 
              { PARAMETER
                opE.left=leftOn;
              }
              if (ImGui::IsItemHovered()) 
              {
                ImGui::SetTooltip("If operator outputs sound, enable left channel output.");
              }
              ImGui::TableNextColumn();
              ImGui::SetCursorPosY(yPosOutsideTablePadding);
              if (ImGui::Checkbox(ESFM_SHORT_NAME(ESFM_RIGHT),&rightOn)) 
              { PARAMETER
                opE.right=rightOn;
              }
              if (ImGui::IsItemHovered()) 
              {
                ImGui::SetTooltip("If operator outputs sound, enable right channel output.");
              }
              ImGui::EndTable();
            }

            ImGui::TableNextColumn();
            float envHeight=sliderHeight;//-ImGui::GetStyle().ItemSpacing.y*2.0f;
            envHeight-=ImGui::GetFrameHeightWithSpacing()*3.0f;
            drawFMEnv(op.tl&maxTl,op.ar&maxArDr,op.dr&maxArDr,((op.rr&15)*2),op.rr&15,op.sl&15,op.sus,op.ssgEnv&8,fmOrigin.alg,maxTl,maxArDr,15,ImVec2(ImGui::GetContentRegionAvail().x,envHeight),ins->type);

            ImGui::Separator();
            if (ImGui::BeginTable("FMParamsInnerESFM",2)) 
            {
              ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.64f);
              ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.36f);
              ImGui::TableNextRow();
              ImGui::TableNextColumn();

              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
              snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_KSL));
              int ksl=kslMap[op.ksl&3];
              if (CWSliderInt("##KSL",&ksl,0,3,tempID)) 
              {
                op.ksl=kslMap[ksl&3];
                PARAMETER;
              } rightClickable

              bool amOn=op.am;
              bool fixedOn=opE.fixed;
              ImGui::TableNextColumn();
              if (ImGui::Checkbox(FM_SHORT_NAME(FM_KSR),&ksrOn)) 
              { PARAMETER
                op.ksr=ksrOn;
              }
              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              if (ImGui::BeginTable("vibAmCheckboxes",2)) 
              {
                ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.0f);
                ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.0f);

                float yPosOutsideTablePadding=ImGui::GetCursorPosY();
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::SetCursorPosY(yPosOutsideTablePadding);
                if (ImGui::Checkbox(FM_SHORT_NAME(FM_VIB),&vibOn)) 
                { PARAMETER
                  op.vib=vibOn;
                }
                ImGui::TableNextColumn();
                ImGui::SetCursorPosY(yPosOutsideTablePadding);
                if (ImGui::Checkbox(FM_SHORT_NAME(FM_AM),&amOn)) 
                { PARAMETER
                  op.am=amOn;
                }

                bool damOn=op.dam;
                bool dvbOn=op.dvb;
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                if (ImGui::Checkbox(FM_SHORT_NAME(FM_DVB),&dvbOn)) 
                { PARAMETER
                  op.dvb=dvbOn;
                }
                ImGui::TableNextColumn();
                if (ImGui::Checkbox(FM_SHORT_NAME(FM_DAM),&damOn)) 
                { PARAMETER
                  op.dam=damOn;
                }
                ImGui::EndTable();
              }
              ImGui::TableNextColumn();
              if (ImGui::Checkbox(FM_SHORT_NAME(FM_SUS),&susOn)) 
              { PARAMETER
                op.sus=susOn;
              }
              if (ImGui::Checkbox(ESFM_NAME(ESFM_FIXED),&fixedOn)) 
              { PARAMETER
                opE.fixed=fixedOn;

                ins->std.get_op_macro(i)->op_get_macro(DIV_MACRO_OP_SSG, true)->vZoom = -1;
                ins->std.get_op_macro(i)->op_get_macro(DIV_MACRO_OP_DT, true)->vZoom = -1;
              }

              ImGui::EndTable();
            }

            ImGui::TableNextColumn();
            op.tl&=maxTl;
            float tlSliderWidth=(ins->type==DIV_INS_ESFM)?20.0f*dpiScale:ImGui::GetFrameHeight();
            float tlSliderHeight=sliderHeight;
            float textX_tl=ImGui::GetCursorPosX();
            P(CWVSliderScalar("##TL",ImVec2(tlSliderWidth,tlSliderHeight),ImGuiDataType_U8,&op.tl,&maxTl,&_ZERO)); rightClickable

            ImGui::SameLine();
            float textX_outLvl=ImGui::GetCursorPosX();
            P(CWVSliderScalar("##OUTLVL",ImVec2(tlSliderWidth,tlSliderHeight),ImGuiDataType_U8,&opE.outLvl,&_ZERO,&_SEVEN)); rightClickable

            ImGui::SameLine();
            float textX_modIn=ImGui::GetCursorPosX();
            P(CWVSliderScalar("##MODIN",ImVec2(tlSliderWidth,tlSliderHeight),ImGuiDataType_U8,&opE.modIn,&_ZERO,&_SEVEN)); rightClickable

            prevCurPos=ImGui::GetCursorPos();
            ImGui::SetCursorPos(ImVec2(textX_tl,textY));
            CENTER_TEXT_20(FM_SHORT_NAME(FM_TL));
            ImGui::TextUnformatted(FM_SHORT_NAME(FM_TL));
            TOOLTIP_TEXT(FM_NAME(FM_TL));

            ImGui::SetCursorPos(ImVec2(textX_outLvl,textY));
            CENTER_TEXT_20(ESFM_SHORT_NAME(ESFM_OUTLVL));
            ImGui::TextUnformatted(ESFM_SHORT_NAME(ESFM_OUTLVL));
            TOOLTIP_TEXT(ESFM_LONG_NAME(ESFM_OUTLVL));

            ImGui::SetCursorPos(ImVec2(textX_modIn,textY));
            CENTER_TEXT_20(ESFM_SHORT_NAME(ESFM_MODIN));
            ImGui::TextUnformatted(ESFM_SHORT_NAME(ESFM_MODIN));
            TOOLTIP_TEXT(ESFM_LONG_NAME(ESFM_MODIN));

            ImGui::SetCursorPos(prevCurPos);

            ImGui::EndTable();
          }
          ImGui::PopStyleVar();

          if (settings.separateFMColors) 
          {
            popAccentColors();
          }

          ImGui::PopID();
        }
        ImGui::EndTable();
      }
      ImGui::PopStyleVar();
    } 
    
    else 
    { // classic
      int columns=2;
      switch (settings.fmLayout) 
      {
        case 1: // 2x2
          columns=2;
          break;
        case 2: // 1x4
          columns=1;
          break;
        case 3: // 4x1
          columns=opCount;
          break;
      }
      if (ImGui::BeginTable("FMOperators",columns,ImGuiTableFlags_SizingStretchSame)) 
      {
        for (int i=0; i<opCount; i++) {
          DivInstrumentFM::Operator& op=fmOrigin.op[i];
          DivInstrumentESFM::Operator& opE=ins->esfm.op[i];
          if ((settings.fmLayout!=3 && ((i+1)&1)) || i==0 || settings.fmLayout==2) ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::Separator();
          ImGui::PushID(fmt::sprintf("op%d",i).c_str());

          // push colors
          if (settings.separateFMColors) 
          {
            bool mod=true;
            // this is the same as the KVS heuristic in platform/esfm.h
            if (opE.outLvl==7) mod=false;
            else if (opE.outLvl>0) 
            {
              if (i==3) mod=false;
              else 
              {
                DivInstrumentESFM::Operator& opENext=ins->esfm.op[i+1];
                if (opENext.modIn==0) mod=false;
                else if ((opE.outLvl-opENext.modIn)>=2) mod=false;
              }
            }
            if (mod) 
            {
              pushAccentColors(
                uiColors[GUI_COLOR_FM_PRIMARY_MOD],
                uiColors[GUI_COLOR_FM_SECONDARY_MOD],
                uiColors[GUI_COLOR_FM_BORDER_MOD],
                uiColors[GUI_COLOR_FM_BORDER_SHADOW_MOD]
              );
            } 
            else 
            {
              pushAccentColors(
                uiColors[GUI_COLOR_FM_PRIMARY_CAR],
                uiColors[GUI_COLOR_FM_SECONDARY_CAR],
                uiColors[GUI_COLOR_FM_BORDER_CAR],
                uiColors[GUI_COLOR_FM_BORDER_SHADOW_CAR]
              );
            }
          }

          ImGui::Dummy(ImVec2(dpiScale,dpiScale));
          String opNameLabel;
          OP_DRAG_POINT;
          ImGui::SameLine();
          opNameLabel=fmt::sprintf("OP%d",i+1);

          if (opsAreMutable) 
          {
            pushToggleColors(op.enable);
            if (ImGui::Button(opNameLabel.c_str())) 
            {
              op.enable=!op.enable;
              PARAMETER;
            }
            popToggleColors();
          } 
          
          else 
          {
            ImGui::TextUnformatted(opNameLabel.c_str());
          }

          ImGui::SameLine();

          bool amOn=op.am;
          if (ImGui::Checkbox(FM_NAME(FM_AM),&amOn)) 
          { PARAMETER
            op.am=amOn;
          }

          int maxTl=63;
          int maxArDr=15;

          bool ksrOn=op.ksr;
          bool vibOn=op.vib;
          bool susOn=op.sus; // don't you make fun of this one

          ImGui::SameLine();
          if (ImGui::Checkbox(FM_NAME(FM_SUS),&susOn)) 
          { PARAMETER
            op.sus=susOn;
          }

          //52.0 controls vert scaling; default 96
          drawFMEnv(op.tl&maxTl,op.ar&maxArDr,op.dr&maxArDr,((op.rr&15)*2),op.rr&15,op.sl&15,op.sus,op.ssgEnv&8,fmOrigin.alg,maxTl,maxArDr,15,ImVec2(ImGui::GetContentRegionAvail().x,52.0*dpiScale),ins->type);
          //P(CWSliderScalar(FM_NAME(FM_AR),ImGuiDataType_U8,&op.ar,&_ZERO,&_THIRTY_ONE)); rightClickable
          if (ImGui::BeginTable("opParams",2,ImGuiTableFlags_SizingStretchProp)) 
          {
            ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.0); \
            ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed,0.0); \

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            opE.delay&=7;
            P(CWSliderScalar("##DELAY",ImGuiDataType_U8,&opE.delay,&_ZERO,&_SEVEN)); rightClickable
            ImGui::TableNextColumn();
            ImGui::Text("%s",ESFM_NAME(ESFM_DELAY));

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            op.ar&=maxArDr;
            P(CWSliderScalar("##AR",ImGuiDataType_U8,&op.ar,&maxArDr,&_ZERO)); rightClickable
            ImGui::TableNextColumn();
            ImGui::Text("%s",FM_NAME(FM_AR));

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            op.dr&=maxArDr;
            P(CWSliderScalar("##DR",ImGuiDataType_U8,&op.dr,&maxArDr,&_ZERO)); rightClickable
            ImGui::TableNextColumn();
            ImGui::Text("%s",FM_NAME(FM_DR));

            if (settings.susPosition==0) 
            {
              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
              P(CWSliderScalar("##SL",ImGuiDataType_U8,&op.sl,&_FIFTEEN,&_ZERO)); rightClickable
              ImGui::TableNextColumn();
              ImGui::Text("%s",FM_NAME(FM_SL));
            }

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            P(CWSliderScalar("##RR",ImGuiDataType_U8,&op.rr,&_FIFTEEN,&_ZERO)); rightClickable
            ImGui::TableNextColumn();
            ImGui::Text("%s",FM_NAME(FM_RR));

            if (settings.susPosition==1) 
            {
              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
              P(CWSliderScalar("##SL",ImGuiDataType_U8,&op.sl,&_FIFTEEN,&_ZERO)); rightClickable
              ImGui::TableNextColumn();
              ImGui::Text("%s",FM_NAME(FM_SL));
            }

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            op.tl&=maxTl;
            P(CWSliderScalar("##TL",ImGuiDataType_U8,&op.tl,&maxTl,&_ZERO)); rightClickable
            ImGui::TableNextColumn();
            ImGui::Text("%s",FM_NAME(FM_TL));

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Separator();
            ImGui::TableNextColumn();
            ImGui::Separator();
            
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

            int ksl=ins->type==DIV_INS_OPLL?op.ksl:kslMap[op.ksl&3];
            if (CWSliderInt("##KSL",&ksl,0,3)) 
            {
              op.ksl=(ins->type==DIV_INS_OPLL?ksl:kslMap[ksl&3]);
              PARAMETER;
            } rightClickable
            ImGui::TableNextColumn();
            ImGui::Text("%s",FM_NAME(FM_KSL));

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            P(CWSliderScalar(FM_NAME(FM_MULT),ImGuiDataType_U8,&op.mult,&_ZERO,&_FIFTEEN)); rightClickable
            ImGui::TableNextColumn();
            ImGui::Text("%s",FM_NAME(FM_MULT));

            bool fixedOn=opE.fixed;
            if (fixedOn) 
            {
              int block=(opE.ct>>2)&7;
              int freqNum=((opE.ct&3)<<8)|((unsigned char)opE.dt);
              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
              if (ImGui::InputInt("##Block",&block,1,1)) 
              {
                if (block<0) block=0;
                if (block>7) block=7;
                opE.ct=(opE.ct&(~(7<<2)))|(block<<2);
              }
              ImGui::TableNextColumn();
              ImGui::Text("Block");
              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
              if (ImGui::InputInt("##FreqNum",&freqNum,1,16)) 
              {
                if (freqNum<0) freqNum=0;
                if (freqNum>1023) freqNum=1023;
                opE.dt=freqNum&0xff;
                opE.ct=(opE.ct&(~3))|(freqNum>>8);
              }
              ImGui::TableNextColumn();
              ImGui::Text("FreqNum");
            } 
            else 
            {
              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
              P(CWSliderScalar("##CT",ImGuiDataType_S8,&opE.ct,&_MINUS_TWENTY_FOUR,&_TWENTY_FOUR)); rightClickable
              if (ImGui::IsItemHovered()) 
              {
                ImGui::SetTooltip("Detune in semitones");
              }
              ImGui::TableNextColumn();
              ImGui::Text("%s",ESFM_NAME(ESFM_CT));

              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
              P(CWSliderScalar("##DT",ImGuiDataType_S8,&opE.dt,&_MINUS_ONE_HUNDRED_TWENTY_EIGHT,&_ONE_HUNDRED_TWENTY_SEVEN)); rightClickable
              if (ImGui::IsItemHovered()) 
              {
                ImGui::SetTooltip("Detune in fractions of semitone.\n128 = +1 semitone, -128 = -1 semitone.");
              }
              ImGui::TableNextColumn();
              ImGui::Text("%s",ESFM_NAME(ESFM_DT));
            }

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            P(CWSliderScalar("##WS",ImGuiDataType_U8,&op.ws,&_ZERO,&_SEVEN,(settings.oplStandardWaveNames?oplWaveformsStandard[op.ws&7]:oplWaveforms[op.ws&7]))); rightClickable
            ImGui::TableNextColumn();
            ImGui::Text("%s",FM_NAME(FM_WS));

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Separator();
            ImGui::TableNextColumn();
            ImGui::Separator();

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            P(CWSliderScalar("##OUTLVL",ImGuiDataType_U8,&opE.outLvl,&_ZERO,&_SEVEN)); rightClickable
            ImGui::TableNextColumn();
            ImGui::Text("%s",ESFM_NAME(ESFM_OUTLVL));

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            P(CWSliderScalar("##MODIN",ImGuiDataType_U8,&opE.modIn,&_ZERO,&_SEVEN)); rightClickable
            ImGui::TableNextColumn();
            ImGui::Text("%s",ESFM_NAME(ESFM_MODIN));

            ImGui::EndTable();
          }

          if (ImGui::Checkbox(FM_NAME(FM_VIB),&vibOn)) 
          { PARAMETER
            op.vib=vibOn;
          }
          ImGui::SameLine();
          if (ImGui::Checkbox(FM_NAME(FM_KSR),&ksrOn)) 
          { PARAMETER
            op.ksr=ksrOn;
          }

          bool dvbOn=op.dvb;
          bool damOn=op.dam;
          bool leftOn=opE.left;
          bool rightOn=opE.right;
          bool fixedOn=opE.fixed;
          if (ImGui::Checkbox(FM_NAME(FM_DVB),&dvbOn)) 
          { PARAMETER
            op.dvb=dvbOn;
          }
          ImGui::SameLine();
          if (ImGui::Checkbox(FM_NAME(FM_DAM),&damOn)) 
          { PARAMETER
            op.dam=damOn;
          }
          if (ImGui::Checkbox(ESFM_NAME(ESFM_LEFT),&leftOn)) 
          { PARAMETER
            opE.left=leftOn;
          }
          if (ImGui::IsItemHovered()) 
          {
            ImGui::SetTooltip("If operator outputs sound, enable left channel output.");
          }
          ImGui::SameLine();
          if (ImGui::Checkbox(ESFM_NAME(ESFM_RIGHT),&rightOn)) 
          { PARAMETER
            opE.right=rightOn;
          }
          if (ImGui::IsItemHovered()) 
          {
            ImGui::SetTooltip("If operator outputs sound, enable right channel output.");
          }
          ImGui::SameLine();
          if (ImGui::Checkbox(ESFM_NAME(ESFM_FIXED),&fixedOn)) 
          { PARAMETER
            opE.fixed=fixedOn;

            ins->std.get_op_macro(i)->op_get_macro(DIV_MACRO_OP_SSG, true)->vZoom = -1;
            ins->std.get_op_macro(i)->op_get_macro(DIV_MACRO_OP_DT, true)->vZoom = -1;
          }

          if (settings.separateFMColors) 
          {
            popAccentColors();
          }

          ImGui::PopID();
        }
        ImGui::EndTable();
      }
    }
    ImGui::EndDisabled();
    ImGui::EndTabItem();
  }

  for (int i=0; i<opCount; i++)
  {
    if((int)ins->std.ops.size() < opCount)
    {
      int limit = opCount - ins->std.ops.size();

      for(int j = 0; j < limit; j++)
      {
        ins->std.ops.push_back(DivInstrumentSTD::OpMacro());
      }
    }
  }

  for (int i=0; i<opCount; i++) 
  {
    snprintf(label,31,"OP%d Macros",i+1);

    if (ImGui::BeginTabItem(label)) 
    {
      ImGui::PushID(i);
      int ordi=i;
      int maxTl=63;
      int maxArDr=15;

      macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_TL),ins,(DivMacroType)DIV_MACRO_OP_TL,ordi,0,maxTl,128,uiColors[GUI_COLOR_MACRO_OTHER]));
      macroList.push_back(FurnaceGUIMacroDesc(ESFM_NAME(ESFM_DELAY),ins,(DivMacroType)DIV_MACRO_OP_DT2,ordi,0,7,64,uiColors[GUI_COLOR_MACRO_OTHER]));
      macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_AR),ins,(DivMacroType)DIV_MACRO_OP_AR,ordi,0,maxArDr,64,uiColors[GUI_COLOR_MACRO_OTHER]));
      macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_DR),ins,(DivMacroType)DIV_MACRO_OP_DR,ordi,0,maxArDr,64,uiColors[GUI_COLOR_MACRO_OTHER]));
      macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_SL),ins,(DivMacroType)DIV_MACRO_OP_SL,ordi,0,15,64,uiColors[GUI_COLOR_MACRO_OTHER]));
      macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_RR),ins,(DivMacroType)DIV_MACRO_OP_RR,ordi,0,15,64,uiColors[GUI_COLOR_MACRO_OTHER]));
      macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_KSL),ins,(DivMacroType)DIV_MACRO_OP_KSL,ordi,0,3,32,uiColors[GUI_COLOR_MACRO_OTHER]));
      macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_MULT),ins,(DivMacroType)DIV_MACRO_OP_MULT,ordi,0,15,64,uiColors[GUI_COLOR_MACRO_OTHER]));
      macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_WS),ins,(DivMacroType)DIV_MACRO_OP_WS,ordi,0,7,64,uiColors[GUI_COLOR_MACRO_OTHER]));
      macroList.push_back(FurnaceGUIMacroDesc(ESFM_NAME(ESFM_OUTLVL),ins,(DivMacroType)DIV_MACRO_OP_EGT,ordi,0,7,64,uiColors[GUI_COLOR_MACRO_OTHER]));
      macroList.push_back(FurnaceGUIMacroDesc(ESFM_NAME(ESFM_MODIN),ins,(DivMacroType)DIV_MACRO_OP_D2R,ordi,0,7,64,uiColors[GUI_COLOR_MACRO_OTHER]));

      if (ins->esfm.op[ordi].fixed) 
      {
        macroList.push_back(FurnaceGUIMacroDesc("Block",ins,(DivMacroType)DIV_MACRO_OP_SSG,ordi,0,7,64,uiColors[GUI_COLOR_MACRO_OTHER],true));
        macroList.push_back(FurnaceGUIMacroDesc("FreqNum",ins,(DivMacroType)DIV_MACRO_OP_DT,ordi,0,1023,160,uiColors[GUI_COLOR_MACRO_OTHER]));
      }

      else 
      {
        macroList.push_back(FurnaceGUIMacroDesc("Op. Arpeggio",ins,(DivMacroType)DIV_MACRO_OP_SSG,ordi,-120,120,160,uiColors[GUI_COLOR_MACRO_PITCH],true,NULL,macroHoverNote,false,NULL,0,true,ins->std.ops[ordi].op_get_macro(DIV_MACRO_OP_SSG, true)->val,true));
        macroList.push_back(FurnaceGUIMacroDesc("Op. Pitch",ins,(DivMacroType)DIV_MACRO_OP_DT,ordi,-2048,2047,160,uiColors[GUI_COLOR_MACRO_PITCH],true,macroRelativeMode,NULL,false,NULL,0,false,NULL,false,true));
      }

      macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_AM),ins,(DivMacroType)DIV_MACRO_OP_AM,ordi,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
      macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_VIB),ins,(DivMacroType)DIV_MACRO_OP_VIB,ordi,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
      macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_DAM),ins,(DivMacroType)DIV_MACRO_OP_DAM,ordi,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
      macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_DVB),ins,(DivMacroType)DIV_MACRO_OP_DVB,ordi,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
      macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_KSR),ins,(DivMacroType)DIV_MACRO_OP_KSR,ordi,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
      macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_SUS),ins,(DivMacroType)DIV_MACRO_OP_SUS,ordi,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
      macroList.push_back(FurnaceGUIMacroDesc("Op. Panning",ins,(DivMacroType)DIV_MACRO_OP_RS,ordi,0,2,40,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,panBits));

      drawMacros(macroList,macroEditStateOP[ordi]);
      ImGui::PopID();
      ImGui::EndTabItem();
    }
  }
}