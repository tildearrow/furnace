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

#include "zsm.h"
#include "../ta-log.h"
#include "../utfutils.h"
#include "song.h"

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
