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

#ifndef _SAMPLE_H
#define _SAMPLE_H

#include "../ta-utils.h"
#include "defines.h"
#include "safeWriter.h"
#include "dataErrors.h"
#include <deque>

enum DivSampleLoopMode: unsigned char {
  DIV_SAMPLE_LOOP_FORWARD=0,
  DIV_SAMPLE_LOOP_BACKWARD,
  DIV_SAMPLE_LOOP_PINGPONG,
  DIV_SAMPLE_LOOP_MAX // boundary for loop mode
};

enum DivSampleDepth: unsigned char {
  DIV_SAMPLE_DEPTH_1BIT=0,
  DIV_SAMPLE_DEPTH_1BIT_DPCM=1,
  DIV_SAMPLE_DEPTH_YMZ_ADPCM=3,
  DIV_SAMPLE_DEPTH_QSOUND_ADPCM=4,
  DIV_SAMPLE_DEPTH_ADPCM_A=5,
  DIV_SAMPLE_DEPTH_ADPCM_B=6,
  DIV_SAMPLE_DEPTH_8BIT=8,
  DIV_SAMPLE_DEPTH_BRR=9,
  DIV_SAMPLE_DEPTH_VOX=10,
  DIV_SAMPLE_DEPTH_16BIT=16,
  DIV_SAMPLE_DEPTH_MAX // boundary for sample depth
};

enum DivResampleFilters {
  DIV_RESAMPLE_NONE=0,
  DIV_RESAMPLE_LINEAR,
  DIV_RESAMPLE_CUBIC,
  DIV_RESAMPLE_BLEP,
  DIV_RESAMPLE_SINC,
  DIV_RESAMPLE_BEST
};

struct DivSampleHistory {
  unsigned char* data;
  unsigned int length, samples;
  DivSampleDepth depth;
  int rate, centerRate, loopStart, loopEnd;
  bool loop, brrEmphasis;
  DivSampleLoopMode loopMode;
  bool hasSample;
  DivSampleHistory(void* d, unsigned int l, unsigned int s, DivSampleDepth de, int r, int cr, int ls, int le, bool lp, bool be, DivSampleLoopMode lm):
    data((unsigned char*)d),
    length(l),
    samples(s),
    depth(de),
    rate(r),
    centerRate(cr),
    loopStart(ls),
    loopEnd(le),
    loop(lp),
    brrEmphasis(be),
    loopMode(lm),
    hasSample(true) {}
  DivSampleHistory(DivSampleDepth de, int r, int cr, int ls, int le, bool lp, bool be, DivSampleLoopMode lm):
    data(NULL),
    length(0),
    samples(0),
    depth(de),
    rate(r),
    centerRate(cr),
    loopStart(ls),
    loopEnd(le),
    loop(lp),
    brrEmphasis(be),
    loopMode(lm),
    hasSample(false) {}
  ~DivSampleHistory();
};

struct DivSample {
  String name;
  // TODO: get rid of loopOffP
  int rate, centerRate, loopStart, loopEnd, loopOffP;
  // valid values are:
  // - 0: ZX Spectrum overlay drum (1-bit)
  // - 1: 1-bit NES DPCM (1-bit)
  // - 3: YMZ ADPCM
  // - 4: QSound ADPCM
  // - 5: ADPCM-A
  // - 6: ADPCM-B
  // - 8: 8-bit PCM
  // - 9: BRR (SNES)
  // - 10: VOX ADPCM
  // - 16: 16-bit PCM
  DivSampleDepth depth;
  bool loop, brrEmphasis;
  // valid values are:
  // - 0: Forward loop
  // - 1: Backward loop
  // - 2: Pingpong loop
  DivSampleLoopMode loopMode;

  bool renderOn[DIV_MAX_SAMPLE_TYPE][DIV_MAX_CHIPS];

  // these are the new data structures.
  signed char* data8; // 8
  short* data16; // 16
  unsigned char* data1; // 0
  unsigned char* dataDPCM; // 1
  unsigned char* dataZ; // 3
  unsigned char* dataQSoundA; // 4
  unsigned char* dataA; // 5
  unsigned char* dataB; // 6
  unsigned char* dataBRR; // 9
  unsigned char* dataVOX; // 10

  unsigned int length8, length16, length1, lengthDPCM, lengthZ, lengthQSoundA, lengthA, lengthB, lengthBRR, lengthVOX;

  unsigned int samples;

  std::deque<DivSampleHistory*> undoHist;
  std::deque<DivSampleHistory*> redoHist;

  /**
   * put sample data.
   * @param w a SafeWriter.
   */
  void putSampleData(SafeWriter* w);

  /**
   * read sample data.
   * @param reader the reader.
   * @param version the format version.
   * @return a DivDataErrors.
   */
  DivDataErrors readSampleData(SafeReader& reader, short version);

  /**
   * check if sample is loopable.
   * @return whether it is loopable.
   */
  bool isLoopable();

  /**
   * get sample start position
   * @return the samples start position.
   */
  int getLoopStartPosition(DivSampleDepth depth=DIV_SAMPLE_DEPTH_MAX);

  /**
   * get sample loop end position
   * @return the samples loop end position.
   */
  int getLoopEndPosition(DivSampleDepth depth=DIV_SAMPLE_DEPTH_MAX);

  /**
   * get sample end position
   * @return the samples end position.
   */
  int getEndPosition(DivSampleDepth depth=DIV_SAMPLE_DEPTH_MAX);

  /**
   * get sample offset
   * @return the sample offset.
   */
  int getSampleOffset(int offset, int length, DivSampleDepth depth=DIV_SAMPLE_DEPTH_MAX);

  /**
   * @warning DO NOT USE - internal functions
   */
  void setSampleCount(unsigned int count);

  /**
   * @warning DO NOT USE - internal functions
   */
  bool resampleNone(double rate);
  bool resampleLinear(double rate);
  bool resampleCubic(double rate);
  bool resampleBlep(double rate);
  bool resampleSinc(double rate);

  /**
   * save this sample to a file.
   * @param path a path.
   * @return whether saving succeeded or not.
   */
  bool save(const char* path);

  /**
   * @warning DO NOT USE - internal function
   * initialize sample data.
   * @param d sample type.
   * @param count number of samples.
   * @return whether it was successful.
   */
  bool initInternal(DivSampleDepth d, int count);

  /**
   * initialize sample data. make sure you have set `depth` before doing so.
   * @param count number of samples.
   * @return whether it was successful.
   */
  bool init(unsigned int count);

  /**
   * resize sample data. make sure the sample has been initialized before doing so.
   * @warning do not attempt to resize a sample outside of a synchronized block!
   * @param count number of samples.
   * @return whether it was successful.
   */
  bool resize(unsigned int count);

  /**
   * remove part of the sample data.
   * @warning do not attempt to strip a sample outside of a synchronized block!
   * @param start the beginning.
   * @param end the end.
   * @return whether it was successful.
   */
  bool strip(unsigned int begin, unsigned int end);

  /**
   * clip the sample data to specified boundaries.
   * @warning do not attempt to trim a sample outside of a synchronized block!
   * @param start the beginning.
   * @param end the end.
   * @return whether it was successful.
   */
  bool trim(unsigned int begin, unsigned int end);

  /**
   * insert silence at specified position.
   * @warning do not attempt to do this outside of a synchronized block!
   * @param pos the beginning.
   * @param length how many samples to insert.
   * @return whether it was successful.
   */
  bool insert(unsigned int pos, unsigned int length);

  /**
   * change the sample rate.
   * @warning do not attempt to resample outside of a synchronized block!
   * @param rate number of samples.
   * @param filter the interpolation filter.
   * @return whether it was successful.
   */
  bool resample(double rate, int filter);

  /**
   * initialize the rest of sample formats for this sample.
   */
  void render(unsigned int formatMask=0xffffffff);

  /**
   * get the sample data for the current depth.
   * @return the sample data, or NULL if not created.
   */
  void* getCurBuf();

  /**
   * get the sample data length for the current depth.
   * @return the sample data length.
   */
  unsigned int getCurBufLen();

  /**
   * prepare an undo step for this sample.
   * @param data whether to include sample data.
   * @param doNotPush if this is true, don't push the DivSampleHistory to the undo history.
   * @return the undo step.
   */
  DivSampleHistory* prepareUndo(bool data, bool doNotPush=false);

  /**
   * undo. you may need to call DivEngine::renderSamples afterwards.
   * @warning do not attempt to undo outside of a synchronized block!
   * @return 0 on failure; 1 on success and 2 on success (data changed).
   */
  int undo();

  /**
   * redo. you may need to call DivEngine::renderSamples afterwards.
   * @warning do not attempt to redo outside of a synchronized block!
   * @return 0 on failure; 1 on success and 2 on success (data changed).
   */
  int redo();
  DivSample():
    name(""),
    rate(32000),
    centerRate(8363),
    loopStart(-1),
    loopEnd(-1),
    loopOffP(0),
    depth(DIV_SAMPLE_DEPTH_16BIT),
    loop(false),
    brrEmphasis(true),
    loopMode(DIV_SAMPLE_LOOP_FORWARD),
    data8(NULL),
    data16(NULL),
    data1(NULL),
    dataDPCM(NULL),
    dataZ(NULL),
    dataQSoundA(NULL),
    dataA(NULL),
    dataB(NULL),
    dataBRR(NULL),
    dataVOX(NULL),
    length8(0),
    length16(0),
    length1(0),
    lengthDPCM(0),
    lengthZ(0),
    lengthQSoundA(0),
    lengthA(0),
    lengthB(0),
    lengthBRR(0),
    lengthVOX(0),
    samples(0) {
    for (int i=0; i<DIV_MAX_CHIPS; i++) {
      for (int j=0; j<DIV_MAX_SAMPLE_TYPE; j++) {
        renderOn[j][i]=true;
      }
    }
  }
  ~DivSample();
};

#endif
