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

// this is the code to a new file picker using Dear ImGui.
// this will eventually replace ImGuiFileDialog as the built-in file picker.

#include "newFilePicker.h"
#include "IconsFontAwesome4.h"
#include "misc/cpp/imgui_stdlib.h"
#include "../ta-log.h"
#include <algorithm>
#include <chrono>
#include <dirent.h>
#include <fcntl.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

static const char* sizeSuffixes=".KMGTPEZ";

static void _fileThread(void* item) {
  ((FurnaceFilePicker*)item)->readDirectorySub();
}

void FurnaceFilePicker::readDirectorySub() {
  /// STAGE 1: get file list
  DIR* dir=opendir(path.c_str());

  if (dir==NULL) {
    failMessage=strerror(errno);
    haveFiles=true;
    haveStat=true;
    return;
  }

  struct dirent* entry=NULL;
  while (true) {
    entry=readdir(dir);
    if (entry==NULL) break;
    if (strcmp(entry->d_name,".")==0) continue;
    if (strcmp(entry->d_name,"..")==0) continue;

    FileEntry* newEntry=new FileEntry;

    newEntry->name=entry->d_name;
    newEntry->nameLower=entry->d_name;
    for (char& i: newEntry->nameLower) {
      if (i>='A' && i<='Z') i+='a'-'A';
    }

    switch (entry->d_type) {
      case DT_REG:
        newEntry->type=FP_TYPE_NORMAL;
        break;
      case DT_DIR:
        newEntry->type=FP_TYPE_DIR;
        newEntry->isDir=true;
        break;
      case DT_LNK: {
        newEntry->type=FP_TYPE_LINK;
        // resolve link to determine whether this is a directory
        String readLinkPath;
        DIR* readLinkDir=NULL;
        if (*path.rbegin()=='/') {
          readLinkPath=path+newEntry->name;
        } else {
          readLinkPath=path+'/'+newEntry->name;
        }
        // silly, but works.
        logV("Read a symlink...");
        readLinkDir=opendir(readLinkPath.c_str());
        if (readLinkDir!=NULL) {
          logV("Is file");
          newEntry->isDir=true;
          closedir(readLinkDir);
        }
        break;
      }
      case DT_SOCK:
        newEntry->type=FP_TYPE_SOCKET;
        break;
      default:
        newEntry->type=FP_TYPE_UNKNOWN;
        break;
    }

    if (!newEntry->isDir) {
      size_t extPos=newEntry->name.rfind('.');
      if (extPos!=String::npos) {
        newEntry->ext=newEntry->name.substr(extPos);
        for (char& i: newEntry->ext) {
          if (i>='A' && i<='Z') i+='a'-'A';
        }
      }
    }

    entries.push_back(newEntry);
    if (stopReading) {
      break;
    }
  }
  if (closedir(dir)!=0) {
    // ?!
  }

  // we're done - this is sufficient to show a file list (and sort by name)
  haveFiles=true;

  /// STAGE 2: retrieve file information
  struct stat st;
  String filePath;
  for (FileEntry* i: entries) {
    if (stopReading) {
      return;
    }

    if (*path.rbegin()=='/') {
      filePath=path+i->name;
    } else {
      filePath=path+'/'+i->name;
    }

    if (stat(filePath.c_str(),&st)<0) {
      // fall back to unknown
      continue;
    }

    // read file information
    entryLock.lock();
    struct tm* retTM=localtime_r(&st.st_mtime,&i->time);
    if (retTM!=NULL) {
      i->hasTime=true;
    }

    i->size=st.st_size;
    i->hasSize=true;
    entryLock.unlock();
  }
  haveStat=true;
}

void FurnaceFilePicker::readDirectory(String path) {
  if (fileThread!=NULL) {
    // stop current file thread
    stopReading=true;
    fileThread->join();
    delete fileThread;
    fileThread=NULL;
  }

  // clear all entries
  sortedEntries.clear();
  for (FileEntry* i: entries) {
    delete i;
  }
  entries.clear();
  chosenEntries.clear();
  updateEntryName();

  // start new file thread
  this->path=path;
  failMessage="";
  haveFiles=false;
  haveStat=false;
  stopReading=false;
  scheduledSort=1;
  fileThread=new std::thread(_fileThread,this);
}

void FurnaceFilePicker::setHomeDir(String where) {
  homeDir=where;
}

void FurnaceFilePicker::updateEntryName() {
  if (chosenEntries.empty()) {
    entryName="";
  } else if (chosenEntries.size()>1) {
    entryName="<multiple files selected>";
  } else {
    entryName=chosenEntries[0]->name;
  }
}

void FurnaceFilePicker::sortFiles() {
  std::chrono::high_resolution_clock::time_point timeStart=std::chrono::high_resolution_clock::now();
  entryLock.lock();
  sortedEntries=entries;

  // sort by name
  std::sort(sortedEntries.begin(),sortedEntries.end(),[this](const FileEntry* a, const FileEntry* b) -> bool {
    if (a->isDir && !b->isDir) return true;
    if (!a->isDir && b->isDir) return false;

    switch (sortMode) {
      case FP_SORT_NAME: {
        // don't do anything. this is handled below.
        break;
      }
      case FP_SORT_EXT: {
        int result=a->ext.compare(b->ext);
        // only sort if extensions differ
        if (result!=0) {
          return result<0;
        }
        break;
      }
      case FP_SORT_SIZE: {
        // only sort if sizes differ
        if (a->size!=b->size) {
          return a->size<b->size;
        }
        break;
      }
      case FP_SORT_DATE: {
        // only sort if dates differ
        if (a->time.tm_year==b->time.tm_year) {
          if (a->time.tm_mon==b->time.tm_mon) {
            if (a->time.tm_mday==b->time.tm_mday) {
              if (a->time.tm_hour==b->time.tm_hour) {
                if (a->time.tm_min==b->time.tm_min) {
                  if (a->time.tm_sec==b->time.tm_sec) {
                    // fall back to sorting by name
                    return a->nameLower<b->nameLower;
                  }
                  return a->time.tm_sec<b->time.tm_sec;
                }
                return a->time.tm_min<b->time.tm_min;
              }
              return a->time.tm_hour<b->time.tm_hour;
            }
            return a->time.tm_mday<b->time.tm_mday;
          }
          return a->time.tm_mon<b->time.tm_mon;
        }
        return a->time.tm_year<b->time.tm_year;
        break;
      }
    }

    // fall back to sorting by name
    return a->nameLower<b->nameLower;
  });
  entryLock.unlock();
  std::chrono::high_resolution_clock::time_point timeEnd=std::chrono::high_resolution_clock::now();
  logV("sortFiles() took %dµs",std::chrono::duration_cast<std::chrono::microseconds>(timeEnd-timeStart).count());
}

void FurnaceFilePicker::filterFiles() {
  if (filter.empty()) {
    filteredEntries=sortedEntries;
    return;
  }

  filteredEntries.clear();

  String lowerFilter=filter;
  for (char& i: lowerFilter) {
    if (i>='A' && i<='Z') i+='a'-'A';
  }

  for (FileEntry* i: sortedEntries) {
    String lowerName=i->name;
    for (char& j: lowerName) {
      if (j>='A' && j<='Z') j+='a'-'A';
    }

    if (lowerName.find(lowerFilter)!=String::npos) {
      filteredEntries.push_back(i);
    }
  }
}

bool FurnaceFilePicker::draw() {
  if (!isOpen) return false;

  String newDir;

  ImGui::SetNextWindowSizeConstraints(ImVec2(800.0,600.0),ImVec2(8000.0,6000.0));
  if (ImGui::Begin(windowName.c_str(),NULL,ImGuiWindowFlags_NoSavedSettings)) {
    if (ImGui::Button(ICON_FA_HOME "##HomeDir")) {
      newDir=homeDir;
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_CHEVRON_UP "##ParentDir")) {
      size_t pos=path.rfind('/');
      if (pos!=String::npos && path!="/") {
        newDir=path.substr(0,pos);
        if (newDir.empty()) newDir="/";
      }
    }
    ImGui::SameLine();
    if (!haveFiles) {
      ImGui::Text("Loading... (%s)",path.c_str());
    } else {
      bool acknowledged=false;
      
      if (haveStat) {
        ImGui::Text("Hiya! (%s)",path.c_str());
      } else {
        ImGui::Text("Loading... (%s)",path.c_str());
      }
      if (ImGui::Button(ICON_FA_REPEAT "##ClearFilter")) {
        filter="";
        filterFiles();
      }
      ImGui::SameLine();
      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
      if (ImGui::InputTextWithHint("##Filter","Search",&filter)) {
        filterFiles();
      }

      if (scheduledSort) {
        scheduledSort=0;
        sortFiles();
        filterFiles();
      }

      ImVec2 tableSize=ImGui::GetContentRegionAvail();
      tableSize.y-=ImGui::GetFrameHeightWithSpacing()*2.0f;

      if (filteredEntries.empty()) {
        if (sortedEntries.empty()) {
          if (failMessage.empty()) {
            ImGui::Text("This directory is empty!");
          } else {
            ImGui::Text("Could not load this directory!\n(%s)",failMessage.c_str());
          }
        } else {
          if (failMessage.empty()) {
            ImGui::Text("No results");
          } else {
            ImGui::Text("Could not load this directory!\n(%s)",failMessage.c_str());
          }
        }
      } else {
        // this is the list view. I might add other view modes in the future...
        if (ImGui::BeginTable("FileList",4,ImGuiTableFlags_Borders|ImGuiTableFlags_ScrollY,tableSize)) {
          ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch);
          ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed,ImGui::CalcTextSize(" .eeee").x);
          ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthFixed,ImGui::CalcTextSize(" 999.99G").x);
          ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthFixed,ImGui::CalcTextSize(" 6969/69/69 04:20").x);
          ImGui::TableSetupScrollFreeze(0,1);

          // header (sort options)
          ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
          ImGui::TableNextColumn();
          if (ImGui::Selectable("Name##SortName")) {
            if (sortMode==FP_SORT_NAME) {
              sortInvert=!sortInvert;
            } else {
              sortMode=FP_SORT_NAME;
              scheduledSort=1;
            }
          }
          ImGui::TableNextColumn();
          if (ImGui::Selectable("Type##SortType")) {
            if (sortMode==FP_SORT_EXT) {
              sortInvert=!sortInvert;
            } else {
              sortMode=FP_SORT_EXT;
              scheduledSort=1;
            }
          }
          ImGui::TableNextColumn();
          if (ImGui::Selectable("Size##SortSize")) {
            if (sortMode==FP_SORT_SIZE) {
              sortInvert=!sortInvert;
            } else {
              sortMode=FP_SORT_SIZE;
              scheduledSort=1;
            }
          }
          ImGui::TableNextColumn();
          if (ImGui::Selectable("Date##SortDate")) {
            if (sortMode==FP_SORT_DATE) {
              sortInvert=!sortInvert;
            } else {
              sortMode=FP_SORT_DATE;
              scheduledSort=1;
            }
          }

          // file list
          entryLock.lock();
          int index=0;
          listClipper.Begin(filteredEntries.size());
          while (listClipper.Step()) {
            for (int _i=listClipper.DisplayStart; _i<listClipper.DisplayEnd; _i++) {
              FileEntry* i=filteredEntries[sortInvert?(filteredEntries.size()-_i-1):_i];

              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              if (i->type==FP_TYPE_DIR) {
                ImGui::PushStyleColor(ImGuiCol_Text,0xff00ffff);
              }
              ImGui::PushID(index++);
              if (ImGui::Selectable("##File",i->isSelected,ImGuiSelectableFlags_AllowDoubleClick|ImGuiSelectableFlags_SpanAllColumns|ImGuiSelectableFlags_SpanAvailWidth)) {
                for (FileEntry* j: chosenEntries) {
                  j->isSelected=false;
                }
                chosenEntries.clear();
                chosenEntries.push_back(i);
                i->isSelected=true;
                updateEntryName();
                if (isMobile) {
                  acknowledged=true;
                } else if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                  acknowledged=true;
                }
              }
              ImGui::PopID();
              ImGui::SameLine();
              
              ImGui::TextUnformatted(i->name.c_str());

              ImGui::TableNextColumn();
              ImGui::TextUnformatted(i->ext.c_str());

              ImGui::TableNextColumn();
              if (i->hasSize && i->type==FP_TYPE_NORMAL) {
                int sizeShift=0;
                uint64_t sizeShifted=i->size;

                while (sizeShifted && sizeShift<7) {
                  sizeShifted>>=10;
                  sizeShift++;
                }

                sizeShift--;

                uint64_t intPart=i->size>>(sizeShift*10);
                uint64_t fracPart=i->size&((1U<<(sizeShift*10))-1);
                // shift so we have sufficient digits for 100
                // (precision loss is negligible)
                if (sizeShift>0) {
                  fracPart=(100*(fracPart>>3))>>((sizeShift*10)-3);
                  if (fracPart>99) fracPart=99;
                  ImGui::Text("%" PRIu64 ".%02" PRIu64 "%c",intPart,fracPart,sizeSuffixes[sizeShift&7]);
                } else {
                  ImGui::Text("%" PRIu64,i->size);
                }
              }

              ImGui::TableNextColumn();
              if (i->hasTime) {
                ImGui::Text("%d/%02d/%02d %02d:%02d",i->time.tm_year+1900,i->time.tm_mon+1,i->time.tm_mday,i->time.tm_hour,i->time.tm_min);
              }

              if (i->type==FP_TYPE_DIR) {
                ImGui::PopStyleColor();
              }
            }
          }
          ImGui::EndTable();
          entryLock.unlock();
        }
      }

      ImGui::AlignTextToFramePadding();
      ImGui::TextUnformatted("Name: ");
      ImGui::SameLine();
      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
      if (ImGui::InputText("##EntryName",&entryName)) {
        // find an entry with this name

        
      }

      ImGui::BeginDisabled(chosenEntries.empty());
      if (ImGui::Button("OK")) {
        // accept entry
        acknowledged=true;
      }
      ImGui::EndDisabled();
      ImGui::SameLine();
      if (ImGui::Button("Cancel")) {
        curStatus=FP_STATUS_CLOSED;
        isOpen=false;
      }

      if (acknowledged) {
        if (!chosenEntries.empty()) {
          if (chosenEntries.size()==1 && chosenEntries[0]->isDir) {
            // go there unless we've been required to select a directory
            if (*path.rbegin()=='/') {
              newDir=path+chosenEntries[0]->name;
            } else {
              newDir=path+'/'+chosenEntries[0]->name;
            }
          } else {
            // select this entry
            curStatus=FP_STATUS_ACCEPTED;
            isOpen=false;
          }
        }
      }
    }
  }
  ImGui::End();

  if (!newDir.empty()) {
    // change directory
    readDirectory(newDir);
  }
  return false;
}

bool FurnaceFilePicker::open(String name, String path, bool modal, const std::vector<String>& filter) {
  if (isOpen) return false;

  readDirectory(path);
  windowName=name;
  isOpen=true;
  return true;
}

const String& FurnaceFilePicker::getEntryName() {
  return entryName;
}

void FurnaceFilePicker::setMobile(bool val) {
  isMobile=val;
}

FilePickerStatus FurnaceFilePicker::getStatus() {
  FilePickerStatus retStatus=curStatus;
  curStatus=FP_STATUS_WAITING;
  return retStatus;
}

void FurnaceFilePicker::loadSettings(DivConfig& conf) {

}

void FurnaceFilePicker::saveSettings(DivConfig& conf) {

}

FurnaceFilePicker::FurnaceFilePicker():
  fileThread(NULL),
  haveFiles(false),
  haveStat(false),
  stopReading(false),
  isOpen(false),
  isMobile(false),
  sortInvert(false),
  scheduledSort(0),
  sortMode(FP_SORT_NAME),
  curStatus(FP_STATUS_WAITING) {

}
