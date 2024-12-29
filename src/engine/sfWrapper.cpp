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

#include "sfWrapper.h"
#include "../fileutils.h"
#include "../ta-log.h"
#include "sndfile.h"
#include <fcntl.h>
#include <unistd.h>

sf_count_t _vioGetSize(void* user) {
  return ((SFWrapper*)user)->ioGetSize();
}

sf_count_t _vioSeek(sf_count_t offset, int whence, void* user) {
  return ((SFWrapper*)user)->ioSeek(offset,whence);
}

sf_count_t _vioRead(void* ptr, sf_count_t count, void* user) {
  return ((SFWrapper*)user)->ioRead(ptr,count);
}

sf_count_t _vioWrite(const void* ptr, sf_count_t count, void* user) {
  return ((SFWrapper*)user)->ioWrite(ptr,count);
}

sf_count_t _vioTell(void* user) {
  return ((SFWrapper*)user)->ioTell();
}

sf_count_t SFWrapper::ioGetSize() {
  sf_count_t ret=(sf_count_t)len;
  if (fileMode==SFM_WRITE || fileMode==SFM_RDWR) {
    ssize_t lastTell=ftell(fp);
    fseek(fp,0,SEEK_END);
    ret=(sf_count_t)ftell(fp);
    fseek(fp,lastTell,SEEK_SET);
  }
  return ret;
}

sf_count_t SFWrapper::ioSeek(sf_count_t offset, int whence) {
  return fseek(fp,offset,whence);
}

sf_count_t SFWrapper::ioRead(void* ptr, sf_count_t count) {
  return fread(ptr,1,count,fp);
}

sf_count_t SFWrapper::ioWrite(const void* ptr, sf_count_t count) {
  return fwrite(ptr,1,count,fp);
}

sf_count_t SFWrapper::ioTell() {
  return ftell(fp);
}

int SFWrapper::doClose() {
  int ret=sf_close(sf);
  fclose(fp);
  fd=-1;
  return ret;
}

void SFWrapper::initVio() {
  vio.get_filelen=_vioGetSize;
  vio.read=_vioRead;
  vio.seek=_vioSeek;
  vio.tell=_vioTell;
  vio.write=_vioWrite;
}

SNDFILE* SFWrapper::doOpen(const char* path, int mode, SF_INFO* sfinfo) {
  initVio();
  logV("SFWrapper: opening %s",path);

  const char* modeC="rb";
  if (mode==SFM_WRITE) {
    modeC="wb";
  }
  if (mode==SFM_RDWR) {
    modeC="rb+";
  }

  fp=ps_fopen(path,modeC);
  if (fp==NULL) {
    logE("SFWrapper: failed to open (%s)",strerror(errno));
    return NULL;
  }

  fd=fileno(fp);
  if (fd==-1) {
    logE("SFWrapper: failed to get file descriptor (%s)",strerror(errno));
    return NULL;
  }

  if (fseek(fp,0,SEEK_END)==-1) {
    logE("SFWrapper: failed to seek to end (%s)",strerror(errno));
    fclose(fp);
    fp=NULL;
    return NULL;
  }

  len=ftell(fp);
  if (len==(SIZE_MAX>>1)) {
    logE("SFWrapper: failed to tell (%s)",strerror(errno));
    len=0;
    fclose(fp);
    fp=NULL;
    return NULL;
  }

  if (fseek(fp,0,SEEK_SET)==-1) {
    logE("SFWrapper: failed to seek to beginning (%s)",strerror(errno));
    len=0;
    fclose(fp);
    fp=NULL;
    return NULL;
  }

  sf=sf_open_virtual(&vio,mode,sfinfo,this);
  if (sf!=NULL) fileMode=mode;
  if (sf==NULL) {
    logE("SFWrapper: WHY IS IT NULL?!");
  }
  return sf;
}

SNDFILE* SFWrapper::doOpenFromWriteFd(int writeFd, SF_INFO* sfinfo) {
  fp=fdopen(writeFd,"w");
  if (fp==NULL) {
    logE("SFWrapper: failed to open file descriptor %d (pipe) as file: %s",writeFd,strerror(errno));
    return NULL;
  }
  fd=writeFd;

  initVio();
  len=0; // I am hoping this is not used when writing
  fileMode=SFM_WRITE;

  sf=sf_open_virtual(&vio,SFM_WRITE,sfinfo,this);
  if (sf==NULL) {
    logE("SFWrapper: WHY IS IT NULL?!");
  }
  return sf;
}
