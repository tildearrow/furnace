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

void DivFilePlayer::fillBlocksNear(size_t pos) {
}

void DivFilePlayer::mix(float** buf, int chans, unsigned int size) {

}

size_t DivFilePlayer::getPos() {
  return playPos;
}

size_t DivFilePlayer::setPos(size_t newPos, unsigned int offset=0) {

}

void DivFilePlayer::play(unsigned int offset=0) {
  playing=true;
}

void DivFilePlayer::stop(unsigned int offset=0) {
  playing=false;
}

bool DivFilePlayer::closeFile() {
  if (sf==NULL) return false;
  sf_close(sf);
  sf=NULL;
}

bool DivFilePlayer::loadFile(const char* path) {
}

String DivFilePlayer::getLastError() {
  return lastError;
}

void DivFilePlayer::setOutputRate(int rate) {

}

float DivFilePlayer::getVolume() {

}

void DivFilePlayer::setVolume(float vol) {

}

DivFilePlayer::DivFilePlayer():
  blocks(NULL),
  sf(NULL),
  playPos(0),
  outRate(44100),
  rateAccum(0),
  volume(1.0f),
  playing(false),
  fileError(false) {
}

DivFilePlayer::~DivFilePlayer() {
  closeFile();
}
