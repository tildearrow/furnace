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

#include "../ta-utils.h"
#include "../engine/config.h"
#include <stdint.h>
#include <thread>
#include "imgui.h"

enum FilePickerStatus {
  FP_STATUS_WAITING=0,
  FP_STATUS_ACCEPTED,
  FP_STATUS_CLOSED
};

class FurnaceFilePicker {
  enum FileType {
    FP_TYPE_UNKNOWN=0,
    FP_TYPE_NORMAL,
    FP_TYPE_DIR,
    FP_TYPE_LINK,
    FP_TYPE_PIPE,
    FP_TYPE_SOCKET,
  };
  struct FileEntry {
    String path;
    String name;
    String ext;
    bool hasSize, hasTime, isDir, isSelected;
    uint64_t size;
    struct tm time;
    FileType type;
    FileEntry():
      hasSize(false), hasTime(false), isDir(false), isSelected(false),
      size(0), type(FP_TYPE_UNKNOWN) {}
  };
  std::vector<FileEntry*> entries;
  std::vector<FileEntry*> sortedEntries;
  std::vector<FileEntry*> filteredEntries;
  std::vector<FileEntry*> chosenEntries;
  std::thread* fileThread;
  std::mutex entryLock;
  String windowName;
  String path, filter;
  String failMessage;
  String homeDir;
  String entryName;
  ImGuiListClipper listClipper;
  bool haveFiles, haveStat, stopReading, isOpen;
  int scheduledSort;
  FilePickerStatus curStatus;

  void sortFiles();
  void filterFiles();
  void clearAllFiles();
  void updateEntryName();
  void readDirectory(String path);

  public:
    void readDirectorySub();
    void setHomeDir(String where);
    FilePickerStatus getStatus();
    const std::vector<FileEntry*>& getSelected();
    bool draw();
    bool open(String name, String path, bool modal);
    void loadSettings(DivConfig& conf);
    void saveSettings(DivConfig& conf);
    FurnaceFilePicker();
};
