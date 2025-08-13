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
#include "IconsFontAwesome4.h"
#include "misc/cpp/imgui_stdlib.h"
#include "plot_nolerp.h"
#include "guiConst.h"
#include "../ta-log.h"
#include <fmt/printf.h>
#include <imgui.h>
#include <imgui_internal.h>

const char* sampleNote[12]={
  "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

#define DIR_DRAG_SOURCE(_d,_a,_c) \
  if (ImGui::BeginDragDropSource()) { \
    dirToMove=_d; \
    assetToMove=_a; \
    ImGui::SetDragDropPayload(_c,NULL,0,ImGuiCond_Once); \
    ImGui::Button(ICON_FA_ARROWS "##AssetDrag"); \
    ImGui::EndDragDropSource(); \
  }

#define DIR_DRAG_TARGET(_d,_a,_type,_c) \
  if (ImGui::BeginDragDropTarget()) { \
    const ImGuiPayload* dragItem=ImGui::AcceptDragDropPayload(_c); \
    if (dragItem!=NULL) { \
      if (dragItem->IsDataType(_c)) { \
        if (assetToMove==-1) { \
          if (dirToMove!=_d && _a==-1) { \
            e->lockEngine([&]() { \
              DivAssetDir val=_type[dirToMove]; \
              _type.erase(_type.begin()+dirToMove); \
              _type.insert(_type.begin()+_d,val); \
            }); \
          } \
        } else { \
          if (dirToMove!=_d || assetToMove!=_a) { \
            logV("%d/%d -> %d/%d",dirToMove,assetToMove,_d,_a); \
            e->lockEngine([&]() { \
              int val=_type[dirToMove].entries[assetToMove]; \
              _type[dirToMove].entries.erase(_type[dirToMove].entries.begin()+assetToMove); \
              _type[_d].entries.insert((_a<0)?(_type[_d].entries.end()):(_type[_d].entries.begin()+_a),val); \
            }); \
          } \
        } \
        dirToMove=-1; \
        assetToMove=-1; \
      } \
    } \
    ImGui::EndDragDropTarget(); \
  }

#define SIMPLE_DRAG_SOURCE(_c,_toMoveVar) \
  if (settings.draggableDataView) { \
    if (ImGui::BeginDragDropSource()) { \
      _toMoveVar=i; \
      ImGui::SetDragDropPayload(_c,NULL,0,ImGuiCond_Once); \
      ImGui::Button(ICON_FA_ARROWS "##AssetDrag"); \
      ImGui::EndDragDropSource(); \
    } \
  }

#define SIMPLE_DRAG_TARGET(_c,_toMoveVar,_curVar,_swapFn,_moveUpFn,_moveDownFn) \
  if (settings.draggableDataView) { \
    if (ImGui::BeginDragDropTarget()) { \
      const ImGuiPayload* payload=ImGui::AcceptDragDropPayload(_c); \
      if (payload!=NULL) { \
        int target=i; \
        bool markModified=false; \
        if (_toMoveVar!=target) { \
          if (ImGui::IsKeyDown(ImGuiKey_ModCtrl)) { \
            markModified=_swapFn(_toMoveVar,target); \
          } else { \
            while (_toMoveVar>target) { \
              if (_moveUpFn(_toMoveVar)) { \
                _toMoveVar--; \
                markModified=true; \
              } else { \
                break; \
              } \
            } \
            while (_toMoveVar<target) { \
              if (_moveDownFn(_toMoveVar)) { \
                _toMoveVar++; \
                markModified=true; \
              } else { \
                break; \
              } \
            } \
          } \
          _curVar=target; \
        } \
        if (markModified) { \
          MARK_MODIFIED; \
        } \
        _toMoveVar=-1; \
      } \
      ImGui::EndDragDropTarget(); \
    } \
  }

void FurnaceGUI::insListItem(int i, int dir, int asset) {
  ImGui::PushID(i);
  String name=(settings.insIconsStyle==0)?"":ICON_FA_CIRCLE_O;
  const char* insType=_("Bug!");
  if (i>=0 && i<e->song.insLen) {
    DivInstrument* ins=e->song.ins[i];
    insType=(ins->type>=DIV_INS_MAX)?_("Unknown"):_(insTypes[ins->type][0]);
    const char** insIcon=NULL;

    if (ins->type>=DIV_INS_MAX) {
      ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_UNKNOWN]);
      insIcon=insTypes[DIV_INS_MAX];
    } else {
      ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_STD+ins->type]);
      insIcon=insTypes[ins->type];
    }

    switch (settings.insIconsStyle) {
      case 1:
        name=fmt::sprintf("%s##_INS%d",insIcon[1],i);
        break;
      case 2:
        name=fmt::sprintf("%s##_INS%d",insIcon[2],i);
        break;
      default:
        name=fmt::sprintf("##_INS%d",i);
        break;
    }
  } else {
    ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_TEXT]);
  }
  bool insReleased=ImGui::Selectable(name.c_str(),(i==-1)?(curIns<0 || curIns>=e->song.insLen):(curIns==i));
  bool insPressed=ImGui::IsItemActivated();
  if (insReleased || (!insListDir && insPressed && !settings.draggableDataView)) {
    curIns=i;
    if (!insReleased || insListDir || settings.draggableDataView) {
      wavePreviewInit=true;
      updateFMPreview=true;
    }
    lastAssetType=0;
    if (settings.insFocusesPattern && patternOpen)
      nextWindow=GUI_WINDOW_PATTERN;
  }
  if (wantScrollListIns && curIns==i) ImGui::SetScrollHereY();
  if (ImGui::IsItemHovered() && i>=0 && !mobileUI) {
    ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_TEXT]);
    ImGui::SetTooltip("%s",insType);
    ImGui::PopStyleColor();
    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
      nextWindow=GUI_WINDOW_INS_EDIT;
    }
  }
  if (i>=0) {
    if (insListDir) {
      DIR_DRAG_SOURCE(dir,asset,"FUR_INSDIR");
      DIR_DRAG_TARGET(dir,asset,e->song.insDir,"FUR_INSDIR");
    } else {
      SIMPLE_DRAG_SOURCE("FUR_INS",insToMove);
      SIMPLE_DRAG_TARGET("FUR_INS",insToMove,curIns,e->swapInstruments,e->moveInsUp,e->moveInsDown);
    }

    if (ImGui::BeginPopupContextItem("InsRightMenu")) {
      curIns=i;
      updateFMPreview=true;
      ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_TEXT]);
      if (ImGui::MenuItem(_("edit"))) {
        nextWindow=GUI_WINDOW_INS_EDIT;
      }
      if (ImGui::MenuItem(_("duplicate"))) {
        doAction(GUI_ACTION_INS_LIST_DUPLICATE);
      }
      if (ImGui::MenuItem(_("replace..."))) {
        doAction((curIns>=0 && curIns<(int)e->song.ins.size())?GUI_ACTION_INS_LIST_OPEN_REPLACE:GUI_ACTION_INS_LIST_OPEN);
      }
      if (ImGui::MenuItem(_("save"))) {
        doAction(GUI_ACTION_INS_LIST_SAVE);
      }
      if (ImGui::MenuItem(_("save (.dmp)"))) {
        doAction(GUI_ACTION_INS_LIST_SAVE_DMP);
      }
      if (ImGui::MenuItem(_("delete"))) {
        doAction(GUI_ACTION_INS_LIST_DELETE);
      }
      ImGui::PopStyleColor();
      ImGui::EndPopup();
    }
  }
  if (i>=0) {
    if (i<(int)e->song.ins.size()) {
      DivInstrument* ins=e->song.ins[i];
      ImGui::SameLine();
      ImGui::TextNoHashHide("%.2X: %s",i,ins->name.c_str());
    } else {
      ImGui::SameLine();
      ImGui::TextNoHashHide("%.2X: <INVALID>",i);
    }
  } else {
    ImGui::SameLine();
    ImGui::TextNoHashHide("- None -");
  }
  ImGui::PopID();
  ImGui::PopStyleColor();
}

void FurnaceGUI::waveListItem(int i, float* wavePreview, int dir, int asset) {
  DivWavetable* wave=e->song.wave[i];
  for (int i=0; i<wave->len; i++) {
    wavePreview[i]=wave->data[i];
  }
  if (wave->len>0) wavePreview[wave->len]=wave->data[wave->len-1];
  ImVec2 curPos=ImGui::GetCursorPos();
  ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign,ImVec2(0,0.5f));
  if (ImGui::Selectable(fmt::sprintf(" %d##_WAVE%d\n",i,i).c_str(),curWave==i,0,ImVec2(0,ImGui::GetFrameHeight()))) {
    curWave=i;
    lastAssetType=1;
  }
  ImGui::PopStyleVar();
  curPos.x+=ImGui::CalcTextSize("2").x*(2+((int)log10(MAX(1,e->song.waveLen-1))));
  if (wantScrollListWave && curWave==i) ImGui::SetScrollHereY();
  if (ImGui::IsItemHovered()) {
    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
      nextWindow=GUI_WINDOW_WAVE_EDIT;
    }
  }
  if (waveListDir || (settings.unifiedDataView && insListDir)) {
    DIR_DRAG_SOURCE(dir,asset,"FUR_WAVEDIR");
    DIR_DRAG_TARGET(dir,asset,e->song.waveDir,"FUR_WAVEDIR");
  } else {
    SIMPLE_DRAG_SOURCE("FUR_WAVE",waveToMove);
    SIMPLE_DRAG_TARGET("FUR_WAVE",waveToMove,curWave,e->swapWaves,e->moveWaveUp,e->moveWaveDown);
  }
  ImGui::SameLine();
  ImGui::SetCursorPos(curPos);
  PlotNoLerp(fmt::sprintf("##_WAVEP%d",i).c_str(),wavePreview,wave->len+1,0,NULL,0,wave->max,ImVec2(ImGui::GetContentRegionAvail().x,ImGui::GetFrameHeight()));
}

void FurnaceGUI::sampleListItem(int i, int dir, int asset) {
  bool memWarning=false;

  ImGui::PushID(i);

  DivSample* sample=e->song.sample[i];
  for (int j=0; j<e->song.systemLen; j++) {
    DivDispatch* dispatch=e->getDispatch(j);
    if (dispatch==NULL) continue;

    for (int k=0; k<DIV_MAX_SAMPLE_TYPE; k++) {
      if (dispatch->getSampleMemCapacity(k)==0) continue;
      if (!dispatch->isSampleLoaded(k,i) && sample->renderOn[k][j]) {
        memWarning=true;
        break;
      }
    }
    if (memWarning) break;
  }
  if (memWarning) ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_SAMPLE_CHIP_WARNING]);
  if (ImGui::Selectable(fmt::sprintf("%d:##_SAM%d",i,i).c_str(),curSample==i)) {
    curSample=i;
    samplePos=0;
    updateSampleTex=true;
    lastAssetType=2;
  }
  if (ImGui::IsItemHovered() && !mobileUI) {
    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
      nextWindow=GUI_WINDOW_SAMPLE_EDIT;
    }
  }
  if (ImGui::BeginPopupContextItem("SampleRightMenu")) {
    curSample=i;
    samplePos=0;
    updateSampleTex=true;
    lastAssetType=2;
    ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_TEXT]);
    if (ImGui::MenuItem(_("edit"))) {
      nextWindow=GUI_WINDOW_SAMPLE_EDIT;
    }
    if (ImGui::MenuItem(_("make instrument"))) {
      doAction(GUI_ACTION_SAMPLE_MAKE_INS);
    }
    if (ImGui::MenuItem(_("make me a drum kit"))) {
      doAction(GUI_ACTION_SAMPLE_LIST_MAKE_MAP);
    }
    if (ImGui::MenuItem(_("duplicate"))) {
      doAction(GUI_ACTION_SAMPLE_LIST_DUPLICATE);
    }
    if (ImGui::MenuItem(_("replace..."))) {
      doAction((curSample>=0 && curSample<(int)e->song.sample.size())?GUI_ACTION_SAMPLE_LIST_OPEN_REPLACE:GUI_ACTION_SAMPLE_LIST_OPEN);
    }
    if (ImGui::MenuItem(_("save"))) {
      doAction(GUI_ACTION_SAMPLE_LIST_SAVE);
    }
    if (ImGui::MenuItem(_("delete"))) {
      doAction(GUI_ACTION_SAMPLE_LIST_DELETE);
    }
    ImGui::PopStyleColor();
    ImGui::EndPopup();
  }
  if (sampleListDir || (settings.unifiedDataView && insListDir)) {
    DIR_DRAG_SOURCE(dir,asset,"FUR_SDIR");
    DIR_DRAG_TARGET(dir,asset,e->song.sampleDir,"FUR_SDIR");
  } else {
    SIMPLE_DRAG_SOURCE("FUR_SAMPLE",sampleToMove);
    SIMPLE_DRAG_TARGET("FUR_SAMPLE",sampleToMove,curSample,e->swapSamples,e->moveSampleUp,e->moveSampleDown);
  }
  ImGui::SameLine();
  ImGui::TextNoHashHide("%s",sample->name.c_str());
  if (memWarning) {
    ImGui::SameLine();
    ImGui::Text(ICON_FA_EXCLAMATION_TRIANGLE);
    if (ImGui::IsItemHovered() && !mobileUI) {
      ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_TEXT]);
      ImGui::SetTooltip(_("out of memory for this sample!"));
      ImGui::PopStyleColor();
    }
    ImGui::PopStyleColor();
  }
  if (wantScrollListSample && curSample==i) ImGui::SetScrollHereY();
  ImGui::PopID();
}

void FurnaceGUI::drawInsList(bool asChild) {
  if (nextWindow==GUI_WINDOW_INS_LIST) {
    insListOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!insListOpen && !asChild) return;
  bool began=false;
  if (asChild) {
    began=ImGui::BeginChild("Instruments");
  } else {
    began=ImGui::Begin("Instruments",&insListOpen,globalWinFlags,_("Instruments"));
  }
  if (began) {
    if (settings.unifiedDataView) settings.horizontalDataView=0;
    if (ImGui::Button(ICON_FA_PLUS "##InsAdd")) {
      if (settings.unifiedDataView) {
        switch (lastAssetType) {
          case 0:
            doAction(GUI_ACTION_INS_LIST_ADD);
            break;
          case 1:
            doAction(GUI_ACTION_WAVE_LIST_ADD);
            break;
          case 2:
            doAction(GUI_ACTION_SAMPLE_LIST_ADD);
            break;
        }
      } else {
        doAction(GUI_ACTION_INS_LIST_ADD);
      }
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip(_("Add"));
    }
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
      makeInsTypeList=e->getPossibleInsTypes();
      displayInsTypeList=true;
      displayInsTypeListMakeInsSample=-1;
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FILES_O "##InsClone")) {
      if (settings.unifiedDataView) {
        switch (lastAssetType) {
          case 0:
            doAction(GUI_ACTION_INS_LIST_DUPLICATE);
            break;
          case 1:
            doAction(GUI_ACTION_WAVE_LIST_DUPLICATE);
            break;
          case 2:
            doAction(GUI_ACTION_SAMPLE_LIST_DUPLICATE);
            break;
        }
      } else {
        doAction(GUI_ACTION_INS_LIST_DUPLICATE);
      }
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip(_("Duplicate"));
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FOLDER_OPEN "##InsLoad")) {
      if (settings.unifiedDataView) {
        switch (lastAssetType) {
          case 0:
            doAction(GUI_ACTION_INS_LIST_OPEN);
            break;
          case 1:
            doAction(GUI_ACTION_WAVE_LIST_OPEN);
            break;
          case 2:
            doAction(GUI_ACTION_SAMPLE_LIST_OPEN);
            break;
        }
      } else {
        doAction(GUI_ACTION_INS_LIST_OPEN);
      }
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip(_("Open"));
    }
    if (ImGui::BeginPopupContextItem("InsOpenOpt")) {
      if (settings.unifiedDataView) {
        if (ImGui::MenuItem(_("replace instrument..."))) {
          doAction((curIns>=0 && curIns<(int)e->song.ins.size())?GUI_ACTION_INS_LIST_OPEN_REPLACE:GUI_ACTION_INS_LIST_OPEN);
        }
        if (ImGui::MenuItem(_("load instrument from TX81Z"))) {
          doAction(GUI_ACTION_TX81Z_REQUEST);
        }

        ImGui::Separator();

        if (ImGui::MenuItem(_("replace wavetable..."))) {
          doAction((curWave>=0 && curWave<(int)e->song.wave.size())?GUI_ACTION_WAVE_LIST_OPEN_REPLACE:GUI_ACTION_WAVE_LIST_OPEN);
        }

        ImGui::Separator();

        if (ImGui::MenuItem(_("replace sample..."))) {
          doAction((curSample>=0 && curSample<(int)e->song.sample.size())?GUI_ACTION_SAMPLE_LIST_OPEN_REPLACE:GUI_ACTION_SAMPLE_LIST_OPEN);
        }
        if (ImGui::MenuItem(_("import raw sample..."))) {
          doAction(GUI_ACTION_SAMPLE_LIST_OPEN_RAW);
        }
        if (ImGui::MenuItem(_("import raw sample (replace)..."))) {
          doAction((curSample>=0 && curSample<(int)e->song.sample.size())?GUI_ACTION_SAMPLE_LIST_OPEN_REPLACE_RAW:GUI_ACTION_SAMPLE_LIST_OPEN_RAW);
        }
      } else {
        if (ImGui::MenuItem(_("replace..."))) {
          doAction((curIns>=0 && curIns<(int)e->song.ins.size())?GUI_ACTION_INS_LIST_OPEN_REPLACE:GUI_ACTION_INS_LIST_OPEN);
        }
        ImGui::Separator();
        if (ImGui::MenuItem(_("load from TX81Z"))) {
          doAction(GUI_ACTION_TX81Z_REQUEST);
        }
      }
      ImGui::EndPopup();
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip(_("Open (insert; right-click to replace)"));
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FLOPPY_O "##InsSave")) {
      if (settings.unifiedDataView) {
        switch (lastAssetType) {
          case 0:
            doAction(GUI_ACTION_INS_LIST_SAVE);
            break;
          case 1:
            doAction(GUI_ACTION_WAVE_LIST_SAVE);
            break;
          case 2:
            doAction(GUI_ACTION_SAMPLE_LIST_SAVE);
            break;
        }
      } else {
        doAction(GUI_ACTION_INS_LIST_SAVE);
      }
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip(_("Save"));
    }
    if (ImGui::BeginPopupContextItem("InsSaveFormats",ImGuiMouseButton_Right)) {
      if (settings.unifiedDataView) {
        if (ImGui::MenuItem(_("save instrument as .dmp..."))) {
          doAction(GUI_ACTION_INS_LIST_SAVE_DMP);
        }

        ImGui::Separator();

        if (ImGui::MenuItem(_("save wavetable as .dmw..."))) {
          doAction(GUI_ACTION_WAVE_LIST_SAVE_DMW);
        }
        if (ImGui::MenuItem(_("save raw wavetable..."))) {
          doAction(GUI_ACTION_WAVE_LIST_SAVE_RAW);
        }

        ImGui::Separator();

        if (ImGui::MenuItem(_("save raw sample..."))) {
          doAction(GUI_ACTION_SAMPLE_LIST_SAVE_RAW);
        }

        ImGui::Separator();

        if (ImGui::MenuItem(_("save all instruments..."))) {
          doAction(GUI_ACTION_INS_LIST_SAVE_ALL);
        }
        if (ImGui::MenuItem(_("save all wavetables..."))) {
          doAction(GUI_ACTION_WAVE_LIST_SAVE_ALL);
        }
        if (ImGui::MenuItem(_("save all samples..."))) {
          doAction(GUI_ACTION_SAMPLE_LIST_SAVE_ALL);
        }
      } else {
        if (ImGui::MenuItem(_("save as .dmp..."))) {
          doAction(GUI_ACTION_INS_LIST_SAVE_DMP);
        }

        if (ImGui::MenuItem(_("save all..."))) {
          doAction(GUI_ACTION_INS_LIST_SAVE_ALL);
        }
      }
      ImGui::EndPopup();
    }
    ImGui::SameLine();
    pushToggleColors(insListDir);
    if (ImGui::Button(ICON_FA_SITEMAP "##DirMode")) {
      doAction(GUI_ACTION_INS_LIST_DIR_VIEW);
    }
    popToggleColors();
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip(_("Toggle folders/standard view"));
    }
    if (!insListDir) {
      ImGui::SameLine();
      if (ImGui::Button(ICON_FA_ARROW_UP "##InsUp")) {
        if (settings.unifiedDataView) {
          switch (lastAssetType) {
            case 0:
              doAction(GUI_ACTION_INS_LIST_MOVE_UP);
              break;
            case 1:
              doAction(GUI_ACTION_WAVE_LIST_MOVE_UP);
              break;
            case 2:
              doAction(GUI_ACTION_SAMPLE_LIST_MOVE_UP);
              break;
          }
        } else {
          doAction(GUI_ACTION_INS_LIST_MOVE_UP);
        }
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(_("Move up"));
      }
      ImGui::SameLine();
      if (ImGui::Button(ICON_FA_ARROW_DOWN "##InsDown")) {
        if (settings.unifiedDataView) {
          switch (lastAssetType) {
            case 0:
              doAction(GUI_ACTION_INS_LIST_MOVE_DOWN);
              break;
            case 1:
              doAction(GUI_ACTION_WAVE_LIST_MOVE_DOWN);
              break;
            case 2:
              doAction(GUI_ACTION_SAMPLE_LIST_MOVE_DOWN);
              break;
          }
        } else {
          doAction(GUI_ACTION_INS_LIST_MOVE_DOWN);
        }
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(_("Move down"));
      }
    } else {
      ImGui::SameLine();
      if (ImGui::Button(ICON_FA_FOLDER "##InsFolder")) {
        folderString="";
      }
      if (ImGui::BeginPopupContextItem("NewInsFolder",ImGuiMouseButton_Left)) {
        ImGui::InputText("##FolderName",&folderString);
        ImGui::SameLine();
        ImGui::BeginDisabled(folderString.empty());
        if (ImGui::Button(_("Create"))) {
          if (settings.unifiedDataView) {
            switch (lastAssetType) {
              case 0:
                e->lockEngine([this]() {
                  e->song.insDir.push_back(DivAssetDir(folderString));
                });
                break;
              case 1:
                e->lockEngine([this]() {
                  e->song.waveDir.push_back(DivAssetDir(folderString));
                });
                break;
              case 2:
                e->lockEngine([this]() {
                  e->song.sampleDir.push_back(DivAssetDir(folderString));
                });
                break;
            }
          } else {
            e->lockEngine([this]() {
              e->song.insDir.push_back(DivAssetDir(folderString));
            });
          }
          ImGui::CloseCurrentPopup();
        }
        ImGui::EndDisabled();
        ImGui::EndPopup();
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(_("New folder"));
      }
    }
    if (lastAssetType==2) {
      ImGui::SameLine();
      if (ImGui::Button(ICON_FA_VOLUME_UP "##PreviewSampleL")) {
        doAction(GUI_ACTION_SAMPLE_LIST_PREVIEW);
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(_("Preview (right click to stop)"));
      }
      if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        doAction(GUI_ACTION_SAMPLE_LIST_STOP_PREVIEW);
      }
    }
    ImGui::SameLine();
    pushDestColor();
    if (ImGui::Button(ICON_FA_TIMES "##InsDelete")) {
      if (settings.unifiedDataView) {
        switch (lastAssetType) {
          case 0:
            doAction(GUI_ACTION_INS_LIST_DELETE);
            break;
          case 1:
            doAction(GUI_ACTION_WAVE_LIST_DELETE);
            break;
          case 2:
            doAction(GUI_ACTION_SAMPLE_LIST_DELETE);
            break;
        }
      } else {
        doAction(GUI_ACTION_INS_LIST_DELETE);
      }
    }
    popDestColor();
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip(_("Delete"));
    }
    ImGui::Separator();
    int availableRows=ImGui::GetContentRegionAvail().y/ImGui::GetFrameHeight();
    if (availableRows<1) availableRows=1;
    int columns=settings.horizontalDataView?(int)(ceil((double)(e->song.ins.size()+1)/(double)availableRows)):1;
    if (columns<1) columns=1;
    if (columns>64) columns=64;
    if (insListDir) columns=1;
    if (ImGui::BeginTable("InsListScroll",columns,(settings.horizontalDataView?ImGuiTableFlags_ScrollX:0)|ImGuiTableFlags_ScrollY)) {
      if (settings.unifiedDataView) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::Selectable(ICON_FA_TASKS "##Instruments",lastAssetType==0)) {
          lastAssetType=0;
        }
        ImGui::SameLine();
        ImGui::Text(_("Instruments"));
        ImGui::Indent();
      }

      if (settings.horizontalDataView && !insListDir) {
        ImGui::TableNextRow();
      }

      if (insListDir) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        insListItem(-1,-1,-1);
        int dirIndex=0;
        int dirToDelete=-1;
        for (DivAssetDir& i: e->song.insDir) {
          String nodeName=fmt::sprintf("%s %s##_ADI%d",i.name.empty()?ICON_FA_FOLDER_O:ICON_FA_FOLDER,i.name.empty()?_("<uncategorized>"):i.name,i.name.empty()?-1:dirIndex);
          String popupID=fmt::sprintf("DirRightMenu%d",dirIndex);
          bool treeNode=ImGui::TreeNodeEx(nodeName.c_str(),ImGuiTreeNodeFlags_SpanAvailWidth|(i.name.empty()?ImGuiTreeNodeFlags_DefaultOpen:0));
          DIR_DRAG_SOURCE(dirIndex,-1,"FUR_INSDIR");
          DIR_DRAG_TARGET(dirIndex,-1,e->song.insDir,"FUR_INSDIR");
          if (ImGui::BeginPopupContextItem(popupID.c_str())) {
            if (ImGui::MenuItem(_("rename..."))) {
              editStr(&i.name);
            }
            if (ImGui::MenuItem(_("delete"))) {
              dirToDelete=dirIndex;
            }
            ImGui::EndPopup();
          }
          if (treeNode) {
            int assetIndex=0;
            for (int j: i.entries) {
              insListItem(j,dirIndex,assetIndex);
              assetIndex++;
            }
            ImGui::TreePop();
          }
          dirIndex++;
        }
        if (dirToDelete!=-1) {
          e->lockEngine([this,dirToDelete]() {
            e->song.insDir.erase(e->song.insDir.begin()+dirToDelete);
            e->checkAssetDir(e->song.insDir,e->song.ins.size());
          });
        }
      } else {
        int curRow=0;
        for (int i=-1; i<(int)e->song.ins.size(); i++) {
          if (!settings.horizontalDataView) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
          } else if (curRow==0) {
            ImGui::TableNextColumn();
          }
          insListItem(i,-1,-1);
          if (settings.horizontalDataView) {
            if (++curRow>=availableRows) curRow=0;
          }
        }
      }

      if (settings.unifiedDataView) {
        ImGui::Unindent();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::Selectable(ICON_FA_AREA_CHART "##Wavetables",lastAssetType==1)) {
          lastAssetType=1;
        }
        ImGui::SameLine();
        ImGui::Text(_("Wavetables"));
        ImGui::Indent();
        actualWaveList();
        ImGui::Unindent();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::Selectable(ICON_FA_VOLUME_UP "##Samples",lastAssetType==2)) {
          lastAssetType=2;
        }
        ImGui::SameLine();
        ImGui::Text(_("Samples"));
        ImGui::Indent();
        actualSampleList();
        ImGui::Unindent();
      }

      ImGui::EndTable();
    }
  }
  if (asChild) {
    ImGui::EndChild();
  } else {
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_INS_LIST;
    ImGui::End();
  }
}

void FurnaceGUI::drawWaveList(bool asChild) {
  if (nextWindow==GUI_WINDOW_WAVE_LIST) {
    waveListOpen=true;
    if (settings.unifiedDataView) {
      ImGui::SetWindowFocus("Instruments");
    } else {
      ImGui::SetNextWindowFocus();
    }
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (settings.unifiedDataView) return;
  if (!waveListOpen && !asChild) return;
  bool began=false;
  if (asChild) {
    began=ImGui::BeginChild("Wavetables");
  } else {
    began=ImGui::Begin("Wavetables",&waveListOpen,globalWinFlags,_("Wavetables"));
  }
  if (began) {
    // hide buttons if there isn't enough space
    // buttons and their space requirements:
    // - new: 2
    // - duplicate: 6
    // - open: 3
    // - save: 5
    // - folder view: 7
    // - move up: 8
    // - move down: 8
    // - delete: 4
    float buttonSize=ImGui::GetStyle().FramePadding.x*2.0f+settings.iconSize*dpiScale+ImGui::GetStyle().ItemSpacing.x;
    float buttonSpace=ImGui::GetContentRegionAvail().x/MAX(1.0f,buttonSize);
    bool mustOpenNewFolder=false;

    if (buttonSpace>=2.0f) {
      // add
      if (ImGui::Button(ICON_FA_PLUS "##WaveAdd")) {
        doAction(GUI_ACTION_WAVE_LIST_ADD);
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(_("Add"));
      }
      ImGui::SameLine();
    }

    if (buttonSpace>=6.0f) {
      // duplicate
      if (ImGui::Button(ICON_FA_FILES_O "##WaveClone")) {
        doAction(GUI_ACTION_WAVE_LIST_DUPLICATE);
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(_("Duplicate"));
      }
      ImGui::SameLine();
    }

    if (buttonSpace>=3.0f) {
      // open
      if (ImGui::Button(ICON_FA_FOLDER_OPEN "##WaveLoad")) {
        doAction(GUI_ACTION_WAVE_LIST_OPEN);
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(_("Open"));
      }
      if (ImGui::BeginPopupContextItem("WaveOpenOpt")) {
        if (ImGui::MenuItem(_("replace..."))) {
          doAction((curWave>=0 && curWave<(int)e->song.wave.size())?GUI_ACTION_WAVE_LIST_OPEN_REPLACE:GUI_ACTION_WAVE_LIST_OPEN);
        }
        ImGui::EndPopup();
      }
      ImGui::SameLine();
    }

    if (buttonSpace>=5.0f) {
      // save
      if (ImGui::Button(ICON_FA_FLOPPY_O "##WaveSave")) {
        doAction(GUI_ACTION_WAVE_LIST_SAVE);
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(_("Save"));
      }
      if (!settings.unifiedDataView) {
        if (ImGui::BeginPopupContextItem("WaveSaveFormats",ImGuiMouseButton_Right)) {
          if (ImGui::MenuItem(_("save as .dmw..."))) {
            doAction(GUI_ACTION_WAVE_LIST_SAVE_DMW);
          }
          if (ImGui::MenuItem(_("save raw..."))) {
            doAction(GUI_ACTION_WAVE_LIST_SAVE_RAW);
          }
          if (ImGui::MenuItem(_("save all..."))) {
            doAction(GUI_ACTION_WAVE_LIST_SAVE_ALL);
          }
          ImGui::EndPopup();
        }
      }
      ImGui::SameLine();
    }

    if (buttonSpace>=7.0f) {
      // dir mode
      pushToggleColors(waveListDir);
      if (ImGui::Button(ICON_FA_SITEMAP "##WaveDirMode")) {
        doAction(GUI_ACTION_WAVE_LIST_DIR_VIEW);
      }
      popToggleColors();
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(_("Toggle folders/standard view"));
      }
      ImGui::SameLine();
    }

    if (buttonSpace>=(waveListDir?7.0f:8.0f)) {
      // move up/down
      if (!waveListDir) {
        if (ImGui::Button(ICON_FA_ARROW_UP "##WaveUp")) {
          doAction(GUI_ACTION_WAVE_LIST_MOVE_UP);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("Move up"));
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_ARROW_DOWN "##WaveDown")) {
          doAction(GUI_ACTION_WAVE_LIST_MOVE_DOWN);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("Move down"));
        }
      } else {
        if (ImGui::Button(ICON_FA_FOLDER "##WaveFolder")) {
          mustOpenNewFolder=true;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("New folder"));
        }
      }
      ImGui::SameLine();
    }

    if (buttonSpace>=4.0f) {
      // delete
      pushDestColor();
      if (ImGui::Button(ICON_FA_TIMES "##WaveDelete")) {
        doAction(GUI_ACTION_WAVE_LIST_DELETE);
      }
      popDestColor();
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(_("Delete"));
      }
      if (buttonSpace<8.0f) {
        ImGui::SameLine();
      }
    }

    if (buttonSpace<(waveListDir?7.0f:8.0f)) {
      if (ImGui::Button(ICON_FA_ELLIPSIS_H "##WaveMore")) {
      }
      if (ImGui::BeginPopupContextItem("WaveListMore",ImGuiMouseButton_Left)) {
        if (buttonSpace<2.0f) if (ImGui::MenuItem("add")) {
          doAction(GUI_ACTION_WAVE_LIST_ADD);
        }
        if (buttonSpace<6.0f) if (ImGui::MenuItem("duplicate")) {
          doAction(GUI_ACTION_WAVE_LIST_DUPLICATE);
        }
        if (buttonSpace<3.0f) {
          if (ImGui::MenuItem("open...")) {
            doAction(GUI_ACTION_WAVE_LIST_OPEN);
          }
          if (ImGui::MenuItem("replace...")) {
            doAction((curWave>=0 && curWave<(int)e->song.wave.size())?GUI_ACTION_WAVE_LIST_OPEN_REPLACE:GUI_ACTION_WAVE_LIST_OPEN);
          }
        }
        if (buttonSpace<5.0f) {
          if (ImGui::MenuItem("save...")) {
            doAction(GUI_ACTION_WAVE_LIST_SAVE);
          }
          if (ImGui::MenuItem(_("save as .dmw..."))) {
            doAction(GUI_ACTION_WAVE_LIST_SAVE_DMW);
          }
          if (ImGui::MenuItem(_("save raw..."))) {
            doAction(GUI_ACTION_WAVE_LIST_SAVE_RAW);
          }
          if (ImGui::MenuItem(_("save all..."))) {
            doAction(GUI_ACTION_WAVE_LIST_SAVE_ALL);
          }
        }
        if (buttonSpace<7.0f) if (ImGui::MenuItem("folder view",NULL,waveListDir)) {
          doAction(GUI_ACTION_WAVE_LIST_DIR_VIEW);
        }
        if (buttonSpace<(waveListDir?7.0f:8.0f)) {
          if (!waveListDir) {
            if (ImGui::MenuItem("move up")) {
              doAction(GUI_ACTION_WAVE_LIST_MOVE_UP);
            }
            if (ImGui::MenuItem("move down")) {
              doAction(GUI_ACTION_WAVE_LIST_MOVE_DOWN);
            }
          } else {
            if (ImGui::MenuItem("new folder")) {
              mustOpenNewFolder=true;
            }
          }
        }
        if (buttonSpace<4.0f) if (ImGui::MenuItem("delete")) {
          doAction(GUI_ACTION_WAVE_LIST_DELETE);
        }
        ImGui::EndPopup();
      }
    }

    if (mustOpenNewFolder) {
      folderString="";
      ImGui::OpenPopup("NewWaveFolder");
    }

    if (ImGui::BeginPopup("NewWaveFolder",ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoSavedSettings)) {
      ImGui::InputText("##FolderName",&folderString);
      ImGui::SameLine();
      ImGui::BeginDisabled(folderString.empty());
      if (ImGui::Button(_("Create"))) {
        e->lockEngine([this]() {
          e->song.waveDir.push_back(DivAssetDir(folderString));
        });
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndDisabled();
      ImGui::EndPopup();
    }

    ImGui::Separator();
    if (ImGui::BeginTable("WaveListScroll",1,ImGuiTableFlags_ScrollY)) {
      actualWaveList();
      ImGui::EndTable();
    }
  }
  if (asChild) {
    ImGui::EndChild();
  } else {
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_WAVE_LIST;
    ImGui::End();
  }
}

void FurnaceGUI::drawSampleList(bool asChild) {
  if (nextWindow==GUI_WINDOW_SAMPLE_LIST) {
    sampleListOpen=true;
    if (settings.unifiedDataView) {
      ImGui::SetWindowFocus("Instruments");
    } else {
      ImGui::SetNextWindowFocus();
    }
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (settings.unifiedDataView) return;
  if (!sampleListOpen && !asChild) return;
  bool began=false;
  if (asChild) {
    began=ImGui::BeginChild("Samples");
  } else {
    began=ImGui::Begin("Samples",&sampleListOpen,globalWinFlags,_("Samples"));
  }
  if (began) {
    // hide buttons if there isn't enough space
    // buttons and their space requirements:
    // - new: 2
    // - duplicate: 7
    // - open: 3
    // - save: 5
    // - folder view: 8
    // - move up: 9
    // - move down: 9
    // - preview: 6
    // - delete: 4
    float buttonSize=ImGui::GetStyle().FramePadding.x*2.0f+settings.iconSize*dpiScale+ImGui::GetStyle().ItemSpacing.x;
    float buttonSpace=ImGui::GetContentRegionAvail().x/MAX(1.0f,buttonSize);
    bool mustOpenNewFolder=false;

    if (buttonSpace>=2.0f) {
      // add
      if (ImGui::Button(ICON_FA_FILE "##SampleAdd")) {
        doAction(GUI_ACTION_SAMPLE_LIST_ADD);
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(_("Add"));
      }
      ImGui::SameLine();
    }

    if (buttonSpace>=7.0f) {
      // duplicate
      if (ImGui::Button(ICON_FA_FILES_O "##SampleClone")) {
        doAction(GUI_ACTION_SAMPLE_LIST_DUPLICATE);
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(_("Duplicate"));
      }
      ImGui::SameLine();
    }

    if (buttonSpace>=3.0f) {
      // open
      if (ImGui::Button(ICON_FA_FOLDER_OPEN "##SampleLoad")) {
        doAction(GUI_ACTION_SAMPLE_LIST_OPEN);
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(_("Open"));
      }
      if (mobileUI && ImGui::IsItemActive() && CHECK_LONG_HOLD) {
        ImGui::OpenPopup("SampleOpenOpt");
        NOTIFY_LONG_HOLD;
      }
      if (ImGui::BeginPopupContextItem("SampleOpenOpt")) {
        if (ImGui::MenuItem(_("replace..."))) {
          doAction((curSample>=0 && curSample<(int)e->song.sample.size())?GUI_ACTION_SAMPLE_LIST_OPEN_REPLACE:GUI_ACTION_SAMPLE_LIST_OPEN);
        }
        ImGui::Separator();
        if (ImGui::MenuItem(_("import raw..."))) {
          doAction(GUI_ACTION_SAMPLE_LIST_OPEN_RAW);
        }
        if (ImGui::MenuItem(_("import raw (replace)..."))) {
          doAction((curSample>=0 && curSample<(int)e->song.sample.size())?GUI_ACTION_SAMPLE_LIST_OPEN_REPLACE_RAW:GUI_ACTION_SAMPLE_LIST_OPEN_RAW);
        }
        ImGui::EndPopup();
      }
      ImGui::SameLine();
    }

    if (buttonSpace>=5.0f) {
      // save
      if (ImGui::Button(ICON_FA_FLOPPY_O "##SampleSave")) {
        doAction(GUI_ACTION_SAMPLE_LIST_SAVE);
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(_("Save"));
      }
      if (mobileUI && ImGui::IsItemActive() && CHECK_LONG_HOLD) {
        ImGui::OpenPopup("SampleSaveOpt");
        NOTIFY_LONG_HOLD;
      }
      if (ImGui::BeginPopupContextItem("SampleSaveOpt")) {
        if (ImGui::MenuItem(_("save raw..."))) {
          doAction(GUI_ACTION_SAMPLE_LIST_SAVE_RAW);
        }
        if (ImGui::MenuItem(_("save all..."))) {
          doAction(GUI_ACTION_SAMPLE_LIST_SAVE_ALL);
        }
        ImGui::EndPopup();
      }
      ImGui::SameLine();
    }

    if (buttonSpace>=8.0f) {
      // dir mode
      pushToggleColors(sampleListDir);
      if (ImGui::Button(ICON_FA_SITEMAP "##SampleDirMode")) {
        doAction(GUI_ACTION_SAMPLE_LIST_DIR_VIEW);
      }
      popToggleColors();
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(_("Toggle folders/standard view"));
      }
      ImGui::SameLine();
    }

    if (buttonSpace>=(sampleListDir?8.0f:9.0f)) {
      if (!sampleListDir) {
        if (ImGui::Button(ICON_FA_ARROW_UP "##SampleUp")) {
          doAction(GUI_ACTION_SAMPLE_LIST_MOVE_UP);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("Move up"));
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_ARROW_DOWN "##SampleDown")) {
          doAction(GUI_ACTION_SAMPLE_LIST_MOVE_DOWN);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("Move down"));
        }
      } else {
        if (ImGui::Button(ICON_FA_FOLDER "##SampleFolder")) {
          mustOpenNewFolder=true;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("New folder"));
        }
      }
      ImGui::SameLine();
    }

    if (buttonSpace>=6.0f) {
      if (ImGui::Button(ICON_FA_VOLUME_UP "##PreviewSampleL")) {
        doAction(GUI_ACTION_SAMPLE_LIST_PREVIEW);
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(_("Preview (right click to stop)"));
      }
      if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        doAction(GUI_ACTION_SAMPLE_LIST_STOP_PREVIEW);
      }
      ImGui::SameLine();
    }

    if (buttonSpace>=4.0f) {
      pushDestColor();
      if (ImGui::Button(ICON_FA_TIMES "##SampleDelete")) {
        doAction(GUI_ACTION_SAMPLE_LIST_DELETE);
      }
      popDestColor();
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(_("Delete"));
      }
      if (buttonSpace<9.0f) {
        ImGui::SameLine();
      }
    }

    if (buttonSpace<(sampleListDir?8.0f:9.0f)) {
      if (ImGui::Button(ICON_FA_ELLIPSIS_H "##SampleMore")) {
      }
      if (ImGui::BeginPopupContextItem("SampleListMore",ImGuiMouseButton_Left)) {
        if (buttonSpace<2.0f) if (ImGui::MenuItem("add")) {
          doAction(GUI_ACTION_SAMPLE_LIST_ADD);
        }
        if (buttonSpace<7.0f) if (ImGui::MenuItem("duplicate")) {
          doAction(GUI_ACTION_SAMPLE_LIST_DUPLICATE);
        }
        if (buttonSpace<3.0f) {
          if (ImGui::MenuItem("open...")) {
            doAction(GUI_ACTION_SAMPLE_LIST_OPEN);
          }
          if (ImGui::MenuItem("replace...")) {
            doAction((curSample>=0 && curSample<(int)e->song.sample.size())?GUI_ACTION_SAMPLE_LIST_OPEN_REPLACE:GUI_ACTION_SAMPLE_LIST_OPEN);
          }
          if (ImGui::MenuItem("import raw...")) {
            doAction(GUI_ACTION_SAMPLE_LIST_OPEN_RAW);
          }
          if (ImGui::MenuItem("import raw (replace)...")) {
            doAction((curSample>=0 && curSample<(int)e->song.sample.size())?GUI_ACTION_SAMPLE_LIST_OPEN_REPLACE_RAW:GUI_ACTION_SAMPLE_LIST_OPEN_RAW);
          }
        }
        if (buttonSpace<5.0f) {
          if (ImGui::MenuItem("save...")) {
            doAction(GUI_ACTION_SAMPLE_LIST_SAVE);
          }
          if (ImGui::MenuItem("save raw...")) {
            doAction(GUI_ACTION_SAMPLE_LIST_SAVE_RAW);
          }
          if (ImGui::MenuItem("save all...")) {
            doAction(GUI_ACTION_SAMPLE_LIST_SAVE_ALL);
          }
        }
        if (buttonSpace<8.0f) if (ImGui::MenuItem("folder view",NULL,sampleListDir)) {
          doAction(GUI_ACTION_SAMPLE_LIST_DIR_VIEW);
        }
        if (buttonSpace<(sampleListDir?8.0f:9.0f)) {
          if (!sampleListDir) {
            if (ImGui::MenuItem("move up")) {
              doAction(GUI_ACTION_SAMPLE_LIST_MOVE_UP);
            }
            if (ImGui::MenuItem("move down")) {
              doAction(GUI_ACTION_SAMPLE_LIST_MOVE_DOWN);
            }
          } else {
            if (ImGui::MenuItem("new folder")) {
              mustOpenNewFolder=true;
            }
          }
        }
        if (buttonSpace<6.0f) {
          if (ImGui::MenuItem("preview")) {
            doAction(GUI_ACTION_SAMPLE_LIST_PREVIEW);
          }
          if (ImGui::MenuItem("stop preview")) {
            doAction(GUI_ACTION_SAMPLE_LIST_STOP_PREVIEW);
          }
        }
        if (buttonSpace<4.0f) if (ImGui::MenuItem("delete")) {
          doAction(GUI_ACTION_SAMPLE_LIST_DELETE);
        }

        ImGui::EndPopup();
      }
    }

    if (mustOpenNewFolder) {
      folderString="";
      ImGui::OpenPopup("NewSampleFolder");
    }

    if (ImGui::BeginPopup("NewSampleFolder",ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoSavedSettings)) {
      ImGui::InputText("##FolderName",&folderString);
      ImGui::SameLine();
      ImGui::BeginDisabled(folderString.empty());
      if (ImGui::Button(_("Create"))) {
        e->lockEngine([this]() {
          e->song.sampleDir.push_back(DivAssetDir(folderString));
        });
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndDisabled();
      ImGui::EndPopup();
    }

    ImGui::Separator();
    if (ImGui::BeginTable("SampleListScroll",1,ImGuiTableFlags_ScrollY)) {
      actualSampleList();
      ImGui::EndTable();
    }
    ImGui::Unindent();
  }
  if (asChild) {
    ImGui::EndChild();
  } else {
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_SAMPLE_LIST;
    ImGui::End();
  }
}

// HACK: template. any way to remove it?
template<typename func_waveItemData> void FurnaceGUI::waveListHorizontalGroup(float* wavePreview, int dir, int count, const func_waveItemData& waveItemData) {
  if (count==0) return;

  float idealWidthMin=225.0f*dpiScale;
  float idealWidthMax=350.0f*dpiScale;
  float availX=ImGui::GetContentRegionAvail().x;
  int columnCount=CLAMP((int)(availX/idealWidthMin),1,count);
  int rowCount=(int)ceilf(count/(float)columnCount);
  columnCount=(int)ceilf(count/(float)rowCount);
  float columnWidth=MIN(CLAMP(availX/columnCount,idealWidthMin,idealWidthMax),availX);
  if (ImGui::BeginTable("##waveListGroupTable",columnCount,ImGuiTableFlags_SizingFixedSame)) {
    for (int col=0; col<columnCount; col++) {
      ImGui::TableSetupColumn("##column",ImGuiTableColumnFlags_WidthFixed,columnWidth);
    }
    for (int row=0; row<rowCount; row++) {
      ImGui::TableNextRow();
      for (int col=0; col<columnCount; col++) {
        ImGui::TableNextColumn();
        int idx=row+col*rowCount;
        if (idx>=count) continue;

        int waveIdx, asset;
        waveItemData(row+col*rowCount,&waveIdx,&asset);
        waveListItem(waveIdx,wavePreview,dir,asset);
      }
    }
    ImGui::EndTable();
  }
}

void FurnaceGUI::actualWaveList() {
  float wavePreview[257];

  if (waveListDir || (settings.unifiedDataView && insListDir)) {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    int dirIndex=0;
    int dirToDelete=-1;
    for (DivAssetDir& i: e->song.waveDir) {
      String nodeName=fmt::sprintf("%s %s##_ADW%d",i.name.empty()?ICON_FA_FOLDER_O:ICON_FA_FOLDER,i.name.empty()?_("<uncategorized>"):i.name,i.name.empty()?-1:dirIndex);
      String popupID=fmt::sprintf("DirRightMenu%d",dirIndex);
      bool treeNode=ImGui::TreeNodeEx(nodeName.c_str(),ImGuiTreeNodeFlags_SpanAvailWidth|(i.name.empty()?ImGuiTreeNodeFlags_DefaultOpen:0));
      DIR_DRAG_SOURCE(dirIndex,-1,"FUR_WAVEDIR");
      DIR_DRAG_TARGET(dirIndex,-1,e->song.waveDir,"FUR_WAVEDIR");
      if (ImGui::BeginPopupContextItem(popupID.c_str())) {
        if (ImGui::MenuItem(_("rename..."))) {
          editStr(&i.name);
        }
        if (ImGui::MenuItem(_("delete"))) {
          dirToDelete=dirIndex;
        }
        ImGui::EndPopup();
      }
      if (treeNode) {
        if (settings.horizontalDataView) {
          waveListHorizontalGroup(wavePreview,dirIndex,i.entries.size(),[&](int i_, int* waveIdx, int* asset) {
            *waveIdx=i.entries[i_];
            *asset=i_;
          });
        } else {
          int assetIndex=0;
          for (int j: i.entries) {
            waveListItem(j,wavePreview,dirIndex,assetIndex);
            assetIndex++;
          }
        }
        ImGui::TreePop();
      }
      dirIndex++;
    }
    if (dirToDelete!=-1) {
      e->lockEngine([this,dirToDelete]() {
        e->song.waveDir.erase(e->song.waveDir.begin()+dirToDelete);
        e->checkAssetDir(e->song.waveDir,e->song.wave.size());
      });
    }
  } else {
    if (settings.horizontalDataView) {
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      waveListHorizontalGroup(wavePreview,-1,(int)e->song.wave.size(),[&](int i, int* waveIdx, int* asset) {
        *waveIdx=i;
        *asset=-1;
      });
    } else {
      for (int i=0; i<(int)e->song.wave.size(); i++) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        waveListItem(i,wavePreview,-1,-1);
      }
    }
  }
}

void FurnaceGUI::actualSampleList() {
  if (sampleListDir || (settings.unifiedDataView && insListDir)) {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    int dirIndex=0;
    int dirToDelete=-1;
    for (DivAssetDir& i: e->song.sampleDir) {
      String nodeName=fmt::sprintf("%s %s##_ADS%d",i.name.empty()?ICON_FA_FOLDER_O:ICON_FA_FOLDER,i.name.empty()?_("<uncategorized>"):i.name,i.name.empty()?-1:dirIndex);
      String popupID=fmt::sprintf("DirRightMenu%d",dirIndex);
      bool treeNode=ImGui::TreeNodeEx(nodeName.c_str(),ImGuiTreeNodeFlags_SpanAvailWidth|(i.name.empty()?ImGuiTreeNodeFlags_DefaultOpen:0));
      DIR_DRAG_SOURCE(dirIndex,-1,"FUR_SDIR");
      DIR_DRAG_TARGET(dirIndex,-1,e->song.sampleDir,"FUR_SDIR");
      if (ImGui::BeginPopupContextItem(popupID.c_str())) {
        if (ImGui::MenuItem(_("rename..."))) {
          editStr(&i.name);
        }
        if (ImGui::MenuItem(_("delete"))) {
          dirToDelete=dirIndex;
        }
        ImGui::EndPopup();
      }
      if (treeNode) {
        int assetIndex=0;
        for (int j: i.entries) {
          sampleListItem(j,dirIndex,assetIndex);
          assetIndex++;
        }
        ImGui::TreePop();
      }
      dirIndex++;
    }
    if (dirToDelete!=-1) {
      e->lockEngine([this,dirToDelete]() {
        e->song.sampleDir.erase(e->song.sampleDir.begin()+dirToDelete);
        e->checkAssetDir(e->song.sampleDir,e->song.sample.size());
      });
    }
  } else {
    for (int i=0; i<(int)e->song.sample.size(); i++) {
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      sampleListItem(i,-1,-1);
    }
  }
}
