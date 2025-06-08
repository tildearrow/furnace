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

#include "gui.h"
#include <zlib.h>

#define FONT_READ_SIZE 262144

struct InflateBlock {
  unsigned char* buf;
  size_t len;
  size_t blockSize;
  InflateBlock(size_t s) {
    buf=new unsigned char[s];
    len=s;
    blockSize=0;
  }
  ~InflateBlock() {
    delete[] buf;
    len=0;
  }
};

ImFont* FurnaceGUI::addFontZlib(const void* data, size_t len, float size_pixels, const ImFontConfig* font_cfg, const ImWchar* glyph_ranges) {
  z_stream zl;
  memset(&zl,0,sizeof(z_stream));
  logV("addFontZlib...");

  zl.avail_in=len;
  zl.next_in=(Bytef*)data;
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
    return NULL;
  }

  std::vector<InflateBlock*> blocks;
  while (true) {
    InflateBlock* ib=new InflateBlock(FONT_READ_SIZE);
    zl.next_out=ib->buf;
    zl.avail_out=ib->len;

    nextErr=inflate(&zl,Z_SYNC_FLUSH);
    if (nextErr!=Z_OK && nextErr!=Z_STREAM_END) {
      if (zl.msg==NULL) {
        logD("zlib error: unknown error! %d",nextErr);
      } else {
        logD("zlib inflate: %s",zl.msg);
      }
      for (InflateBlock* i: blocks) delete i;
      blocks.clear();
      delete ib;
      inflateEnd(&zl);
      return NULL;
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
    } else {
      logD("zlib end: %s",zl.msg);
    }
    for (InflateBlock* i: blocks) delete i;
    blocks.clear();
    return NULL;
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
    return NULL;
  }
  unsigned char* finalData=(unsigned char*)malloc(finalSize);
  for (InflateBlock* i: blocks) {
    memcpy(&finalData[curSeek],i->buf,i->blockSize);
    curSeek+=i->blockSize;
    delete i;
  }
  blocks.clear();
  len=finalSize;

  ImFontConfig fontConfig=(font_cfg==NULL)?ImFontConfig():(*font_cfg);
  fontConfig.FontDataOwnedByAtlas=true;

  return ImGui::GetIO().Fonts->AddFontFromMemoryTTF(finalData,finalSize,size_pixels,&fontConfig,glyph_ranges);
}
