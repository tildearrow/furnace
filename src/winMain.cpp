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

#include "utfutils.h"

typedef HRESULT (*SPDA)(int);

int WINAPI WinMain(HINSTANCE inst, HINSTANCE prevInst, PSTR args, int state) {
  if (AttachConsole(ATTACH_PARENT_PROCESS)==0) {
    if (GetLastError()==ERROR_ACCESS_DENIED) FreeConsole();
  }

  int argc=0;
  wchar_t** argw=CommandLineToArgvW(GetCommandLineW(),&argc);
  char** argv=new char*[argc+1];
  argv[argc]=NULL;
  for (int i=0; i<argc; i++) {
    std::string str=utf16To8(argw[i]);
    argv[i]=new char[str.size()+1];
    strcpy(argv[i],str.c_str());
  }

  // set DPI awareness
  HMODULE shcore=LoadLibraryW(L"shcore.dll");
  if (shcore!=NULL) {
    SPDA ta_SetProcessDpiAwareness=(SPDA)GetProcAddress(shcore,"SetProcessDpiAwareness");
    if (ta_SetProcessDpiAwareness!=NULL) {
      HRESULT result=ta_SetProcessDpiAwareness(2);
      if (result!=S_OK) {
        // ???
      }
    }
    if (!FreeLibrary(shcore)) {
      // ???
    }
  }
  
  return main(argc,argv);
}
