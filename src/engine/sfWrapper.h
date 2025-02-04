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

// sfWrapper.h: libsndfile FILE* wrapper to work around Windows issue with
//              non-ASCII chars in file path
//              I wanted to do this in sndfile directly, but it's a
//              submodule...

#ifndef _SFWRAPPER_H
#define _SFWRAPPER_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sndfile.h>
#include "../ta-utils.h"

class SFWrapper {
  FILE* f;
  size_t len;
  SF_VIRTUAL_IO vio;
  SNDFILE* sf;
  int fileMode;

  public:
    sf_count_t ioGetSize();
    sf_count_t ioSeek(sf_count_t offset, int whence);
    sf_count_t ioRead(void* ptr, sf_count_t count);
    sf_count_t ioWrite(const void* ptr, sf_count_t count);
    sf_count_t ioTell();

    int doClose();
    SNDFILE* doOpen(const char* path, int mode, SF_INFO* sfinfo);
    SFWrapper():
      f(NULL),
      len(0),
      sf(NULL),
      fileMode(0) {}
};

#endif
