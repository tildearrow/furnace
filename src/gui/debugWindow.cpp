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
#include "guiConst.h"
#include "debug.h"
#include "IconsFontAwesome4.h"
#include <SDL_timer.h>
#include <fmt/printf.h>
#include "imgui.h"
#include "imgui_internal.h"

PendingDrawOsc _debugDo;
static float oscDebugData[2048];
static int oscDebugLen=800;
static int oscDebugHeight=400;
static float oscDebugLineSize=1.0;
static float oscDebugMin=-1.0;
static float oscDebugMax=1.0;
static float oscDebugPower=1.0;
static int oscDebugRepeat=1;
static int numApples=1;
static int getGainChan=0;
static int getGainVol=0;

static void _drawOsc(const ImDrawList* drawList, const ImDrawCmd* cmd) {
  if (cmd!=NULL) {
    if (cmd->UserCallbackData!=NULL) {
      ((FurnaceGUI*)(((PendingDrawOsc*)cmd->UserCallbackData)->gui))->runPendingDrawOsc((PendingDrawOsc*)cmd->UserCallbackData);
    }
  }
}

void FurnaceGUI::drawDebug() {
  static int bpOrder;
  static int bpRow;
  static int bpTick;
  static bool bpOn;

  static double ptcClock;
  static double ptcDivider;
  static int ptcOctave;
  static int ptcMode;
  static int ptcBlockBits;
  if (nextWindow==GUI_WINDOW_DEBUG) {
    debugOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!debugOpen) return;
  ImGui::SetNextWindowSizeConstraints(ImVec2(100.0f*dpiScale,100.0f*dpiScale),ImVec2(canvasW,canvasH));
  if (ImGui::Begin("Debug",&debugOpen,globalWinFlags|ImGuiWindowFlags_NoDocking,_("Debug"))) {
    ImGui::Text("NOTE: use with caution.");
    if (ImGui::TreeNode("Debug Controls")) {
      if (e->isHalted()) {
        if (ImGui::Button("Resume")) e->resume();
      } else {
        if (ImGui::Button("Pause")) e->halt();
      }
      ImGui::SameLine();
      if (ImGui::Button("Frame Advance")) e->haltWhen(DIV_HALT_TICK);
      ImGui::SameLine();
      if (ImGui::Button("Row Advance")) e->haltWhen(DIV_HALT_ROW);
      ImGui::SameLine();
      if (ImGui::Button("Pattern Advance")) e->haltWhen(DIV_HALT_PATTERN);

      if (ImGui::Button("Play Command Stream")) nextWindow=GUI_WINDOW_CS_PLAYER;

      if (ImGui::Button("Panic")) e->syncReset();
      ImGui::SameLine();
      if (ImGui::Button("Abort")) {
        abort();
      }
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("Breakpoint")) {
      ImGui::InputInt("Order",&bpOrder);
      ImGui::InputInt("Row",&bpRow);
      ImGui::InputInt("Tick",&bpTick);
      ImGui::Checkbox("Enable",&bpOn);
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("Chip Status")) {
      ImGui::Text("for best results set latency to minimum or use the Frame Advance button.");
      ImGui::Columns(e->song.systemLen);
      for (int i=0; i<e->song.systemLen; i++) {
        void* ch=e->getDispatch(i);
        ImGui::TextColored(uiColors[GUI_COLOR_ACCENT_PRIMARY],"Chip %d: %s",i,getSystemName(e->song.system[i]));
        if (e->song.system[i]==DIV_SYSTEM_NULL) {
          ImGui::Text("NULL");
        } else {
          putDispatchChip(ch,e->song.system[i]);
        }
        ImGui::NextColumn();
      }
      ImGui::Columns();
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("Dispatch Status")) {
      ImGui::Text("for best results set latency to minimum or use the Frame Advance button.");
      ImGui::Columns(e->getTotalChannelCount());
      for (int i=0; i<e->getTotalChannelCount(); i++) {
        void* ch=e->getDispatchChanState(i);
        ImGui::TextColored(uiColors[GUI_COLOR_ACCENT_PRIMARY],"Ch. %d: %d, %d",i,e->dispatchOfChan[i],e->dispatchChanOfChan[i]);
        if (ch==NULL) {
          ImGui::Text("NULL");
        } else {
          putDispatchChan(ch,e->dispatchChanOfChan[i],e->sysOfChan[i]);
        }
        ImGui::NextColumn();
      }
      ImGui::Columns();
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("Channel Status")) {
      ImGui::Text("for best results set latency to minimum or use the Frame Advance button.");
      ImGui::Columns(e->getTotalChannelCount());
      for (int i=0; i<e->getTotalChannelCount(); i++) {
        DivChannelState* ch=e->getChanState(i);
        ImGui::TextColored(uiColors[GUI_COLOR_ACCENT_PRIMARY],"Channel %d:",i);
        if (ch==NULL) {
          ImGui::Text("NULL");
        } else {
          ImGui::Text("* General:");
          ImGui::Text("- note = %d",ch->note);
          ImGui::Text("- oldNote = %d",ch->oldNote);
          ImGui::Text("- pitch = %d",ch->pitch);
          ImGui::Text("- portaSpeed = %d",ch->portaSpeed);
          ImGui::Text("- portaNote = %d",ch->portaNote);
          ImGui::Text("- volume = %.4x",ch->volume);
          ImGui::Text("- volSpeed = %d",ch->volSpeed);
          ImGui::Text("- volSpeedTarget = %d",ch->volSpeedTarget);
          ImGui::Text("- cut = %d",ch->cut);
          ImGui::Text("- rowDelay = %d",ch->rowDelay);
          ImGui::Text("- volMax = %.4x",ch->volMax);
          ImGui::Text("- delayOrder = %d",ch->delayOrder);
          ImGui::Text("- delayRow = %d",ch->delayRow);
          ImGui::Text("- retrigSpeed = %d",ch->retrigSpeed);
          ImGui::Text("- retrigTick = %d",ch->retrigTick);
          ImGui::PushStyleColor(ImGuiCol_Text,(ch->vibratoDepth>0)?uiColors[GUI_COLOR_MACRO_VOLUME]:uiColors[GUI_COLOR_TEXT]);
          ImGui::Text("* Vibrato:");
          ImGui::Text("- depth = %d",ch->vibratoDepth);
          ImGui::Text("- rate = %d",ch->vibratoRate);
          ImGui::Text("- pos = %d",ch->vibratoPos);
          ImGui::Text("- shape = %d",ch->vibratoShape);
          ImGui::Text("- fine = %d",ch->vibratoFine);
          ImGui::PopStyleColor();
          ImGui::PushStyleColor(ImGuiCol_Text,(ch->tremoloDepth>0)?uiColors[GUI_COLOR_MACRO_VOLUME]:uiColors[GUI_COLOR_TEXT]);
          ImGui::Text("* Tremolo:");
          ImGui::Text("- depth = %d",ch->tremoloDepth);
          ImGui::Text("- rate = %d",ch->tremoloRate);
          ImGui::Text("- pos = %d",ch->tremoloPos);
          ImGui::PopStyleColor();
          ImGui::PushStyleColor(ImGuiCol_Text,(ch->arp>0)?uiColors[GUI_COLOR_MACRO_VOLUME]:uiColors[GUI_COLOR_TEXT]);
          ImGui::Text("* Arpeggio:");
          ImGui::Text("- arp = %.2X",ch->arp);
          ImGui::Text("- stage = %d",ch->arpStage);
          ImGui::Text("- ticks = %d",ch->arpTicks);
          ImGui::PopStyleColor();
          ImGui::Text("* Miscellaneous:");
          ImGui::TextColored(ch->doNote?uiColors[GUI_COLOR_MACRO_VOLUME]:uiColors[GUI_COLOR_HEADER],">> Do Note");
          ImGui::TextColored(ch->legato?uiColors[GUI_COLOR_MACRO_VOLUME]:uiColors[GUI_COLOR_HEADER],">> Legato");
          ImGui::TextColored(ch->portaStop?uiColors[GUI_COLOR_MACRO_VOLUME]:uiColors[GUI_COLOR_HEADER],">> PortaStop");
          ImGui::TextColored(ch->keyOn?uiColors[GUI_COLOR_MACRO_VOLUME]:uiColors[GUI_COLOR_HEADER],">> Key On");
          ImGui::TextColored(ch->keyOff?uiColors[GUI_COLOR_MACRO_VOLUME]:uiColors[GUI_COLOR_HEADER],">> Key Off");
          ImGui::TextColored(ch->nowYouCanStop?uiColors[GUI_COLOR_MACRO_VOLUME]:uiColors[GUI_COLOR_HEADER],">> NowYouCanStop");
          ImGui::TextColored(ch->stopOnOff?uiColors[GUI_COLOR_MACRO_VOLUME]:uiColors[GUI_COLOR_HEADER],">> Stop on Off");
          ImGui::TextColored(ch->arpYield?uiColors[GUI_COLOR_MACRO_VOLUME]:uiColors[GUI_COLOR_HEADER],">> Arp Yield");
          ImGui::TextColored(ch->delayLocked?uiColors[GUI_COLOR_MACRO_VOLUME]:uiColors[GUI_COLOR_HEADER],">> DelayLocked");
          ImGui::TextColored(ch->inPorta?uiColors[GUI_COLOR_MACRO_VOLUME]:uiColors[GUI_COLOR_HEADER],">> InPorta");
          ImGui::TextColored(ch->scheduledSlideReset?uiColors[GUI_COLOR_MACRO_VOLUME]:uiColors[GUI_COLOR_HEADER],">> SchedSlide");
        }
        ImGui::NextColumn();
      }
      ImGui::Columns();
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("Playback Status")) {
      String pdi=e->getPlaybackDebugInfo();
      ImGui::TextWrapped("%s",pdi.c_str());
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("Sample Debug")) {
      for (int i=0; i<e->song.sampleLen; i++) {
        DivSample* sample=e->getSample(i);
        if (sample==NULL) {
          ImGui::Text("%d: <NULL!>",i);
          continue;
        }
        if (ImGui::TreeNode(fmt::sprintf("%d: %s",i,sample->name).c_str())) {
          ImGui::Text("rate: %d",sample->rate);
          ImGui::Text("centerRate: %d",sample->centerRate);
          ImGui::Text("loopStart: %d",sample->loopStart);
          ImGui::Text("loopEnd: %d", sample->loopEnd);
          ImGui::Text(sample->loop?"loop: Enabled":"loop: Disabled");
          if (sampleLoopModes[sample->loopMode]!=NULL) {
            ImGui::Text("loopMode: %d (%s)",(unsigned char)sample->loopMode,sampleLoopModes[sample->loopMode]);
          } else {
            ImGui::Text("loopMode: %d (<NULL!>)",(unsigned char)sample->loopMode);
          }

          ImGui::Text("depth: %d",(unsigned char)sample->depth);
          ImGui::Text("length8: %d",sample->length8);
          ImGui::Text("length16: %d",sample->length16);
          ImGui::Text("length1: %d",sample->length1);
          ImGui::Text("lengthDPCM: %d",sample->lengthDPCM);
          ImGui::Text("lengthZ: %d",sample->lengthZ);
          ImGui::Text("lengthQSoundA: %d",sample->lengthQSoundA);
          ImGui::Text("lengthA: %d",sample->lengthA);
          ImGui::Text("lengthB: %d",sample->lengthB);
          ImGui::Text("lengthBRR: %d",sample->lengthBRR);
          ImGui::Text("lengthVOX: %d",sample->lengthVOX);

          ImGui::Text("samples: %d",sample->samples);
          ImGui::TreePop();
        }
      }
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("Oscilloscope Debug")) {
      int c=0;
      ImGui::Checkbox("FFT debug view",&debugFFT);
      for (int i=0; i<e->song.systemLen; i++) {
        DivSystem system=e->song.system[i];
        if (e->getChannelCount(system)>0) {
          if (ImGui::TreeNode(fmt::sprintf("%d: %s",i,e->getSystemName(system)).c_str())) {
            if (ImGui::BeginTable("OscilloscopeTable",5,ImGuiTableFlags_Borders|ImGuiTableFlags_SizingStretchSame)) {
              ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
              ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed);
              ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch);
              ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthStretch);
              ImGui::TableSetupColumn("c4",ImGuiTableColumnFlags_WidthStretch);

              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              ImGui::Text("Channel");
              ImGui::TableNextColumn();
              ImGui::Text("Follow");
              ImGui::TableNextColumn();
              ImGui::Text("Needle");
              ImGui::TableNextColumn();
              ImGui::Text("Data");
              ImGui::TableNextColumn();
              ImGui::Text("I am so bored!");

              for (int j=0; j<e->getChannelCount(system); j++, c++) {
                DivDispatchOscBuffer* oscBuf=e->getOscBuffer(c);
                if (oscBuf==NULL) {
                  ImGui::TableNextRow();
                  // channel
                  ImGui::TableNextColumn();
                  ImGui::Text("%d",j);
                  ImGui::TableNextColumn();
                  ImGui::Text("<NULL!>");
                  continue;
                }
                ImGui::TableNextRow();
                // channel
                ImGui::TableNextColumn();
                ImGui::Text("%d",j);
                // follow
                ImGui::TableNextColumn();
                ImGui::Checkbox(fmt::sprintf("##%d_OSCFollow_%d",i,c).c_str(),&oscBuf->follow);
                // address
                ImGui::TableNextColumn();
                unsigned int needle=oscBuf->needle;
                ImGui::Text("%.8x",needle);
                // data
                ImGui::TableNextColumn();
                ImGui::Text("%d",oscBuf->data[needle>>16]);
                // save
                ImGui::TableNextColumn();
                if (ImGui::Button(fmt::sprintf("Help me!##%d_OSCSaveDamnIt",j).c_str())) {
                  FILE* f=fopen("/tmp/oscbuf.bin","wb");
                  if (f==NULL) {
                    showError(fmt::sprintf("screw this bullshit! it FAILED! %s\nnow I am about to be consumed by the tsunami!\nCOME GET ME FAST, my location is [REDACTED] CQD SOS",strerror(errno)));
                  } else {
                    e->synchronized([f,oscBuf]() {
                      fwrite(oscBuf->data,sizeof(short),65536,f);
                    });
                    fclose(f);
                    showError(fmt::sprintf("DATA WRITTEN TO /tmp/oscbuf.bin\nthe needle was at %.8x during operation ((needle>>16)<<1 = %u for its actual position in the file).\n\nnow go inspect the file and find out who's biting on that stupid triangle before\nyou succumb to the tsunami, tildearrow is stabbed by Codename Redacted and the Furnace factory comes to a\nperennial halt. HURRY UP!!!",needle,needle>>16));
                  }
                }
              }
              ImGui::EndTable();
            }
            ImGui::TreePop();
          }
        } else {
          ImGui::Text("%d: <NULL!>",i);
          continue;
        }
      }
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("Touch Point Information")) {
      ImGui::Text("active:");
      ImGui::Indent();
      for (TouchPoint& i: activePoints) {
        ImGui::Text("- %d: %.1f, %.1f (%.2f)",i.id,i.x,i.y,i.x);
      }
      ImGui::Unindent();
      ImGui::Text("pressed:");
      ImGui::Indent();
      for (TouchPoint& i: pressedPoints) {
        ImGui::Text("- %d: %.1f, %.1f (%.2f)",i.id,i.x,i.y,i.x);
      }
      ImGui::Unindent();
      ImGui::Text("released:");
      ImGui::Indent();
      for (TouchPoint& i: releasedPoints) {
        ImGui::Text("- %d: %.1f, %.1f (%.2f)",i.id,i.x,i.y,i.x);
      }
      ImGui::Unindent();
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("File Selection Test")) {
      if (ImGui::Button("Test Open")) {
        openFileDialog(GUI_FILE_TEST_OPEN);
      }
      ImGui::SameLine();
      if (ImGui::Button("Test Open Multi")) {
        openFileDialog(GUI_FILE_TEST_OPEN_MULTI);
      }
      ImGui::SameLine();
      if (ImGui::Button("Test Save")) {
        openFileDialog(GUI_FILE_TEST_SAVE);
      }
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("Do Action")) {
      char bindID[1024];
      for (int j=0; j<GUI_ACTION_MAX; j++) {
        if (strcmp(guiActions[j].friendlyName,"")==0) continue;
        if (strstr(guiActions[j].friendlyName,"---")==guiActions[j].friendlyName) {
          ImGui::TextUnformatted(guiActions[j].friendlyName);
        } else {
          snprintf(bindID,1024,"%s##DO_%d",guiActions[j].friendlyName,j);
          if (ImGui::Button(bindID)) {
            doAction(j);
          }
        }
      }
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("Scroll Text Test")) {
      /*
      ImGui::ScrollText(ImGui::GetID("scrolltest1"),"Lorem ipsum, quia dolor sit, amet, consectetur, adipisci velit, sed quia non numquam eius modi tempora incidunt, ut labore et dolore magnam aliquam quaerat voluptatem. ut enim ad minima veniam, quis nostrum exercitationem ullam corporis suscipit laboriosam, nisi ut aliquid ex ea commodi consequatur?");
      ImGui::ScrollText(ImGui::GetID("scrolltest2"),"quis autem vel eum iure reprehenderit");
      ImGui::ScrollText(ImGui::GetID("scrolltest3"),"qui in ea voluptate velit esse",ImVec2(100.0f*dpiScale,0),true);
      ImGui::ScrollText(ImGui::GetID("scrolltest4"),"quam nihil molestiae consequatur, vel illum, qui dolorem eum fugiat, quo voluptas nulla pariatur?",ImVec2(0,0),true);
      */
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("Pitch Table Calculator")) {
      ImGui::InputDouble("Clock",&ptcClock);
      ImGui::InputDouble("Divider/FreqBase",&ptcDivider);
      ImGui::InputInt("Octave",&ptcOctave);
      if (ImGui::RadioButton("Frequency",ptcMode==0)) ptcMode=0;
      ImGui::SameLine();
      if (ImGui::RadioButton("Period",ptcMode==1)) ptcMode=1;
      ImGui::SameLine();
      if (ImGui::RadioButton("FreqNum/Block",ptcMode==2)) ptcMode=2;

      if (ptcMode==2) {
        if (ImGui::InputInt("FreqNum Bits",&ptcBlockBits)) {
          if (ptcBlockBits<0) ptcBlockBits=0;
          if (ptcBlockBits>13) ptcBlockBits=13;
        }
      }

      if (ImGui::BeginTable("PitchTable",7)) {
        ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
        ImGui::TableNextColumn();
        ImGui::Text("Note");
        ImGui::TableNextColumn();
        ImGui::Text("Pitch");

        ImGui::TableNextColumn();
        ImGui::Text("Base");
        ImGui::TableNextColumn();
        ImGui::Text("Hex");

        ImGui::TableNextColumn();
        ImGui::Text("Final");

        ImGui::TableNextColumn();
        ImGui::Text("Hex");

        ImGui::TableNextColumn();
        ImGui::Text("Delta");

        int lastFinal=0;
        for (int i=0; i<12; i++) {
          int note=(12*ptcOctave)+i;
          int pitch=0;

          int base=e->calcBaseFreq(ptcClock,ptcDivider,note,ptcMode==1);
          int final=e->calcFreq(base,pitch,ptcMode==1,0,0,ptcClock,ptcDivider,(ptcMode==2)?ptcBlockBits:0);

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::Text("%d",note);
          ImGui::TableNextColumn();
          ImGui::Text("%d",pitch);

          ImGui::TableNextColumn();
          ImGui::Text("%d",base);
          ImGui::TableNextColumn();
          ImGui::Text("%x",base);

          ImGui::TableNextColumn();
          ImGui::Text("%d",final);
          ImGui::TableNextColumn();
          ImGui::Text("%x",final);

          ImGui::TableNextColumn();
          ImGui::Text("%d",final-lastFinal);

          lastFinal=final;
        }

        ImGui::EndTable();
      }
      ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx("Window Debug",ImGuiTreeNodeFlags_DefaultOpen)) {
      ImGui::Text("Screen: %dx%d+%d+%d",scrW,scrH,scrX,scrY);
      ImGui::Text("Screen (Conf): %dx%d+%d+%d",scrConfW,scrConfH,scrConfX,scrConfY);
      ImGui::Text("Canvas: %dx%d",canvasW,canvasH);
      ImGui::Text("Maximized: %d",scrMax);
      ImGui::Text("System Managed Scale: %d",sysManagedScale);
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("Audio Debug")) {
      TAAudioDesc& audioWant=e->getAudioDescWant();
      TAAudioDesc& audioGot=e->getAudioDescGot();

      ImGui::Text("want:");
      ImGui::Text("- name: %s",audioWant.name.c_str());
      ImGui::Text("- device name: %s",audioWant.deviceName.c_str());
      ImGui::Text("- rate: %f",audioWant.rate);
      ImGui::Text("- buffer size: %d",audioWant.bufsize);
      ImGui::Text("- fragments: %d",audioWant.fragments);
      ImGui::Text("- inputs: %d",audioWant.inChans);
      ImGui::Text("- outputs: %d",audioWant.outChans);
      ImGui::Text("- format: %d",audioWant.outFormat);

      ImGui::Text("got:");
      ImGui::Text("- name: %s",audioGot.name.c_str());
      ImGui::Text("- device name: %s",audioGot.deviceName.c_str());
      ImGui::Text("- rate: %f",audioGot.rate);
      ImGui::Text("- buffer size: %d",audioGot.bufsize);
      ImGui::Text("- fragments: %d",audioGot.fragments);
      ImGui::Text("- inputs: %d",audioGot.inChans);
      ImGui::Text("- outputs: %d",audioGot.outChans);
      ImGui::Text("- format: %d",audioGot.outFormat);

      ImGui::Text("last call to nextBuf(): in %d, out %d, size %d",e->lastNBIns,e->lastNBOuts,e->lastNBSize);

      ImGui::TreePop();
    }
    if (ImGui::TreeNode("MIDI Debug")) {
      if (ImGui::Button("Enable Debug (go to log viewer)")) {
        e->setMidiDebug(true);
        nextWindow=GUI_WINDOW_LOG;
      }
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("Visualizer Debug")) {
      if (ImGui::BeginTable("visX",3,ImGuiTableFlags_Borders)) {
        ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
        ImGui::TableNextColumn();
        ImGui::Text("channel");
        ImGui::TableNextColumn();
        ImGui::Text("patChanX");
        ImGui::TableNextColumn();
        ImGui::Text("patChanSlideY");

        for (int i=0; i<e->getTotalChannelCount(); i++) {
          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::Text("%d",i);
          ImGui::TableNextColumn();
          ImGui::Text("%f",patChanX[i]);
          ImGui::TableNextColumn();
          ImGui::Text("%f",patChanSlideY[i]);
        }

        ImGui::EndTable();
      }
      
      ImGui::Text("particle count: %d",(int)particles.size());

      ImGui::TreePop();
    }
    if (ImGui::TreeNode("Playground")) {
      if (pgSys<0 || pgSys>=e->song.systemLen) pgSys=0;
      if (ImGui::BeginCombo("Chip",fmt::sprintf("%d. %s",pgSys+1,e->getSystemName(e->song.system[pgSys])).c_str())) {
        for (int i=0; i<e->song.systemLen; i++) {
          if (ImGui::Selectable(fmt::sprintf("%d. %s",i+1,e->getSystemName(e->song.system[i])).c_str())) {
            pgSys=i;
            break;
          }
        }
        ImGui::EndCombo();
      }
      ImGui::Text("Program");
      if (pgProgram.empty()) {
        ImGui::Text("-nothing here-");
      } else {
        char id[32];
        for (size_t index=0; index<pgProgram.size(); index++) {
          DivRegWrite& i=pgProgram[index];
          snprintf(id,31,"pgw%d",(int)index);
          ImGui::PushID(id);
          ImGui::SetNextItemWidth(100.0f*dpiScale);
          ImGui::InputScalar("##PAddress",ImGuiDataType_U32,&i.addr,NULL,NULL,"%.2X",ImGuiInputTextFlags_CharsHexadecimal);
          ImGui::SameLine();
          ImGui::Text("=");
          ImGui::SameLine();
          ImGui::SetNextItemWidth(100.0f*dpiScale);
          ImGui::InputScalar("##PValue",ImGuiDataType_U16,&i.val,NULL,NULL,"%.2X",ImGuiInputTextFlags_CharsHexadecimal);
          ImGui::SameLine();
          if (ImGui::Button(ICON_FA_TIMES "##PRemove")) {
            pgProgram.erase(pgProgram.begin()+index);
            index--;
          }
          ImGui::PopID();
        }
      }
      if (ImGui::Button("Execute")) {
        e->poke(pgSys,pgProgram);
      }
      ImGui::SameLine();
      if (ImGui::Button("Clear")) {
        pgProgram.clear();
      }
      
      ImGui::AlignTextToFramePadding();
      ImGui::Text("Address");
      ImGui::SameLine();
      ImGui::SetNextItemWidth(100.0f*dpiScale);
      ImGui::InputInt("##PAddress",&pgAddr,0,0,ImGuiInputTextFlags_CharsHexadecimal);
      ImGui::SameLine();
      ImGui::Text("Value");
      ImGui::SameLine();
      ImGui::SetNextItemWidth(100.0f*dpiScale);
      ImGui::InputInt("##PValue",&pgVal,0,0,ImGuiInputTextFlags_CharsHexadecimal);
      ImGui::SameLine();
      if (ImGui::Button("Write")) {
        e->poke(pgSys,pgAddr,pgVal);
      }
      ImGui::SameLine();
      if (ImGui::Button("Add")) {
        pgProgram.push_back(DivRegWrite(pgAddr,pgVal));
      }
      if (ImGui::TreeNode("Register Cheatsheet")) {
        const char** sheet=e->getRegisterSheet(pgSys);
        if (sheet==NULL) {
          ImGui::Text("no cheatsheet available for this chip.");
        } else {
          if (ImGui::BeginTable("RegisterSheet",2,ImGuiTableFlags_SizingFixedSame)) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Name");
            ImGui::TableNextColumn();
            ImGui::Text("Address");
            for (int i=0; sheet[i]!=NULL; i+=2) {
              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              ImGui::Text("%s",sheet[i]);
              ImGui::TableNextColumn();
              ImGui::Text("$%s",sheet[i+1]);
            }
            ImGui::EndTable();
          }
        }
        ImGui::TreePop();
      }
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("ADSR Test Area")) {
      static int tl, ar, dr, d2r, sl, rr, sus, egt, algOrGlobalSus, instType;
      static float maxArDr, maxTl, maxRr;
      ImGui::Text("This window was done out of frustration");
      drawFMEnv(tl,ar,dr,d2r,rr,sl,sus,egt,algOrGlobalSus,maxTl,maxArDr,maxRr,ImVec2(200.0f*dpiScale,100.0f*dpiScale),instType);

      ImGui::InputInt("tl",&tl);
      ImGui::InputInt("ar",&ar);
      ImGui::InputInt("dr",&dr);
      ImGui::InputInt("d2r",&d2r);
      ImGui::InputInt("sl",&sl);
      ImGui::InputInt("rr",&rr);
      ImGui::InputInt("sus",&sus);
      ImGui::InputInt("egt",&egt);
      ImGui::InputInt("algOrGlobalSus",&algOrGlobalSus);
      ImGui::InputInt("instType",&instType);
      ImGui::InputFloat("maxArDr",&maxArDr);
      ImGui::InputFloat("maxTl",&maxTl);
      ImGui::InputFloat("maxRr",&maxRr);
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("FM Preview")) {
      float asFloat[FM_PREVIEW_SIZE];
      for (int i=0; i<FM_PREVIEW_SIZE; i++) {
        asFloat[i]=(float)fmPreview[i]/8192.0f;
      }
      ImGui::PlotLines("##DebugFMPreview",asFloat,FM_PREVIEW_SIZE,0,"Preview",-1.0,1.0,ImVec2(300.0f*dpiScale,150.0f*dpiScale));
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("Recent Files")) {
      ImGui::Text("Items: %d - Max: %d",(int)recentFile.size(),settings.maxRecentFile);
      ImGui::Text("readPos: %d - writePos: %d",(int)recentFile.readPos,(int)recentFile.writePos);
      ImGui::Indent();
      for (size_t i=0; i<recentFile.size(); i++) {
        ImGui::Text("%d: %s",(int)i,recentFile[i].c_str());
      }
      ImGui::Unindent();
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("Texture Test")) {
      ImGui::Text("Create and Destroy 128 Textures");
      if (ImGui::Button("No Write")) {
        for (int i=0; i<128; i++) {
          FurnaceGUITexture* t=rend->createTexture(false,2048,2048,true,bestTexFormat);
          if (t==NULL) {
            showError(fmt::sprintf("Failure! %d",i));
            break;
          }
          rend->destroyTexture(t);
        }
      }
      if (ImGui::Button("Write (update)")) {
        unsigned char* data=new unsigned char[2048*2048*4];
        for (int i=0; i<2048*2048*4; i++) {
          data[i]=rand();
        }
        for (int i=0; i<128; i++) {
          FurnaceGUITexture* t=rend->createTexture(false,2048,2048,true,bestTexFormat);
          if (t==NULL) {
            showError(fmt::sprintf("Failure! %d",i));
            break;
          }
          rend->updateTexture(t,data,2048*4);
          rend->destroyTexture(t);
        }
        delete[] data;
      }
      if (ImGui::Button("Write (lock)")) {
        unsigned char* data=NULL;
        int pitch=0;
        for (int i=0; i<128; i++) {
          FurnaceGUITexture* t=rend->createTexture(false,2048,2048,true,bestTexFormat);
          if (t==NULL) {
            showError(fmt::sprintf("Failure! %d",i));
            break;
          }
          if (rend->lockTexture(t,(void**)&data,&pitch)) {
            for (int i=0; i<2048*2048*4; i++) {
              data[i]=rand();
            }
            rend->unlockTexture(t);
          }
          rend->destroyTexture(t);
        }
      }
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("Locale Test")) {
      ImGui::TextUnformatted(_("This is a language test."));
      ImGui::TextUnformatted(_("This is another language test."));
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("Osc Render Test")) {
      ImGui::InputInt("Length",&oscDebugLen);
      ImGui::InputInt("Height",&oscDebugHeight);
      ImGui::InputFloat("Line Size",&oscDebugLineSize);
      ImGui::InputFloat("Min",&oscDebugMin);
      ImGui::InputFloat("Max",&oscDebugMax);
      ImGui::InputFloat("Power",&oscDebugPower);
      ImGui::InputInt("Repeat",&oscDebugRepeat);

      if (oscDebugLen<1) oscDebugLen=1;
      if (oscDebugLen>2048) oscDebugLen=2048;

      if (oscDebugHeight<1) oscDebugHeight=1;
      if (oscDebugHeight>2048) oscDebugHeight=2048;

      memset(oscDebugData,0,2048*sizeof(float));
      for (int i=0; i<oscDebugLen; i++) {
        oscDebugData[i]=oscDebugMin+(oscDebugMax-oscDebugMin)*pow((float)((i*oscDebugRepeat)%oscDebugLen)/(float)oscDebugLen,oscDebugPower);
      }
      
      if (rend->supportsDrawOsc()) {
        ImDrawList* dl=ImGui::GetWindowDrawList();
        ImGuiWindow* window=ImGui::GetCurrentWindow();

        ImVec2 size=ImVec2(oscDebugLen,oscDebugHeight);
        ImVec2 minArea=window->DC.CursorPos;
        ImVec2 maxArea=ImVec2(
          minArea.x+size.x,
          minArea.y+size.y
        );
        ImRect rect=ImRect(minArea,maxArea);
        ImGuiStyle& style=ImGui::GetStyle();
        ImGui::ItemSize(size,style.FramePadding.y);
        if (ImGui::ItemAdd(rect,ImGui::GetID("debugOsc"))) {
          _debugDo.gui=this;
          _debugDo.data=oscDebugData;
          _debugDo.len=oscDebugLen;
          _debugDo.pos0=rect.Min;
          _debugDo.pos1=rect.Max;
          _debugDo.color=ImVec4(1.0,1.0,1.0,1.0);
          _debugDo.lineSize=dpiScale*oscDebugLineSize;

          dl->AddCallback(_drawOsc,&_debugDo);
          dl->AddCallback(ImDrawCallback_ResetRenderState,NULL);
        }
      } else {
        ImGui::Text("Render Backend does not support osc rendering.");
      }
      ImGui::TreePop();
    }
#ifdef HAVE_LOCALE
    if (ImGui::TreeNode("Plural Form Test")) {
      ImGui::InputInt("Number",&numApples);
      ImGui::Text(ngettext("%d apple","%d apples",numApples),numApples);
      ImGui::TreePop();
    }
#endif
    if (ImGui::TreeNode("Get Gain Test")) {
      float realVol=e->getGain(getGainChan,getGainVol);
      ImGui::InputInt("Chan",&getGainChan);
      ImGui::InputInt("Vol",&getGainVol);
      ImGui::Text("result: %.0f%%",realVol*100.0f);
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("Cursor Undo Debug")) {
      auto DrawSpot=[&](const CursorJumpPoint& spot) {
        ImGui::Text("[%d:%d] <%d:%d, %d>", spot.subSong, spot.order, spot.point.xCoarse, spot.point.xFine, spot.point.y);
      };
      if (ImGui::BeginChild("##CursorUndoDebugChild", ImVec2(0, 300), true)) {
        if (ImGui::BeginTable("##CursorUndoDebug", 2, ImGuiTableFlags_Borders|ImGuiTableFlags_SizingStretchSame)) {
          for (size_t row=0; row<MAX(cursorUndoHist.size(),cursorRedoHist.size()); ++row) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if (row<cursorUndoHist.size()) DrawSpot(cursorUndoHist[cursorUndoHist.size()-row-1]);
            ImGui::TableNextColumn();
            if (row<cursorRedoHist.size()) DrawSpot(cursorRedoHist[cursorRedoHist.size()-row-1]);
          }
          ImGui::EndTable();
        }
      }
      ImGui::EndChild();
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("User Interface")) {
      if (ImGui::Button("Inspect")) {
        inspectorOpen=!inspectorOpen;
      }
      if (ImGui::Button("Spoiler")) {
        spoilerOpen=!spoilerOpen;
      }
      if (ImGui::Button("Kill Graphics")) {
        killGraphics=true;
      }
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("Performance")) {
      double perfFreq=SDL_GetPerformanceFrequency()/1000000.0;
      int lastProcTime=(int)e->processTime/1000;
      TAAudioDesc& audioGot=e->getAudioDescGot();

      ImGui::Text("video frame: %.0fµs",ImGui::GetIO().DeltaTime*1000000.0);
      ImGui::Text("audio frame: %.0fµs",1000000.0*(double)audioGot.bufsize/(double)audioGot.rate);
      ImGui::Separator();

      ImGui::Text("audio: %dµs",lastProcTime);
      ImGui::Text("render: %.0fµs",(double)renderTimeDelta/perfFreq);
      ImGui::Text("draw: %.0fµs",(double)drawTimeDelta/perfFreq);
      ImGui::Text("swap: %.0fµs",(double)swapTimeDelta/perfFreq);
      ImGui::Text("layout: %.0fµs",(double)layoutTimeDelta/perfFreq);
      ImGui::Text("event: %.0fµs",(double)eventTimeDelta/perfFreq);
      ImGui::Separator();

      ImGui::Text("details:");
      for (int i=0; i<perfMetricsLastLen; i++) {
        ImGui::Text("%s: %.0fµs",perfMetricsLast[i].name,(double)perfMetricsLast[i].elapsed/perfFreq);
      }
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("Settings")) {
      if (ImGui::Button("Sync")) syncSettings();
      ImGui::SameLine();
      if (ImGui::Button("Commit")) commitSettings();
      ImGui::SameLine();
      if (ImGui::Button("Force Load")) e->loadConf();
      ImGui::SameLine();
      if (ImGui::Button("Force Save")) e->saveConf();
      ImGui::TreePop();
    }
    ImGui::Text("Song format version %d",e->song.version);
    ImGui::Text("Furnace version " DIV_VERSION " (%d)",DIV_ENGINE_VERSION);
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_DEBUG;
  ImGui::End();
}
