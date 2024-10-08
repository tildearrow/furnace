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

#include "gui.h"
#include <fmt/printf.h>
#include "imgui.h"
#include "guiConst.h"

// TODO: memory safety
String disasmCmd(unsigned char* buf, size_t bufLen, unsigned int addr) {
  if (addr>=bufLen) return "???";

  if (buf[addr]<0xb4) {
    return fmt::sprintf("note %s",noteNames[buf[addr]]);
  } else switch (buf[addr]) {
    case 0xb4:
      return "note null";
      break;
    case 0xb5:
      return "off";
      break;
    case 0xb6:
      return "offrel";
      break;
    case 0xb7:
      return "mrel";
      break;
    case 0xb8:
      return fmt::sprintf("ins $%.2x",(int)buf[addr+1]);
      break;
    case 0xbe:
      return fmt::sprintf("pan $%x, $%x",(int)buf[addr+1],(int)buf[addr+2]);
      break;
    case 0xc0:
      return fmt::sprintf("preporta $%.2x",(int)buf[addr+1]);
      break;
    case 0xc2:
      return fmt::sprintf("vib %d, %d",(int)buf[addr+1],(int)buf[addr+2]);
      break;
    case 0xc3:
      return fmt::sprintf("vibrange %d",(int)buf[addr+1]);
      break;
    case 0xc4:
      return fmt::sprintf("vibshape %d",(int)buf[addr+1]);
      break;
    case 0xc5:
      return fmt::sprintf("pitch $%.2x",(int)buf[addr+1]);
      break;
    case 0xc6:
      return fmt::sprintf("arp %d, %d",(int)buf[addr+1],(int)buf[addr+2]);
      break;
    case 0xc7:
      return fmt::sprintf("vol $%.2x",(int)buf[addr+1]);
      break;
    case 0xc8:
      return fmt::sprintf("volslide %d",(int)((short)(buf[addr+1]|(buf[addr+2]<<8))));
      break;
    case 0xc9:
      return fmt::sprintf("porta %d, %d",(int)buf[addr+1],(int)buf[addr+2]);
      break;
    case 0xca:
      return fmt::sprintf("legato %d",(int)buf[addr+1]);
      break;
    case 0xe0: case 0xe1: case 0xe2: case 0xe3:
    case 0xe4: case 0xe5: case 0xe6: case 0xe7:
    case 0xe8: case 0xe9: case 0xea: case 0xeb:
    case 0xec: case 0xed: case 0xee: case 0xef:
      return fmt::sprintf("qwait (%d)",(int)(buf[addr]-0xe0));
      break;
    case 0xfc:
      return fmt::sprintf("waits %d",(int)(buf[addr+1]|(buf[addr+2]<<8)));
      break;
    case 0xfd:
      return fmt::sprintf("waitc %d",(int)buf[addr+1]);
      break;
    case 0xfe:
      return "wait 1";
      break;
    case 0xff:
      return "stop";
      break;
    default:
      return "ill";
      break;
  }
  return "TODO";
}

void FurnaceGUI::drawCSPlayer() {
  if (nextWindow==GUI_WINDOW_CS_PLAYER) {
    csPlayerOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!csPlayerOpen) return;
  if (ImGui::Begin("Command Stream Player",&csPlayerOpen,globalWinFlags,_("Command Stream Player"))) {
    if (ImGui::Button(_("Load"))) {
      openFileDialog(GUI_FILE_CMDSTREAM_OPEN);
    }
    ImGui::SameLine();
    if (ImGui::Button(_("Kill"))) {
      if (!e->killStream()) {
        showError(_("Kikai wa mou shindeiru!"));
      }
    }
    ImGui::SameLine();
    if (ImGui::Button(_("Burn Current Song"))) {
      SafeWriter* w=e->saveCommand();
      if (w!=NULL) {
        if (!e->playStream(w->getFinalBuf(),w->size())) {
          showError(e->getLastError());
          w->finish();
          delete w;
        } else {
          w->disown();
          delete w;
        }
      }
    }

    DivCSPlayer* cs=e->getStreamPlayer();
    if (cs) {
      if (ImGui::BeginTabBar("CSOptions")) {
        int chans=e->getTotalChannelCount();
        if (ImGui::BeginTabItem(_("Status"))) {
          if (ImGui::BeginTable("CSStat",12,ImGuiTableFlags_SizingFixedSame|ImGuiTableFlags_ScrollX|ImGuiTableFlags_Borders)) {
            ImGui::TableSetupScrollFreeze(1,1);
            ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
            ImGui::TableNextColumn();
            ImGui::Text(_("channel"));
            ImGui::TableNextColumn();
            ImGui::Text(_("start"));
            ImGui::TableNextColumn();
            ImGui::Text(_("PC"));
            ImGui::TableNextColumn();
            ImGui::Text(_("wait"));
            ImGui::TableNextColumn();
            ImGui::Text(_("SP"));
            ImGui::TableNextColumn();
            ImGui::Text(_("note"));
            ImGui::TableNextColumn();
            ImGui::Text(_("pitch"));
            ImGui::TableNextColumn();
            ImGui::Text(_("vol"));
            ImGui::TableNextColumn();
            ImGui::Text(_("vols"));
            ImGui::TableNextColumn();
            ImGui::Text(_("volst"));
            ImGui::TableNextColumn();
            ImGui::Text(_("vib"));
            ImGui::TableNextColumn();
            ImGui::Text(_("porta"));
            ImGui::TableNextColumn();
            ImGui::Text(_("arp"));

            for (int i=0; i<chans; i++) {
              DivCSChannelState* state=cs->getChanState(i);
              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              ImGui::Text("%d",i);
              ImGui::TableNextColumn();
              ImGui::Text("$%.4x",state->startPos);
              ImGui::TableNextColumn();
              ImGui::Text("$%.4x",state->readPos);
              ImGui::TableNextColumn();
              ImGui::Text("%d/%d",state->waitTicks,state->lastWaitLen);
              ImGui::TableNextColumn();
              ImGui::Text("%d",state->callStackPos);
              ImGui::TableNextColumn();
              ImGui::Text("%d",state->note);
              ImGui::TableNextColumn();
              ImGui::Text("%d",state->pitch);
              ImGui::TableNextColumn();
              ImGui::Text("$%.4X",state->volume);
              ImGui::TableNextColumn();
              ImGui::Text("%+d",state->volSpeed);
              ImGui::TableNextColumn();
              ImGui::Text("%+d",state->volSpeedTarget);
              ImGui::TableNextColumn();
              ImGui::Text("%d/%d (%d)",state->vibratoDepth,state->vibratoRate,state->vibratoPos);
              ImGui::TableNextColumn();
              ImGui::Text("-> %d (%d)",state->portaTarget,state->portaSpeed);
              ImGui::TableNextColumn();
              ImGui::Text("$%.2X",state->arp);
            }

            ImGui::EndTable();
          }
          ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(_("Trace"))) {
          ImGui::PushFont(patFont);
          if (ImGui::BeginTable("CSTrace",chans,ImGuiTableFlags_SizingFixedSame|ImGuiTableFlags_Borders|ImGuiTableFlags_ScrollX)) {
            char tempID[32];
            for (int i=0; i<chans; i++) {
              snprintf(tempID,31,"c%d",i);
              ImGui::TableSetupColumn(tempID,ImGuiTableColumnFlags_WidthFixed,200.0*dpiScale);
            }
            ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
            for (int i=0; i<chans; i++) {
              DivCSChannelState* state=cs->getChanState(i);
              ImGui::TableNextColumn();
              ImGui::Text("%d: $%.4x",i,state->readPos);
            }

            ImGui::TableNextRow();
            unsigned char* buf=cs->getData();
            size_t bufSize=cs->getDataLen();
            for (int i=0; i<chans; i++) {
              DivCSChannelState* state=cs->getChanState(i);
              ImGui::TableNextColumn();
              int maxItems=(ImGui::GetContentRegionAvail().y/MAX(ImGui::GetTextLineHeightWithSpacing(),1.0f));
              if (maxItems>=DIV_MAX_CSTRACE) maxItems=DIV_MAX_CSTRACE-1;

              int tracePos=state->tracePos;

              for (int j=(tracePos-maxItems)&(DIV_MAX_CSTRACE-1); j!=tracePos; j=(j+1)&(DIV_MAX_CSTRACE-1)) {
                if (state->trace[j]==0) {
                  ImGui::TextUnformatted("...");
                } else {
                  String dis=disasmCmd(buf,bufSize,state->trace[j]);
                  ImGui::Text("%.4x: %s",state->trace[j],dis.c_str());
                }
              }
            }

            ImGui::EndTable();
          }
          ImGui::PopFont();
          ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(_("Disassemble"))) {
          ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(_("Hex"))) {
          ImGui::PushFont(patFont);
          if (ImGui::BeginTable("CSHexPos",chans,ImGuiTableFlags_SizingStretchSame)) {
            ImGui::TableNextRow();
            for (int i=0; i<chans; i++) {
              ImGui::TableNextColumn();
              ImGui::Text("%d",i);
            }
            ImGui::TableNextRow();
            for (int i=0; i<chans; i++) {
              DivCSChannelState* state=cs->getChanState(i);
              ImGui::TableNextColumn();
              ImGui::Text("$%.4x",state->readPos);
            }
            ImGui::EndTable();
          }

          float oneCharSize=ImGui::CalcTextSize("A").x;
          float threeCharSize=ImGui::CalcTextSize("AA").x;
          float charViewSize=ImGui::CalcTextSize("0123456789ABCDEF").x;
          float fiveCharSize=ImGui::CalcTextSize("AAAAA").x;

          if (ImGui::BeginTable("CSHexView",19,ImGuiTableFlags_ScrollY)) {
            char charView[17];
            ImGui::TableSetupScrollFreeze(1,1);
            ImGui::TableSetupColumn("addr",ImGuiTableColumnFlags_WidthFixed,fiveCharSize);
            ImGui::TableSetupColumn("d0",ImGuiTableColumnFlags_WidthFixed,threeCharSize);
            ImGui::TableSetupColumn("d1",ImGuiTableColumnFlags_WidthFixed,threeCharSize);
            ImGui::TableSetupColumn("d2",ImGuiTableColumnFlags_WidthFixed,threeCharSize);
            ImGui::TableSetupColumn("d3",ImGuiTableColumnFlags_WidthFixed,threeCharSize);
            ImGui::TableSetupColumn("d4",ImGuiTableColumnFlags_WidthFixed,threeCharSize);
            ImGui::TableSetupColumn("d5",ImGuiTableColumnFlags_WidthFixed,threeCharSize);
            ImGui::TableSetupColumn("d6",ImGuiTableColumnFlags_WidthFixed,threeCharSize);
            ImGui::TableSetupColumn("d7",ImGuiTableColumnFlags_WidthFixed,threeCharSize);
            ImGui::TableSetupColumn("d8",ImGuiTableColumnFlags_WidthFixed,threeCharSize);
            ImGui::TableSetupColumn("d9",ImGuiTableColumnFlags_WidthFixed,threeCharSize);
            ImGui::TableSetupColumn("d10",ImGuiTableColumnFlags_WidthFixed,threeCharSize);
            ImGui::TableSetupColumn("d11",ImGuiTableColumnFlags_WidthFixed,threeCharSize);
            ImGui::TableSetupColumn("d12",ImGuiTableColumnFlags_WidthFixed,threeCharSize);
            ImGui::TableSetupColumn("d13",ImGuiTableColumnFlags_WidthFixed,threeCharSize);
            ImGui::TableSetupColumn("d14",ImGuiTableColumnFlags_WidthFixed,threeCharSize);
            ImGui::TableSetupColumn("d15",ImGuiTableColumnFlags_WidthFixed,threeCharSize);
            ImGui::TableSetupColumn("spacer",ImGuiTableColumnFlags_WidthFixed,oneCharSize);
            ImGui::TableSetupColumn("char",ImGuiTableColumnFlags_WidthFixed,charViewSize);

            // header
            ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
            ImGui::TableNextColumn();
            for (int i=0; i<16; i++) {
              ImGui::TableNextColumn();
              ImGui::Text("%X",i);
            }

            // content
            unsigned char* buf=cs->getData();
            size_t bufSize=cs->getDataLen();
            csClipper.Begin((bufSize+15)>>4,ImGui::GetTextLineHeightWithSpacing());
            while (csClipper.Step()) {
              //std::vector<int> highlightsUnsorted;
              std::vector<int> highlights;
              int nextHighlight=-1;
              int highlightPos=0;

              for (int i=0; i<chans; i++) {
                DivCSChannelState* state=cs->getChanState(i);
                if ((int)state->readPos>=(csClipper.DisplayStart<<4) && (int)state->readPos<=(csClipper.DisplayEnd<<4)) {
                  highlights.push_back(state->readPos);
                }
              }
              if (!highlights.empty()) nextHighlight=highlights[0];


              for (int i=csClipper.DisplayStart; i<csClipper.DisplayEnd; i++) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,ImGui::GetColorU32(ImGuiCol_TableHeaderBg));
                ImGui::Text("%.4X",i<<4);

                for (int j=0; j<16; j++) {
                  int pos=(i<<4)|j;
                  ImGui::TableNextColumn();
                  if (pos>=(int)bufSize) continue;
                  if (pos==nextHighlight) {
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,ImGui::GetColorU32(ImGuiCol_HeaderActive));
                    highlightPos++;
                    if (highlightPos>=(int)highlights.size()) {
                      nextHighlight=-1;
                    } else {
                      nextHighlight=highlights[highlightPos];
                    }
                  }
                  ImGui::Text("%.2X",buf[pos]);
                }

                ImGui::TableNextColumn();
                ImGui::TableNextColumn();
                for (int j=0; j<16; j++) {
                  int pos=(i<<4)|j;
                  if (pos>=(int)bufSize) {
                    charView[j]=' ';
                  } else if (buf[pos]>=0x20 && buf[pos]<=0x7e) {
                    charView[j]=buf[pos];
                  } else {
                    charView[j]='.';
                  }
                }
                charView[16]=0;

                ImGui::TextUnformatted(charView);
              }
            }
            csClipper.End();

            ImGui::EndTable();
          }
          ImGui::PopFont();
          ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
      }
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_CS_PLAYER;
  ImGui::End();
}
