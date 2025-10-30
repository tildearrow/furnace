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

#include "../ta-utils.h"
#include <thread>
#include <mutex>
#include <condition_variable>

#ifdef HAVE_SNDFILE
#include "sfWrapper.h"
#else
typedef void SNDFILE;
struct SF_INFO {
  int invalid;
};
#endif

class DivFilePlayer {
  float* sincTable;
  float* discardBuf;
  float** blocks;
  bool* priorityBlock;
  size_t numBlocks;
  String lastError;
  SFWrapper sfw;
  SNDFILE* sf;
  SF_INFO si;

  ssize_t playPos;
  ssize_t lastWantBlock;
  ssize_t wantBlock;
  int outRate;
  int rateAccum;
  float volume;
  bool playing;
  bool fileError;
  bool quitThread;
  bool threadHasQuit;
  bool isActive;

  std::thread* cacheThread;
  std::mutex cacheMutex;
  std::mutex cacheThreadLock;
  std::condition_variable cacheCV;

  void fillBlocksNear(ssize_t pos);
  void collectGarbage(ssize_t pos);
  float getSampleAt(ssize_t pos, int ch);

  public:
    void runCacheThread();

    size_t getMemUsage();

    void mix(float** buf, int chans, unsigned int size);
    ssize_t getPos();
    ssize_t setPos(ssize_t newPos, unsigned int offset=0);
    ssize_t setPosSeconds(ssize_t seconds, unsigned int micros, unsigned int offset=0);
    
    bool isBlockPresent(ssize_t pos);
    bool setBlockPriority(ssize_t pos, bool priority);
    bool isLoaded();
    bool isPlaying();
    void play(unsigned int offset=0);
    void stop(unsigned int offset=0);
    bool closeFile();
    bool loadFile(const char* path);

    String getLastError();
    const SF_INFO& getFileInfo();
    void setOutputRate(int rate);
    float getVolume();
    void setVolume(float vol);
    bool getActive();
    void setActive(bool active);

    DivFilePlayer();
    ~DivFilePlayer();
};

#endif
