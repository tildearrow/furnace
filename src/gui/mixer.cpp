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

#include "gui.h"
#include "intConst.h"
#include <fmt/printf.h>
#include <imgui.h>
#include "../ta-log.h"
#include "imgui_internal.h"

const char* portNamesStereo[2]={
  _N("left"),
  _N("right")
};

ImVec2 FurnaceGUI::calcPortSetSize(String label, int ins, int outs) {
  ImGuiStyle& style=ImGui::GetStyle();

  ImVec2 labelSize=ImGui::CalcTextSize(label.c_str(),NULL,false,ImGui::GetWindowSize().x*0.6f);

  ImVec2 size=labelSize;

  // pad
  size.x+=style.FramePadding.x*2.0f;
  size.y+=style.FramePadding.y*2.0f;

  // space for ports
  size.y+=MAX(ins,outs)*(labelSize.y+style.FramePadding.y+style.ItemSpacing.y);

  return size;
}

bool FurnaceGUI::portSet(String label, unsigned int portSetID, int ins, int outs, int activeIns, int activeOuts, int& clickedPort, std::map<unsigned int,ImVec2>& portPos) {
  String portID=fmt::sprintf("portSet%.4x",portSetID);

  ImDrawList* dl=ImGui::GetWindowDrawList();
  ImGuiWindow* window=ImGui::GetCurrentWindow();
  ImGuiStyle& style=ImGui::GetStyle();

  ImVec2 labelSize=ImGui::CalcTextSize(label.c_str(),NULL,false,ImGui::GetWindowSize().x*0.6f);

  ImVec2 size=labelSize;

  // pad
  size.x+=style.FramePadding.x*2.0f;
  size.y+=style.FramePadding.y*2.0f;

  // space for ports
  size.y+=MAX(ins,outs)*(ImGui::GetFontSize()+style.FramePadding.y+style.ItemSpacing.y);

  ImVec4 portSetBorderColor=uiColors[GUI_COLOR_PATCHBAY_PORTSET];
  ImVec4 portSetColor=ImVec4(
    portSetBorderColor.x*0.75f,
    portSetBorderColor.y*0.75f,
    portSetBorderColor.z*0.75f,
    portSetBorderColor.w
  );

  ImVec4 portBorderColor=uiColors[GUI_COLOR_PATCHBAY_PORT];
  ImVec4 portColor=ImVec4(
    portBorderColor.x*0.75f,
    portBorderColor.y*0.75f,
    portBorderColor.z*0.75f,
    portBorderColor.w
  );

  ImVec4 portBorderColorH=uiColors[GUI_COLOR_PATCHBAY_PORT_HIDDEN];
  ImVec4 portColorH=ImVec4(
    portBorderColorH.x*0.75f,
    portBorderColorH.y*0.75f,
    portBorderColorH.z*0.75f,
    portBorderColorH.w
  );

  ImVec2 minArea=window->DC.CursorPos;
  ImVec2 maxArea=ImVec2(
    minArea.x+size.x,
    minArea.y+size.y
  );
  ImRect rect=ImRect(minArea,maxArea);

  ImVec2 textPos=ImVec2(
    minArea.x+style.FramePadding.x,
    minArea.y+style.FramePadding.y
  );

  ImGui::ItemSize(size,style.FramePadding.y);
  bool visible=ImGui::ItemAdd(rect,ImGui::GetID(portID.c_str()));
  bool hovered=false;
  bool active=false;
  if (visible) {
    hovered=ImGui::ItemHoverable(rect,ImGui::GetID(portID.c_str()),0);
    active=(hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left));

    if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
      ImGui::OpenPopup("SubPortOptions");
      selectedPortSet=portSetID;
      clickedPort=-1;
    }

    if (hovered) hoveredPortSet=portSetID;
    if (active) clickedPort=-1;

    // label
    dl->AddRectFilled(minArea,maxArea,ImGui::GetColorU32(portSetColor),0.0f);
    dl->AddRect(minArea,maxArea,ImGui::GetColorU32((selectedPortSet==portSetID)?uiColors[GUI_COLOR_TEXT]:portSetBorderColor),0.0f,0,dpiScale);
    dl->AddText(ImGui::GetFont(),ImGui::GetFontSize(),textPos,ImGui::GetColorU32(uiColors[GUI_COLOR_TEXT]),label.c_str(),NULL,ImGui::GetWindowSize().x*0.6f);
  }

  // input ports
  for (int i=0; i<ins; i++) {
    String portLabel=_("input");
    String subPortID=fmt::sprintf("subPort%.5x",(portSetID<<4)|i);
    if (ins==2) {
      portLabel=_(portNamesStereo[i&1]);
    } else if (ins>2) {
      portLabel=fmt::sprintf("%d",i+1);
    }
    ImVec2 portLabelSize=ImGui::CalcTextSize(portLabel.c_str());

    ImVec2 portMin=ImVec2(
      minArea.x,
      minArea.y+style.FramePadding.y+labelSize.y+style.ItemSpacing.y+(style.ItemSpacing.y+portLabelSize.y+style.FramePadding.y)*i
    );
    ImVec2 portMax=ImVec2(
      minArea.x+portLabelSize.x+style.FramePadding.x,
      portMin.y+style.FramePadding.y+portLabelSize.y
    );
    ImRect portRect=ImRect(portMin,portMax);
    ImVec2 portLabelPos=portMin;
    portLabelPos.x+=style.FramePadding.x*0.5;
    portLabelPos.y+=style.FramePadding.y*0.5;

    portPos[(portSetID<<4)|i]=ImLerp(portMin,portMax,ImVec2(0.0f,0.5f));

    if (visible) {
      if (ImGui::ItemAdd(portRect,ImGui::GetID(subPortID.c_str()))) {
        dl->AddRectFilled(portMin,portMax,ImGui::GetColorU32((i<activeIns)?portColor:portColorH),0.0f);
        dl->AddRect(portMin,portMax,ImGui::GetColorU32((i<activeIns)?portBorderColor:portBorderColorH),0.0f,dpiScale);
        dl->AddText(portLabelPos,ImGui::GetColorU32(uiColors[GUI_COLOR_TEXT]),portLabel.c_str());
      }

      if (ImGui::IsMouseHoveringRect(portMin,portMax)) {
        hoveredSubPort=i;
        if (active) clickedPort=i;
      }
    }
  }

  // output ports
  for (int i=0; i<outs; i++) {
    String portLabel=_("output");
    String subPortID=fmt::sprintf("subPort%.5x",(portSetID<<4)|i);
    if (outs==2) {
      portLabel=_(portNamesStereo[i&1]);
    } else if (outs>2) {
      portLabel=fmt::sprintf("%d",i+1);
    }
    ImVec2 portLabelSize=ImGui::CalcTextSize(portLabel.c_str());

    ImVec2 portMin=ImVec2(
      maxArea.x-portLabelSize.x-style.FramePadding.x,
      minArea.y+style.FramePadding.y+labelSize.y+style.ItemSpacing.y+(style.ItemSpacing.y+portLabelSize.y+style.FramePadding.y)*i
    );
    ImVec2 portMax=ImVec2(
      maxArea.x,
      portMin.y+style.FramePadding.y+portLabelSize.y
    );
    ImRect portRect=ImRect(portMin,portMax);
    ImVec2 portLabelPos=portMin;
    portLabelPos.x+=style.FramePadding.x*0.5;
    portLabelPos.y+=style.FramePadding.y*0.5;

    portPos[(portSetID<<4)|i]=ImLerp(portMin,portMax,ImVec2(1.0f,0.5f));

    if (visible) {
      if (ImGui::ItemAdd(portRect,ImGui::GetID(subPortID.c_str()))) {
        dl->AddRectFilled(portMin,portMax,ImGui::GetColorU32((i<activeOuts)?portColor:portColorH),0.0f);
        dl->AddRect(portMin,portMax,ImGui::GetColorU32((i<activeOuts)?portBorderColor:portBorderColorH),0.0f,dpiScale);
        dl->AddText(portLabelPos,ImGui::GetColorU32(uiColors[GUI_COLOR_TEXT]),portLabel.c_str());
      }

      if (ImGui::IsMouseHoveringRect(portMin,portMax)) {
        if (active) clickedPort=i;
        hoveredSubPort=i;
    }
    }
  }

  if (visible && hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) return true;

  return false;
}

void FurnaceGUI::drawMixer() {
  if (nextWindow==GUI_WINDOW_MIXER) {
    mixerOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!mixerOpen) return;
  if (mobileUI) {
    patWindowPos=(portrait?ImVec2(0.0f,(mobileMenuPos*-0.65*canvasH)):ImVec2((0.16*canvasH)+0.5*canvasW*mobileMenuPos,0.0f));
    patWindowSize=(portrait?ImVec2(canvasW,canvasH-(0.16*canvasW)):ImVec2(canvasW-(0.16*canvasH),canvasH));
    ImGui::SetNextWindowPos(patWindowPos);
    ImGui::SetNextWindowSize(patWindowSize);
  } else {
    ImGui::SetNextWindowSizeConstraints(ImVec2(400.0f*dpiScale,200.0f*dpiScale),ImVec2(canvasW,canvasH));
  }
  if (ImGui::Begin("Mixer",&mixerOpen,globalWinFlags|(settings.allowEditDocking?0:ImGuiWindowFlags_NoDocking),_("Mixer"))) {
    if (ImGui::BeginTabBar("MixerView")) {
      if (ImGui::BeginTabItem(_("Mixer"))) {
        const float itemWidth=60*dpiScale;
        float maxY=ImGui::GetContentRegionAvail().y;
        if (settings.mixerLayout) {
          ImGui::TextUnformatted(_("Master Volume"));
          if (settings.mixerStyle==2) {
            ImVec2 pos=ImGui::GetCursorScreenPos(),
                  size=ImVec2(ImGui::GetContentRegionAvail().x,ImGui::GetFontSize()+ImGui::GetStyle().FramePadding.y*2.0f);
            drawVolMeterInternal(ImGui::GetWindowDrawList(),ImRect(pos,pos+size),peak,e->getAudioDescGot().outChans,true);
            ImGui::PushStyleColor(ImGuiCol_FrameBg,0);
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive,0);
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered,127<<IM_COL32_A_SHIFT);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,0);
          }
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::SliderFloat("##mixerMaster",&e->song.masterVol,0,3,"%.2fx")) {
            if (e->song.masterVol<0) e->song.masterVol=0;
            if (e->song.masterVol>3) e->song.masterVol=3;
            MARK_MODIFIED;
          } rightClickable
          if (settings.mixerStyle==2) {
            ImGui::PopStyleColor(3);
            ImGui::PopStyleVar();
          } else if (settings.mixerStyle==1) {
            ImVec2 pos=ImGui::GetCursorScreenPos(),
                  size=ImVec2(ImGui::GetContentRegionAvail().x,ImGui::GetFontSize()+ImGui::GetStyle().FramePadding.y*2.0f);
            drawVolMeterInternal(ImGui::GetWindowDrawList(),ImRect(pos,pos+size),peak,e->getAudioDescGot().outChans,true);
            ImGui::Dummy(size);
          }
        } else {
          VerticalText(maxY,true,_("Master Volume"));
          ImGui::SameLine();
          if (settings.mixerStyle==2) {
            ImVec2 pos=ImGui::GetCursorScreenPos();
            drawVolMeterInternal(ImGui::GetWindowDrawList(),ImRect(pos,pos+ImVec2(40*dpiScale,maxY)),peak,e->getAudioDescGot().outChans,false);
            ImGui::PushStyleColor(ImGuiCol_FrameBg,0);
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive,0);
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered,127<<IM_COL32_A_SHIFT);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,0);
          }
          if (ImGui::VSliderFloat("##mixerMaster",ImVec2(40*dpiScale,maxY),&e->song.masterVol,0,3,"%.2fx")) {
            if (e->song.masterVol<0) e->song.masterVol=0;
            if (e->song.masterVol>3) e->song.masterVol=3;
            MARK_MODIFIED;
          } rightClickable
          ImGui::SameLine();
          if (settings.mixerStyle==2) {
            ImGui::PopStyleColor(3);
            ImGui::PopStyleVar();
          } else if (settings.mixerStyle==1) {
            ImVec2 pos=ImGui::GetCursorScreenPos();
            drawVolMeterInternal(ImGui::GetWindowDrawList(),ImRect(pos,pos+ImVec2(40*dpiScale,maxY)),peak,e->getAudioDescGot().outChans,false);
            ImGui::Dummy(ImVec2(40*dpiScale,maxY));
            ImGui::SameLine();
          }
          // figure out if we need to cut the height for the scrollbar
          float itemCalcWidth=
            itemWidth+
            1.5f*ImGui::GetStyle().FramePadding.x+
            4*ImGui::GetStyle().FramePadding.x+
            dpiScale+ // separator
            (settings.mixerStyle==1?(itemWidth-ImGui::GetFontSize()-ImGui::GetStyle().FramePadding.x):0)
          ;
          float realwidth=ImGui::GetWindowWidth()-ImGui::GetCursorPosX();
          if ((itemCalcWidth*e->song.systemLen)>realwidth) maxY-=ImGui::GetStyle().ScrollbarSize;
        }
        if (ImGui::BeginChild("##mixerPerChipContainer",ImVec2(0,0),0,ImGuiWindowFlags_HorizontalScrollbar)) {
          for (int i=0; i<e->song.systemLen; i++) {
            if (settings.mixerLayout==0) {
              ImGui::GetWindowDrawList()->AddRectFilled(
                ImGui::GetCursorScreenPos(),
                ImGui::GetCursorScreenPos()+ImVec2(dpiScale,maxY),
                ImGui::GetColorU32(ImGuiCol_Separator)
              );
              ImGui::Dummy(ImVec2(dpiScale,maxY));
              ImGui::SameLine();
            }
            if (chipMixer(i,ImVec2(itemWidth,maxY))) MARK_MODIFIED;
            if (settings.mixerLayout==0) ImGui::SameLine();
          }
        }
        ImGui::EndChild();
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(_("Patchbay"))) {
        std::map<unsigned int,ImVec2> portPos;

        if (ImGui::BeginTable("PatchbayOptions",3)) {
          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          if (ImGui::Checkbox(_("Automatic patchbay"),&e->song.patchbayAuto)) {
            if (e->song.patchbayAuto) e->autoPatchbayP();
            MARK_MODIFIED;
          }
          ImGui::TableNextColumn();
          ImGui::Checkbox(_("Display hidden ports"),&displayHiddenPorts);
          ImGui::TableNextColumn();
          ImGui::Checkbox(_("Display internal"),&displayInternalPorts);
          ImGui::EndTable();
        }

        hoveredPortSet=0x1fff;
        hoveredSubPort=-1;

        if (ImGui::BeginChild("Patchbay",ImVec2(0,0),ImGuiChildFlags_Borders)) {
          ImDrawList* dl=ImGui::GetWindowDrawList();
          ImVec2 topPos=ImGui::GetCursorPos();
          ImVec2 sysSize=calcPortSetSize(_("System"),displayHiddenPorts?DIV_MAX_OUTPUTS:e->getAudioDescGot().outChans,0);
          topPos.x+=ImGui::GetContentRegionAvail().x-sysSize.x;
          if (ImGui::GetContentRegionAvail().y>sysSize.y) topPos.y+=(ImGui::GetContentRegionAvail().y-sysSize.y)*0.5+ImGui::GetScrollY();

          if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) selectedPortSet=0x1fff;

          if (portDragActive) {
            dl->AddLine(subPortPos,ImGui::GetMousePos(),ImGui::GetColorU32(uiColors[GUI_COLOR_PATCHBAY_CONNECTION]),2.0f*dpiScale);
          }

          for (int i=0; i<e->song.systemLen; i++) {
            DivDispatch* dispatch=e->getDispatch(i);
            if (dispatch==NULL) continue;
            int outputs=dispatch->getOutputCount();
            if (portSet(fmt::sprintf("%d. %s",i+1,getSystemName(e->song.system[i])),i,0,outputs,0,outputs,selectedSubPort,portPos)) {
              selectedPortSet=i;
              if (selectedSubPort>=0) {
                portDragActive=true;
                ImGui::InhibitInertialScroll();

                auto subPortI=portPos.find((selectedPortSet<<4)|selectedSubPort);
                if (subPortI!=portPos.cend()) {
                  subPortPos=subPortI->second;
                } else {
                  portDragActive=false;
                }
              }
            }
          }

          // file player/metronome/sample preview
          if (displayInternalPorts) {
            if (portSet(_("Music Player"),0xffc,0,16,0,16,selectedSubPort,portPos)) {
              selectedPortSet=0xffc;
              if (selectedSubPort>=0) {
                portDragActive=true;
                ImGui::InhibitInertialScroll();
                auto subPortI=portPos.find((selectedPortSet<<4)|selectedSubPort);
                if (subPortI!=portPos.cend()) {
                  subPortPos=subPortI->second;
                } else {
                  portDragActive=false;
                }
              }
            }
            if (portSet(_("Sample Preview"),0xffd,0,1,0,1,selectedSubPort,portPos)) {
              selectedPortSet=0xffd;
              if (selectedSubPort>=0) {
                portDragActive=true;
                ImGui::InhibitInertialScroll();
                auto subPortI=portPos.find((selectedPortSet<<4)|selectedSubPort);
                if (subPortI!=portPos.cend()) {
                  subPortPos=subPortI->second;
                } else {
                  portDragActive=false;
                }
              }
            }
            if (portSet(_("Metronome"),0xffe,0,1,0,1,selectedSubPort,portPos)) {
              selectedPortSet=0xffe;
              if (selectedSubPort>=0) {
                portDragActive=true;
                ImGui::InhibitInertialScroll();
                auto subPortI=portPos.find((selectedPortSet<<4)|selectedSubPort);
                if (subPortI!=portPos.cend()) {
                  subPortPos=subPortI->second;
                } else {
                  portDragActive=false;
                }
              }
            }
          }

          ImGui::SetCursorPos(topPos);
          if (portSet(_("System"),0x1000,displayHiddenPorts?DIV_MAX_OUTPUTS:e->getAudioDescGot().outChans,0,e->getAudioDescGot().outChans,0,selectedSubPort,portPos)) {
            selectedPortSet=0x1000;
            if (selectedSubPort>=0) {
              portDragActive=true;
              ImGui::InhibitInertialScroll();
              auto subPortI=portPos.find((selectedPortSet<<4)|selectedSubPort);
              if (subPortI!=portPos.cend()) {
                subPortPos=subPortI->second;
              } else {
                portDragActive=false;
              }
            }
          }

          if (portDragActive) {
            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
              portDragActive=false;
              if (hoveredPortSet!=0x1fff && hoveredSubPort>=0 && selectedPortSet!=hoveredPortSet) {
                unsigned int src=(selectedPortSet<<4)|selectedSubPort;
                unsigned int dest=(hoveredPortSet<<4)|hoveredSubPort;

                if (src&0x10000) {
                  src^=dest;
                  dest^=src;
                  src^=dest;
                }

                src&=0xffff;
                dest&=0xffff;

                if (!e->patchConnect(src,dest)) {
                  e->patchDisconnect(src,dest);
                }
                MARK_MODIFIED;
              }
            }
          }

          // draw connections
          for (unsigned int i: e->song.patchbay) {
            if ((i>>20)==selectedPortSet) continue;
            auto portSrcI=portPos.find(i>>16);
            auto portDestI=portPos.find(0x10000|(i&0xffff));
            if (portSrcI!=portPos.cend() && portDestI!=portPos.cend()) {
              ImVec2 portSrc=portSrcI->second;
              ImVec2 portDest=portDestI->second;
              dl->AddLine(portSrc,portDest,ImGui::GetColorU32(uiColors[GUI_COLOR_PATCHBAY_CONNECTION_BG]),2.0f*dpiScale);
            }
          }

          // foreground
          for (unsigned int i: e->song.patchbay) {
            if ((i>>20)!=selectedPortSet) continue;
            auto portSrcI=portPos.find(i>>16);
            auto portDestI=portPos.find(0x10000|(i&0xffff));
            if (portSrcI!=portPos.cend() && portDestI!=portPos.cend()) {
              ImVec2 portSrc=portSrcI->second;
              ImVec2 portDest=portDestI->second;
              dl->AddLine(portSrc,portDest,ImGui::GetColorU32(uiColors[GUI_COLOR_PATCHBAY_CONNECTION]),2.0f*dpiScale);
            }
          }
        }
        if (ImGui::BeginPopup("SubPortOptions",ImGuiWindowFlags_NoMove|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoSavedSettings)) {
          if (ImGui::MenuItem(_("disconnect all"))) {
            e->patchDisconnectAll(selectedPortSet);
            MARK_MODIFIED;
          }
          ImGui::EndPopup();
        }
        ImGui::EndChild();
        ImGui::EndTabItem();
      }
      ImGui::EndTabBar();
    }

  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_MIXER;
  ImGui::End();
}

bool FurnaceGUI::chipMixer(int which, ImVec2 size) {
  bool ret=false;
  ImGui::PushID(which);
  ImGui::BeginGroup();
    float textHeight=ImGui::GetFontSize();

    float vol=fabs(e->song.systemVol[which]);
    bool doInvert=e->song.systemVol[which]<0;
    if (settings.mixerLayout) {
      if (ImGui::BeginTable("mixerVectRow1",2)) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(e->getSystemName(e->song.system[which]));
        ImGui::TableNextColumn();
        if (ImGui::Checkbox("##ChipInvert",&doInvert)) {
          e->song.systemVol[which]=doInvert?-vol:vol;
          ret=true;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("Invert"));
        }
        ImGui::EndTable();
      }
      if (settings.mixerStyle==2) {
        ImVec2 pos=ImGui::GetCursorScreenPos(),
            posMax=pos+ImVec2(ImGui::GetContentRegionAvail().x,ImGui::GetFontSize()+ImGui::GetStyle().FramePadding.y*2.0f);
        drawVolMeterInternal(ImGui::GetWindowDrawList(),ImRect(pos,posMax),e->chipPeak[which],e->getDispatch(which)->getOutputCount(),true);

        ImGui::PushStyleColor(ImGuiCol_FrameBg,0);
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive,0);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered,127<<IM_COL32_A_SHIFT);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,0);
      }
      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
      if (ImGui::SliderFloat("##ChipVol",&vol,0.0f,2.0f)) {
        if (doInvert) {
          if (vol<0.0001) vol=0.0001;
        }
        if (vol<0) vol=0;
        if (vol>10) vol=10;
        e->song.systemVol[which]=doInvert?-vol:vol;
        ret=true;
      } rightClickable
      if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
        ImGui::SetTooltip(_("Volume"));
      }
      if (settings.mixerStyle==2) {
        ImGui::PopStyleVar(1);
        ImGui::PopStyleColor(3);
      } 
      if (ImGui::BeginTable("mixerVectRow3",2)) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::SliderFloat("##ChipPan",&e->song.systemPan[which],-1.0f,1.0f)) {
          if (e->song.systemPan[which]<-1.0f) e->song.systemPan[which]=-1.0f;
          if (e->song.systemPan[which]>1.0f) e->song.systemPan[which]=1.0f;
          ret=true;
        } rightClickable
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
          ImGui::SetTooltip(_("Panning"));
        }
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::SliderFloat("##ChipPanFR",&e->song.systemPanFR[which],-1.0f,1.0f)) {
          if (e->song.systemPanFR[which]<-1.0f) e->song.systemPanFR[which]=-1.0f;
          if (e->song.systemPanFR[which]>1.0f) e->song.systemPanFR[which]=1.0f;
          ret=true;
        } rightClickable
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
          ImGui::SetTooltip(_("Front/Rear"));
        }
        ImGui::EndTable();
      }
      if (settings.mixerStyle==1) {
        ImVec2 pos=ImGui::GetCursorScreenPos(),
              size=ImVec2(ImGui::GetContentRegionAvail().x,ImGui::GetFontSize()+ImGui::GetStyle().FramePadding.y*2.0f);
        ImGui::Dummy(size);
        drawVolMeterInternal(ImGui::GetWindowDrawList(),ImRect(pos,pos+size),e->chipPeak[which],e->getDispatch(which)->getOutputCount(),true);
      }
    } else {
      if (ImGui::Checkbox("##ChipInvert",&doInvert)) {
        e->song.systemVol[which]=doInvert?-vol:vol;
        ret=true;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(_("Invert"));
      }
      // hack to get the same line from here
      ImGui::SameLine();
      ImVec2 curPos=ImGui::GetCursorPos();
      ImGui::NewLine();

      float volSliderHeight=size.y-ImGui::GetStyle().FramePadding.y*7-textHeight*2;

      VerticalText(volSliderHeight-(ImGui::GetCursorPosY()-curPos.y),true,"%s",e->getSystemName(e->song.system[which]));

      ImGui::SameLine();

      float vTextWidth=textHeight+2*ImGui::GetStyle().FramePadding.x;

      ImGui::SetCursorPos(curPos);
      ImVec2 pos=ImGui::GetCursorScreenPos();
      if (settings.mixerStyle==2) {
        drawVolMeterInternal(ImGui::GetWindowDrawList(),ImRect(pos,pos+ImVec2(size.x-vTextWidth,volSliderHeight)),e->chipPeak[which],e->getDispatch(which)->getOutputCount(),false);

        ImGui::PushStyleColor(ImGuiCol_FrameBg,0);
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive,0);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered,127<<IM_COL32_A_SHIFT);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,0);
      }
      if (ImGui::VSliderFloat("##ChipVol",ImVec2(size.x-vTextWidth,volSliderHeight),&vol,0.0f,2.0f)) {
        if (doInvert) {
          if (vol<0.0001) vol=0.0001;
        }
        if (vol<0) vol=0;
        if (vol>10) vol=10;
        e->song.systemVol[which]=doInvert?-vol:vol;
        ret=true;
      } rightClickable
      if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
        ImGui::SetTooltip(_("Volume"));
      }
      if (settings.mixerStyle==2) {
        ImGui::PopStyleVar(1);
        ImGui::PopStyleColor(3);
      } else if (settings.mixerStyle==1) {
        ImGui::SetCursorPos(curPos+ImVec2(size.x-vTextWidth+ImGui::GetStyle().FramePadding.x,0));
        pos=ImGui::GetCursorScreenPos();
        ImGui::Dummy(ImVec2(size.x-vTextWidth,volSliderHeight));
        drawVolMeterInternal(ImGui::GetWindowDrawList(),ImRect(pos,pos+ImVec2(size.x-vTextWidth,volSliderHeight)),e->chipPeak[which],e->getDispatch(which)->getOutputCount(),false);
      }
      float panSliderWidth=size.x+1.5f*ImGui::GetStyle().FramePadding.x+((settings.mixerStyle!=1)?0:size.x-vTextWidth+ImGui::GetStyle().FramePadding.x);
      ImGui::SetNextItemWidth(panSliderWidth);
      if (ImGui::SliderFloat("##ChipPan",&e->song.systemPan[which],-1.0f,1.0f)) {
        if (e->song.systemPan[which]<-1.0f) e->song.systemPan[which]=-1.0f;
        if (e->song.systemPan[which]>1.0f) e->song.systemPan[which]=1.0f;
        ret=true;
      } rightClickable
      if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
        ImGui::SetTooltip(_("Panning"));
      }

      ImGui::SetNextItemWidth(panSliderWidth);
      if (ImGui::SliderFloat("##ChipPanFR",&e->song.systemPanFR[which],-1.0f,1.0f)) {
        if (e->song.systemPanFR[which]<-1.0f) e->song.systemPanFR[which]=-1.0f;
        if (e->song.systemPanFR[which]>1.0f) e->song.systemPanFR[which]=1.0f;
        ret=true;
      } rightClickable
      if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
        ImGui::SetTooltip(_("Front/Rear"));
      }
    }

  ImGui::EndGroup();
  ImGui::PopID();
  return ret;
}
