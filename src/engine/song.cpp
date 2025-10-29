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
  totalRows(0),
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

void DivSubSong::calcTimestamps(int chans, std::vector<DivGroovePattern>& grooves, int jumpTreatment, int ignoreJumpAtEnd, int brokenSpeedSel, int delayBehavior, int firstPat) {
  // reduced version of the playback routine for calculation.
  std::chrono::high_resolution_clock::time_point timeStart=std::chrono::high_resolution_clock::now();

  // reset state
  ts.totalSeconds=0;
  ts.totalMicros=0;
  ts.totalTicks=0;
  ts.totalRows=0;
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
  bool songWillEnd=false;
  bool endOfSong=false;
  bool rowChanged=false;

  memset(rowDelay,0,DIV_MAX_CHANS);
  memset(delayOrder,0,DIV_MAX_CHANS);
  memset(delayRow,0,DIV_MAX_CHANS);
  if (divider<1) divider=1;

  auto tinyProcessRow=[&,this](int i, bool afterDelay) {
    // if this is after delay, use the order/row where delay occurred
    int whatOrder=afterDelay?delayOrder[i]:curOrder;
    int whatRow=afterDelay?delayRow[i]:curRow;
    DivPattern* p=pat[i].getPattern(orders.ord[i][whatOrder],false);
    // pre effects
    if (!afterDelay) {
      // set to true if we found an EDxx effect
      bool returnAfterPre=false;
      // check all effects
      for (int j=0; j<pat[i].effectCols; j++) {
        short effect=p->newData[whatRow][DIV_PAT_FX(j)];
        short effectVal=p->newData[whatRow][DIV_PAT_FXVAL(j)];

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
            if (effectVal>0) curVirtualTempoN=effectVal;
            break;
          case 0xfe: // virtual tempo den
            if (effectVal>0) curVirtualTempoD=effectVal;
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
              if ((curOrder<(ordersLen-1) || !ignoreJumpAtEnd)) {
                // changeOrd -2 means increase order by 1
                // it overrides a previous 0Bxx effect
                changeOrd=-2;
                changePos=effectVal;
              }
            } else if (jumpTreatment==1) {
              // - 1: old Furnace (same as 2 but ignored if 0Bxx is present)
              if (changeOrd<0 && (curOrder<(ordersLen-1) || !ignoreJumpAtEnd)) {
                changeOrd=-2;
                changePos=effectVal;
              }
            } else {
              // - 0: normal
              if (curOrder<(ordersLen-1) || !ignoreJumpAtEnd) {
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
              bool comparison=(delayBehavior==1)?(effectVal<=nextSpeed):(effectVal<(nextSpeed*(timeBase+1)));
              if (delayBehavior==2) comparison=true;
              if (comparison) {
                // set the delay row, order and timer
                rowDelay[i]=effectVal;
                delayOrder[i]=whatOrder;
                delayRow[i]=whatRow;

                // once we're done with pre-effects, get out and don't process any further
                returnAfterPre=true;
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
    for (int j=0; j<pat[i].effectCols; j++) {
      short effect=p->newData[whatRow][DIV_PAT_FX(j)];
      short effectVal=p->newData[whatRow][DIV_PAT_FXVAL(j)];

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
          break;
        case 0xff: // stop song
          shallStopSched=true;
          break;
      }
    }
  };

  auto tinyNextRow=[&,this]() {
    // store the previous position
    prevOrder=curOrder;
    prevRow=curRow;

    if (songWillEnd) {
      endOfSong=true;
    }

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
        songWillEnd=true;
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
          songWillEnd=true;
          // the walked array is used for loop detection
          // since we've reached the end, we are guaranteed to loop here, so
          // just reset it.
          memset(wsWalked,0,8192);
          curOrder=0;
        }
      }
    }
    rowChanged=true;
    ts.totalRows++;

    // new loop detection routine
    // if we're stepping on a row we've already walked over, we found loop
    // if the song is going to stop though, don't do anything
    if (!songWillEnd && wsWalked[((curOrder<<5)+(curRow>>3))&8191]&(1<<(curRow&7)) && !shallStopSched) {
      logV("loop reached");
      songWillEnd=true;
      memset(wsWalked,0,8192);
    }
    // perform speed alternation
    // COMPAT FLAG: broken speed alternation
    if (brokenSpeedSel) {
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

    if (songWillEnd && !endOfSong) {
      ts.loopEnd.order=prevOrder;
      ts.loopEnd.row=prevRow;
    }
  };

  // MAKE IT WORK
  while (!endOfSong) {
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

    // log row time here
    if (rowChanged && !endOfSong) {
      if (ts.orders[prevOrder]==NULL) {
        ts.orders[prevOrder]=new DivSongTimestamps::Timestamp[DIV_MAX_ROWS];
        for (int i=0; i<DIV_MAX_ROWS; i++) {
          ts.orders[prevOrder][i].seconds=-1;
        }
      }
      ts.orders[prevOrder][prevRow]=DivSongTimestamps::Timestamp(ts.totalSeconds,ts.totalMicros);
      rowChanged=false;
    }

    if (!endOfSong) {
      // update playback time
      double dt=divider;//*((double)virtualTempoN/(double)MAX(1,virtualTempoD));
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
    }
    if (ts.maxRow[curOrder]<curRow) ts.maxRow[curOrder]=curRow;
  }

  ts.totalRows--;
  ts.loopStart.order=prevOrder;
  ts.loopStart.row=prevRow;
  ts.loopStartTime=ts.getTimes(ts.loopStart.order,ts.loopStart.row);

  std::chrono::high_resolution_clock::time_point timeEnd=std::chrono::high_resolution_clock::now();
  logV("calcTimestamps() took %dµs",std::chrono::duration_cast<std::chrono::microseconds>(timeEnd-timeStart).count());
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
    int curStart=-1;

    // find possible subsongs
    logD("finding subsongs...");
    while (++curStart<i->ordersLen) {
      i->calcTimestamps(chans,grooves,jumpTreatment,ignoreJumpAtEnd,brokenSpeedSel,delayBehavior,curStart);
      if (!i->ts.isLoopable) break;
      
      // make sure we don't pick the same range twice
      if (!subSongEnd.empty()) {
        if (subSongEnd.back()==i->ts.loopEnd.order) continue;
      }

      logV("found a subsong: %d-%d",curStart,i->ts.loopEnd.order);
      subSongStart.push_back(curStart);
      subSongEnd.push_back(i->ts.loopEnd.order);
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
