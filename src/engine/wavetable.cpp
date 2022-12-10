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
    logV("header is invalid: %c%c%c%c",magic[0],magic[1],magic[2],magic[3]);
    return DIV_DATA_INVALID_HEADER;
  }
  reader.readI(); // reserved

  reader.readString(); // ignored for now
  len=reader.readI();
  min=reader.readI();
  max=reader.readI();

  if (len>256) {
    logE("invalid len: %d",len);
    return DIV_DATA_INVALID_DATA;
  }

  if (min!=0) {
    logW("invalid min %d",min);
    min=0;
  }

  if (max>255) {
    logW("invalid max %d",max);
    max=255;
  }

  for (int i=0; i<len; i++) {
    data[i]=reader.readI();
  }

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

bool DivWavetable::saveDMW(const char* path) {
  SafeWriter* w=new SafeWriter();
  w->init();

  // write width
  w->writeI(len);

  // check height
  w->writeC(max);
  if (max==255) {
    // write as new format (because 0xff means that)
    w->writeC(1); // format version
    w->writeC(max); // actual height

    // waveform data
    for (int i=0; i<len; i++) {
      w->writeI(data[i]&0xff);
    }
  } else {
    // write as old format
    for (int i=0; i<len; i++) {
      w->writeC(data[i]);
    }
  }

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

bool DivWavetable::saveRaw(const char* path) {
  SafeWriter* w=new SafeWriter();
  w->init();

  // waveform data
  for (int i=0; i<len; i++) {
    w->writeC(data[i]);
  }

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
