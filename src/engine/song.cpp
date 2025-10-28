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

void DivSubSong::findLength(int loopOrder, int loopRow, double fadeoutLen, int& rowsForFadeout, bool& hasFFxx, std::vector<int>& orders_vec, std::vector<DivGroovePattern>& grooves, int& length, int& loopTick, int chans, int jumpTreatment, int ignoreJumpAtEnd, int firstPat) {
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
  int tickCounter = 0; //how many ticks passed since the start of song

  int curGrooveIndex = 0;
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
      if (i == loopOrder && j == loopRow) {
          loopTick = tickCounter;
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

      // Calculate ticks for this row
      int ticksThisRow = 6; // default

      if (curSpeeds.len > 0) {
          ticksThisRow = curSpeeds.val[curGrooveIndex % curSpeeds.len];
          curGrooveIndex++;
      }

      tickCounter += ticksThisRow;

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
