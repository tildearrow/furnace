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

#include "filePlayer.h"
#include "../ta-log.h"
#include <inttypes.h>

#define DIV_FPCACHE_BLOCK_SHIFT 16
#define DIV_FPCACHE_BLOCK_SIZE (1U<<DIV_FPCACHE_BLOCK_SHIFT)
#define DIV_FPCACHE_BLOCK_MASK (DIV_FPCACHE_BLOCK_SIZE-1)

#define DIV_FPCACHE_BLOCKS_FROM_FILL 8
#define DIV_FPCACHE_DISCARD_SIZE 4096

void DivFilePlayer::fillBlocksNear(size_t pos) {
  logV("DivFilePlayer: fillBlocksNear(%" PRIu64 ")",pos);

  // don't if file isn't present
  if (!blocks) return;

  // don't if there was an I/O error
  if (fileError) return;

  // don't read anything if we cannot seek
  // (if this is set the file is already read in its entirety)
  if (!si.seekable) return;

  size_t firstBlock=pos>>DIV_FPCACHE_BLOCK_SHIFT;
  size_t lastBlock=firstBlock+DIV_FPCACHE_BLOCKS_FROM_FILL;
  if (lastBlock>=numBlocks) lastBlock=numBlocks-1;

  // don't read if we're after end of file
  if (firstBlock>lastBlock) return;
  
  bool needToFill=false;
  for (size_t i=firstBlock; i<=lastBlock; i++) {
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

  if ((curSeek&DIV_FPCACHE_BLOCK_MASK)!=0 || (((size_t)curSeek)>>DIV_FPCACHE_BLOCK_SHIFT)!=firstBlock) {
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
  for (size_t i=firstBlock; i<=lastBlock; i++) {
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

void DivFilePlayer::mix(float** buf, int chans, unsigned int size) {
  // fill with zero if we don't have a file
  if (sf==NULL) {
    for (int i=0; i<chans; i++) {
      memset(buf[i],0,size*sizeof(float));
    }
    return;
  }

  for (unsigned int i=0; i<size; i++) {
    if (playing) {
      size_t blockIndex=playPos>>DIV_FPCACHE_BLOCK_SHIFT;
      if (blockIndex!=lastBlock) {
        fillBlocksNear(playPos);
        lastBlock=blockIndex;
      }
      if (blockIndex>=numBlocks) {
        // stop here
        for (int j=0; j<chans; j++) {
          buf[j][i]=0.0f;
        }
        continue;
      }
      float* block=blocks[blockIndex];
      size_t posInBlock=(playPos&DIV_FPCACHE_BLOCK_MASK)*si.channels;

      // put
      if (block==NULL) {
        for (int j=0; j<chans; j++) {
          buf[j][i]=0.0f;
        }
      } else if (si.channels==1) {
        for (int j=0; j<chans; j++) {
          buf[j][i]=block[posInBlock]*volume;
        }
      } else {
        for (int j=0; j<chans; j++) {
          if (j>=si.channels) {
            buf[j][i]=0.0f;
          } else {
            buf[j][i]=block[posInBlock++]*volume;
          }
        }
      }

      // advance
      rateAccum+=si.samplerate;
      while (rateAccum>=outRate) {
        rateAccum-=outRate;
        playPos++;
        if (playPos>=(size_t)si.frames) {
          playPos=0;
        }
      }
    } else {
      for (int j=0; j<chans; j++) {
        buf[j][i]=0.0f;
      }
    }
  }
}

size_t DivFilePlayer::getPos() {
  return playPos;
}

size_t DivFilePlayer::setPos(size_t newPos, unsigned int offset) {
  playPos=newPos;
  return playPos;
}

bool DivFilePlayer::isBlockPresent(size_t pos) {
  if (blocks==NULL) return false;
  return (blocks[pos>>DIV_FPCACHE_BLOCK_SHIFT]!=NULL);
}

bool DivFilePlayer::isLoaded() {
  return (sf!=NULL);
}

bool DivFilePlayer::isPlaying() {
  return playing;
}

void DivFilePlayer::play(unsigned int offset) {
  logV("DivFilePlayer: playing");
  playing=true;
}

void DivFilePlayer::stop(unsigned int offset) {
  logV("DivFilePlayer: stopping");
  playing=false;
}

bool DivFilePlayer::closeFile() {
  if (sf==NULL) return false;
  sfw.doClose();
  sf=NULL;
  playing=false;

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
  memset(blocks,0,numBlocks*sizeof(void*));

  playPos=0;
  lastBlock=SIZE_MAX;
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

DivFilePlayer::DivFilePlayer():
  discardBuf(NULL),
  blocks(NULL),
  numBlocks(0),
  sf(NULL),
  playPos(0),
  lastBlock(SIZE_MAX),
  outRate(44100),
  rateAccum(0),
  volume(1.0f),
  playing(false),
  fileError(false) {
  memset(&si,0,sizeof(SF_INFO));
}

DivFilePlayer::~DivFilePlayer() {
  closeFile();
}
