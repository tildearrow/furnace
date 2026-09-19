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

#include "engine.h"
#include "bsr.h"
#include <fmt/printf.h>

bool DivEngine::supportedByS98(DivSystem which) {
  switch (which) {
    case DIV_SYSTEM_AY8910:
    case DIV_SYSTEM_YM2203:
    case DIV_SYSTEM_YM2203_CSM:
    case DIV_SYSTEM_YM2203_EXT:
    case DIV_SYSTEM_YM2608:
    case DIV_SYSTEM_YM2608_CSM:
    case DIV_SYSTEM_YM2608_EXT:
    case DIV_SYSTEM_YM2612:
    case DIV_SYSTEM_YM2612_CSM:
    case DIV_SYSTEM_YM2612_EXT:
    case DIV_SYSTEM_YM2612_DUALPCM:
    case DIV_SYSTEM_YM2612_DUALPCM_EXT:
    case DIV_SYSTEM_YM2151:
    case DIV_SYSTEM_OPLL:
    case DIV_SYSTEM_OPLL_DRUMS:
    case DIV_SYSTEM_OPL:
    case DIV_SYSTEM_OPL_DRUMS:
    case DIV_SYSTEM_OPL2:
    case DIV_SYSTEM_OPL2_DRUMS:
    case DIV_SYSTEM_OPL3:
    case DIV_SYSTEM_OPL3_DRUMS:
    case DIV_SYSTEM_SMS:
      return true;
    default:
      return false;
  }
  return false;
}

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

SafeWriter* DivEngine::saveS98(float tickRate, bool* sysToExport, bool loop, int trailingTicks) {
  SafeWriter* w;
  warnings="";

  // config
  std::vector<int> toExport;

  for (int i=0; i<song.systemLen; i++) {
    if (sysToExport!=NULL) {
      if (!sysToExport[i]) continue;
    }
    if (!supportedByS98(song.system[i])) continue;
    toExport.push_back(i);
  }

  if (toExport.empty()) {
    logE("No systems selected for S98");
    lastError="No systems selected for S98";
    return NULL;
  }

  if (tickRate<1.0f) {
    // automatic - detect the tick rate
    // items in this vector are stored as array*5 to facilitate GCD calculation
    std::vector<unsigned int> tickRateChanges;

    // start with the song's tick rate
    float hz5=curSubSong->hz*5.0f;
    float curTickRate=curSubSong->hz;
    bool firstRow=false;
    bool giveUp=false;

    auto addTickRateChange=[&tickRateChanges,&curTickRate](unsigned int hz) {
      // discard rates too low
      if (hz<1) return;
      curTickRate=(float)hz/5.0f;
      // discard duplicates
      for (unsigned int& i: tickRateChanges) {
        if (i==hz) return;
      }
      // insert new rate
      tickRateChanges.push_back(hz);
      logD("adding tick rate change (%.1f)",(float)hz/5.0f);
    };

    // check whether YM2612 DualPCM is present
    // if so then assume a high tick rate
    for (int i: toExport) {
      if (song.system[i]==DIV_SYSTEM_YM2612_DUALPCM || song.system[i]==DIV_SYSTEM_YM2612_DUALPCM_EXT) {
        logD("YM2612 DualPCM detected - using high rate");
        giveUp=true;
        break;
      }
    }

    // scan the song for tick rate changes and PCM usage
    if (!giveUp) for (int i=0; i<curSubSong->ordersLen; i++) {
      for (int j=0; j<curSubSong->patLen; j++) {
        for (int k=0; k<song.chans; k++) {
          DivPattern* pat=curSubSong->pat[k].getPattern(i,false);

          // find tick rate effects
          for (int l=0; l<curSubSong->pat[k].effectCols; l++) {
            if (pat->newData[j][DIV_PAT_FX(l)]==0xf0) { // F0xx - set tempo
              addTickRateChange(pat->newData[j][DIV_PAT_FXVAL(l)]*2);
            } else if ((pat->newData[j][DIV_PAT_FX(l)]&0xfc)==0xc0) { // Cxxx - set tick rate
              addTickRateChange(5*(pat->newData[j][DIV_PAT_FXVAL(l)]|((pat->newData[j][DIV_PAT_FX(l)]&3)<<8)));
            }
          }
          // push the initial tick rate (it may have changed at the very beginning of the song, so that's why we do it here)
          if (!firstRow) {
            if (tickRateChanges.empty()) {
              // if tickRate*5 is not an integer then push an artificially high rate to skip LCM calculation
              float fracPart=hz5-(int)hz5;
              if (fracPart>0.001 && fracPart<0.999) {
                giveUp=true;
              } else {
                addTickRateChange(hz5);
              }
            }
            firstRow=true;
          }

          // check for high speed writes (AY PCM/TFX and YM2612 DAC)
          bool doCheck=false;
          for (int l: toExport) {
            if (song.dispatchOfChan[k]==l) {
              doCheck=true;
              break;
            }
          }
          if (doCheck && pat->newData[j][DIV_PAT_INS]!=-1) {
            DivInstrument* ins=getIns(pat->newData[j][DIV_PAT_INS]);
            bool isItAY=(
              song.sysOfChan[k]==DIV_SYSTEM_AY8910 ||
              (song.sysOfChan[k]==DIV_SYSTEM_YM2203 && song.dispatchChanOfChan[k]>=3 && song.dispatchChanOfChan[k]<6) ||
              (song.sysOfChan[k]==DIV_SYSTEM_YM2203_EXT && song.dispatchChanOfChan[k]>=6 && song.dispatchChanOfChan[k]<9) ||
              (song.sysOfChan[k]==DIV_SYSTEM_YM2203_CSM && song.dispatchChanOfChan[k]>=7 && song.dispatchChanOfChan[k]<10) ||
              (song.sysOfChan[k]==DIV_SYSTEM_YM2608 && song.dispatchChanOfChan[k]>=6 && song.dispatchChanOfChan[k]<9) ||
              (song.sysOfChan[k]==DIV_SYSTEM_YM2608_EXT && song.dispatchChanOfChan[k]>=9 && song.dispatchChanOfChan[k]<12) ||
              (song.sysOfChan[k]==DIV_SYSTEM_YM2608_CSM && song.dispatchChanOfChan[k]>=10 && song.dispatchChanOfChan[k]<13) ||
              (song.sysOfChan[k]==DIV_SYSTEM_YM2610_FULL && song.dispatchChanOfChan[k]>=4 && song.dispatchChanOfChan[k]<7) ||
              (song.sysOfChan[k]==DIV_SYSTEM_YM2610_FULL_EXT && song.dispatchChanOfChan[k]>=7 && song.dispatchChanOfChan[k]<10) ||
              (song.sysOfChan[k]==DIV_SYSTEM_YM2610_CSM && song.dispatchChanOfChan[k]>=8 && song.dispatchChanOfChan[k]<11) ||
              (song.sysOfChan[k]==DIV_SYSTEM_YM2610B && song.dispatchChanOfChan[k]>=6 && song.dispatchChanOfChan[k]<9) ||
              (song.sysOfChan[k]==DIV_SYSTEM_YM2610B_EXT && song.dispatchChanOfChan[k]>=9 && song.dispatchChanOfChan[k]<12) ||
              (song.sysOfChan[k]==DIV_SYSTEM_YM2610B_CSM && song.dispatchChanOfChan[k]>=10 && song.dispatchChanOfChan[k]<13)
            );

            if (
              (ins->type==DIV_INS_AMIGA || ins->amiga.useSample) && (
                isItAY ||
                (song.sysOfChan[k]==DIV_SYSTEM_YM2612 && song.dispatchChanOfChan[k]==5) ||
                (song.sysOfChan[k]==DIV_SYSTEM_YM2612_EXT && song.dispatchChanOfChan[k]==8) ||
                (song.sysOfChan[k]==DIV_SYSTEM_YM2612_CSM && song.dispatchChanOfChan[k]==8)
              )
            ) {
              // sample mode is enabled - use a high rate
              logD("AY PCM/YM2612 DAC detected - using high rate");
              giveUp=true;
            } else if (isItAY) {
              // is timer FX enabled?
              if (ins->std.ex6Macro.len>0) {
                // if so then we need a high tick rate
                logD("AY TFX detected - using high rate");
                giveUp=true;
              }
            }
          }

          if (giveUp) break;
        }
        if (giveUp) break;
      }
      if (giveUp) break;
    }

    if (giveUp) {
      // assume high rate
      tickRate=50000.0f;
    } else if (tickRateChanges.size()<2) {
      // no tick rate changes - use song tick rate
      tickRate=curTickRate;
    } else {
      // calculate least common multiplier of all rates
      unsigned int cur=tickRateChanges[0];
      for (unsigned int& i: tickRateChanges) {
        // don't calculate LCM if it's too large as the multiplication may fail
        if (cur>=50000) break;
        if (cur==0 || i==0) break;
        cur=(cur*i)/gcd2(cur,i);
      }

      // limit the final tick rate to 10000Hz
      if (cur>=50000) cur=50000;

      tickRate=(float)cur/5.0f;
    }
    logI("estimated global tick rate: %fHz",tickRate);
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
  logI("rate: %d/%d",rateNum,rateDenom);
  logI("final tick rate: %fHz",tickRate);
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
    DivDispatch* dispatch=disCont[i].dispatch;
    DivConfig& flags=song.systemFlags[i];
    int sys=0;
    switch (song.system[i]) {
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
    if (song.systemPan[i]<-0.5f) mixPan=0b10;
    else if (song.systemPan[i]>0.5f) mixPan=0b01;
    switch (song.system[i]) {
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
  stop();
  repeatPattern=false;
  shallStop=false;
  setOrder(0);
  synchronizedSoft([this, &data, tickRate, loop, trailingTicks, toExport, &loopPos]() {
    std::vector<DivDelayedWrite> delayedWrites[DIV_MAX_CHIPS];
    double origRate=got.rate;
    got.rate=tickRate;

    // determine loop point
    calcSongTimestamps();
    int loopOrder=curSubSong->ts.loopStart.order;
    int loopRow=curSubSong->ts.loopStart.row;
    logD("loop point: %d %d",loopOrder,loopRow);

    // reset the playback state
    curOrder=0;
    freelance=false;
    playing=false;
    extValuePresent=false;
    remainingLoops=-1;

    // render samples
    uint8_t cmdIDs[DIV_MAX_CHIPS];
    memset(cmdIDs,0xff,sizeof(cmdIDs));
    for (size_t idx=0; idx<toExport.size(); idx++) {
      int i=toExport[idx];
      if (i>=0 && i<DIV_MAX_CHIPS) cmdIDs[i]=idx*2;
      else continue;
      disCont[i].dispatch->toggleRegisterDump(true);
      // Unlike VGM, S98 doesn't have a provision for initial RAM data
      // So we need to write them as register write commands...
      switch (song.system[i]) {
        case DIV_SYSTEM_YM2608:
        case DIV_SYSTEM_YM2608_CSM:
        case DIV_SYSTEM_YM2608_EXT: {
          DivDispatch* dis=disCont[i].dispatch;
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
    }

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

    playSub(false);

    while (!done) {
      if (loopPos==-1) {
        if (loopOrder==curOrder && loopRow==curRow) {
          if ((ticks-((tempoAccum+virtualTempoN)/virtualTempoD))<=0) {
            writeLoop=true;
          }
        }
      }
      tickPos.push_back(data.size());
      if (nextTick(false,true)) {
        if (trailing) beenOneLoopAlready=true;
        trailing=true;
        if (!loop) countDown=0;
        for (int i=0; i<song.chans; i++) {
        if (cmdIDs[song.dispatchOfChan[i]]==0xff) continue;
          chan[i].wentThroughNote=false;
        }
      }
      if (trailing) {
        switch (trailingTicks) {
          case -1: { // automatic
            bool stillHaveTo=false;
            for (int i=0; i<song.chans; i++) {
              if (cmdIDs[song.dispatchOfChan[i]]==0xff) continue;
              if (chan[i].goneThroughNote) continue;
              if (chan[i].wentThroughNote) {
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
        if (song.compatFlags.loopModality!=2) countDown=0;
      }
      if (countDown<=0 || !playing || beenOneLoopAlready) {
        done=true;
        if (!loop) {
          for (int i=0; i<song.systemLen; i++) {
            disCont[i].dispatch->getRegisterWrites().clear();
          }
          break;
        }
        if (!playing) {
          writeLoop=false;
          loopPos=-1;
        }
      }

      // calculate number of samples in this tick
      int wait=cycles;

      // get register dumps and put them into delayed writes
      int writeNum=0;
      for (int i=0; i<song.systemLen; i++) {
        int curDelay=0;
        std::vector<DivRegWrite>& writes=disCont[i].dispatch->getRegisterWrites();
        for (DivRegWrite& j: writes) {
          if (j.addr==0xfffffffe) { // delay
            curDelay+=(double)j.val*(tickRate/(double)disCont[i].dispatch->rate);
            if (curDelay>wait) curDelay=wait-1;
          } else {
            sortedWrites.push_back(std::pair<int,DivDelayedWrite>(i,DivDelayedWrite(curDelay,writeNum++,j.addr,j.val)));
          }
        }
        writes.clear();
      }

      // render stream of all chips
      for (int i=0; i<song.systemLen; i++) {
        disCont[i].dispatch->fillStream(delayedWrites[i],tickRate,wait);
        for (DivDelayedWrite& j: delayedWrites[i]) {
          sortedWrites.push_back(std::pair<int,DivDelayedWrite>(i,j));
        }
        delayedWrites[i].clear();
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
          DivSystem sys=song.system[i.first];
          writeWait(data,totalWait);
          totalWait=0;
          writeCmd(data,sys,cmdID,i.second.write.addr,i.second.write.val);
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

    got.rate=origRate;
    for (int i: toExport) {
      disCont[i].dispatch->toggleRegisterDump(false);
    }

    remainingLoops=-1;
    playing=false;
    freelance=false;
    extValuePresent=false;
  });

  logI("writing data...");
  w->write(data.data(),data.size());

  // write tags
  int tagPos=w->tell();
  String notes;
  for (char i: song.notes) {
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
    song.name,song.author,song.category,song.systemName,notes
  ),false);

  // finalize header
  w->seek(0x10,SEEK_SET);
  w->writeI(tagPos);
  w->writeI(dataPos);
  //w->writeI(0);
  w->writeI((loopPos==-1 || !loop)?0:loopPos);

  logI("finished!");
  return w;
}
