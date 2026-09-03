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

#include "../fileutils.h"

// the only implementation for now
class TAFilePOSIX: public TAFile {
  FILE* handle;
  public:
    size_t read(void* ptr, size_t count) {
      const size_t ret=fread(ptr,1,count,handle);
      if (ret==0) {
        if (ferror(handle)) {
          throw errno;
        }
      }
      return ret;
    }

    size_t write(void* ptr, size_t count) {
      const size_t ret=fwrite(ptr,1,count,handle);
      if (ret==0) {
        if (ferror(handle)) {
          throw errno;
        }
      }
      return ret;
    }

    void seek(long off, int whence) {
      if (!fseek(handle,off,whence)) throw errno;
    }

    size_t tell() {
      const long ret=ftell(handle);
      if (ret==-1) throw errno;
      return ret;
    }

    bool eof() {
      return feof(handle);
    }

    void* getNativeHandle() {
      return handle;
    }

    TAFilePOSIX(FILE* f):
      handle(f) {}
    ~TAFilePOSIX() {
      fclose(handle);
      handle=NULL;
    }
};

TAFile* taOpenFile(const char* path, TAFileModes mode) {
  char modeStr[8];
  unsigned char modeStrPos=0;
  memset(modeStr,0,8);

  if (mode&TA_FILE_READ) {
    modeStr[modeStrPos++]='r';
  }
  if (mode&TA_FILE_WRITE) {
    modeStr[modeStrPos++]='w';
  }
  if (mode&TA_FILE_APPEND) {
    modeStr[modeStrPos++]='a';
  }
  if (mode&TA_FILE_PRESERVE) {
    modeStr[modeStrPos++]='+';
  }

  if (mode&TA_FILE_BINARY) {
    modeStr[modeStrPos++]='b';
  }

  if (modeStr[0]==0 || modeStr[0]=='b') throw EINVAL;

  FILE* f=ps_fopen(path,modeStr);
  if (f==NULL) throw errno;

  return new TAFilePOSIX(f);
}