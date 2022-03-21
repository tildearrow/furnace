#include "gui.h"
#include "plot_nolerp.h"
#include "misc/cpp/imgui_stdlib.h"

void FurnaceGUI::drawWaveEdit() {
  if (nextWindow==GUI_WINDOW_WAVE_EDIT) {
    waveEditOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!waveEditOpen) return;
  float wavePreview[256];
  ImGui::SetNextWindowSizeConstraints(ImVec2(450.0f*dpiScale,300.0f*dpiScale),ImVec2(scrW*dpiScale,scrH*dpiScale));
  if (ImGui::Begin("Wavetable Editor",&waveEditOpen,settings.allowEditDocking?0:ImGuiWindowFlags_NoDocking)) {
    if (curWave<0 || curWave>=(int)e->song.wave.size()) {
      ImGui::Text("no wavetable selected");
    } else {
      DivWavetable* wave=e->song.wave[curWave];
      ImGui::Text("Width");
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("use a width of:\n- any on Amiga/X1-010\n- 32 on Game Boy, PC Engine and WonderSwan\nany other widths will be scaled during playback.");
      }
      ImGui::SameLine();
      ImGui::SetNextItemWidth(128.0f*dpiScale);
      if (ImGui::InputInt("##_WTW",&wave->len,1,2)) {
        if (wave->len>256) wave->len=256;
        if (wave->len<1) wave->len=1;
        e->notifyWaveChange(curWave);
        if (wavePreviewOn) e->previewWave(curWave,wavePreviewNote);
        MARK_MODIFIED;
      }
      ImGui::SameLine();
      ImGui::Text("Height");
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("use a height of:\n- 15 for Game Boy and WonderSwan\n- 31 for PC Engine\nany other heights will be scaled during playback.");
      }
      ImGui::SameLine();
      ImGui::SetNextItemWidth(128.0f*dpiScale);
      if (ImGui::InputInt("##_WTH",&wave->max,1,2)) {
        if (wave->max>255) wave->max=255;
        if (wave->max<1) wave->max=1;
        e->notifyWaveChange(curWave);
        MARK_MODIFIED;
      }
      for (int i=0; i<wave->len; i++) {
        if (wave->data[i]>wave->max) wave->data[i]=wave->max;
        wavePreview[i]=wave->data[i];
      }
      if (wave->len>0) wavePreview[wave->len]=wave->data[wave->len-1];
      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x); // wavetable text input size found here
      if (ImGui::InputText("##MMLWave",&mmlStringW)) {
        decodeMMLStrW(mmlStringW,wave->data,wave->len,wave->max);
      }
      if (!ImGui::IsItemActive()) {
        encodeMMLStr(mmlStringW,wave->data,wave->len,-1,-1);
      }

      ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,ImVec2(0.0f,0.0f));

      ImVec2 contentRegion=ImGui::GetContentRegionAvail(); // wavetable graph size determined here
      if (ImGui::GetContentRegionAvail().y > (ImGui::GetContentRegionAvail().x / 2.0f)) {
        contentRegion=ImVec2(ImGui::GetContentRegionAvail().x,ImGui::GetContentRegionAvail().x / 2.0f);
      }
      PlotNoLerp("##Waveform",wavePreview,wave->len+1,0,NULL,0,wave->max,contentRegion);
      if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        waveDragStart=ImGui::GetItemRectMin();
        waveDragAreaSize=contentRegion;
        waveDragMin=0;
        waveDragMax=wave->max;
        waveDragLen=wave->len;
        waveDragActive=true;
        waveDragTarget=wave->data;
        processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y);
        e->notifyWaveChange(curWave);
        modified=true;
      }
      ImGui::PopStyleVar();
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_WAVE_EDIT;
  ImGui::End();
}