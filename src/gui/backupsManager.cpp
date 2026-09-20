/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2026 tildearrow and contributors
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
#include "../utfutils.h"
#include "../fileutils.h"

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#endif

bool FurnaceGUI::splitBackupName(const char* input, String& backupName, struct tm& backupTime) {
  size_t len=strlen(input);
  if (len<4) return false;

  const char* firstHyphen=NULL;
  const char* secondHyphen=NULL;
  bool whichHyphen=false;
  bool isDateValid=true;
  // -YYYYMMDD-hhmmss.fur
  if (strcmp(&input[len-4],".fur")!=0) return false;
  // find two hyphens
  for (const char* i=input+len; i!=input; i--) {
    if ((*i)=='-') {
      if (whichHyphen) {
        firstHyphen=i;
        break;
      } else {
        secondHyphen=i;
        whichHyphen=true;
      }
    }
  }
  if (firstHyphen==NULL) return false;
  if (secondHyphen==NULL) return false;

  // get the time
  int whichChar=0;
  for (const char* i=secondHyphen+1; *i; i++) {
    if ((*i)<'0' || (*i)>'9') {
      isDateValid=false;
      break;
    }
    switch (whichChar++) {
      case 0:
        backupTime.tm_hour=((*i)-'0')*10;
        break;
      case 1:
        backupTime.tm_hour+=(*i)-'0';
        break;
      case 2:
        backupTime.tm_min=((*i)-'0')*10;
        break;
      case 3:
        backupTime.tm_min+=(*i)-'0';
        break;
      case 4:
        backupTime.tm_sec=((*i)-'0')*10;
        break;
      case 5:
        backupTime.tm_sec+=(*i)-'0';
        break;
    }
    if (whichChar>=6) break;
  }
  if (whichChar!=6) return false;
  if (!isDateValid) return false;
  if (backupTime.tm_hour>23) return false;
  if (backupTime.tm_min>59) return false;
  // intentional
  if (backupTime.tm_sec>60) return false;

  // get the date
  String theDate="";
  for (const char* i=firstHyphen+1; *i; i++) {
    if ((*i)=='-') break;
    if ((*i)<'0' || (*i)>'9') {
      isDateValid=false;
      break;
    }
    theDate+=*i;
  }
  if (!isDateValid) return false;
  if (theDate.size()<5) return false;
  if (theDate.size()>14) return false;
  String mmdd=theDate.substr(theDate.size()-4);
  if (mmdd.size()!=4) return false;
  backupTime.tm_mon=(mmdd[0]-'0')*10+(mmdd[1]-'0')-1;
  backupTime.tm_mday=(mmdd[2]-'0')*10+(mmdd[3]-'0');
  if (backupTime.tm_mon>12) return false;
  if (backupTime.tm_mday>31) return false;
  String yyyy=theDate.substr(0,theDate.size()-4);
  try {
    backupTime.tm_year=std::stoi(yyyy)-1900;
  } catch (std::exception& e) {
    return false;
  }

  backupName="";
  for (const char* i=input; i!=firstHyphen && (*i); i++) {
    backupName+=*i;
  }

  return true;
}

void FurnaceGUI::purgeBackups(int year, int month, int day) {
#ifdef _WIN32
  String findPath=backupPath+String(DIR_SEPARATOR_STR)+String("*.fur");
  WString findPathW=utf8To16(findPath.c_str());
  WIN32_FIND_DATAW next;
  HANDLE backDir=FindFirstFileW(findPathW.c_str(),&next);
  if (backDir!=INVALID_HANDLE_VALUE) {
    do {
      String backupName;
      struct tm backupTime;
      String cFileNameU=utf16To8(next.cFileName);
      bool deleteBackup=false;
      if (!splitBackupName(cFileNameU.c_str(),backupName,backupTime)) continue;

      if (year==0) {
        deleteBackup=true;
      } else if (backupTime.tm_year<(year-1900)) {
        deleteBackup=true;
      } else if (backupTime.tm_year==(year-1900) && backupTime.tm_mon<(month-1)) {
        deleteBackup=true;
      } else if (backupTime.tm_year==(year-1900) && backupTime.tm_mon==(month-1) && backupTime.tm_mday<day) {
        deleteBackup=true;
      }

      if (deleteBackup) {
        String nextPath=backupPath+DIR_SEPARATOR_STR+cFileNameU;
        deleteFile(nextPath.c_str());
      }
    } while (FindNextFileW(backDir,&next)!=0);
    FindClose(backDir);
  }
#else
  DIR* backDir=opendir(backupPath.c_str());
  if (backDir==NULL) {
    logW("could not open backups dir!");
    return;
  }
  while (true) {
    String backupName;
    struct tm backupTime;
    struct dirent* next=readdir(backDir);
    bool deleteBackup=false;
    if (next==NULL) break;
    if (strcmp(next->d_name,".")==0) continue;
    if (strcmp(next->d_name,"..")==0) continue;
    if (!splitBackupName(next->d_name,backupName,backupTime)) continue;

    if (year==0) {
      deleteBackup=true;
    } else if (backupTime.tm_year<(year-1900)) {
      deleteBackup=true;
    } else if (backupTime.tm_year==(year-1900) && backupTime.tm_mon<(month-1)) {
      deleteBackup=true;
    } else if (backupTime.tm_year==(year-1900) && backupTime.tm_mon==(month-1) && backupTime.tm_mday<day) {
      deleteBackup=true;
    }

    if (deleteBackup) {
      String nextPath=backupPath+DIR_SEPARATOR_STR+next->d_name;
      deleteFile(nextPath.c_str());
    }
  }
  closedir(backDir);
#endif
  refreshBackups=true;
}

void FurnaceGUI::drawBackupsManager() {
  if (nextWindow==GUI_WINDOW_BACKUPS_MANAGER) {
    backupsManagerOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!backupsManagerOpen) return;
  if (ImGui::Begin("Backup Management",&backupsManagerOpen,globalWinFlags,_("Backup Management"))) {
    bool purgeDateChanged=false;

    ImGui::AlignTextToFramePadding();
    ImGui::Text(_("Purge before:"));
    ImGui::SameLine();
    ImGui::SetNextItemWidth(60.0f*dpiScale);
    if (ImGui::InputInt("##PYear",&purgeYear,0,0)) purgeDateChanged=true;
    ImGui::SameLine();
    ImGui::SetNextItemWidth(40.0f*dpiScale);
    if (ImGui::InputInt("##PMonth",&purgeMonth,0,0)) purgeDateChanged=true;
    ImGui::SameLine();
    ImGui::SetNextItemWidth(40.0f*dpiScale);
    if (ImGui::InputInt("##PDay",&purgeDay,0,0)) purgeDateChanged=true;

    if (purgeDateChanged) {
      // check month/day validity
      time_t thisMakesNoSense=time(NULL);
      bool tmFailed=false;
      struct tm curTime;
#ifdef _WIN32
      struct tm* tempTM=localtime(&thisMakesNoSense);
      if (tempTM==NULL) {
        memset(&curTime,0,sizeof(struct tm));
        tmFailed=true;
      } else {
        memcpy(&curTime,tempTM,sizeof(struct tm));
      }
#else
      if (localtime_r(&thisMakesNoSense,&curTime)==NULL) {
        memset(&curTime,0,sizeof(struct tm));
        tmFailed=true;
      }
#endif

      // don't allow dates in the future
      if (!tmFailed) {
        int curYear=curTime.tm_year+1900;
        int curMonth=curTime.tm_mon+1;
        int curDay=curTime.tm_mday;

        if (purgeYear<1) purgeYear=1;
        if (purgeYear>curYear) purgeYear=curYear;

        if (purgeYear==curYear) {
          if (purgeMonth>curMonth) purgeMonth=curMonth;

          if (purgeMonth==curMonth) {
            if (purgeDay>curDay) purgeDay=curDay;
          }
        }
      }

      // general checks
      if (purgeYear<1) purgeYear=1;
      if (purgeMonth<1) purgeMonth=1;
      if (purgeMonth>12) purgeMonth=12;
      if (purgeDay<1) purgeDay=1;

      // 1752 calendar alignment
      if (purgeYear==1752 && purgeMonth==9) {
        if (purgeDay>2 && purgeDay<14) purgeDay=2;
      }
      if (purgeMonth==2) {
        // leap year
        if ((purgeYear&3)==0 && ((purgeYear%100)!=0 || (purgeYear%400)==0)) {
          if (purgeDay>29) purgeDay=29;
        } else {
          if (purgeDay>28) purgeDay=28;
        }
      } else if (purgeMonth==1 || purgeMonth==3 || purgeMonth==5 || purgeMonth==7 || purgeMonth==8 || purgeMonth==10 || purgeMonth==12) {
        if (purgeDay>31) purgeDay=31;
      } else {
        if (purgeDay>30) purgeDay=30;
      }
    }

    ImGui::SameLine();
    if (ImGui::Button(_("Go##PDate"))) {
      purgeBackups(purgeYear,purgeMonth,purgeDay);
    }

    backupEntryLock.lock();
    ImGui::AlignTextToFramePadding();
    if (totalBackupSize>=(1ULL<<50ULL)) {
      ImGui::Text(_("%" PRIu64 "PB used"),totalBackupSize>>50);
    } else if (totalBackupSize>=(1ULL<<40ULL)) {
      ImGui::Text(_("%" PRIu64 "TB used"),totalBackupSize>>40);
    } else if (totalBackupSize>=(1ULL<<30ULL)) {
      ImGui::Text(_("%" PRIu64 "GB used"),totalBackupSize>>30);
    } else if (totalBackupSize>=(1ULL<<20ULL)) {
      ImGui::Text(_("%" PRIu64 "MB used"),totalBackupSize>>20);
    } else if (totalBackupSize>=(1ULL<<10ULL)) {
      ImGui::Text(_("%" PRIu64 "KB used"),totalBackupSize>>10);
    } else {
      ImGui::Text(_("%" PRIu64 " bytes used"),totalBackupSize);
    }

    ImGui::SameLine();

    if (ImGui::Button(_("Refresh"))) {
      refreshBackups=true;
    }
    ImGui::SameLine();
    if (ImGui::Button(_("Delete all"))) {
      purgeBackups(0,0,0);
    }

    if (ImGui::BeginTable("BackupList",3,ImGuiTableFlags_ScrollY|ImGuiTableFlags_Borders)) {
      ImGui::TableSetupScrollFreeze(0, 1);
      ImGui::TableSetupColumn(_("Name"),ImGuiTableColumnFlags_WidthStretch,0.6f);
      ImGui::TableSetupColumn(_("Size"),ImGuiTableColumnFlags_WidthStretch,0.15f);
      ImGui::TableSetupColumn(_("Latest"),ImGuiTableColumnFlags_WidthStretch,0.25f);

      ImGui::TableHeadersRow();

      for (FurnaceGUIBackupEntry& i: backupEntries) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(i.name.c_str());
        ImGui::TableNextColumn();
        if (i.size>=(1ULL<<50ULL)) {
          ImGui::Text(_("%" PRIu64 "P"),i.size>>50);
        } else if (i.size>=(1ULL<<40ULL)) {
          ImGui::Text(_("%" PRIu64 "T"),i.size>>40);
        } else if (i.size>=(1ULL<<30ULL)) {
          ImGui::Text(_("%" PRIu64 "G"),i.size>>30);
        } else if (i.size>=(1ULL<<20ULL)) {
          ImGui::Text(_("%" PRIu64 "M"),i.size>>20);
        } else if (i.size>=(1ULL<<10ULL)) {
          ImGui::Text(_("%" PRIu64 "K"),i.size>>10);
        } else {
          ImGui::Text(_("%" PRIu64 ""),i.size);
        }
        ImGui::TableNextColumn();
        ImGui::Text("%d-%02d-%02d",i.lastEntryTime.tm_year+1900,i.lastEntryTime.tm_mon+1,i.lastEntryTime.tm_mday);
      }

      ImGui::EndTable();
    }
    backupEntryLock.unlock();
    if (refreshBackups) {
      refreshBackups=false;
      if (backupEntryTask.valid()) backupEntryTask.get();
      backupEntryTask=std::async(std::launch::async,[this]() -> bool {
        backupEntryLock.lock();
        backupEntries.clear();
        totalBackupSize=0;
        backupEntryLock.unlock();

#ifdef _WIN32
        String findPath=backupPath+String(DIR_SEPARATOR_STR)+String("*.fur");
        WString findPathW=utf8To16(findPath.c_str());
        WIN32_FIND_DATAW next;
        HANDLE backDir=FindFirstFileW(findPathW.c_str(),&next);
        if (backDir!=INVALID_HANDLE_VALUE) {
          do {
            FurnaceGUIBackupEntry nextEntry;
            String cFileNameU=utf16To8(next.cFileName);
            if (!splitBackupName(cFileNameU.c_str(),nextEntry.name,nextEntry.lastEntryTime)) continue;

            nextEntry.size=(((uint64_t)next.nFileSizeHigh)<<32)|next.nFileSizeLow;

            backupEntryLock.lock();
            backupEntries.push_back(nextEntry);
            totalBackupSize+=nextEntry.size;
            backupEntryLock.unlock();
          } while (FindNextFileW(backDir,&next)!=0);
          FindClose(backDir);
        }
#else
        DIR* backDir=opendir(backupPath.c_str());
        if (backDir==NULL) {
          logW("could not open backups dir!");
          return false;
        }
        while (true) {
          FurnaceGUIBackupEntry nextEntry;
          struct stat nextStat;
          struct dirent* next=readdir(backDir);
          if (next==NULL) break;
          if (strcmp(next->d_name,".")==0) continue;
          if (strcmp(next->d_name,"..")==0) continue;
          if (!splitBackupName(next->d_name,nextEntry.name,nextEntry.lastEntryTime)) continue;

          String nextPath=backupPath+DIR_SEPARATOR_STR+next->d_name;

          if (stat(nextPath.c_str(),&nextStat)>=0) {
            nextEntry.size=nextStat.st_size;
          }

          backupEntryLock.lock();
          backupEntries.push_back(nextEntry);
          totalBackupSize+=nextEntry.size;
          backupEntryLock.unlock();
        }
        closedir(backDir);
#endif

        // sort and merge
        backupEntryLock.lock();
        std::sort(backupEntries.begin(),backupEntries.end(),[](const FurnaceGUIBackupEntry& a, const FurnaceGUIBackupEntry& b) -> bool {
          int sc=strcmp(a.name.c_str(),b.name.c_str());
          if (sc==0) {
            if (a.lastEntryTime.tm_year==b.lastEntryTime.tm_year) {
              if (a.lastEntryTime.tm_mon==b.lastEntryTime.tm_mon) {
                if (a.lastEntryTime.tm_mday==b.lastEntryTime.tm_mday) {
                  if (a.lastEntryTime.tm_hour==b.lastEntryTime.tm_hour) {
                    if (a.lastEntryTime.tm_min==b.lastEntryTime.tm_min) {
                      return (a.lastEntryTime.tm_sec<b.lastEntryTime.tm_sec);
                    } else {
                      return (a.lastEntryTime.tm_min<b.lastEntryTime.tm_min);
                    }
                  } else {
                    return (a.lastEntryTime.tm_hour<b.lastEntryTime.tm_hour);
                  }
                } else {
                  return (a.lastEntryTime.tm_mday<b.lastEntryTime.tm_mday);
                }
              } else {
                return (a.lastEntryTime.tm_mon<b.lastEntryTime.tm_mon);
              }
            } else {
              return (a.lastEntryTime.tm_year<b.lastEntryTime.tm_year);
            }
          }

          return sc<0;
        });
        for (size_t i=1; i<backupEntries.size(); i++) {
          FurnaceGUIBackupEntry& prevEntry=backupEntries[i-1];
          FurnaceGUIBackupEntry& thisEntry=backupEntries[i];

          if (thisEntry.name==prevEntry.name) {
            prevEntry.size+=thisEntry.size;
            backupEntries.erase(backupEntries.begin()+i);
            i--;
          }
        }
        backupEntryLock.unlock();
        return true;
      });
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_BACKUPS_MANAGER;
  ImGui::End();
}
