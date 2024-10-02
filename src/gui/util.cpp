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
      return _("<nothing>");
    }
  }
  String ret;
  if (key&FURKMOD_CTRL) ret+=_("Ctrl-");
  if (key&FURKMOD_META) ret+=META_MODIFIER_NAME;
  if (key&FURKMOD_ALT) ret+=_("Alt-");
  if (key&FURKMOD_SHIFT) ret+=_("Shift-");
  if ((key&FURK_MASK)==0xffffff) {
    ret+="...";
    return ret;
  }
  const char* name=SDL_GetKeyName(key&FURK_MASK);
  if (name==NULL) {
    ret+=_("Unknown");
  } else if (name[0]==0) {
    ret+=_("Unknown");
  } else {
    ret+=name;
  }
  return ret;
}

double sinus(double x) {
  return sin(x);
}
double rectSin(double x) {
  return sin(x) > 0 ? sin(x) : 0;
}
double absSin(double x) {
  return fabs(sin(x));
}

double square(double x) {
  return fmod(x, (2 * M_PI)) >= M_PI ? -1 : 1;
}
double rectSquare(double x) {
  return square(x) > 0 ? square(x) : 0;
}

double quartSin(double x) {
  return absSin(x) * rectSquare(2 * x);
}
double squiSin(double x) {
  return sin(x) >= 0 ? sin(2 * x) : 0;
}
double squiAbsSin(double x) {
  return fabs(squiSin(x));
}

double saw(double x) {
  return atan(tan(x / 2)) / (M_PI / 2);
}
double rectSaw(double x) {
  return saw(x) > 0 ? saw(x) : 0;
}
double absSaw(double x) {
  return saw(x) < 0 ? saw(x) + 1 : saw(x);
}


double cubSaw(double x) {
  return pow(saw(x), 3);
}
double rectCubSaw(double x) {
  return pow(rectSaw(x), 3);
}
double absCubSaw(double x) {
  return pow(absSaw(x), 3);
}

double cubSine(double x) {
  return pow(sin(x), 3);
}
double rectCubSin(double x) {
  return pow(rectSin(x), 3);
}
double absCubSin(double x) {
  return pow(absSin(x), 3);
}
double quartCubSin(double x) {
  return pow(quartSin(x), 3);
}
double squishCubSin(double x) {
  return pow(squiSin(x), 3);
}
double squishAbsCubSin(double x) {
  return pow(squiAbsSin(x), 3);
}

double triangle(double x) {
  return asin(sin(x)) / (M_PI / 2);
}
double rectTri(double x) {
  return triangle(x) > 0 ? triangle(x) : 0;
}
double absTri(double x) {
  return fabs(triangle(x));
}
double quartTri(double x) {
  return absTri(x) * rectSquare(2 * x);
}
double squiTri(double x) {
  return sin(x) >= 0 ? triangle(2 * x) : 0;
}
double absSquiTri(double x) {
  return fabs(squiTri(x));
}

double cubTriangle(double x) {
  return pow(triangle(x), 3);
}
double cubRectTri(double x) {
  return pow(rectTri(x), 3);
}
double cubAbsTri(double x) {
  return pow(absTri(x), 3);
}
double cubQuartTri(double x) {
  return pow(quartTri(x), 3);
}
double cubSquiTri(double x) {
  return pow(squiTri(x), 3);
}
double absCubSquiTri(double x) {
  return fabs(cubSquiTri(x));
}

String getMultiKeysName(const int* keys, int keyCount, bool emptyNone) {
  String ret;
  for (int i=0; i<keyCount; i++) {
    if (keys[i]==0) continue;
    if (!ret.empty()) ret+=", ";
    ret+=getKeyName(keys[i]);
  }

  if (ret.empty()) {
    if (emptyNone) {
      return "";
    } else {
      return _("<nothing>");
    }
  } else {
    return ret;
  }
}
