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

#include "tiuna.h"
#include "../engine.h"
#include "../ta-log.h"
#include <fmt/printf.h>
#include <algorithm>
#include <map>
#include <tuple>
#include <vector>

struct TiunaNew {
  short pitch;
  signed char ins;
  signed char vol;
  short sync;
  TiunaNew():
    pitch(-1),
    ins(-1),
    vol(-1),
    sync(-1) {}
};

struct TiunaLast {
  short pitch;
  signed char ins;
  signed char vol;
  int tick;
  bool forcePitch;
  TiunaLast():
    pitch(0),
    ins(0),
    vol(0),
    tick(1),
    forcePitch(true) {}
};

struct TiunaCmd {
  signed char pitchChange;
  short pitchSet;
  signed char ins;
  signed char vol;
  short sync;
  short wait;
  TiunaCmd():
    pitchChange(-1),
    pitchSet(-1),
    ins(-1),
    vol(-1),
    sync(-1),
    wait(-1) {}
};

struct TiunaBytes {
  unsigned char ch;
  int ticks;
  unsigned char size;
  unsigned char buf[16];
  friend bool operator==(const TiunaBytes& l, const TiunaBytes& r) {
    if (l.size!=r.size) return false;
    if (l.ticks!=r.ticks) return false;
    return memcmp(l.buf,r.buf,l.size)==0;
  }
  TiunaBytes(unsigned char c, int t, unsigned char s, std::initializer_list<unsigned char> b):
    ch(c),
    ticks(t),
    size(s) {
    // because C++14 does not support data() on initializer_list
    unsigned char p=0;
    for (unsigned char i: b) {
      buf[p++]=i;
    }
  }
  TiunaBytes():
    ch(0),
    ticks(0),
    size(0) {
    memset(buf,0,16);
  }
};

struct TiunaMatch {
  int pos;
  int endPos;
  int size;
  int id;
  TiunaMatch(int p, int ep, int s, int _i):
    pos(p),
    endPos(ep),
    size(s),
    id(_i) {}
  TiunaMatch():
    pos(0),
    endPos(0),
    size(0),
    id(0) {}
};

struct TiunaMatches {
  int bytesSaved;
  int length;
  int ticks;
  std::vector<int> pos;
  TiunaMatches():
    bytesSaved(INT32_MIN),
    length(0),
    ticks(0) {}
};

static void writeCmd(std::vector<TiunaBytes>& cmds, TiunaCmd& cmd, unsigned char ch, int& lastWait, int fromTick, int toTick) {
  while (fromTick<toTick) {
    int val=MIN(toTick-fromTick,256);
    if (lastWait!=val) {
      cmd.wait=val;
      lastWait=val;
    }
    TiunaBytes nbuf;
    unsigned char vcw1=0x80;
    unsigned char vcw2=0;
    unsigned char vcwLen=0;
    unsigned char nlen=0;
    nbuf.ch=ch;
    nbuf.ticks=val;
    if (cmd.sync>=0) {
      nbuf.buf[nlen++]=0b00111110;
      nbuf.buf[nlen++]=cmd.sync;
    }
    if (cmd.wait>=17) {
      nbuf.buf[nlen++]=0b00111111;
      nbuf.buf[nlen++]=cmd.wait-1;
    }
    if (cmd.pitchChange>=0) {
      nbuf.buf[nlen++]=0b01000000|cmd.pitchChange;
    }
    if (cmd.pitchSet>=0) {
      nbuf.buf[nlen++]=0b01100000|(cmd.pitchSet>>8);
      nbuf.buf[nlen++]=cmd.pitchSet&0xff;
    }
    if (cmd.vol>=1) {
      vcw1|=0x40|cmd.vol;
      vcwLen++;
    }
    if (cmd.ins>=0) {
      vcw1|=0x20;
      if (vcwLen==0) vcw1|=cmd.ins;
      else vcw2=cmd.ins;
      vcwLen++;
    }
    if (cmd.wait>=1 && cmd.wait<17) {
      unsigned char val=cmd.wait-1;
      vcw1|=0x10;
      if (vcwLen==0) vcw1|=val;
      else if (vcwLen==1) vcw2=val;
      else vcw2|=(val<<4);
      vcwLen++;
    }
    nbuf.buf[nlen++]=vcw1;
    if (vcwLen>=2) nbuf.buf[nlen++]=vcw2;
    nbuf.size=nlen;
    cmds.push_back(nbuf);
    cmd=TiunaCmd();
    fromTick+=val;
  }
}

void DivExportTiuna::run() {
  int loopOrder, loopOrderRow, loopEnd;
  int tick=0;
  SafeWriter* w;
  std::map<int,TiunaCmd> allCmds[2];

  // config
  String baseLabel=conf.getString("baseLabel","song");
  int firstBankSize=conf.getInt("firstBankSize",3072);
  int otherBankSize=conf.getInt("otherBankSize",4096-48);
  int tiaIdx=conf.getInt("sysToExport",-1);

  e->stop();
  e->repeatPattern=false;
  e->shallStop=false;
  e->setOrder(0);
  e->synchronizedSoft([&]() {
    // determine loop point
    // bool stopped=false;
    loopOrder=0;
    loopOrderRow=0;
    loopEnd=0;
    e->walkSong(loopOrder,loopOrderRow,loopEnd);
    logAppendf("loop point: %d %d",loopOrder,loopOrderRow);

    w=new SafeWriter;
    w->init();

    if (tiaIdx<0 || tiaIdx>=e->song.systemLen) {
      tiaIdx=-1;
      for (int i=0; i<e->song.systemLen; i++) {
        if (e->song.system[i]==DIV_SYSTEM_TIA) {
          tiaIdx=i;
          break;
        }
      }
      if (tiaIdx<0) {
        logAppend("ERROR: selected TIA system not found");
        failed=true;
        running=false;
        return;
      }
    } else if (e->song.system[tiaIdx]!=DIV_SYSTEM_TIA) {
      logAppend("ERROR: selected chip is not a TIA!");
      failed=true;
      running=false;
      return;
    }

    e->disCont[tiaIdx].dispatch->toggleRegisterDump(true);

    // write patterns
    // bool writeLoop=false;
    logAppend("recording sequence...");
    bool done=false;
    e->playSub(false);
    
    // int loopTick=-1;
    TiunaLast last[2];
    TiunaNew news[2];
    while (!done) {
      // TODO implement loop
      // if (loopTick<0 && loopOrder==curOrder && loopOrderRow==curRow
      //   && (ticks-((tempoAccum+virtualTempoN)/virtualTempoD))<=0
      // ) {
      //   writeLoop=true;
      //   loopTick=tick;
      //   // invalidate last register state so it always force an absolute write after loop
      //   for (int i=0; i<2; i++) {
      //     last[i]=TiunaLast();
      //     last[i].pitch=-1;
      //     last[i].ins=-1;
      //     last[i].vol=-1;
      //   }
      // }
      if (e->nextTick(false,true) || !e->playing) {
        // stopped=!playing;
        done=true;
        break;
      }
      for (int i=0; i<2; i++) {
        news[i]=TiunaNew();
      }
      // get register dumps
      std::vector<DivRegWrite>& writes=e->disCont[tiaIdx].dispatch->getRegisterWrites();
      for (const DivRegWrite& i: writes) {
        switch (i.addr) {
          case 0xfffe0000:
          case 0xfffe0001:
            news[i.addr&1].pitch=i.val;
            break;
          case 0xfffe0002:
            news[0].sync=i.val;
            break;
          case 0x15:
          case 0x16:
            news[i.addr-0x15].ins=i.val;
            break;
          case 0x19:
          case 0x1a:
            news[i.addr-0x19].vol=i.val;
            break;
          default: break;
        }
      }
      writes.clear();
      // collect changes
      for (int i=0; i<2; i++) {
        TiunaCmd cmds;
        bool hasCmd=false;
        if (news[i].pitch>=0 && (last[i].forcePitch || news[i].pitch!=last[i].pitch)) {
          int dt=news[i].pitch-last[i].pitch;
          if (!last[i].forcePitch && abs(dt)<=16) {
            if (dt<0) cmds.pitchChange=15-dt;
            else cmds.pitchChange=dt-1;
          }
          else cmds.pitchSet=news[i].pitch;
          last[i].pitch=news[i].pitch;
          last[i].forcePitch=false;
          hasCmd=true;
        }
        if (news[i].ins>=0 && news[i].ins!=last[i].ins) {
          cmds.ins=news[i].ins;
          last[i].ins=news[i].ins;
          hasCmd=true;
        }
        if (news[i].vol>=0 && news[i].vol!=last[i].vol) {
          cmds.vol=(news[i].vol-last[i].vol)&0xf;
          last[i].vol=news[i].vol;
          hasCmd=true;
        }
        if (news[i].sync>=0) {
          cmds.sync=news[i].sync;
          hasCmd=true;
        }
        if (hasCmd) allCmds[i][tick]=cmds;
      }
      e->cmdStream.clear();
      tick++;
    }
    for (int i=0; i<e->song.systemLen; i++) {
      e->disCont[i].dispatch->getRegisterWrites().clear();
      e->disCont[i].dispatch->toggleRegisterDump(false);
    }

    e->remainingLoops=-1;
    e->playing=false;
    e->freelance=false;
    e->extValuePresent=false;
  });

  if (failed) return;

  // render commands
  logAppend("rendering commands...");
  std::vector<TiunaBytes> renderedCmds;
  w->writeText(fmt::format(
    "; Generated by Furnace " DIV_VERSION "\n"
    "; Name:   {}\n"
    "; Author: {}\n"
    "; Album:  {}\n"
    "; Subsong #{}: {}\n\n",
    e->song.name,e->song.author,e->song.category,e->curSubSongIndex+1,e->curSubSong->name
  ));
  for (int i=0; i<2; i++) {
    TiunaCmd lastCmd;
    int lastTick=0;
    int lastWait=0;
    // bool looped=false;
    for (auto& kv: allCmds[i]) {
      // if (!looped && !stopped && loopTick>=0 && kv.first>=loopTick) {
      //   writeCmd(w,&lastCmd,&lastWait,loopTick-lastTick);
      //   w->writeText(".loop\n");
      //   lastTick=loopTick;
      //   looped=true;
      // }
      writeCmd(renderedCmds,lastCmd,i,lastWait,lastTick,kv.first);
      lastTick=kv.first;
      lastCmd=kv.second;
    }
    writeCmd(renderedCmds,lastCmd,i,lastWait,lastTick,tick);
    // if (stopped || loopTick<0) w->writeText(".loop\n    db 0\n");
  }
  // compress commands
  std::vector<TiunaMatch> confirmedMatches;
  std::vector<int> callTicks;
  int cmId=0;
  int cmdSize=renderedCmds.size();
  bool* processed=new bool[cmdSize];
  memset(processed,0,cmdSize*sizeof(bool));
  logAppend("compressing! this may take a while.");
  int maxCmId=(MAX(firstBankSize/1024,1))*256;
  int lastMaxPMVal=100000;
  logAppendf("max cmId: %d",maxCmId);
  logAppendf("commands: %d",cmdSize);
  while (firstBankSize>768 && cmId<maxCmId) {
    if (mustAbort) {
      logAppend("aborted!");
      failed=true;
      running=false;
      delete[] processed;
      return;
    }

    float theOtherSide=pow(1.0/float(MAX(1,lastMaxPMVal)),0.2)*0.98;
    progress[0].amount=theOtherSide+(1.0-theOtherSide)*((float)cmId/(float)maxCmId);

    logAppendf("start CM %04x...",cmId);
    std::map<int,TiunaMatches> potentialMatches;
    for (int i=0; i<cmdSize-1;) {
      // continue and skip if it's part of previous confirmed matches
      while (i<cmdSize-1 && processed[i]) i++;
      if (i>=cmdSize-1) break;
      progress[1].amount=(float)i/(float)(cmdSize-1);
      std::vector<TiunaMatch> match;
      int ch=renderedCmds[i].ch;
      for (int j=i+1; j<cmdSize;) {
        if (processed[i]) break;
        //while (j<cmdSize && processed[i]) j++;
        if (j>=cmdSize) break;
        int k=0;
        int ticks=0;
        int size=0;
        while (
          (i+k)<j && (i+k)<cmdSize && (j+k)<cmdSize &&
          (ticks+renderedCmds[i+k].ticks)<=256 &&
          // match runs can't cross channels
          // as channel end command would be insterted there later
          renderedCmds[i+k].ch==ch &&
          renderedCmds[j+k].ch==ch &&
          renderedCmds[i+k]==renderedCmds[j+k] &&
          !processed[i+k] && !processed[j+k]
        ) {
          ticks+=renderedCmds[i+k].ticks;
          size+=renderedCmds[i+k].size;
          k++;
        }
        if (size>2) match.push_back(TiunaMatch(j,j+k,size,0));
        if (k==0) k++;
        j+=k;
      }
      if (match.empty()) {
        i++;
        continue;
      }
      // find a length that results in most bytes saved
      TiunaMatches matches;
      int curSize=0;
      int curLength=1;
      int curTicks=0;
      while (true) {
        int bytesSaved=-4;
        bool found=false;
        for (const TiunaMatch& j: match) {
          if ((j.endPos-j.pos)>=curLength) {
            if (!found) {
              found=true;
              curSize+=renderedCmds[i+curLength-1].size;
              curTicks+=renderedCmds[i+curLength-1].ticks;
            }
            bytesSaved+=curSize-2;
          }
        }
        if (!found) break;
        if (bytesSaved>matches.bytesSaved) {
          matches.length=curLength;
          matches.bytesSaved=bytesSaved;
          matches.ticks=curTicks;
        }
        curLength++;
      }
      if (matches.bytesSaved>0) {
        matches.pos.push_back(i);
        for (const TiunaMatch& j: match) {
          if ((j.endPos-j.pos)>=matches.length) {
            matches.pos.push_back(j.pos);
          }
        }
        potentialMatches[i]=matches;
      }
      i++;
    }
    if (potentialMatches.empty()) {
      logAppend("potentialMatches is empty");
      break;
    }
    int maxPMIdx=0;
    int maxPMVal=0;
    for (const auto& i: potentialMatches) {
      if (i.second.bytesSaved>maxPMVal) {
        maxPMVal=i.second.bytesSaved;
        maxPMIdx=i.first;
      }
    }
    int maxPMLen=potentialMatches[maxPMIdx].length;
    for (const int i: potentialMatches[maxPMIdx].pos) {
      confirmedMatches.push_back({i,i+maxPMLen,0,cmId});
      memset(processed+i,1,maxPMLen);
      //std::fill(processed.begin()+i,processed.begin()+(i+maxPMLen),true);
    }
    callTicks.push_back(potentialMatches[maxPMIdx].ticks);
    logAppendf("CM %04x added: pos=%d,len=%d,matches=%d,saved=%d",cmId,maxPMIdx,maxPMLen,potentialMatches[maxPMIdx].pos.size(),maxPMVal);
    lastMaxPMVal=maxPMVal;
    cmId++;
  }
  progress[0].amount=1.0f;
  progress[1].amount=1.0f;
  logAppend("generating data...");
  delete[] processed;
  std::sort(confirmedMatches.begin(),confirmedMatches.end(),[](const TiunaMatch& l, const TiunaMatch& r){
    return l.pos<r.pos;
  });
  // ignore last call IDs >256 that don't fill up a page
  // as they tends to increase the final size due to page alignment
  int cmIdLen=cmId>256?(cmId&~255):cmId;
  // overlap check
  for (int i=1; i<(int)confirmedMatches.size(); i++) {
    if (confirmedMatches[i-1].endPos<=confirmedMatches[i].pos) continue;
    logAppend("ERROR: impossible overlap found in matches list, please report");
    failed=true;
    running=false;
    return;
  }
  SafeWriter dbg;
  dbg.init();
  dbg.writeText(fmt::format("renderedCmds size={}\n",renderedCmds.size()));
  for (const auto& i: confirmedMatches) {
    dbg.writeText(fmt::format("pos={},end={},id={}\n",i.pos,i.endPos,i.id,i.size));
  }

  // write commands
  int totalSize=0;
  int cnt=cmIdLen;
  w->writeText(fmt::format("    .section {0}_bank0\n    .align $100\n{0}_calltable",baseLabel));
  while (cnt>0) {
    int cnt2=MIN(cnt,256);
    w->writeText("\n    .byte ");
    for (int j=0; j<cnt2; j++) {
      w->writeText(fmt::format("<{}_c{},",baseLabel,cmIdLen-cnt+j));
    }
    for (int j=cnt2; j<256; j++) {
      w->writeText("0,");
    }
    w->seek(-1,SEEK_CUR);
    w->writeText("\n    .byte ");
    for (int j=0; j<cnt2; j++) {
      w->writeText(fmt::format(">{}_c{},",baseLabel,cmIdLen-cnt+j));
    }
    for (int j=cnt2; j<256; j++) {
      w->writeText("0,");
    }
    w->seek(-1,SEEK_CUR);
    w->writeText("\n    .byte ");
    for (int j=0; j<cnt2; j++) {
      w->writeText(fmt::format("{}_c{}>>13,",baseLabel,cmIdLen-cnt+j));
    }
    for (int j=cnt2; j<256; j++) {
      w->writeText("0,");
    }
    w->seek(-1,SEEK_CUR);
    w->writeText("\n    .byte ");
    for (int j=0; j<cnt2; j++) {
      w->writeText(fmt::format("{},",callTicks[cmIdLen-cnt+j]&0xff));
    }
    w->seek(-1,SEEK_CUR);
    totalSize+=768+cnt2;
    cnt-=cnt2;
  }
  w->writeC('\n');
  if (totalSize>firstBankSize) {
    logAppend("ERROR: first bank is not large enough to contain call table");
    failed=true;
    running=false;
    return;
  }

  int curBank=0;
  int bankSize=totalSize;
  int maxBankSize=firstBankSize;
  int curCh=-1;
  std::vector<bool> callVisited=std::vector<bool>(cmIdLen,false);
  auto cmIter=confirmedMatches.begin();
  for (int i=0; i<(int)renderedCmds.size(); i++) {
    int writeCall=-1;
    TiunaBytes cmd=renderedCmds[i];
    if (cmIter!=confirmedMatches.end() && i==cmIter->pos) {
      if (cmIter->id<cmIdLen) {
        if (callVisited[cmIter->id]) {
          unsigned char idLo=cmIter->id&0xff;
          unsigned char idHi=cmIter->id>>8;
          cmd=TiunaBytes(cmd.ch,0,2,{idHi,idLo});
          i=cmIter->endPos-1;
        } else {
          writeCall=cmIter->id;
          callVisited[writeCall]=true;
        }
      }
      cmIter++;
    }
    if (cmd.ch!=curCh) {
      if (curCh>=0) {
        w->writeText("    .text x\"e0\"\n");
        totalSize++;
        bankSize++;
      }
      if (bankSize+cmd.size>=maxBankSize) {
        maxBankSize=otherBankSize;
        curBank++;
        w->writeText(fmt::format("    .endsection\n\n    .section {}_bank{}",baseLabel,curBank));
        bankSize=0;
      }
      w->writeText(fmt::format("\n{}_ch{}\n",baseLabel,cmd.ch));
      curCh=cmd.ch;
    }
    if (bankSize+cmd.size+1>=maxBankSize) {
      maxBankSize=otherBankSize;
      curBank++;
      w->writeText(fmt::format("    .text x\"c0\"\n    .endsection\n\n    .section {}_bank{}\n",baseLabel,curBank));
      totalSize++;
      bankSize=0;
    }
    if (writeCall>=0) {
      w->writeText(fmt::format("{}_c{}\n",baseLabel,writeCall));
    }
    w->writeText("    .text x\"");
    for (int j=0; j<cmd.size; j++) {
      w->writeText(fmt::format("{:02x}",cmd.buf[j]));
    }
    w->writeText("\"\n");
    totalSize+=cmd.size;
    bankSize+=cmd.size;
  }
  w->writeText("    .text x\"e0\"\n    .endsection\n");
  totalSize++;
  logAppendf("total size: %d bytes (%d banks)",totalSize,curBank+1);

  output.push_back(DivROMExportOutput("export.asm",w));
  
  logAppend("finished!");

  running=false;
}

bool DivExportTiuna::go(DivEngine* eng) {
  progress[0].name="Compression";
  progress[0].amount=0.0f;
  progress[1].name="Confirmed Matches";
  progress[1].amount=0.0f;

  e=eng;
  running=true;
  failed=false;
  mustAbort=false;
  exportThread=new std::thread(&DivExportTiuna::run,this);
  return true;
}

void DivExportTiuna::wait() {
  if (exportThread!=NULL) {
    exportThread->join();
    delete exportThread;
  }
}

void DivExportTiuna::abort() {
  mustAbort=true;
  wait();
}

bool DivExportTiuna::isRunning() {
  return running;
}

bool DivExportTiuna::hasFailed() {
  return failed;
}

DivROMExportProgress DivExportTiuna::getProgress(int index) {
  if (index<0 || index>2) return progress[2];
  return progress[index];
}
