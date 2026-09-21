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
#include "../intConst.h"
#include "../util.h"
#include "../engine/platform/sound/sid3.h"

static const char* filtModeBits[5]={
  _N("low"),
  _N("band"),
  _N("high"),
  _N("ch3off"),
  NULL
};

static const char* minModModeBits[3]={
  _N("invert right"),
  _N("invert left"),
  NULL
};

static const char* sid3ControlBits[4]={
  _N("phase"),
  _N("sync"),
  _N("ring"),
  NULL
};

static const char* sid3WaveMixModes[6]={
  _N("8580 SID"),
  _N("Bitwise AND"),
  _N("Bitwise OR"),
  _N("Bitwise XOR"),
  _N("Sum of the signals"),
  NULL
};

static const char* sid3SpecialWaveforms[]={
  _N("Sine"),
  _N("Rect. Sine"),
  _N("Abs. Sine"),
  _N("Quart. Sine"),
  _N("Squish. Sine"),
  _N("Abs. Squish. Sine"),

  _N("Rect. Saw"),
  _N("Abs. Saw"),

  _N("Cubed Saw"),
  _N("Rect. Cubed Saw"),
  _N("Abs. Cubed Saw"),

  _N("Cubed Sine"),
  _N("Rect. Cubed Sine"),
  _N("Abs. Cubed Sine"),
  _N("Quart. Cubed Sine"),
  _N("Squish. Cubed Sine"),
  _N("Squish. Abs. Cub. Sine"),

  _N("Rect. Triangle"),
  _N("Abs. Triangle"),
  _N("Quart. Triangle"),
  _N("Squish. Triangle"),
  _N("Abs. Squish. Triangle"),

  _N("Cubed Triangle"),
  _N("Rect. Cubed Triangle"),
  _N("Abs. Cubed Triangle"),
  _N("Quart. Cubed Triangle"),
  _N("Squish. Cubed Triangle"),
  _N("Squish. Abs. Cub. Triangle"),

  // clipped

  _N("Clipped Sine"),
  _N("Clipped Rect. Sine"),
  _N("Clipped Abs. Sine"),
  _N("Clipped Quart. Sine"),
  _N("Clipped Squish. Sine"),
  _N("Clipped Abs. Squish. Sine"),

  _N("Clipped Rect. Saw"),
  _N("Clipped Abs. Saw"),

  _N("Clipped Cubed Saw"),
  _N("Clipped Rect. Cubed Saw"),
  _N("Clipped Abs. Cubed Saw"),

  _N("Clipped Cubed Sine"),
  _N("Clipped Rect. Cubed Sine"),
  _N("Clipped Abs. Cubed Sine"),
  _N("Clipped Quart. Cubed Sine"),
  _N("Clipped Squish. Cubed Sine"),
  _N("Clipped Squish. Abs. Cub. Sine"),

  _N("Clipped Rect. Triangle"),
  _N("Clipped Abs. Triangle"),
  _N("Clipped Quart. Triangle"),
  _N("Clipped Squish. Triangle"),
  _N("Clipped Abs. Squish. Triangle"),

  _N("Clipped Cubed Triangle"),
  _N("Clipped Rect. Cubed Triangle"),
  _N("Clipped Abs. Cubed Triangle"),
  _N("Clipped Quart. Cubed Triangle"),
  _N("Clipped Squish. Cubed Triangle"),
  _N("Clipped Squish. Abs. Cub. Triangle"),

  // two clipped simple waves

  _N("Clipped Triangle"),
  _N("Clipped Saw")
};

static const char* sid3ShapeBits[6]={
  _N("triangle"),
  _N("saw"),
  _N("pulse"),
  _N("noise"),
  _N("special wave"),
  NULL
};

static const char* sid3FilterMatrixBits[5]={
  _N("From filter 1"),
  _N("From filter 2"),
  _N("From filter 3"),
  _N("From filter 4"),
  NULL
};

const int _SID3_SPECIAL_WAVES=SID3_NUM_SPECIAL_WAVES-1;
const int _SID3_NUM_CHANNELS=SID3_NUM_CHANNELS;
const int _SID3_NUM_CHANNELS_MINUS_ONE=SID3_NUM_CHANNELS-1;

typedef double (*WaveFunc) (double a);

WaveFunc waveFuncsIns[]={
  sinus,
  rectSin,
  absSin,
  quartSin,
  squiSin,
  squiAbsSin,
  
  rectSaw,
  absSaw,
  
  cubSaw,
  rectCubSaw,
  absCubSaw,
  
  cubSine,
  rectCubSin,
  absCubSin,
  quartCubSin,
  squishCubSin,
  squishAbsCubSin,

  rectTri,
  absTri,
  quartTri,
  squiTri,
  absSquiTri,

  cubTriangle,
  cubRectTri,
  cubAbsTri,
  cubQuartTri,
  cubSquiTri,
  absCubSquiTri
};


String macroSID3SpecialWaves(int id, float val, void* u) {
  if ((int)val<0 || (int)val>=SID3_NUM_SPECIAL_WAVES) return "???";

  return fmt::sprintf("%d: %s",id,_(sid3SpecialWaveforms[(int)val%SID3_NUM_SPECIAL_WAVES]));
}

String macroSID3SourceChan(int id, float val, void* u) {
  if ((int)val>SID3_NUM_CHANNELS) return "???";

  if ((int)val==SID3_NUM_CHANNELS) {
    return _("Self");
  } else if ((int)val==SID3_NUM_CHANNELS-1) {
    return _("PCM/Wave channel");
  } else {
    return fmt::sprintf(_("Channel %d"),(int)val+1);
  }
}

String macroSID3NoiseLFSR(int id, float val, void* u) {
  return _(
    "values close to SID2 noise modes:\n\n"
    "Mode 1: 524288\n"
    "Mode 2: 66\n"
    "Mode 3: 541065280"
  );
}

String macroSID3WaveMixMode(int id, float val, void* u) {
  if ((int)val<0 || (int)val>4) return "???";

  return fmt::sprintf("%d: %s",id,_(sid3WaveMixModes[(int)val]));
}

void FurnaceGUI::drawSID3Env(unsigned char tl, unsigned char ar, unsigned char dr, unsigned char d2r, unsigned char rr, unsigned char sl, unsigned char sus, unsigned char egt, unsigned char algOrGlobalSus, float maxTl, float maxArDr, float maxRr, const ImVec2& size, unsigned short instType) {
  ImDrawList* dl=ImGui::GetWindowDrawList();
  ImGuiWindow* window=ImGui::GetCurrentWindow();

  ImVec2 minArea=window->DC.CursorPos;
  ImVec2 maxArea=ImVec2(
    minArea.x+size.x,
    minArea.y+size.y
  );
  ImRect rect=ImRect(minArea,maxArea);
  ImGuiStyle& style=ImGui::GetStyle();
  ImU32 color=ImGui::GetColorU32(uiColors[GUI_COLOR_FM_ENVELOPE]);
  ImU32 colorR=ImGui::GetColorU32(uiColors[GUI_COLOR_FM_ENVELOPE_RELEASE]); // Relsease triangle
  ImU32 colorS=ImGui::GetColorU32(uiColors[GUI_COLOR_FM_ENVELOPE_SUS_GUIDE]); // Sustain horiz/vert line color
  ImGui::ItemSize(size,style.FramePadding.y);
  if (ImGui::ItemAdd(rect,ImGui::GetID("fmEnv"))) {
    ImGui::RenderFrame(rect.Min,rect.Max,ImGui::GetColorU32(ImGuiCol_FrameBg),true,style.FrameRounding);

    // Adjust for OPLL global sustain setting
    if (instType==DIV_INS_OPLL && algOrGlobalSus==1.0) {
      rr=5.0;
    }
    // calculate x positions
    float arPos=float(maxArDr-(float)ar)/maxArDr; // peak of AR, start of DR
    float drPos=arPos+(((float)sl/255.0)*(float(maxArDr-(float)dr)/maxArDr)); // end of DR, start of D2R
    float d2rPos=drPos+(((255.0-(float)sl)/255.0)*(float(255.0-(float)d2r)/255.0)); // End of D2R
    float rrPos=(float(maxRr-(float)rr)/float(maxRr)); // end of RR

    // shrink all the x positions horizontally
    arPos/=2.0;
    drPos/=2.0;
    d2rPos/=2.0;
    rrPos/=1.0;

    ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2(0.0,1.0)); // the bottom corner
    ImVec2 pos2=ImLerp(rect.Min,rect.Max,ImVec2(arPos,((float)tl/maxTl))); // peak of AR, start of DR
    ImVec2 pos3=ImLerp(rect.Min,rect.Max,ImVec2(drPos,(float)(((float)tl/maxTl)+((float)sl/255.0)-(((float)tl/maxTl)*((float)sl/255.0))))); // end of DR, start of D2R
    ImVec2 pos4=ImLerp(rect.Min,rect.Max,ImVec2(d2rPos,1.0)); // end of D2R
    ImVec2 posRStart=ImLerp(rect.Min,rect.Max,ImVec2(0.0,((float)tl/maxTl))); // release start
    ImVec2 posREnd=ImLerp(rect.Min,rect.Max,ImVec2(rrPos,1.0));// release end
    ImVec2 posSLineHEnd=ImLerp(rect.Min,rect.Max,ImVec2(1.0,(float)(((float)tl/maxTl)+((float)sl/255.0)-(((float)tl/maxTl)*((float)sl/255.0))))); // sustain horizontal line end
    ImVec2 posSLineVEnd=ImLerp(rect.Min,rect.Max,ImVec2(drPos,1.0)); // sustain vertical line end
    ImVec2 posDecayRate0Pt=ImLerp(rect.Min,rect.Max,ImVec2(1.0,((float)tl/maxTl))); // Height of the peak of AR, forever
    ImVec2 posDecay2Rate0Pt=ImLerp(rect.Min,rect.Max,ImVec2(1.0,(float)(((float)tl/maxTl)+((float)sl/255.0)-(((float)tl/maxTl)*((float)sl/255.0))))); // Height of the peak of SR, forever

    // dl->Flags=ImDrawListFlags_AntiAliasedLines|ImDrawListFlags_AntiAliasedLinesUseTex;
    if ((float)ar==0.0) { // if AR = 0, the envelope never starts
      dl->AddTriangleFilled(posRStart,posREnd,pos1,colorS); // draw release as shaded triangle behind everything
      addAALine(dl,pos1,pos4,color); // draw line on ground
    } else if ((float)dr==0.0 && (float)sl!=0.0) { // if DR = 0 and SL is not 0, then the envelope stays at max volume forever
      dl->AddTriangleFilled(posRStart,posREnd,pos1,colorS); // draw release as shaded triangle behind everything
      // addAALine(dl,pos3,posSLineHEnd,colorS); // draw horiz line through sustain level
      // addAALine(dl,pos3,posSLineVEnd,colorS); // draw vert. line through sustain level
      addAALine(dl,pos1,pos2,color); // A
      addAALine(dl,pos2,posDecayRate0Pt,color); // Line from A to end of graph
    } else if ((float)d2r==0.0 || ((instType==DIV_INS_OPL || instType==DIV_INS_SNES || instType == DIV_INS_ESFM) && sus==1.0) || (instType==DIV_INS_OPLL && egt!=0.0)) { // envelope stays at the sustain level forever
      dl->AddTriangleFilled(posRStart,posREnd,pos1,colorS); // draw release as shaded triangle behind everything
      addAALine(dl,pos3,posSLineHEnd,colorR); // draw horiz line through sustain level
      addAALine(dl,pos3,posSLineVEnd,colorR); // draw vert. line through sustain level
      addAALine(dl,pos1,pos2,color); // A
      addAALine(dl,pos2,pos3,color); // D
      addAALine(dl,pos3,posDecay2Rate0Pt,color); // Line from D to end of graph
    } else { // draw graph normally
      dl->AddTriangleFilled(posRStart,posREnd,pos1,colorS); // draw release as shaded triangle behind everything
      addAALine(dl,pos3,posSLineHEnd,colorR); // draw horiz line through sustain level
      addAALine(dl,pos3,posSLineVEnd,colorR); // draw vert. line through sustain level
      addAALine(dl,pos1,pos2,color); // A
      addAALine(dl,pos2,pos3,color); // D
      addAALine(dl,pos3,pos4,color); // D2
    }
    //dl->Flags^=ImDrawListFlags_AntiAliasedLines|ImDrawListFlags_AntiAliasedLinesUseTex;
  }
}


void FurnaceGUI::drawWaveformSID3(unsigned char type, const ImVec2& size) {
  ImDrawList* dl=ImGui::GetWindowDrawList();
  ImGuiWindow* window=ImGui::GetCurrentWindow();

  ImVec2 waveform[65];
  const size_t waveformLen=64;

  ImVec2 minArea=window->DC.CursorPos;
  ImVec2 maxArea=ImVec2(
    minArea.x+size.x,
    minArea.y+size.y
  );
  ImRect rect=ImRect(minArea,maxArea);
  ImGuiStyle& style=ImGui::GetStyle();
  ImU32 color=ImGui::GetColorU32(uiColors[GUI_COLOR_FM_WAVE]);
  ImGui::ItemSize(size,style.FramePadding.y);
  if (ImGui::ItemAdd(rect,ImGui::GetID("SID3wsDisplay"))) {
    ImGui::RenderFrame(rect.Min,rect.Max,ImGui::GetColorU32(ImGuiCol_FrameBg),true,style.FrameRounding);

    if (type<SID3_NUM_UNIQUE_SPECIAL_WAVES) {
      for (size_t i=0; i<=waveformLen; i++) {
        float x=(float)i/(float)waveformLen;
        float y=waveFuncsIns[type](x*2.0*M_PI);
        waveform[i]=ImLerp(rect.Min,rect.Max,ImVec2(x,0.5-y*0.4));
      }
    } else if (type>=SID3_NUM_UNIQUE_SPECIAL_WAVES && type<SID3_NUM_UNIQUE_SPECIAL_WAVES*2) {
      for (size_t i=0; i<=waveformLen; i++) {
        float x=(float)i/(float)waveformLen;
        float y=waveFuncsIns[type-SID3_NUM_UNIQUE_SPECIAL_WAVES](x*2.0*M_PI);

        y*=2.0f; // clipping

        if (y>1.0f) y=1.0f;
        if (y<-1.0f) y=-1.0f;

        waveform[i]=ImLerp(rect.Min,rect.Max,ImVec2(x,0.5-y*0.48));
      }
    } else {
      if (type==SID3_NUM_UNIQUE_SPECIAL_WAVES*2) {
        for (size_t i=0; i<=waveformLen; i++) {
          float x=(float)i/(float)waveformLen;
          float y=triangle(x*2.0*M_PI);

          y*=2.0f; // clipping

          if (y>1.0f) y=1.0f;
          if (y<-1.0f) y=-1.0f;

          waveform[i]=ImLerp(rect.Min,rect.Max,ImVec2(x,0.5-y*0.4));
        }
      }
      if (type==SID3_NUM_UNIQUE_SPECIAL_WAVES*2+1) {
        for (size_t i=0; i<=waveformLen; i++) {
          float x=(float)i/(float)waveformLen;
          float y=saw(x*2.0*M_PI);

          y*=2.0f; // clipping

          if (y>1.0f) y=1.0f;
          if (y<-1.0f) y=-1.0f;

          waveform[i]=ImLerp(rect.Min,rect.Max,ImVec2(x,0.5-y*0.4));
        }
      }
    }

    dl->AddPolyline(waveform,waveformLen+1,color,dpiScale,ImDrawFlags_None);
  }
}


void FurnaceGUI::insEditSID3(DivInstrument* ins) {
  char buffer[100];
  char buffer2[100];

  if (ImGui::BeginTabItem("SID3")) {
    if (ImGui::BeginTable("sid3Waves",2,0)) {
      ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed,0.0f);
      ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed,0.0f);

      ImGui::TableNextRow();
      ImGui::TableNextColumn();

      ImGui::AlignTextToFramePadding();
      ImGui::Text(_("Waveform"));
      ImGui::SameLine();
      pushToggleColors(ins->sid3.triOn);
      if (ImGui::Button(_("tri"))) { PARAMETER
        ins->sid3.triOn=!ins->sid3.triOn;
      }
      popToggleColors();
      ImGui::SameLine();
      pushToggleColors(ins->sid3.sawOn);
      if (ImGui::Button(_("saw"))) { PARAMETER
        ins->sid3.sawOn=!ins->sid3.sawOn;
      }
      popToggleColors();
      ImGui::SameLine();
      pushToggleColors(ins->sid3.pulseOn);
      if (ImGui::Button(_("pulse"))) { PARAMETER
        ins->sid3.pulseOn=!ins->sid3.pulseOn;
      }
      popToggleColors();
      ImGui::SameLine();
      pushToggleColors(ins->sid3.noiseOn);
      if (ImGui::Button(_("noise"))) { PARAMETER
        ins->sid3.noiseOn=!ins->sid3.noiseOn;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(_("Like in SID2,specific noise LFSR feedback bits config can produce tonal waves.\n"
        "Refer to the manual for LFSR bits macro configurations for which frequency calculation is altered\n"
        "in a way that makes tonal noise stay in tune."));
      }
      popToggleColors();
      ImGui::SameLine();

      P(ImGui::Checkbox(_("1-bit noise"),&ins->sid3.oneBitNoise));
      ImGui::SameLine();

      pushToggleColors(ins->sid3.specialWaveOn);
      if (ImGui::Button(_("special"))) { PARAMETER
        ins->sid3.specialWaveOn=!ins->sid3.specialWaveOn;
      }
      popToggleColors();

      P(CWSliderScalar(_("Special wave"),ImGuiDataType_U8,&ins->sid3.special_wave,&_ZERO,&_SID3_SPECIAL_WAVES,_(sid3SpecialWaveforms[ins->sid3.special_wave%SID3_NUM_SPECIAL_WAVES]))); rightClickable

      if (ImGui::Checkbox(_("Wavetable channel"),&ins->sid3.doWavetable)) {
        PARAMETER;
        ins->temp.vZoom[DIV_MACRO_WAVE]=-1;
        for (int i=0; i<256; i++) {
          ins->std.waveMacro.val[i]=0;
        }
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(_("Forces waveform macro to control wavetable index."));
      }

      bool invLeft=ins->sid3.phaseInv&SID3_INV_SIGNAL_LEFT;
      if (ImGui::Checkbox(_("Inv. left"),&invLeft)) { PARAMETER
        ins->sid3.phaseInv^=SID3_INV_SIGNAL_LEFT;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(_("Invert left channel signal"));
      }
      ImGui::SameLine();
      bool invRight=ins->sid3.phaseInv&SID3_INV_SIGNAL_RIGHT;
      if (ImGui::Checkbox(_("Inv. right"),&invRight)) { PARAMETER
        ins->sid3.phaseInv^=SID3_INV_SIGNAL_RIGHT;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(_("Invert right channel signal"));
      }

      ImGui::TableNextColumn();

      CENTER_TEXT(_("Special wave preview"));
      ImGui::TextUnformatted(_("Special wave preview"));
      drawWaveformSID3(ins->sid3.special_wave,ImVec2(120.0f*dpiScale,70.0f*dpiScale));

      ImGui::EndTable();
    }

    ImVec2 sliderSize=ImVec2(30.0f*dpiScale,256.0*dpiScale);

    if (ImGui::BeginTable("SID3EnvParams",6,ImGuiTableFlags_NoHostExtendX)) {
      ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
      ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
      ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
      ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
      ImGui::TableSetupColumn("c4",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
      ImGui::TableSetupColumn("c5",ImGuiTableColumnFlags_WidthStretch);

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      CENTER_TEXT(_("A"));
      ImGui::TextUnformatted(_("A"));
      ImGui::TableNextColumn();
      CENTER_TEXT(_("D"));
      ImGui::TextUnformatted(_("D"));
      ImGui::TableNextColumn();
      CENTER_TEXT(_("S"));
      ImGui::TextUnformatted(_("S"));
      ImGui::TableNextColumn();
      CENTER_TEXT(_("SR"));
      ImGui::TextUnformatted(_("SR"));
      ImGui::TableNextColumn();
      CENTER_TEXT(_("R"));
      ImGui::TextUnformatted(_("R"));
      ImGui::TableNextColumn();
      CENTER_TEXT(_("Envelope"));
      ImGui::TextUnformatted(_("Envelope"));

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      P(CWVSliderScalar("##Attack",sliderSize,ImGuiDataType_U8,&ins->sid3.a,&_ZERO,&_TWO_HUNDRED_FIFTY_FIVE)); rightClickable
      ImGui::TableNextColumn();
      P(CWVSliderScalar("##Decay",sliderSize,ImGuiDataType_U8,&ins->sid3.d,&_ZERO,&_TWO_HUNDRED_FIFTY_FIVE)); rightClickable
      ImGui::TableNextColumn();
      P(CWVSliderScalar("##Sustain",sliderSize,ImGuiDataType_U8,&ins->sid3.s,&_ZERO,&_TWO_HUNDRED_FIFTY_FIVE)); rightClickable
      ImGui::TableNextColumn();
      P(CWVSliderScalar("##SustainRate",sliderSize,ImGuiDataType_U8,&ins->sid3.sr,&_ZERO,&_TWO_HUNDRED_FIFTY_FIVE)); rightClickable
      ImGui::TableNextColumn();
      P(CWVSliderScalar("##Release",sliderSize,ImGuiDataType_U8,&ins->sid3.r,&_ZERO,&_TWO_HUNDRED_FIFTY_FIVE)); rightClickable
      ImGui::TableNextColumn();
      // the (ins->sid3.r==15?(ins->sid3.r-1):ins->sid3.r) is used so release part never becomes horizontal (which isn't the case with SID3 envelope)
      drawSID3Env(0,(ins->sid3.a==0?(255):(256-ins->sid3.a)),(ins->sid3.d==0?(255):(256-ins->sid3.d)),ins->sid3.sr,255-(ins->sid3.r==255?(ins->sid3.r-1):ins->sid3.r),255-ins->sid3.s,0,0,0,255,256,255,ImVec2(ImGui::GetContentRegionAvail().x,sliderSize.y),ins->type);

      ImGui::EndTable();
    }
    
    if (!ins->sid3.doWavetable) {
      strncpy(buffer,macroSID3WaveMixMode(0,(float)ins->sid3.mixMode,NULL).c_str(),40);
      P(CWSliderScalar(_("Wave Mix Mode"),ImGuiDataType_U8,&ins->sid3.mixMode,&_ZERO,&_FOUR,buffer));
      P(CWSliderScalar(_("Duty"),ImGuiDataType_U16,&ins->sid3.duty,&_ZERO,&_SIXTY_FIVE_THOUSAND_FIVE_HUNDRED_THIRTY_FIVE)); rightClickable
      P(CWSliderScalar(_("Feedback"),ImGuiDataType_U8,&ins->sid3.feedback,&_ZERO,&_TWO_HUNDRED_FIFTY_FIVE));
      bool resetDuty=ins->sid3.resetDuty;
      if (ImGui::Checkbox(_("Reset duty on new note"),&resetDuty)) { PARAMETER
        ins->sid3.resetDuty=resetDuty;
      }
      if (ImGui::Checkbox(_("Absolute Duty Macro"),&ins->sid3.dutyIsAbs)) {
        ins->temp.vZoom[DIV_MACRO_DUTY]=-1;
        PARAMETER;
      }
    }

    bool ringMod=ins->sid3.ringMod;
    if (ImGui::Checkbox(_("Ring Modulation"),&ringMod)) { PARAMETER
      ins->sid3.ringMod=ringMod;
    }

    ImGui::SameLine();

    strncpy(buffer,macroSID3SourceChan(0,(float)ins->sid3.ring_mod_source,NULL).c_str(),40);
    P(CWSliderScalar(_("Source channel##rmsrc"),ImGuiDataType_U8,&ins->sid3.ring_mod_source,&_ZERO,&_SID3_NUM_CHANNELS,buffer));

    bool oscSync=ins->sid3.oscSync;
    if (ImGui::Checkbox(_("Oscillator Sync"),&oscSync)) { PARAMETER
      ins->sid3.oscSync=oscSync;
    }

    ImGui::SameLine();

    strncpy(buffer,macroSID3SourceChan(0,(float)ins->sid3.sync_source,NULL).c_str(),40);
    P(CWSliderScalar(_("Source channel##hssrc"),ImGuiDataType_U8,&ins->sid3.sync_source,&_ZERO,&_SID3_NUM_CHANNELS_MINUS_ONE,buffer));

    bool phaseMod=ins->sid3.phase_mod;
    if (ImGui::Checkbox(_("Phase modulation"),&phaseMod)) { PARAMETER
      ins->sid3.phase_mod=phaseMod;
    }

    ImGui::SameLine();

    strncpy(buffer,macroSID3SourceChan(0,(float)ins->sid3.phase_mod_source,NULL).c_str(),40);
    P(CWSliderScalar(_("Source channel##pmsrc"),ImGuiDataType_U8,&ins->sid3.phase_mod_source,&_ZERO,&_SID3_NUM_CHANNELS_MINUS_ONE,buffer));

    ImGui::Separator();

    if (!ins->sid3.doWavetable) {
      bool sepNoisePitch=ins->sid3.separateNoisePitch;
      if (ImGui::Checkbox(_("Separate noise pitch"),&sepNoisePitch)) { PARAMETER
        ins->sid3.separateNoisePitch=sepNoisePitch;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(_("Make noise pitch independent from other waves' pitch.\nNoise pitch will be controllable via macros."));
      }
    }

    for (int i=0; i<SID3_NUM_FILTERS; i++) {
      DivInstrumentSID3::Filter* filt=&ins->sid3.filt[i];

      if (filt->enabled) {
        ImGui::Separator();
      }

      bool enable=filt->enabled;
      snprintf(buffer,100,_("Enable filter %d"),i+1);
      if (ImGui::Checkbox(buffer,&enable)) { PARAMETER
        filt->enabled=enable;
      }

      if (filt->enabled) {
        bool init=filt->init;
        snprintf(buffer,100,_("Initialize filter %d"),i+1);
        if (ImGui::Checkbox(buffer,&init)) { PARAMETER
          filt->init=init;
        }
        ImGui::SameLine();
        snprintf(buffer,100,_("Connect to channel input##contoinput%d"),i+1);
        bool toInput=filt->mode&SID3_FILTER_CHANNEL_INPUT;
        if (ImGui::Checkbox(buffer,&toInput)) { PARAMETER
          filt->mode^=SID3_FILTER_CHANNEL_INPUT;
        }

        snprintf(buffer,100,_("Cutoff##fcut%d"),i+1);
        P(CWSliderScalar(buffer,ImGuiDataType_U16,&filt->cutoff,&_ZERO,&_SIXTY_FIVE_THOUSAND_FIVE_HUNDRED_THIRTY_FIVE)); rightClickable
        snprintf(buffer,100,_("Resonance##fres%d"),i+1);
        P(CWSliderScalar(buffer,ImGuiDataType_U8,&filt->resonance,&_ZERO,&_TWO_HUNDRED_FIFTY_FIVE)); rightClickable
        snprintf(buffer,100,_("Output volume##foutvol%d"),i+1);
        P(CWSliderScalar(buffer,ImGuiDataType_U8,&filt->output_volume,&_ZERO,&_TWO_HUNDRED_FIFTY_FIVE)); rightClickable
        snprintf(buffer,100,_("Distortion level##fdist%d"),i+1);
        P(CWSliderScalar(buffer,ImGuiDataType_U8,&filt->distortion_level,&_ZERO,&_TWO_HUNDRED_FIFTY_FIVE)); rightClickable

        ImGui::AlignTextToFramePadding();
        ImGui::Text(_("Filter Mode"));
        ImGui::SameLine();

        bool lp=filt->mode&SID3_FILTER_LP;
        pushToggleColors(lp);
        snprintf(buffer,100,_("low##flow%d"),i+1);
        if (ImGui::Button(buffer)) { PARAMETER
          filt->mode^=SID3_FILTER_LP;
        }
        popToggleColors();
        ImGui::SameLine();

        bool bp=filt->mode&SID3_FILTER_BP;
        pushToggleColors(bp);
        snprintf(buffer,100,_("band##fband%d"),i+1);
        if (ImGui::Button(buffer)) { PARAMETER
          filt->mode^=SID3_FILTER_BP;
        }
        popToggleColors();
        ImGui::SameLine();

        bool hp=filt->mode&SID3_FILTER_HP;
        pushToggleColors(hp);
        snprintf(buffer,100,_("high##fhigh%d"),i+1);
        if (ImGui::Button(buffer)) { PARAMETER
          filt->mode^=SID3_FILTER_HP;
        }
        popToggleColors();


        ImGui::SameLine();
        snprintf(buffer,100,_("Connect to channel output##contooutput%d"),i+1);
        bool toOutput=filt->mode&SID3_FILTER_OUTPUT;
        if (ImGui::Checkbox(buffer,&toOutput)) { PARAMETER
          filt->mode^=SID3_FILTER_OUTPUT;
        }

        snprintf(buffer,100,_("Absolute cutoff macro##abscutoff%d"),i+1);
        bool absCutoff=filt->absoluteCutoff;
        if (ImGui::Checkbox(buffer,&absCutoff)) { PARAMETER
          filt->absoluteCutoff=!filt->absoluteCutoff;
          ins->temp.vZoom[DIV_MACRO_OP_D2R+(i<<5)]=-1;
        }

        snprintf(buffer,100,_("Change cutoff with pitch##bindcutoff%d"),i+1);
        P(ImGui::Checkbox(buffer,&filt->bindCutoffToNote));
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("Filter cutoff will change with frequency/pitch.\nSee settings below."));
        }

        if (filt->bindCutoffToNote) {
          snprintf(buffer,100,_("Decrease cutoff when pitch increases##decreasecutoff%d"),i+1);
          P(ImGui::Checkbox(buffer,&filt->bindCutoffToNoteDir));
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("If this is enabled,filter cutoff will decrease if you increase the pitch.\n"
            "If this is disabled,filter cutoff will increase if you increase the pitch."));
          }

          snprintf(buffer2,100,_("%s"),noteNameNormal(filt->bindCutoffToNoteCenter));
          snprintf(buffer,100,_("Cutoff change center note##bindcutcenternote%d"),i+1);
          P(CWSliderScalar(buffer,ImGuiDataType_U8,&filt->bindCutoffToNoteCenter,&_ZERO,&_ONE_HUNDRED_SEVENTY_NINE,buffer2)); rightClickable
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("The center note for cutoff changes. At this note no cutoff change happens.\nAs pitch goes lower or higher,cutoff changes apply."));
          }

          snprintf(buffer,100,_("Cutoff change strength##bindcutstrength%d"),i+1);
          P(CWSliderScalar(buffer,ImGuiDataType_U8,&filt->bindCutoffToNoteStrength,&_ZERO,&_TWO_HUNDRED_FIFTY_FIVE)); rightClickable
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("How much cutoff changes for given pitch change."));
          }
          snprintf(buffer,100,_("Scale cutoff only once on new note##bindcutnn%d"),i+1);
          P(ImGui::Checkbox(buffer,&filt->bindCutoffOnNote));
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("Filter cutoff will be changed only once on new note.\nIf this option is disabled,cutoff scaling will be applied\nevery time a pitch change happens."));
          }
        }

        snprintf(buffer,100,_("Change resonance with pitch##bindres%d"),i+1);
        P(ImGui::Checkbox(buffer,&filt->bindResonanceToNote));
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("Filter resonance will change with frequency/pitch.\nSee settings below."));
        }

        if (filt->bindResonanceToNote) {
          snprintf(buffer,100,_("Decrease resonance when pitch increases##decreaseres%d"),i+1);
          P(ImGui::Checkbox(buffer,&filt->bindResonanceToNoteDir));
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("If this is enabled,filter resonance will decrease if you increase the pitch.\n"
            "If this is disabled,filter resonance will increase if you increase the pitch."));
          }

          snprintf(buffer2,100,_("%s"),noteNameNormal(filt->bindResonanceToNoteCenter));
          snprintf(buffer,100,_("Resonance change center note##bindrescenternote%d"),i+1);
          P(CWSliderScalar(buffer,ImGuiDataType_U8,&filt->bindResonanceToNoteCenter,&_ZERO,&_ONE_HUNDRED_SEVENTY_NINE,buffer2)); rightClickable
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("The center note for resonance changes. At this note no resonance change happens.\nAs pitch goes lower or higher,resonance changes apply."));
          }

          snprintf(buffer,100,_("Resonance change strength##bindresstrength%d"),i+1);
          P(CWSliderScalar(buffer,ImGuiDataType_U8,&filt->bindResonanceToNoteStrength,&_ZERO,&_TWO_HUNDRED_FIFTY_FIVE)); rightClickable
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("How much resonance changes for given pitch change."));
          }
          snprintf(buffer,100,_("Scale resonance only once on new note##bindresnn%d"),i+1);
          P(ImGui::Checkbox(buffer,&filt->bindResonanceOnNote));
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("Filter resonance will be changed only once on new note.\nIf this option is disabled,resonance scaling will be applied\nevery time a pitch change happens."));
          }
        }
      }
    }

    ImGui::Separator();

    if (ImGui::BeginTable("SID3filtmatrix",1)) {
      if (waveGenVisible) ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed,250.0f*dpiScale);
      ImGui::TableNextRow();
      ImGui::TableNextColumn();

      CENTER_TEXT(_("Filters connection matrix"));
      ImGui::Text(_("Filters connection matrix"));

      if (ImGui::BeginTable("SID3checkboxesmatrix",3+SID3_NUM_FILTERS)) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text(">>");
        ImGui::TableNextColumn();
        ImGui::Text(_("In"));

        for (int i=0; i<SID3_NUM_FILTERS; i++) {
          ImGui::TableNextColumn();
          ImGui::Text("%d",i+1);
        }

        ImGui::TableNextColumn();
        ImGui::Text(_("Out"));

        ImGui::TableNextRow();

        for (int i=0; i<SID3_NUM_FILTERS; i++) {
          DivInstrumentSID3::Filter* filt=&ins->sid3.filt[i];

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::Text("%d",i+1);

          ImGui::TableNextColumn();

          snprintf(buffer,40,"##filtmatrixin%d",i+1);
          bool toInput=filt->mode&SID3_FILTER_CHANNEL_INPUT;
          if (ImGui::Checkbox(buffer,&toInput)) { PARAMETER
            filt->mode^=SID3_FILTER_CHANNEL_INPUT;
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("Feed signal from channel to filter %d input"),i+1);
          }

          for (int j=0; j<SID3_NUM_FILTERS; j++) {
            ImGui::TableNextColumn();
            snprintf(buffer,40,"##filtmatrix%d%d",i+1,j+1);

            bool enable=filt->filter_matrix&(1<<j);
            if (ImGui::Checkbox(buffer,&enable)) { PARAMETER
              filt->filter_matrix^=(1<<j);
            }
            if (ImGui::IsItemHovered()) {
              ImGui::SetTooltip(_("Feed signal from filter %d output to filter %d input"),j+1,i+1);
            }
          }

          ImGui::TableNextColumn();

          snprintf(buffer,40,"##filtmatrixout%d",i+1);
          bool toOutput=filt->mode&SID3_FILTER_OUTPUT;
          if (ImGui::Checkbox(buffer,&toOutput)) { PARAMETER
            filt->mode^=SID3_FILTER_OUTPUT;
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("Feed signal from filter %d output to channel output"),i+1);
          }
        }

        ImGui::EndTable();
      }

      ImGui::EndTable();
    }

    ImGui::EndTabItem();
  }

  if (!ins->amiga.useSample) {
    insTabWavetable(ins);
  }
  insTabSample(ins);

  std::vector<FurnaceGUIMacroDesc> macroList;

  for (int i=0; i<SID3_NUM_FILTERS; i++) {
    snprintf(buffer,40,_("Filter %d macros"),i+1);

    if (ImGui::BeginTabItem(buffer)) {
      macroList.push_back(FurnaceGUIMacroDesc(_("Cutoff"),&ins->std.opMacros[i].d2rMacro,ins->sid3.filt[i].absoluteCutoff?0:-65535,65535,160,uiColors[GUI_COLOR_MACRO_FILTER]));
      macroList.push_back(FurnaceGUIMacroDesc(_("Resonance"),&ins->std.opMacros[i].damMacro,0,255,160,uiColors[GUI_COLOR_MACRO_FILTER]));
      macroList.push_back(FurnaceGUIMacroDesc(_("Filter Toggle"),&ins->std.opMacros[i].drMacro,0,1,32,uiColors[GUI_COLOR_MACRO_FILTER],false,NULL,NULL,true));
      macroList.push_back(FurnaceGUIMacroDesc(_("Filter Mode"),&ins->std.opMacros[i].ksrMacro,0,3,48,uiColors[GUI_COLOR_MACRO_FILTER],false,NULL,NULL,true,filtModeBits));
      macroList.push_back(FurnaceGUIMacroDesc(_("Distortion Level"),&ins->std.opMacros[i].dt2Macro,0,255,160,uiColors[GUI_COLOR_MACRO_FILTER]));
      macroList.push_back(FurnaceGUIMacroDesc(_("Output Volume"),&ins->std.opMacros[i].dtMacro,0,255,160,uiColors[GUI_COLOR_MACRO_FILTER]));
      macroList.push_back(FurnaceGUIMacroDesc(_("Channel Input Connection"),&ins->std.opMacros[i].dvbMacro,0,1,32,uiColors[GUI_COLOR_MACRO_FILTER],false,NULL,NULL,true));
      macroList.push_back(FurnaceGUIMacroDesc(_("Channel Output Connection"),&ins->std.opMacros[i].egtMacro,0,1,32,uiColors[GUI_COLOR_MACRO_FILTER],false,NULL,NULL,true));
      macroList.push_back(FurnaceGUIMacroDesc(_("Connection Matrix Row"),&ins->std.opMacros[i].kslMacro,0,SID3_NUM_FILTERS,16*SID3_NUM_FILTERS,uiColors[GUI_COLOR_MACRO_FILTER],false,NULL,NULL,true,sid3FilterMatrixBits));

      drawMacros(macroList,macroEditStateOP[i],ins);

      ImGui::EndTabItem();
    }
  }

  if (ImGui::BeginTabItem(_("Macros"))) {
    macroList.push_back(FurnaceGUIMacroDesc(_("Volume"),&ins->std.volMacro,0,255,160,uiColors[GUI_COLOR_MACRO_VOLUME]));

    macroList.push_back(FurnaceGUIMacroDesc(_("Arpeggio"),&ins->std.arpMacro,-120,120,160,uiColors[GUI_COLOR_MACRO_PITCH],true,NULL,macroHoverNote,false,NULL,true,ins->std.arpMacro.val));
    macroList.push_back(FurnaceGUIMacroDesc(_("Pitch"),&ins->std.pitchMacro,-2048,2047,160,uiColors[GUI_COLOR_MACRO_PITCH],true,macroRelativeMode));

    if (ins->sid3.doWavetable) {
      int waveCount=MAX(1,e->song.waveLen-1);
      macroList.push_back(FurnaceGUIMacroDesc(_("Waveform"),&ins->std.waveMacro,0,waveCount,160,uiColors[GUI_COLOR_MACRO_WAVE],false,NULL,NULL,false,NULL));
    } else {
      macroList.push_back(FurnaceGUIMacroDesc(_("Duty"),&ins->std.dutyMacro,ins->sid3.dutyIsAbs?0:-65535,65535,160,uiColors[GUI_COLOR_MACRO_OTHER]));
      macroList.push_back(FurnaceGUIMacroDesc(_("Waveform"),&ins->std.waveMacro,0,5,16 * 5,uiColors[GUI_COLOR_MACRO_WAVE],false,NULL,NULL,true,sid3ShapeBits));
      macroList.push_back(FurnaceGUIMacroDesc(_("Special Wave"),&ins->std.algMacro,0,SID3_NUM_SPECIAL_WAVES - 1,160,uiColors[GUI_COLOR_MACRO_WAVE],false,NULL,macroSID3SpecialWaves));
    }

    if (ins->sid3.separateNoisePitch && !ins->sid3.doWavetable) {
      macroList.push_back(FurnaceGUIMacroDesc(_("Noise Arpeggio"),&ins->std.opMacros[3].amMacro,-120,120,160,uiColors[GUI_COLOR_MACRO_PITCH],true,NULL,macroHoverNote,false,NULL,true,ins->std.opMacros[3].amMacro.val,true));
      macroList.push_back(FurnaceGUIMacroDesc(_("Noise Pitch"),&ins->std.opMacros[0].arMacro,-2048,2047,160,uiColors[GUI_COLOR_MACRO_PITCH],true,macroRelativeMode,NULL,false,NULL,false,NULL,false,true));
    }

    macroList.push_back(FurnaceGUIMacroDesc(_("Panning (left)"),&ins->std.panLMacro,0,255,160,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL));
    macroList.push_back(FurnaceGUIMacroDesc(_("Panning (right)"),&ins->std.panRMacro,0,255,160,uiColors[GUI_COLOR_MACRO_OTHER]));

    macroList.push_back(FurnaceGUIMacroDesc(_("Channel inversion"),&ins->std.opMacros[2].arMacro,0,2,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,minModModeBits));

    macroList.push_back(FurnaceGUIMacroDesc(_("Key On/Off"),&ins->std.opMacros[0].amMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));

    macroList.push_back(FurnaceGUIMacroDesc(_("Special"),&ins->std.ex1Macro,0,3,48,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,sid3ControlBits));

    macroList.push_back(FurnaceGUIMacroDesc(_("Ring Mod Source"),&ins->std.fmsMacro,0,SID3_NUM_CHANNELS,64,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,macroSID3SourceChan));
    macroList.push_back(FurnaceGUIMacroDesc(_("Hard Sync Source"),&ins->std.amsMacro,0,SID3_NUM_CHANNELS - 1,64,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,macroSID3SourceChan));
    macroList.push_back(FurnaceGUIMacroDesc(_("Phase Mod Source"),&ins->std.fbMacro,0,SID3_NUM_CHANNELS - 1,64,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,macroSID3SourceChan));
    
    if (!ins->sid3.doWavetable) {
      macroList.push_back(FurnaceGUIMacroDesc(_("Feedback"),&ins->std.opMacros[3].arMacro,0,255,160,uiColors[GUI_COLOR_MACRO_OTHER]));
    }

    macroList.push_back(FurnaceGUIMacroDesc(_("Phase Reset"),&ins->std.phaseResetMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));

    if (!ins->sid3.doWavetable) {
      macroList.push_back(FurnaceGUIMacroDesc(_("Noise Phase Reset"),&ins->std.opMacros[1].amMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
    }
    macroList.push_back(FurnaceGUIMacroDesc(_("Envelope Reset"),&ins->std.opMacros[2].amMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));

    macroList.push_back(FurnaceGUIMacroDesc(_("Attack"),&ins->std.ex2Macro,0,255,160,uiColors[GUI_COLOR_MACRO_ENVELOPE]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Decay"),&ins->std.ex3Macro,0,255,160,uiColors[GUI_COLOR_MACRO_ENVELOPE]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Sustain"),&ins->std.ex4Macro,0,255,160,uiColors[GUI_COLOR_MACRO_ENVELOPE]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Sustain Rate"),&ins->std.ex5Macro,0,255,160,uiColors[GUI_COLOR_MACRO_ENVELOPE]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Release"),&ins->std.ex6Macro,0,255,160,uiColors[GUI_COLOR_MACRO_ENVELOPE]));

    if (!ins->sid3.doWavetable) {
      macroList.push_back(FurnaceGUIMacroDesc(_("Noise LFSR bits"),&ins->std.ex7Macro,0,30,16 * 30,uiColors[GUI_COLOR_MACRO_NOISE],false,NULL,macroSID3NoiseLFSR,true));
      macroList.push_back(FurnaceGUIMacroDesc(_("1-Bit Noise"),&ins->std.opMacros[1].arMacro,0,1,32,uiColors[GUI_COLOR_MACRO_NOISE],false,NULL,NULL,true));
      macroList.push_back(FurnaceGUIMacroDesc(_("Wave Mix"),&ins->std.ex8Macro,0,4,64,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,macroSID3WaveMixMode));
    } else {
      macroList.push_back(FurnaceGUIMacroDesc(_("Sample Mode"),&ins->std.opMacros[1].arMacro,0,1,32,uiColors[GUI_COLOR_MACRO_NOISE],false,NULL,NULL,true));
    }

    drawMacros(macroList,macroEditStateMacros,ins);
    ImGui::EndTabItem();
  }
}
