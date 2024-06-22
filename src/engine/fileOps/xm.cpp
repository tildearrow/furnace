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

bool DivEngine::loadXM(unsigned char* file, size_t len) {
  struct InvalidHeaderException {};
  bool success=false;
  char magic[32];
  SafeReader reader=SafeReader(file,len);
  warnings="";

  try {
    DivSong ds;
    ds.version=DIV_VERSION_XM;
    //ds.linearPitch=0;
    //ds.pitchMacroIsLinear=false;
    ds.noSlidesOnFirstTick=true;
    ds.rowResetsArpPos=true;
    ds.ignoreJumpAtEnd=false;
    ds.pitchSlideSpeed=12;

    logV("Extended Module");

    // load here
    if (!reader.seek(0,SEEK_SET)) {
      logE("premature end of file!");
      lastError="incomplete file";
      delete[] file;
      return false;
    }
    reader.read(magic,17);

    if (memcmp(magic,DIV_XM_MAGIC,17)!=0) {
      logW("invalid magic");
      throw EndOfFileException(&reader,reader.tell());
    }

    ds.name=reader.readString(20);

    // 0x1a
    reader.readC();

    if (active) quitDispatch();
    BUSY_BEGIN_SOFT;
    saveLock.lock();
    song.unload();
    song=ds;
    changeSong(0);
    recalcChans();
    saveLock.unlock();
    BUSY_END;
    if (active) {
      initDispatch();
      BUSY_BEGIN;
      renderSamples();
      reset();
      BUSY_END;
    }
    success=true;
  } catch (EndOfFileException& e) {
    //logE("premature end of file!");
    lastError="incomplete file";
  } catch (InvalidHeaderException& e) {
    //logE("invalid header!");
    lastError="invalid header!";
  }
  return success;
}

