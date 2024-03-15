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

#include "fileOpsCommon.h"

bool DivEngine::load(unsigned char* f, size_t slen) {
  unsigned char* file;
  size_t len;
  if (slen<18) {
    logE("too small!");
    lastError="file is too small";
    delete[] f;
    return false;
  }

  if (!systemsRegistered) registerSystems();

  // step 1: try loading as a zlib-compressed file
  logD("trying zlib...");
  try {
    z_stream zl;
    memset(&zl,0,sizeof(z_stream));

    zl.avail_in=slen;
    zl.next_in=(Bytef*)f;
    zl.zalloc=NULL;
    zl.zfree=NULL;
    zl.opaque=NULL;

    int nextErr;
    nextErr=inflateInit(&zl);
    if (nextErr!=Z_OK) {
      if (zl.msg==NULL) {
        logD("zlib error: unknown! %d",nextErr);
      } else {
        logD("zlib error: %s",zl.msg);
      }
      inflateEnd(&zl);
      lastError="not a .dmf/.fur song";
      throw NotZlibException(0);
    }

    std::vector<InflateBlock*> blocks;
    while (true) {
      InflateBlock* ib=new InflateBlock(DIV_READ_SIZE);
      zl.next_out=ib->buf;
      zl.avail_out=ib->len;

      nextErr=inflate(&zl,Z_SYNC_FLUSH);
      if (nextErr!=Z_OK && nextErr!=Z_STREAM_END) {
        if (zl.msg==NULL) {
          logD("zlib error: unknown error! %d",nextErr);
          lastError="unknown decompression error";
        } else {
          logD("zlib inflate: %s",zl.msg);
          lastError=fmt::sprintf("decompression error: %s",zl.msg);
        }
        for (InflateBlock* i: blocks) delete i;
        blocks.clear();
        delete ib;
        inflateEnd(&zl);
        throw NotZlibException(0);
      }
      ib->blockSize=ib->len-zl.avail_out;
      blocks.push_back(ib);
      if (nextErr==Z_STREAM_END) {
        break;
      }
    }
    nextErr=inflateEnd(&zl);
    if (nextErr!=Z_OK) {
      if (zl.msg==NULL) {
        logD("zlib end error: unknown error! %d",nextErr);
        lastError="unknown decompression finish error";
      } else {
        logD("zlib end: %s",zl.msg);
        lastError=fmt::sprintf("decompression finish error: %s",zl.msg);
      }
      for (InflateBlock* i: blocks) delete i;
      blocks.clear();
      throw NotZlibException(0);
    }

    size_t finalSize=0;
    size_t curSeek=0;
    for (InflateBlock* i: blocks) {
      finalSize+=i->blockSize;
    }
    if (finalSize<1) {
      logD("compressed too small!");
      lastError="file too small";
      for (InflateBlock* i: blocks) delete i;
      blocks.clear();
      throw NotZlibException(0);
    }
    file=new unsigned char[finalSize];
    for (InflateBlock* i: blocks) {
      memcpy(&file[curSeek],i->buf,i->blockSize);
      curSeek+=i->blockSize;
      delete i;
    }
    blocks.clear();
    len=finalSize;
    delete[] f;
  } catch (NotZlibException& e) {
    logD("not zlib. loading as raw...");
    file=f;
    len=slen;
  }

  // step 2: try loading as .fur or .dmf
  if (memcmp(file,DIV_DMF_MAGIC,16)==0) {
    return loadDMF(file,len); 
  } else if (memcmp(file,DIV_FTM_MAGIC,18)==0) {
    return loadFTM(file,len);
  } else if (memcmp(file,DIV_FUR_MAGIC,16)==0) {
    return loadFur(file,len);
  } else if (memcmp(file,DIV_FUR_MAGIC_DS0,16)==0) {
    return loadFur(file,len,DIV_FUR_VARIANT_B);
  } else if (memcmp(file,DIV_FC13_MAGIC,4)==0 || memcmp(file,DIV_FC14_MAGIC,4)==0) {
    return loadFC(file,len);
  }

  // step 3: try loading as .mod
  if (loadMod(file,len)) {
    delete[] f;
    return true;
  }
  
  // step 4: not a valid file
  logE("not a valid module!");
  lastError="not a compatible song";
  delete[] file;
  return false;
}
