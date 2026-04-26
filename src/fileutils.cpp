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

#include "fileutils.h"
#ifdef _WIN32
#include "utfutils.h"
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#endif

FILE* ps_fopen(const char* path, const char* mode) {
#ifdef _WIN32
  return _wfopen(utf8To16(path).c_str(),utf8To16(mode).c_str());
#else
  return fopen(path,mode);
#endif
}

// TODO: copy in case of failure
bool moveFiles(const char* src, const char* dest) {
#ifdef _WIN32
  return MoveFileW(utf8To16(src).c_str(),utf8To16(dest).c_str());
#else
  if (rename(src,dest)==-1) {
    return false;
  }
  return true;
#endif
}

bool copyFiles(const char* src, const char* dest) {
  FILE* f=ps_fopen(src,"rb");
  if (f==NULL) return false;
  
  FILE* of=ps_fopen(dest,"wb");
  if (of==NULL) {
    fclose(f);
    return false;
  }

  char block[2048];
  while (!feof(f)) {
    size_t readTotal=fread(block,1,2048,f);
    if (readTotal) fwrite(block,1,readTotal,of);
  }

  fclose(of);
  fclose(f);
  return true;
}

bool deleteFile(const char* path) {
#ifdef _WIN32
  return DeleteFileW(utf8To16(path).c_str());
#else
  return (unlink(path)==0);
#endif
}

int fileExists(const char* path) {
#ifdef _WIN32
  if (PathFileExistsW(utf8To16(path).c_str())) return 1;
  // which errors could PathFileExists possibly throw?
  switch (GetLastError()) {
    case ERROR_FILE_EXISTS:
      return 1;
      break;
    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND:
    case ERROR_INVALID_DRIVE:
    case ERROR_DEV_NOT_EXIST:
    case ERROR_NETNAME_DELETED:
    case ERROR_BAD_NET_NAME:
      return 0;
      break;
  }
  return -1;
#else
  if (access(path,F_OK)==0) return 1;
  if (errno==ENOENT) return 0;
  return -1;
#endif
}

bool dirExists(const char* what) {
#ifdef _WIN32
  WString ws=utf8To16(what);
  return (PathIsDirectoryW(ws.c_str())!=FALSE);
#else
  struct stat st;
  if (stat(what,&st)<0) return false;
  return (st.st_mode&S_IFDIR);
#endif
}

bool makeDir(const char* path) {
#ifdef _WIN32
  return (SHCreateDirectoryExW(NULL,utf8To16(path).c_str(),NULL)==ERROR_SUCCESS);
#else
  return (mkdir(path,0755)==0);
#endif
}

int touchFile(const char* path) {
#ifdef _WIN32
  HANDLE h=CreateFileW(utf8To16(path).c_str(),GENERIC_WRITE,FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,CREATE_NEW,FILE_ATTRIBUTE_TEMPORARY,NULL);
  if (h==INVALID_HANDLE_VALUE) {
    switch (GetLastError()) {
      case ERROR_FILE_EXISTS:
        return -EEXIST;
        break;
    }
    return -EPERM;
  }
  if (CloseHandle(h)==0) {
    return -EBADF;
  }
  return 0;
#else
  int fd=open(path,O_CREAT|O_WRONLY|O_TRUNC|O_EXCL,0666);
  if (fd<0) return -errno;
  close(fd);
  return 0;
#endif
}
