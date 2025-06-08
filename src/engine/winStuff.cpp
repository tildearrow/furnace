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

#include "winStuff.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include "../ta-log.h"
#include "../utfutils.h"

String getWinConfigPath() {
  wchar_t path[4096];
  WString configPath;
  HRESULT configHR;
  if ((configHR=SHGetFolderPathW(NULL,CSIDL_APPDATA,NULL,0,path))==S_OK) {
    configPath=path;
    configPath+=L"\\furnace";
    if (!PathIsDirectoryW(configPath.c_str())) {
      logI("creating config dir...");
      int mkdirRet;
      if ((mkdirRet=SHCreateDirectoryExW(NULL,configPath.c_str(),NULL))!=ERROR_SUCCESS) {
        logW("could not make config dir! (%.8x)",mkdirRet);
        configPath=L".";
      }
    }
  } else {
    logW("unable to determine config directory! (%.8x)",configHR);
    configPath=L".";
  }
  return utf16To8(configPath.c_str());
}
