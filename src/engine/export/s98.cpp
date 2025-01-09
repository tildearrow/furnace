/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2024 tildearrow and contributors
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

#include "s98.h"
#include "../engine.h"
#include "../ta-log.h"
#include <fmt/printf.h>

constexpr int MASTER_CLOCK_PREC=(sizeof(void*)==8)?8:0;

static void writeWait(std::vector<uint8_t>& data, unsigned int newWait) {
  if (newWait==1) data.push_back(0xff);
  else if (newWait>1) {
    data.push_back(0xfe);
    newWait-=2;
    do {
      uint8_t val=newWait&0x7f;
      newWait>>=7;
      if (newWait>0) val|=0x80;
      data.push_back(val);
    } while (newWait>0);
  }
}

static void writeCmd(std::vector<uint8_t>& data, DivSystem sys, uint8_t cmdID, unsigned int addr, uint8_t val) {
  if (cmdID==0xff) return;
  if (addr>=0x100) cmdID++;
  data.insert(data.end(),{cmdID,(uint8_t)(addr&0xff),val});
}

void DivExportS98::run() {
  SafeWriter* w;

  // config
  float tickRate=conf.getFloat("s98rate",e->getHz());
  bool loop=conf.getBool("loop",true);
  int trailingTicks=conf.getInt("trailingTicks",-1);
  std::vector<int> toExport=conf.getIntList("toExport",{});
  if (toExport.empty()) {
    logAppend("ERROR: No systems selected for S98");
    failed=true;
    running=false;
    return;
  }
  int dataPos=0;
  int loopPos=-1;

  // write header
  int rateNum=10;
  int rateDenom=tickRate*rateNum;
  while (rateNum<1000000000 && (tickRate*rateNum-(float)rateDenom>1e-6f)) {
    rateNum*=10;
    rateDenom=tickRate*rateNum;
  }
  tickRate=(float)rateDenom/rateNum;
  w=new SafeWriter;
  w->init();
  w->write("S983",4);
  w->writeI(rateNum);
  w->writeI(rateDenom);
  w->writeI(0); // compression, unused
  w->writeI(0); // tag offset, will be written later
  w->writeI(0); // data offset, will be written later
  w->writeI(0); // loop offset, will be written later
  w->writeI(toExport.size());
  for (int i: toExport) {
    DivDispatch* dispatch=e->disCont[i].dispatch;
    DivConfig& flags=e->song.systemFlags[i];
    int sys=0;
    switch (e->song.system[i]) {
      case DIV_SYSTEM_AY8910:
        // S98 permanently has half clock for YM2149F, so convert to AY-3-8910 for those without it
        // S5B has half clock regardless of the flag
        if (flags.getInt("chipType",0)==2 || flags.getBool("halfClock",false)) {
          sys=1;
        } else {
          sys=15;
        }
        break;
      case DIV_SYSTEM_YM2203:
      case DIV_SYSTEM_YM2203_CSM:
      case DIV_SYSTEM_YM2203_EXT:
        sys=2;
        break;
      case DIV_SYSTEM_YM2612:
      case DIV_SYSTEM_YM2612_CSM:
      case DIV_SYSTEM_YM2612_EXT:
      case DIV_SYSTEM_YM2612_DUALPCM:
      case DIV_SYSTEM_YM2612_DUALPCM_EXT:
        sys=3;
        break;
      case DIV_SYSTEM_YM2608:
      case DIV_SYSTEM_YM2608_CSM:
      case DIV_SYSTEM_YM2608_EXT:
        sys=4;
        break;
      case DIV_SYSTEM_YM2151:
        sys=5;
        break;
      case DIV_SYSTEM_OPLL:
      case DIV_SYSTEM_OPLL_DRUMS:
        sys=6;
        break;
      case DIV_SYSTEM_OPL:
      case DIV_SYSTEM_OPL_DRUMS:
        sys=7;
        break;
      case DIV_SYSTEM_OPL2:
      case DIV_SYSTEM_OPL2_DRUMS:
        sys=8;
        break;
      case DIV_SYSTEM_OPL3:
      case DIV_SYSTEM_OPL3_DRUMS:
        sys=9;
        break;
      case DIV_SYSTEM_SMS:
        sys=16;
        break;
      default: break;
    }
    int pan=0;
    int mixPan=0;
    if (e->song.systemPan[i]<-0.5f) mixPan=0b10;
    else if (e->song.systemPan[i]>0.5f) mixPan=0b01;
    switch (e->song.system[i]) {
      case DIV_SYSTEM_AY8910:
      case DIV_SYSTEM_YM2203:
      case DIV_SYSTEM_YM2203_CSM:
      case DIV_SYSTEM_YM2203_EXT:
        pan=mixPan*0b01010101;
        if (flags.getBool("stereo",false)) pan|=0b010010;
        break;
      case DIV_SYSTEM_OPLL:
      case DIV_SYSTEM_OPLL_DRUMS:
      case DIV_SYSTEM_OPL:
      case DIV_SYSTEM_OPL_DRUMS:
      case DIV_SYSTEM_OPL2:
      case DIV_SYSTEM_OPL2_DRUMS:
      case DIV_SYSTEM_SMS:
        pan=mixPan;
        break;
      default: break;
    }
    w->writeI(sys);
    w->writeI(dispatch->chipClock);
    w->writeI(pan);
    w->writeI(0); // reserved
  }
  dataPos=w->tell();

  std::vector<uint8_t> data;
  e->stop();
  e->repeatPattern=false;
  e->shallStop=false;
  e->setOrder(0);
  e->synchronizedSoft([this, &data, tickRate, loop, trailingTicks, toExport, &loopPos]() {
    double origRate=e->got.rate;
    e->got.rate=tickRate;

    // determine loop point
    int loopOrder=0;
    int loopRow=0;
    int loopEnd=0;
    e->walkSong(loopOrder,loopRow,loopEnd);
    logAppendf("loop point: %d %d",loopOrder,loopRow);

    // reset the playback state
    e->curOrder=0;
    e->freelance=false;
    e->playing=false;
    e->extValuePresent=false;
    e->remainingLoops=-1;

    // render samples
    uint8_t cmdIDs[DIV_MAX_CHIPS];
    memset(cmdIDs,0xff,sizeof(cmdIDs));
    for (size_t idx=0; idx<toExport.size(); idx++) {
      int i=toExport[idx];
      if (i>=0 && i<DIV_MAX_CHIPS) cmdIDs[i]=idx*2;
      else continue;
      e->disCont[i].dispatch->toggleRegisterDump(true);
      // Unlike VGM, S98 doesn't have a provision for initial RAM data
      // So we need to write them as register write commands...
      switch (e->song.system[i]) {
        case DIV_SYSTEM_YM2608:
        case DIV_SYSTEM_YM2608_CSM:
        case DIV_SYSTEM_YM2608_EXT: {
          DivDispatch* dis=e->disCont[i].dispatch;
          size_t memLen=dis->getSampleMemUsage(0);
          if (memLen==0) break;
          const uint8_t* mem=(const uint8_t*)dis->getSampleMem(0);
          uint8_t a1=idx*2+1;
          data.insert(data.end(),{
            a1,0x10,0x80, // reset flags
            a1,0x00,0x61, // record to mem, reset
            a1,0x00,0x60, // release reset
            a1,0x01,0x02, // 8-bit DRAM
            a1,0x0c,0xff, // limit to 256K
            a1,0x0d,0x1f, //
            a1,0x02,0x00, // start addr = 0
            a1,0x03,0x00, //
            a1,0x04,(uint8_t)(((memLen-1)>>5)&0xff),  // stop addr
            a1,0x05,(uint8_t)(((memLen-1)>>13)&0xff), //
          });
          for (size_t j=0; j<memLen; j++) {
            data.insert(data.end(),{a1,0x08,mem[j]});
          }
          data.insert(data.end(),{
            a1,0x00,0x00, // stop recording
            a1,0x10,0x80, // reset flags
          });
          break;
        }
        default: break;
      }
      progress[0].amount=(float)idx/toExport.size();
    }

    progress[0].amount=1.f;

    // Prepare to write song data
    unsigned int totalWait=0;
    bool writeLoop=false;
    bool alreadyWroteLoop=false;
    bool done=false;
    std::vector<size_t> tickPos;
    std::vector<int> tickSample;
    bool trailing=false;
    bool beenOneLoopAlready=false;
    int countDown=MAX(0,trailingTicks)+1;
    std::vector<std::pair<int,DivDelayedWrite>> sortedWrites;

    e->playSub(false);

    while (!done) {
      if (mustAbort) {
        logAppend("aborted!");
        failed=true;
        running=false;
        return;
      }
      if (loopPos==-1) {
        if (loopOrder==e->curOrder && loopRow==e->curRow) {
          if ((e->ticks-((e->tempoAccum+e->virtualTempoN)/e->virtualTempoD))<=0) {
            writeLoop=true;
          }
        }
      }
      tickPos.push_back(data.size());
      if (e->nextTick(false,true)) {
        if (trailing) beenOneLoopAlready=true;
        trailing=true;
        if (!loop) countDown=0;
        for (int i=0; i<e->chans; i++) {
        if (cmdIDs[e->dispatchOfChan[i]]==0xff) continue;
          e->chan[i].wentThroughNote=false;
        }
      }
      if (trailing) {
        switch (trailingTicks) {
          case -1: { // automatic
            bool stillHaveTo=false;
            for (int i=0; i<e->chans; i++) {
              if (cmdIDs[e->dispatchOfChan[i]]==0xff) continue;
              if (e->chan[i].goneThroughNote) continue;
              if (e->chan[i].wentThroughNote) {
                stillHaveTo=true;
                break;
              }
            }
            if (!stillHaveTo) countDown=0;
            break;
          }
          case -2: // one loop
            break;
          default: // custom
            countDown--;
            break;
        }
        if (e->song.loopModality!=2) countDown=0;
      }
      if (countDown<=0 || !e->playing || beenOneLoopAlready) {
        done=true;
        if (!loop) {
          for (int i=0; i<e->song.systemLen; i++) {
            e->disCont[i].dispatch->getRegisterWrites().clear();
          }
          break;
        }
        if (!e->playing) {
          writeLoop=false;
          loopPos=-1;
        }
      }

      // calculate number of samples in this tick
      int wait=e->cycles>>MASTER_CLOCK_PREC;

      // get register dumps and put them into delayed writes
      int writeNum=0;
      for (int i=0; i<e->song.systemLen; i++) {
        int curDelay=0;
        std::vector<DivRegWrite>& writes=e->disCont[i].dispatch->getRegisterWrites();
        for (DivRegWrite& j: writes) {
          if (j.addr==0xfffffffe) { // delay
            curDelay+=(double)j.val*(tickRate/(double)e->disCont[i].dispatch->rate);
            if (curDelay>wait) curDelay=wait-1;
          } else {
            sortedWrites.push_back(std::pair<int,DivDelayedWrite>(i,DivDelayedWrite(curDelay,writeNum++,j.addr,j.val)));
          }
        }
        writes.clear();
      }

      // put writes
      if (!sortedWrites.empty()) {
        // sort writes
        std::sort(sortedWrites.begin(),sortedWrites.end(),[](const std::pair<int,DivDelayedWrite>& a, const std::pair<int,DivDelayedWrite>& b) -> bool {
          if (a.second.time==b.second.time) {
            return a.second.order<b.second.order;
          }
          return a.second.time<b.second.time;
        });

        // write it out
        int lastOne=0;
        for (std::pair<int,DivDelayedWrite>& i: sortedWrites) {
          if (i.second.time>lastOne) {
            // write delay
            totalWait+=i.second.time-lastOne;
            lastOne=i.second.time;
          }
          // write write
          uint8_t cmdID=cmdIDs[i.first];
          if (cmdID==0xff) continue;
          DivSystem sys=e->song.system[i.first];
          writeWait(data,totalWait);
          totalWait=0;
          if (i.second.write.addr==0xffffffff) { // Furnace fake reset
            for (auto& j: e->generateResetWrites(sys)) {
              writeCmd(data,sys,cmdID,j.addr,j.val);
            }
          }
          else writeCmd(data,sys,cmdID,i.second.write.addr,i.second.write.val);
        }
        sortedWrites.clear();
        wait-=lastOne;
      }

      // write wait
      totalWait+=wait;
      if (writeLoop && !alreadyWroteLoop) {
        writeWait(data,totalWait);
        totalWait=0;
        writeLoop=false;
        alreadyWroteLoop=true;
        loopPos=data.size();
      }
    }
    // end of song
    writeWait(data,totalWait);
    data.push_back(0xfd);

    e->got.rate=origRate;
    for (int i: toExport) {
      e->disCont[i].dispatch->toggleRegisterDump(false);
    }

    e->remainingLoops=-1;
    e->playing=false;
    e->freelance=false;
    e->extValuePresent=false;
  });

  logAppend("writing data...");
  progress[1].amount=0.95f;
  w->write(data.data(),data.size());

  // write tags
  int tagPos=w->tell();
  String notes;
  for (char i: e->song.notes) {
    if (i=='\n') notes.append("\ncomment=");
    else notes.push_back(i);
  }
  // UTF-8 BOM is written here so players won't decode tags in Shift-JIS
  w->writeString(fmt::format(
    "[S98]\xef\xbb\xbf"
    "title={}\n"
    "artist={}\n"
    "game={}\n"
    "system={}\n"
    "s98by=Furnace (chiptune tracker)\n"
    "comment={}\n"
    "\0",
    e->song.name,e->song.author,e->song.category,e->song.systemName,notes
  ),false);

  // finalize header
  w->seek(0x10,SEEK_SET);
  w->writeI(tagPos);
  w->writeI(dataPos);
  w->writeI(loopPos==-1?0:loopPos);

  output.emplace_back("out.s98",w);
  progress[1].amount=1.0f;
  logAppend("finished!");
  running=false;
}

bool DivExportS98::go(DivEngine* eng) {
  progress[0].name="Samples";
  progress[0].amount=0.0f;
  progress[1].name="Song Data";
  progress[1].amount=0.0f;

  e=eng;
  running=true;
  failed=false;
  mustAbort=false;
  exportThread=new std::thread(&DivExportS98::run,this);
  return true;
}

void DivExportS98::wait() {
  if (exportThread!=NULL) {
    exportThread->join();
    delete exportThread;
  }
}

void DivExportS98::abort() {
  mustAbort=true;
  wait();
}

bool DivExportS98::isRunning() {
  return running;
}

bool DivExportS98::hasFailed() {
  return failed;
}

DivROMExportProgress DivExportS98::getProgress(int index) {
  if (index<0 || index>2) return progress[2];
  return progress[index];
}
