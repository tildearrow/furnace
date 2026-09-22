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

void FurnaceGUI::alterSampleMap(int column, int val) {
  if (curIns<0 || curIns>=(int)e->song.ins.size()) return;
  DivInstrument* ins=e->song.ins[curIns];
  int sampleMapMin=sampleMapSelStart;
  int sampleMapMax=sampleMapSelEnd;
  if (sampleMapMin>sampleMapMax) {
    sampleMapMin^=sampleMapMax;
    sampleMapMax^=sampleMapMin;
    sampleMapMin^=sampleMapMax;
  }

  for (int i=sampleMapMin; i<=sampleMapMax; i++) {
    if (i<0 || i>=180) continue;

    if (sampleMapColumn==1 && column==1) {
      ins->amiga.noteMap[i].freq=val;
    } else if (sampleMapColumn==0 && column==0) {
      if (val<0) {
        ins->amiga.noteMap[i].map=-1;
      } else if (sampleMapDigit>0) {
        ins->amiga.noteMap[i].map*=10;
        ins->amiga.noteMap[i].map+=val;
      } else {
        ins->amiga.noteMap[i].map=val;
      }
      if (ins->amiga.noteMap[i].map>=(int)e->song.sample.size()) {
        ins->amiga.noteMap[i].map=((int)e->song.sample.size())-1;
      }
    } else if (sampleMapColumn==2 && column==2) {
      if (val<0) {
        ins->amiga.noteMap[i].dpcmFreq=-1;
      } else if (sampleMapDigit>0) {
        ins->amiga.noteMap[i].dpcmFreq*=10;
        ins->amiga.noteMap[i].dpcmFreq+=val;
      } else {
        ins->amiga.noteMap[i].dpcmFreq=val;
      }
      if (ins->amiga.noteMap[i].dpcmFreq>15) {
        ins->amiga.noteMap[i].dpcmFreq%=10;
      }
    } else if (sampleMapColumn==3 && column==3) {
      if (val<0) {
        ins->amiga.noteMap[i].dpcmDelta=-1;
      } else if (sampleMapDigit>0) {
        if (ins->amiga.noteMap[i].dpcmDelta>7) {

          ins->amiga.noteMap[i].dpcmDelta=val;
        } else {
          ins->amiga.noteMap[i].dpcmDelta<<=4;
          ins->amiga.noteMap[i].dpcmDelta+=val;
        }
      } else {
        ins->amiga.noteMap[i].dpcmDelta=val;
      }
    }
  }

  bool advance=false;
  if (sampleMapColumn==1 && column==1) {
    advance=true;
  } else if (sampleMapColumn==0 && column==0) {
    int digits=1;
    if (e->song.sample.size()>=10) digits=2;
    if (e->song.sample.size()>=100) digits=3;
    if (e->song.sample.size()>=1000) digits=4;
    if (e->song.sample.size()>=10000) digits=5;
    if (++sampleMapDigit>=digits) {
      sampleMapDigit=0;
      advance=true;
    }
  } else if (sampleMapColumn==2 && column==2) {
    if (++sampleMapDigit>=2) {
      sampleMapDigit=0;
      advance=true;
    }
  } else if (sampleMapColumn==3 && column==3) {
    if (++sampleMapDigit>=2) {
      sampleMapDigit=0;
      advance=true;
    }
  }

  if (advance && sampleMapMin==sampleMapMax) {
    sampleMapSelStart++;
    if (sampleMapSelStart>179) sampleMapSelStart=179;
    sampleMapSelEnd=sampleMapSelStart;
  }

  MARK_MODIFIED;
}


// the actual tab

void FurnaceGUI::insTabSample(DivInstrument* ins) {
  const char* sampleTabName=_("Sample");
  if (ins->type==DIV_INS_NES) sampleTabName=_("DPCM");
  if (ImGui::BeginTabItem(sampleTabName)) {
    if (ins->type==DIV_INS_NES && e->song.compatFlags.oldDPCM) {
      ImGui::Text(_("new DPCM features disabled (compatibility)!"));
      if (ImGui::Button(_("click here to enable them."))) {
        e->song.compatFlags.oldDPCM=false;
        MARK_MODIFIED;
      }
      ImGui::EndTabItem();
      return;
    }

    String sName;
    bool wannaOpenSMPopup=false;
    if (ins->amiga.initSample<0 || ins->amiga.initSample>=e->song.sampleLen) {
      sName=_("none selected");
    } else {
      sName=e->song.sample[ins->amiga.initSample]->name;
    }
    if (ins->type==DIV_INS_PCE ||
        ins->type==DIV_INS_MIKEY ||
        ins->type==DIV_INS_X1_010 ||
        ins->type==DIV_INS_SWAN ||
        ins->type==DIV_INS_AY ||
        ins->type==DIV_INS_AY8930 ||
        ins->type==DIV_INS_VRC6 ||
        ins->type==DIV_INS_SU ||
        ins->type==DIV_INS_NDS ||
        ins->type==DIV_INS_SUPERVISION ||
        ins->type==DIV_INS_SID3) {
      P(ImGui::Checkbox(_("Use sample"),&ins->amiga.useSample));
      if (ins->type==DIV_INS_X1_010) {
        if (ImGui::InputInt(_("Sample bank slot##BANKSLOT"),&ins->x1_010.bankSlot,1,4)) { PARAMETER
          if (ins->x1_010.bankSlot<0) ins->x1_010.bankSlot=0;
          if (ins->x1_010.bankSlot>=7) ins->x1_010.bankSlot=7;
          notifySampleChange=true;
        }
      }
    }
    ImGui::AlignTextToFramePadding();
    ImGui::Text(_("Sample"));
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::BeginCombo("##ISample",sName.c_str())) {
      String id;
      for (int i=0; i<e->song.sampleLen; i++) {
        id=fmt::sprintf("%d: %s",i,e->song.sample[i]->name);
        if (ImGui::Selectable(id.c_str(),ins->amiga.initSample==i)) { PARAMETER
          ins->amiga.initSample=i;
          notifySampleChange=true;
        }
      }
      ImGui::EndCombo();
    }
    // Wavetable
    if (ins->type==DIV_INS_AMIGA || ins->type==DIV_INS_SNES || ins->type==DIV_INS_GBA_DMA || ins->type==DIV_INS_GBA_MINMOD) {
      const char* useWaveText=ins->type==DIV_INS_AMIGA?_("Use wavetable (Amiga/Generic DAC only)"):_("Use wavetable");
      ImGui::BeginDisabled(ins->amiga.useNoteMap);
      P(ImGui::Checkbox(useWaveText,&ins->amiga.useWave));
      if (ins->amiga.useWave) {
        int len=ins->amiga.waveLen+1;
        int origLen=len;
        if (ImGui::InputInt(_("Width"),&len,2,16)) {
          if (ins->type==DIV_INS_SNES || ins->type==DIV_INS_GBA_DMA) {
            if (len<16) len=16;
            if (len>256) len=256;
            if (len>origLen) {
              ins->amiga.waveLen=((len+15)&(~15))-1;
            } else {
              ins->amiga.waveLen=(len&(~15))-1;
            }
          } else {
            if (len<2) len=2;
            if (len>256) len=256;
            ins->amiga.waveLen=(len&(~1))-1;
          }
          PARAMETER
        }
      }
      ImGui::EndDisabled();
    } else {
      ins->amiga.useWave=false;
    }
    // Note map
    ImGui::BeginDisabled(ins->amiga.useWave);
    P(ImGui::Checkbox(_("Use sample map"),&ins->amiga.useNoteMap));
    if ((ins->type==DIV_INS_MULTIPCM) && ImGui::IsItemHovered()) {
      ImGui::SetTooltip(_("Only for OPL4 PCM."));
    }
    if (ins->amiga.useNoteMap) {
      if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows)) sampleMapFocused=false;
      if (curWindowLast!=GUI_WINDOW_INS_EDIT) sampleMapFocused=false;
      if (!sampleMapFocused) sampleMapDigit=0;
      if (ImGui::BeginTable("NoteMap",(ins->type==DIV_INS_NES)?5:4,ImGuiTableFlags_ScrollY|ImGuiTableFlags_Borders|ImGuiTableFlags_SizingStretchSame)) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthFixed);
        if (ins->type==DIV_INS_NES) ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("c4",ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableSetupScrollFreeze(0,1);

        ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        ImGui::Text("#");
        if (ins->type==DIV_INS_NES) {
          ImGui::TableNextColumn();
          ImGui::Text(_("pitch"));
          ImGui::TableNextColumn();
          ImGui::Text(_("delta"));
        } else {
          ImGui::TableNextColumn();
          ImGui::Text(_("note"));
        }
        ImGui::TableNextColumn();
        ImGui::Text(_("sample name"));
        int sampleMapMin=sampleMapSelStart;
        int sampleMapMax=sampleMapSelEnd;
        if (sampleMapMin>sampleMapMax) {
          sampleMapMin^=sampleMapMax;
          sampleMapMax^=sampleMapMin;
          sampleMapMin^=sampleMapMax;
        }

        ImGui::PushStyleColor(ImGuiCol_Header,ImGui::GetColorU32(ImGuiCol_HeaderHovered));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive,ImGui::GetColorU32(ImGuiCol_HeaderHovered));
        for (int i=0; i<180; i++) {
          DivInstrumentAmiga::SampleMap& sampleMap=ins->amiga.noteMap[i];
          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,ImGui::GetColorU32(ImGuiCol_TableHeaderBg));
          ImGui::AlignTextToFramePadding();
          ImGui::Text("%s",noteNames[i]);
          ImGui::TableNextColumn();
          if (sampleMap.map<0 || sampleMap.map>=e->song.sampleLen) {
            sName=fmt::sprintf("-----##SM%d",i);
            sampleMap.map=-1;
          } else {
            sName=fmt::sprintf("%5d##SM%d",sampleMap.map,i);
          }
          ImGui::PushFont(patFont);
          ImGui::AlignTextToFramePadding();
          ImGui::SetNextItemWidth(ImGui::CalcTextSize("00000").x);
          ImGui::Selectable(sName.c_str(),(sampleMapWaitingInput && sampleMapColumn==0 && i>=sampleMapMin && i<=sampleMapMax));
          if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            sampleMapFocused=true;
            sampleMapColumn=0;
            sampleMapDigit=0;
            sampleMapSelStart=i;
            sampleMapSelEnd=i;

            sampleMapMin=sampleMapSelStart;
            sampleMapMax=sampleMapSelEnd;
            if (sampleMapMin>sampleMapMax) {
              sampleMapMin^=sampleMapMax;
              sampleMapMax^=sampleMapMin;
              sampleMapMin^=sampleMapMax;
            }
            ImGui::InhibitInertialScroll();
          }
          if (sampleMapFocused && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            sampleMapSelEnd=i;
          }
          if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
            if (sampleMapSelStart==sampleMapSelEnd) {
              sampleMapFocused=true;
              sampleMapColumn=0;
              sampleMapDigit=0;
              sampleMapSelStart=i;
              sampleMapSelEnd=i;

              sampleMapMin=sampleMapSelStart;
              sampleMapMax=sampleMapSelEnd;
              if (sampleMapMin>sampleMapMax) {
                sampleMapMin^=sampleMapMax;
                sampleMapMax^=sampleMapMin;
                sampleMapMin^=sampleMapMax;
              }
            }
            if (sampleMapFocused) {
              wannaOpenSMPopup=true;
            }
          }
          ImGui::PopFont();

          if (ins->type==DIV_INS_NES) {
            // pitch
            ImGui::TableNextColumn();
            ImGui::PushFont(patFont);
            ImGui::AlignTextToFramePadding();
            ImGui::SetNextItemWidth(ImGui::CalcTextSize("0000").x);
            if (sampleMap.dpcmFreq<0) {
              sName=fmt::sprintf(" -- ##SD1%d",i);
            } else {
              sName=fmt::sprintf(" %2d ##SD1%d",sampleMap.dpcmFreq,i);
            }
            ImGui::Selectable(sName.c_str(),(sampleMapWaitingInput && sampleMapColumn==2 && i>=sampleMapMin && i<=sampleMapMax));

            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
              sampleMapFocused=true;
              sampleMapColumn=2;
              sampleMapDigit=0;
              sampleMapSelStart=i;
              sampleMapSelEnd=i;

              sampleMapMin=sampleMapSelStart;
              sampleMapMax=sampleMapSelEnd;
              if (sampleMapMin>sampleMapMax) {
                sampleMapMin^=sampleMapMax;
                sampleMapMax^=sampleMapMin;
                sampleMapMin^=sampleMapMax;
              }
              ImGui::InhibitInertialScroll();
            }
            if (sampleMapFocused && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
              sampleMapSelEnd=i;
            }
            if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
              if (sampleMapSelStart==sampleMapSelEnd) {
                sampleMapFocused=true;
                sampleMapColumn=2;
                sampleMapDigit=0;
                sampleMapSelStart=i;
                sampleMapSelEnd=i;

                sampleMapMin=sampleMapSelStart;
                sampleMapMax=sampleMapSelEnd;
                if (sampleMapMin>sampleMapMax) {
                  sampleMapMin^=sampleMapMax;
                  sampleMapMax^=sampleMapMin;
                  sampleMapMin^=sampleMapMax;
                }
              }
              if (sampleMapFocused) {
                wannaOpenSMPopup=true;
              }
            }

            ImGui::PopFont();

            // delta
            ImGui::TableNextColumn();
            ImGui::PushFont(patFont);
            ImGui::AlignTextToFramePadding();
            ImGui::SetNextItemWidth(ImGui::CalcTextSize("0000").x);
            if (sampleMap.dpcmDelta<0) {
              sName=fmt::sprintf(" -- ##SD2%d",i);
            } else {
              sName=fmt::sprintf(" %2X ##SD2%d",sampleMap.dpcmDelta,i);
            }
            ImGui::Selectable(sName.c_str(),(sampleMapWaitingInput && sampleMapColumn==3 && i>=sampleMapMin && i<=sampleMapMax));

            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
              sampleMapFocused=true;
              sampleMapColumn=3;
              sampleMapDigit=0;
              sampleMapSelStart=i;
              sampleMapSelEnd=i;

              sampleMapMin=sampleMapSelStart;
              sampleMapMax=sampleMapSelEnd;
              if (sampleMapMin>sampleMapMax) {
                sampleMapMin^=sampleMapMax;
                sampleMapMax^=sampleMapMin;
                sampleMapMin^=sampleMapMax;
              }
              ImGui::InhibitInertialScroll();
            }
            if (sampleMapFocused && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
              sampleMapSelEnd=i;
            }
            if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
              if (sampleMapSelStart==sampleMapSelEnd) {
                sampleMapFocused=true;
                sampleMapColumn=3;
                sampleMapDigit=0;
                sampleMapSelStart=i;
                sampleMapSelEnd=i;

                sampleMapMin=sampleMapSelStart;
                sampleMapMax=sampleMapSelEnd;
                if (sampleMapMin>sampleMapMax) {
                  sampleMapMin^=sampleMapMax;
                  sampleMapMax^=sampleMapMin;
                  sampleMapMin^=sampleMapMax;
                }
              }
              if (sampleMapFocused) {
                wannaOpenSMPopup=true;
              }
            }

            ImGui::PopFont();
          } else {
            ImGui::TableNextColumn();
            sName="???";
            if ((sampleMap.freq)>=0 && (sampleMap.freq)<180) {
              sName=noteNames[sampleMap.freq];
            }
            sName+=fmt::sprintf("##SN%d",i);
            ImGui::PushFont(patFont);
            ImGui::AlignTextToFramePadding();
            ImGui::SetNextItemWidth(ImGui::CalcTextSize("00000").x);
            ImGui::Selectable(sName.c_str(),(sampleMapWaitingInput && sampleMapColumn==1 && i>=sampleMapMin && i<=sampleMapMax));
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
              sampleMapFocused=true;
              sampleMapColumn=1;
              sampleMapDigit=0;
              sampleMapSelStart=i;
              sampleMapSelEnd=i;

              sampleMapMin=sampleMapSelStart;
              sampleMapMax=sampleMapSelEnd;
              if (sampleMapMin>sampleMapMax) {
                sampleMapMin^=sampleMapMax;
                sampleMapMax^=sampleMapMin;
                sampleMapMin^=sampleMapMax;
              }
              ImGui::InhibitInertialScroll();
            }
            if (sampleMapFocused && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
              sampleMapSelEnd=i;
            }
            if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
              if (sampleMapSelStart==sampleMapSelEnd) {
                sampleMapFocused=true;
                sampleMapColumn=1;
                sampleMapDigit=0;
                sampleMapSelStart=i;
                sampleMapSelEnd=i;

                sampleMapMin=sampleMapSelStart;
                sampleMapMax=sampleMapSelEnd;
                if (sampleMapMin>sampleMapMax) {
                  sampleMapMin^=sampleMapMax;
                  sampleMapMax^=sampleMapMin;
                  sampleMapMin^=sampleMapMax;
                }
              }
              if (sampleMapFocused) {
                wannaOpenSMPopup=true;
              }
            }
            ImGui::PopFont();
          }

          ImGui::TableNextColumn();
          String prevName="---";
          if (sampleMap.map>=0 && sampleMap.map<e->song.sampleLen) {
            prevName=e->song.sample[sampleMap.map]->name;
          }
          ImGui::PushID(i+2);
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::BeginCombo("##SMSample",prevName.c_str())) {
            if (ImGui::Selectable("---",sampleMap.map==-1)) {
              sampleMap.map=-1;
            }
            for (int k=0; k<e->song.sampleLen; k++) {
              String itemName=fmt::sprintf("%d: %s",k,e->song.sample[k]->name);
              if (ImGui::Selectable(itemName.c_str(),sampleMap.map==k)) {
                sampleMap.map=k;
              }
            }
            ImGui::EndCombo();
          }
          ImGui::PopID();
        }
        ImGui::PopStyleColor(2);
        ImGui::EndTable();
      }
    } else {
      sampleMapFocused=false;
    }
    ImGui::EndDisabled();
    if (wannaOpenSMPopup) {
      ImGui::OpenPopup("SampleMapUtils");
    }
    if (ImGui::BeginPopup("SampleMapUtils",ImGuiWindowFlags_NoMove|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoSavedSettings)) {
      if (sampleMapSelStart==sampleMapSelEnd && sampleMapSelStart>=0 && sampleMapSelStart<180) {
        if (ins->type==DIV_INS_NES) {
          if (ImGui::MenuItem(_("set entire map to this pitch"))) {
            if (sampleMapSelStart>=0 && sampleMapSelStart<180) {
              for (int i=0; i<180; i++) {
                if (i==sampleMapSelStart) continue;
                ins->amiga.noteMap[i].dpcmFreq=ins->amiga.noteMap[sampleMapSelStart].dpcmFreq;
              }
            }
          }
          if (ImGui::MenuItem(_("set entire map to this delta counter value"))) {
            if (sampleMapSelStart>=0 && sampleMapSelStart<180) {
              for (int i=0; i<180; i++) {
                if (i==sampleMapSelStart) continue;
                ins->amiga.noteMap[i].dpcmDelta=ins->amiga.noteMap[sampleMapSelStart].dpcmDelta;
              }
            }
          }
        } else {
          if (ImGui::MenuItem(_("set entire map to this note"))) {
            if (sampleMapSelStart>=0 && sampleMapSelStart<180) {
              for (int i=0; i<180; i++) {
                if (i==sampleMapSelStart) continue;
                ins->amiga.noteMap[i].freq=ins->amiga.noteMap[sampleMapSelStart].freq;
              }
            }
          }
        }
        if (ImGui::MenuItem(_("set entire map to this sample"))) {
          if (sampleMapSelStart>=0 && sampleMapSelStart<180) {
            for (int i=0; i<180; i++) {
              if (i==sampleMapSelStart) continue;
              ins->amiga.noteMap[i].map=ins->amiga.noteMap[sampleMapSelStart].map;
            }
          }
        }
      }
      if (ins->type==DIV_INS_NES) {
        if (ImGui::MenuItem(_("reset pitches"))) {
          for (int i=0; i<180; i++) {
            ins->amiga.noteMap[i].dpcmFreq=15;
          }
        }
        if (ImGui::MenuItem(_("clear delta counter values"))) {
          for (int i=0; i<180; i++) {
            ins->amiga.noteMap[i].dpcmDelta=-1;
          }
        }
      } else {
        if (ImGui::MenuItem(_("reset notes"))) {
          for (int i=0; i<180; i++) {
            ins->amiga.noteMap[i].freq=i;
          }
        }
      }
      if (ImGui::MenuItem(_("clear map samples"))) {
        for (int i=0; i<180; i++) {
          ins->amiga.noteMap[i].map=-1;
        }
      }
      ImGui::EndPopup();
    }
    ImGui::EndTabItem();
  } else {
    sampleMapFocused=false;
  }
}

