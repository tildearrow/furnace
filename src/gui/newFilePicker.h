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

#ifndef _NEW_FILE_DIALOG_H
#define _NEW_FILE_DIALOG_H

#include "../ta-utils.h"
#include "../engine/config.h"
#include <stdint.h>
#include <functional>
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

enum FilePickerFlags {
  // display file picker as a modal pop-up
  FP_FLAGS_MODAL=1,
  // don't close the file picker on result
  FP_FLAGS_NO_CLOSE=2,
  // confirm overwrite and don't auto-acknowledge-on-tap on mobile
  FP_FLAGS_SAVE=4,
  // allow multiple selection
  FP_FLAGS_MULTI_SELECT=8,
  // directory selection mode
  FP_FLAGS_DIR_SELECT=16,
  // allows you to embed the file picker in an existing window (with draw())
  // DO NOT USE with FP_FLAGS_MODAL!
  FP_FLAGS_EMBEDDABLE=32
};

typedef std::function<void(const char*)> FilePickerSelectCallback;

class FurnaceFilePicker {
  enum SortModes {
    FP_SORT_NAME=0,
    FP_SORT_EXT,
    FP_SORT_SIZE,
    FP_SORT_DATE,

    FP_SORT_MAX
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
    bool hasSize, hasTime, isDir, isHidden, isSelected;
    uint64_t size;
    struct tm time;
    FileType type;
    FileEntry():
      hasSize(false), hasTime(false), isDir(false), isHidden(false), isSelected(false),
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
  String path, filter, searchQuery;
  String failMessage;
  String homeDir;
  String entryName;
  ImGuiListClipper listClipper;
  ImVec2 minSize, maxSize;
  bool haveFiles, haveStat, stopReading, isOpen, isSave_, isMobile, focusEntryName;
  bool sortInvert[FP_SORT_MAX];
  bool multiSelect;
  bool confirmOverwrite, dirSelect, noClose, isModal, isEmbed, hasSizeConstraints;
  bool isPathBookmarked, isSearch;
  int scheduledSort, imguiFlags, editingBookmark;
  size_t curFilterType;
  float lastScrollY;
  int enforceScrollY;
  SortModes sortMode;
  FilePickerStatus curStatus;
  FilePickerSelectCallback selCallback;

  std::vector<FileTypeStyle> fileTypeRegistry;
  FileTypeStyle defaultTypeStyle[FP_TYPE_MAX];

  // for "create directory"
  String mkdirPath, mkdirError;

  // for "create bookmark"
  String newBookmarkName, newBookmarkPath;

  // for the "edit path" button
  String editablePath;
  bool editingPath;

  // configuration
  String configPrefix;
  std::vector<String> bookmarks;
  bool showBookmarks;
  bool showHiddenFiles;
  bool singleClickSelect;
  bool clearSearchOnDirChange;
  bool sortDirsFirst;
  bool naturalSort;
  bool displayType, displaySize, displayDate;

  void sortFiles();
  void filterFiles();
  void clearAllFiles();
  void updateEntryName();
  bool readDirectory(String path);
  String normalizePath(const String& which);
  bool isPathAbsolute(const String& p);
  void addBookmark(const String& p, String n="");
  FileEntry* makeEntry(void* _entry, const char* prefix=NULL);
  void completeStat();

  void drawFileList(ImVec2& tableSize, bool& acknowledged);
  void drawBookmarks(ImVec2& tableSize, String& newDir);

  void acceptAndClose();

  public:
    void readDirectorySub();
    void searchSub(String subPath, int depth);
    void setHomeDir(String where);
    FilePickerStatus getStatus();
    const String& getPath();
    const String& getEntryName();
    const std::vector<String>& getSelected();
    void setMobile(bool val);
    void setSizeConstraints(const ImVec2& min, const ImVec2& max);
    bool draw(ImGuiWindowFlags winFlags=0);
    bool isOpened();
    bool isSave();
    void close();
    bool open(String name, String path, String hint, int flags, const std::vector<String>& filter, FilePickerSelectCallback selectCallback=NULL);
    void loadSettings(DivConfig& conf);
    void saveSettings(DivConfig& conf);
    void setTypeStyle(FileType type, ImVec4 color, String icon);
    void registerType(String ext, ImVec4 color, String icon);
    void clearTypes();
    void setConfigPrefix(String prefix);
    FurnaceFilePicker();
};

#endif
