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

#include <string.h>
#include "scaling.h"
#include "../ta-log.h"
#include <SDL.h>

#ifdef _WIN32
#include <windows.h>
typedef HRESULT (*GDFM)(HMONITOR,int,UINT*,UINT*);
#endif

#ifdef __APPLE__
extern "C" {
#include "macstuff.h"
}
#endif

#if defined(__unix__) || defined(ANDROID)
#include <dlfcn.h>
typedef void* (*XOD)(const char*);
typedef int (*XCD)(void*);
typedef int (*XDS)(void*);
typedef int (*XDW)(void*,int);
#endif

double getScaleFactor(const char* driverHint) {
  double ret=1.0;

  // Windows
#ifdef _WIN32
  POINT nullPoint;
  nullPoint.x=-1;
  nullPoint.y=-1;
  HMONITOR disp=MonitorFromPoint(nullPoint,MONITOR_DEFAULTTOPRIMARY);

  if (disp==NULL) {
    logW("could not find a monitor - no scaling detection available!");
    return 1.0;
  }

  HMODULE shcore=LoadLibraryW(L"shcore.dll");
  if (shcore==NULL) {
    logW("could not find shcore.dll (%.8x) - no scaling detection available!",GetLastError());
    return 1.0;
  }
  GDFM ta_GetDpiForMonitor=(GDFM)GetProcAddress(shcore,"GetDpiForMonitor");
  if (ta_GetDpiForMonitor==NULL) {
    logW("GetDpiForMonitor not found (%.8x) - no scaling detection available!",GetLastError());

    if (!FreeLibrary(shcore)) {
      logE("could not free shcore.dll (%.8x)!",GetLastError());
    }
    return 1.0;
  }

  unsigned int dpiX=96;
  unsigned int dpiY=96;
  HRESULT result=ta_GetDpiForMonitor(disp,0,&dpiX,&dpiY);
  if (result!=S_OK) {
    logW("GetDpiForMonitor failure (%.8x) - no scaling detection available!",result);

    if (!FreeLibrary(shcore)) {
      logE("could not free shcore.dll (%.8x)!",GetLastError());
    }
    return 1.0;
  }

  ret=(double)(dpiX+dpiY)/192.0;

  if (!FreeLibrary(shcore)) {
    logE("could not free shcore.dll (%.8x)!",GetLastError());
  }

  return ret;
#endif

  // macOS - backingScaleFactor
#ifdef __APPLE__
  if (driverHint==NULL) {
    return getMacDPIScale();
  } else if (strcmp(driverHint,"cocoa")==0 || strcmp(driverHint,"uikit")==0) {
    return getMacDPIScale();
  }
#endif

#if defined(__unix__) || defined(ANDROID)
  if (driverHint==NULL) {
    return ret;
  }

  // X11  
  if (strcmp(driverHint,"x11")==0) {
    void* libX11=dlopen("libX11.so",RTLD_LAZY|RTLD_LOCAL);
    if (libX11==NULL) {
      logW("could not load libX11.so (%s) - no scaling detection available!",dlerror());
      return 1.0;
    }

    XOD ta_XOpenDisplay=(XOD)dlsym(libX11,"XOpenDisplay");
    if (ta_XOpenDisplay==NULL) {
      logW("XOpenDisplay not found (%s) - no scaling detection available!",dlerror());
      if (dlclose(libX11)!=0) {
        logE("could not free libX11.so (%s)!",dlerror());
      }
      return 1.0;
    }

    XCD ta_XCloseDisplay=(XCD)dlsym(libX11,"XCloseDisplay");
    if (ta_XCloseDisplay==NULL) {
      logW("XCloseDisplay not found (%s) - no scaling detection available!",dlerror());
      if (dlclose(libX11)!=0) {
        logE("could not free libX11.so (%s)!",dlerror());
      }
      return 1.0;
    }

    XDS ta_XDefaultScreen=(XDS)dlsym(libX11,"XDefaultScreen");
    if (ta_XDefaultScreen==NULL) {
      logW("XDefaultScreen not found (%s) - no scaling detection available!",dlerror());
      if (dlclose(libX11)!=0) {
        logE("could not free libX11.so (%s)!",dlerror());
      }
      return 1.0;
    }

    XDW ta_XDisplayWidth=(XDW)dlsym(libX11,"XDisplayWidth");
    if (ta_XDisplayWidth==NULL) {
      logW("XDisplayWidth not found (%s) - no scaling detection available!",dlerror());
      if (dlclose(libX11)!=0) {
        logE("could not free libX11.so (%s)!",dlerror());
      }
      return 1.0;
    }

    XDW ta_XDisplayWidthMM=(XDW)dlsym(libX11,"XDisplayWidthMM");
    if (ta_XDisplayWidthMM==NULL) {
      logW("XDisplayWidthMM not found (%s) - no scaling detection available!",dlerror());
      if (dlclose(libX11)!=0) {
        logE("could not free libX11.so (%s)!",dlerror());
      }
      return 1.0;
    }

    // dl mess
    void* disp=NULL;
    int screen=0;
    int dpi=96;

    disp=ta_XOpenDisplay(NULL);
    if (disp==NULL) {
      logW("couldn't open X display - no scaling detection available!",dlerror());
      if (dlclose(libX11)!=0) {
        logE("could not free libX11.so (%s)!",dlerror());
      }
      return 1.0;
    }

    screen=ta_XDefaultScreen(disp);

    dpi=(int)(0.5+(25.4*(double)ta_XDisplayWidth(disp,screen)/(double)ta_XDisplayWidthMM(disp,screen)));

    ta_XCloseDisplay(disp);

    ret=round(dpi/96.0);

    if (dlclose(libX11)!=0) {
      logE("could not free libX11.so (%s)!",dlerror());
    }

    return ret;
  }

  // Wayland
  if (strcmp(driverHint,"wayland")==0) {
    // give up (we handle scaling factor detection after window creation)
    return 1.0;
  }
#endif

  // SDL fallback
#ifdef ANDROID
  float dpiScaleF=160.0f;
  if (SDL_GetDisplayDPI(0,&dpiScaleF,NULL,NULL)==0) {
    ret=dpiScaleF/160.0f;
    if (ret<1) ret=1;
    logI("dpiScaleF: %f",dpiScaleF);
    logI("ret: %f",ret);
  }

#else
  float dpiScaleF=96.0f;
  if (SDL_GetDisplayDPI(0,&dpiScaleF,NULL,NULL)==0) {
    ret=round(dpiScaleF/96.0f);
    if (ret<1) ret=1;
  }
#endif

  // couldn't detect scaling factor :<
  return ret;
}
