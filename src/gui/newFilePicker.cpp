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
#include <IconsFontAwesome4.h>
#include <dirent.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <time.h>

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
    switch (entry->d_type) {
      case DT_REG:
        newEntry->type=FP_TYPE_NORMAL;
        break;
      case DT_DIR:
        newEntry->type=FP_TYPE_DIR;
        break;
      case DT_LNK:
        newEntry->type=FP_TYPE_LINK;
        break;
      case DT_SOCK:
        newEntry->type=FP_TYPE_SOCKET;
        break;
      default:
        newEntry->type=FP_TYPE_UNKNOWN;
        break;
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
    struct tm* retTM=localtime_r(&st.st_mtime,&i->time);
    if (retTM!=NULL) {
      i->hasTime=true;
    }

    i->size=st.st_size;
    i->hasSize=true;
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

  // start new file thread
  this->path=path;
  failMessage="";
  haveFiles=false;
  haveStat=false;
  stopReading=false;
  fileThread=new std::thread(_fileThread,this);
}

bool FurnaceFilePicker::draw() {
  if (!isOpen) return false;

  String newDir;

  ImGui::SetNextWindowSizeConstraints(ImVec2(800.0,600.0),ImVec2(8000.0,6000.0));
  if (ImGui::Begin(windowName.c_str(),NULL,ImGuiWindowFlags_NoSavedSettings)) {
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
      if (haveStat) {
        ImGui::Text("Hiya! (%s)",path.c_str());
      } else {
        ImGui::Text("Loading... (%s)",path.c_str());
      }

      if (entries.empty()) {
        if (failMessage.empty()) {
          ImGui::Text("This directory is empty!");
        } else {
          ImGui::Text("Could not load this directory!\n(%s)",failMessage.c_str());
        }
      } else {
        if (ImGui::BeginTable("FileList",3,ImGuiTableFlags_Borders|ImGuiTableFlags_ScrollY)) {
          entryLock.lock();
          int index=0;
          listClipper.Begin(entries.size());
          while (listClipper.Step()) {
            for (int _i=listClipper.DisplayStart; _i<listClipper.DisplayEnd; _i++) {
              FileEntry* i=entries[_i];

              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              if (i->type==FP_TYPE_DIR) {
                ImGui::PushStyleColor(ImGuiCol_Text,0xff00ffff);
              }
              ImGui::PushID(index++);
              if (ImGui::Selectable("##File",false)) {
                if (i->type==FP_TYPE_DIR || i->type==FP_TYPE_LINK) {
                  if (*path.rbegin()=='/') {
                    newDir=path+i->name;
                  } else {
                    newDir=path+'/'+i->name;
                  }
                }
              }
              ImGui::PopID();
              ImGui::SameLine();
              
              ImGui::TextUnformatted(i->name.c_str());

              ImGui::TableNextColumn();
              if (i->hasSize) {
                ImGui::Text("%" PRIu64,i->size);
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
    }
  }
  ImGui::End();

  if (!newDir.empty()) {
    // change directory
    readDirectory(newDir);
  }
  return false;
}

bool FurnaceFilePicker::open(String name, String path, bool modal) {
  if (isOpen) return false;

  readDirectory(path);
  windowName=name;
  isOpen=true;
  return true;
}

FurnaceFilePicker::FurnaceFilePicker():
  fileThread(NULL),
  haveFiles(false),
  haveStat(false),
  stopReading(false),
  isOpen(false) {

}
