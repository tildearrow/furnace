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

#include "dataErrors.h"
#include "engine.h"
#include "wavetable.h"
#include "../ta-log.h"
#include "../fileutils.h"

void DivWavetable::putWaveData(SafeWriter* w) {
  size_t blockStartSeek, blockEndSeek;

  w->write("WAVE",4);
  blockStartSeek=w->tell();
  w->writeI(0);

  w->writeC(0); // name
  w->writeI(len);
  w->writeI(min);
  w->writeI(max);
  for (int j=0; j<len; j++) {
    w->writeI(data[j]);
  }

  blockEndSeek=w->tell();
  w->seek(blockStartSeek,SEEK_SET);
  w->writeI(blockEndSeek-blockStartSeek-4);
  w->seek(0,SEEK_END);
}

DivDataErrors DivWavetable::readWaveData(SafeReader& reader, short version) {
  char magic[4];
  reader.read(magic,4);
  if (memcmp(magic,"WAVE",4)!=0) {
    return DIV_DATA_INVALID_HEADER;
  }
  reader.readI(); // reserved

  reader.readString(); // ignored for now
  len=reader.readI();
  min=reader.readI();
  max=reader.readI();

  if (len>256 || min!=0 || max>255) return DIV_DATA_INVALID_DATA;

  reader.read(data,4*len);

  return DIV_DATA_SUCCESS;
}

bool DivWavetable::save(const char* path) {
  SafeWriter* w=new SafeWriter();
  w->init();

  // write magic
  w->write("-Furnace waveta-",16);

  // write version
  w->writeS(DIV_ENGINE_VERSION);

  // reserved
  w->writeS(0);

  putWaveData(w);

  FILE* outFile=ps_fopen(path,"wb");
  if (outFile==NULL) {
    logE("could not save wavetable: %s!",strerror(errno));
    w->finish();
    return false;
  }
  if (fwrite(w->getFinalBuf(),1,w->size(),outFile)!=w->size()) {
    logW("did not write entire wavetable!");
  }
  fclose(outFile);
  w->finish();
  return true;
}
