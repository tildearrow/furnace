/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2022 tildearrow and contributors
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
#include <imgui.h>
#include <imgui_internal.h>
#include <math.h>
#include "../ta-log.h"
#include "IconsFontAwesome4.h"
#include "misc/cpp/imgui_stdlib.h"
#include <fmt/printf.h>
#include "guiConst.h"
#include "sampleUtil.h"
#include "util.h"

#define CENTER_TEXT(text) \
  ImGui::SetCursorPosX(ImGui::GetCursorPosX()+0.5*(ImGui::GetContentRegionAvail().x-ImGui::CalcTextSize(text).x));

void FurnaceGUI::drawSampleEdit() {
  if (nextWindow==GUI_WINDOW_SAMPLE_EDIT) {
    sampleEditOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!sampleEditOpen) return;
  if (mobileUI) {
    patWindowPos=(portrait?ImVec2(0.0f,(mobileMenuPos*-0.65*scrH*dpiScale)):ImVec2((0.16*scrH*dpiScale)+0.5*scrW*dpiScale*mobileMenuPos,0.0f));
    patWindowSize=(portrait?ImVec2(scrW*dpiScale,scrH*dpiScale-(0.16*scrW*dpiScale)-(pianoOpen?(0.4*scrW*dpiScale):0.0f)):ImVec2(scrW*dpiScale-(0.16*scrH*dpiScale),scrH*dpiScale-(pianoOpen?(0.3*scrH*dpiScale):0.0f)));
    ImGui::SetNextWindowPos(patWindowPos);
    ImGui::SetNextWindowSize(patWindowSize);
  }
  if (ImGui::Begin("Sample Editor",&sampleEditOpen,globalWinFlags|(settings.allowEditDocking?0:ImGuiWindowFlags_NoDocking))) {
    if (curSample<0 || curSample>=(int)e->song.sample.size()) {
      ImGui::SetCursorPosY(ImGui::GetCursorPosY()+(ImGui::GetContentRegionAvail().y-ImGui::GetFrameHeightWithSpacing()*2.0f)*0.5f);
      CENTER_TEXT("no sample selected");
      ImGui::Text("no sample selected");
      if (ImGui::BeginTable("noAssetCenter",3)) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.5f);
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.5f);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();

        if (e->song.sample.size()>0) {
          if (ImGui::BeginCombo("##SampleSelect","select one...")) {
            actualSampleList();
            ImGui::EndCombo();
          }
          ImGui::SameLine();
          ImGui::TextUnformatted("or");
          ImGui::SameLine();
        }
        if (ImGui::Button("Open")) {
          doAction(GUI_ACTION_SAMPLE_LIST_OPEN);
        }
        ImGui::SameLine();
        ImGui::TextUnformatted("or");
        ImGui::SameLine();
        if (ImGui::Button("Create New")) {
          doAction(GUI_ACTION_SAMPLE_LIST_ADD);
        }

        ImGui::TableNextColumn();
        ImGui::EndTable();
      }
    } else {
      DivSample* sample=e->song.sample[curSample];
      String sampleType="Invalid";
      if (sample->depth<DIV_SAMPLE_DEPTH_MAX) {
        if (sampleDepths[sample->depth]!=NULL) {
          sampleType=sampleDepths[sample->depth];
        }
      }
      String loopType="Invalid";
      if (sample->loopMode<DIV_SAMPLE_LOOP_MAX) {
        if (sampleLoopModes[sample->loopMode]!=NULL) {
          loopType=sampleLoopModes[sample->loopMode];
        }
      }
      if (!settings.sampleLayout) {
        ImGui::Text("Name");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::InputText("##SampleName",&sample->name)) {
          MARK_MODIFIED;
        }

        if (ImGui::BeginTable("SampleProps",4,ImGuiTableFlags_SizingStretchSame)) {
          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::Text("Type");
          ImGui::SameLine();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::BeginCombo("##SampleType",sampleType.c_str())) {
            for (int i=0; i<DIV_SAMPLE_DEPTH_MAX; i++) {
              if (sampleDepths[i]==NULL) continue;
              if (ImGui::Selectable(sampleDepths[i])) {
                sample->prepareUndo(true);
                e->lockEngine([sample]() {
                  sample->render();
                });
                sample->depth=(DivSampleDepth)i;
                e->renderSamplesP();
                updateSampleTex=true;
                MARK_MODIFIED;
              }
            }
            ImGui::EndCombo();
          }

          ImGui::TableNextColumn();
          ImGui::Text("C-4 (Hz)");
          ImGui::SameLine();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::InputInt("##SampleCenter",&sample->centerRate,10,200)) { MARK_MODIFIED
            if (sample->centerRate<100) sample->centerRate=100;
            if (sample->centerRate>65535) sample->centerRate=65535;
          }

          ImGui::TableNextColumn();
          ImGui::Text("Compat Rate");
          ImGui::SameLine();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::InputInt("##SampleRate",&sample->rate,10,200)) { MARK_MODIFIED
            if (sample->rate<100) sample->rate=100;
            if (sample->rate>96000) sample->rate=96000;
          }

          ImGui::TableNextColumn();
          bool doLoop=(sample->isLoopable());
          if (ImGui::Checkbox("Loop",&doLoop)) { MARK_MODIFIED
            if (doLoop) {
              sample->loop=true;
              sample->loopStart=0;
              sample->loopEnd=sample->samples;
            } else {
              sample->loop=false;
              sample->loopStart=-1;
              sample->loopEnd=sample->samples;
            }
            updateSampleTex=true;
            if (e->getSampleFormatMask()&(1U<<DIV_SAMPLE_DEPTH_BRR)) {
              e->renderSamplesP();
            }
          }
          if (ImGui::IsItemHovered() && sample->depth==DIV_SAMPLE_DEPTH_BRR) {
            ImGui::SetTooltip("changing the loop in a BRR sample may result in glitches!");
          }
          if (doLoop || keepLoopAlive) {
            keepLoopAlive=false;
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Loop Mode");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::BeginCombo("##SampleLoopMode",loopType.c_str())) {
              for (int i=0; i<DIV_SAMPLE_LOOP_MAX; i++) {
                if (sampleLoopModes[i]==NULL) continue;
                if (ImGui::Selectable(sampleLoopModes[i])) {
                  sample->prepareUndo(true);
                  sample->loopMode=(DivSampleLoopMode)i;
                  e->renderSamplesP();
                  updateSampleTex=true;
                  MARK_MODIFIED;
                }
              }
              ImGui::EndCombo();
            }
            ImGui::TableNextColumn();
            ImGui::Text("Loop Start");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::InputInt("##LoopStartPosition",&sample->loopStart,1,16)) { MARK_MODIFIED
              if (sample->loopStart<0) {
                sample->loopStart=0;
              }
              if (sample->loopStart>sample->loopEnd) {
                sample->loopStart=sample->loopEnd;
              }
              updateSampleTex=true;
              if (e->getSampleFormatMask()&(1U<<DIV_SAMPLE_DEPTH_BRR)) {
                e->renderSamplesP();
              }
            }
            if (ImGui::IsItemActive()) {
              keepLoopAlive=true;
            }
            if (ImGui::IsItemHovered() && sample->depth==DIV_SAMPLE_DEPTH_BRR) {
              ImGui::SetTooltip("changing the loop in a BRR sample may result in glitches!");
            }
            ImGui::TableNextColumn();
            ImGui::Text("Loop End");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::InputInt("##LoopEndPosition",&sample->loopEnd,1,16)) { MARK_MODIFIED
              if (sample->loopEnd<sample->loopStart) {
                sample->loopEnd=sample->loopStart;
              }
              if (sample->loopEnd>=(int)sample->samples) {
                sample->loopEnd=sample->samples;
              }
              updateSampleTex=true;
              if (e->getSampleFormatMask()&(1U<<DIV_SAMPLE_DEPTH_BRR)) {
                e->renderSamplesP();
              }
            }
            if (ImGui::IsItemActive()) {
              keepLoopAlive=true;
            }
            if (ImGui::IsItemHovered() && sample->depth==DIV_SAMPLE_DEPTH_BRR) {
              ImGui::SetTooltip("changing the loop in a BRR sample may result in glitches!");
            }
          }
          ImGui::EndTable();
        }

        /*
        if (ImGui::Button("Apply")) {
          e->renderSamplesP();
        }
        ImGui::SameLine();
        */
        ImGui::Separator();

        pushToggleColors(!sampleDragMode);
        if (ImGui::Button(ICON_FA_I_CURSOR "##SSelect")) {
          sampleDragMode=false;
        }
        popToggleColors();
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Edit mode: Select");
        }
        ImGui::SameLine();
        pushToggleColors(sampleDragMode);
        if (ImGui::Button(ICON_FA_PENCIL "##SDraw")) {
          sampleDragMode=true;
        }
        popToggleColors();
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Edit mode: Draw");
        }
        ImGui::BeginDisabled(sample->depth!=DIV_SAMPLE_DEPTH_8BIT && sample->depth!=DIV_SAMPLE_DEPTH_16BIT);
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(4.0*dpiScale,dpiScale));
        ImGui::SameLine();
        ImGui::Button(ICON_FA_ARROWS_H "##SResize");
        if (ImGui::IsItemClicked()) {
          resizeSize=sample->samples;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Resize");
        }
        if (openSampleResizeOpt) {
          openSampleResizeOpt=false;
          ImGui::OpenPopup("SResizeOpt");
        }
        if (ImGui::BeginPopupContextItem("SResizeOpt",ImGuiPopupFlags_MouseButtonLeft)) {
          if (ImGui::InputInt("Samples",&resizeSize,1,64)) {
            if (resizeSize<0) resizeSize=0;
            if (resizeSize>16777215) resizeSize=16777215;
          }
          if (ImGui::Button("Resize")) {
            sample->prepareUndo(true);
            e->lockEngine([this,sample]() {
              if (!sample->resize(resizeSize)) {
                showError("couldn't resize! make sure your sample is 8 or 16-bit.");
              }
              e->renderSamples();
            });
            updateSampleTex=true;
            sampleSelStart=-1;
            sampleSelEnd=-1;
            MARK_MODIFIED;
            ImGui::CloseCurrentPopup();
          }
          ImGui::EndPopup();
        } else {
          resizeSize=sample->samples;
        }
        ImGui::SameLine();
        ImGui::Button(ICON_FA_EXPAND "##SResample");
        if (ImGui::IsItemClicked()) {
          resampleTarget=sample->rate;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Resample");
        }
        if (openSampleResampleOpt) {
          openSampleResampleOpt=false;
          ImGui::OpenPopup("SResampleOpt");
        }
        if (ImGui::BeginPopupContextItem("SResampleOpt",ImGuiPopupFlags_MouseButtonLeft)) {
          ImGui::Text("Rate");
          if (ImGui::InputDouble("##SRRate",&resampleTarget,1.0,50.0,"%g")) {
            if (resampleTarget<0) resampleTarget=0;
            if (resampleTarget>96000) resampleTarget=96000;
          }
          ImGui::SameLine();
          if (ImGui::Button("0.5x")) {
            resampleTarget*=0.5;
          }
          ImGui::SameLine();
          if (ImGui::Button("==")) {
            resampleTarget=sample->rate;
          }
          ImGui::SameLine();
          if (ImGui::Button("2.0x")) {
            resampleTarget*=2.0;
          }
          double factor=resampleTarget/(double)sample->rate;
          if (ImGui::InputDouble("Factor",&factor,0.125,0.5,"%g")) {
            resampleTarget=(double)sample->rate*factor;
            if (resampleTarget<0) resampleTarget=0;
            if (resampleTarget>96000) resampleTarget=96000;
          }
          ImGui::Combo("Filter",&resampleStrat,resampleStrats,6);
          if (ImGui::Button("Resample")) {
            sample->prepareUndo(true);
            e->lockEngine([this,sample]() {
              if (!sample->resample(resampleTarget,resampleStrat)) {
                showError("couldn't resample! make sure your sample is 8 or 16-bit.");
              }
              e->renderSamples();
            });
            updateSampleTex=true;
            sampleSelStart=-1;
            sampleSelEnd=-1;
            MARK_MODIFIED;
            ImGui::CloseCurrentPopup();
          }
          ImGui::EndPopup();
        } else {
          resampleTarget=sample->rate;
        }
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(4.0*dpiScale,dpiScale));
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_UNDO "##SUndo")) {
          doUndoSample();
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Undo");
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_REPEAT "##SRedo")) {
          doRedoSample();
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Redo");
        }
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(4.0*dpiScale,dpiScale));
        ImGui::SameLine();
        ImGui::Button(ICON_FA_VOLUME_UP "##SAmplify");
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Amplify");
        }
        if (openSampleAmplifyOpt) {
          openSampleAmplifyOpt=false;
          ImGui::OpenPopup("SAmplifyOpt");
        }
        if (ImGui::BeginPopupContextItem("SAmplifyOpt",ImGuiPopupFlags_MouseButtonLeft)) {
          ImGui::Text("Volume");
          if (ImGui::InputFloat("##SRVolume",&amplifyVol,10.0,50.0,"%g%%")) {
            if (amplifyVol<0) amplifyVol=0;
            if (amplifyVol>10000) amplifyVol=10000;
          }
          ImGui::SameLine();
          ImGui::Text("(%.1fdB)",20.0*log10(amplifyVol/100.0f));
          if (ImGui::Button("Apply")) {
            sample->prepareUndo(true);
            e->lockEngine([this,sample]() {
              SAMPLE_OP_BEGIN;
              float vol=amplifyVol/100.0f;

              if (sample->depth==DIV_SAMPLE_DEPTH_16BIT) {
                for (unsigned int i=start; i<end; i++) {
                  float val=sample->data16[i]*vol;
                  if (val<-32768) val=-32768;
                  if (val>32767) val=32767;
                  sample->data16[i]=val;
                }
              } else if (sample->depth==DIV_SAMPLE_DEPTH_8BIT) {
                for (unsigned int i=start; i<end; i++) {
                  float val=sample->data8[i]*vol;
                  if (val<-128) val=-128;
                  if (val>127) val=127;
                  sample->data8[i]=val;
                }
              }

              updateSampleTex=true;

              e->renderSamples();
            });
            MARK_MODIFIED;
            ImGui::CloseCurrentPopup();
          }
          ImGui::EndPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_ARROWS_V "##SNormalize")) {
          doAction(GUI_ACTION_SAMPLE_NORMALIZE);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Normalize");
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_ARROW_UP "##SFadeIn")) {
          doAction(GUI_ACTION_SAMPLE_FADE_IN);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Fade in");
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_ARROW_DOWN "##SFadeOut")) {
          doAction(GUI_ACTION_SAMPLE_FADE_OUT);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Fade out");
        }
        ImGui::SameLine();
        ImGui::Button(ICON_FA_ADJUST "##SInsertSilence");
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Insert silence");
        }
        if (openSampleSilenceOpt) {
          openSampleSilenceOpt=false;
          ImGui::OpenPopup("SSilenceOpt");
        }
        if (ImGui::BeginPopupContextItem("SSilenceOpt",ImGuiPopupFlags_MouseButtonLeft)) {
          if (ImGui::InputInt("Samples",&silenceSize,1,64)) {
            if (silenceSize<0) silenceSize=0;
            if (silenceSize>16777215) silenceSize=16777215;
          }
          if (ImGui::Button("Go")) {
            int pos=(sampleSelStart==-1 || sampleSelStart==sampleSelEnd)?sample->samples:sampleSelStart;
            sample->prepareUndo(true);
            e->lockEngine([this,sample,pos]() {
              if (!sample->insert(pos,silenceSize)) {
                showError("couldn't insert! make sure your sample is 8 or 16-bit.");
              }
              e->renderSamples();
            });
            updateSampleTex=true;
            sampleSelStart=pos;
            sampleSelEnd=pos+silenceSize;
            MARK_MODIFIED;
            ImGui::CloseCurrentPopup();
          }
          ImGui::EndPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_ERASER "##SSilence")) {
          doAction(GUI_ACTION_SAMPLE_SILENCE);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Apply silence");
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_TIMES "##SDelete")) {
          doAction(GUI_ACTION_SAMPLE_DELETE);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Delete");
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_CROP "##STrim")) {
          doAction(GUI_ACTION_SAMPLE_TRIM);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Trim");
        }
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(4.0*dpiScale,dpiScale));
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_BACKWARD "##SReverse")) {
          doAction(GUI_ACTION_SAMPLE_REVERSE);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Reverse");
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_SORT_AMOUNT_ASC "##SInvert")) {
          doAction(GUI_ACTION_SAMPLE_INVERT);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Invert");
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_LEVEL_DOWN "##SSign")) {
          doAction(GUI_ACTION_SAMPLE_SIGN);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Signed/unsigned exchange");
        }
        ImGui::SameLine();
        ImGui::Button(ICON_FA_INDUSTRY "##SFilter");
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Apply filter");
        }
        if (openSampleFilterOpt) {
          openSampleFilterOpt=false;
          ImGui::OpenPopup("SFilterOpt");
        }
        if (ImGui::BeginPopupContextItem("SFilterOpt",ImGuiPopupFlags_MouseButtonLeft)) {
          float lowP=sampleFilterL*100.0f;
          float bandP=sampleFilterB*100.0f;
          float highP=sampleFilterH*100.0f;
          float resP=sampleFilterRes*100.0f;
          ImGui::Text("Cutoff:");
          if (ImGui::SliderFloat("From",&sampleFilterCutStart,0.0f,sample->rate*0.5,"%.0fHz")) {
            if (sampleFilterCutStart<0.0) sampleFilterCutStart=0.0;
            if (sampleFilterCutStart>sample->rate*0.5) sampleFilterCutStart=sample->rate*0.5;
          }
          if (ImGui::SliderFloat("To",&sampleFilterCutEnd,0.0f,sample->rate*0.5,"%.0fHz")) {
            if (sampleFilterCutEnd<0.0) sampleFilterCutEnd=0.0;
            if (sampleFilterCutEnd>sample->rate*0.5) sampleFilterCutEnd=sample->rate*0.5;
          }
          ImGui::Separator();
          if (ImGui::SliderFloat("Resonance",&resP,0.0f,99.0f,"%.1f%%")) {
            sampleFilterRes=resP/100.0f;
            if (sampleFilterRes<0.0f) sampleFilterRes=0.0f;
            if (sampleFilterRes>0.99f) sampleFilterRes=0.99f;
          }
          ImGui::Text("Power");
          ImGui::SameLine();
          if (ImGui::RadioButton("1x",sampleFilterPower==1)) {
            sampleFilterPower=1;
          }
          ImGui::SameLine();
          if (ImGui::RadioButton("2x",sampleFilterPower==2)) {
            sampleFilterPower=2;
          }
          ImGui::SameLine();
          if (ImGui::RadioButton("3x",sampleFilterPower==3)) {
            sampleFilterPower=3;
          }
          ImGui::Separator();
          if (ImGui::SliderFloat("Low-pass",&lowP,0.0f,100.0f,"%.1f%%")) {
            sampleFilterL=lowP/100.0f;
            if (sampleFilterL<0.0f) sampleFilterL=0.0f;
            if (sampleFilterL>1.0f) sampleFilterL=1.0f;
          }
          if (ImGui::SliderFloat("Band-pass",&bandP,0.0f,100.0f,"%.1f%%")) {
            sampleFilterB=bandP/100.0f;
            if (sampleFilterB<0.0f) sampleFilterB=0.0f;
            if (sampleFilterB>1.0f) sampleFilterB=1.0f;
          }
          if (ImGui::SliderFloat("High-pass",&highP,0.0f,100.0f,"%.1f%%")) {
            sampleFilterH=highP/100.0f;
            if (sampleFilterH<0.0f) sampleFilterH=0.0f;
            if (sampleFilterH>1.0f) sampleFilterH=1.0f;
          }

          if (ImGui::Button("Apply")) {
            sample->prepareUndo(true);
            e->lockEngine([this,sample]() {
              SAMPLE_OP_BEGIN;
              float res=1.0-pow(sampleFilterRes,0.5f);
              float low=0;
              float band=0;
              float high=0;

              double power=(sampleFilterCutStart>sampleFilterCutEnd)?0.5:2.0;

              if (sample->depth==DIV_SAMPLE_DEPTH_16BIT) {
                for (unsigned int i=start; i<end; i++) {
                  double freq=sampleFilterCutStart+(sampleFilterCutEnd-sampleFilterCutStart)*pow(double(i-start)/double(end-start),power);
                  double cut=sin((freq/double(sample->rate))*M_PI);

                  for (int j=0; j<sampleFilterPower; j++) {
                    low=low+cut*band;
                    high=float(sample->data16[i])-low-(res*band);
                    band=cut*high+band;
                  }

                  float val=low*sampleFilterL+band*sampleFilterB+high*sampleFilterH;
                  if (val<-32768) val=-32768;
                  if (val>32767) val=32767;
                  sample->data16[i]=val;
                }
              } else if (sample->depth==DIV_SAMPLE_DEPTH_8BIT) {
                for (unsigned int i=start; i<end; i++) {
                  double freq=sampleFilterCutStart+(sampleFilterCutEnd-sampleFilterCutStart)*pow(double(i-start)/double(end-start),power);
                  double cut=sin((freq/double(sample->rate))*M_PI);

                  for (int j=0; j<sampleFilterPower; j++) {
                    low=low+cut*band;
                    high=float(sample->data8[i])-low-(res*band);
                    band=cut*high+band;
                  }

                  float val=low*sampleFilterL+band*sampleFilterB+high*sampleFilterH;
                  if (val<-128) val=-128;
                  if (val>127) val=127;
                  sample->data8[i]=val;
                }
              }

              updateSampleTex=true;

              e->renderSamples();
            });
            MARK_MODIFIED;
            ImGui::CloseCurrentPopup();
          }
          ImGui::EndPopup();
        }
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(4.0*dpiScale,dpiScale));
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_PLAY "##PreviewSample")) {
          e->previewSample(curSample);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Preview sample");
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_STOP "##StopSample")) {
          e->stopSamplePreview();
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Stop sample preview");
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_UPLOAD "##MakeIns")) {
          doAction(GUI_ACTION_SAMPLE_MAKE_INS);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Create instrument from sample");
        }

        ImGui::SameLine();
        double zoomPercent=100.0/sampleZoom;
        ImGui::Text("Zoom");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(150.0f*dpiScale);
        if (ImGui::InputDouble("##SZoom",&zoomPercent,5.0,20.0,"%g%%")) {
          if (zoomPercent>10000.0) zoomPercent=10000.0;
          if (zoomPercent<1.0) zoomPercent=1.0;
          sampleZoom=100.0/zoomPercent;
          if (sampleZoom<0.01) sampleZoom=0.01;
          sampleZoomAuto=false;
          updateSampleTex=true;
        }
        ImGui::SameLine();
        if (sampleZoomAuto) {
          if (ImGui::Button("100%")) {
            sampleZoom=1.0;
            sampleZoomAuto=false;
            updateSampleTex=true;
          }
        } else {
          if (ImGui::Button("Auto")) {
            sampleZoomAuto=true;
            updateSampleTex=true;
          }
        }
      } else {
        ImGui::Text("Name");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::InputText("##SampleName",&sample->name)) {
          MARK_MODIFIED;
        }

        if (ImGui::BeginTable("SampleProps",2,ImGuiTableFlags_SizingStretchSame)) {
          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::Text("Type");
          ImGui::SameLine();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::BeginCombo("##SampleType",sampleType.c_str())) {
            for (int i=0; i<DIV_SAMPLE_DEPTH_MAX; i++) {
              if (sampleDepths[i]==NULL) continue;
              if (ImGui::Selectable(sampleDepths[i])) {
                sample->prepareUndo(true);
                e->lockEngine([sample]() {
                  sample->render();
                });
                sample->depth=(DivSampleDepth)i;
                e->renderSamplesP();
                updateSampleTex=true;
                MARK_MODIFIED;
              }
            }
            ImGui::EndCombo();
          }

          ImGui::TableNextColumn();
          ImGui::Text("C-4 (Hz)");
          ImGui::SameLine();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::InputInt("##SampleCenter",&sample->centerRate,10,200)) { MARK_MODIFIED
            if (sample->centerRate<100) sample->centerRate=100;
            if (sample->centerRate>65535) sample->centerRate=65535;
          }

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::Text("Compat Rate");
          ImGui::SameLine();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::InputInt("##SampleRate",&sample->rate,10,200)) { MARK_MODIFIED
            if (sample->rate<100) sample->rate=100;
            if (sample->rate>96000) sample->rate=96000;
          }

          ImGui::TableNextColumn();
          bool doLoop=(sample->isLoopable());
          if (ImGui::Checkbox("Loop",&doLoop)) { MARK_MODIFIED
            if (doLoop) {
              sample->loop=true;
              sample->loopStart=0;
              sample->loopEnd=sample->samples;
            } else {
              sample->loop=false;
              sample->loopStart=-1;
              sample->loopEnd=sample->samples;
            }
            updateSampleTex=true;
            if (e->getSampleFormatMask()&(1U<<DIV_SAMPLE_DEPTH_BRR)) {
              e->renderSamplesP();
            }
          }
          if (ImGui::IsItemHovered() && sample->depth==DIV_SAMPLE_DEPTH_BRR) {
            ImGui::SetTooltip("changing the loop in a BRR sample may result in glitches!");
          }
          if (doLoop || keepLoopAlive) {
            keepLoopAlive=false;
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Loop Mode");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::BeginCombo("##SampleLoopMode",loopType.c_str())) {
              for (int i=0; i<DIV_SAMPLE_LOOP_MAX; i++) {
                if (sampleLoopModes[i]==NULL) continue;
                if (ImGui::Selectable(sampleLoopModes[i])) {
                  sample->prepareUndo(true);
                  sample->loopMode=(DivSampleLoopMode)i;
                  e->renderSamplesP();
                  updateSampleTex=true;
                  MARK_MODIFIED;
                }
              }
              ImGui::EndCombo();
            }
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Loop Start");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::InputInt("##LoopStartPosition",&sample->loopStart,1,16)) { MARK_MODIFIED
              if (sample->loopStart<0) {
                sample->loopStart=0;
              }
              if (sample->loopStart>sample->loopEnd) {
                sample->loopStart=sample->loopEnd;
              }
              if (e->getSampleFormatMask()&(1U<<DIV_SAMPLE_DEPTH_BRR)) {
                e->renderSamplesP();
              }
              updateSampleTex=true;
            }
            if (ImGui::IsItemActive()) {
              keepLoopAlive=true;
            }
            if (ImGui::IsItemHovered() && sample->depth==DIV_SAMPLE_DEPTH_BRR) {
              ImGui::SetTooltip("changing the loop in a BRR sample may result in glitches!");
            }
            ImGui::TableNextColumn();
            ImGui::Text("Loop End");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::InputInt("##LoopEndPosition",&sample->loopEnd,1,16)) { MARK_MODIFIED
              if (sample->loopEnd<sample->loopStart) {
                sample->loopEnd=sample->loopStart;
              }
              if (sample->loopEnd>=(int)sample->samples) {
                sample->loopEnd=sample->samples;
              }
              if (e->getSampleFormatMask()&(1U<<DIV_SAMPLE_DEPTH_BRR)) {
                e->renderSamplesP();
              }
              updateSampleTex=true;
            }
            if (ImGui::IsItemActive()) {
              keepLoopAlive=true;
            }
            if (ImGui::IsItemHovered() && sample->depth==DIV_SAMPLE_DEPTH_BRR) {
              ImGui::SetTooltip("changing the loop in a BRR sample may result in glitches!");
            }
          }
          ImGui::EndTable();
        }

        /*
        if (ImGui::Button("Apply")) {
          e->renderSamplesP();
        }
        ImGui::SameLine();
        */
        ImGui::Separator();

        pushToggleColors(!sampleDragMode);
        if (ImGui::Button(ICON_FA_I_CURSOR "##SSelect")) {
          sampleDragMode=false;
        }
        popToggleColors();
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Edit mode: Select");
        }
        ImGui::SameLine();
        pushToggleColors(sampleDragMode);
        if (ImGui::Button(ICON_FA_PENCIL "##SDraw")) {
          sampleDragMode=true;
        }
        popToggleColors();
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Edit mode: Draw");
        }
        ImGui::BeginDisabled(sample->depth!=DIV_SAMPLE_DEPTH_8BIT && sample->depth!=DIV_SAMPLE_DEPTH_16BIT);
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(7.0*dpiScale,dpiScale));
        ImGui::SameLine();
        ImGui::Button(ICON_FA_ARROWS_H "##SResize");
        if (ImGui::IsItemClicked()) {
          resizeSize=sample->samples;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Resize");
        }
        if (openSampleResizeOpt) {
          openSampleResizeOpt=false;
          ImGui::OpenPopup("SResizeOpt");
        }
        if (ImGui::BeginPopupContextItem("SResizeOpt",ImGuiPopupFlags_MouseButtonLeft)) {
          if (ImGui::InputInt("Samples",&resizeSize,1,64)) {
            if (resizeSize<0) resizeSize=0;
            if (resizeSize>16777215) resizeSize=16777215;
          }
          if (ImGui::Button("Resize")) {
            sample->prepareUndo(true);
            e->lockEngine([this,sample]() {
              if (!sample->resize(resizeSize)) {
                showError("couldn't resize! make sure your sample is 8 or 16-bit.");
              }
              e->renderSamples();
            });
            updateSampleTex=true;
            sampleSelStart=-1;
            sampleSelEnd=-1;
            MARK_MODIFIED;
            ImGui::CloseCurrentPopup();
          }
          ImGui::EndPopup();
        } else {
          resizeSize=sample->samples;
        }
        ImGui::SameLine();
        ImGui::Button(ICON_FA_EXPAND "##SResample");
        if (ImGui::IsItemClicked()) {
          resampleTarget=sample->rate;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Resample");
        }
        if (openSampleResampleOpt) {
          openSampleResampleOpt=false;
          ImGui::OpenPopup("SResampleOpt");
        }
        if (ImGui::BeginPopupContextItem("SResampleOpt",ImGuiPopupFlags_MouseButtonLeft)) {
          ImGui::Text("Rate");
          if (ImGui::InputDouble("##SRRate",&resampleTarget,1.0,50.0,"%g")) {
            if (resampleTarget<0) resampleTarget=0;
            if (resampleTarget>96000) resampleTarget=96000;
          }
          ImGui::SameLine();
          if (ImGui::Button("0.5x")) {
            resampleTarget*=0.5;
          }
          ImGui::SameLine();
          if (ImGui::Button("==")) {
            resampleTarget=sample->rate;
          }
          ImGui::SameLine();
          if (ImGui::Button("2.0x")) {
            resampleTarget*=2.0;
          }
          double factor=resampleTarget/(double)sample->rate;
          if (ImGui::InputDouble("Factor",&factor,0.125,0.5,"%g")) {
            resampleTarget=(double)sample->rate*factor;
            if (resampleTarget<0) resampleTarget=0;
            if (resampleTarget>96000) resampleTarget=96000;
          }
          ImGui::Combo("Filter",&resampleStrat,resampleStrats,6);
          if (ImGui::Button("Resample")) {
            sample->prepareUndo(true);
            e->lockEngine([this,sample]() {
              if (!sample->resample(resampleTarget,resampleStrat)) {
                showError("couldn't resample! make sure your sample is 8 or 16-bit.");
              }
              e->renderSamples();
            });
            updateSampleTex=true;
            sampleSelStart=-1;
            sampleSelEnd=-1;
            MARK_MODIFIED;
            ImGui::CloseCurrentPopup();
          }
          ImGui::EndPopup();
        } else {
          resampleTarget=sample->rate;
        }
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(0.5*dpiScale,dpiScale));
        ImGui::SameLine();
        ImGui::Button(ICON_FA_INDUSTRY "##SFilter");
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Apply filter");
        }
        if (openSampleFilterOpt) {
          openSampleFilterOpt=false;
          ImGui::OpenPopup("SFilterOpt");
        }
        if (ImGui::BeginPopupContextItem("SFilterOpt",ImGuiPopupFlags_MouseButtonLeft)) {
          float lowP=sampleFilterL*100.0f;
          float bandP=sampleFilterB*100.0f;
          float highP=sampleFilterH*100.0f;
          float resP=sampleFilterRes*100.0f;
          ImGui::Text("Cutoff:");
          if (CWSliderFloat("From",&sampleFilterCutStart,0.0f,sample->rate*0.5,"%.0fHz")) {
            if (sampleFilterCutStart<0.0) sampleFilterCutStart=0.0;
            if (sampleFilterCutStart>sample->rate*0.5) sampleFilterCutStart=sample->rate*0.5;
          }
          if (CWSliderFloat("To",&sampleFilterCutEnd,0.0f,sample->rate*0.5,"%.0fHz")) {
            if (sampleFilterCutEnd<0.0) sampleFilterCutEnd=0.0;
            if (sampleFilterCutEnd>sample->rate*0.5) sampleFilterCutEnd=sample->rate*0.5;
          }
          ImGui::Separator();
          if (CWSliderFloat("Resonance",&resP,0.0f,99.0f,"%.1f%%")) {
            sampleFilterRes=resP/100.0f;
            if (sampleFilterRes<0.0f) sampleFilterRes=0.0f;
            if (sampleFilterRes>0.99f) sampleFilterRes=0.99f;
          }
          ImGui::Text("Power");
          ImGui::SameLine();
          if (ImGui::RadioButton("1x",sampleFilterPower==1)) {
            sampleFilterPower=1;
          }
          ImGui::SameLine();
          if (ImGui::RadioButton("2x",sampleFilterPower==2)) {
            sampleFilterPower=2;
          }
          ImGui::SameLine();
          if (ImGui::RadioButton("3x",sampleFilterPower==3)) {
            sampleFilterPower=3;
          }
          ImGui::Separator();
          if (CWSliderFloat("Low-pass",&lowP,0.0f,100.0f,"%.1f%%")) {
            sampleFilterL=lowP/100.0f;
            if (sampleFilterL<0.0f) sampleFilterL=0.0f;
            if (sampleFilterL>1.0f) sampleFilterL=1.0f;
          }
          if (CWSliderFloat("Band-pass",&bandP,0.0f,100.0f,"%.1f%%")) {
            sampleFilterB=bandP/100.0f;
            if (sampleFilterB<0.0f) sampleFilterB=0.0f;
            if (sampleFilterB>1.0f) sampleFilterB=1.0f;
          }
          if (CWSliderFloat("High-pass",&highP,0.0f,100.0f,"%.1f%%")) {
            sampleFilterH=highP/100.0f;
            if (sampleFilterH<0.0f) sampleFilterH=0.0f;
            if (sampleFilterH>1.0f) sampleFilterH=1.0f;
          }

          if (ImGui::Button("Apply")) {
            sample->prepareUndo(true);
            e->lockEngine([this,sample]() {
              SAMPLE_OP_BEGIN;
              float res=1.0-pow(sampleFilterRes,0.5f);
              float low=0;
              float band=0;
              float high=0;

              double power=(sampleFilterCutStart>sampleFilterCutEnd)?0.5:2.0;

              if (sample->depth==DIV_SAMPLE_DEPTH_16BIT) {
                for (unsigned int i=start; i<end; i++) {
                  double freq=sampleFilterCutStart+(sampleFilterCutEnd-sampleFilterCutStart)*pow(double(i-start)/double(end-start),power);
                  double cut=sin((freq/double(sample->rate))*M_PI);

                  for (int j=0; j<sampleFilterPower; j++) {
                    low=low+cut*band;
                    high=float(sample->data16[i])-low-(res*band);
                    band=cut*high+band;
                  }

                  float val=low*sampleFilterL+band*sampleFilterB+high*sampleFilterH;
                  if (val<-32768) val=-32768;
                  if (val>32767) val=32767;
                  sample->data16[i]=val;
                }
              } else if (sample->depth==DIV_SAMPLE_DEPTH_8BIT) {
                for (unsigned int i=start; i<end; i++) {
                  double freq=sampleFilterCutStart+(sampleFilterCutEnd-sampleFilterCutStart)*pow(double(i-start)/double(end-start),power);
                  double cut=sin((freq/double(sample->rate))*M_PI);

                  for (int j=0; j<sampleFilterPower; j++) {
                    low=low+cut*band;
                    high=float(sample->data8[i])-low-(res*band);
                    band=cut*high+band;
                  }

                  float val=low*sampleFilterL+band*sampleFilterB+high*sampleFilterH;
                  if (val<-128) val=-128;
                  if (val>127) val=127;
                  sample->data8[i]=val;
                }
              }

              updateSampleTex=true;

              e->renderSamples();
            });
            MARK_MODIFIED;
            ImGui::CloseCurrentPopup();
          }
          ImGui::EndPopup();
        }
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(0.5*dpiScale,dpiScale));
        ImGui::SameLine();


        ImGui::Button(ICON_FA_VOLUME_UP "##SAmplify");
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Amplify");
        }
        if (openSampleAmplifyOpt) {
          openSampleAmplifyOpt=false;
          ImGui::OpenPopup("SAmplifyOpt");
        }
        if (ImGui::BeginPopupContextItem("SAmplifyOpt",ImGuiPopupFlags_MouseButtonLeft)) {
          ImGui::Text("Volume");
          if (ImGui::InputFloat("##SRVolume",&amplifyVol,10.0,50.0,"%g%%")) {
            if (amplifyVol<0) amplifyVol=0;
            if (amplifyVol>10000) amplifyVol=10000;
          }
          ImGui::SameLine();
          ImGui::Text("(%.1fdB)",20.0*log10(amplifyVol/100.0f));
          if (ImGui::Button("Apply")) {
            sample->prepareUndo(true);
            e->lockEngine([this,sample]() {
              SAMPLE_OP_BEGIN;
              float vol=amplifyVol/100.0f;

              if (sample->depth==DIV_SAMPLE_DEPTH_16BIT) {
                for (unsigned int i=start; i<end; i++) {
                  float val=sample->data16[i]*vol;
                  if (val<-32768) val=-32768;
                  if (val>32767) val=32767;
                  sample->data16[i]=val;
                }
              } else if (sample->depth==DIV_SAMPLE_DEPTH_8BIT) {
                for (unsigned int i=start; i<end; i++) {
                  float val=sample->data8[i]*vol;
                  if (val<-128) val=-128;
                  if (val>127) val=127;
                  sample->data8[i]=val;
                }
              }

              updateSampleTex=true;

              e->renderSamples();
            });
            MARK_MODIFIED;
            ImGui::CloseCurrentPopup();
          }
          ImGui::EndPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_ARROWS_V "##SNormalize")) {
          doAction(GUI_ACTION_SAMPLE_NORMALIZE);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Normalize");
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_ARROW_UP "##SFadeIn")) {
          doAction(GUI_ACTION_SAMPLE_FADE_IN);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Fade in");
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_ARROW_DOWN "##SFadeOut")) {
          doAction(GUI_ACTION_SAMPLE_FADE_OUT);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Fade out");
        }

        if (ImGui::Button(ICON_FA_UNDO "##SUndo")) {
          doUndoSample();
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Undo");
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_REPEAT "##SRedo")) {
          doRedoSample();
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Redo");
        }
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(7.0*dpiScale,dpiScale));
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_BACKWARD "##SReverse")) {
          doAction(GUI_ACTION_SAMPLE_REVERSE);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Reverse");
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_SORT_AMOUNT_ASC "##SInvert")) {
          doAction(GUI_ACTION_SAMPLE_INVERT);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Invert");
        }
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(0.5*dpiScale,dpiScale));
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_LEVEL_DOWN "##SSign")) {
          doAction(GUI_ACTION_SAMPLE_SIGN);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Signed/unsigned exchange");
        }
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(0.5*dpiScale,dpiScale));
        ImGui::SameLine();
        ImGui::Button(ICON_FA_ADJUST "##SInsertSilence");
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Insert silence");
        }
        if (openSampleSilenceOpt) {
          openSampleSilenceOpt=false;
          ImGui::OpenPopup("SSilenceOpt");
        }
        if (ImGui::BeginPopupContextItem("SSilenceOpt",ImGuiPopupFlags_MouseButtonLeft)) {
          if (ImGui::InputInt("Samples",&silenceSize,1,64)) {
            if (silenceSize<0) silenceSize=0;
            if (silenceSize>16777215) silenceSize=16777215;
          }
          if (ImGui::Button("Go")) {
            int pos=(sampleSelStart==-1 || sampleSelStart==sampleSelEnd)?sample->samples:sampleSelStart;
            sample->prepareUndo(true);
            e->lockEngine([this,sample,pos]() {
              if (!sample->insert(pos,silenceSize)) {
                showError("couldn't insert! make sure your sample is 8 or 16-bit.");
              }
              e->renderSamples();
            });
            updateSampleTex=true;
            sampleSelStart=pos;
            sampleSelEnd=pos+silenceSize;
            MARK_MODIFIED;
            ImGui::CloseCurrentPopup();
          }
          ImGui::EndPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_ERASER "##SSilence")) {
          doAction(GUI_ACTION_SAMPLE_SILENCE);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Apply silence");
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_TIMES "##SDelete")) {
          doAction(GUI_ACTION_SAMPLE_DELETE);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Delete");
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_CROP "##STrim")) {
          doAction(GUI_ACTION_SAMPLE_TRIM);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Trim");
        }

        ImGui::EndDisabled();

        if (ImGui::Button(ICON_FA_PLAY "##PreviewSample")) {
          e->previewSample(curSample);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Preview sample");
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_STOP "##StopSample")) {
          e->stopSamplePreview();
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Stop sample preview");
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_UPLOAD "##MakeIns")) {
          doAction(GUI_ACTION_SAMPLE_MAKE_INS);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Create instrument from sample");
        }
        ImGui::SameLine();
        double zoomPercent=100.0/sampleZoom;
        ImGui::Text("Zoom");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(150.0f*dpiScale);
        if (ImGui::InputDouble("##SZoom",&zoomPercent,5.0,20.0,"%g%%")) {
          if (zoomPercent>10000.0) zoomPercent=10000.0;
          if (zoomPercent<1.0) zoomPercent=1.0;
          sampleZoom=100.0/zoomPercent;
          if (sampleZoom<0.01) sampleZoom=0.01;
          sampleZoomAuto=false;
          updateSampleTex=true;
        }
        ImGui::SameLine();
        if (sampleZoomAuto) {
          if (ImGui::Button("100%")) {
            sampleZoom=1.0;
            sampleZoomAuto=false;
            updateSampleTex=true;
          }
        } else {
          if (ImGui::Button("Auto")) {
            sampleZoomAuto=true;
            updateSampleTex=true;
          }
        }
      }

      ImGui::Separator();

      ImVec2 avail=ImGui::GetContentRegionAvail(); // graph size determined here
      if (ImGui::GetContentRegionAvail().y>(ImGui::GetContentRegionAvail().x*0.5f)) {
        avail=ImVec2(ImGui::GetContentRegionAvail().x,ImGui::GetContentRegionAvail().x*0.5f);
      }
      avail.y-=ImGui::GetFontSize()+ImGui::GetStyle().ItemSpacing.y+ImGui::GetStyle().ScrollbarSize;
      if (avail.y<1.0) { // Prevents crash
        avail.y=1.0;
      }

      int availX=avail.x;
      int availY=avail.y;

      if (sampleZoomAuto) {
        samplePos=0;
        if (sample->samples<1 || avail.x<=0) {
          sampleZoom=1.0;
        } else {
          sampleZoom=(double)sample->samples/avail.x;
        }
        if (sampleZoom!=prevSampleZoom) {
          prevSampleZoom=sampleZoom;
          updateSampleTex=true;
        }
      }

      if (sampleTex==NULL || sampleTexW!=avail.x || sampleTexH!=avail.y) {
        if (sampleTex!=NULL) {
          SDL_DestroyTexture(sampleTex);
          sampleTex=NULL;
        }
        if (avail.x>=1 && avail.y>=1) {
          logD("recreating sample texture.");
          sampleTex=SDL_CreateTexture(sdlRend,SDL_PIXELFORMAT_ABGR8888,SDL_TEXTUREACCESS_STREAMING,avail.x,avail.y);
          sampleTexW=avail.x;
          sampleTexH=avail.y;
          if (sampleTex==NULL) {
            logE("error while creating sample texture! %s",SDL_GetError());
          } else {
            updateSampleTex=true;
          }
        }
      }

      if (sampleTex!=NULL) {
        if (updateSampleTex) {
          unsigned int* data=NULL;
          int pitch=0;
          logD("updating sample texture.");
          if (SDL_LockTexture(sampleTex,NULL,(void**)&data,&pitch)!=0) {
            logE("error while locking sample texture! %s",SDL_GetError());
          } else {
            ImU32 bgColor=ImGui::GetColorU32(uiColors[GUI_COLOR_SAMPLE_BG]);
            ImU32 bgColorLoop=ImGui::GetColorU32(uiColors[GUI_COLOR_SAMPLE_LOOP]);
            ImU32 lineColor=ImGui::GetColorU32(uiColors[GUI_COLOR_SAMPLE_FG]);
            ImU32 centerLineColor=ImGui::GetColorU32(uiColors[GUI_COLOR_SAMPLE_CENTER]);
            for (int i=0; i<availY; i++) {
              for (int j=0; j<availX; j++) {
                int scaledPos=samplePos+(j*sampleZoom);
                if (sample->isLoopable() && (scaledPos>=sample->loopStart && scaledPos<=sample->loopEnd)) {
                  data[i*availX+j]=bgColorLoop;
                } else {
                  data[i*availX+j]=bgColor;
                }
              }
            }
            if (availY>0) {
              for (int i=availX*(availY>>1); i<availX*(1+(availY>>1)); i++) {
                data[i]=centerLineColor;
              }
            }
            unsigned int xCoarse=samplePos;
            unsigned int xFine=0;
            unsigned int xAdvanceCoarse=sampleZoom;
            unsigned int xAdvanceFine=fmod(sampleZoom,1.0)*16777216;
            for (unsigned int i=0; i<(unsigned int)availX; i++) {
              if (xCoarse>=sample->samples) break;
              int y1, y2;
              int totalAdvance=0;
              if (sample->depth==DIV_SAMPLE_DEPTH_8BIT) {
                y1=((unsigned char)sample->data8[xCoarse]^0x80)*availY/256;
              } else {
                y1=((unsigned short)sample->data16[xCoarse]^0x8000)*availY/65536;
              }
              xFine+=xAdvanceFine;
              if (xFine>=16777216) {
                xFine-=16777216;
                totalAdvance++;
              }
              totalAdvance+=xAdvanceCoarse;
              if (xCoarse>=sample->samples) break;
              do {
                if (sample->depth==DIV_SAMPLE_DEPTH_8BIT) {
                  y2=((unsigned char)sample->data8[xCoarse]^0x80)*availY/256;
                } else {
                  y2=((unsigned short)sample->data16[xCoarse]^0x8000)*availY/65536;
                }
                if (y1>y2) {
                  y2^=y1;
                  y1^=y2;
                  y2^=y1;
                }
                if (y1<0) y1=0;
                if (y1>=availY) y1=availY-1;
                if (y2<0) y2=0;
                if (y2>=availY) y2=availY-1;
                for (int j=y1; j<=y2; j++) {
                  data[i+availX*(availY-j-1)]=lineColor;
                }
                if (totalAdvance>0) xCoarse++;
              } while ((totalAdvance--)>0);
            }
            SDL_UnlockTexture(sampleTex);
          }
          updateSampleTex=false;
        }

        ImGui::ImageButton(sampleTex,avail,ImVec2(0,0),ImVec2(1,1),0);

        ImVec2 rectMin=ImGui::GetItemRectMin();
        ImVec2 rectMax=ImGui::GetItemRectMax();
        ImVec2 rectSize=ImGui::GetItemRectSize();

        if (ImGui::IsItemClicked()) {
          nextWindow=GUI_WINDOW_SAMPLE_EDIT;
          if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            sampleDragActive=false;
            sampleSelStart=0;
            sampleSelEnd=sample->samples;
          } else {
            if (sample->samples>0) {
              sampleDragStart=rectMin;
              sampleDragAreaSize=rectSize;
              switch (sample->depth) {
                case DIV_SAMPLE_DEPTH_8BIT:
                  sampleDrag16=false;
                  sampleDragTarget=(void*)sample->data8;
                  break;
                case DIV_SAMPLE_DEPTH_16BIT:
                  sampleDrag16=true;
                  sampleDragTarget=(void*)sample->data16;
                  break;
                default:
                  sampleDrag16=true;
                  sampleDragTarget=NULL;
                  break;
              }
              sampleDragLen=sample->samples;
              sampleDragActive=true;
              sampleSelStart=-1;
              sampleSelEnd=-1;
              if (sampleDragMode) sample->prepareUndo(true);
              processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y);
            }
          }
        }

        if (!sampleDragMode && ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
          ImGui::OpenPopup("SRightClick");
        }

        if (ImGui::BeginPopup("SRightClick",ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_AlwaysAutoResize)) {
          ImGui::BeginDisabled(sample->depth!=DIV_SAMPLE_DEPTH_8BIT && sample->depth!=DIV_SAMPLE_DEPTH_16BIT);
          if (ImGui::MenuItem("cut",BIND_FOR(GUI_ACTION_SAMPLE_CUT))) {
            doAction(GUI_ACTION_SAMPLE_CUT);
          }
          ImGui::EndDisabled();
          if (ImGui::MenuItem("copy",BIND_FOR(GUI_ACTION_SAMPLE_COPY))) {
            doAction(GUI_ACTION_SAMPLE_COPY);
          }
          ImGui::BeginDisabled(sample->depth!=DIV_SAMPLE_DEPTH_8BIT && sample->depth!=DIV_SAMPLE_DEPTH_16BIT);
          if (ImGui::MenuItem("paste",BIND_FOR(GUI_ACTION_SAMPLE_PASTE))) {
            doAction(GUI_ACTION_SAMPLE_PASTE);
          }
          if (ImGui::MenuItem("paste (replace)",BIND_FOR(GUI_ACTION_SAMPLE_PASTE_REPLACE))) {
            doAction(GUI_ACTION_SAMPLE_PASTE_REPLACE);
          }
          if (ImGui::MenuItem("paste (mix)",BIND_FOR(GUI_ACTION_SAMPLE_PASTE_MIX))) {
            doAction(GUI_ACTION_SAMPLE_PASTE_MIX);
          }
          ImGui::EndDisabled();
          if (ImGui::MenuItem("select all",BIND_FOR(GUI_ACTION_SAMPLE_SELECT_ALL))) {
            doAction(GUI_ACTION_SAMPLE_SELECT_ALL);
          }
          ImGui::Separator();
          if (ImGui::MenuItem("set loop to selection",BIND_FOR(GUI_ACTION_SAMPLE_SET_LOOP))) {
            doAction(GUI_ACTION_SAMPLE_SET_LOOP);
          }
          if (ImGui::MenuItem("create wavetable from selection",BIND_FOR(GUI_ACTION_SAMPLE_CREATE_WAVE))) {
            doAction(GUI_ACTION_SAMPLE_CREATE_WAVE);
          }
          ImGui::EndPopup();
        }

        String statusBar=sampleDragMode?"Draw":"Select";
        bool drawSelection=false;

        if (!sampleDragMode) {
          if (sampleSelStart>=0 && sampleSelEnd>=0) {
            int start=sampleSelStart;
            int end=sampleSelEnd;
            if (start>end) {
              start^=end;
              end^=start;
              start^=end;
            }
            if (start==end) {
              statusBar+=fmt::sprintf(" (%d)",start);
            } else {
              statusBar+=fmt::sprintf(" (%d-%d: %d samples)",start,end,end-start);
            }
            drawSelection=true;
          }
        }

        if (ImGui::IsItemHovered()) {
          if (ctrlWheeling) {
            double zoomPercent=100.0/sampleZoom;
            zoomPercent+=wheelY*10.0;
            if (zoomPercent>10000.0) zoomPercent=10000.0;
            if (zoomPercent<1.0) zoomPercent=1.0;
            sampleZoom=100.0/zoomPercent;
            if (sampleZoom<0.01) sampleZoom=0.01;
            sampleZoomAuto=false;
            int bounds=((int)sample->samples-round(rectSize.x*sampleZoom));
            if (bounds<0) bounds=0;
            if (samplePos>bounds) samplePos=bounds;
            updateSampleTex=true;
          } else {
            if (wheelY!=0) {
              if (!sampleZoomAuto) {
                double scrollAmount=MAX(fabs((double)wheelY*sampleZoom*60.0),1.0);
                if (wheelY>0) {
                  samplePos+=scrollAmount;
                } else {
                  samplePos-=scrollAmount;
                }
                if (samplePos<0) samplePos=0;
                int bounds=((int)sample->samples-round(rectSize.x*sampleZoom));
                if (bounds<0) bounds=0;
                if (samplePos>bounds) samplePos=bounds;
                updateSampleTex=true;
              }
            }
          }

          int posX=-1;
          int posY=0;
          ImVec2 pos=ImGui::GetMousePos();
          pos.x-=rectMin.x;
          pos.y-=rectMin.y;

          if (sampleZoom>0) {
            posX=samplePos+pos.x*sampleZoom;
            if (posX>(int)sample->samples) posX=-1;
          }
          posY=(0.5-pos.y/rectSize.y)*((sample->depth==DIV_SAMPLE_DEPTH_8BIT)?255:32767);
          if (posX>=0) {
            statusBar+=fmt::sprintf(" | (%d, %d)",posX,posY);
          }
        }

        if (e->isPreviewingSample()) {
          statusBar+=fmt::sprintf(" | %.2fHz",e->getSamplePreviewRate());

          int start=sampleSelStart;
          int end=sampleSelEnd;
          if (start>end) {
            start^=end;
            end^=start;
            start^=end;
          }
          ImDrawList* dl=ImGui::GetWindowDrawList();
          ImVec2 p1=rectMin;
          p1.x+=(e->getSamplePreviewPos()-samplePos)/sampleZoom;     
          ImVec4 posColor=uiColors[GUI_COLOR_SAMPLE_NEEDLE];
          ImVec4 posTrail1=posColor;
          ImVec4 posTrail2=posColor;
          posTrail1.w*=0.5f;
          posTrail2.w=0.0f;
          float trailDistance=(e->getSamplePreviewRate()/100.0f)/sampleZoom;

          if (p1.x<rectMin.x) p1.x=rectMin.x;
          if (p1.x>rectMax.x) p1.x=rectMax.x;

          ImVec2 p2=p1;
          p2.y=rectMax.y;

          dl->AddRectFilledMultiColor(
            ImVec2(p1.x-trailDistance,p1.y),
            p2,
            ImGui::GetColorU32(posTrail2),
            ImGui::GetColorU32(posTrail1),
            ImGui::GetColorU32(posTrail1),
            ImGui::GetColorU32(posTrail2)
          );
          dl->AddLine(p1,p2,ImGui::GetColorU32(posColor));
        }

        if (drawSelection) {
          int start=sampleSelStart;
          int end=sampleSelEnd;
          if (start>end) {
            start^=end;
            end^=start;
            start^=end;
          }
          ImDrawList* dl=ImGui::GetWindowDrawList();
          ImVec2 p1=rectMin;
          p1.x+=(start-samplePos)/sampleZoom;

          ImVec2 p2=ImVec2(rectMin.x+(end-samplePos)/sampleZoom,rectMax.y);
          ImVec4 boundColor=uiColors[GUI_COLOR_SAMPLE_SEL_POINT];
          ImVec4 selColor=uiColors[GUI_COLOR_SAMPLE_SEL];

          if (p1.x<rectMin.x) p1.x=rectMin.x;
          if (p1.x>rectMax.x) p1.x=rectMax.x;

          if (p2.x<rectMin.x) p2.x=rectMin.x;
          if (p2.x>rectMax.x) p2.x=rectMax.x;

          dl->AddRectFilled(p1,p2,ImGui::GetColorU32(selColor));
          dl->AddLine(ImVec2(p1.x,p1.y),ImVec2(p1.x,p2.y),ImGui::GetColorU32(boundColor));
          if (start!=end) {
            dl->AddLine(ImVec2(p2.x,p1.y),ImVec2(p2.x,p2.y),ImGui::GetColorU32(boundColor));
          }
        }

        ImS64 scrollV=samplePos;
        ImS64 availV=round(rectSize.x*sampleZoom);
        ImS64 contentsV=MAX(sample->samples,MAX(availV,1));

        ImGuiID scrollbarID=ImGui::GetID("sampleScroll");
        ImGui::KeepAliveID(scrollbarID);
        if (ImGui::ScrollbarEx(ImRect(ImVec2(rectMin.x,rectMax.y),ImVec2(rectMax.x,rectMax.y+ImGui::GetStyle().ScrollbarSize)),scrollbarID,ImGuiAxis_X,&scrollV,availV,contentsV,0)) {
          if (!sampleZoomAuto && samplePos!=scrollV) {
            samplePos=scrollV;
            updateSampleTex=true;
          }
        }

        if (sample->depth!=DIV_SAMPLE_DEPTH_8BIT && sample->depth!=DIV_SAMPLE_DEPTH_16BIT && sampleDragMode) {
          statusBar="Non-8/16-bit samples cannot be edited without prior conversion.";
        }
        
        ImGui::SetCursorPosY(ImGui::GetCursorPosY()+ImGui::GetStyle().ScrollbarSize);
        ImGui::Text("%s",statusBar.c_str());
      }
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_SAMPLE_EDIT;
  ImGui::End();
}

void FurnaceGUI::doUndoSample() {
  if (!sampleEditOpen) return;
  if (curSample<0 || curSample>=(int)e->song.sample.size()) return;
  DivSample* sample=e->song.sample[curSample];
  e->lockEngine([this,sample]() {
    if (sample->undo()==2) {
      e->renderSamples();
      updateSampleTex=true;
    }
  });
}

void FurnaceGUI::doRedoSample() {
  if (!sampleEditOpen) return;
  if (curSample<0 || curSample>=(int)e->song.sample.size()) return;
  DivSample* sample=e->song.sample[curSample];
  e->lockEngine([this,sample]() {
    if (sample->redo()==2) {
      e->renderSamples();
      updateSampleTex=true;
    }
  });
}
