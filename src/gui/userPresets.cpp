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
#include "../baseutils.h"
#include "../fileutils.h"
#include <fmt/printf.h>
#include "IconsFontAwesome4.h"
#include <imgui.h>
#include "misc/cpp/imgui_stdlib.h"

#ifdef _WIN32
#define PRESETS_FILE "\\presets.cfg"
#else
#define PRESETS_FILE "/presets.cfg"
#endif

#define REDUNDANCY_NUM_ATTEMPTS 5
#define CHECK_BUF_SIZE 8192

std::vector<FurnaceGUISysDef>* digDeep(std::vector<FurnaceGUISysDef>& entries, int depth) {
  if (depth==0) return &entries;
  std::vector<FurnaceGUISysDef>& result=entries;

  for (int i=0; i<depth; i++) {
    if (result.empty()) {
      logW("digDeep: %d is as far as it goes!",depth);
      break;
    }
    result=result.at(result.size()).subDefs;
  }
  return &result;
}

bool FurnaceGUI::loadUserPresets(bool redundancy, String path, bool append) {
  if (path.empty()) path=e->getConfigPath()+PRESETS_FILE;
  String line, lineStr;
  logD("opening user presets: %s",path);

  FILE* f=NULL;

  if (redundancy) {
    unsigned char* readBuf=new unsigned char[CHECK_BUF_SIZE];
    size_t readBufLen=0;
    for (int i=0; i<REDUNDANCY_NUM_ATTEMPTS; i++) {
      bool viable=false;
      if (i>0) {
        line=fmt::sprintf("%s.%d",path,i);
      } else {
        line=path;
      }
      logV("trying: %s",line);

      // try to open config
      f=ps_fopen(line.c_str(),"rb");
      // check whether we could open it
      if (f==NULL) {
        logV("fopen(): %s",strerror(errno));
        continue;
      }

      // check whether there's something
      while (!feof(f)) {
        readBufLen=fread(readBuf,1,CHECK_BUF_SIZE,f);
        if (ferror(f)) {
          logV("fread(): %s",strerror(errno));
          break;
        }

        for (size_t j=0; j<readBufLen; j++) {
          if (readBuf[j]==0) {
            viable=false;
            logW("a zero?");
            break;
          }
          if (readBuf[j]!='\r' && readBuf[j]!='\n' && readBuf[j]!=' ') {
            viable=true;
          }
        }

        if (viable) break;
      }

      // there's something
      if (viable) {
        if (fseek(f,0,SEEK_SET)==-1) {
          logV("fseek(): %s",strerror(errno));
          viable=false;
        } else {
          break;
        }
      }
      
      // close it (because there's nothing)
      fclose(f);
      f=NULL;
    }
    delete[] readBuf;

    // we couldn't read at all
    if (f==NULL) {
      logD("presets file does not exist");
      return false;
    }
  } else {
    f=ps_fopen(path.c_str(),"rb");
    if (f==NULL) {
      logD("presets file does not exist");
      return false;
    }
  }

  // now read stuff
  FurnaceGUISysCategory* userCategory=NULL;

  for (FurnaceGUISysCategory& i: sysCategories) {
    if (strcmp(i.name,_("User"))==0) {
      userCategory=&i;
      break;
    }
  }

  if (userCategory==NULL) {
    logE("could not find user category!");
    fclose(f);
    return false;
  }

  if (!append) userCategory->systems.clear();

  char nextLine[4096];
  lineStr="";
  while (!feof(f)) {
    if (fgets(nextLine,4095,f)==NULL) {
      break;
    }
    lineStr+=nextLine;
    if (!lineStr.empty() && !feof(f)) {
      if (lineStr[lineStr.size()-1]!='\n') {
        continue;
      }
    }

    int indent=0;
    bool readIndent=true;
    bool keyOrValue=false;
    String key="";
    String value="";
    for (char i: lineStr) {
      if (i=='\n') break;
      if (readIndent) {
        if (i==' ') {
          indent++;
        } else {
          readIndent=false;
        }
      }
      if (!readIndent) {
        if (keyOrValue) {
          value+=i;
        } else {
          if (i=='=') {
            keyOrValue=true;
          } else {
            key+=i;
          }
        }
      }
    }
    indent>>=1;

    if (!key.empty()) {
      std::vector<FurnaceGUISysDef>* where=digDeep(userCategory->systems,indent);
      where->push_back(FurnaceGUISysDef(key.c_str(),value.c_str(),e));
    }

    lineStr="";
    lineStr.reserve(4096);
  }

  fclose(f);
  return true;
}

void writeSubEntries(FILE* f, std::vector<FurnaceGUISysDef>& entries, int depth) {
  for (FurnaceGUISysDef& i: entries) {
    String safeName;
    safeName.reserve(i.name.size());
    bool beginning=false;
    for (unsigned char j: i.name) {
      if (beginning && j==' ') continue;
      if (j=='=') continue;
      if (j<0x20) continue;
      safeName+=j;
    }
    
    String data;
    for (int i=0; i<depth; i++) {
      data+="  ";
    }
    data+=fmt::sprintf("%s=%s\n",safeName,taEncodeBase64(i.definition));
    fputs(data.c_str(),f);

    writeSubEntries(f,i.subDefs,depth+1);
  }
}

bool FurnaceGUI::saveUserPresets(bool redundancy, String path) {
  if (path.empty()) path=e->getConfigPath()+PRESETS_FILE;
  FurnaceGUISysCategory* userCategory=NULL;

  for (FurnaceGUISysCategory& i: sysCategories) {
    if (strcmp(i.name,_("User"))==0) {
      userCategory=&i;
      break;
    }
  }

  if (userCategory==NULL) {
    logE("could not find user category!");
    return false;
  }

  if (redundancy) {
    char oldPath[4096];
    char newPath[4096];

    if (fileExists(path.c_str())==1) {
      logD("rotating preset files...");
      for (int i=4; i>=0; i--) {
        if (i>0) {
          snprintf(oldPath,4095,"%s.%d",path.c_str(),i);
        } else {
          strncpy(oldPath,path.c_str(),4095);
        }
        snprintf(newPath,4095,"%s.%d",path.c_str(),i+1);

        if (i>=4) {
          logV("remove %s",oldPath);
          deleteFile(oldPath);
        } else {
          logV("move %s to %s",oldPath,newPath);
          moveFiles(oldPath,newPath);
        }
      }
    }
  }
  logD("saving user presets: %s",path);
  FILE* f=ps_fopen(path.c_str(),"wb");
  if (f==NULL) {
    logW("could not write presets! %s",strerror(errno));
    return false;
  }

  writeSubEntries(f,userCategory->systems,0);

  fclose(f);
  logD("presets written successfully.");
  return true;
}

// user presets management
void FurnaceGUI::printPresets(std::vector<FurnaceGUISysDef>& items, size_t depth, std::vector<int>& depthStack) {
  if (depth>0) ImGui::Indent();
  int index=0;
  for (FurnaceGUISysDef& i: items) {
    bool isSelected=(selectedUserPreset.size()==(depth+1));
    if (isSelected) {
      for (size_t j=0; j<=depth; j++) {
        int item=-1;
        if (j>=depthStack.size()) {
          item=index;
        } else {
          item=depthStack[j];
        }

        if (selectedUserPreset[j]!=item) {
          isSelected=false;
          break;
        }
      }
    }
    ImGui::PushID(index+1);
    if (ImGui::Selectable(i.name.c_str(),isSelected)) {
      selectedUserPreset=depthStack;
      selectedUserPreset.push_back(index);
    }
    ImGui::PopID();
    if (!i.subDefs.empty()) {
      depthStack.push_back(index);
      ImGui::PushID(index);
      printPresets(i.subDefs,depth+1,depthStack);
      ImGui::PopID();
      depthStack.pop_back();
    }
    index++;
  }
  if (depth>0) ImGui::Unindent();
}

FurnaceGUISysDef* FurnaceGUI::selectPreset(std::vector<FurnaceGUISysDef>& items) {
  FurnaceGUISysDef* ret=NULL;
  for (size_t i=0; i<selectedUserPreset.size(); i++) {
    if (selectedUserPreset[i]<0 || selectedUserPreset[i]>=(int)items.size()) return NULL;
    ret=&items[selectedUserPreset[i]];
    if (i<selectedUserPreset.size()-1) {
      items=ret->subDefs;
    }
  }
  return ret;
}

void FurnaceGUI::drawUserPresets() {
  if (nextWindow==GUI_WINDOW_USER_PRESETS) {
    userPresetsOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!userPresetsOpen) return;
  if (ImGui::Begin("User Systems",&userPresetsOpen,globalWinFlags,_("User Systems"))) {
    FurnaceGUISysCategory* userCategory=NULL;
    for (FurnaceGUISysCategory& i: sysCategories) {
      if (strcmp(i.name,_("User"))==0) {
        userCategory=&i;
        break;
      }
    }

    std::vector<int> depthStack;

    if (userCategory==NULL) {
      ImGui::Text(_("Error! User category does not exist!"));
    } else if (ImGui::BeginTable("UserPresets",2,ImGuiTableFlags_BordersInnerV,ImVec2(ImGui::GetContentRegionAvail().x,ImGui::GetContentRegionAvail().y-ImGui::GetFrameHeightWithSpacing()))) {
      ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.25f);
      ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.75f);
      // preset list
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      if (ImGui::BeginChild("UList",ImVec2(ImGui::GetContentRegionAvail().x,ImGui::GetContentRegionAvail().y-ImGui::GetFrameHeightWithSpacing()))) {
        ImGui::AlignTextToFramePadding();
        ImGui::Text(_("Systems"));
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_PLUS "##AddPreset")) {
          userCategory->systems.push_back(FurnaceGUISysDef(_("New Preset"),{}));
          selectedUserPreset.clear();
          selectedUserPreset.push_back(userCategory->systems.size()-1);
        }
        printPresets(userCategory->systems,0,depthStack);
      }
      ImGui::EndChild();

      // editor
      ImGui::TableNextColumn();
      if (ImGui::BeginChild("UEdit",ImVec2(ImGui::GetContentRegionAvail().x,ImGui::GetContentRegionAvail().y-ImGui::GetFrameHeightWithSpacing()))) {
        if (selectedUserPreset.empty()) {
          ImGui::Text(_("select a preset"));
        } else {
          FurnaceGUISysDef* preset=selectPreset(userCategory->systems);
          bool doRemovePreset=false;

          if (preset!=NULL) {
            ImGui::AlignTextToFramePadding();
            ImGui::Text(_("Name"));
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x-ImGui::CalcTextSize(_("Remove")).x-ImGui::GetStyle().ItemSpacing.x*2.0-ImGui::GetStyle().ItemInnerSpacing.x*2.0);
            ImGui::InputText("##PName",&preset->name);
            ImGui::SameLine();
            pushDestColor();
            if (ImGui::Button(_("Remove##UPresetRemove"))) {
              doRemovePreset=true;
            }
            popDestColor();

            ImGui::Separator();

            int doRemove=-1;
            bool mustBake=false;

            for (size_t i=0; i<preset->orig.size(); i++) {
              String tempID;
              FurnaceGUISysDefChip& chip=preset->orig[i];

              bool doInvert=(chip.vol<0);
              float vol=fabs(chip.vol);
              ImGui::PushID(i);

              tempID=fmt::sprintf("%s##USystem",getSystemName(chip.sys));
              ImGui::Button(tempID.c_str(),ImVec2(ImGui::GetContentRegionAvail().x-ImGui::CalcTextSize(_("Invert")).x-ImGui::GetFrameHeightWithSpacing()*2.0-ImGui::GetStyle().ItemSpacing.x*2.0,0));
              if (ImGui::BeginPopupContextItem("SysPickerCU",ImGuiPopupFlags_MouseButtonLeft)) {
                DivSystem picked=systemPicker(false);
                if (picked!=DIV_SYSTEM_NULL) {
                  chip.sys=picked;
                  mustBake=true;
                  ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
              }

              ImGui::SameLine();
              if (ImGui::Checkbox(_("Invert"),&doInvert)) {
                chip.vol=-chip.vol;
                mustBake=true;
              }
              ImGui::SameLine();
              pushDestColor();
              if (ImGui::Button(ICON_FA_MINUS "##USysRemove")) {
                doRemove=i;
                mustBake=true;
              }
              popDestColor();
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x-ImGui::GetFrameHeightWithSpacing()*2.0-ImGui::GetStyle().ItemSpacing.x*2.0);
              if (CWSliderFloat(_("Volume"),&vol,0.0f,3.0f)) {
                if (doInvert) {
                  if (vol<0.0001) vol=0.0001;
                }
                if (vol<0) vol=0;
                if (vol>10) vol=10;
                chip.vol=doInvert?-vol:vol;
                mustBake=true;
              } rightClickable
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x-ImGui::GetFrameHeightWithSpacing()*2.0-ImGui::GetStyle().ItemSpacing.x*2.0);
              if (CWSliderFloat(_("Panning"),&chip.pan,-1.0f,1.0f)) {
                if (chip.pan<-1.0f) chip.pan=-1.0f;
                if (chip.pan>1.0f) chip.pan=1.0f;
                mustBake=true;
              } rightClickable
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x-ImGui::GetFrameHeightWithSpacing()*2.0-ImGui::GetStyle().ItemSpacing.x*2.0);
              if (CWSliderFloat(_("Front/Rear"),&chip.panFR,-1.0f,1.0f)) {
                if (chip.panFR<-1.0f) chip.panFR=-1.0f;
                if (chip.panFR>1.0f) chip.panFR=1.0f;
                mustBake=true;
              } rightClickable

              if (ImGui::TreeNode(_("Configure"))) {
                DivConfig sysFlags;
                sysFlags.loadFromMemory(chip.flags.c_str());
                if (drawSysConf(-1,i,chip.sys,sysFlags,false)) {
                  chip.flags=sysFlags.toString();
                  mustBake=true;
                }
                ImGui::TreePop();
              }

              ImGui::PopID();
            }

            if (doRemove>=0) {
              preset->orig.erase(preset->orig.begin()+doRemove);
              mustBake=true;
            }

            ImGui::Button(ICON_FA_PLUS "##SysAddU");
            if (ImGui::BeginPopupContextItem("SysPickerU",ImGuiPopupFlags_MouseButtonLeft)) {
              DivSystem picked=systemPicker(false);
              if (picked!=DIV_SYSTEM_NULL) {
                preset->orig.push_back(FurnaceGUISysDefChip(picked,1.0f,0.0f,""));
                mustBake=true;
                ImGui::CloseCurrentPopup();
              }
              ImGui::EndPopup();
            }

            ImGui::Separator();

            ImGui::Text(_("Advanced"));
            if (ImGui::InputTextMultiline("##UExtra",&preset->extra,ImVec2(ImGui::GetContentRegionAvail().x,120.0f*dpiScale),ImGuiInputTextFlags_UndoRedo)) {
              mustBake=true;
            }
            if (ImGui::IsItemHovered()) {
              ImGui::SetTooltip(_(
                "insert additional settings in `option=value` format.\n"
                "available options:\n"
                "- tickRate \n"
                "- chanMask \n"
              ));
            }

            if (mustBake) preset->bake();
          } else {
            selectedUserPreset.clear();
          }

          if (doRemovePreset) {
            std::vector<FurnaceGUISysDef>& items=userCategory->systems;
            FurnaceGUISysDef* target=NULL;
            for (size_t i=0; i<selectedUserPreset.size(); i++) {
              if (selectedUserPreset[i]<0 || selectedUserPreset[i]>(int)items.size()) break;
              target=&items[selectedUserPreset[i]];
              if (i<selectedUserPreset.size()-1) {
                items=target->subDefs;
              } else {
                items.erase(items.begin()+selectedUserPreset[i]);
              }
            }

            selectedUserPreset.clear();
          }
        }
      }
      ImGui::EndChild();

      ImGui::EndTable();
    }

    if (ImGui::Button(_("Save and Close"))) {
      userPresetsOpen=false;
    }
    ImGui::SameLine();
    ImGui::Dummy(ImVec2(8.0f*dpiScale,1.0f));
    ImGui::SameLine();
    if (ImGui::Button(_("Import"))) {
      openFileDialog(GUI_FILE_IMPORT_USER_PRESETS);
    }
    ImGui::SameLine();
    if (ImGui::Button(_("Import (replace)"))) {
      openFileDialog(GUI_FILE_IMPORT_USER_PRESETS_REPLACE);
    }
    ImGui::SameLine();
    if (ImGui::Button(_("Export"))) {
      openFileDialog(GUI_FILE_EXPORT_USER_PRESETS);
    }
  }
  if (!userPresetsOpen) {
    saveUserPresets(true);
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_USER_PRESETS;
  ImGui::End();
}
