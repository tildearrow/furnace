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

#define DIV_FPCACHE_BLOCK_SHIFT 16
#define DIV_FPCACHE_BLOCK_SIZE (1U<<DIV_FPCACHE_BLOCK_SHIFT)
#define DIV_FPCACHE_BLOCK_MASK (DIV_FPCACHE_BLOCK_SIZE-1)

#define DIV_FPCACHE_BLOCKS_FROM_FILL 2
#define DIV_FPCACHE_DISCARD_SIZE 4096

void DivFilePlayer::fillBlocksNear(size_t pos) {
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
      firstBlock=blocks[i];
      break;
    }
  }

  if (!needToFill) return;

  // check whether we need to seek
  sf_count_seek curSeek=sf_seek(sf,0,SEEK_CUR);
  if (curSeek==-1) {
    // I/O error
    fileError=true;
    return;
  }

  

  // read blocks
  for (size_t i=firstBlock; i<=lastBlock; i++) {
    if (!blocks[i]) {
      blocks[i]=new float[DIV_FPCACHE_BLOCK_SIZE*si.channels];
      memset(blocks[i],0,
    }
    sf_count_t totalRead=sf_readf_float(sf,blocks[i],DIV_FPCACHE_BLOCK_SIZE);
    if (totalRead<DIV_FPCACHE_BLOCK_SIZE) {
      // we've reached end of file
    }
  }
}

void DivFilePlayer::mix(float** buf, int chans, unsigned int size) {
  
}

size_t DivFilePlayer::getPos() {
  return playPos;
}

size_t DivFilePlayer::setPos(size_t newPos, unsigned int offset=0) {
  playPos=newPos;
}

void DivFilePlayer::play(unsigned int offset=0) {
  playing=true;
}

void DivFilePlayer::stop(unsigned int offset=0) {
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
}

bool DivFilePlayer::loadFile(const char* path) {
  sf=sfw.doOpen(path,SFM_READ,&si);
  if (sf==NULL) {
    return false;
  }

  numBlocks=(DIV_FPCACHE_BLOCK_MASK+si.frames)>>DIV_FPCACHE_BLOCK_SHIFT;
  blocks=new float*[numBlocks];
  memset(blocks,0,numBlocks*sizeof(void*));

  playPos=0;
  rateAccum=0;
  fileError=false;

  // read the entire file if not seekable
  if (!si.seekable) {
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
  }

  discardBuf=new float[DIV_FPCACHE_DISCARD_SIZE*si.channels];
}

String DivFilePlayer::getLastError() {
  return lastError;
}

void DivFilePlayer::setOutputRate(int rate) {
  outRate=0;
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
