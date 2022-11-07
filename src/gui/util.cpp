/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2022 tildearrow and contributors
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

#include "util.h"
#include "gui.h"

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include "../utfutils.h"
#else
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#endif

String getHomeDir() {
#ifdef IS_MOBILE

#ifdef ANDROID
  return "/storage/emulated/0/";
#else
  return "/";
#endif

#else
  String ret;
  char tempDir[4096];

#ifdef _WIN32
  wchar_t* up=_wgetenv(L"USERPROFILE");
  if (up!=NULL) {
    ret=utf16To8(up);
    ret+='\\';
  }
#else
  char* home=getenv("HOME");
  if (home!=NULL) {
    ret=home;
    ret+='/';
  } else {
    int uid=getuid();
    struct passwd* entry=getpwuid(uid);
    if (entry!=NULL) {
      if (entry->pw_dir!=NULL) {
        ret=entry->pw_dir;
        ret+='/';
      }
    }
  }
#endif

  if (ret=="") { // fallback
#ifdef _WIN32
    GetCurrentDirectory(4095,tempDir);
    ret=tempDir;
    ret+='\\';
#else
    char* unused=getcwd(tempDir,4095);
    char* unused1=unused; // dang it compiler
    unused=unused1;
    ret=tempDir;
    ret+='/';
#endif
  }

  return ret;
#endif
}

String getKeyName(int key, bool emptyNone) {
  if (key==0) {
    if (emptyNone) {
      return "";
    } else {
      return "<nothing>";
    }
  }
  String ret;
  if (key&FURKMOD_CTRL) ret+="Ctrl-";
  if (key&FURKMOD_META) ret+=META_MODIFIER_NAME;
  if (key&FURKMOD_ALT) ret+="Alt-";
  if (key&FURKMOD_SHIFT) ret+="Shift-";
  if ((key&FURK_MASK)==0xffffff) {
    ret+="...";
    return ret;
  }
  const char* name=SDL_GetKeyName(key&FURK_MASK);
  if (name==NULL) {
    ret+="Unknown";
  } else if (name[0]==0) {
    ret+="Unknown";
  } else {
    ret+=name;
  }
  return ret;
}
