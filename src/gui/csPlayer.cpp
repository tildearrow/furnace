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
#include <fmt/printf.h>
#include "imgui.h"
#include "guiConst.h"

String disasmCmd(unsigned char* buf, size_t bufLen, unsigned int addr, unsigned char* speedDial) {
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
      if (addr+1>=bufLen) return "???";
      return fmt::sprintf("ins $%.2x",(int)buf[addr+1]);
      break;
    case 0xc0:
      if (addr+1>=bufLen) return "???";
      return fmt::sprintf("preporta $%.2x",(int)buf[addr+1]);
      break;
    case 0xc2:
      if (addr+1>=bufLen) return "???";
      return fmt::sprintf("vib $%.2x",(int)buf[addr+1]);
      break;
    case 0xc3:
      if (addr+1>=bufLen) return "???";
      return fmt::sprintf("vibrange %d",(int)buf[addr+1]);
      break;
    case 0xc4:
      if (addr+1>=bufLen) return "???";
      return fmt::sprintf("vibshape %d",(int)buf[addr+1]);
      break;
    case 0xc5:
      if (addr+1>=bufLen) return "???";
      return fmt::sprintf("pitch $%.2x",(int)buf[addr+1]);
      break;
    case 0xc6:
      if (addr+1>=bufLen) return "???";
      return fmt::sprintf("arp $%.2x",(int)buf[addr+1]);
      break;
    case 0xc7:
      if (addr+1>=bufLen) return "???";
      return fmt::sprintf("vol $%.2x",(int)buf[addr+1]);
      break;
    case 0xc8:
      if (addr+2>=bufLen) return "???";
      return fmt::sprintf("volslide %d",(int)((short)(buf[addr+1]|(buf[addr+2]<<8))));
      break;
    case 0xc9:
      if (addr+2>=bufLen) return "???";
      return fmt::sprintf("porta %d, %d",(int)buf[addr+1],(int)buf[addr+2]);
      break;
    case 0xca:
      if (addr+1>=bufLen) return "???";
      return fmt::sprintf("legato %d",(int)buf[addr+1]);
      break;
    case 0xcb:
      if (addr+4>=bufLen) return "???";
      return fmt::sprintf("volporta %d, %d",(int)((short)(buf[addr+1]|(buf[addr+2]<<8))),(int)((short)(buf[addr+3]|(buf[addr+4]<<8))));
      break;
    case 0xcc:
      if (addr+1>=bufLen) return "???";
      return fmt::sprintf("tremolo $%.2x",(int)buf[addr+1]);
      break;
    case 0xcd:
      if (addr+1>=bufLen) return "???";
      return fmt::sprintf("panbrello $%.2x",(int)buf[addr+1]);
      break;
    case 0xce:
      if (addr+1>=bufLen) return "???";
      return fmt::sprintf("panslide %d",(signed char)buf[addr+1]);
      break;
    case 0xcf:
      if (addr+2>=bufLen) return "???";
      return fmt::sprintf("pan $%x, $%x",(int)buf[addr+1],(int)buf[addr+2]);
      break;
    case 0xe0: case 0xe1: case 0xe2: case 0xe3:
    case 0xe4: case 0xe5: case 0xe6: case 0xe7:
    case 0xe8: case 0xe9: case 0xea: case 0xeb:
    case 0xec: case 0xed: case 0xee: case 0xef: {
      unsigned char cmd=speedDial[buf[addr]&15];
      int cmdLen=DivCS::getCmdLength(cmd);
      if ((addr+cmdLen)>=bufLen) return "???";
      String ret=fmt::sprintf("qcmd%d %s",buf[addr]-0xd0,(cmd<DIV_CMD_MAX)?cmdName[cmd]:"INVALID");
      for (int i=0; i<cmdLen; i++) {
        ret+=fmt::sprintf(", %.2x",buf[addr+1+i]);
      }
      return ret;
      break;
    }
    case 0xf0: case 0xf1: case 0xf2: case 0xf3:
    case 0xf4: case 0xf5: case 0xf6: case 0xf7:
    case 0xf8: case 0xf9: case 0xfa: case 0xfb:
    case 0xfc: case 0xfd: case 0xfe: case 0xff:
      return fmt::sprintf("qwait (%d)",(int)(buf[addr]-0xe0));
      break;
    case 0xd0:
      if (addr+3>=bufLen) return "???";
      return fmt::sprintf("opt $%.2x%.2x%.2x",(int)buf[addr+1],(int)buf[addr+2],(int)buf[addr+3]);
      break;
    case 0xd1:
      return "nop";
      break;
    case 0xd4:
      if (addr+2>=bufLen) return "???";
      return fmt::sprintf("callsym $%.4x",(int)(buf[addr+1]|(buf[addr+2]<<8)));
      break;
    case 0xd5:
      if (addr+4>=bufLen) return "???";
      return fmt::sprintf("call $%.8x",(unsigned int)(buf[addr+1]|(buf[addr+2]<<8)|(buf[addr+3]<<16)|(buf[addr+4]<<24)));
      break;
    case 0xd6:
      return "offwait";
      break;
    case 0xd7: {
      if (addr+1>=bufLen) return "???";
      int cmdLen=DivCS::getCmdLength(buf[addr+1]);
      if ((addr+1+cmdLen)>=bufLen) return "???";
      String ret=fmt::sprintf("cmd %s",(buf[addr+1]<DIV_CMD_MAX)?cmdName[buf[addr+1]]:"INVALID");
      for (int i=0; i<cmdLen; i++) {
        ret+=fmt::sprintf(", %.2x",buf[addr+2+i]);
      }
      return ret;
      break;
    }
    case 0xd8:
      if (addr+2>=bufLen) return "???";
      return fmt::sprintf("call $%.4x",(unsigned int)(buf[addr+1]|(buf[addr+2]<<8)));
      break;
    case 0xd9:
      return "ret";
      break;
    case 0xda:
      return fmt::sprintf("jmp $%.8x",(unsigned int)(buf[addr+1]|(buf[addr+2]<<8)|(buf[addr+3]<<16)|(buf[addr+4]<<24)));
      break;
    case 0xdb:
      return fmt::sprintf("rate $%.8x",(unsigned int)(buf[addr+1]|(buf[addr+2]<<8)|(buf[addr+3]<<16)|(buf[addr+4]<<24)));
      break;
    case 0xdc:
      if (addr+2>=bufLen) return "???";
      return fmt::sprintf("waits %d",(int)(buf[addr+1]|(buf[addr+2]<<8)));
      break;
    case 0xdd:
      if (addr+1>=bufLen) return "???";
      return fmt::sprintf("waitc %d",(int)buf[addr+1]);
      break;
    case 0xde:
      return "wait 1";
      break;
    case 0xdf:
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
      e->killStream();
      exportCmdStream(true,"");
    }
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
      ImGui::OpenPopup("CSOptions");
    }
    ImGui::SameLine();
    if (e->isHalted()) {
      if (ImGui::Button("Resume")) e->resume();
    } else {
      if (ImGui::Button("Pause")) e->halt();
    }
    ImGui::SameLine();
    ImGui::Button(_("Burn Options"));
    if (ImGui::BeginPopupContextItem("CSOptions",ImGuiPopupFlags_MouseButtonLeft)) {
      commandExportOptions();
      ImGui::EndPopup();
    }

    DivCSPlayer* cs=e->getStreamPlayer();
    if (cs) {
      if (ImGui::BeginTabBar("CSOptions")) {
        int chans=e->getTotalChannelCount();
        if (ImGui::BeginTabItem(_("Status"))) {
          if (ImGui::BeginTable("CSStat",13,ImGuiTableFlags_SizingStretchSame|ImGuiTableFlags_ScrollX|ImGuiTableFlags_Borders)) {
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
              ImGui::Text("%d: $%.4x (>>%d)",i,state->readPos,state->callStackPos);
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
                  String dis=disasmCmd(buf,bufSize,state->trace[j],cs->getFastCmds());
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
          bool mustProceed=false;
          ImGui::AlignTextToFramePadding();
          ImGui::Text("Address");
          ImGui::SameLine();
          ImGui::InputScalar("##DisAsmAddr",ImGuiDataType_U32,&csDisAsmAddr,0,0,"%.8X",ImGuiInputTextFlags_CharsHexadecimal);
          ImGui::SameLine();
          if (ImGui::Button("Go")) {
            mustProceed=true;
          }

          if (mustProceed) {
            csDisAsm.clear();
            unsigned char* buf=cs->getData();
            for (size_t i=csDisAsmAddr; i<cs->getDataLen();) {
              int insLen=DivCS::getInsLength(buf[i],(((i+1)<cs->getDataLen())?buf[i+1]:0),cs->getFastCmds());
              if (insLen<1) {
                logE("INS %x NOT IMPLEMENTED...",buf[i]);
                break;
              }

              CSDisAsmIns ins;
              ins.addr=i;
              memcpy(ins.data,&buf[i],insLen);
              ins.len=insLen;
              csDisAsm.push_back(ins);

              i+=insLen;
            }
          }
          
          if (!csDisAsm.empty()) {
            ImGui::PushFont(patFont);
            if (ImGui::BeginTable("CSDisAsm",chans,ImGuiTableFlags_SizingFixedFit|ImGuiTableFlags_ScrollY)) {
              ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed,oneChar.x*2.0f);
              ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed,oneChar.x*9.0f);
              ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthFixed,oneChar.x*3.0f*6.0f);
              ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch);
              ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
              ImGui::TableNextColumn();
              ImGui::Text("pc");
              ImGui::TableNextColumn();
              ImGui::Text("addr");
              ImGui::TableNextColumn();
              ImGui::Text("hex");
              ImGui::TableNextColumn();
              ImGui::Text("ins");

              for (CSDisAsmIns& i: csDisAsm) {
                ImGui::TableNextRow(0,oneChar.y);
                ImGui::TableNextColumn();
                // this is the "PC is here row"...


                ImGui::TableNextColumn();
                ImGui::Text("%.8x",i.addr);

                ImGui::TableNextColumn();
                for (int j=0; j<i.len; j++) {
                  ImGui::Text("%.2x",i.data[j]);
                  ImGui::SameLine();
                }

                ImGui::TableNextColumn();
                String dis=disasmCmd(i.data,8,0,cs->getFastCmds());
                ImGui::Text("%s",dis.c_str());

                // jmp/ret separator
                if (i.data[0]==0xd9 || i.data[0]==0xda) {
                  ImGui::TableNextRow(0,oneChar.y);
                  ImGui::TableNextColumn();
                  ImGui::Separator();
                  ImGui::TableNextColumn();
                  ImGui::Separator();
                  ImGui::TableNextColumn();
                  ImGui::Separator();
                  ImGui::TableNextColumn();
                  ImGui::Separator();
                }
              }

              ImGui::EndTable();
            }
            ImGui::PopFont();
          }
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
            unsigned short* accessTS=cs->getDataAccess();
            unsigned int csTick=cs->getCurTick();
            const float fadeTime=64.0f;
            size_t bufSize=cs->getDataLen();
            csClipper.Begin((bufSize+15)>>4,ImGui::GetTextLineHeightWithSpacing());
            while (csClipper.Step()) {
              for (int i=csClipper.DisplayStart; i<csClipper.DisplayEnd; i++) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,ImGui::GetColorU32(ImGuiCol_TableHeaderBg));
                ImGui::Text("%.4X",i<<4);

                for (int j=0; j<16; j++) {
                  int pos=(i<<4)|j;
                  ImGui::TableNextColumn();
                  if (pos>=(int)bufSize) continue;
                  float cellAlpha=(float)(fadeTime-(((short)(csTick&0xffff))-(short)accessTS[pos]))/fadeTime;
                  if (cellAlpha>0.0f) {
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,ImGui::GetColorU32(ImGuiCol_HeaderActive,cellAlpha));
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
        if (ImGui::BeginTabItem(_("Stream Info"))) {
          ImGui::Text("%d bytes",(int)cs->getDataLen());
          ImGui::Text("%u channels",cs->getFileChans());
          ImGui::Text("preset delays:");
          for (int i=0; i<16; i++) {
            ImGui::SameLine();
            ImGui::Text("%d",cs->getFastDelays()[i]);
          }
          ImGui::Text("speed dial commands:");
          for (int i=0; i<16; i++) {
            ImGui::SameLine();
            ImGui::Text("%d",cs->getFastCmds()[i]);
          }
          ImGui::Text("ticks: %u",cs->getCurTick());
          ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(_("Call Stack"))) {
          ImGui::PushFont(patFont);
          if (ImGui::BeginTable("CSCallStack",chans,ImGuiTableFlags_SizingFixedFit|ImGuiTableFlags_ScrollX)) {
            char tempID[32];
            for (int i=0; i<chans; i++) {
              snprintf(tempID,31,"c%d",i);
              ImGui::TableSetupColumn(tempID,ImGuiTableColumnFlags_WidthFixed,oneChar.x*10.0f);
            }
            ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
            for (int i=0; i<chans; i++) {
              DivCSChannelState* state=cs->getChanState(i);
              ImGui::TableNextColumn();
              ImGui::Text("%d (>>%d)",i,state->callStackPos);
            }
            ImGui::TableNextRow();
            for (int i=0; i<chans; i++) {
              DivCSChannelState* state=cs->getChanState(i);
              ImGui::TableNextColumn();
              for (int j=0; j<state->callStackPos; j++) {
                ImGui::Text("$%.4x",state->callStack[j]);
              }
              ImGui::Text("$%.4x",state->readPos);
            }
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
