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
      logI("creating config dir...\n");
      int mkdirRet;
      if ((mkdirRet=SHCreateDirectory(NULL,configPath.c_str()))!=ERROR_SUCCESS) {
        logW("could not make config dir! (%.8x)\n",mkdirRet);
        configPath=L".";
      }
    }
  } else {
    logW("unable to determine config directory! (%.8x)\n",configHR);
    configPath=L".";
  }
  return utf16To8(configPath.c_str());
}
