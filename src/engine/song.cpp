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

#include "song.h"
#include "../ta-log.h"

DivSongTimestamps::Timestamp DivSongTimestamps::getTimes(int order, int row) {
  if (order<0 || order>=DIV_MAX_PATTERNS) return Timestamp(-1,0);
  if (row<0 || row>=DIV_MAX_ROWS) return Timestamp(-1,0);
  Timestamp* t=orders[order];
  if (t==NULL) return Timestamp(-1,0);
  return t[row];
}

DivSongTimestamps::DivSongTimestamps():
  totalSeconds(0),
  totalMicros(0),
  totalTicks(0),
  isLoopDefined(false),
  isLoopable(true) {
  memset(orders,0,DIV_MAX_PATTERNS*sizeof(void*));
  memset(maxRow,0,DIV_MAX_PATTERNS);
}

DivSongTimestamps::~DivSongTimestamps() {
  for (int i=0; i<DIV_MAX_PATTERNS; i++) {
    if (orders[i]) {
      delete[] orders[i];
      orders[i]=NULL;
    }
  }
}
bool DivSubSong::walk(int& loopOrder, int& loopRow, int& loopEnd, int chans, int jumpTreatment, int ignoreJumpAtEnd, int firstPat) {
  loopOrder=0;
  loopRow=0;
  loopEnd=-1;
  int nextOrder=-1;
  int nextRow=0;
  int effectVal=0;
  int lastSuspectedLoopEnd=-1;
  DivPattern* subPat[DIV_MAX_CHANS];
  unsigned char wsWalked[8192];
  memset(wsWalked,0,8192);
  if (firstPat>0) {
    memset(wsWalked,255,32*firstPat);
  }
  for (int i=firstPat; i<ordersLen; i++) {
    for (int j=0; j<chans; j++) {
      subPat[j]=pat[j].getPattern(orders.ord[j][i],false);
    }
    if (i>lastSuspectedLoopEnd) {
      lastSuspectedLoopEnd=i;
    }
    for (int j=nextRow; j<patLen; j++) {
      nextRow=0;
      bool changingOrder=false;
      bool jumpingOrder=false;
      if (wsWalked[((i<<5)+(j>>3))&8191]&(1<<(j&7))) {
        loopOrder=i;
        loopRow=j;
        loopEnd=lastSuspectedLoopEnd;
        return true;
      }
      for (int k=0; k<chans; k++) {
        for (int l=0; l<pat[k].effectCols; l++) {
          effectVal=subPat[k]->newData[j][DIV_PAT_FXVAL(l)];
          if (effectVal<0) effectVal=0;
          if (subPat[k]->newData[j][DIV_PAT_FX(l)]==0x0d) {
            if (jumpTreatment==2) {
              if ((i<ordersLen-1 || !ignoreJumpAtEnd)) {
                nextOrder=i+1;
                nextRow=effectVal;
                jumpingOrder=true;
              }
            } else if (jumpTreatment==1) {
              if (nextOrder==-1 && (i<ordersLen-1 || !ignoreJumpAtEnd)) {
                nextOrder=i+1;
                nextRow=effectVal;
                jumpingOrder=true;
              }
            } else {
              if ((i<ordersLen-1 || !ignoreJumpAtEnd)) {
                if (!changingOrder) {
                  nextOrder=i+1;
                }
                jumpingOrder=true;
                nextRow=effectVal;
              }
            }
          } else if (subPat[k]->newData[j][DIV_PAT_FX(l)]==0x0b) {
            if (nextOrder==-1 || jumpTreatment==0) {
              nextOrder=effectVal;
              if (jumpTreatment==1 || jumpTreatment==2 || !jumpingOrder) {
                nextRow=0;
              }
              changingOrder=true;
            }
          }
        }
      }

      wsWalked[((i<<5)+(j>>3))&8191]|=1<<(j&7);
      
      if (nextOrder!=-1) {
        i=nextOrder-1;
        nextOrder=-1;
        break;
      }
    }
  }
  return false;
}

double calcRowLenInSeconds(const DivGroovePattern& speeds, float hz, int vN, int vD, int timeBaseFromSong) {
  double hl=1; //count for 1 row
  if (hl<=0.0) hl=4.0;
  double timeBase=timeBaseFromSong+1;
  double speedSum=0;
  for (int i=0; i<MIN(16,speeds.len); i++) {
    speedSum+=speeds.val[i];
  }
  speedSum/=MAX(1,speeds.len);
  if (timeBase<1.0) timeBase=1.0;
  if (speedSum<1.0) speedSum=1.0;
  if (vD<1) vD=1;
  return 1.0/((60.0*hz/(timeBase*hl*speedSum))*(double)vN/(double)vD/60.0);
}

void DivSubSong::findLength(int loopOrder, int loopRow, double fadeoutLen, int& rowsForFadeout, bool& hasFFxx, std::vector<int>& orders_vec, std::vector<DivGroovePattern>& grooves, int& length, int chans, int jumpTreatment, int ignoreJumpAtEnd, int firstPat) {
  length=0;
  hasFFxx=false;
  rowsForFadeout=0;

  float secondsPerThisRow=0.0f;

  DivGroovePattern curSpeeds=speeds; //simulate that we are playing the song, track all speed/BPM/tempo/engine rate changes
  short curVirtualTempoN=virtualTempoN;
  short curVirtualTempoD=virtualTempoD;
  float curHz=hz;
  double curDivider=(double)timeBase;

  double curLen=0.0; //how many seconds passed since the start of song

  int nextOrder=-1;
  int nextRow=0;
  int effectVal=0;
  int lastSuspectedLoopEnd=-1;
  DivPattern* subPat[DIV_MAX_CHANS];
  unsigned char wsWalked[8192];
  memset(wsWalked,0,8192);
  if (firstPat>0) {
    memset(wsWalked,255,32*firstPat);
  }
  for (int i=firstPat; i<ordersLen; i++) {
    bool jumped=false;

    for (int j=0; j<chans; j++) {
      subPat[j]=pat[j].getPattern(orders.ord[j][i],false);
    }
    if (i>lastSuspectedLoopEnd) {
      lastSuspectedLoopEnd=i;
    }
    for (int j=nextRow; j<patLen; j++) {
      nextRow=0;
      bool changingOrder=false;
      bool jumpingOrder=false;
      if (wsWalked[((i<<5)+(j>>3))&8191]&(1<<(j&7))) {
        return;
      }
      for (int k=0; k<chans; k++) {
        for (int l=0; l<pat[k].effectCols; l++) {
          effectVal=subPat[k]->newData[j][DIV_PAT_FXVAL(l)];
          if (effectVal<0) effectVal=0;

          if (subPat[k]->newData[j][DIV_PAT_FX(l)]==0xff) {
            hasFFxx=true;

            // FFxx makes YOU SHALL NOT PASS!!! move
            orders_vec.push_back(j+1); // order len
            length+=j+1; // add length of order to song length

            return;
          }

          switch (subPat[k]->newData[j][DIV_PAT_FX(l)]) {
            case 0x09: { // select groove pattern/speed 1
              if (grooves.empty()) {
                if (effectVal>0) curSpeeds.val[0]=effectVal;
              } else {
                if (effectVal<(short)grooves.size()) {
                  curSpeeds=grooves[effectVal];
                  //curSpeed=0;
                }
              }
              break;
            }
            case 0x0f: { //  speed 1/speed 2
              if (curSpeeds.len==2 && grooves.empty()) {
                if (effectVal>0) curSpeeds.val[1]=effectVal;
              } else {
                if (effectVal>0) curSpeeds.val[0]=effectVal;
              }
              break;
            }
            case 0xfd: { //  virtual tempo num
              if (effectVal>0) curVirtualTempoN=effectVal;
              break;
            }
            case 0xfe: { //  virtual tempo den
              if (effectVal>0) curVirtualTempoD=effectVal;
              break;
            }
            case 0xf0: { //  set Hz by tempo (set bpm)
              curDivider=(double)effectVal*2.0/5.0;
              if (curDivider<1) curDivider=1;
              break;
            }
          }

          if (subPat[k]->newData[j][DIV_PAT_FX(l)]==0x0d) {
            if (jumpTreatment==2) {
              if ((i<ordersLen-1 || !ignoreJumpAtEnd)) {
                nextOrder=i+1;
                nextRow=effectVal;
                jumpingOrder=true;
              }
            } else if (jumpTreatment==1) {
              if (nextOrder==-1 && (i<ordersLen-1 || !ignoreJumpAtEnd)) {
                nextOrder=i+1;
                nextRow=effectVal;
                jumpingOrder=true;
              }
            } else {
              if ((i<ordersLen-1 || !ignoreJumpAtEnd)) {
                if (!changingOrder) {
                  nextOrder=i+1;
                }
                jumpingOrder=true;
                nextRow=effectVal;
              }
            }
          } else if (subPat[k]->newData[j][DIV_PAT_FX(l)]==0x0b) {
            if (nextOrder==-1 || jumpTreatment==0) {
              nextOrder=effectVal;
              if (jumpTreatment==1 || jumpTreatment==2 || !jumpingOrder) {
                nextRow=0;
              }
              changingOrder=true;
            }
          }
        }
      }

      if (i>loopOrder || (i==loopOrder && j>loopRow)) {
        //  we count each row fadeout lasts. When our time is greater than fadeout length we successfully counted the number of fadeout rows
        if (curLen<=fadeoutLen && fadeoutLen>0.0) {
          secondsPerThisRow=calcRowLenInSeconds(speeds,curHz,curVirtualTempoN,curVirtualTempoD,curDivider);
          curLen+=secondsPerThisRow;
          rowsForFadeout++;
        }
      }

      wsWalked[((i<<5)+(j>>3))&8191]|=1<<(j&7);
      
      if (nextOrder!=-1) {
        i=nextOrder-1;
        orders_vec.push_back(j+1); // order len
        length+=j+1; // add length of order to song length
        jumped=true;
        nextOrder=-1;
        break;
      }
    }
    if (!jumped) { // if no jump occured we add full pattern length
      orders_vec.push_back(patLen); // order len
      length+=patLen; // add length of order to song length
    }
  }
}

void DivSubSong::calcTimestamps(int chans, std::vector<DivGroovePattern>& grooves, int jumpTreatment, int ignoreJumpAtEnd, int brokenSpeedSel, int delayBehavior, int firstPat) {
  // reduced version of the playback routine for calculation.

  // reset state
  ts.totalSeconds=0;
  ts.totalMicros=0;
  ts.totalTicks=0;
  ts.isLoopDefined=true;
  ts.isLoopable=true;

  memset(ts.maxRow,0,DIV_MAX_PATTERNS);
  
  for (int i=0; i<DIV_MAX_PATTERNS; i++) {
    if (ts.orders[i]) {
      delete[] ts.orders[i];
      ts.orders[i]=NULL;
    }
  }

  // walking state
  unsigned char wsWalked[8192];
  memset(wsWalked,0,8192);
  if (firstPat>0) {
    memset(wsWalked,255,32*firstPat);
  }
  int curOrder=firstPat;
  int curRow=0;
  int prevOrder=firstPat;
  int prevRow=0;
  DivGroovePattern curSpeeds=speeds;
  int curVirtualTempoN=virtualTempoN;
  int curVirtualTempoD=virtualTempoD;
  int nextSpeed=curSpeeds.val[0];
  double divider=hz;
  double totalMicrosOff=0.0;
  int ticks=1;
  int tempoAccum=0;
  int curSpeed=0;
  int changeOrd=-1;
  int changePos=0;
  unsigned char rowDelay[DIV_MAX_CHANS];
  unsigned char delayOrder[DIV_MAX_CHANS];
  unsigned char delayRow[DIV_MAX_CHANS];
  bool shallStopSched=false;
  bool shallStop=false;
  bool endOfSong=false;
  bool rowChanged=false;

  memset(rowDelay,0,DIV_MAX_CHANS);
  memset(delayOrder,0,DIV_MAX_CHANS);
  memset(delayRow,0,DIV_MAX_CHANS);
  if (divider<1) divider=1;

  auto tinyProcessRow=[&](int i, bool afterDelay) {
    // if this is after delay, use the order/row where delay occurred
    int whatOrder=afterDelay?delayOrder[i]:curOrder;
    int whatRow=afterDelay?delayRow[i]:curRow;
    DivPattern* pat=pat[i].getPattern(orders.ord[i][whatOrder],false);
    // pre effects
    if (!afterDelay) {
      // set to true if we found an EDxx effect
      bool returnAfterPre=false;
      // check all effects
      for (int j=0; j<curPat[i].effectCols; j++) {
        short effect=pat->newData[whatRow][DIV_PAT_FX(j)];
        short effectVal=pat->newData[whatRow][DIV_PAT_FXVAL(j)];

        // empty effect value is the same as zero
        if (effectVal==-1) effectVal=0;
        effectVal&=255;

        switch (effect) {
          case 0x09: // select groove pattern/speed 1
            if (grooves.empty()) {
              // special case: sets speed 1 if the song lacks groove patterns
              if (effectVal>0) curSpeeds.val[0]=effectVal;
            } else {
              // sets the groove pattern and resets current speed index
              if (effectVal<(short)grooves.size()) {
                curSpeeds=grooves[effectVal];
                curSpeed=0;
              }
            }
            break;
          case 0x0f: // speed 1/speed 2
            // if the value is 0 then ignore it
            if (curSpeeds.len==2 && grooves.empty()) {
              // if there are two speeds and no groove patterns, set the second speed
              if (effectVal>0) curSpeeds.val[1]=effectVal;
            } else {
              // otherwise set the first speed
              if (effectVal>0) curSpeeds.val[0]=effectVal;
            }
            break;
          case 0xfd: // virtual tempo num
            if (effectVal>0) virtualTempoN=effectVal;
            break;
          case 0xfe: // virtual tempo den
            if (effectVal>0) virtualTempoD=effectVal;
            break;
          case 0x0b: // change order
            // this actually schedules an order change
            // we perform this change at the end of nextRow()

            // COMPAT FLAG: simultaneous jump treatment
            if (changeOrd==-1 || jumpTreatment==0) {
              changeOrd=effectVal;
              if (jumpTreatment==1 || jumpTreatment==2) {
                changePos=0;
              }
            }
            break;
          case 0x0d: // next order
            // COMPAT FLAG: simultaneous jump treatment
            if (jumpTreatment==2) {
              // - 2: DefleMask (jump to next order unless it is the last one and ignoreJumpAtEnd is on)
              if ((curOrder<(curSubSong->ordersLen-1) || !ignoreJumpAtEnd)) {
                // changeOrd -2 means increase order by 1
                // it overrides a previous 0Bxx effect
                changeOrd=-2;
                changePos=effectVal;
              }
            } else if (jumpTreatment==1) {
              // - 1: old Furnace (same as 2 but ignored if 0Bxx is present)
              if (changeOrd<0 && (curOrder<(curSubSong->ordersLen-1) || !ignoreJumpAtEnd)) {
                changeOrd=-2;
                changePos=effectVal;
              }
            } else {
              // - 0: normal
              if (curOrder<(curSubSong->ordersLen-1) || !ignoreJumpAtEnd) {
                // set the target order if not set, allowing you to use 0B and 0D regardless of position
                if (changeOrd<0) {
                  changeOrd=-2;
                }
                changePos=effectVal;
              }
            }
            break;
          case 0xed: // delay
            if (effectVal!=0) {
              // COMPAT FLAG: cut/delay effect policy (delayBehavior)
              // - 0: strict
              //   - delays equal or greater to the speed * timeBase are ignored
              // - 1: strict old
              //   - delays equal or greater to the speed are ignored
              // - 2: lax (default)
              //   - no delay is ever ignored unless overridden by another
              bool comparison=(delayBehavior==1)?(effectVal<=nextSpeed):(effectVal<(nextSpeed*(curSubSong->timeBase+1)));
              if (delayBehavior==2) comparison=true;
              if (comparison) {
                // set the delay row, order and timer
                chan[i].rowDelay=effectVal;
                chan[i].delayOrder=whatOrder;
                chan[i].delayRow=whatRow;

                // this here was a compatibility hack for DefleMask...
                // if the delay time happens to be equal to the speed, it'll
                // result in "delay lock" which halts all row processing
                // until another good EDxx effect is found
                // for some reason this didn't occur on Neo Geo...
                // this hack is disabled due to its dirtiness and the fact I
                // don't feel like being compatible with a buggy tracker any further
                if (effectVal==nextSpeed) {
                  //if (sysOfChan[i]!=DIV_SYSTEM_YM2610 && sysOfChan[i]!=DIV_SYSTEM_YM2610_EXT) chan[i].delayLocked=true;
                } else {
                  chan[i].delayLocked=false;
                }

                // once we're done with pre-effects, get out and don't process any further
                returnAfterPre=true;
              } else {
                logV("higher than nextSpeed! %d>%d",effectVal,nextSpeed);
                chan[i].delayLocked=false;
              }
            }
            break;
        }
      }
      // stop processing if EDxx was found
      if (returnAfterPre) return;
    } else {
      //logV("honoring delay at position %d",whatRow);
    }


    // effects
    for (int j=0; j<curPat[i].effectCols; j++) {
      short effect=pat->newData[whatRow][DIV_PAT_FX(j)];
      short effectVal=pat->newData[whatRow][DIV_PAT_FXVAL(j)];

      // an empty effect value is treated as zero
      if (effectVal==-1) effectVal=0;
      effectVal&=255;

      // tempo/tick rate effects
      switch (effect) {
        case 0xc0: case 0xc1: case 0xc2: case 0xc3: // set tick rate
          // Cxxx, where `xxx` is between 1 and 1023
          divider=(double)(((effect&0x3)<<8)|effectVal);
          if (divider<1) divider=1;
          break;
        case 0xf0: // set tempo
          divider=(double)effectVal*2.0/5.0;
          if (divider<1) divider=1;
          cycles=got.rate/divider;
          break;
        case 0xff: // stop song
          shallStopSched=true;
          break;
      }
    }
  };

  auto tinyNextRow=[&]() {
    for (int i=0; i<chans; i++) {
      tinyProcessRow(i,false);
    }

    // mark this row as "walked" over
    wsWalked[((curOrder<<5)+(curRow>>3))&8191]|=1<<(curRow&7);

    // commit a pending jump if there is one
    // otherwise, advance row position
    if (changeOrd!=-1) {
      // jump to order and reset position
      curRow=changePos;
      changePos=0;
      // jump to next order if it is -2
      if (changeOrd==-2) changeOrd=curOrder+1;
      curOrder=changeOrd;

      // if we're out of bounds, return to the beginning
      // if this happens we're guaranteed to loop
      if (curOrder>=ordersLen) {
        curOrder=0;
        ts.isLoopDefined=false;
        endOfSong=true;
        memset(wsWalked,0,8192);
      }
      changeOrd=-1;
    } else if (++curRow>=patLen) {
      // if we are here it means we reached the end of this pattern, so
      // advance to next order unless the song is about to stop
      if (shallStopSched) {
        curRow=patLen-1;
      } else {
        // go to next order
        curRow=0;
        if (++curOrder>=ordersLen) {
          logV("end of orders reached");
          ts.isLoopDefined=false;
          endOfSong=true;
          // the walked array is used for loop detection
          // since we've reached the end, we are guaranteed to loop here, so
          // just reset it.
          memset(wsWalked,0,8192);
          curOrder=0;
        }
      }
    }
    rowChanged=true;

    // new loop detection routine
    // if we're stepping on a row we've already walked over, we found loop
    // if the song is going to stop though, don't do anything
    if (!endOfSong && wsWalked[((curOrder<<5)+(curRow>>3))&8191]&(1<<(curRow&7)) && !shallStopSched) {
      logV("loop reached");
      endOfSong=true;
      memset(wsWalked,0,8192);
    }

    // perform speed alternation
    // COMPAT FLAG: broken speed alternation
    if (song.brokenSpeedSel) {
      unsigned char speed2=(curSpeeds.len>=2)?curSpeeds.val[1]:curSpeeds.val[0];
      unsigned char speed1=curSpeeds.val[0];
      
      // if the pattern length is odd and the current order is odd, use speed 2 for even rows and speed 1 for odd ones
      // we subtract firstPat from curOrder as firstPat is used by a function which finds sub-songs
      // (the beginning of a new sub-song will be in order 0)
      if ((patLen&1) && (curOrder-firstPat)&1) {
        ticks=((curRow&1)?speed2:speed1)*(timeBase+1);
        nextSpeed=(curRow&1)?speed1:speed2;
      } else {
        ticks=((curRow&1)?speed1:speed2)*(timeBase+1);
        nextSpeed=(curRow&1)?speed2:speed1;
      }
    } else {
      // normal speed alternation
      // set the number of ticks and cycle to the next speed
      ticks=curSpeeds.val[curSpeed]*(timeBase+1);
      curSpeed++;
      if (curSpeed>=curSpeeds.len) curSpeed=0;
      // cache the next speed for future operations
      nextSpeed=curSpeeds.val[curSpeed];
    }
  };

  // MAKE IT WORK
  while (!endOfSong) {
    // store the previous position
    prevOrder=curOrder;
    prevRow=curRow;

    // cycle channels to find a tick rate/tempo change effect after delay
    // (unfortunately Cxxx and F0xx are not pre-effects and obey EDxx)
    for (int i=0; i<chans; i++) {
      if (rowDelay[i]>0) {
        if (--rowDelay[i]==0) {
          tinyProcessRow(i,true);
        }
      }
    }

    // run virtual tempo
    tempoAccum+=curVirtualTempoN;
    while (tempoAccum>=curVirtualTempoD) {
      tempoAccum-=curVirtualTempoD;
      // tick counter
      if (--ticks<=0) {
        if (shallStopSched) {
          shallStop=true;
          break;
        } else if (endOfSong) {
          break;
        }

        // next row
        tinyNextRow();
        break;
      }

      // limit tempo accumulator
      if (tempoAccum>1023) tempoAccum=1023;
    }

    if (shallStop) {
      // FFxx found - the song doesn't loop
      ts.isLoopable=false;
      ts.isLoopDefined=false;
      break;
    }

    // update playback time
    double dt=divider*((double)virtualTempoN/(double)MAX(1,virtualTempoD));
    ts.totalTicks++;

    ts.totalMicros+=1000000/dt;
    totalMicrosOff+=fmod(1000000.0,dt);
    while (totalMicrosOff>=dt) {
      totalMicrosOff-=dt;
      ts.totalMicros++;
    }
    if (ts.totalMicros>=1000000) {
      ts.totalMicros-=1000000;
      // who's gonna play a song for 68 years?
      if (ts.totalSeconds<0x7fffffff) ts.totalSeconds++;
    }

    // log row time here
    if (!endOfSong) {
      if (rowChanged) {
        if (ts.orders[curOrder]==NULL) ts.orders[curOrder]=new Timestamp[DIV_MAX_ROWS];
        ts.orders[curOrder][curRow]=Timestamp(ts.totalSeconds,ts.totalMicros);
        rowChanged=false;
      }
    }
    if (maxRow[curOrder]<curRow) maxRow[curOrder]=curRow;
  }

  ts.loopStart.order=curOrder;
  ts.loopStart.row=curRow;
  ts.loopEnd.order=prevOrder;
  ts.loopEnd.row=prevRow;
}

void DivSubSong::clearData() {
  for (int i=0; i<DIV_MAX_CHANS; i++) {
    pat[i].wipePatterns();
  }

  memset(orders.ord,0,DIV_MAX_CHANS*DIV_MAX_PATTERNS);
  ordersLen=1;
}

void DivSubSong::removeUnusedPatterns() {
  for (int i=0; i<DIV_MAX_CHANS; i++) {
    bool used[DIV_MAX_PATTERNS];
    memset(used,0,DIV_MAX_PATTERNS*sizeof(bool));

    for (int j=0; j<ordersLen; j++) {
      used[orders.ord[i][j]]=true;
    }

    for (int j=0; j<DIV_MAX_PATTERNS; j++) {
      if (!used[j] && pat[i].data[j]!=NULL) {
        delete pat[i].data[j];
        pat[i].data[j]=NULL;
      }
    }
  }
}

void DivSubSong::optimizePatterns() {
  for (int i=0; i<DIV_MAX_CHANS; i++) {
    logD("optimizing channel %d...",i);
    std::vector<std::pair<int,int>> clearOuts=pat[i].optimize();
    for (auto& j: clearOuts) {
      for (int k=0; k<DIV_MAX_PATTERNS; k++) {
        if (orders.ord[i][k]==j.first) {
          orders.ord[i][k]=j.second;
        }
      }
    }
  }
}

void DivSubSong::rearrangePatterns() {
  for (int i=0; i<DIV_MAX_CHANS; i++) {
    logD("re-arranging channel %d...",i);
    std::vector<std::pair<int,int>> clearOuts=pat[i].rearrange();
    for (auto& j: clearOuts) {
      for (int k=0; k<DIV_MAX_PATTERNS; k++) {
        if (orders.ord[i][k]==j.first) {
          orders.ord[i][k]=j.second;
        }
      }
    }
  }
}

void DivSubSong::sortOrders() {
  for (int i=0; i<DIV_MAX_CHANS; i++) {
    DivPattern* patPointer[DIV_MAX_PATTERNS];
    unsigned char orderMap[DIV_MAX_PATTERNS];
    bool seen[DIV_MAX_PATTERNS];
    int orderMapLen=0;

    memcpy(patPointer,pat[i].data,DIV_MAX_PATTERNS*sizeof(void*));
    memset(orderMap,0,DIV_MAX_PATTERNS);
    memset(seen,0,DIV_MAX_PATTERNS*sizeof(bool));

    // 1. sort orders
    for (int j=0; j<ordersLen; j++) {
      if (!seen[orders.ord[i][j]]) {
        orderMap[orders.ord[i][j]]=orderMapLen++;
        seen[orders.ord[i][j]]=true;
      }
    }

    // 2. populate the rest
    for (int j=0; j<DIV_MAX_PATTERNS; j++) {
      if (!seen[j]) orderMap[j]=orderMapLen++;
    }
    
    // 3. swap pattern pointers
    for (int j=0; j<DIV_MAX_PATTERNS; j++) {
      pat[i].data[orderMap[j]]=patPointer[j];
    }

    // 4. swap orders
    for (int j=0; j<ordersLen; j++) {
      orders.ord[i][j]=orderMap[orders.ord[i][j]];
    }
  }
}

void DivSubSong::makePatUnique() {
  for (int i=0; i<DIV_MAX_CHANS; i++) {
    logD("making channel %d unique...",i);
    bool seen[DIV_MAX_PATTERNS];
    bool used[DIV_MAX_PATTERNS];

    memset(seen,0,DIV_MAX_PATTERNS*sizeof(bool));
    memset(used,0,DIV_MAX_PATTERNS*sizeof(bool));

    // 1. populate used patterns
    for (int j=0; j<ordersLen; j++) {
      used[orders.ord[i][j]]=true;
    }

    // 2. make patterns unique
    for (int j=0; j<ordersLen; j++) {
      if (seen[orders.ord[i][j]]) {
        for (int k=0; k<DIV_MAX_PATTERNS; k++) {
          if (!used[k]) {
            // copy here
            DivPattern* dest=pat[i].getPattern(k,true);
            DivPattern* src=pat[i].getPattern(orders.ord[i][j],false);
            src->copyOn(dest);
            used[k]=true;
            orders.ord[i][j]=k;
            break;
          }
        }
      } else {
        seen[orders.ord[i][j]]=true;
      }
    }
  }
}

void DivSong::findSubSongs(int chans) {
  std::vector<DivSubSong*> newSubSongs;
  for (DivSubSong* i: subsong) {
    std::vector<int> subSongStart;
    std::vector<int> subSongEnd;
    int loopOrder=0;
    int loopRow=0;
    int loopEnd=-1;
    int curStart=-1;

    // find possible subsongs
    logD("finding subsongs...");
    while (++curStart<i->ordersLen) {
      if (!i->walk(loopOrder,loopRow,loopEnd,chans,jumpTreatment,ignoreJumpAtEnd,curStart)) break;
      
      // make sure we don't pick the same range twice
      if (!subSongEnd.empty()) {
        if (subSongEnd.back()==loopEnd) continue;
      }

      logV("found a subsong: %d-%d",curStart,loopEnd);
      subSongStart.push_back(curStart);
      subSongEnd.push_back(loopEnd);
    }

    // if this is the only song, quit
    if (subSongStart.size()<2) {
      subSongStart.clear();
      subSongEnd.clear();
      newSubSongs.clear();
      continue;
    }

    // now copy the song
    bool isTouched[DIV_MAX_CHANS][DIV_MAX_PATTERNS];
    memset(isTouched,0,DIV_MAX_CHANS*DIV_MAX_PATTERNS*sizeof(bool));
    for (size_t j=1; j<subSongStart.size(); j++) {
      bool isUsed[DIV_MAX_CHANS][DIV_MAX_PATTERNS];
      int start=subSongStart[j];
      int end=subSongEnd[j];

      DivSubSong* theCopy=new DivSubSong;

      theCopy->name=i->name;
      theCopy->notes=i->notes;
      theCopy->hilightA=i->hilightA;
      theCopy->hilightB=i->hilightB;
      theCopy->timeBase=i->timeBase;
      theCopy->arpLen=i->arpLen;
      theCopy->speeds=i->speeds;
      theCopy->virtualTempoN=i->virtualTempoN;
      theCopy->virtualTempoD=i->virtualTempoD;
      theCopy->hz=i->hz;
      theCopy->patLen=i->patLen;

      // copy orders
      memset(isUsed,0,DIV_MAX_CHANS*DIV_MAX_PATTERNS*sizeof(bool));
      for (int k=start, kIndex=0; k<=end; k++, kIndex++) {
        for (int l=0; l<DIV_MAX_CHANS; l++) {
          theCopy->orders.ord[l][kIndex]=i->orders.ord[l][k];
          isUsed[l][i->orders.ord[l][k]]=true;
          isTouched[l][i->orders.ord[l][k]]=true;
        }
      }
      theCopy->ordersLen=end-start+1;
      
      memcpy(theCopy->chanShow,i->chanShow,DIV_MAX_CHANS*sizeof(bool));
      memcpy(theCopy->chanShowChanOsc,i->chanShowChanOsc,DIV_MAX_CHANS*sizeof(bool));
      memcpy(theCopy->chanCollapse,i->chanCollapse,DIV_MAX_CHANS);

      for (int k=0; k<DIV_MAX_CHANS; k++) {
        theCopy->chanName[k]=i->chanName[k];
        theCopy->chanShortName[j]=i->chanShortName[k];

        theCopy->pat[k].effectCols=i->pat[k].effectCols;

        for (int l=0; l<DIV_MAX_PATTERNS; l++) {
          if (i->pat[k].data[l]==NULL) continue;
          if (!isUsed[k][l]) continue;
          DivPattern* origPat=i->pat[k].getPattern(l,false);
          DivPattern* copyPat=theCopy->pat[k].getPattern(l,true);
          origPat->copyOn(copyPat);
        }
      }

      newSubSongs.push_back(theCopy);
    }

    // and cut this one
    i->ordersLen=subSongEnd[0]+1;

    // remove taken patterns as well, as long as they're not used in the original subsong
    // first unmark patterns which are used
    for (int j=subSongStart[0]; j<=subSongEnd[0]; j++) {
      for (int k=0; k<DIV_MAX_CHANS; k++) {
        isTouched[k][i->orders.ord[k][j]]=false;
      }
    }

    // then remove the rest
    for (int j=0; j<DIV_MAX_CHANS; j++) {
      for (int k=0; k<DIV_MAX_PATTERNS; k++) {
        if (isTouched[j][k]) {
          if (i->pat[j].data[k]!=NULL) {
            delete i->pat[j].data[k];
            i->pat[j].data[k]=NULL;
          }
        }
      }
    }
  }

  // append every subsong we found
  for (DivSubSong* i: newSubSongs) {
    subsong.push_back(i);
  }

}

void DivSong::clearSongData() {
  for (DivSubSong* i: subsong) {
    i->clearData();
    delete i;
  }
  subsong.clear();
  subsong.push_back(new DivSubSong);
}

void DivSong::clearInstruments() {
  for (DivInstrument* i: ins) {
    delete i;
  }
  ins.clear();
  insLen=0;
}

void DivSong::clearWavetables() {
  for (DivWavetable* i: wave) {
    delete i;
  }
  wave.clear();
  waveLen=0;
}

void DivSong::clearSamples() {
  for (DivSample* i: sample) {
    delete i;
  }
  sample.clear();
  sampleLen=0;
}

void DivSong::unload() {
  for (DivInstrument* i: ins) {
    delete i;
  }
  ins.clear();
  insLen=0;

  for (DivWavetable* i: wave) {
    delete i;
  }
  wave.clear();
  waveLen=0;

  for (DivSample* i: sample) {
    delete i;
  }
  sample.clear();
  sampleLen=0;

  for (DivSubSong* i: subsong) {
    i->clearData();
    delete i;
  }
  subsong.clear();
}
