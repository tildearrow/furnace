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

#include "sid.h"
#include "../engine.h"
#include "../ta-log.h"
#include "../../fileutils.h"
#include "../../executils.h"
#include "../platform/c64.h"
#include <fmt/printf.h>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include "../../utfutils.h"
#else
#include <dirent.h>
#include <unistd.h>
#endif

/**
 * a write group for each channel's flags.
 */
struct WriteGroup {
  unsigned char data[3];
  unsigned char enable;

  String toString() const {
    String ret=fmt::sprintf("%.2x %.2x %.2x",data[0],data[1],data[2]);
    for (int i=0; i<3; i++) {
      if (!(enable&(1<<i))) {
        ret[i*3]='-';
        ret[1+(i*3)]='-';
      }
    }
    return ret;
  }
  inline bool operator<(const WriteGroup& other) const {
    return (memcmp((const void*)&data,(const void*)&other,4)<0);
  }
  WriteGroup():
    data{0},
    enable(0) {}
};

/**
 * used to keep track of likely absolute writes (notes and abs duty/cutoff) and relative ones (vibrato/slides).
 */
struct DeltaWrite {
  unsigned short val;
  bool absol;
  DeltaWrite(unsigned short v, bool a=true):
    val(v),
    absol(a) {}
};

struct DeltaMapped {
  int popularity, rank;
  bool used;
  DeltaMapped& operator++() {
    ++popularity;
    return *this;
  }
  DeltaMapped(int p, int r, bool u=false):
    popularity(p),
    rank(r),
    used(u) {}
  DeltaMapped():
    popularity(0),
    rank(-1),
    used(0) {}
};

struct DataBlock {
  int* data;
  size_t len;
  int useCount;
  DataBlock(int* d, size_t l, int uc):
    data(d),
    len(l),
    useCount(uc) {}
  DataBlock():
    data(NULL),
    len(0),
    useCount(0) {}
};

// compressed chan flags/ADSR regs format:
// 00-7F: call group + next tick
// 80-EF: call sub-block
// F0-FB: preset wait (F0 is always one tick)
// FC: call group ext (16-bit index follows) + next tick
// FD: call sub-block addr (16-bit addr follows)
// FE: wait (8-bit delay follows)
// FF: stop/loop/ret from sub-block

void writeData(SafeWriter* w, int* data, size_t len) {
  int inDelay=0;
  for (size_t i=0; i<len; i++) {
    switch (data[i]) {
      case -1:
        inDelay++;
        break;
      case -2:
      case -3:
        if (inDelay) {
          if (inDelay>=12) {
            w->writeC(0xfe);
            w->writeC(inDelay);
          } else {
            w->writeC(0xf0+inDelay);
          }
          inDelay=0;
        }
        w->writeC(0xff);
        break;
      case -4:
        // nop
        break;
      default:
        if (inDelay) {
          if (inDelay>=12) {
            w->writeC(0xfe);
            w->writeC(inDelay);
          } else {
            w->writeC(0xf0+inDelay);
          }
          inDelay=0;
        }
        if (data[i]&0x80000000) {
          w->writeC(0x80|(data[i]&0x7f));
        } else {
          w->writeC(data[i]&0x7f);
        }
    }
  }
}

size_t stripNops(int* ptr, size_t len) {
  size_t ret=0;
  for (size_t i=0; i<len; i++) {
    if (i!=ret) ptr[ret]=ptr[i];
    if (ptr[i]!=-4) ret++;
  }
  return ret;
}

void sortDeltas(std::map<unsigned short,DeltaMapped>& deltas) {
  int sortRank=0;
  while (true) {
    auto best=deltas.end();
    int bestPopularity=0;
    for (auto i=deltas.begin(); i!=deltas.end(); i++) {
      if (i->second.rank!=-1) continue;
      if (i->second.popularity>bestPopularity) {
        best=i;
        bestPopularity=i->second.popularity;
      }
    }
    if (best==deltas.end()) break;
    logV("%d. %.4x: %d",sortRank+1,best->first,best->second.popularity);
    best->second.rank=sortRank++;
  }
}

void DivExportC64::run() {
  SafeWriter* w=new SafeWriter;
  w->init(); 

  // play the song and dump registers
  // then we'll compress each channel appropriately
  // frequency won't be stored as such and we'll have a tiny vibrato/porta engine instead to save space

  e->stop();
  e->repeatPattern=false;
  e->setOrder(0);

  logAppend("playing and logging register writes...");

  std::vector<WriteGroup> chWrites[4];
  std::vector<DeltaWrite> chDuty[3];
  std::vector<DeltaWrite> chFreq[3];
  std::vector<DeltaWrite> cutoffWrites;

  std::map<WriteGroup,int> writePopularity;
  std::vector<WriteGroup> writePopularitySorted;

  std::map<unsigned short,DeltaMapped> freqPopularity;
  std::map<unsigned short,DeltaMapped> freqDeltaPopularity;
  std::map<unsigned short,DeltaMapped> dutyPopularity;
  std::map<unsigned short,DeltaMapped> dutyDeltaPopularity;

  int* allWrites;
  size_t allWritesLen=0;
  size_t chWritesOff[4];

  int* allFreqs;
  size_t allFreqsLen=0;
  size_t chFreqsOff[4];

  e->synchronizedSoft([&]() {
    // Determine loop point.
    e->calcSongTimestamps();
    int loopOrder=e->curSubSong->ts.loopStart.order;
    int loopRow=e->curSubSong->ts.loopStart.row;
    logAppendf("loop point: %d %d",loopOrder,loopRow);
    e->warnings="";

    // Reset the playback state.
    e->curOrder=0;
    e->freelance=false;
    e->playing=false;
    e->extValuePresent=false;
    e->remainingLoops=-1;

    // Prepare to write song data.
    e->playSub(false);
    bool done=false;
    e->disCont[0].dispatch->toggleRegisterDump(true);

    unsigned char state[32];
    unsigned char prevState[32];
    memset(state,0,32);
    memset(prevState,0,32);

    while (!done) {
      if (e->nextTick(false,true) || !e->playing) {
        done=true;
        for (int i=0; i<e->song.systemLen; i++) {
          e->disCont[i].dispatch->getRegisterWrites().clear();
        }
        break;
      }
      // get register dumps
      std::vector<DivRegWrite>& writes=e->disCont[0].dispatch->getRegisterWrites();
      if (!writes.empty()) {
        for (DivRegWrite& write: writes) {
          //logV("%x = %x",write.addr,write.val);
          state[write.addr&0x1f]=write.val;
        }
        writes.clear();
      }

      // collect channel writes
      for (int i=0; i<3; i++) {
        WriteGroup wg;
        for (int j=(7*i)+4, j_b=0; j<(7*i)+7; j++, j_b++) {
          if (state[j]!=prevState[j]) {
            wg.data[j_b]=state[j];
            wg.enable|=1<<j_b;
          }
        }

        if (wg.enable) writePopularity[wg]++;
        chWrites[i].push_back(wg);
      }

      // collect global writes
      WriteGroup wg;
      for (int j=0x17, j_b=0; j<=0x18; j++, j_b++) {
        if (state[j]!=prevState[j]) {
          wg.data[j_b]=state[j];
          wg.enable|=1<<j_b;
        }
      }

      if (wg.enable) writePopularity[wg]++;
      chWrites[3].push_back(wg);

      // collect frequency/duty writes
      for (int i=0; i<3; i++) {
        DivPlatformC64::Channel* ch=(DivPlatformC64::Channel*)e->getDispatchChanState(i);
        
        chFreq[i].push_back(DeltaWrite(state[7*i]|(state[(7*i)+1]<<8),ch->freqWasAbs));
        chDuty[i].push_back(DeltaWrite(state[(7*i)+2]|((state[(7*i)+3]&15)<<8),ch->dutyWasAbs));
      }

      // collect cutoff writes
      cutoffWrites.push_back((state[0x16]<<3)|(state[0x15]&7));

      memcpy(prevState,state,32);
    }
    // end of song

    // done - close out.
    e->disCont[0].dispatch->toggleRegisterDump(false);

    e->remainingLoops=-1;
    e->playing=false;
    e->freelance=false;
    e->extValuePresent=false;
  });

  unsigned short lastFreq[3];
  lastFreq[0]=0;
  lastFreq[1]=0;
  lastFreq[2]=0;
  for (size_t i=0; i<chFreq[0].size(); i++) {
    for (int j=0; j<3; j++) {
      unsigned short delta=chFreq[j][i].val-lastFreq[j];
      lastFreq[j]=chFreq[j][i].val;
      if (delta==0) continue;
      ++freqPopularity[chFreq[j][i].val];
      ++freqDeltaPopularity[delta];
    }
  }

  unsigned short lastDuty[3];
  lastDuty[0]=0;
  lastDuty[1]=0;
  lastDuty[2]=0;
  for (size_t i=0; i<chDuty[0].size(); i++) {
    for (int j=0; j<3; j++) {
      unsigned short delta=(chDuty[j][i].val-lastDuty[j])&0xfff;
      lastDuty[j]=chDuty[j][i].val;
      if (delta==0) continue;
      ++dutyPopularity[chDuty[j][i].val];
      ++dutyDeltaPopularity[delta];
    }
  }

  // sort frequencies and duties by popularity
  logV("frequency popularity:");
  sortDeltas(freqPopularity);

  logV("frequency delta popularity:");
  sortDeltas(freqDeltaPopularity);

  logV("duty popularity:");
  sortDeltas(dutyPopularity);

  logV("duty delta popularity:");
  sortDeltas(dutyDeltaPopularity);

  // sort writes by popularity
  logV("write popularity:");
  while (!writePopularity.empty()) {
    WriteGroup mostPopular;
    int score=0;
    for (auto& i: writePopularity) {
      if (i.second>score) {
        mostPopular=i.first;
        score=i.second;
      }
    }
    logV("%d. %s (%d)",(int)writePopularitySorted.size(),mostPopular.toString(),score);
    writePopularitySorted.push_back(mostPopular);
    writePopularity.erase(mostPopular);
  }

  // put it back into the map so we can easily index these
  int index=0;
  for (WriteGroup& i: writePopularitySorted) {
    writePopularity[i]=index++;
  }

  // prepare to map frequency writes
  logD("calculating initial frequency size...");
  for (int i=0; i<3; i++) {
    // +1 for end of data marker
    allFreqsLen+=chFreq[i].size()+1;
  }
  allFreqs=new int[allFreqsLen];
  logV("%d entries",(int)allFreqsLen);

  // map frequency writes
  logD("mapping frequency writes...");
  int* allFreqsPtr=allFreqs;
  for (int i=0; i<3; i++) {
    chFreqsOff[i]=allFreqsPtr-allFreqs;
    DeltaMapped* mapped=NULL;
    DeltaMapped* mappedDelta=NULL;

    logV("------- CHANNEL %d -------",i);
    for (size_t _j=0; _j<chFreq[i].size(); _j++) {
      DeltaWrite& j=chFreq[i][_j];

      // the idea is to check which is more convenient to use.
      // delta, or absolute.
      // for this we have to check the next write, and see whether we have identical delta values.
      // if not, then check which is more popular, and use that.

      try {
        mapped=&freqPopularity[j.val];
      } catch (std::exception& e) {
        mapped=NULL;
        logW("missing entry for write!");
      }

      // compare
      if (_j==0) {
        // the first value is always absolute
        *allFreqsPtr=mapped->rank;
        mapped->used=true;
      } else {
        // check whether we have a change
        DeltaWrite& j_prev=chFreq[i][_j-1];
        if (j_prev.val==j.val) {
          *allFreqsPtr=-1;
        } else {
          // find the delta value's rank
          try {
            mappedDelta=&freqDeltaPopularity[(j.val-j_prev.val)&0xffff];
          } catch (std::exception& e) {
            mappedDelta=NULL;
            logW("missing entry for write!");
          }

          // check which is more popular
          if (mappedDelta->popularity>mapped->popularity) {
            *allFreqsPtr=0x40000000|mappedDelta->rank;
            mappedDelta->used=true;
          } else {
            *allFreqsPtr=mapped->rank;
            mapped->used=true;
          }
        }
      }

      logV("%.8x",*allFreqsPtr);

      allFreqsPtr++;
    }
    // end of data marker
    *(allFreqsPtr++)=-2;
    chFreq[i].clear();
    logV("chFreqsOff[%d]: %d",i,(int)chFreqsOff[i]);
  }

  // prepare to conglomerate status writes
  logD("calculating initial status size...");
  for (int i=0; i<4; i++) {
    // +1 for end of data marker
    allWritesLen+=chWrites[i].size()+1;
  }
  allWrites=new int[allWritesLen];
  logV("%d entries",(int)allWritesLen);

  // conglomerate writes
  logD("mapping status writes...");
  int* allWritesPtr=allWrites;
  for (int i=0; i<4; i++) {
    chWritesOff[i]=allWritesPtr-allWrites;
    for (WriteGroup& j: chWrites[i]) {
      if (j.enable) {
        try {
          *allWritesPtr=writePopularity[j];
        } catch (std::exception& e) {
          logW("missing entry for write!");
          *allWritesPtr=-1;
        }
      } else {
        *allWritesPtr=-1;
      }
      allWritesPtr++;
    }
    // end of data marker
    *(allWritesPtr++)=-2;
    chWrites[i].clear();
    logV("chWritesOff[%d]: %d",i,(int)chWritesOff[i]);
  }

  // find patterns and compress
  logD("compressing...");
  std::vector<DataBlock> blocks;

  bool foundBlock=true;
  bool ignoreCalls=true;
  while (foundBlock) {
    int bestBenefit=0;
    size_t bestSize=0;
    size_t bestPos=0;
    foundBlock=false;
    logD("current data size: %d",(int)allWritesLen);
    logD("finding blocks...");
    for (size_t size=2; size<(ignoreCalls?16:16); size++) {
      logD("size %d...",(int)size);
      for (size_t i=0; i<allWritesLen-size; i++) {
        // check whether this block may be searched for:
        // - it shouldn't contain exclusively wait commands
        // - it shouldn't contain special commands
        // - it's best it does not begin with a wait
        bool isValid=false;
        bool areWeWaiting=false;
        int numOfWaits=0;
        for (size_t j=i; j<i+size; j++) {
          //if (j==i) {
            //if (allWrites[j]==-1) break;
          //}
          if (allWrites[j]>=0) {
            isValid=true;
            areWeWaiting=false;
          } else if (allWrites[j]==-1) { // wait one tick
            if (areWeWaiting) numOfWaits++;
            areWeWaiting=true;
          } else if (allWrites[j]==-2 || allWrites[j]==-3 || allWrites[j]==-4) { // other commands/blocks
            isValid=false;
            break;
          } else if (allWrites[j]&0x80000000 && ignoreCalls) {
            isValid=false;
            break;
          } else {
            isValid=true;
            areWeWaiting=false;
          }
        }

        if (!isValid) continue;

        // otherwise begin block search
        int matchCount=0;
        for (size_t j=i+size; j<allWritesLen-size; j++) {
          if (memcmp(&allWrites[j],&allWrites[i],size*sizeof(int))==0) {
            matchCount++;
            j+=size-1;
          }
        }

        // calculate benefit
        if (matchCount>1) {
          // -1 for ret command
          // -2 for entry in call table
          int benefit=(matchCount*(size-1-numOfWaits))-3;
          if (benefit>bestBenefit) {
            bestBenefit=benefit;
            bestSize=size;
            bestPos=i;
            foundBlock=true;
            logV("- %x (benefit %d)",i,benefit);
          }
        }
      }
    }

    // isolate block and begin replacing around
    if (foundBlock) {
      logD("best block: %x (%d bytes; benefit %d)",bestPos,(int)bestSize,bestBenefit);

      // copy block
      size_t newBlockLen=bestSize+1;
      int* newBlock=new int[newBlockLen];
      int useCount=0;

      memcpy(newBlock,&allWrites[bestPos],bestSize*sizeof(int));
      newBlock[bestSize]=-3; // ret

      // replace occurrences of this block
      for (size_t j=bestPos; j<allWritesLen-bestSize; j++) {
        if (memcmp(&allWrites[j],newBlock,bestSize*sizeof(int))==0) {
          for (size_t k=j; k<j+bestSize; k++) {
            allWrites[k]=-4; // nop
            // block index
            if (k==j) allWrites[k]=0x80000000|(int)blocks.size();
          }

          useCount++;
          j+=bestSize-1;
        }
      }

      logD("pushing... (%d)",(int)blocks.size());
      blocks.push_back(DataBlock(newBlock,newBlockLen,useCount));
      if (blocks.size()>=112) {
        logD("block limit reached!");
        break;
      }
    } else {
      if (ignoreCalls) {
        logD("no more blocks - now considering calls...");
        ignoreCalls=false;
        foundBlock=true;
      } else {
        logD("no more blocks");
      }
    }

    allWritesLen=stripNops(allWrites,allWritesLen);
  }

  writeData(w,allWrites,allWritesLen);
  for (DataBlock& i: blocks) {
    writeData(w,i.data,i.len);
  }

  output.push_back(DivROMExportOutput("export.sid",w));

  logAppend("finished!");

  running=false;
}

bool DivExportC64::go(DivEngine* eng) {
  progress[0].name="Progress";
  progress[0].amount=0.0f;

  e=eng;
  running=true;
  failed=false;
  mustAbort=false;
  exportThread=new std::thread(&DivExportC64::run,this);
  return true;
}

void DivExportC64::wait() {
  if (exportThread!=NULL) {
    exportThread->join();
    delete exportThread;
  }
}

void DivExportC64::abort() {
  mustAbort=true;
  wait();
}

bool DivExportC64::isRunning() {
  return running;
}

bool DivExportC64::hasFailed() {
  return failed;
}

DivROMExportProgress DivExportC64::getProgress(int index) {
  if (index<0 || index>1) return progress[1];
  return progress[index];
}
