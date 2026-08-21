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

#ifndef _FILEPLAYER_H
#define _FILEPLAYER_H

#include "../ta-utils.h"
#include "../timeutils.h"
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

  int64_t playPos;
  int64_t lastWantBlock;
  int64_t wantBlock;
  int outRate;
  int rateAccum;
  float volume;
  bool playing;
  bool fileError;
  bool quitThread;
  bool threadHasQuit;
  bool isActive;

  int64_t pendingPos;
  unsigned int pendingPosOffset;
  unsigned int pendingPlayOffset;
  unsigned int pendingStopOffset;

  std::thread* cacheThread;
  std::mutex cacheMutex;
  std::mutex cacheThreadLock;
  std::condition_variable cacheCV;

  void fillBlocksNear(int64_t pos);
  void collectGarbage(int64_t pos);
  float getSampleAt(int64_t pos, int ch);

  public:
    void runCacheThread();

    size_t getMemUsage();

    void mix(float** buf, int chans, unsigned int size);
    int64_t getPos();
    TimeMicros getPosSeconds();
    int64_t setPos(int64_t newPos, unsigned int offset=UINT_MAX);
    int64_t setPosSeconds(TimeMicros newTime, unsigned int offset=UINT_MAX);
    
    bool isBlockPresent(int64_t pos);
    bool setBlockPriority(int64_t pos, bool priority);
    bool isLoaded();
    bool isPlaying();
    void play(unsigned int offset=UINT_MAX);
    void stop(unsigned int offset=UINT_MAX);
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
