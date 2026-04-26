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

#include "filePlayer.h"
#include "filter.h"
#include "../ta-log.h"
#include <inttypes.h>
#include <chrono>

#define DIV_FPCACHE_BLOCK_SHIFT 15
#define DIV_FPCACHE_BLOCK_SIZE (1<<DIV_FPCACHE_BLOCK_SHIFT)
#define DIV_FPCACHE_BLOCK_MASK (DIV_FPCACHE_BLOCK_SIZE-1)

#define DIV_FPCACHE_BLOCKS_FROM_FILL 3
#define DIV_FPCACHE_DISCARD_SIZE 4096

// 5MB should be enough
#define DIV_MAX_MEMORY (5<<20)

#define DIV_NO_BLOCK (-10)

void DivFilePlayer::fillBlocksNear(ssize_t pos) {
  logV("DivFilePlayer: fillBlocksNear(%" PRIu64 ")",pos);

  // don't if file isn't present
  if (!blocks) return;

  // don't if there was an I/O error
  if (fileError) return;

  // don't read anything if we cannot seek
  // (if this is set the file is already read in its entirety)
  if (!si.seekable) return;

  ssize_t firstBlock=pos>>DIV_FPCACHE_BLOCK_SHIFT;
  ssize_t lastBlock=firstBlock+DIV_FPCACHE_BLOCKS_FROM_FILL;
  if (firstBlock<0) firstBlock=0;
  if (firstBlock>=(ssize_t)numBlocks) firstBlock=numBlocks-1;
  if (lastBlock<0) lastBlock=0;
  if (lastBlock>=(ssize_t)numBlocks) lastBlock=numBlocks-1;

  // don't read if we're after end of file
  if (firstBlock>lastBlock) return;
  
  bool needToFill=false;
  for (ssize_t i=firstBlock; i<=lastBlock; i++) {
    if (i<0 || i>=(ssize_t)numBlocks) continue;
    if (!blocks[i]) {
      needToFill=true;
      firstBlock=i;
      break;
    }
  }

  if (!needToFill) return;

  // check whether we need to seek
  sf_count_t curSeek=sf_seek(sf,0,SEEK_CUR);
  if (curSeek==-1) {
    // I/O error
    fileError=true;
    return;
  }

  if ((curSeek&DIV_FPCACHE_BLOCK_MASK)!=0 || (curSeek>>DIV_FPCACHE_BLOCK_SHIFT)!=firstBlock) {
    // we need to seek
    logV("- seeking");
    // we seek to a previous position in order to compensate for possible decoding differences when seeking
    // (usually in lossy codecs)
    sf_count_t seekWhere=firstBlock<<DIV_FPCACHE_BLOCK_SHIFT;
    if (seekWhere<DIV_FPCACHE_DISCARD_SIZE) {
      curSeek=sf_seek(sf,0,SEEK_SET);
      // discard
      if (sf_readf_float(sf,discardBuf,seekWhere)!=seekWhere) {
        // this is a problem
      }
    } else {
      seekWhere-=DIV_FPCACHE_DISCARD_SIZE;
      curSeek=sf_seek(sf,seekWhere,SEEK_SET);
      // discard
      if (sf_readf_float(sf,discardBuf,DIV_FPCACHE_DISCARD_SIZE)!=DIV_FPCACHE_DISCARD_SIZE) {
        // this is a problem
      }
    }
  }

  // read blocks
  for (ssize_t i=firstBlock; i<=lastBlock; i++) {
    if (!blocks[i]) {
      blocks[i]=new float[DIV_FPCACHE_BLOCK_SIZE*si.channels];
      memset(blocks[i],0,DIV_FPCACHE_BLOCK_SIZE*si.channels*sizeof(float));
    }
    logV("- reading block %" PRIu64,i);
    sf_count_t totalRead=sf_readf_float(sf,blocks[i],DIV_FPCACHE_BLOCK_SIZE);
    if (totalRead<DIV_FPCACHE_BLOCK_SIZE) {
      // we've reached end of file
    }
  }
}

void DivFilePlayer::collectGarbage(ssize_t pos) {
  // don't if file isn't present
  if (!blocks) return;

  // don't if there was an I/O error
  if (fileError) return;
  
  // don't if we cannot seek
  if (!si.seekable) return;

  size_t memUsage=getMemUsage();
  if (memUsage<DIV_MAX_MEMORY) return;

  pos>>=DIV_FPCACHE_BLOCK_SHIFT;
  if (pos<0) pos=0;
  if (pos>=(ssize_t)numBlocks) pos=numBlocks-1;

  // collect garbage
  // start with blocks before the given position
  // then try with blocks after given position
  // prioritize far away ones
  // do not destroy priority blocks
  for (ssize_t i=0; i<pos-2; i++) {
    if (!blocks[i]) continue;
    if (priorityBlock[i]) continue;
    logV("erasing block %d",(int)i);
    float* block=blocks[i];
    blocks[i]=NULL;
    delete[] block;

    memUsage-=DIV_FPCACHE_BLOCK_SIZE*si.channels*sizeof(float);
    if (memUsage<DIV_MAX_MEMORY) return;
  }
  for (ssize_t i=numBlocks-1; i>pos+DIV_FPCACHE_BLOCKS_FROM_FILL; i--) {
    if (!blocks[i]) continue;
    if (priorityBlock[i]) continue;
    logV("erasing block %d",(int)i);
    float* block=blocks[i];
    blocks[i]=NULL;
    delete[] block;

    memUsage-=DIV_FPCACHE_BLOCK_SIZE*si.channels*sizeof(float);
    if (memUsage<DIV_MAX_MEMORY) return;
  }
}

void DivFilePlayer::runCacheThread() {
  std::unique_lock<std::mutex> lock(cacheThreadLock);

  while (!quitThread) {
    ssize_t wantBlockC=wantBlock;
    if (wantBlockC!=DIV_NO_BLOCK) {
      wantBlock=DIV_NO_BLOCK;
      logV("thread fill %" PRIu64,wantBlockC);
      fillBlocksNear(wantBlockC);
      collectGarbage(wantBlockC);
    }
    cacheCV.wait(lock);
  }

  threadHasQuit=true;
  logV("DivFilePlayer: cache thread over.");
}

float DivFilePlayer::getSampleAt(ssize_t pos, int ch) {
  if (blocks==NULL) return 0.0f;
  ssize_t blockIndex=pos>>DIV_FPCACHE_BLOCK_SHIFT;
  if (blockIndex<0 || blockIndex>=(ssize_t)numBlocks) return 0.0f;

  float* block=blocks[blockIndex];
  size_t posInBlock=(pos&DIV_FPCACHE_BLOCK_MASK)*si.channels+ch;
  if (block==NULL) return 0.0f;

  return block[posInBlock];
}

void DivFilePlayer::mix(float** buf, int chans, unsigned int size) {
  // fill with zero if we don't have a file
  if (sf==NULL) {
    for (int i=0; i<chans; i++) {
      memset(buf[i],0,size*sizeof(float));
    }
    return;
  }

  float actualVolume=volume+1.0f;
  if (actualVolume<0.0f) actualVolume=0.0f;
  if (actualVolume>1.0f) actualVolume=1.0f;

  if (wantBlock!=DIV_NO_BLOCK) {
    cacheCV.notify_one();
  }

  for (unsigned int i=0; i<size; i++) {
    // acknowledge pending events
    if (pendingPosOffset==i) {
      pendingPosOffset=UINT_MAX;
      playPos=pendingPos;
      rateAccum=0;
    }
    if (pendingPlayOffset==i) {
      playing=true;
    }
    if (pendingStopOffset==i) {
      playing=false;
    }

    ssize_t blockIndex=playPos>>DIV_FPCACHE_BLOCK_SHIFT;
    if (blockIndex!=lastWantBlock) {
      wantBlock=playPos;
      cacheCV.notify_one();
      lastWantBlock=blockIndex;
    }

    if (playing) {
      // sinc interpolation
      float x[8];

      unsigned int n=(8192*rateAccum)/outRate;
      n&=8191;
      float* t1=&sincTable[(8191-n)<<2];
      float* t2=&sincTable[n<<2];
      if (si.channels==1) {
        // mono optimization
        for (int k=0; k<8; k++) {
          x[k]=getSampleAt(playPos+k-3,0);
        }

        float s=(
          x[0]*t2[3]+
          x[1]*t2[2]+
          x[2]*t2[1]+
          x[3]*t2[0]+
          x[4]*t1[0]+
          x[5]*t1[1]+
          x[6]*t1[2]+
          x[7]*t1[3]
        )*actualVolume;

        for (int j=0; j<chans; j++) {
          buf[j][i]=s;
        }
      } else for (int j=0; j<chans; j++) {
        if (j>=si.channels) {
          buf[j][i]=0.0f;
          continue;
        }

        for (int k=0; k<8; k++) {
          x[k]=getSampleAt(playPos+k-3,j);
        }
        buf[j][i]=(
          x[0]*t2[3]+
          x[1]*t2[2]+
          x[2]*t2[1]+
          x[3]*t2[0]+
          x[4]*t1[0]+
          x[5]*t1[1]+
          x[6]*t1[2]+
          x[7]*t1[3]
        )*actualVolume;
      }

      // advance
      rateAccum+=si.samplerate;
      while (rateAccum>=outRate) {
        rateAccum-=outRate;
        playPos++;
        /*if (playPos>=(ssize_t)si.frames) {
          playPos=0;
        }*/
      }
    } else {
      for (int j=0; j<chans; j++) {
        buf[j][i]=0.0f;
      }
    }
  }
}

ssize_t DivFilePlayer::getPos() {
  return playPos;
}

TimeMicros DivFilePlayer::getPosSeconds() {
  if (sf==NULL) {
    return TimeMicros(0,0);
  }
  double microsD=playPos%si.samplerate;
  return TimeMicros(
    playPos/si.samplerate, // seconds
    (int)((1000000.0*microsD)/(double)si.samplerate) // microseconds
  );
}

ssize_t DivFilePlayer::setPos(ssize_t newPos, unsigned int offset) {
  if (offset==UINT_MAX) {
    playPos=newPos;
    rateAccum=0;
    wantBlock=playPos;
    logD("DivFilePlayer: setPos(%" PRIi64 ")",newPos);
    return playPos;
  } else {
    pendingPosOffset=offset;
    pendingPos=newPos;
    wantBlock=playPos;
    logD("DivFilePlayer: offset %u setPos(%" PRIi64 ")",offset,newPos);
    return newPos;
  }
}

ssize_t DivFilePlayer::setPosSeconds(TimeMicros newTime, unsigned int offset) {
  if (sf==NULL) return 0;
  double microsD=(double)si.samplerate*((double)newTime.micros/1000000.0);
  if (offset==UINT_MAX) {
    playPos=(ssize_t)newTime.seconds*(ssize_t)si.samplerate+(int)microsD;
    rateAccum=0;
    wantBlock=playPos;
    logD("DivFilePlayer: setPosSeconds(%s)",newTime.toString());
    return playPos;
  } else {
    pendingPosOffset=offset;
    pendingPos=(ssize_t)newTime.seconds*(ssize_t)si.samplerate+(int)microsD;
    wantBlock=pendingPos;
    logD("DivFilePlayer: offset %u setPosSeconds(%s)",offset,newTime.toString());
    return pendingPos;
  }
}

size_t DivFilePlayer::getMemUsage() {
  if (blocks==NULL) return 0;
  size_t ret=0;
  for (size_t i=0; i<numBlocks; i++) {
    if (blocks[i]) ret+=DIV_FPCACHE_BLOCK_SIZE*si.channels*sizeof(float);
  }
  return ret;
}

bool DivFilePlayer::isBlockPresent(ssize_t pos) {
  if (blocks==NULL) return false;
  ssize_t which=pos>>DIV_FPCACHE_BLOCK_SHIFT;
  if (which<0 || which>=(ssize_t)numBlocks) return false;
  return (blocks[which]!=NULL);
}

bool DivFilePlayer::setBlockPriority(ssize_t pos, bool priority) {
  if (priorityBlock==NULL) return false;
  ssize_t which=pos>>DIV_FPCACHE_BLOCK_SHIFT;
  if (which<0 || which>=(ssize_t)numBlocks) return false;
  priorityBlock[which]=priority;
  return priority;
}

bool DivFilePlayer::isLoaded() {
  return (sf!=NULL);
}

bool DivFilePlayer::isPlaying() {
  return playing;
}

void DivFilePlayer::play(unsigned int offset) {
  if (offset!=UINT_MAX) {
    pendingPlayOffset=offset;
    logV("DivFilePlayer: playing (offset: %u)",offset);
  } else {
    playing=true;
    logV("DivFilePlayer: playing");
  }
}

void DivFilePlayer::stop(unsigned int offset) {
  if (offset!=UINT_MAX) {
    pendingStopOffset=offset;
    logV("DivFilePlayer: stopping (offset: %u)",offset);
  } else {
    playing=false;
    logV("DivFilePlayer: stopping");
  }
}

bool DivFilePlayer::closeFile() {
  if (sf==NULL) return false;

  logD("DivFilePlayer: closing file.");

  if (cacheThread) {
    quitThread=true;
    while (!threadHasQuit) {
      cacheCV.notify_one();
      std::this_thread::yield();
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    // this join is guaranteed to work
    cacheThread->join();
    delete cacheThread;
    cacheThread=NULL;
  }

  sfw.doClose();
  sf=NULL;
  playing=false;
  quitThread=false;
  threadHasQuit=false;

  for (size_t i=0; i<numBlocks; i++) {
    if (blocks[i]) {
      delete[] blocks[i];
      blocks[i]=NULL;
    }
  }
  numBlocks=0;
  delete[] blocks;
  blocks=NULL;

  delete[] discardBuf;
  discardBuf=NULL;

  return true;
}

bool DivFilePlayer::loadFile(const char* path) {
  if (sf!=NULL) closeFile();

  logD("DivFilePlayer: opening file...");
  sf=sfw.doOpen(path,SFM_READ,&si);
  if (sf==NULL) {
    logE("could not open file!");
    return false;
  }

  logV("- samples: %d",si.frames);
  logV("- channels: %d",si.channels);
  logV("- rate: %d",si.samplerate);

  numBlocks=(DIV_FPCACHE_BLOCK_MASK+si.frames)>>DIV_FPCACHE_BLOCK_SHIFT;
  blocks=new float*[numBlocks];
  priorityBlock=new bool[numBlocks];
  memset(blocks,0,numBlocks*sizeof(void*));
  memset(priorityBlock,0,numBlocks*sizeof(bool));

  // mark the first blocks as important
  for (size_t i=0; i<DIV_FPCACHE_BLOCKS_FROM_FILL; i++) {
    if (i>=numBlocks) break;
    priorityBlock[i]=true;
  }

  playPos=0;
  lastWantBlock=DIV_NO_BLOCK;
  rateAccum=0;
  fileError=false;

  // read the entire file if not seekable
  if (!si.seekable) {
    logV("file not seekable - reading...");
    for (size_t i=0; i<numBlocks; i++) {
      blocks[i]=new float[DIV_FPCACHE_BLOCK_SIZE*si.channels];
    }
    for (size_t i=0; i<numBlocks; i++) {
      sf_count_t totalRead=sf_readf_float(sf,blocks[i],DIV_FPCACHE_BLOCK_SIZE);
      if (totalRead<DIV_FPCACHE_BLOCK_SIZE) {
        // we've reached end of file
        break;
      }
    }
  } else {
    logV("file is seekable");
    // read the first couple blocks
    fillBlocksNear(0);
  }

  discardBuf=new float[DIV_FPCACHE_DISCARD_SIZE*si.channels];

  // stsrt the block cache thread
  if (cacheThread==NULL) {
    quitThread=false;
    threadHasQuit=false;
    cacheThread=new std::thread(&DivFilePlayer::runCacheThread,this);
  }
  return true;
}

String DivFilePlayer::getLastError() {
  return lastError;
}

const SF_INFO& DivFilePlayer::getFileInfo() {
  return si;
}

void DivFilePlayer::setOutputRate(int rate) {
  if (rate<1) return;
  outRate=rate;
}

float DivFilePlayer::getVolume() {
  return volume;
}

void DivFilePlayer::setVolume(float vol) {
  volume=vol;
}

bool DivFilePlayer::getActive() {
  return isActive;
}

void DivFilePlayer::setActive(bool active) {
  isActive=active;
}

DivFilePlayer::DivFilePlayer():
  discardBuf(NULL),
  blocks(NULL),
  priorityBlock(NULL),
  numBlocks(0),
  sf(NULL),
  playPos(0),
  lastWantBlock(DIV_NO_BLOCK),
  wantBlock(DIV_NO_BLOCK),
  outRate(44100),
  rateAccum(0),
  volume(0.0f),
  playing(false),
  fileError(false),
  quitThread(false),
  threadHasQuit(false),
  isActive(false),
  pendingPos(0),
  pendingPosOffset(UINT_MAX),
  pendingPlayOffset(UINT_MAX),
  pendingStopOffset(UINT_MAX),
  cacheThread(NULL) {
  memset(&si,0,sizeof(SF_INFO));
  sincTable=DivFilterTables::getSincTable8();
}

DivFilePlayer::~DivFilePlayer() {
  closeFile();
}
