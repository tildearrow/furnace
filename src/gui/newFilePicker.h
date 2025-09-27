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

enum FileType {
  FP_TYPE_UNKNOWN=0,
  FP_TYPE_NORMAL,
  FP_TYPE_DIR,
  FP_TYPE_LINK,
  FP_TYPE_PIPE,
  FP_TYPE_SOCKET,
  FP_TYPE_CHARDEV,
  FP_TYPE_BLOCKDEV,

  FP_TYPE_MAX
};

class FurnaceFilePicker {
  enum SortModes {
    FP_SORT_NAME=0,
    FP_SORT_EXT,
    FP_SORT_SIZE,
    FP_SORT_DATE
  };
  struct FileTypeStyle {
    String ext;
    String icon;
    ImVec4 color;
    FileTypeStyle():
      ext(""), icon(""), color(0.0f,0.0f,0.0f,1.0f) {}
  };
  struct FileEntry {
    String path;
    String name;
    String nameLower;
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
  std::vector<String> finalSelection;
  std::vector<String> filterOptions;
  std::thread* fileThread;
  std::mutex entryLock;
  String windowName;
  String path, filter;
  String failMessage;
  String homeDir;
  String entryName;
  ImGuiListClipper listClipper;
  bool haveFiles, haveStat, stopReading, isOpen, isMobile, sortInvert;
  int scheduledSort;
  size_t curFilterType;
  SortModes sortMode;
  FilePickerStatus curStatus;

  std::vector<FileTypeStyle> fileTypeRegistry;
  FileTypeStyle defaultTypeStyle[FP_TYPE_MAX];

  // for "create directory"
  String mkdirPath, mkdirError;

  // for the "edit path" button
  String editablePath;
  bool editingPath;

  void sortFiles();
  void filterFiles();
  void clearAllFiles();
  void updateEntryName();
  void readDirectory(String path);
  String normalizePath(const String& which);
  bool isPathAbsolute(const String& p);

  public:
    void readDirectorySub();
    void setHomeDir(String where);
    FilePickerStatus getStatus();
    const String& getEntryName();
    const std::vector<String>& getSelected();
    void setMobile(bool val);
    bool draw();
    bool open(String name, String path, bool modal, const std::vector<String>& filter);
    void loadSettings(DivConfig& conf);
    void saveSettings(DivConfig& conf);
    void setTypeStyle(FileType type, ImVec4 color, String icon);
    void registerType(String ext, ImVec4 color, String icon);
    void clearTypes();
    FurnaceFilePicker();
};
