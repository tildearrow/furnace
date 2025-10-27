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

#ifndef _FILEPLAYER_H
#define _FILEPLAYER_H

#include <mutex>

#ifdef HAVE_SNDFILE
#include "sfWrapper.h"
#else
typedef void SNDFILE;
struct SF_INFO {
  int invalid;
};
#endif

class DivFilePlayer {
  float* discardBuf;
  float** blocks;
  size_t numBlocks;
  String lastError;
  SFWrapper sfw;
  SNDFILE* sf;
  SF_INFO si;

  size_t playPos;
  int outRate;
  int rateAccum;
  float volume;
  bool playing;
  bool fileError;

  std::mutex cacheMutex;

  void fillBlocksNear(size_t pos);

  public:
    void mix(float** buf, int chans, unsigned int size);
    size_t getPos();
    size_t setPos(size_t newPos, unsigned int offset=0);
    
    bool isPlaying();
    void play(unsigned int offset=0);
    void stop(unsigned int offset=0);
    bool closeFile();
    bool loadFile(const char* path);

    String getLastError();
    void setOutputRate(int rate);
    float getVolume();
    void setVolume(float vol);

    DivFilePlayer();
    ~DivFilePlayer();
};
