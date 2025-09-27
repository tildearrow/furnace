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

// the name of this function is somewhat misleading.
// it filters files by type, then sorts them.
void FurnaceFilePicker::sortFiles() {
  std::chrono::high_resolution_clock::time_point timeStart=std::chrono::high_resolution_clock::now();
  entryLock.lock();
  // check for "no filter"
  if (filterOptions[curFilterType+1]=="*") {
    // copy entire list
    sortedEntries=entries;
  } else {
    // sort by extension
    std::vector<String> parsedSort;
    String nextType;
    for (char i: filterOptions[curFilterType+1]) {
      switch (i) {
        case '*': // ignore
          break;
        case ' ': // separator
          if (!nextType.empty()) {
            parsedSort.push_back(nextType);
            nextType="";
          }
          break;
        default: // push
          nextType.push_back(i);
          break;
      }
    }
    if (!nextType.empty()) {
      parsedSort.push_back(nextType);
      nextType="";
    }

    sortedEntries.clear();
    for (FileEntry* i: entries) {
      if (i->isDir) {
        sortedEntries.push_back(i);
        continue;
      }
      for (const String& j: parsedSort) {
        if (i->ext==j) {
          sortedEntries.push_back(i);
          break;
        }
      }
    }
  }

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
    if (i->nameLower.find(lowerFilter)!=String::npos) {
      filteredEntries.push_back(i);
    }
  }
}

bool FurnaceFilePicker::draw() {
  if (!isOpen) return false;

  String newDir;
  bool acknowledged=false;

  ImGui::SetNextWindowSizeConstraints(ImVec2(800.0,600.0),ImVec2(8000.0,6000.0));
  if (ImGui::Begin(windowName.c_str(),NULL,ImGuiWindowFlags_NoSavedSettings)) {
    // header bars
    if (ImGui::Button(ICON_FA_PLUS "##MakeDir")) {
      mkdirError="";
      mkdirPath="";
    }
    if (ImGui::BeginPopupContextItem("CreateDir",ImGuiPopupFlags_MouseButtonLeft)) {
      if (mkdirError.empty()) {
        ImGui::Text("Directory name:");

        ImGui::InputText("##DirName",&mkdirPath);

        ImGui::BeginDisabled(mkdirPath.empty());
        if (ImGui::Button("OK")) {
          if (mkdirPath.empty()) {
            mkdirError="Maybe try that again under better circumstances...";
          } else {
            // convert to absolute path if necessary
            if (mkdirPath[0]!='/') {
              if (*path.rbegin()=='/') {
                mkdirPath=path+mkdirPath;
              } else {
                mkdirPath=path+'/'+mkdirPath;
              }
            }
            // create directory
            int result=mkdir(mkdirPath.c_str(),0755);
            if (result!=0) {
              mkdirError=strerror(errno);
            } else {
              newDir=mkdirPath;
              ImGui::CloseCurrentPopup();
            }
          }
        }
        ImGui::EndDisabled();
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
          ImGui::CloseCurrentPopup();
        }
      } else {
        ImGui::Text("I can't! (%s)\nCheck whether the path is correct and you have access to it.",mkdirError.c_str());
        if (ImGui::Button("Back")) {
          mkdirError="";
        }
      }
      ImGui::EndPopup();
    }

    ImGui::SameLine();
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine();

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
    if (ImGui::Button(ICON_FA_PENCIL "##EditPath")) {
      editablePath=path;
      editingPath=true;
    }

    ImGui::SameLine();
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine();

    if (editingPath) {
      ImGui::Text("It's a little early for that... don't you think?");
    } else {
      // TODO: path buttons
      ImGui::TextUnformatted(path.c_str());
    }

    // search bar
    if (ImGui::Button(ICON_FA_REPEAT "##ClearFilter")) {
      filter="";
      filterFiles();
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::InputTextWithHint("##Filter","Search",&filter)) {
      filterFiles();
    }

    if (scheduledSort && haveFiles) {
      scheduledSort=0;
      sortFiles();
      filterFiles();
    }

    ImVec2 tableSize=ImGui::GetContentRegionAvail();
    tableSize.y-=ImGui::GetFrameHeightWithSpacing()*2.0f;

    // display a message on empty dir, no matches or error
    if (!haveFiles) {
      if (ImGui::BeginTable("LoadingFiles",1,ImGuiTableFlags_BordersOuter,tableSize)) {
        ImGui::EndTable();
      }
    } else if (filteredEntries.empty()) {
      if (ImGui::BeginTable("NoFiles",3,ImGuiTableFlags_BordersOuter,tableSize)) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.5f);
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.5f);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        ImGui::TableNextColumn();
        ImGui::SetCursorPosY(ImGui::GetCursorPosY()+(tableSize.y-ImGui::GetTextLineHeight())*0.5);
        if (sortedEntries.empty()) {
          if (failMessage.empty()) {
            ImGui::Text("This directory is empty!");
          } else {
            ImGui::Text("%s!",failMessage.c_str());
          }
        } else {
          if (failMessage.empty()) {
            ImGui::Text("No results");
          } else {
            ImGui::Text("%s!",failMessage.c_str());
          }
        }

        ImGui::TableNextColumn();
        ImGui::EndTable();
      }
    } else {
      // this is the list view. I might add other view modes in the future...
      if (ImGui::BeginTable("FileList",4,ImGuiTableFlags_BordersOuter|ImGuiTableFlags_ScrollY|ImGuiTableFlags_RowBg,tableSize)) {
        float rowHeight=ImGui::GetTextLineHeight()+ImGui::GetStyle().CellPadding.y*2.0f;
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed,ImGui::CalcTextSize(" .eeee").x);
        ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthFixed,ImGui::CalcTextSize(" 999.99G").x);
        ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthFixed,ImGui::CalcTextSize(" 6969/69/69 04:20").x);
        ImGui::TableSetupScrollFreeze(0,1);

        // header (sort options)
        const char* nameHeader="Name##SortName";
        const char* typeHeader="Type##SortType";
        const char* sizeHeader="Size##SortSize";
        const char* dateHeader="Date##SortDate";

        switch (sortMode) {
          case FP_SORT_NAME:
            if (sortInvert) {
              nameHeader=ICON_FA_CHEVRON_UP "Name##SortName";
            } else {
              nameHeader=ICON_FA_CHEVRON_DOWN "Name##SortName";
            }
            break;
          case FP_SORT_EXT:
            if (sortInvert) {
              typeHeader=ICON_FA_CHEVRON_UP "Type##SortType";
            } else {
              typeHeader=ICON_FA_CHEVRON_DOWN "Type##SortType";
            }
            break;
          case FP_SORT_SIZE:
            if (sortInvert) {
              sizeHeader=ICON_FA_CHEVRON_UP "Size##SortSize";
            } else {
              sizeHeader=ICON_FA_CHEVRON_DOWN "Size##SortSize";
            }
            break;
          case FP_SORT_DATE:
            if (sortInvert) {
              dateHeader=ICON_FA_CHEVRON_UP "Date##SortDate";
            } else {
              dateHeader=ICON_FA_CHEVRON_DOWN "Date##SortDate";
            }
            break;
        }

        ImGui::TableNextRow(ImGuiTableRowFlags_Headers,rowHeight);
        ImGui::TableNextColumn();
        if (ImGui::Selectable(nameHeader)) {
          if (sortMode==FP_SORT_NAME) {
            sortInvert=!sortInvert;
          } else {
            sortMode=FP_SORT_NAME;
            scheduledSort=1;
          }
        }
        ImGui::TableNextColumn();
        if (ImGui::Selectable(typeHeader)) {
          if (sortMode==FP_SORT_EXT) {
            sortInvert=!sortInvert;
          } else {
            sortMode=FP_SORT_EXT;
            scheduledSort=1;
          }
        }
        ImGui::TableNextColumn();
        if (ImGui::Selectable(sizeHeader)) {
          if (sortMode==FP_SORT_SIZE) {
            sortInvert=!sortInvert;
          } else {
            sortMode=FP_SORT_SIZE;
            scheduledSort=1;
          }
        }
        ImGui::TableNextColumn();
        if (ImGui::Selectable(dateHeader)) {
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
        listClipper.Begin(filteredEntries.size(),rowHeight);
        while (listClipper.Step()) {
          for (int _i=listClipper.DisplayStart; _i<listClipper.DisplayEnd; _i++) {
            FileEntry* i=filteredEntries[sortInvert?(filteredEntries.size()-_i-1):_i];
            FileTypeStyle* style=&defaultTypeStyle[i->type];

            // get style for this entry
            if (!i->ext.empty()) {
              for (FileTypeStyle& j: fileTypeRegistry) {
                if (i->ext==j.ext) {
                  style=&j;
                  break;
                }
              }
            }

            // draw
            ImGui::TableNextRow(0,rowHeight);
            ImGui::TableNextColumn();
            ImGui::PushStyleColor(ImGuiCol_Text,ImGui::GetColorU32(style->color));
            ImGui::PushID(index++);
            if (ImGui::Selectable(style->icon.c_str(),i->isSelected,ImGuiSelectableFlags_AllowDoubleClick|ImGuiSelectableFlags_SpanAllColumns|ImGuiSelectableFlags_SpanAvailWidth)) {
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

            ImGui::PopStyleColor();
          }
        }
        ImGui::EndTable();
        entryLock.unlock();
      }
    }

    // file name input
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("Name: ");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x*0.68f);
    if (ImGui::InputText("##EntryName",&entryName)) {
      // find an entry with this name
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::BeginCombo("##FilterType",filterOptions[curFilterType].c_str())) {
      for (size_t i=0; i<filterOptions.size(); i+=2) {
        if (ImGui::Selectable(filterOptions[i].c_str(),curFilterType==i)) {
          curFilterType=i;
          scheduledSort=1;
        }
      }
      ImGui::EndCombo();
    }

    // OK/Cancel buttons
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
    ImGui::SameLine();
    if (!haveFiles) {
      ImGui::Text("Loading...");
    } else if (!haveStat) {
      ImGui::Text("Loading... (stat)");
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
  ImGui::End();

  if (!newDir.empty()) {
    // change directory
    readDirectory(newDir);
  }
  return false;
}

bool FurnaceFilePicker::open(String name, String path, bool modal, const std::vector<String>& filter) {
  if (isOpen) return false;
  if (filter.size()&1) {
    logE("invalid filter data! it should be an even-sized vector with even elements containing names and odd ones being the filters.");
    return false;
  }

  filterOptions=filter;

  if (filterOptions.size()<2) {
    filterOptions.push_back("all files");
    filterOptions.push_back("*");
  }
  curFilterType=0;

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

void FurnaceFilePicker::setTypeStyle(FileType type, ImVec4 color, String icon) {
  // "##File" is appended here for performance.
  defaultTypeStyle[type].icon=icon+"##File";
  defaultTypeStyle[type].color=color;
}

void FurnaceFilePicker::registerType(String ext, ImVec4 color, String icon) {
  FileTypeStyle t;
  t.ext=ext;
  // "##File" is appended here for performance.
  t.icon=icon+"##File";
  t.color=color;
  fileTypeRegistry.push_back(t);
}

void FurnaceFilePicker::clearTypes() {
  fileTypeRegistry.clear();
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
  curFilterType(0),
  sortMode(FP_SORT_NAME),
  curStatus(FP_STATUS_WAITING),
  editingPath(false) {
  for (int i=0; i<FP_TYPE_MAX; i++) {
    // "##File" is appended here for performance.
    defaultTypeStyle[i].icon=ICON_FA_QUESTION "##File";
  }
}
