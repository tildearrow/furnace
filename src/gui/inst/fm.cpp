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

void FurnaceGUI::drawInsFMFM(DivInstrument* ins)
{
  opCount=4;
  opsAreMutable=true;

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

    if (ImGui::BeginTable("fmDetails",3,ImGuiTableFlags_SizingStretchSame)) 
    {
      ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.0f);
      ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.0f);
      ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.0f);

      ImGui::TableNextRow();
      
      ImGui::TableNextColumn();
      P(CWSliderScalar(FM_NAME(FM_FB),ImGuiDataType_U8,&ins->fm.fb,&_ZERO,&_SEVEN)); rightClickable
      P(CWSliderScalar(FM_NAME(FM_FMS),ImGuiDataType_U8,&ins->fm.fms,&_ZERO,&_SEVEN)); rightClickable
      ImGui::TableNextColumn();
      P(CWSliderScalar(FM_NAME(FM_ALG),ImGuiDataType_U8,&ins->fm.alg,&_ZERO,&_SEVEN)); rightClickable
      P(CWSliderScalar(FM_NAME(FM_AMS),ImGuiDataType_U8,&ins->fm.ams,&_ZERO,&_THREE)); rightClickable
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
        drawAlgorithm(ins->fm.alg,FM_ALGS_4OP,ImVec2(ImGui::GetContentRegionAvail().x,48.0*dpiScale));
      }

      kvsConfig(ins);
      ImGui::EndTable();
    }

    bool willDisplayOps=true;

    ImGui::BeginDisabled(!willDisplayOps);
    if (settings.fmLayout==0) 
    {
      int numCols=15;
      if (ImGui::BeginTable("FMOperators",numCols,ImGuiTableFlags_SizingStretchProp|ImGuiTableFlags_BordersH|ImGuiTableFlags_BordersOuterV)) 
      {
        // configure columns
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed); // op name
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.05f); // ar
        ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.05f); // dr
        ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthStretch,0.05f); // sl
        ImGui::TableSetupColumn("c4",ImGuiTableColumnFlags_WidthStretch,0.05f); // d2r
        ImGui::TableSetupColumn("c5",ImGuiTableColumnFlags_WidthStretch,0.05f); // rr
        ImGui::TableSetupColumn("c6",ImGuiTableColumnFlags_WidthFixed); // -separator-
        ImGui::TableSetupColumn("c7",ImGuiTableColumnFlags_WidthStretch,0.05f); // tl
        ImGui::TableSetupColumn("c8",ImGuiTableColumnFlags_WidthStretch,0.05f); // rs/ksl
        ImGui::TableSetupColumn("c10",ImGuiTableColumnFlags_WidthStretch,0.05f); // dt
        ImGui::TableSetupColumn("c15",ImGuiTableColumnFlags_WidthFixed); // am
        ImGui::TableSetupColumn("c12",ImGuiTableColumnFlags_WidthFixed); // -separator-
        ImGui::TableSetupColumn("c13",ImGuiTableColumnFlags_WidthStretch,0.2f); // ssg/waveform
        ImGui::TableSetupColumn("c14",ImGuiTableColumnFlags_WidthStretch,0.3f); // env

        // header
        ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
        ImGui::TableNextColumn();
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
        CENTER_TEXT(FM_SHORT_NAME(FM_D2R));
        ImGui::TextUnformatted(FM_SHORT_NAME(FM_D2R));
        TOOLTIP_TEXT(FM_NAME(FM_D2R));
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
        CENTER_TEXT(FM_SHORT_NAME(FM_RS));
        ImGui::TextUnformatted(FM_SHORT_NAME(FM_RS));
        TOOLTIP_TEXT(FM_NAME(FM_RS));
        ImGui::TableNextColumn();
        CENTER_TEXT(FM_SHORT_NAME(FM_MULT));
        ImGui::TextUnformatted(FM_SHORT_NAME(FM_MULT));
        TOOLTIP_TEXT(FM_NAME(FM_MULT));
        ImGui::TableNextColumn();
        CENTER_TEXT(FM_SHORT_NAME(FM_DT));
        ImGui::TextUnformatted(FM_SHORT_NAME(FM_DT));
        TOOLTIP_TEXT(FM_NAME(FM_DT));
        ImGui::TableNextColumn();
        CENTER_TEXT(FM_SHORT_NAME(FM_AM));
        ImGui::TextUnformatted(FM_SHORT_NAME(FM_AM));
        TOOLTIP_TEXT(FM_NAME(FM_AM));
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        CENTER_TEXT(FM_NAME(FM_SSG));
        ImGui::TextUnformatted(FM_NAME(FM_SSG));
        ImGui::TableNextColumn();
        CENTER_TEXT("Envelope");
        ImGui::TextUnformatted("Envelope");

        float sliderHeight=32.0f*dpiScale;

        for (int i=0; i<opCount; i++) 
        {
          DivInstrumentFM::Operator& op=fmOrigin.op[opOrder[i]];

          ImGui::TableNextRow();
          ImGui::TableNextColumn();

          // push colors
          if (settings.separateFMColors) 
          {
            bool mod=true;
            if (opCount==4) 
            {
              if (i==1) mod=false;
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
          }

          ImGui::PushID(fmt::sprintf("op%d",i).c_str());
          String opNameLabel;
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

          // drag point
          OP_DRAG_POINT;

          int maxTl=127;
          int maxArDr=31;
          bool ssgOn=op.ssgEnv&8;
          unsigned char ssgEnv=op.ssgEnv&7;

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
          op.d2r&=31;
          CENTER_VSLIDER;
          P(CWVSliderScalar("##D2R",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.d2r,&_THIRTY_ONE,&_ZERO)); rightClickable

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
          P(CWVSliderScalar("##RS",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.rs,&_ZERO,&_THREE)); rightClickable

          ImGui::TableNextColumn();
          CENTER_VSLIDER;
          P(CWVSliderScalar("##MULT",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.mult,&_ZERO,&_FIFTEEN)); rightClickable

          
          int detune=detuneMap[settings.unsignedDetune?1:0][op.dt&7];
          ImGui::TableNextColumn();
          CENTER_VSLIDER;
          if (CWVSliderInt("##DT",ImVec2(20.0f*dpiScale,sliderHeight),&detune,settings.unsignedDetune?0:-3,settings.unsignedDetune?7:4)) 
          { PARAMETER
            if (detune<-3) detune=-3;
            if (detune>7) detune=7;
            op.dt=detuneUnmap[settings.unsignedDetune?1:0][detune+3];
          } rightClickable

          ImGui::TableNextColumn();
          bool amOn=op.am;

          ImGui::SetCursorPosY(ImGui::GetCursorPosY()+0.5*(sliderHeight-ImGui::GetFrameHeight()));
          if (ImGui::Checkbox("##AM",&amOn)) 
          { PARAMETER
            op.am=amOn;
          }

          ImGui::TableNextColumn();
          ImGui::Dummy(ImVec2(4.0f*dpiScale,2.0f*dpiScale));
          ImGui::TableNextColumn();
          ImGui::BeginDisabled(!ssgOn);
          drawSSGEnv(op.ssgEnv&7,ImVec2(ImGui::GetContentRegionAvail().x,sliderHeight-ImGui::GetFrameHeightWithSpacing()));
          ImGui::EndDisabled();
          if (ImGui::Checkbox("##SSGOn",&ssgOn)) 
          { PARAMETER
            op.ssgEnv=(op.ssgEnv&7)|(ssgOn<<3);
          }

          ImGui::SameLine();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (CWSliderScalar("##SSG",ImGuiDataType_U8,&ssgEnv,&_ZERO,&_SEVEN,ssgEnvTypes[ssgEnv])) 
          { PARAMETER
            op.ssgEnv=(op.ssgEnv&8)|(ssgEnv&7);
          }

          ImGui::TableNextColumn();
          drawFMEnv(op.tl&maxTl,op.ar&maxArDr,op.dr&maxArDr,(ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_OPLL || ins->type==DIV_INS_ESFM)?((op.rr&15)*2):op.d2r&31,op.rr&15,op.sl&15,op.sus,op.ssgEnv&8,fmOrigin.alg,maxTl,maxArDr,15,ImVec2(ImGui::GetContentRegionAvail().x,sliderHeight),ins->type);

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
          DivInstrumentFM::Operator& op=fmOrigin.op[opOrder[i]];
          if ((settings.fmLayout!=6 && ((i+1)&1)) || i==0 || settings.fmLayout==5) ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::PushID(fmt::sprintf("op%d",i).c_str());

          // push colors
          if (settings.separateFMColors) 
          {
            bool mod=true;
            if (opCount==4) 
            {
              if (opIsOutput[fmOrigin.alg&7][i]) mod=false;
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
          if (opsAreMutable) 
          {
            pushToggleColors(op.enable);
            if (ImGui::Button(tempID)) 
            {
              op.enable=!op.enable;
              PARAMETER;
            }

            popToggleColors();
          }

          float sliderHeight=200.0f*dpiScale;
          float waveWidth=140.0*dpiScale*((ins->type==DIV_INS_ESFM)?0.85f:1.0f);
          float waveHeight=sliderHeight-ImGui::GetFrameHeightWithSpacing()*4.5f;

          int maxTl=127;
          int maxArDr=31;

          bool ssgOn=op.ssgEnv&8;
          unsigned char ssgEnv=op.ssgEnv&7;

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
            CENTER_TEXT_20(FM_SHORT_NAME(FM_AR));
            ImGui::TextUnformatted(FM_SHORT_NAME(FM_AR));
            TOOLTIP_TEXT(FM_NAME(FM_AR));
            ImGui::TableNextColumn();
            ImGui::Text("SSG-EG");
            ImGui::TableNextColumn();
            ImGui::Text("Envelope");
            ImGui::TableNextColumn();

            // A/D/S/R
            ImGui::TableNextColumn();

            op.ar&=maxArDr;
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

            float textX_D2R=0.0f;
            ImGui::SameLine();
            op.d2r&=31;
            textX_D2R=ImGui::GetCursorPosX();
            P(CWVSliderScalar("##D2R",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.d2r,&_THIRTY_ONE,&_ZERO)); rightClickable

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

            ImGui::SetCursorPos(ImVec2(textX_D2R,textY));
            CENTER_TEXT_20(FM_SHORT_NAME(FM_D2R));
            ImGui::TextUnformatted(FM_SHORT_NAME(FM_D2R));
            TOOLTIP_TEXT(FM_NAME(FM_D2R));

            ImGui::SetCursorPos(prevCurPos);
            
            ImGui::TableNextColumn();

            ImGui::BeginDisabled(!ssgOn);
            drawSSGEnv(op.ssgEnv&7,ImVec2(waveWidth,waveHeight));
            ImGui::EndDisabled();
            if (ImGui::Checkbox("##SSGOn",&ssgOn)) 
            { PARAMETER
              op.ssgEnv=(op.ssgEnv&7)|(ssgOn<<3);
            }

            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (CWSliderScalar("##SSG",ImGuiDataType_U8,&ssgEnv,&_ZERO,&_SEVEN,ssgEnvTypes[ssgEnv])) 
            { PARAMETER
              op.ssgEnv=(op.ssgEnv&8)|(ssgEnv&7);
            }
            
            // params
            ImGui::Separator();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_MULT));
            P(CWSliderScalar("##MULT",ImGuiDataType_U8,&op.mult,&_ZERO,&_FIFTEEN,tempID)); rightClickable

            int detune=detuneMap[settings.unsignedDetune?1:0][op.dt&7];
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_DT));
            if (CWSliderInt("##DT",&detune,settings.unsignedDetune?0:-3,settings.unsignedDetune?7:4,tempID)) 
            { PARAMETER
              if (detune<-3) detune=-3;
              if (detune>7) detune=7;
              op.dt=detuneUnmap[settings.unsignedDetune?1:0][detune+3];
            } rightClickable

            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_RS));
            P(CWSliderScalar("##RS",ImGuiDataType_U8,&op.rs,&_ZERO,&_THREE,tempID)); rightClickable

            ImGui::TableNextColumn();
            float envHeight=sliderHeight;//-ImGui::GetStyle().ItemSpacing.y*2.0f;

            drawFMEnv(op.tl&maxTl,op.ar&maxArDr,op.dr&maxArDr,op.d2r&31,op.rr&15,op.sl&15,op.sus,op.ssgEnv&8,fmOrigin.alg,maxTl,maxArDr,15,ImVec2(ImGui::GetContentRegionAvail().x,envHeight),ins->type);

            ImGui::TableNextColumn();
            op.tl&=maxTl;
            float tlSliderWidth=(ins->type==DIV_INS_ESFM)?20.0f*dpiScale:ImGui::GetFrameHeight();
            float tlSliderHeight=sliderHeight-((ins->type==DIV_INS_FM || ins->type==DIV_INS_OPM)?(ImGui::GetFrameHeightWithSpacing()+ImGui::CalcTextSize(FM_SHORT_NAME(FM_AM)).y+ImGui::GetStyle().ItemSpacing.y):0.0f);
            float textX_tl=ImGui::GetCursorPosX();
            P(CWVSliderScalar("##TL",ImVec2(tlSliderWidth,tlSliderHeight),ImGuiDataType_U8,&op.tl,&maxTl,&_ZERO)); rightClickable

            CENTER_TEXT(FM_SHORT_NAME(FM_AM));
            ImGui::TextUnformatted(FM_SHORT_NAME(FM_AM));
            TOOLTIP_TEXT(FM_NAME(FM_AM));
            bool amOn=op.am;
            if (ImGui::Checkbox("##AM",&amOn)) 
            { PARAMETER
              op.am=amOn;
            }

            prevCurPos=ImGui::GetCursorPos();
            ImGui::SetCursorPos(ImVec2(textX_tl,textY));
            CENTER_TEXT(FM_SHORT_NAME(FM_TL));
            ImGui::TextUnformatted(FM_SHORT_NAME(FM_TL));
            TOOLTIP_TEXT(FM_NAME(FM_TL));

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
        for (int i=0; i<opCount; i++)
        {
          DivInstrumentFM::Operator& op=fmOrigin.op[opOrder[i]];

          if ((settings.fmLayout!=3 && ((i+1)&1)) || i==0 || settings.fmLayout==2) ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::Separator();
          ImGui::PushID(fmt::sprintf("op%d",i).c_str());

          // push colors
          if (settings.separateFMColors) {
            bool mod=true;
            if (opIsOutput[fmOrigin.alg&7][i]) mod=false;
            
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
            if (ImGui::Button(opNameLabel.c_str())) {
              op.enable=!op.enable;
              PARAMETER;
            }
            popToggleColors();
          }

          ImGui::SameLine();

          bool amOn=op.am;
          if (ImGui::Checkbox(FM_NAME(FM_AM),&amOn)) 
          { PARAMETER
            op.am=amOn;
          }

          int maxTl=127;
          int maxArDr=31;

          bool ssgOn=op.ssgEnv&8;
          unsigned char ssgEnv=op.ssgEnv&7;
          ImGui::SameLine();
          if (ImGui::Checkbox((ins->type==DIV_INS_OPLL)?FM_NAME(FM_EGS):"SSG On",&ssgOn)) 
          { PARAMETER
            op.ssgEnv=(op.ssgEnv&7)|(ssgOn<<3);
          }

          //52.0 controls vert scaling; default 96
          drawFMEnv(op.tl&maxTl,op.ar&maxArDr,op.dr&maxArDr,op.d2r&31,op.rr&15,op.sl&15,op.sus,op.ssgEnv&8,fmOrigin.alg,maxTl,maxArDr,15,ImVec2(ImGui::GetContentRegionAvail().x,52.0*dpiScale),ins->type);
          //P(CWSliderScalar(FM_NAME(FM_AR),ImGuiDataType_U8,&op.ar,&_ZERO,&_THIRTY_ONE)); rightClickable
          if (ImGui::BeginTable("opParams",2,ImGuiTableFlags_SizingStretchProp)) 
          {
            ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.0);
            ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed,0.0);

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
            P(CWSliderScalar("##D2R",ImGuiDataType_U8,&op.d2r,&_THIRTY_ONE,&_ZERO)); rightClickable
            ImGui::TableNextColumn();
            ImGui::Text("%s",FM_NAME(FM_D2R));

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

            P(CWSliderScalar("##RS",ImGuiDataType_U8,&op.rs,&_ZERO,&_THREE)); rightClickable
            ImGui::TableNextColumn();
            ImGui::Text("%s",FM_NAME(FM_RS));

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            P(CWSliderScalar(FM_NAME(FM_MULT),ImGuiDataType_U8,&op.mult,&_ZERO,&_FIFTEEN)); rightClickable
            ImGui::TableNextColumn();
            ImGui::Text("%s",FM_NAME(FM_MULT));

            int detune=detuneMap[settings.unsignedDetune?1:0][op.dt&7];
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

            if (CWSliderInt("##DT",&detune,settings.unsignedDetune?0:-3,settings.unsignedDetune?7:4)) 
            { PARAMETER
              if (detune<-3) detune=-3;
              if (detune>7) detune=7;
              op.dt=detuneUnmap[settings.unsignedDetune?1:0][detune+3];
            } rightClickable

            ImGui::TableNextColumn();
            ImGui::Text("%s",FM_NAME(FM_DT));

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

            if (CWSliderScalar("##SSG",ImGuiDataType_U8,&ssgEnv,&_ZERO,&_SEVEN,ssgEnvTypes[ssgEnv])) 
            { PARAMETER
              op.ssgEnv=(op.ssgEnv&8)|(ssgEnv&7);
            } rightClickable

            ImGui::TableNextColumn();
            ImGui::Text("%s",FM_NAME(FM_SSG));

            ImGui::EndTable();
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
  
  if (ImGui::BeginTabItem("FM Macros")) 
  {
    macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_ALG),ins,DIV_MACRO_ALG,0xff,0,7,96,uiColors[GUI_COLOR_MACRO_OTHER]));
    macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_FB),ins,DIV_MACRO_FB,0xff,0,7,96,uiColors[GUI_COLOR_MACRO_OTHER]));
    macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_FMS),ins,DIV_MACRO_FMS,0xff,0,7,96,uiColors[GUI_COLOR_MACRO_OTHER]));
    macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_AMS),ins,DIV_MACRO_AMS,0xff,0,3,48,uiColors[GUI_COLOR_MACRO_OTHER]));
    macroList.push_back(FurnaceGUIMacroDesc("LFO Speed",ins,DIV_MACRO_EX3,0xff,0,8,96,uiColors[GUI_COLOR_MACRO_OTHER]));
    macroList.push_back(FurnaceGUIMacroDesc("OpMask",ins,DIV_MACRO_EX4,0xff,0,4,128,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,fmOperatorBits));

    for(int i = 0; i < (int)ins->std.macros.size(); i++) // reset macro zoom
    {
      ins->std.macros[i].vZoom = -1;
    }

    drawMacros(macroList,macroEditStateFM);
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
      int ordi=orderedOps[i];
      int maxTl=127;
      int maxArDr=31;

      macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_TL),ins,(DivMacroType)DIV_MACRO_OP_TL,ordi,0,maxTl,128,uiColors[GUI_COLOR_MACRO_OTHER]));
      macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_AR),ins,(DivMacroType)DIV_MACRO_OP_AR,ordi,0,maxArDr,64,uiColors[GUI_COLOR_MACRO_OTHER]));
      macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_DR),ins,(DivMacroType)DIV_MACRO_OP_DR,ordi,0,maxArDr,64,uiColors[GUI_COLOR_MACRO_OTHER]));
      macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_D2R),ins,(DivMacroType)DIV_MACRO_OP_D2R,ordi,0,31,64,uiColors[GUI_COLOR_MACRO_OTHER]));
      macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_RR),ins,(DivMacroType)DIV_MACRO_OP_RR,ordi,0,15,64,uiColors[GUI_COLOR_MACRO_OTHER]));
      macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_SL),ins,(DivMacroType)DIV_MACRO_OP_SL,ordi,0,15,64,uiColors[GUI_COLOR_MACRO_OTHER]));
      macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_RS),ins,(DivMacroType)DIV_MACRO_OP_RS,ordi,0,3,32,uiColors[GUI_COLOR_MACRO_OTHER]));
      macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_MULT),ins,(DivMacroType)DIV_MACRO_OP_MULT,ordi,0,15,64,uiColors[GUI_COLOR_MACRO_OTHER]));
      macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_DT),ins,(DivMacroType)DIV_MACRO_OP_DT,ordi,0,7,64,uiColors[GUI_COLOR_MACRO_OTHER]));
      macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_AM),ins,(DivMacroType)DIV_MACRO_OP_AM,ordi,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
      macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_SSG),ins,(DivMacroType)DIV_MACRO_OP_SSG,ordi,0,4,64,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,ssgEnvBits));

      drawMacros(macroList,macroEditStateOP[ordi]);
      ImGui::PopID();
      ImGui::EndTabItem();
    }
  }
}

void FurnaceGUI::drawInsFM(DivInstrument* ins)
{
  if(ins->type == DIV_INS_FM)
  {
    drawInsFMFM(ins); return;
  }

  opCount=4;
  if (ins->type==DIV_INS_OPLL) opCount=2;
  if (ins->type==DIV_INS_OPL) opCount=(ins->fm.ops==4)?4:2;
  opsAreMutable=(ins->type==DIV_INS_FM || ins->type==DIV_INS_OPM);

  if (ImGui::BeginTabItem("FM")) {
    DivInstrumentFM& fmOrigin=(ins->type==DIV_INS_OPLL && ins->fm.opllPreset>0 && ins->fm.opllPreset<16)?opllPreview:ins->fm;

    isPresentCount=0;
    memset(isPresent,0,4*sizeof(bool));
    for (int i=0; i<e->song.systemLen; i++) {
      if (e->song.system[i]==DIV_SYSTEM_VRC7) {
        isPresent[3]=true;
      } else if (e->song.system[i]==DIV_SYSTEM_OPLL || e->song.system[i]==DIV_SYSTEM_OPLL_DRUMS) {
        isPresent[(e->song.systemFlags[i].getInt("patchSet",0))&3]=true;
      }
    }
    if (!isPresent[0] && !isPresent[1] && !isPresent[2] && !isPresent[3]) {
      isPresent[0]=true;
    }
    for (int i=0; i<4; i++) {
      if (isPresent[i]) isPresentCount++;
    }
    presentWhich=0;
    for (int i=0; i<4; i++) {
      if (isPresent[i]) {
        presentWhich=i;
        break;
      }
    }

    if (ImGui::BeginTable("fmDetails",3,(ins->type==DIV_INS_ESFM)?ImGuiTableFlags_SizingStretchProp:ImGuiTableFlags_SizingStretchSame)) {
      ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,((ins->type==DIV_INS_ESFM)?0.50f:0.0f));
      ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,((ins->type==DIV_INS_ESFM)?0.15f:0.0f));
      ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,((ins->type==DIV_INS_ESFM)?0.35f:0.0f));

      ImGui::TableNextRow();
      switch (ins->type) {
        case DIV_INS_FM:
        case DIV_INS_OPM:
          ImGui::TableNextColumn();
          P(CWSliderScalar(FM_NAME(FM_FB),ImGuiDataType_U8,&ins->fm.fb,&_ZERO,&_SEVEN)); rightClickable
          P(CWSliderScalar(FM_NAME(FM_FMS),ImGuiDataType_U8,&ins->fm.fms,&_ZERO,&_SEVEN)); rightClickable
          ImGui::TableNextColumn();
          P(CWSliderScalar(FM_NAME(FM_ALG),ImGuiDataType_U8,&ins->fm.alg,&_ZERO,&_SEVEN)); rightClickable
          P(CWSliderScalar(FM_NAME(FM_AMS),ImGuiDataType_U8,&ins->fm.ams,&_ZERO,&_THREE)); rightClickable
          ImGui::TableNextColumn();
          if (fmPreviewOn) {
            drawFMPreview(ImVec2(ImGui::GetContentRegionAvail().x,48.0*dpiScale));
            if (!fmPreviewPaused) {
              renderFMPreview(ins,1);
              WAKE_UP;
            }
          } else {
            drawAlgorithm(ins->fm.alg,FM_ALGS_4OP,ImVec2(ImGui::GetContentRegionAvail().x,48.0*dpiScale));
          }
          kvsConfig(ins);
          break;
        case DIV_INS_OPZ:
          ImGui::TableNextColumn();
          P(CWSliderScalar(FM_NAME(FM_FB),ImGuiDataType_U8,&ins->fm.fb,&_ZERO,&_SEVEN)); rightClickable
          P(CWSliderScalar(FM_NAME(FM_FMS),ImGuiDataType_U8,&ins->fm.fms,&_ZERO,&_SEVEN)); rightClickable
          P(CWSliderScalar(FM_NAME(FM_FMS2),ImGuiDataType_U8,&ins->fm.fms2,&_ZERO,&_SEVEN)); rightClickable
          ImGui::TableNextColumn();
          P(CWSliderScalar(FM_NAME(FM_ALG),ImGuiDataType_U8,&ins->fm.alg,&_ZERO,&_SEVEN)); rightClickable
          P(CWSliderScalar(FM_NAME(FM_AMS),ImGuiDataType_U8,&ins->fm.ams,&_ZERO,&_THREE)); rightClickable
          P(CWSliderScalar(FM_NAME(FM_AMS2),ImGuiDataType_U8,&ins->fm.ams2,&_ZERO,&_THREE)); rightClickable
          ImGui::TableNextColumn();
          if (fmPreviewOn) {
            drawFMPreview(ImVec2(ImGui::GetContentRegionAvail().x,48.0*dpiScale));
            if (!fmPreviewPaused) {
              renderFMPreview(ins,1);
              WAKE_UP;
            }
          } else {
            drawAlgorithm(ins->fm.alg,FM_ALGS_4OP,ImVec2(ImGui::GetContentRegionAvail().x,48.0*dpiScale));
          }
          kvsConfig(ins);

          if (ImGui::Button("Request from TX81Z")) {
            doAction(GUI_ACTION_TX81Z_REQUEST);
          }
          /* 
          ImGui::SameLine();
          if (ImGui::Button("Send to TX81Z")) {
            showError("Coming soon!");
          }
          */
          break;
        case DIV_INS_OPL:
        case DIV_INS_OPL_DRUMS: {
          bool fourOp=(ins->fm.ops==4 || ins->type==DIV_INS_OPL_DRUMS);
          bool drums=ins->fm.opllPreset==16;
          int algMax=fourOp?3:1;
          ImGui::TableNextColumn();
          ins->fm.alg&=algMax;
          P(CWSliderScalar(FM_NAME(FM_FB),ImGuiDataType_U8,&ins->fm.fb,&_ZERO,&_SEVEN)); rightClickable
          if (ins->type==DIV_INS_OPL) {
            ImGui::BeginDisabled(ins->fm.opllPreset==16);
            if (ImGui::Checkbox("4-op",&fourOp)) { PARAMETER
              ins->fm.ops=fourOp?4:2;
            }
            ImGui::EndDisabled();
          }
          ImGui::TableNextColumn();
          P(CWSliderScalar(FM_NAME(FM_ALG),ImGuiDataType_U8,&ins->fm.alg,&_ZERO,&algMax)); rightClickable
          if (ins->type==DIV_INS_OPL) {
            if (ImGui::Checkbox("Drums",&drums)) { PARAMETER
              ins->fm.opllPreset=drums?16:0;
            }
          }
          ImGui::TableNextColumn();
          if (fmPreviewOn) {
            drawFMPreview(ImVec2(ImGui::GetContentRegionAvail().x,48.0*dpiScale));
            if (!fmPreviewPaused) {
              renderFMPreview(ins,1);
              WAKE_UP;
            }
          } else {
            drawAlgorithm(ins->fm.alg&algMax,fourOp?FM_ALGS_4OP_OPL:FM_ALGS_2OP_OPL,ImVec2(ImGui::GetContentRegionAvail().x,48.0*dpiScale));
          }
          kvsConfig(ins);
          break;
        }
        case DIV_INS_OPLL: {
          bool dc=fmOrigin.fms;
          bool dm=fmOrigin.ams;
          bool sus=ins->fm.alg;
          ImGui::TableNextColumn();
          ImGui::BeginDisabled(ins->fm.opllPreset!=0);
          P(CWSliderScalar(FM_NAME(FM_FB),ImGuiDataType_U8,&fmOrigin.fb,&_ZERO,&_SEVEN)); rightClickable
          if (ImGui::Checkbox(FM_NAME(FM_DC),&dc)) { PARAMETER
            fmOrigin.fms=dc;
          }
          ImGui::EndDisabled();
          ImGui::TableNextColumn();
          if (ImGui::Checkbox(FM_NAME(FM_SUS),&sus)) { PARAMETER
            ins->fm.alg=sus;
          }
          ImGui::BeginDisabled(ins->fm.opllPreset!=0);
          if (ImGui::Checkbox(FM_NAME(FM_DM),&dm)) { PARAMETER
            fmOrigin.ams=dm;
          }
          ImGui::EndDisabled();
          ImGui::TableNextColumn();
          if (fmPreviewOn) {
            drawFMPreview(ImVec2(ImGui::GetContentRegionAvail().x,24.0*dpiScale));
            if (!fmPreviewPaused) {
              renderFMPreview(ins,1);
              WAKE_UP;
            }
          } else {
            drawAlgorithm(0,FM_ALGS_2OP_OPL,ImVec2(ImGui::GetContentRegionAvail().x,24.0*dpiScale));
          }
          kvsConfig(ins,false);

          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

          if (ImGui::BeginCombo("##LLPreset",opllInsNames[presentWhich][ins->fm.opllPreset])) {
            if (isPresentCount>1) {
              if (ImGui::BeginTable("LLPresetList",isPresentCount)) {
                ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
                for (int i=0; i<4; i++) {
                  if (!isPresent[i]) continue;
                  ImGui::TableNextColumn();
                  ImGui::Text("%s name",opllVariants[i]);
                }
                for (int i=0; i<17; i++) {
                  ImGui::TableNextRow();
                  for (int j=0; j<4; j++) {
                    if (!isPresent[j]) continue;
                    ImGui::TableNextColumn();
                    ImGui::PushID(j*17+i);
                    if (ImGui::Selectable(opllInsNames[j][i])) {
                      ins->fm.opllPreset=i;
                    }
                    ImGui::PopID();
                  }
                }
                ImGui::EndTable();
              }
            } else {
              for (int i=0; i<17; i++) {
                if (ImGui::Selectable(opllInsNames[presentWhich][i])) {
                  ins->fm.opllPreset=i;
                }
              }
            }
            ImGui::EndCombo();
          }
          break;
        }
        case DIV_INS_ESFM: {
          ImGui::TableNextColumn();
          P(CWSliderScalar(ESFM_LONG_NAME(ESFM_NOISE),ImGuiDataType_U8,&ins->esfm.noise,&_ZERO,&_THREE,esfmNoiseModeNames[ins->esfm.noise&3])); rightClickable
          ImGui::TextUnformatted(esfmNoiseModeDescriptions[ins->esfm.noise&3]);
          ImGui::TableNextColumn();
          ImGui::TableNextColumn();
          if (fmPreviewOn) {
            drawFMPreview(ImVec2(ImGui::GetContentRegionAvail().x,48.0*dpiScale));
            if (!fmPreviewPaused) {
              renderFMPreview(ins,1);
              WAKE_UP;
            }
          } else {
            drawESFMAlgorithm(ins->esfm, ImVec2(ImGui::GetContentRegionAvail().x,48.0*dpiScale));
          }
          kvsConfig(ins);
        }
        default:
          break;
      }
      ImGui::EndTable();
    }

    if (((ins->type==DIV_INS_OPLL || ins->type==DIV_INS_OPL) && ins->fm.opllPreset==16) || ins->type==DIV_INS_OPL_DRUMS) {
      ins->fm.ops=2;
      P(ImGui::Checkbox("Fixed frequency mode",&ins->fm.fixedDrums));
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("when enabled, drums will be set to the specified frequencies, ignoring the note.");
      }
      if (ins->fm.fixedDrums) {
        int block=0;
        int fNum=0;
        if (ImGui::BeginTable("fixedDrumSettings",3)) {
          ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
          ImGui::TableNextColumn();
          ImGui::Text("Drum");
          ImGui::TableNextColumn();
          ImGui::Text("Block");
          ImGui::TableNextColumn();
          ImGui::Text("FreqNum");

          DRUM_FREQ("Kick","##DBlock0","##DFreq0",ins->fm.kickFreq);
          DRUM_FREQ("Snare/Hi-hat","##DBlock1","##DFreq1",ins->fm.snareHatFreq);
          DRUM_FREQ("Tom/Top","##DBlock2","##DFreq2",ins->fm.tomTopFreq);
          ImGui::EndTable();
        }
      }
    }

    bool willDisplayOps=true;
    if (ins->type==DIV_INS_OPLL && ins->fm.opllPreset!=0) willDisplayOps=false;
    if (!willDisplayOps && ins->type==DIV_INS_OPLL) {
      ins->fm.op[1].tl&=15;
      P(CWSliderScalar("Volume##TL",ImGuiDataType_U8,&ins->fm.op[1].tl,&_FIFTEEN,&_ZERO)); rightClickable
      if (ins->fm.opllPreset==16) {
        ImGui::Text("this volume slider only works in compatibility (non-drums) system.");
      }

      // update OPLL preset preview
      if (ins->fm.opllPreset>0 && ins->fm.opllPreset<16) {
        const opll_patch_t* patchROM=NULL;

        switch (presentWhich) {
          case 1:
            patchROM=OPLL_GetPatchROM(opll_type_ymf281);
            break;
          case 2:
            patchROM=OPLL_GetPatchROM(opll_type_ym2423);
            break;
          case 3:
            patchROM=OPLL_GetPatchROM(opll_type_ds1001);
            break;
          default:
            patchROM=OPLL_GetPatchROM(opll_type_ym2413);
            break;
        }

        const opll_patch_t* patch=&patchROM[ins->fm.opllPreset-1];

        opllPreview.alg=ins->fm.alg;
        opllPreview.fb=patch->fb;
        opllPreview.fms=patch->dm;
        opllPreview.ams=patch->dc;

        opllPreview.op[0].tl=patch->tl;
        opllPreview.op[1].tl=ins->fm.op[1].tl;

        for (int i=0; i<2; i++) {
          opllPreview.op[i].am=patch->am[i];
          opllPreview.op[i].vib=patch->vib[i];
          opllPreview.op[i].ssgEnv=patch->et[i]?8:0;
          opllPreview.op[i].ksr=patch->ksr[i];
          opllPreview.op[i].mult=patch->multi[i];
          opllPreview.op[i].ar=patch->ar[i];
          opllPreview.op[i].dr=patch->dr[i];
          opllPreview.op[i].sl=patch->sl[i];
          opllPreview.op[i].rr=patch->rr[i];
        }
      }
    }

    ImGui::BeginDisabled(!willDisplayOps);
    if (settings.fmLayout==0) {
      int numCols=15;
      if (ins->type==DIV_INS_OPL ||ins->type==DIV_INS_OPL_DRUMS) numCols=13;
      if (ins->type==DIV_INS_OPLL) numCols=12;
      if (ins->type==DIV_INS_OPZ) numCols=19;
      if (ins->type==DIV_INS_ESFM) numCols=19;
      if (ImGui::BeginTable("FMOperators",numCols,ImGuiTableFlags_SizingStretchProp|ImGuiTableFlags_BordersH|ImGuiTableFlags_BordersOuterV)) {
        // configure columns
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed); // op name
        if (ins->type==DIV_INS_ESFM) {
          ImGui::TableSetupColumn("c0e0",ImGuiTableColumnFlags_WidthStretch,0.05f); // outLvl
          ImGui::TableSetupColumn("c0e1",ImGuiTableColumnFlags_WidthFixed); // -separator-
          ImGui::TableSetupColumn("c0e2",ImGuiTableColumnFlags_WidthStretch,0.05f); // delay
        }
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.05f); // ar
        ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.05f); // dr
        ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthStretch,0.05f); // sl
        if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM) {
          ImGui::TableSetupColumn("c4",ImGuiTableColumnFlags_WidthStretch,0.05f); // d2r
        }
        ImGui::TableSetupColumn("c5",ImGuiTableColumnFlags_WidthStretch,0.05f); // rr
        ImGui::TableSetupColumn("c6",ImGuiTableColumnFlags_WidthFixed); // -separator-
        ImGui::TableSetupColumn("c7",ImGuiTableColumnFlags_WidthStretch,0.05f); // tl
        ImGui::TableSetupColumn("c8",ImGuiTableColumnFlags_WidthStretch,0.05f); // rs/ksl
        if (ins->type==DIV_INS_OPZ) {
          ImGui::TableSetupColumn("c8z0",ImGuiTableColumnFlags_WidthStretch,0.05f); // egs
          ImGui::TableSetupColumn("c8z1",ImGuiTableColumnFlags_WidthStretch,0.05f); // rev
        }
        if (ins->type==DIV_INS_ESFM) {
          ImGui::TableSetupColumn("c8e0",ImGuiTableColumnFlags_WidthStretch,0.05f); // outLvl
        }
        ImGui::TableSetupColumn("c9",ImGuiTableColumnFlags_WidthStretch,0.05f); // mult

        if (ins->type==DIV_INS_OPZ) {
          ImGui::TableSetupColumn("c9z",ImGuiTableColumnFlags_WidthStretch,0.05f); // fine
        }

        if (ins->type==DIV_INS_ESFM) {
          ImGui::TableSetupColumn("c9e",ImGuiTableColumnFlags_WidthStretch,0.05f); // ct
        }

        if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM || ins->type==DIV_INS_ESFM) {
          ImGui::TableSetupColumn("c10",ImGuiTableColumnFlags_WidthStretch,0.05f); // dt
        }
        if (ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM) {
          ImGui::TableSetupColumn("c11",ImGuiTableColumnFlags_WidthStretch,0.05f); // dt2
        }
        ImGui::TableSetupColumn("c15",ImGuiTableColumnFlags_WidthFixed); // am

        ImGui::TableSetupColumn("c12",ImGuiTableColumnFlags_WidthFixed); // -separator-
        if (ins->type!=DIV_INS_OPLL && ins->type!=DIV_INS_OPM) {
          ImGui::TableSetupColumn("c13",ImGuiTableColumnFlags_WidthStretch,0.2f); // ssg/waveform
        }
        ImGui::TableSetupColumn("c14",ImGuiTableColumnFlags_WidthStretch,0.3f); // env

        // header
        ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
        ImGui::TableNextColumn();
        if (ins->type==DIV_INS_ESFM) {
          ImGui::TableNextColumn();
          CENTER_TEXT(ESFM_SHORT_NAME(ESFM_MODIN));
          ImGui::TextUnformatted(ESFM_SHORT_NAME(ESFM_MODIN));
          TOOLTIP_TEXT(ESFM_LONG_NAME(ESFM_MODIN));
          ImGui::TableNextColumn();
          ImGui::TableNextColumn();
          CENTER_TEXT(ESFM_SHORT_NAME(ESFM_DELAY));
          ImGui::TextUnformatted(ESFM_SHORT_NAME(ESFM_DELAY));
          TOOLTIP_TEXT(ESFM_LONG_NAME(ESFM_DELAY));
        }
        ImGui::TableNextColumn();
        CENTER_TEXT(FM_SHORT_NAME(FM_AR));
        ImGui::TextUnformatted(FM_SHORT_NAME(FM_AR));
        TOOLTIP_TEXT(FM_NAME(FM_AR));
        ImGui::TableNextColumn();
        CENTER_TEXT(FM_SHORT_NAME(FM_DR));
        ImGui::TextUnformatted(FM_SHORT_NAME(FM_DR));
        TOOLTIP_TEXT(FM_NAME(FM_DR));
        if (settings.susPosition==0) {
          ImGui::TableNextColumn();
          CENTER_TEXT(FM_SHORT_NAME(FM_SL));
          ImGui::TextUnformatted(FM_SHORT_NAME(FM_SL));
          TOOLTIP_TEXT(FM_NAME(FM_SL));
        }
        if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM) {
          ImGui::TableNextColumn();
          CENTER_TEXT(FM_SHORT_NAME(FM_D2R));
          ImGui::TextUnformatted(FM_SHORT_NAME(FM_D2R));
          TOOLTIP_TEXT(FM_NAME(FM_D2R));
        }
        ImGui::TableNextColumn();
        CENTER_TEXT(FM_SHORT_NAME(FM_RR));
        ImGui::TextUnformatted(FM_SHORT_NAME(FM_RR));
        TOOLTIP_TEXT(FM_NAME(FM_RR));
        if (settings.susPosition==1) {
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
        if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM) {
          CENTER_TEXT(FM_SHORT_NAME(FM_RS));
          ImGui::TextUnformatted(FM_SHORT_NAME(FM_RS));
          TOOLTIP_TEXT(FM_NAME(FM_RS));
        } else {
          CENTER_TEXT(FM_SHORT_NAME(FM_KSL));
          ImGui::TextUnformatted(FM_SHORT_NAME(FM_KSL));
          TOOLTIP_TEXT(FM_NAME(FM_KSL));
        }
        if (ins->type==DIV_INS_OPZ) {
          ImGui::TableNextColumn();
          CENTER_TEXT(FM_SHORT_NAME(FM_EGSHIFT));
          ImGui::TextUnformatted(FM_SHORT_NAME(FM_EGSHIFT));
          TOOLTIP_TEXT(FM_NAME(FM_EGSHIFT));
          ImGui::TableNextColumn();
          CENTER_TEXT(FM_SHORT_NAME(FM_REV));
          ImGui::TextUnformatted(FM_SHORT_NAME(FM_REV));
          TOOLTIP_TEXT(FM_NAME(FM_REV));
        }
        if (ins->type==DIV_INS_ESFM) {
          ImGui::TableNextColumn();
          CENTER_TEXT(ESFM_SHORT_NAME(ESFM_OUTLVL));
          ImGui::TextUnformatted(ESFM_SHORT_NAME(ESFM_OUTLVL));
          TOOLTIP_TEXT(ESFM_LONG_NAME(ESFM_OUTLVL));
        }
        ImGui::TableNextColumn();
        CENTER_TEXT(FM_SHORT_NAME(FM_MULT));
        ImGui::TextUnformatted(FM_SHORT_NAME(FM_MULT));
        TOOLTIP_TEXT(FM_NAME(FM_MULT));
        if (ins->type==DIV_INS_OPZ) {
          ImGui::TableNextColumn();
          CENTER_TEXT(FM_SHORT_NAME(FM_FINE));
          ImGui::TextUnformatted(FM_SHORT_NAME(FM_FINE));
          TOOLTIP_TEXT(FM_NAME(FM_FINE));
        }
        if (ins->type==DIV_INS_ESFM) {
          ImGui::TableNextColumn();
          CENTER_TEXT(ESFM_SHORT_NAME(ESFM_CT));
          ImGui::TextUnformatted(ESFM_SHORT_NAME(ESFM_CT));
          TOOLTIP_TEXT(ESFM_LONG_NAME(ESFM_CT));
        }
        ImGui::TableNextColumn();
        if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM) {
          CENTER_TEXT(FM_SHORT_NAME(FM_DT));
          ImGui::TextUnformatted(FM_SHORT_NAME(FM_DT));
          TOOLTIP_TEXT(FM_NAME(FM_DT));
          ImGui::TableNextColumn();
        }
        if (ins->type==DIV_INS_ESFM) {
          CENTER_TEXT(ESFM_SHORT_NAME(ESFM_DT));
          ImGui::TextUnformatted(ESFM_SHORT_NAME(ESFM_DT));
          TOOLTIP_TEXT(ESFM_LONG_NAME(ESFM_DT));
          ImGui::TableNextColumn();
        }
        if (ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM) {
          CENTER_TEXT(FM_SHORT_NAME(FM_DT2));
          ImGui::TextUnformatted(FM_SHORT_NAME(FM_DT2));
          TOOLTIP_TEXT(FM_NAME(FM_DT2));
          ImGui::TableNextColumn();
        }
        if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPM) {
          CENTER_TEXT(FM_SHORT_NAME(FM_AM));
          ImGui::TextUnformatted(FM_SHORT_NAME(FM_AM));
          TOOLTIP_TEXT(FM_NAME(FM_AM));
        } else {
          CENTER_TEXT("Other");
          ImGui::TextUnformatted("Other");
        }
        ImGui::TableNextColumn();
        if (ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_ESFM) {
          ImGui::TableNextColumn();
          CENTER_TEXT(FM_NAME(FM_WS));
          ImGui::TextUnformatted(FM_NAME(FM_WS));
        } else if (ins->type!=DIV_INS_OPLL && ins->type!=DIV_INS_OPM) {
          ImGui::TableNextColumn();
          CENTER_TEXT(FM_NAME(FM_SSG));
          ImGui::TextUnformatted(FM_NAME(FM_SSG));
        }
        ImGui::TableNextColumn();
        CENTER_TEXT("Envelope");
        ImGui::TextUnformatted("Envelope");

        float sliderHeight=32.0f*dpiScale;

        for (int i=0; i<opCount; i++) {
          DivInstrumentFM::Operator& op=fmOrigin.op[(opCount==4 && ins->type!=DIV_INS_OPL_DRUMS && ins->type!=DIV_INS_ESFM)?opOrder[i]:i];
          DivInstrumentESFM::Operator& opE=ins->esfm.op[i];

          ImGui::TableNextRow();
          ImGui::TableNextColumn();

          // push colors
          if (settings.separateFMColors) {
            bool mod=true;
            if (ins->type==DIV_INS_OPL_DRUMS) {
              mod=false;
            } else if (ins->type==DIV_INS_ESFM) {
              // this is the same as the KVS heuristic in platform/esfm.h
              if (opE.outLvl==7) mod=false;
              else if (opE.outLvl>0) {
                if (i==3) mod=false;
                else {
                  DivInstrumentESFM::Operator& opENext=ins->esfm.op[i+1];
                  if (opENext.modIn==0) mod=false;
                  else if ((opE.outLvl-opENext.modIn)>=2) mod=false;
                }
              }
            } else if (opCount==4) {
              if (ins->type==DIV_INS_OPL) {
                if (opIsOutputOPL[fmOrigin.alg&3][i]) mod=false;
              } else {
                if (opIsOutput[fmOrigin.alg&7][i]) mod=false;
              }
            } else {
              if (i==1 || (ins->type==DIV_INS_OPL && (fmOrigin.alg&1))) mod=false;
            }
            if (mod) {
              pushAccentColors(
                uiColors[GUI_COLOR_FM_PRIMARY_MOD],
                uiColors[GUI_COLOR_FM_SECONDARY_MOD],
                uiColors[GUI_COLOR_FM_BORDER_MOD],
                uiColors[GUI_COLOR_FM_BORDER_SHADOW_MOD]
              );
            } else {
              pushAccentColors(
                uiColors[GUI_COLOR_FM_PRIMARY_CAR],
                uiColors[GUI_COLOR_FM_SECONDARY_CAR],
                uiColors[GUI_COLOR_FM_BORDER_CAR],
                uiColors[GUI_COLOR_FM_BORDER_SHADOW_CAR]
              );
            }
          }

          if (i==0) {
            sliderHeight=(ImGui::GetContentRegionAvail().y/opCount)-ImGui::GetStyle().ItemSpacing.y;
            float sliderMinHeightOPL=ImGui::GetFrameHeight()*4.0+ImGui::GetStyle().ItemSpacing.y*3.0;
            float sliderMinHeightESFM=ImGui::GetFrameHeight()*5.0+ImGui::GetStyle().ItemSpacing.y*4.0;
            if ((ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_OPLL) && sliderHeight<sliderMinHeightOPL) {
              sliderHeight=sliderMinHeightOPL;
            }
            if (ins->type==DIV_INS_ESFM && sliderHeight<sliderMinHeightESFM) {
              sliderHeight=sliderMinHeightESFM;
            }
          }

          ImGui::PushID(fmt::sprintf("op%d",i).c_str());
          String opNameLabel;
          if (ins->type==DIV_INS_OPL_DRUMS) {
            opNameLabel=fmt::sprintf("%s",oplDrumNames[i]);
          } else if (ins->type==DIV_INS_OPL && fmOrigin.opllPreset==16) {
            if (i==1) {
              opNameLabel="Kick";
            } else {
              opNameLabel="Env";
            }
          } else {
            opNameLabel=fmt::sprintf("OP%d",i+1);
          }
          if (opsAreMutable) {
            pushToggleColors(op.enable);
            if (ImGui::Button(opNameLabel.c_str())) {
              op.enable=!op.enable;
              PARAMETER;
            }
            popToggleColors();
          } else {
            ImGui::TextUnformatted(opNameLabel.c_str());
          }

          // drag point
          OP_DRAG_POINT;

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
          bool ssgOn=op.ssgEnv&8;
          bool ksrOn=op.ksr;
          bool vibOn=op.vib;
          bool susOn=op.sus;
          bool fixedOn=opE.fixed;
          unsigned char ssgEnv=op.ssgEnv&7;

          if (ins->type==DIV_INS_ESFM) {
            ImGui::TableNextColumn();
            CENTER_VSLIDER;
            P(CWVSliderScalar("##MODIN",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&opE.modIn,&_ZERO,&_SEVEN)); rightClickable
            ImGui::TableNextColumn();
            ImGui::Dummy(ImVec2(4.0f*dpiScale,2.0f*dpiScale));
            ImGui::TableNextColumn();
            opE.delay&=7;
            CENTER_VSLIDER;
            P(CWVSliderScalar("##DELAY",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&opE.delay,&_ZERO,&_SEVEN)); rightClickable
          }

          ImGui::TableNextColumn();
          op.ar&=maxArDr;
          CENTER_VSLIDER;
          P(CWVSliderScalar("##AR",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.ar,&maxArDr,&_ZERO)); rightClickable

          ImGui::TableNextColumn();
          op.dr&=maxArDr;
          CENTER_VSLIDER;
          P(CWVSliderScalar("##DR",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.dr,&maxArDr,&_ZERO)); rightClickable

          if (settings.susPosition==0) {
            ImGui::TableNextColumn();
            op.sl&=15;
            CENTER_VSLIDER;
            P(CWVSliderScalar("##SL",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.sl,&_FIFTEEN,&_ZERO)); rightClickable
          }

          if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM) {
            ImGui::TableNextColumn();
            op.d2r&=31;
            CENTER_VSLIDER;
            P(CWVSliderScalar("##D2R",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.d2r,&_THIRTY_ONE,&_ZERO)); rightClickable
          }

          ImGui::TableNextColumn();
          op.rr&=15;
          CENTER_VSLIDER;
          P(CWVSliderScalar("##RR",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.rr,&_FIFTEEN,&_ZERO)); rightClickable

          if (settings.susPosition==1) {
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
          if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM) {
            P(CWVSliderScalar("##RS",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.rs,&_ZERO,&_THREE)); rightClickable
          } else {
            int ksl=ins->type==DIV_INS_OPLL?op.ksl:kslMap[op.ksl&3];
            if (CWVSliderInt("##KSL",ImVec2(20.0f*dpiScale,sliderHeight),&ksl,0,3)) {
              op.ksl=(ins->type==DIV_INS_OPLL?ksl:kslMap[ksl&3]);
              PARAMETER;
            } rightClickable
          }

          if (ins->type==DIV_INS_OPZ) {
            ImGui::TableNextColumn();
            CENTER_VSLIDER;
            P(CWVSliderScalar("##EGS",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.ksl,&_ZERO,&_THREE)); rightClickable

            ImGui::TableNextColumn();
            CENTER_VSLIDER;
            P(CWVSliderScalar("##REV",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.dam,&_ZERO,&_SEVEN)); rightClickable
          }

          if (ins->type==DIV_INS_ESFM) {
            ImGui::TableNextColumn();
            CENTER_VSLIDER;
            P(CWVSliderScalar("##OUTLVL",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&opE.outLvl,&_ZERO,&_SEVEN)); rightClickable
          }

          ImGui::TableNextColumn();
          CENTER_VSLIDER;
          P(CWVSliderScalar("##MULT",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.mult,&_ZERO,&_FIFTEEN)); rightClickable

          if (ins->type==DIV_INS_OPZ) {
            ImGui::TableNextColumn();
            CENTER_VSLIDER;
            P(CWVSliderScalar("##FINE",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.dvb,&_ZERO,&_FIFTEEN)); rightClickable
          }

          if (ins->type==DIV_INS_ESFM) {
            ImGui::TableNextColumn();
            CENTER_VSLIDER;
            P(CWVSliderScalar("##CT",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_S8,&opE.ct,&_MINUS_TWENTY_FOUR,&_TWENTY_FOUR)); rightClickable
            if (ImGui::IsItemHovered()) {
              ImGui::SetTooltip("Detune in semitones");
            }
          }

          if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM) {
            int detune=detuneMap[settings.unsignedDetune?1:0][op.dt&7];
            ImGui::TableNextColumn();
            CENTER_VSLIDER;
            if (CWVSliderInt("##DT",ImVec2(20.0f*dpiScale,sliderHeight),&detune,settings.unsignedDetune?0:-3,settings.unsignedDetune?7:4)) { PARAMETER
              if (detune<-3) detune=-3;
              if (detune>7) detune=7;
              op.dt=detuneUnmap[settings.unsignedDetune?1:0][detune+3];
            } rightClickable

            if (ins->type!=DIV_INS_FM) {
              ImGui::TableNextColumn();
              CENTER_VSLIDER;
              P(CWVSliderScalar("##DT2",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.dt2,&_ZERO,&_THREE)); rightClickable
            }

            ImGui::TableNextColumn();
            bool amOn=op.am;
            if (ins->type==DIV_INS_OPZ) {
              bool egtOn=op.egt;
              if (egtOn) {
                ImGui::SetCursorPosY(ImGui::GetCursorPosY()+0.5*(sliderHeight-ImGui::GetFrameHeight()*4.0-ImGui::GetStyle().ItemSpacing.y*3.0));
              } else {
                ImGui::SetCursorPosY(ImGui::GetCursorPosY()+0.5*(sliderHeight-ImGui::GetFrameHeight()*2.0-ImGui::GetStyle().ItemSpacing.y*1.0));
              }
              if (ImGui::Checkbox("AM",&amOn)) { PARAMETER
                op.am=amOn;
              }
              if (ImGui::Checkbox("Fixed",&egtOn)) { PARAMETER
                op.egt=egtOn;
              }
              if (egtOn) {
                int block=op.dt;
                int freqNum=(op.mult<<4)|(op.dvb&15);
                if (ImGui::InputInt("Block",&block,1,1)) {
                  if (block<0) block=0;
                  if (block>7) block=7;
                  op.dt=block;
                }
                if (ImGui::InputInt("FreqNum",&freqNum,1,16)) {
                  if (freqNum<0) freqNum=0;
                  if (freqNum>255) freqNum=255;
                  op.mult=freqNum>>4;
                  op.dvb=freqNum&15;
                }
              }
            } else {
              ImGui::SetCursorPosY(ImGui::GetCursorPosY()+0.5*(sliderHeight-ImGui::GetFrameHeight()));
              if (ImGui::Checkbox("##AM",&amOn)) { PARAMETER
                op.am=amOn;
              }
            }

            if (ins->type!=DIV_INS_OPL && ins->type!=DIV_INS_OPL_DRUMS && ins->type!=DIV_INS_OPZ && ins->type!=DIV_INS_OPM) {
              ImGui::TableNextColumn();
              ImGui::Dummy(ImVec2(4.0f*dpiScale,2.0f*dpiScale));
              ImGui::TableNextColumn();
              ImGui::BeginDisabled(!ssgOn);
              drawSSGEnv(op.ssgEnv&7,ImVec2(ImGui::GetContentRegionAvail().x,sliderHeight-ImGui::GetFrameHeightWithSpacing()));
              ImGui::EndDisabled();
              if (ImGui::Checkbox("##SSGOn",&ssgOn)) { PARAMETER
                op.ssgEnv=(op.ssgEnv&7)|(ssgOn<<3);
              }

              ImGui::SameLine();
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
              if (CWSliderScalar("##SSG",ImGuiDataType_U8,&ssgEnv,&_ZERO,&_SEVEN,ssgEnvTypes[ssgEnv])) { PARAMETER
                op.ssgEnv=(op.ssgEnv&8)|(ssgEnv&7);
              }
            }
          } else if (ins->type==DIV_INS_ESFM) {
            ImGui::TableNextColumn();
            CENTER_VSLIDER;
            P(CWVSliderScalar("##DT",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_S8,&opE.dt,&_MINUS_ONE_HUNDRED_TWENTY_EIGHT,&_ONE_HUNDRED_TWENTY_SEVEN)); rightClickable
            if (ImGui::IsItemHovered()) {
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

            if (ImGui::BeginTable("panCheckboxes",(fixedOn)?3:2,ImGuiTableFlags_SizingStretchProp)) {
              ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,1.0);
              ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,1.0);
              if (fixedOn) {
                ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,1.2);
              }

              float yCoordBeforeTablePadding=ImGui::GetCursorPosY();
              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              ImGui::SetCursorPosY(yCoordBeforeTablePadding);
              if (ImGui::Checkbox(ESFM_SHORT_NAME(ESFM_LEFT),&leftOn)) { PARAMETER
                opE.left=leftOn;
              }
              if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("If operator outputs sound, enable left channel output.");
              }
              ImGui::TableNextColumn();
              ImGui::SetCursorPosY(yCoordBeforeTablePadding);
              if (ImGui::Checkbox(ESFM_SHORT_NAME(ESFM_RIGHT),&rightOn)) { PARAMETER
                opE.right=rightOn;
              }
              if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("If operator outputs sound, enable right channel output.");
              }
              if (fixedOn) {
                ImGui::TableNextColumn();
                ImGui::SetCursorPosY(yCoordBeforeTablePadding);
                if (ImGui::Checkbox(FM_SHORT_NAME(FM_AM),&amOn)) { PARAMETER
                  op.am=amOn;
                }
              }
              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              if (ImGui::Checkbox(FM_SHORT_NAME(FM_KSR),&ksrOn)) { PARAMETER
                op.ksr=ksrOn;
              }
              ImGui::TableNextColumn();
              if (ImGui::Checkbox(FM_SHORT_NAME(FM_SUS),&susOn)) { PARAMETER
                op.sus=susOn;
              }
              if (fixedOn) {
                bool damOn=op.dam;
                ImGui::TableNextColumn();
                if (ImGui::Checkbox(FM_SHORT_NAME(FM_DAM),&damOn)) { PARAMETER
                  op.dam=damOn;
                }
              }
              ImGui::EndTable();
            }
            ImGui::SetCursorPosY(ImGui::GetCursorPosY()-0.5*ImGui::GetStyle().ItemSpacing.y);
            if (ImGui::Checkbox(ESFM_NAME(ESFM_FIXED),&fixedOn)) { PARAMETER
              opE.fixed=fixedOn;

              ins->std.get_op_macro(i)->op_get_macro(DIV_MACRO_OP_SSG, true)->vZoom = -1;
              ins->std.get_op_macro(i)->op_get_macro(DIV_MACRO_OP_DT, true)->vZoom = -1;
            }
            if (ins->type==DIV_INS_ESFM) {
              if (fixedOn) {
                int block=(opE.ct>>2)&7;
                int freqNum=((opE.ct&3)<<8)|((unsigned char)opE.dt);
                if (ImGui::InputInt("Block",&block,1,1)) {
                  if (block<0) block=0;
                  if (block>7) block=7;
                  opE.ct=(opE.ct&(~(7<<2)))|(block<<2);
                }
                if (ImGui::InputInt("FreqNum",&freqNum,1,16)) {
                  if (freqNum<0) freqNum=0;
                  if (freqNum>1023) freqNum=1023;
                  opE.dt=freqNum&0xff;
                  opE.ct=(opE.ct&(~3))|(freqNum>>8);
                }
              } else {
                if (ImGui::BeginTable("amVibCheckboxes",2,ImGuiTableFlags_SizingStretchSame)) {
                  ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.0);
                  ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.0);

                  float yCoordBeforeTablePadding=ImGui::GetCursorPosY();
                  ImGui::TableNextRow();
                  ImGui::TableNextColumn();
                  ImGui::SetCursorPosY(yCoordBeforeTablePadding);
                  if (ImGui::Checkbox(FM_SHORT_NAME(FM_AM),&amOn)) { PARAMETER
                    op.am=amOn;
                  }
                  ImGui::TableNextColumn();
                  ImGui::SetCursorPosY(yCoordBeforeTablePadding);
                  if (ImGui::Checkbox(FM_SHORT_NAME(FM_VIB),&vibOn)) { PARAMETER
                    op.vib=vibOn;
                  }
                  bool damOn=op.dam;
                  bool dvbOn=op.dvb;
                  ImGui::TableNextRow();
                  ImGui::TableNextColumn();
                  if (ImGui::Checkbox(FM_SHORT_NAME(FM_DAM),&damOn)) { PARAMETER
                    op.dam=damOn;
                  }
                  ImGui::TableNextColumn();
                  if (ImGui::Checkbox(FM_SHORT_NAME(FM_DVB),&dvbOn)) { PARAMETER
                    op.dvb=dvbOn;
                  }
                  ImGui::EndTable();
                }
              }
            }
          } else if (ins->type!=DIV_INS_OPM) {
            ImGui::TableNextColumn();
            bool amOn=op.am;
            ImGui::SetCursorPosY(ImGui::GetCursorPosY()+0.5*(sliderHeight-ImGui::GetFrameHeight()*4.0-ImGui::GetStyle().ItemSpacing.y*3.0));
            if (ImGui::Checkbox(FM_NAME(FM_AM),&amOn)) { PARAMETER
              op.am=amOn;
            }
            if (ImGui::Checkbox(FM_NAME(FM_VIB),&vibOn)) { PARAMETER
              op.vib=vibOn;
            }
            if (ImGui::Checkbox(FM_NAME(FM_KSR),&ksrOn)) { PARAMETER
              op.ksr=ksrOn;
            }
            if (ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS) {
              if (ImGui::Checkbox(FM_NAME(FM_SUS),&susOn)) { PARAMETER
                op.sus=susOn;
              }
            } else if (ins->type==DIV_INS_OPLL) {
              if (ImGui::Checkbox(FM_NAME(FM_EGS),&ssgOn)) { PARAMETER
                op.ssgEnv=(op.ssgEnv&7)|(ssgOn<<3);
              }
            }
          }

          if (ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_ESFM) {
            ImGui::TableNextColumn();
            ImGui::Dummy(ImVec2(4.0f*dpiScale,2.0f*dpiScale));
            ImGui::TableNextColumn();

            drawWaveform(op.ws&7,ins->type==DIV_INS_OPZ,ImVec2(ImGui::GetContentRegionAvail().x,sliderHeight-ImGui::GetFrameHeightWithSpacing()*((ins->type==DIV_INS_ESFM && fixedOn)?3.0f:1.0f)));
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            P(CWSliderScalar("##WS",ImGuiDataType_U8,&op.ws,&_ZERO,&_SEVEN,(ins->type==DIV_INS_OPZ)?opzWaveforms[op.ws&7]:(settings.oplStandardWaveNames?oplWaveformsStandard[op.ws&7]:oplWaveforms[op.ws&7]))); rightClickable
            if ((ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS) && ImGui::IsItemHovered()) {
              ImGui::SetTooltip("OPL2/3 only (last 4 waveforms are OPL3 only)");
            }
            if (ins->type==DIV_INS_ESFM && fixedOn) {
              if (ImGui::Checkbox(FM_SHORT_NAME(FM_VIB),&vibOn)) { PARAMETER
                op.vib=vibOn;
              }
              bool dvbOn=op.dvb;
              if (ImGui::Checkbox(FM_SHORT_NAME(FM_DVB),&dvbOn)) { PARAMETER
                op.dvb=dvbOn;
              }
            }
          } else if (ins->type==DIV_INS_OPLL || ins->type==DIV_INS_OPM) {
            ImGui::TableNextColumn();
            ImGui::Dummy(ImVec2(4.0f*dpiScale,2.0f*dpiScale));
          }

          ImGui::TableNextColumn();
          drawFMEnv(op.tl&maxTl,op.ar&maxArDr,op.dr&maxArDr,(ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_OPLL || ins->type==DIV_INS_ESFM)?((op.rr&15)*2):op.d2r&31,op.rr&15,op.sl&15,op.sus,op.ssgEnv&8,fmOrigin.alg,maxTl,maxArDr,15,ImVec2(ImGui::GetContentRegionAvail().x,sliderHeight),ins->type);

          if (settings.separateFMColors) {
            popAccentColors();
          }

          ImGui::PopID();
        }

        ImGui::EndTable();
      }
    } else if (settings.fmLayout>=4 && settings.fmLayout<=6) { // alternate
      int columns=2;
      switch (settings.fmLayout) {
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
      if (ImGui::BeginTable("AltFMOperators",columns,ImGuiTableFlags_SizingStretchSame|ImGuiTableFlags_BordersInner)) {
        for (int i=0; i<opCount; i++) {
          DivInstrumentFM::Operator& op=fmOrigin.op[(opCount==4 && ins->type!=DIV_INS_OPL_DRUMS && ins->type!=DIV_INS_ESFM)?opOrder[i]:i];
          DivInstrumentESFM::Operator& opE=ins->esfm.op[i];
          if ((settings.fmLayout!=6 && ((i+1)&1)) || i==0 || settings.fmLayout==5) ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::PushID(fmt::sprintf("op%d",i).c_str());

          // push colors
          if (settings.separateFMColors) {
            bool mod=true;
            if (ins->type==DIV_INS_OPL_DRUMS) {
              mod=false;
            } else if (ins->type==DIV_INS_ESFM) {
              // this is the same as the KVS heuristic in platform/esfm.h
              if (opE.outLvl==7) mod=false;
              else if (opE.outLvl>0) {
                if (i==3) mod=false;
                else {
                  DivInstrumentESFM::Operator& opENext=ins->esfm.op[i+1];
                  if (opENext.modIn==0) mod=false;
                  else if ((opE.outLvl-opENext.modIn)>=2) mod=false;
                }
              }
            } else if (opCount==4) {
              if (ins->type==DIV_INS_OPL) {
                if (opIsOutputOPL[fmOrigin.alg&3][i]) mod=false;
              } else {
                if (opIsOutput[fmOrigin.alg&7][i]) mod=false;
              }
            } else {
              if (i==1 || (ins->type==DIV_INS_OPL && (fmOrigin.alg&1))) mod=false;
            }
            if (mod) {
              pushAccentColors(
                uiColors[GUI_COLOR_FM_PRIMARY_MOD],
                uiColors[GUI_COLOR_FM_SECONDARY_MOD],
                uiColors[GUI_COLOR_FM_BORDER_MOD],
                uiColors[GUI_COLOR_FM_BORDER_SHADOW_MOD]
              );
            } else {
              pushAccentColors(
                uiColors[GUI_COLOR_FM_PRIMARY_CAR],
                uiColors[GUI_COLOR_FM_SECONDARY_CAR],
                uiColors[GUI_COLOR_FM_BORDER_CAR],
                uiColors[GUI_COLOR_FM_BORDER_SHADOW_CAR]
              );
            }
          }

          ImGui::Dummy(ImVec2(dpiScale,dpiScale));
          if (ins->type==DIV_INS_OPL_DRUMS) {
            snprintf(tempID,1024,"%s",oplDrumNames[i]);
          } else if (ins->type==DIV_INS_OPL && fmOrigin.opllPreset==16) {
            if (i==1) {
              snprintf(tempID,1024,"Envelope 2 (kick only)");
            } else {
              snprintf(tempID,1024,"Envelope");
            }
          } else {
            snprintf(tempID,1024,"Operator %d",i+1);
          }
          float nextCursorPosX=ImGui::GetCursorPosX()+0.5*(ImGui::GetContentRegionAvail().x-ImGui::CalcTextSize(tempID).x-(opsAreMutable?(ImGui::GetStyle().FramePadding.x*2.0f):0.0f));
          OP_DRAG_POINT;
          ImGui::SameLine();
          ImGui::SetCursorPosX(nextCursorPosX);
          if (opsAreMutable) {
            pushToggleColors(op.enable);
            if (ImGui::Button(tempID)) {
              op.enable=!op.enable;
              PARAMETER;
            }
            popToggleColors();
          } else {
            ImGui::TextUnformatted(tempID);
          }

          float sliderHeight=200.0f*dpiScale;
          float waveWidth=140.0*dpiScale*((ins->type==DIV_INS_ESFM)?0.85f:1.0f);
          float waveHeight=sliderHeight-ImGui::GetFrameHeightWithSpacing()*((ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPL || ins->type==DIV_INS_ESFM)?5.0f:4.5f);

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

          bool ssgOn=op.ssgEnv&8;
          bool ksrOn=op.ksr;
          bool vibOn=op.vib;
          bool egtOn=op.egt;
          bool susOn=op.sus; // yawn
          unsigned char ssgEnv=op.ssgEnv&7;

          ImGui::PushStyleVar(ImGuiStyleVar_CellPadding,oldPadding);
          if (ImGui::BeginTable("opParams",4,ImGuiTableFlags_BordersInnerV)) {
            ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed,waveWidth);
            ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthFixed);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            float textY=ImGui::GetCursorPosY();
            if (ins->type==DIV_INS_ESFM) {
              CENTER_TEXT_20(ESFM_SHORT_NAME(ESFM_DELAY));
              ImGui::TextUnformatted(ESFM_SHORT_NAME(ESFM_DELAY));
              TOOLTIP_TEXT(ESFM_LONG_NAME(ESFM_DELAY));
            } else {
              CENTER_TEXT_20(FM_SHORT_NAME(FM_AR));
              ImGui::TextUnformatted(FM_SHORT_NAME(FM_AR));
              TOOLTIP_TEXT(FM_NAME(FM_AR));
            }
            ImGui::TableNextColumn();
            if (ins->type==DIV_INS_FM) {
              ImGui::Text("SSG-EG");
            } else {
              ImGui::Text("Waveform");
            }
            ImGui::TableNextColumn();
            ImGui::Text("Envelope");
            ImGui::TableNextColumn();

            // A/D/S/R
            ImGui::TableNextColumn();

            if (ins->type==DIV_INS_ESFM) {
              opE.delay&=7;
              P(CWVSliderScalar("##DELAY",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&opE.delay,&_ZERO,&_SEVEN)); rightClickable
              ImGui::SameLine();
            }

            op.ar&=maxArDr;
            float textX_AR=ImGui::GetCursorPosX();
            P(CWVSliderScalar("##AR",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.ar,&maxArDr,&_ZERO)); rightClickable

            ImGui::SameLine();
            op.dr&=maxArDr;
            float textX_DR=ImGui::GetCursorPosX();
            P(CWVSliderScalar("##DR",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.dr,&maxArDr,&_ZERO)); rightClickable

            float textX_SL=0.0f;
            if (settings.susPosition==0) {
              ImGui::SameLine();
              op.sl&=15;
              textX_SL=ImGui::GetCursorPosX();
              P(CWVSliderScalar("##SL",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.sl,&_FIFTEEN,&_ZERO)); rightClickable
            }

            float textX_D2R=0.0f;
            if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM) {
              ImGui::SameLine();
              op.d2r&=31;
              textX_D2R=ImGui::GetCursorPosX();
              P(CWVSliderScalar("##D2R",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.d2r,&_THIRTY_ONE,&_ZERO)); rightClickable
            }

            ImGui::SameLine();
            op.rr&=15;
            float textX_RR=ImGui::GetCursorPosX();
            P(CWVSliderScalar("##RR",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.rr,&_FIFTEEN,&_ZERO)); rightClickable

            if (settings.susPosition==1) {
              ImGui::SameLine();
              op.sl&=15;
              textX_SL=ImGui::GetCursorPosX();
              P(CWVSliderScalar("##SL",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.sl,&_FIFTEEN,&_ZERO)); rightClickable
            }

            ImVec2 prevCurPos=ImGui::GetCursorPos();

            // labels
            if (ins->type==DIV_INS_ESFM) {
              ImGui::SetCursorPos(ImVec2(textX_AR,textY));
              CENTER_TEXT_20(FM_SHORT_NAME(FM_AR));
              ImGui::TextUnformatted(FM_SHORT_NAME(FM_AR));
              TOOLTIP_TEXT(FM_NAME(FM_AR));
            }

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

            if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM) {
              ImGui::SetCursorPos(ImVec2(textX_D2R,textY));
              CENTER_TEXT_20(FM_SHORT_NAME(FM_D2R));
              ImGui::TextUnformatted(FM_SHORT_NAME(FM_D2R));
              TOOLTIP_TEXT(FM_NAME(FM_D2R));
            }

            ImGui::SetCursorPos(prevCurPos);
            
            ImGui::TableNextColumn();
            switch (ins->type) {
              case DIV_INS_FM: {
                // SSG
                ImGui::BeginDisabled(!ssgOn);
                drawSSGEnv(op.ssgEnv&7,ImVec2(waveWidth,waveHeight));
                ImGui::EndDisabled();
                if (ImGui::Checkbox("##SSGOn",&ssgOn)) { PARAMETER
                  op.ssgEnv=(op.ssgEnv&7)|(ssgOn<<3);
                }

                ImGui::SameLine();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (CWSliderScalar("##SSG",ImGuiDataType_U8,&ssgEnv,&_ZERO,&_SEVEN,ssgEnvTypes[ssgEnv])) { PARAMETER
                  op.ssgEnv=(op.ssgEnv&8)|(ssgEnv&7);
                }
                
                // params
                ImGui::Separator();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_MULT));
                P(CWSliderScalar("##MULT",ImGuiDataType_U8,&op.mult,&_ZERO,&_FIFTEEN,tempID)); rightClickable

                int detune=detuneMap[settings.unsignedDetune?1:0][op.dt&7];
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_DT));
                if (CWSliderInt("##DT",&detune,settings.unsignedDetune?0:-3,settings.unsignedDetune?7:4,tempID)) { PARAMETER
                  if (detune<-3) detune=-3;
                  if (detune>7) detune=7;
                  op.dt=detuneUnmap[settings.unsignedDetune?1:0][detune+3];
                } rightClickable

                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_RS));
                P(CWSliderScalar("##RS",ImGuiDataType_U8,&op.rs,&_ZERO,&_THREE,tempID)); rightClickable

                break;
              }
              case DIV_INS_OPM: {
                drawWaveform(0,true,ImVec2(waveWidth,waveHeight));
                
                // params
                ImGui::Separator();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_MULT));
                P(CWSliderScalar("##MULT",ImGuiDataType_U8,&op.mult,&_ZERO,&_FIFTEEN,tempID)); rightClickable

                int detune=detuneMap[settings.unsignedDetune?1:0][op.dt&7];
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_DT));
                if (CWSliderInt("##DT",&detune,settings.unsignedDetune?0:-3,settings.unsignedDetune?7:4,tempID)) { PARAMETER
                  if (detune<-3) detune=-3;
                  if (detune>7) detune=7;
                  op.dt=detuneUnmap[settings.unsignedDetune?1:0][detune+3];
                } rightClickable

                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_DT2));
                P(CWSliderScalar("##DT2",ImGuiDataType_U8,&op.dt2,&_ZERO,&_THREE,tempID)); rightClickable

                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_RS));
                P(CWSliderScalar("##RS",ImGuiDataType_U8,&op.rs,&_ZERO,&_THREE,tempID)); rightClickable
                break;
              }
              case DIV_INS_OPLL:
                // waveform
                drawWaveform(i==0?(fmOrigin.ams&1):(fmOrigin.fms&1),ins->type==DIV_INS_OPZ,ImVec2(waveWidth,waveHeight));

                // params
                ImGui::Separator();
                if (ImGui::BeginTable("FMParamsInner",2)) {
                  ImGui::TableNextRow();
                  ImGui::TableNextColumn();
                  bool amOn=op.am;
                  if (ImGui::Checkbox(FM_NAME(FM_AM),&amOn)) { PARAMETER
                    op.am=amOn;
                  }
                  ImGui::TableNextColumn();
                  if (ImGui::Checkbox(FM_NAME(FM_KSR),&ksrOn)) { PARAMETER
                    op.ksr=ksrOn;
                  }

                  ImGui::TableNextRow();
                  ImGui::TableNextColumn();
                  if (ImGui::Checkbox(FM_NAME(FM_VIB),&vibOn)) { PARAMETER
                    op.vib=vibOn;
                  }
                  ImGui::TableNextColumn();
                  if (ImGui::Checkbox(FM_NAME(FM_EGS),&ssgOn)) { PARAMETER
                    op.ssgEnv=(op.ssgEnv&7)|(ssgOn<<3);
                  }
                  
                  ImGui::EndTable();
                }

                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_MULT));
                P(CWSliderScalar("##MULT",ImGuiDataType_U8,&op.mult,&_ZERO,&_FIFTEEN,tempID)); rightClickable

                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_KSL));
                P(CWSliderScalar("##KSL",ImGuiDataType_U8,&op.ksl,&_ZERO,&_THREE,tempID)); rightClickable

                break;
              case DIV_INS_OPL:
              case DIV_INS_OPL_DRUMS: {
                // waveform
                drawWaveform(op.ws&7,ins->type==DIV_INS_OPZ,ImVec2(waveWidth,waveHeight));
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                P(CWSliderScalar("##WS",ImGuiDataType_U8,&op.ws,&_ZERO,&_SEVEN,(ins->type==DIV_INS_OPZ)?opzWaveforms[op.ws&7]:(settings.oplStandardWaveNames?oplWaveformsStandard[op.ws&7]:oplWaveforms[op.ws&7]))); rightClickable
                if ((ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS) && ImGui::IsItemHovered()) {
                  ImGui::SetTooltip("OPL2/3 only (last 4 waveforms are OPL3 only)");
                }

                // params
                ImGui::Separator();
                if (ImGui::BeginTable("FMParamsInner",2)) {
                  ImGui::TableNextRow();
                  ImGui::TableNextColumn();
                  bool amOn=op.am;
                  if (ImGui::Checkbox(FM_NAME(FM_AM),&amOn)) { PARAMETER
                    op.am=amOn;
                  }
                  ImGui::TableNextColumn();
                  if (ImGui::Checkbox(FM_NAME(FM_KSR),&ksrOn)) { PARAMETER
                    op.ksr=ksrOn;
                  }

                  ImGui::TableNextRow();
                  ImGui::TableNextColumn();
                  if (ImGui::Checkbox(FM_NAME(FM_VIB),&vibOn)) { PARAMETER
                    op.vib=vibOn;
                  }
                  ImGui::TableNextColumn();
                  if (ImGui::Checkbox(FM_NAME(FM_SUS),&susOn)) { PARAMETER
                    op.sus=susOn;
                  }
                  
                  ImGui::EndTable();
                }

                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_MULT));
                P(CWSliderScalar("##MULT",ImGuiDataType_U8,&op.mult,&_ZERO,&_FIFTEEN,tempID)); rightClickable

                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_KSL));
                int ksl=kslMap[op.ksl&3];
                if (CWSliderInt("##KSL",&ksl,0,3,tempID)) {
                  op.ksl=kslMap[ksl&3];
                  PARAMETER;
                } rightClickable

                break;
              }
              case DIV_INS_OPZ: {
                // waveform
                drawWaveform(op.ws&7,ins->type==DIV_INS_OPZ,ImVec2(waveWidth,waveHeight));
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                P(CWSliderScalar("##WS",ImGuiDataType_U8,&op.ws,&_ZERO,&_SEVEN,(ins->type==DIV_INS_OPZ)?opzWaveforms[op.ws&7]:(settings.oplStandardWaveNames?oplWaveformsStandard[op.ws&7]:oplWaveforms[op.ws&7]))); rightClickable
                if ((ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS) && ImGui::IsItemHovered()) {
                  ImGui::SetTooltip("OPL2/3 only (last 4 waveforms are OPL3 only)");
                }

                // params
                ImGui::Separator();
                if (egtOn) {
                  int block=op.dt;
                  int freqNum=(op.mult<<4)|(op.dvb&15);
                  ImGui::Text("Block");
                  ImGui::SameLine();
                  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                  ImVec2 cursorAlign=ImGui::GetCursorPos();
                  if (ImGui::InputInt("##Block",&block,1,1)) {
                    if (block<0) block=0;
                    if (block>7) block=7;
                    op.dt=block;
                  }
                  
                  ImGui::Text("Freq");
                  ImGui::SameLine();
                  ImGui::SetCursorPos(ImVec2(cursorAlign.x,ImGui::GetCursorPosY()));
                  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                  if (ImGui::InputInt("##FreqNum",&freqNum,1,16)) {
                    if (freqNum<0) freqNum=0;
                    if (freqNum>255) freqNum=255;
                    op.mult=freqNum>>4;
                    op.dvb=freqNum&15;
                  }
                } else {
                  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                  snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_MULT));
                  P(CWSliderScalar("##MULT",ImGuiDataType_U8,&op.mult,&_ZERO,&_FIFTEEN,tempID)); rightClickable

                  int detune=detuneMap[settings.unsignedDetune?1:0][op.dt&7];
                  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                  snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_DT));
                  if (CWSliderInt("##DT",&detune,settings.unsignedDetune?0:-3,settings.unsignedDetune?7:4,tempID)) { PARAMETER
                    if (detune<-3) detune=-3;
                    if (detune>7) detune=7;
                    op.dt=detuneUnmap[settings.unsignedDetune?1:0][detune+3];
                  } rightClickable
                }

                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_DT2));
                P(CWSliderScalar("##DT2",ImGuiDataType_U8,&op.dt2,&_ZERO,&_THREE,tempID)); rightClickable
                if (ImGui::IsItemHovered()) {
                  ImGui::SetTooltip("Only on YM2151 (OPM)");
                }

                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_RS));
                P(CWSliderScalar("##RS",ImGuiDataType_U8,&op.rs,&_ZERO,&_THREE,tempID)); rightClickable
                break;
              }
              case DIV_INS_ESFM:
                // waveform
                drawWaveform(op.ws&7,ins->type==DIV_INS_OPZ,ImVec2(waveWidth,waveHeight));
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                P(CWSliderScalar("##WS",ImGuiDataType_U8,&op.ws,&_ZERO,&_SEVEN,(ins->type==DIV_INS_OPZ)?opzWaveforms[op.ws&7]:(settings.oplStandardWaveNames?oplWaveformsStandard[op.ws&7]:oplWaveforms[op.ws&7]))); rightClickable

                // params
                ImGui::Separator();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_MULT));
                P(CWSliderScalar("##MULT",ImGuiDataType_U8,&op.mult,&_ZERO,&_FIFTEEN,tempID)); rightClickable

                if (opE.fixed) {
                  int block=(opE.ct>>2)&7;
                  int freqNum=((opE.ct&3)<<8)|((unsigned char)opE.dt);
                  ImGui::Text("Blk");
                  if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Block");
                  }
                  ImGui::SameLine();
                  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                  //ImVec2 cursorAlign=ImGui::GetCursorPos();
                  if (ImGui::InputInt("##Block",&block,1,1)) {
                    if (block<0) block=0;
                    if (block>7) block=7;
                    opE.ct=(opE.ct&(~(7<<2)))|(block<<2);
                  }

                  ImGui::Text("F");
                  if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Frequency (F-Num)");
                  }
                  ImGui::SameLine();
                  //ImGui::SetCursorPos(ImVec2(cursorAlign.x,ImGui::GetCursorPosY()));
                  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                  if (ImGui::InputInt("##FreqNum",&freqNum,1,16)) {
                    if (freqNum<0) freqNum=0;
                    if (freqNum>1023) freqNum=1023;
                    opE.dt=freqNum&0xff;
                    opE.ct=(opE.ct&(~3))|(freqNum>>8);
                  }
                } else {
                  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                  snprintf(tempID,1024,"%s: %%d",ESFM_NAME(ESFM_CT));
                  P(CWSliderScalar("##CT",ImGuiDataType_S8,&opE.ct,&_MINUS_TWENTY_FOUR,&_TWENTY_FOUR,tempID)); rightClickable
                  if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Detune in semitones");
                  }

                  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                  snprintf(tempID,1024,"%s: %%d",ESFM_NAME(ESFM_DT));
                  P(CWSliderScalar("##DT",ImGuiDataType_S8,&opE.dt,&_MINUS_ONE_HUNDRED_TWENTY_EIGHT,&_ONE_HUNDRED_TWENTY_SEVEN,tempID)); rightClickable
                  if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Detune in fractions of semitone.\n128 = +1 semitone, -128 = -1 semitone.");
                  }
                }

                if (ImGui::BeginTable("panCheckboxes",2,ImGuiTableFlags_SizingStretchSame)) {
                  ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.0f);
                  ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.0f);

                  float yPosOutsideTablePadding=ImGui::GetCursorPosY();
                  bool leftOn=opE.left;
                  bool rightOn=opE.right;
                  ImGui::TableNextRow();
                  ImGui::TableNextColumn();
                  ImGui::SetCursorPosY(yPosOutsideTablePadding);
                  if (ImGui::Checkbox(ESFM_SHORT_NAME(ESFM_LEFT),&leftOn)) { PARAMETER
                    opE.left=leftOn;
                  }
                  if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("If operator outputs sound, enable left channel output.");
                  }
                  ImGui::TableNextColumn();
                  ImGui::SetCursorPosY(yPosOutsideTablePadding);
                  if (ImGui::Checkbox(ESFM_SHORT_NAME(ESFM_RIGHT),&rightOn)) { PARAMETER
                    opE.right=rightOn;
                  }
                  if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("If operator outputs sound, enable right channel output.");
                  }
                  ImGui::EndTable();
                }
                break;
              default:
                break;
            }

            ImGui::TableNextColumn();
            float envHeight=sliderHeight;//-ImGui::GetStyle().ItemSpacing.y*2.0f;
            if (ins->type==DIV_INS_OPZ) {
              envHeight-=ImGui::GetFrameHeightWithSpacing()*2.0f;
            }
            if (ins->type==DIV_INS_ESFM) {
              envHeight-=ImGui::GetFrameHeightWithSpacing()*3.0f;
            }
            drawFMEnv(op.tl&maxTl,op.ar&maxArDr,op.dr&maxArDr,(ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_OPLL || ins->type==DIV_INS_ESFM)?((op.rr&15)*2):op.d2r&31,op.rr&15,op.sl&15,op.sus,op.ssgEnv&8,fmOrigin.alg,maxTl,maxArDr,15,ImVec2(ImGui::GetContentRegionAvail().x,envHeight),ins->type);

            if (ins->type==DIV_INS_OPZ) {
              ImGui::Separator();
              if (ImGui::BeginTable("FMParamsInnerOPZ",2)) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                if (!egtOn) {
                  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                  snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_FINE));
                  P(CWSliderScalar("##FINE",ImGuiDataType_U8,&op.dvb,&_ZERO,&_FIFTEEN,tempID)); rightClickable
                }

                ImGui::TableNextColumn();
                bool amOn=op.am;
                if (ImGui::Checkbox(FM_NAME(FM_AM),&amOn)) { PARAMETER
                  op.am=amOn;
                }
                ImGui::SameLine();
                if (ImGui::Checkbox("Fixed",&egtOn)) { PARAMETER
                  op.egt=egtOn;
                }

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_EGSHIFT));
                P(CWSliderScalar("##EGShift",ImGuiDataType_U8,&op.ksl,&_ZERO,&_THREE,tempID)); rightClickable

                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_REV));
                P(CWSliderScalar("##REV",ImGuiDataType_U8,&op.dam,&_ZERO,&_SEVEN,tempID)); rightClickable

                ImGui::TableNextColumn();


                ImGui::EndTable();
              }
            }

            if (ins->type==DIV_INS_ESFM) {
              ImGui::Separator();
              if (ImGui::BeginTable("FMParamsInnerESFM",2)) {
                ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.64f);
                ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.36f);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_KSL));
                int ksl=kslMap[op.ksl&3];
                if (CWSliderInt("##KSL",&ksl,0,3,tempID)) {
                  op.ksl=kslMap[ksl&3];
                  PARAMETER;
                } rightClickable

                bool amOn=op.am;
                bool fixedOn=opE.fixed;
                ImGui::TableNextColumn();
                if (ImGui::Checkbox(FM_SHORT_NAME(FM_KSR),&ksrOn)) { PARAMETER
                  op.ksr=ksrOn;
                }
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                if (ImGui::BeginTable("vibAmCheckboxes",2)) {
                  ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.0f);
                  ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.0f);

                  float yPosOutsideTablePadding=ImGui::GetCursorPosY();
                  ImGui::TableNextRow();
                  ImGui::TableNextColumn();
                  ImGui::SetCursorPosY(yPosOutsideTablePadding);
                  if (ImGui::Checkbox(FM_SHORT_NAME(FM_VIB),&vibOn)) { PARAMETER
                    op.vib=vibOn;
                  }
                  ImGui::TableNextColumn();
                  ImGui::SetCursorPosY(yPosOutsideTablePadding);
                  if (ImGui::Checkbox(FM_SHORT_NAME(FM_AM),&amOn)) { PARAMETER
                    op.am=amOn;
                  }

                  bool damOn=op.dam;
                  bool dvbOn=op.dvb;
                  ImGui::TableNextRow();
                  ImGui::TableNextColumn();
                  if (ImGui::Checkbox(FM_SHORT_NAME(FM_DVB),&dvbOn)) { PARAMETER
                    op.dvb=dvbOn;
                  }
                  ImGui::TableNextColumn();
                  if (ImGui::Checkbox(FM_SHORT_NAME(FM_DAM),&damOn)) { PARAMETER
                    op.dam=damOn;
                  }
                  ImGui::EndTable();
                }
                ImGui::TableNextColumn();
                if (ImGui::Checkbox(FM_SHORT_NAME(FM_SUS),&susOn)) { PARAMETER
                  op.sus=susOn;
                }
                if (ImGui::Checkbox(ESFM_NAME(ESFM_FIXED),&fixedOn)) { PARAMETER
                  opE.fixed=fixedOn;

                  ins->std.get_op_macro(i)->op_get_macro(DIV_MACRO_OP_SSG, true)->vZoom = -1;
                  ins->std.get_op_macro(i)->op_get_macro(DIV_MACRO_OP_DT, true)->vZoom = -1;
                }

                ImGui::EndTable();
              }
            }

            ImGui::TableNextColumn();
            op.tl&=maxTl;
            float tlSliderWidth=(ins->type==DIV_INS_ESFM)?20.0f*dpiScale:ImGui::GetFrameHeight();
            float tlSliderHeight=sliderHeight-((ins->type==DIV_INS_FM || ins->type==DIV_INS_OPM)?(ImGui::GetFrameHeightWithSpacing()+ImGui::CalcTextSize(FM_SHORT_NAME(FM_AM)).y+ImGui::GetStyle().ItemSpacing.y):0.0f);
            float textX_tl=ImGui::GetCursorPosX();
            P(CWVSliderScalar("##TL",ImVec2(tlSliderWidth,tlSliderHeight),ImGuiDataType_U8,&op.tl,&maxTl,&_ZERO)); rightClickable

            if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPM) {
              CENTER_TEXT(FM_SHORT_NAME(FM_AM));
              ImGui::TextUnformatted(FM_SHORT_NAME(FM_AM));
              TOOLTIP_TEXT(FM_NAME(FM_AM));
              bool amOn=op.am;
              if (ImGui::Checkbox("##AM",&amOn)) { PARAMETER
                op.am=amOn;
              }
            }

            if (ins->type==DIV_INS_ESFM) {
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
            } else {
              prevCurPos=ImGui::GetCursorPos();
              ImGui::SetCursorPos(ImVec2(textX_tl,textY));
              CENTER_TEXT(FM_SHORT_NAME(FM_TL));
              ImGui::TextUnformatted(FM_SHORT_NAME(FM_TL));
              TOOLTIP_TEXT(FM_NAME(FM_TL));

              ImGui::SetCursorPos(prevCurPos);
            }

            ImGui::EndTable();
          }
          ImGui::PopStyleVar();

          if (settings.separateFMColors) {
            popAccentColors();
          }

          ImGui::PopID();
        }
        ImGui::EndTable();
      }
      ImGui::PopStyleVar();
    } else { // classic
      int columns=2;
      switch (settings.fmLayout) {
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
      if (ImGui::BeginTable("FMOperators",columns,ImGuiTableFlags_SizingStretchSame)) {
        for (int i=0; i<opCount; i++) {
          DivInstrumentFM::Operator& op=fmOrigin.op[(opCount==4 && ins->type!=DIV_INS_OPL_DRUMS && ins->type!=DIV_INS_ESFM)?opOrder[i]:i];
          DivInstrumentESFM::Operator& opE=ins->esfm.op[i];
          if ((settings.fmLayout!=3 && ((i+1)&1)) || i==0 || settings.fmLayout==2) ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::Separator();
          ImGui::PushID(fmt::sprintf("op%d",i).c_str());

          // push colors
          if (settings.separateFMColors) {
            bool mod=true;
            if (ins->type==DIV_INS_OPL_DRUMS) {
              mod=false;
            } else if (ins->type==DIV_INS_ESFM) {
              // this is the same as the KVS heuristic in platform/esfm.h
              if (opE.outLvl==7) mod=false;
              else if (opE.outLvl>0) {
                if (i==3) mod=false;
                else {
                  DivInstrumentESFM::Operator& opENext=ins->esfm.op[i+1];
                  if (opENext.modIn==0) mod=false;
                  else if ((opE.outLvl-opENext.modIn)>=2) mod=false;
                }
              }
            } else if (opCount==4) {
              if (ins->type==DIV_INS_OPL) {
                if (opIsOutputOPL[fmOrigin.alg&3][i]) mod=false;
              } else {
                if (opIsOutput[fmOrigin.alg&7][i]) mod=false;
              }
            } else {
              if (i==1 || (ins->type==DIV_INS_OPL && (fmOrigin.alg&1))) mod=false;
            }
            if (mod) {
              pushAccentColors(
                uiColors[GUI_COLOR_FM_PRIMARY_MOD],
                uiColors[GUI_COLOR_FM_SECONDARY_MOD],
                uiColors[GUI_COLOR_FM_BORDER_MOD],
                uiColors[GUI_COLOR_FM_BORDER_SHADOW_MOD]
              );
            } else {
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
          if (ins->type==DIV_INS_OPL_DRUMS) {
            opNameLabel=fmt::sprintf("%s",oplDrumNames[i]);
          } else if (ins->type==DIV_INS_OPL && fmOrigin.opllPreset==16) {
            if (i==1) {
              opNameLabel="Envelope 2 (kick only)";
            } else {
              opNameLabel="Envelope";
            }
          } else {
            opNameLabel=fmt::sprintf("OP%d",i+1);
          }
          if (opsAreMutable) {
            pushToggleColors(op.enable);
            if (ImGui::Button(opNameLabel.c_str())) {
              op.enable=!op.enable;
              PARAMETER;
            }
            popToggleColors();
          } else {
            ImGui::TextUnformatted(opNameLabel.c_str());
          }

          ImGui::SameLine();

          bool amOn=op.am;
          if (ImGui::Checkbox(FM_NAME(FM_AM),&amOn)) { PARAMETER
            op.am=amOn;
          }

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

          bool ssgOn=op.ssgEnv&8;
          bool ksrOn=op.ksr;
          bool vibOn=op.vib;
          bool susOn=op.sus; // don't you make fun of this one
          unsigned char ssgEnv=op.ssgEnv&7;
          if (ins->type!=DIV_INS_OPL && ins->type!=DIV_INS_OPL_DRUMS && ins->type!=DIV_INS_OPZ && ins->type!=DIV_INS_OPM && ins->type!=DIV_INS_ESFM) {
            ImGui::SameLine();
            if (ImGui::Checkbox((ins->type==DIV_INS_OPLL)?FM_NAME(FM_EGS):"SSG On",&ssgOn)) { PARAMETER
              op.ssgEnv=(op.ssgEnv&7)|(ssgOn<<3);
            }
          }

          if (ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_ESFM) {
            ImGui::SameLine();
            if (ImGui::Checkbox(FM_NAME(FM_SUS),&susOn)) { PARAMETER
              op.sus=susOn;
            }
          }

          if (ins->type==DIV_INS_OPZ) {
            ImGui::SameLine();
            bool fixedOn=op.egt;
            if (ImGui::Checkbox("Fixed",&fixedOn)) { PARAMETER
              op.egt=fixedOn;
            }
          }

          //52.0 controls vert scaling; default 96
          drawFMEnv(op.tl&maxTl,op.ar&maxArDr,op.dr&maxArDr,(ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_OPLL || ins->type==DIV_INS_ESFM)?((op.rr&15)*2):op.d2r&31,op.rr&15,op.sl&15,op.sus,op.ssgEnv&8,fmOrigin.alg,maxTl,maxArDr,15,ImVec2(ImGui::GetContentRegionAvail().x,52.0*dpiScale),ins->type);
          //P(CWSliderScalar(FM_NAME(FM_AR),ImGuiDataType_U8,&op.ar,&_ZERO,&_THIRTY_ONE)); rightClickable
          if (ImGui::BeginTable("opParams",2,ImGuiTableFlags_SizingStretchProp)) {
            ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.0); \
            ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed,0.0); \

            if (ins->type==DIV_INS_ESFM) {
              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
              opE.delay&=7;
              P(CWSliderScalar("##DELAY",ImGuiDataType_U8,&opE.delay,&_ZERO,&_SEVEN)); rightClickable
              ImGui::TableNextColumn();
              ImGui::Text("%s",ESFM_NAME(ESFM_DELAY));
            }

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

            if (settings.susPosition==0) {
              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
              P(CWSliderScalar("##SL",ImGuiDataType_U8,&op.sl,&_FIFTEEN,&_ZERO)); rightClickable
              ImGui::TableNextColumn();
              ImGui::Text("%s",FM_NAME(FM_SL));
            }

            if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM) {
              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
              P(CWSliderScalar("##D2R",ImGuiDataType_U8,&op.d2r,&_THIRTY_ONE,&_ZERO)); rightClickable
              ImGui::TableNextColumn();
              ImGui::Text("%s",FM_NAME(FM_D2R));
            }

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            P(CWSliderScalar("##RR",ImGuiDataType_U8,&op.rr,&_FIFTEEN,&_ZERO)); rightClickable
            ImGui::TableNextColumn();
            ImGui::Text("%s",FM_NAME(FM_RR));

            if (settings.susPosition==1) {
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
            if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM) {
              P(CWSliderScalar("##RS",ImGuiDataType_U8,&op.rs,&_ZERO,&_THREE)); rightClickable
              ImGui::TableNextColumn();
              ImGui::Text("%s",FM_NAME(FM_RS));
            } else {
              int ksl=ins->type==DIV_INS_OPLL?op.ksl:kslMap[op.ksl&3];
              if (CWSliderInt("##KSL",&ksl,0,3)) {
                op.ksl=(ins->type==DIV_INS_OPLL?ksl:kslMap[ksl&3]);
                PARAMETER;
              } rightClickable
              ImGui::TableNextColumn();
              ImGui::Text("%s",FM_NAME(FM_KSL));
            }

            if (ins->type==DIV_INS_OPZ) {
              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
              P(CWSliderScalar(FM_NAME(FM_EGSHIFT),ImGuiDataType_U8,&op.ksl,&_ZERO,&_THREE)); rightClickable
              ImGui::TableNextColumn();
              ImGui::Text("%s",FM_NAME(FM_EGSHIFT));

              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
              P(CWSliderScalar(FM_NAME(FM_REV),ImGuiDataType_U8,&op.dam,&_ZERO,&_SEVEN)); rightClickable
              ImGui::TableNextColumn();
              ImGui::Text("%s",FM_NAME(FM_REV));
            }

            if (ins->type==DIV_INS_OPZ) {
              if (op.egt) {
                int block=op.dt;
                int freqNum=(op.mult<<4)|(op.dvb&15);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (CWSliderInt(FM_NAME(FM_MULT),&block,0,7)) { PARAMETER
                  if (block<0) block=0;
                  if (block>7) block=7;
                  op.dt=block;
                } rightClickable
                ImGui::TableNextColumn();
                ImGui::Text("Block");

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (CWSliderInt(FM_NAME(FM_FINE),&freqNum,0,255)) { PARAMETER
                  if (freqNum<0) freqNum=0;
                  if (freqNum>255) freqNum=255;
                  op.mult=freqNum>>4;
                  op.dvb=freqNum&15;
                } rightClickable
                ImGui::TableNextColumn();
                ImGui::Text("FreqNum");
              } else {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                P(CWSliderScalar(FM_NAME(FM_MULT),ImGuiDataType_U8,&op.mult,&_ZERO,&_FIFTEEN)); rightClickable
                ImGui::TableNextColumn();
                ImGui::Text("%s",FM_NAME(FM_MULT));

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                P(CWSliderScalar(FM_NAME(FM_FINE),ImGuiDataType_U8,&op.dvb,&_ZERO,&_FIFTEEN)); rightClickable
                ImGui::TableNextColumn();
                ImGui::Text("%s",FM_NAME(FM_FINE));
              }
            } else {
              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
              P(CWSliderScalar(FM_NAME(FM_MULT),ImGuiDataType_U8,&op.mult,&_ZERO,&_FIFTEEN)); rightClickable
              ImGui::TableNextColumn();
              ImGui::Text("%s",FM_NAME(FM_MULT));
            }
            
            if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM) {
              if (!(ins->type==DIV_INS_OPZ && op.egt)) {
                int detune=detuneMap[settings.unsignedDetune?1:0][op.dt&7];
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (CWSliderInt("##DT",&detune,settings.unsignedDetune?0:-3,settings.unsignedDetune?7:4)) { PARAMETER
                  if (detune<-3) detune=-3;
                  if (detune>7) detune=7;
                  op.dt=detuneUnmap[settings.unsignedDetune?1:0][detune+3];
                } rightClickable
                ImGui::TableNextColumn();
                ImGui::Text("%s",FM_NAME(FM_DT));
              }

              if (ins->type!=DIV_INS_FM) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                P(CWSliderScalar("##DT2",ImGuiDataType_U8,&op.dt2,&_ZERO,&_THREE)); rightClickable
                ImGui::TableNextColumn();
                ImGui::Text("%s",FM_NAME(FM_DT2));
              }

              if (ins->type==DIV_INS_FM) { // OPN only
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (CWSliderScalar("##SSG",ImGuiDataType_U8,&ssgEnv,&_ZERO,&_SEVEN,ssgEnvTypes[ssgEnv])) { PARAMETER
                  op.ssgEnv=(op.ssgEnv&8)|(ssgEnv&7);
                } rightClickable
                ImGui::TableNextColumn();
                ImGui::Text("%s",FM_NAME(FM_SSG));
              }
            }

            if (ins->type==DIV_INS_ESFM) {
              bool fixedOn=opE.fixed;
              if (fixedOn) {
                int block=(opE.ct>>2)&7;
                int freqNum=((opE.ct&3)<<8)|((unsigned char)opE.dt);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::InputInt("##Block",&block,1,1)) {
                  if (block<0) block=0;
                  if (block>7) block=7;
                  opE.ct=(opE.ct&(~(7<<2)))|(block<<2);
                }
                ImGui::TableNextColumn();
                ImGui::Text("Block");
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::InputInt("##FreqNum",&freqNum,1,16)) {
                  if (freqNum<0) freqNum=0;
                  if (freqNum>1023) freqNum=1023;
                  opE.dt=freqNum&0xff;
                  opE.ct=(opE.ct&(~3))|(freqNum>>8);
                }
                ImGui::TableNextColumn();
                ImGui::Text("FreqNum");
              } else {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                P(CWSliderScalar("##CT",ImGuiDataType_S8,&opE.ct,&_MINUS_TWENTY_FOUR,&_TWENTY_FOUR)); rightClickable
                if (ImGui::IsItemHovered()) {
                  ImGui::SetTooltip("Detune in semitones");
                }
                ImGui::TableNextColumn();
                ImGui::Text("%s",ESFM_NAME(ESFM_CT));

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                P(CWSliderScalar("##DT",ImGuiDataType_S8,&opE.dt,&_MINUS_ONE_HUNDRED_TWENTY_EIGHT,&_ONE_HUNDRED_TWENTY_SEVEN)); rightClickable
                if (ImGui::IsItemHovered()) {
                  ImGui::SetTooltip("Detune in fractions of semitone.\n128 = +1 semitone, -128 = -1 semitone.");
                }
                ImGui::TableNextColumn();
                ImGui::Text("%s",ESFM_NAME(ESFM_DT));
              }

            }

            if (ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_ESFM) {
              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
              P(CWSliderScalar("##WS",ImGuiDataType_U8,&op.ws,&_ZERO,&_SEVEN,(ins->type==DIV_INS_OPZ)?opzWaveforms[op.ws&7]:(settings.oplStandardWaveNames?oplWaveformsStandard[op.ws&7]:oplWaveforms[op.ws&7]))); rightClickable
              if ((ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS) && ImGui::IsItemHovered()) {
                ImGui::SetTooltip("OPL2/3 only (last 4 waveforms are OPL3 only)");
              }
              ImGui::TableNextColumn();
              ImGui::Text("%s",FM_NAME(FM_WS));
            }

            if (ins->type==DIV_INS_ESFM) {
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
            }

            ImGui::EndTable();
          }

          if (ins->type==DIV_INS_OPLL || ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_ESFM) {
            if (ImGui::Checkbox(FM_NAME(FM_VIB),&vibOn)) { PARAMETER
              op.vib=vibOn;
            }
            ImGui::SameLine();
            if (ImGui::Checkbox(FM_NAME(FM_KSR),&ksrOn)) { PARAMETER
              op.ksr=ksrOn;
            }
          }

          if (ins->type==DIV_INS_ESFM) {
            bool dvbOn=op.dvb;
            bool damOn=op.dam;
            bool leftOn=opE.left;
            bool rightOn=opE.right;
            bool fixedOn=opE.fixed;
            if (ImGui::Checkbox(FM_NAME(FM_DVB),&dvbOn)) { PARAMETER
              op.dvb=dvbOn;
            }
            ImGui::SameLine();
            if (ImGui::Checkbox(FM_NAME(FM_DAM),&damOn)) { PARAMETER
              op.dam=damOn;
            }
            if (ImGui::Checkbox(ESFM_NAME(ESFM_LEFT),&leftOn)) { PARAMETER
              opE.left=leftOn;
            }
            if (ImGui::IsItemHovered()) {
              ImGui::SetTooltip("If operator outputs sound, enable left channel output.");
            }
            ImGui::SameLine();
            if (ImGui::Checkbox(ESFM_NAME(ESFM_RIGHT),&rightOn)) { PARAMETER
              opE.right=rightOn;
            }
            if (ImGui::IsItemHovered()) {
              ImGui::SetTooltip("If operator outputs sound, enable right channel output.");
            }
            ImGui::SameLine();
            if (ImGui::Checkbox(ESFM_NAME(ESFM_FIXED),&fixedOn)) { PARAMETER
              opE.fixed=fixedOn;

              ins->std.get_op_macro(i)->op_get_macro(DIV_MACRO_OP_SSG, true)->vZoom = -1;
              ins->std.get_op_macro(i)->op_get_macro(DIV_MACRO_OP_DT, true)->vZoom = -1;
            }
          }

          if (settings.separateFMColors) {
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
  if (ins->type!=DIV_INS_ESFM) {
    if (ImGui::BeginTabItem("FM Macros")) {
      if (ins->type==DIV_INS_OPLL) {
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_SUS),ins,DIV_MACRO_ALG,0xff,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_FB),ins,DIV_MACRO_FB,0xff,0,7,96,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_DC),ins,DIV_MACRO_FMS,0xff,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_DM),ins,DIV_MACRO_AMS,0xff,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
      } else {
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_ALG),ins,DIV_MACRO_ALG,0xff,0,7,96,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_FB),ins,DIV_MACRO_FB,0xff,0,7,96,uiColors[GUI_COLOR_MACRO_OTHER]));
        if (ins->type!=DIV_INS_OPL && ins->type!=DIV_INS_OPL_DRUMS) {
          if (ins->type==DIV_INS_OPZ) {
            // TODO: FMS2/AMS2 macros
            macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_FMS),ins,DIV_MACRO_FMS,0xff,0,7,96,uiColors[GUI_COLOR_MACRO_OTHER]));
            macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_AMS),ins,DIV_MACRO_AMS,0xff,0,3,48,uiColors[GUI_COLOR_MACRO_OTHER]));
          } else {
            macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_FMS),ins,DIV_MACRO_FMS,0xff,0,7,96,uiColors[GUI_COLOR_MACRO_OTHER]));
            macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_AMS),ins,DIV_MACRO_AMS,0xff,0,3,48,uiColors[GUI_COLOR_MACRO_OTHER]));
          }
        }
      }

      if (ins->type==DIV_INS_FM) {
        macroList.push_back(FurnaceGUIMacroDesc("LFO Speed",ins,DIV_MACRO_EX3,0xff,0,8,96,uiColors[GUI_COLOR_MACRO_OTHER]));
      }
      if (ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM) {
        macroList.push_back(FurnaceGUIMacroDesc("AM Depth",ins,DIV_MACRO_EX1,0xff,0,127,128,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc("PM Depth",ins,DIV_MACRO_EX2,0xff,0,127,128,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc("LFO Speed",ins,DIV_MACRO_EX3,0xff,0,255,128,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc("LFO Shape",ins,DIV_MACRO_WAVE,0xff,0,3,48,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,macroLFOWaves));
      }
      if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPM) {
        macroList.push_back(FurnaceGUIMacroDesc("OpMask",ins,DIV_MACRO_EX4,0xff,0,4,128,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,fmOperatorBits));
      } else if (ins->type==DIV_INS_OPZ) {
        macroList.push_back(FurnaceGUIMacroDesc("AM Depth 2",ins,DIV_MACRO_EX5,0xff,0,127,128,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc("PM Depth 2",ins,DIV_MACRO_EX6,0xff,0,127,128,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc("LFO2 Speed",ins,DIV_MACRO_EX7,0xff,0,255,128,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc("LFO2 Shape",ins,DIV_MACRO_EX8,0xff,0,3,48,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,macroLFOWaves));
      }

      for(int i = 0; i < (int)ins->std.macros.size(); i++) // reset macro zoom
      {
        ins->std.macros[i].vZoom = -1;
      }

      drawMacros(macroList,macroEditStateFM);
        ImGui::EndTabItem();
    }
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

  for (int i=0; i<opCount; i++) {
    if (ins->type==DIV_INS_OPL_DRUMS) {
      if (i>0) break;
      snprintf(label,31,"Operator Macros");
    } else {
      snprintf(label,31,"OP%d Macros",i+1);
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
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_TL),ins,(DivMacroType)DIV_MACRO_OP_TL,ordi,0,maxTl,128,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_AR),ins,(DivMacroType)DIV_MACRO_OP_AR,ordi,0,maxArDr,64,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_DR),ins,(DivMacroType)DIV_MACRO_OP_DR,ordi,0,maxArDr,64,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_SL),ins,(DivMacroType)DIV_MACRO_OP_SL,ordi,0,15,64,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_RR),ins,(DivMacroType)DIV_MACRO_OP_RR,ordi,0,15,64,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_KSL),ins,(DivMacroType)DIV_MACRO_OP_KSL,ordi,0,3,32,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_MULT),ins,(DivMacroType)DIV_MACRO_OP_MULT,ordi,0,15,64,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_WS),ins,(DivMacroType)DIV_MACRO_OP_WS,ordi,0,7,64,uiColors[GUI_COLOR_MACRO_OTHER]));

        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_AM),ins,(DivMacroType)DIV_MACRO_OP_AM,ordi,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_VIB),ins,(DivMacroType)DIV_MACRO_OP_VIB,ordi,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_KSR),ins,(DivMacroType)DIV_MACRO_OP_KSR,ordi,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_SUS),ins,(DivMacroType)DIV_MACRO_OP_SUS,ordi,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
      } else if (ins->type==DIV_INS_OPLL) {
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_TL),ins,(DivMacroType)DIV_MACRO_OP_TL,ordi,0,maxTl,128,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_AR),ins,(DivMacroType)DIV_MACRO_OP_AR,ordi,0,maxArDr,64,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_DR),ins,(DivMacroType)DIV_MACRO_OP_DR,ordi,0,maxArDr,64,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_SL),ins,(DivMacroType)DIV_MACRO_OP_SL,ordi,0,15,64,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_RR),ins,(DivMacroType)DIV_MACRO_OP_RR,ordi,0,15,64,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_KSL),ins,(DivMacroType)DIV_MACRO_OP_KSL,ordi,0,3,32,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_MULT),ins,(DivMacroType)DIV_MACRO_OP_MULT,ordi,0,15,64,uiColors[GUI_COLOR_MACRO_OTHER]));

        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_AM),ins,(DivMacroType)DIV_MACRO_OP_AM,ordi,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_VIB),ins,(DivMacroType)DIV_MACRO_OP_VIB,ordi,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_KSR),ins,(DivMacroType)DIV_MACRO_OP_KSR,ordi,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_EGS),ins,(DivMacroType)DIV_MACRO_OP_EGT,ordi,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
      } else if (ins->type==DIV_INS_ESFM) {
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
        if (ins->esfm.op[ordi].fixed) {
          macroList.push_back(FurnaceGUIMacroDesc("Block",ins,(DivMacroType)DIV_MACRO_OP_SSG,ordi,0,7,64,uiColors[GUI_COLOR_MACRO_OTHER],true));
          macroList.push_back(FurnaceGUIMacroDesc("FreqNum",ins,(DivMacroType)DIV_MACRO_OP_DT,ordi,0,1023,160,uiColors[GUI_COLOR_MACRO_OTHER]));
        } else {
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
      } else {
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_TL),ins,(DivMacroType)DIV_MACRO_OP_TL,ordi,0,maxTl,128,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_AR),ins,(DivMacroType)DIV_MACRO_OP_AR,ordi,0,maxArDr,64,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_DR),ins,(DivMacroType)DIV_MACRO_OP_DR,ordi,0,maxArDr,64,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_D2R),ins,(DivMacroType)DIV_MACRO_OP_D2R,ordi,0,31,64,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_RR),ins,(DivMacroType)DIV_MACRO_OP_RR,ordi,0,15,64,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_SL),ins,(DivMacroType)DIV_MACRO_OP_SL,ordi,0,15,64,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_RS),ins,(DivMacroType)DIV_MACRO_OP_RS,ordi,0,3,32,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_MULT),ins,(DivMacroType)DIV_MACRO_OP_MULT,ordi,0,15,64,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_DT),ins,(DivMacroType)DIV_MACRO_OP_DT,ordi,0,7,64,uiColors[GUI_COLOR_MACRO_OTHER]));
        if (ins->type==DIV_INS_OPM || ins->type==DIV_INS_OPZ) {
          macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_DT2),ins,(DivMacroType)DIV_MACRO_OP_DT2,ordi,0,3,32,uiColors[GUI_COLOR_MACRO_OTHER]));
        }
        macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_AM),ins,(DivMacroType)DIV_MACRO_OP_AM,ordi,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));

        if (ins->type==DIV_INS_FM) {
          macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_SSG),ins,(DivMacroType)DIV_MACRO_OP_SSG,ordi,0,4,64,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,ssgEnvBits));
        }
      }

      drawMacros(macroList,macroEditStateOP[ordi]);
      ImGui::PopID();
      ImGui::EndTabItem();
    }
  }
}