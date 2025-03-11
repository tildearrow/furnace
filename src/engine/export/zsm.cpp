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

#include "zsm.h"
#include "../engine.h"
#include "../ta-log.h"
#include <fmt/printf.h>

/// DivZSM definitions

#define ZSM_HEADER_SIZE 16
#define ZSM_VERSION 1
#define ZSM_YM_CMD 0x40
#define ZSM_DELAY_CMD 0x80
#define ZSM_YM_MAX_WRITES 63
#define ZSM_SYNC_MAX_WRITES 31
#define ZSM_DELAY_MAX 127
#define ZSM_EOF ZSM_DELAY_CMD

#define ZSM_EXT ZSM_YM_CMD
#define ZSM_EXT_PCM 0x00
#define ZSM_EXT_CHIP 0x40
#define ZSM_EXT_SYNC 0x80
#define ZSM_EXT_CUSTOM 0xC0

enum YM_STATE { ym_PREV, ym_NEW, ym_STATES };
enum PSG_STATE { psg_PREV, psg_NEW, psg_STATES };

class DivZSM {
  private:
    struct S_pcmInst {
      int geometry;
      unsigned int offset, length, loopPoint;
      bool isLooped;
    };
    SafeWriter* w;
    int ymState[ym_STATES][256];
    int psgState[psg_STATES][64];
    int pcmRateCache;
    int pcmCtrlRVCache;
    int pcmCtrlDCCache;
    unsigned int pcmLoopPointCache;
    bool pcmIsLooped;
    std::vector<DivRegWrite> ymwrites;
    std::vector<DivRegWrite> pcmMeta;
    std::vector<unsigned char> pcmData;
    std::vector<unsigned char> pcmCache;
    std::vector<S_pcmInst> pcmInsts;
    std::vector<DivRegWrite> syncCache;
    int loopOffset;
    int numWrites;
    int ticks;
    int tickRate;
    int ymMask;
    int psgMask;
    bool optimize;
  public:
    DivZSM();
    ~DivZSM();
    void init(unsigned int rate = 60);
    int getoffset();
    void writeYM(unsigned char a, unsigned char v);
    void writePSG(unsigned char a, unsigned char v);
    void writePCM(unsigned char a, unsigned char v);
    void writeSync(unsigned char a, unsigned char v);
    void setOptimize(bool o);
    void tick(int numticks = 1);
    void setLoopPoint();
    SafeWriter* finish();
  private:
    void flushWrites();
    void flushTicks();
};

/// DivZSM implementation

DivZSM::DivZSM() {
  w=NULL;
  init();
}

DivZSM::~DivZSM() {
}

void DivZSM::init(unsigned int rate) {
  if (w!=NULL) delete w;
  w=new SafeWriter;
  w->init();
  // write default ZSM data header
  w->write("zm",2); // magic header
  w->writeC(ZSM_VERSION);
  // no loop offset
  w->writeS(0);
  w->writeC(0);
  // no PCM
  w->writeS(0x00);
  w->writeC(0x00);
  // FM channel mask
  w->writeC(0x00);
  // PSG channel mask
  w->writeS(0x00);
  w->writeS((unsigned short)rate);
  // 2 reserved bytes (set to zero)
  w->writeS(0x00);
  tickRate=rate;
  loopOffset=-1;
  numWrites=0;
  ticks=0;
  // Initialize YM/PSG states
  memset(&ymState,-1,sizeof(ymState));
  memset(&psgState,-1,sizeof(psgState));
  // Initialize PCM states
  pcmRateCache=-1;
  pcmCtrlRVCache=-1;
  pcmCtrlDCCache=-1;
  pcmIsLooped=false;
  pcmLoopPointCache=0;
  // Channel masks
  ymMask=0;
  psgMask=0;
  // Optimize writes
  optimize=true;
}

int DivZSM::getoffset() {
  return w->tell();
}

void DivZSM::writeYM(unsigned char a, unsigned char v) {
  int lastMask=ymMask;
  if (a==0x19 && v>=0x80) a=0x1a; // AMD/PSD use same reg addr. store PMD as 0x1a
  if (a==0x08 && (v&0xf8)) ymMask|=(1<<(v&0x07)); // mark chan as in-use if keyDN
  if (a!=0x08) ymState[ym_NEW][a]=v; // cache the newly-written value
  bool writeit=false; // used to suppress spurious writes to unused channels
  if (a<0x20) {
    if (a==0x08) {
      // write keyUPDN messages if channel is active.
      writeit=(ymMask&(1<<(v&0x07)))>0;
    } else {
      // do not suppress global registers
      writeit=true;
    }
  } else {
    writeit=(ymMask&(1<<(a&0x07)))>0; // a&0x07 = chan ID for regs >=0x20
  }
  if (lastMask!=ymMask) {
    // if the ymMask just changed, then the channel has become active.
    // This can only happen on a KeyDN event, so voice=v&0x07
    // insert a keyUP just to be safe.
    ymwrites.push_back(DivRegWrite(0x08,v&0x07));
    numWrites++;
    // flush the ym_NEW cached states for this channel into the ZSM....
    for (int i=0x20+(v&0x07); i<=0xff; i+=8) {
      if (ymState[ym_NEW][i]!=ymState[ym_PREV][i]) {
        ymwrites.push_back(DivRegWrite(i,ymState[ym_NEW][i]));
        numWrites++;
        // ...and update the shadow
        ymState[ym_PREV][i]=ymState[ym_NEW][i];
      }
    }
  }
  // Handle the current write if channel is active
  if (writeit && ((ymState[ym_NEW][a]!=ymState[ym_PREV][a]) || a==0x08)) {
    // update YM shadow if not the KeyUPDN register.
    if (a!=8) ymState[ym_PREV][a]=ymState[ym_NEW][a];
    // if reg=PMD, then change back to real register 0x19
    if (a==0x1a) a=0x19;
    ymwrites.push_back(DivRegWrite(a,v));
    numWrites++;
  }
}

void DivZSM::writeSync(unsigned char a, unsigned char v) {
  return syncCache.push_back(DivRegWrite(a,v));
}

void DivZSM::writePSG(unsigned char a, unsigned char v) {
  if (a>=69) {
    logD("ZSM: ignoring VERA PSG write a=%02x v=%02x",a,v);
    return;
  } else if (a==68) {
    // Sync event
    numWrites++;
    return writeSync(0x00,v);
  } else if (a>=64) {
    return writePCM(a-64,v);
  }
  if (psgState[psg_PREV][a]==v) {
    if (psgState[psg_NEW][a]!=v) {
      // NEW value is being reset to the same as PREV value
      // so it is no longer a new write.
      numWrites--;
    }
  } else {
    if (psgState[psg_PREV][a]==psgState[psg_NEW][a]) {
      // if this write changes the NEW cached value to something other
      // than the PREV value, then this is a new write.
      numWrites++;
    }
  }
  psgState[psg_NEW][a]=v;
  // mark channel as used in the psgMask if volume is set>0.
  if ((a&3)==2 && (v&0x3f)) psgMask|=(1<<(a>>2));
}

void DivZSM::writePCM(unsigned char a, unsigned char v) {
  if (a==0) { // PCM Ctrl
    // cache the depth and channels but don't write it to the
    // register queue
    pcmCtrlDCCache=v&0x30;
    // save only the reset bit and volume (if it isn't a dupe)
    if (pcmCtrlRVCache!=(v&0x8f)) {
      pcmMeta.push_back(DivRegWrite(a,(v&0x8f)));
      pcmCtrlRVCache=v&0x8f;
      numWrites++;
    }
  } else if (a==1) { // PCM Rate
    if (pcmRateCache!=v) {
      pcmMeta.push_back(DivRegWrite(a,v));
      pcmRateCache=v;
      numWrites++;
    }
  } else if (a==2) { // PCM data
    pcmCache.push_back(v);
    numWrites++;
  } else if (a==3) { // PCM loop point
    pcmLoopPointCache=(pcmLoopPointCache>>8)|(v<<16);
    pcmIsLooped=true;
  }
}

void DivZSM::tick(int numticks) {
  flushWrites();
  ticks+=numticks;
}

void DivZSM::setLoopPoint() {
  tick(0); // flush any ticks+writes
  flushTicks(); // flush ticks incase no writes were pending
  logI("ZSM: loop at file offset %d bytes",w->tell());
  loopOffset=w->tell();
  // update the ZSM header's loop offset value
  w->seek(0x03,SEEK_SET);
  w->writeS((short)(loopOffset&0xffff));
  w->writeC((unsigned char)((loopOffset>>16)&0xff));
  w->seek(loopOffset,SEEK_SET);
  // reset the PSG shadow and write cache
  memset(&psgState,-1,sizeof(psgState));
  // reset the PCM caches that would inhibit dupes
  pcmRateCache=-1;
  pcmCtrlRVCache=-1;
  // reset the YM shadow....
  memset(&ymState[ym_PREV],-1,sizeof(ymState[ym_PREV]));
  // ... and cache (except for unused channels)
  memset(&ymState[ym_NEW],-1,0x20);
  for (int chan=0; chan<8; chan++) {
    // do not clear state for as-yet-unused channels
    if (!(ymMask&(1<<chan))) continue;
    // clear the state for channels in use so they match the unknown state
    // of the YM shadow.
    for (int i=0x20+chan; i<=0xff; i+=8) {
      ymState[ym_NEW][i]=-1;
    }
  }
}

void DivZSM::setOptimize(bool o) {
  optimize=o;
}

SafeWriter* DivZSM::finish() {
  tick(0); // flush any pending writes / ticks
  flushTicks(); // flush ticks in case there were no writes pending
  w->writeC(ZSM_EOF);
  if (pcmInsts.size()>256) {
    logE("ZSM: more than the maximum number of PCM instruments exist. Skipping PCM export entirely.");
    pcmData.clear();
    pcmInsts.clear();
  } else if (pcmData.size()) { // if exists, write PCM instruments and blob to the end of file
    unsigned int pcmOff=w->tell();
    w->writeC('P');
    w->writeC('C');
    w->writeC('M');
    w->writeC((unsigned char)pcmInsts.size()-1);
    int i=0;
    for (S_pcmInst& inst: pcmInsts) {
      // write out the instruments
      // PCM playback location follows:
      //   <instrument number>
      //   <geometry (depth and channel)>
      //   <l m h> of PCM data offset
      //   <l m h> of length
      w->writeC((unsigned char)i&0xff);
      w->writeC((unsigned char)inst.geometry&0x30);
      w->writeC((unsigned char)inst.offset&0xff);
      w->writeC((unsigned char)(inst.offset>>8)&0xff);
      w->writeC((unsigned char)(inst.offset>>16)&0xff);
      w->writeC((unsigned char)inst.length&0xff);
      w->writeC((unsigned char)(inst.length>>8)&0xff);
      w->writeC((unsigned char)(inst.length>>16)&0xff);
      // Feature mask: Lxxxxxxx
      //   L = Loop enabled
      w->writeC((unsigned char)inst.isLooped<<7);
      // Sample loop point <l m h>
      w->writeC((unsigned char)inst.loopPoint&0xff);
      w->writeC((unsigned char)(inst.loopPoint>>8)&0xff);
      w->writeC((unsigned char)(inst.loopPoint>>16)&0xff);
      // Reserved for future use
      w->writeS(0);
      w->writeS(0);
      i++;
    }
    for (unsigned char& c: pcmData) {
      w->writeC(c);
    }
    pcmData.clear();
    // update PCM offset in file
    w->seek(0x06,SEEK_SET);
    w->writeC((unsigned char)pcmOff&0xff);
    w->writeC((unsigned char)(pcmOff>>8)&0xff);
    w->writeC((unsigned char)(pcmOff>>16)&0xff);
  }
  // update channel use masks.
  w->seek(0x09,SEEK_SET);
  w->writeC((unsigned char)(ymMask&0xff));
  w->writeS((short)(psgMask&0xffff));
  return w;
}

void DivZSM::flushWrites() {
  logD("ZSM: flushWrites.... numwrites=%d ticks=%d ymwrites=%d pcmMeta=%d pcmCache=%d pcmData=%d syncCache=%d",numWrites,ticks,ymwrites.size(),pcmMeta.size(),pcmCache.size(),pcmData.size(),syncCache.size());
  if (numWrites==0) return;
  bool hasFlushed=false;
  for (unsigned char i=0; i<64; i++) {
    if (psgState[psg_NEW][i]==psgState[psg_PREV][i]) continue;
    // if optimize=true, suppress writes to PSG voices that are not audible (volume=0 or R+L=0)
    // ZSMKit has a feature that can benefit from having silent channels
    // updated, so this is something that can be toggled off or on for export
    if (optimize && (i&3)!=2 && (psgState[psg_NEW][(i&0x3c)+2]&0x3f)==0) continue; // vol
    if (optimize && (i&3)!=2 && (psgState[psg_NEW][(i&0x3c)+2]&0xc0)==0) continue; // R+L
    psgState[psg_PREV][i]=psgState[psg_NEW][i];
    if (!hasFlushed) {
      flushTicks();
      hasFlushed=true;
    }
    w->writeC(i);
    w->writeC(psgState[psg_NEW][i]);
  }
  int n=0; // n=completed YM writes. used to determine when to write the CMD byte...
  for (DivRegWrite& write: ymwrites) {
    if (!hasFlushed) {
      flushTicks();
      hasFlushed=true;
    }
    if (n%ZSM_YM_MAX_WRITES==0) {
      if (ymwrites.size()-n>ZSM_YM_MAX_WRITES) {
        w->writeC((unsigned char)(ZSM_YM_CMD+ZSM_YM_MAX_WRITES));
        logD("ZSM: YM-write: %d (%02x) [max]",ZSM_YM_MAX_WRITES,ZSM_YM_MAX_WRITES+ZSM_YM_CMD);
      } else {
        w->writeC((unsigned char)(ZSM_YM_CMD+ymwrites.size()-n));
        logD("ZSM: YM-write: %d (%02x)",ymwrites.size()-n,ZSM_YM_CMD+ymwrites.size()-n);
      }
    }
    n++;
    w->writeC(write.addr);
    w->writeC(write.val);
  }
  ymwrites.clear();
  unsigned int pcmInst=0;
  unsigned int pcmOff=0;
  unsigned int pcmLen=0;
  int extCmd0Len=pcmMeta.size()*2;
  if (pcmCache.size()) {
    // collapse stereo data to mono if both channels are fully identical
    // which cuts PCM data size in half for center-panned PCM events
    if (pcmCtrlDCCache&0x10) { // stereo bit is on
      unsigned int e;
      if (pcmCtrlDCCache&0x20) { // 16-bit
        // for 16-bit PCM data, the size must be a multiple of 4
        if (pcmCache.size()%4==0) {
          // check for identical L+R channels
          for (e=0; e<pcmCache.size(); e+=4) {
            if (pcmCache[e]!=pcmCache[e+2] || pcmCache[e+1]!=pcmCache[e+3]) break;
          }
          if (e==pcmCache.size()) { // did not find a mismatch
            // collapse the data to mono 16-bit
            for (e=0; e<pcmCache.size()>>1; e+=2) {
              pcmCache[e]=pcmCache[e<<1];
              pcmCache[e+1]=pcmCache[(e<<1)+1];
            }
            pcmCache.resize(pcmCache.size()>>1);
            pcmCtrlDCCache&=(unsigned char)~0x10; // clear stereo bit
            pcmLoopPointCache>>=1; // halve the loop point
          }
        }
      } else { // 8-bit
        // for 8-bit PCM data, the size must be a multiple of 2
        if (pcmCache.size()%2==0) {
          // check for identical L+R channels
          for (e=0; e<pcmCache.size(); e+=2) {
            if (pcmCache[e]!=pcmCache[e+1]) break;
          }
          if (e==pcmCache.size()) { // did not find a mismatch
            // collapse the data to mono 8-bit
            for (e=0; e<pcmCache.size()>>1; e++) {
              pcmCache[e]=pcmCache[e<<1];
            }
            pcmCache.resize(pcmCache.size()>>1);
            pcmCtrlDCCache&=(unsigned char)~0x10; // clear stereo bit
            pcmLoopPointCache>>=1; // halve the loop point
          }
        }
      }
    }
    // check to see if the most recent received blob matches any of the previous data
    // and reuse it if there is a match, otherwise append the cache to the rest of
    // the PCM data
    std::vector<unsigned char>::iterator it;
    it=std::search(pcmData.begin(),pcmData.end(),pcmCache.begin(),pcmCache.end());
    pcmOff=std::distance(pcmData.begin(),it);
    pcmLen=pcmCache.size();
    logD("ZSM: pcmOff: %d pcmLen: %d",pcmOff,pcmLen);
    if (it==pcmData.end()) {
      pcmData.insert(pcmData.end(),pcmCache.begin(),pcmCache.end());
    }
    pcmCache.clear();
    extCmd0Len+=2;
    // search for a matching PCM instrument definition
    for (S_pcmInst& inst: pcmInsts) {
      if (inst.offset==pcmOff && inst.length==pcmLen && inst.geometry==pcmCtrlDCCache && inst.isLooped==pcmIsLooped && inst.loopPoint==pcmLoopPointCache)
        break;
      pcmInst++;
    }
    if (pcmInst==pcmInsts.size()) {
      S_pcmInst inst;
      inst.geometry=pcmCtrlDCCache;
      inst.offset=pcmOff;
      inst.length=pcmLen;
      inst.loopPoint=pcmLoopPointCache;
      inst.isLooped=pcmIsLooped;
      pcmInsts.push_back(inst);
    }
    pcmIsLooped=false;
    pcmLoopPointCache=0;
  }
  if (extCmd0Len>63) { // this would be bad, but will almost certainly never happen
    logE("ZSM: extCmd 0 exceeded maximum length of 63: %d",extCmd0Len);
    extCmd0Len=0;
    pcmMeta.clear();
  }
  if (extCmd0Len) { // we have some PCM events to write
    if (!hasFlushed) {
      flushTicks();
      hasFlushed=true;
    }
    w->writeC(ZSM_EXT);
    w->writeC(ZSM_EXT_PCM|(unsigned char)extCmd0Len);
    for (DivRegWrite& write: pcmMeta) {
      w->writeC(write.addr);
      w->writeC(write.val);
    }
    pcmMeta.clear();
    if (pcmLen) {
      w->writeC(0x02); // 0x02 = Instrument trigger
      w->writeC((unsigned char)pcmInst&0xff);
    }
  }
  n=0;
  for (DivRegWrite& write: syncCache) {
    if (!hasFlushed) {
      flushTicks();
      hasFlushed=true;
    }
    if (n%ZSM_SYNC_MAX_WRITES==0) {
      w->writeC(ZSM_EXT);
      if (syncCache.size()-n>ZSM_SYNC_MAX_WRITES) {
        w->writeC((unsigned char)(ZSM_EXT_SYNC|(ZSM_SYNC_MAX_WRITES<<1)));
      } else {
        w->writeC((unsigned char)(ZSM_EXT_SYNC|((syncCache.size()-n)<<1)));
      }
    }
    n++;
    w->writeC(write.addr);
    w->writeC(write.val);
  }
  syncCache.clear();
  numWrites=0;
}

void DivZSM::flushTicks() {
  while (ticks>ZSM_DELAY_MAX) {
    logD("ZSM: write delay %d (max)",ZSM_DELAY_MAX);
    w->writeC((unsigned char)(ZSM_DELAY_CMD+ZSM_DELAY_MAX));
    ticks-=ZSM_DELAY_MAX;
  }
  if (ticks>0) {
    logD("ZSM: write delay %d",ticks);
    w->writeC(ZSM_DELAY_CMD+ticks);
  }
  ticks=0;
}

/// ZSM export

void DivExportZSM::run() {
  // settings
  unsigned int zsmrate=conf.getInt("zsmrate",60);
  bool loop=conf.getBool("loop",true);
  bool optimize=conf.getBool("optimize",true);

  // system IDs
  int VERA=-1;
  int YM=-1;
  int IGNORED=0;

  // find indexes for YM and VERA. Ignore other systems.
  for (int i=0; i<e->song.systemLen; i++) {
    switch (e->song.system[i]) {
      case DIV_SYSTEM_VERA:
        if (VERA>=0) {
          IGNORED++;
          break;
        }
        VERA=i;
        logAppendf("VERA detected as chip id %d",i);
        break;
      case DIV_SYSTEM_YM2151:
        if (YM>=0) {
          IGNORED++;
          break;
        }
        YM=i;
        logAppendf("YM detected as chip id %d",i);
        break;
      default:
        IGNORED++;
        logAppendf("Ignoring chip %d systemID %d",i,(int)e->song.system[i]);
        break;
    }
  }
  if (VERA<0 && YM<0) {
    logAppend("ERROR: No supported systems for ZSM");
    failed=true;
    running=false;
    return;
  }
  if (IGNORED>0) {
    logAppendf("ZSM export ignoring %d unsupported system%c",IGNORED,IGNORED>1?'s':' ');
  }

  DivZSM zsm;

  e->stop();
  e->repeatPattern=false;
  e->setOrder(0);
  e->synchronizedSoft([&]() {
    double origRate=e->got.rate;
    e->got.rate=zsmrate&0xffff;

    // determine loop point
    int loopOrder=0;
    int loopRow=0;
    int loopEnd=0;
    e->walkSong(loopOrder,loopRow,loopEnd);
    logAppendf("loop point: %d %d",loopOrder,loopRow);

    zsm.init(zsmrate);

    // reset the playback state
    e->curOrder=0;
    e->freelance=false;
    e->playing=false;
    e->extValuePresent=false;
    e->remainingLoops=-1;

    // Prepare to write song data
    e->playSub(false);
    //size_t tickCount=0;
    bool done=false;
    bool loopNow=false;
    int loopPos=-1;
    if (VERA>=0) e->disCont[VERA].dispatch->toggleRegisterDump(true);
    if (YM>=0) {
      e->disCont[YM].dispatch->toggleRegisterDump(true);
      // emit LFO initialization commands
      zsm.writeYM(0x18,0);    // freq=0
      zsm.writeYM(0x19,0x7F); // AMD =7F
      zsm.writeYM(0x19,0xFF); // PMD =7F
      // TODO: incorporate the Furnace meta-command for init data and filter
      //       out writes to otherwise-unused channels.
    }
    // Indicate the song's tuning as a sync meta-event
    // specified in terms of how many 1/256th semitones
    // the song is offset from standard A-440 tuning.
    // This is mainly to benefit visualizations in players
    // for non-standard tunings so that they can avoid
    // displaying the entire song held in pitch bend.
    // Tunings offsets that exceed a half semitone
    // will simply be represented in a different key
    // by nature of overflowing the signed char value
    signed char tuningoffset=(signed char)(round(3072*(log(e->song.tuning/440.0)/log(2))))&0xff;
    zsm.writeSync(0x01,tuningoffset);
    // Set optimize flag, which mainly buffers PSG writes
    // whenever the channel is silent
    zsm.setOptimize(optimize);

    while (!done) {
      if (loopPos==-1) {
        if (loopOrder==e->curOrder && loopRow==e->curRow && loop)
          loopNow=true;
        if (loopNow) {
          // If Virtual Tempo is in use, our exact loop point
          // might be skipped due to quantization error.
          // If this happens, the tick immediately following is our loop point.
          if (e->ticks==1 || !(loopOrder==e->curOrder && loopRow==e->curRow)) {
            loopPos=zsm.getoffset();
            zsm.setLoopPoint();
            loopNow=false;
          }
        }
      }
      if (e->nextTick() || !e->playing) {
        done=true;
        if (!loop) {
          for (int i=0; i<e->song.systemLen; i++) {
            e->disCont[i].dispatch->getRegisterWrites().clear();
          }
          break;
        }
        if (!e->playing) {
          loopPos=-1;
        }
      }
      // get register dumps
      for (int j=0; j<2; j++) {
        int i=0;
        // dump YM writes first
        if (j==0) {
          if (YM<0) {
            continue;
          } else {
            i=YM;
          }
        }
        // dump VERA writes second
        if (j==1) {
          if (VERA<0) {
            continue;
          } else {
            i=VERA;
          }
        }
        std::vector<DivRegWrite>& writes=e->disCont[i].dispatch->getRegisterWrites();
        if (writes.size()>0)
          logD("zsmOps: Writing %d messages to chip %d",writes.size(),i);
        for (DivRegWrite& write: writes) {
          if (i==YM) {
            if (done && write.addr==0x08 && (write.val&0x78)>0) continue; // don't process keydown on lookahead
            zsm.writeYM(write.addr&0xff,write.val);
          }
          if (i==VERA) {
            if (done && write.addr>=64) continue; // don't process any PCM or sync events on the loop lookahead
            zsm.writePSG(write.addr&0xff,write.val);
          }
        }
        writes.clear();
      }

      // write wait
      int totalWait=e->cycles;
      if (totalWait>0 && !done) {
        zsm.tick(totalWait);
        //tickCount+=totalWait;
      }
    }
    // end of song

    // done - close out.
    e->got.rate=origRate;
    if (VERA>=0) e->disCont[VERA].dispatch->toggleRegisterDump(false);
    if (YM>=0) e->disCont[YM].dispatch->toggleRegisterDump(false);

    e->remainingLoops=-1;
    e->playing=false;
    e->freelance=false;
    e->extValuePresent=false;
  });

  progress[0].amount=1.0f;

  logAppend("finished!");

  output.push_back(DivROMExportOutput("out.zsm",zsm.finish()));
  running=false;
}

/// DivExpottZSM - FRONTEND

bool DivExportZSM::go(DivEngine* eng) {
  progress[0].name="Generate";
  progress[0].amount=0.0f;

  e=eng;
  running=true;
  failed=false;
  mustAbort=false;
  exportThread=new std::thread(&DivExportZSM::run,this);
  return true;
}

void DivExportZSM::wait() {
  if (exportThread!=NULL) {
    exportThread->join();
    delete exportThread;
  }
}

void DivExportZSM::abort() {
  mustAbort=true;
  wait();
}

bool DivExportZSM::isRunning() {
  return running;
}

bool DivExportZSM::hasFailed() {
  return failed;
}

DivROMExportProgress DivExportZSM::getProgress(int index) {
  if (index<0 || index>1) return progress[1];
  return progress[index];
}
