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

void FurnaceGUI::drawInsOPLL(DivInstrument* ins)
{
  opCount=2;
  opsAreMutable=false;

  if (ImGui::BeginTabItem("FM")) {
    DivInstrumentFM& fmOrigin=(ins->type==DIV_INS_OPLL && ins->fm.opllPreset>0 && ins->fm.opllPreset<16)?opllPreview:ins->fm;

    isPresentCount=0;
    memset(isPresent,0,4*sizeof(bool));
    for (int i=0; i<e->song.systemLen; i++) 
    {
      if (e->song.system[i]==DIV_SYSTEM_VRC7) 
      {
        isPresent[3]=true;
      } 
      else if (e->song.system[i]==DIV_SYSTEM_OPLL || e->song.system[i]==DIV_SYSTEM_OPLL_DRUMS) 
      {
        isPresent[(e->song.systemFlags[i].getInt("patchSet",0))&3]=true;
      }
    }
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

    if (ImGui::BeginTable("fmDetails",3,(ins->type==DIV_INS_ESFM)?ImGuiTableFlags_SizingStretchProp:ImGuiTableFlags_SizingStretchSame)) {
      ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.0f);
      ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.0f);
      ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.0f);

      ImGui::TableNextRow();
      
      bool dc=fmOrigin.fms;
      bool dm=fmOrigin.ams;
      bool sus=ins->fm.alg;
      ImGui::TableNextColumn();
      ImGui::BeginDisabled(ins->fm.opllPreset!=0);
      P(CWSliderScalar(FM_NAME(FM_FB),ImGuiDataType_U8,&fmOrigin.fb,&_ZERO,&_SEVEN)); rightClickable
      if (ImGui::Checkbox(FM_NAME(FM_DC),&dc)) 
      { PARAMETER
        fmOrigin.fms=dc;
      }
      ImGui::EndDisabled();
      ImGui::TableNextColumn();
      if (ImGui::Checkbox(FM_NAME(FM_SUS),&sus)) 
      { PARAMETER
        ins->fm.alg=sus;
      }
      ImGui::BeginDisabled(ins->fm.opllPreset!=0);
      if (ImGui::Checkbox(FM_NAME(FM_DM),&dm)) 
      { PARAMETER
        fmOrigin.ams=dm;
      }
      ImGui::EndDisabled();
      ImGui::TableNextColumn();
      if (fmPreviewOn) 
      {
        drawFMPreview(ImVec2(ImGui::GetContentRegionAvail().x,24.0*dpiScale));
        if (!fmPreviewPaused) 
        {
          renderFMPreview(ins,1);
          WAKE_UP;
        }
      } 
      else 
      {
        drawAlgorithm(0,FM_ALGS_2OP_OPL,ImVec2(ImGui::GetContentRegionAvail().x,24.0*dpiScale));
      }
      kvsConfig(ins,false);

      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

      if (ImGui::BeginCombo("##LLPreset",opllInsNames[presentWhich][ins->fm.opllPreset])) 
      {
        if (isPresentCount>1) 
        {
          if (ImGui::BeginTable("LLPresetList",isPresentCount)) 
          {
            ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
            for (int i=0; i<4; i++) 
            {
              if (!isPresent[i]) continue;
              ImGui::TableNextColumn();
              ImGui::Text("%s name",opllVariants[i]);
            }

            for (int i=0; i<17; i++) 
            {
              ImGui::TableNextRow();
              for (int j=0; j<4; j++) {
                if (!isPresent[j]) continue;
                ImGui::TableNextColumn();
                ImGui::PushID(j*17+i);
                if (ImGui::Selectable(opllInsNames[j][i])) 
                {
                  ins->fm.opllPreset=i;
                }
                ImGui::PopID();
              }
            }
            ImGui::EndTable();
          }
        } 
        else 
        {
          for (int i=0; i<17; i++) 
          {
            if (ImGui::Selectable(opllInsNames[presentWhich][i])) 
            {
              ins->fm.opllPreset=i;
            }
          }
        }
        ImGui::EndCombo();
      }
      ImGui::EndTable();
    }

    if (ins->fm.opllPreset==16) 
    {
      ins->fm.ops=2;
      P(ImGui::Checkbox("Fixed frequency mode",&ins->fm.fixedDrums));
      if (ImGui::IsItemHovered()) 
      {
        ImGui::SetTooltip("when enabled, drums will be set to the specified frequencies, ignoring the note.");
      }
      if (ins->fm.fixedDrums) 
      {
        int block=0;
        int fNum=0;
        if (ImGui::BeginTable("fixedDrumSettings",3)) 
        {
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
      if (ins->fm.opllPreset==16) 
      {
        ImGui::Text("this volume slider only works in compatibility (non-drums) system.");
      }

      // update OPLL preset preview
      if (ins->fm.opllPreset>0 && ins->fm.opllPreset<16) 
      {
        const opll_patch_t* patchROM=NULL;

        switch (presentWhich) 
        {
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

        for (int i=0; i<2; i++) 
        {
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
    if (settings.fmLayout==0)
    {
      int numCols=12;
      if (ImGui::BeginTable("FMOperators",numCols,ImGuiTableFlags_SizingStretchProp|ImGuiTableFlags_BordersH|ImGuiTableFlags_BordersOuterV)) 
      {
        // configure columns
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed); // op name
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.05f); // ar
        ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.05f); // dr
        ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthStretch,0.05f); // sl
        ImGui::TableSetupColumn("c5",ImGuiTableColumnFlags_WidthStretch,0.05f); // rr
        ImGui::TableSetupColumn("c6",ImGuiTableColumnFlags_WidthFixed); // -separator-
        ImGui::TableSetupColumn("c7",ImGuiTableColumnFlags_WidthStretch,0.05f); // tl
        ImGui::TableSetupColumn("c8",ImGuiTableColumnFlags_WidthStretch,0.05f); // rs/ksl
        ImGui::TableSetupColumn("c9",ImGuiTableColumnFlags_WidthStretch,0.05f); // mult
        ImGui::TableSetupColumn("c15",ImGuiTableColumnFlags_WidthFixed); // am
        ImGui::TableSetupColumn("c12",ImGuiTableColumnFlags_WidthFixed); // -separator-
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
        CENTER_TEXT(FM_SHORT_NAME(FM_MULT));
        ImGui::TextUnformatted(FM_SHORT_NAME(FM_MULT));
        TOOLTIP_TEXT(FM_NAME(FM_MULT));
        ImGui::TableNextColumn();
        CENTER_TEXT("Other");
        ImGui::TextUnformatted("Other");
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        CENTER_TEXT("Envelope");
        ImGui::TextUnformatted("Envelope");

        float sliderHeight=32.0f*dpiScale;

        for (int i=0; i<opCount; i++) 
        {
          DivInstrumentFM::Operator& op=fmOrigin.op[i];

          ImGui::TableNextRow();
          ImGui::TableNextColumn();

          // push colors
          if (settings.separateFMColors) {
            bool mod=true;
            if (i==1) mod=false;

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
            float sliderMinHeightOPL=ImGui::GetFrameHeight()*4.0+ImGui::GetStyle().ItemSpacing.y*3.0;
            if (sliderHeight<sliderMinHeightOPL) 
            {
              sliderHeight=sliderMinHeightOPL;
            }
          }

          ImGui::PushID(fmt::sprintf("op%d",i).c_str());
          String opNameLabel;
          opNameLabel=fmt::sprintf("OP%d",i+1);
          ImGui::TextUnformatted(opNameLabel.c_str());

          // drag point
          OP_DRAG_POINT;

          int maxTl=0;

          if (i==1) 
          {
            maxTl=15;
          } 
          else 
          {
            maxTl=63;
          }
          
          int maxArDr=15;
          bool ksrOn=op.ksr;
          bool vibOn=op.vib;
          bool ssgOn=op.ssgEnv&8;

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
          int ksl=op.ksl;
          if (CWVSliderInt("##KSL",ImVec2(20.0f*dpiScale,sliderHeight),&ksl,0,3)) 
          {
            op.ksl=ksl;
            PARAMETER;
          } rightClickable

          ImGui::TableNextColumn();
          CENTER_VSLIDER;
          P(CWVSliderScalar("##MULT",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.mult,&_ZERO,&_FIFTEEN)); rightClickable

          ImGui::TableNextColumn();
          bool amOn=op.am;
          ImGui::SetCursorPosY(ImGui::GetCursorPosY()+0.5*(sliderHeight-ImGui::GetFrameHeight()*4.0-ImGui::GetStyle().ItemSpacing.y*3.0));
          if (ImGui::Checkbox(FM_NAME(FM_AM),&amOn)) 
          { PARAMETER
            op.am=amOn;
          }
          if (ImGui::Checkbox(FM_NAME(FM_VIB),&vibOn)) 
          { PARAMETER
            op.vib=vibOn;
          }
          if (ImGui::Checkbox(FM_NAME(FM_KSR),&ksrOn)) 
          { PARAMETER
            op.ksr=ksrOn;
          }
          if (ImGui::Checkbox(FM_NAME(FM_EGS),&ssgOn)) 
          { PARAMETER
            op.ssgEnv=(op.ssgEnv&7)|(ssgOn<<3);
          }

          ImGui::TableNextColumn();
          ImGui::Dummy(ImVec2(4.0f*dpiScale,2.0f*dpiScale));

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
          if ((settings.fmLayout!=6 && ((i+1)&1)) || i==0 || settings.fmLayout==5) ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::PushID(fmt::sprintf("op%d",i).c_str());

          // push colors
          if (settings.separateFMColors) 
          {
            bool mod=true;
            if (i==1 || (fmOrigin.alg&1)) mod=false;

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
          float waveHeight=sliderHeight-ImGui::GetFrameHeightWithSpacing()*((ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPL || ins->type==DIV_INS_ESFM)?5.0f:4.5f);

          int maxTl=0;
          if (ins->type==DIV_INS_OPLL) 
          {
            if (i==1) 
            {
              maxTl=15;
            } 
            else 
            {
              maxTl=63;
            }
          }
          int maxArDr=15;

          bool ssgOn=op.ssgEnv&8;
          bool ksrOn=op.ksr;
          bool vibOn=op.vib;

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
            ImGui::Text("Waveform");
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

            ImGui::SetCursorPos(prevCurPos);
            
            ImGui::TableNextColumn();
            
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

            ImGui::TableNextColumn();
            float envHeight=sliderHeight;//-ImGui::GetStyle().ItemSpacing.y*2.0f;
            drawFMEnv(op.tl&maxTl,op.ar&maxArDr,op.dr&maxArDr,((op.rr&15)*2),op.rr&15,op.sl&15,op.sus,op.ssgEnv&8,fmOrigin.alg,maxTl,maxArDr,15,ImVec2(ImGui::GetContentRegionAvail().x,envHeight),ins->type);

            ImGui::TableNextColumn();
            op.tl&=maxTl;
            float tlSliderWidth=(ins->type==DIV_INS_ESFM)?20.0f*dpiScale:ImGui::GetFrameHeight();
            float tlSliderHeight=sliderHeight-((ins->type==DIV_INS_FM || ins->type==DIV_INS_OPM)?(ImGui::GetFrameHeightWithSpacing()+ImGui::CalcTextSize(FM_SHORT_NAME(FM_AM)).y+ImGui::GetStyle().ItemSpacing.y):0.0f);
            float textX_tl=ImGui::GetCursorPosX();
            P(CWVSliderScalar("##TL",ImVec2(tlSliderWidth,tlSliderHeight),ImGuiDataType_U8,&op.tl,&maxTl,&_ZERO)); rightClickable

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
        for (int i=0; i<opCount; i++) {
          DivInstrumentFM::Operator& op=fmOrigin.op[i];
          if ((settings.fmLayout!=3 && ((i+1)&1)) || i==0 || settings.fmLayout==2) ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::Separator();
          ImGui::PushID(fmt::sprintf("op%d",i).c_str());

          // push colors
          if (settings.separateFMColors) {
            bool mod=true;
            if (i==1 || (fmOrigin.alg&1)) mod=false;
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

          ImGui::TextUnformatted(opNameLabel.c_str());

          ImGui::SameLine();

          bool amOn=op.am;
          if (ImGui::Checkbox(FM_NAME(FM_AM),&amOn)) 
          { PARAMETER
            op.am=amOn;
          }

          int maxTl=0;
          if (ins->type==DIV_INS_OPLL)
          {
            if (i==1) 
            {
              maxTl=15;
            } 
            else 
            {
              maxTl=63;
            }
          }

          int maxArDr=15;

          bool ssgOn=op.ssgEnv&8;
          bool ksrOn=op.ksr;
          bool vibOn=op.vib;

          ImGui::SameLine();
          if (ImGui::Checkbox((ins->type==DIV_INS_OPLL)?FM_NAME(FM_EGS):"SSG On",&ssgOn)) 
          { PARAMETER
            op.ssgEnv=(op.ssgEnv&7)|(ssgOn<<3);
          }

          //52.0 controls vert scaling; default 96
          drawFMEnv(op.tl&maxTl,op.ar&maxArDr,op.dr&maxArDr,((op.rr&15)*2),op.rr&15,op.sl&15,op.sus,op.ssgEnv&8,fmOrigin.alg,maxTl,maxArDr,15,ImVec2(ImGui::GetContentRegionAvail().x,52.0*dpiScale),ins->type);
          //P(CWSliderScalar(FM_NAME(FM_AR),ImGuiDataType_U8,&op.ar,&_ZERO,&_THIRTY_ONE)); rightClickable
          if (ImGui::BeginTable("opParams",2,ImGuiTableFlags_SizingStretchProp)) {
            ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.0); \
            ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed,0.0); \

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
            if (CWSliderInt("##KSL",&ksl,0,3)) {
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

  if (ImGui::BeginTabItem("FM Macros")) 
  {
    macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_SUS),ins,DIV_MACRO_ALG,0xff,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
    macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_FB),ins,DIV_MACRO_FB,0xff,0,7,96,uiColors[GUI_COLOR_MACRO_OTHER]));
    macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_DC),ins,DIV_MACRO_FMS,0xff,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
    macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_DM),ins,DIV_MACRO_AMS,0xff,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));

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

  for (int i=0; i<opCount; i++) {
    snprintf(label,31,"OP%d Macros",i+1);

    if (ImGui::BeginTabItem(label)) 
    {
      ImGui::PushID(i);
      int ordi=i;
      int maxTl=0;

      if (i==1) 
      {
        maxTl=15;
      } 
      else 
      {
        maxTl=63;
      }
      int maxArDr=15;

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

      drawMacros(macroList,macroEditStateOP[ordi]);
      ImGui::PopID();
      ImGui::EndTabItem();
    }
  }

  if (ImGui::BeginTabItem("Macros")) 
  {
    volMax=63;
    waveLabel="Patch";
    waveMax=15;

    macroList.push_back(FurnaceGUIMacroDesc(volumeLabel,ins,DIV_MACRO_VOL,0xff,volMin,volMax,160,uiColors[GUI_COLOR_MACRO_VOLUME]));
    macroList.push_back(FurnaceGUIMacroDesc("Arpeggio",ins,DIV_MACRO_ARP,0xff,-120,120,160,uiColors[GUI_COLOR_MACRO_PITCH],true,NULL,macroHoverNote,false,NULL,0,true,ins->std.get_macro(DIV_MACRO_ARP, true)->val));
    macroList.push_back(FurnaceGUIMacroDesc("Pitch",ins,DIV_MACRO_PITCH,0xff,-2048,2047,160,uiColors[GUI_COLOR_MACRO_PITCH],true,macroRelativeMode));
    macroList.push_back(FurnaceGUIMacroDesc(waveLabel, ins, DIV_MACRO_WAVE, 0xff, 0, waveMax, 160, uiColors[GUI_COLOR_MACRO_WAVE], false, NULL, NULL));

    macroList.push_back(FurnaceGUIMacroDesc("Phase Reset",ins,DIV_MACRO_PHASE_RESET,0xff,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));

    drawMacros(macroList,macroEditStateMacros);
    ImGui::EndTabItem();
  }
}