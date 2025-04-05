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

#include "engine.h"
#include "../ta-log.h"
#include <unordered_map>

//#define DISABLE_BLOCK_SEARCH

int DivCS::getInsLength(unsigned char ins, unsigned char ext) {
  switch (ins) {
    case 0xb8: // ins
    case 0xc0: // pre porta
    case 0xc3: // vib range
    case 0xc4: // vib shape
    case 0xc5: // pitch
    case 0xc7: // volume
    case 0xca: // legato
    case 0xfd: // waitc
      return 2;
    case 0xbe: // pan
    case 0xc2: // vibrato
    case 0xc6: // arpeggio
    case 0xc8: // vol slide
    case 0xc9: // porta
      return 3;
    // speed dial commands
    case 0xd0: case 0xd1: case 0xd2: case 0xd3:
    case 0xd4: case 0xd5: case 0xd6: case 0xd7:
    case 0xd8: case 0xd9: case 0xda: case 0xdb:
    case 0xdc: case 0xdd: case 0xde: case 0xdf:
      return 0;
    case 0xf0: // opt
      return 4;
    case 0xf2: // opt command
    case 0xf7: { // cmd
      // determine length from secondary
      if (ext==0) return 0;
      return 0;
    }
    case 0xf8: // callb16
    case 0xfc: // waits
      return 3;
    case 0xf4: // callsym
    case 0xf5: // call
    case 0xf6: // callb32
    case 0xfa: // jmp
    case 0xfb: // rate
      return 5;
  }
  return 1;
}

void writePackedCommandValues(SafeWriter* w, const DivCommand& c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON:
      if (c.value==DIV_NOTE_NULL) {
        w->writeC(0xb4);
      } else {
        w->writeC(CLAMP(c.value+60,0,0xb3));
      }
      break;
    case DIV_CMD_NOTE_OFF:
    case DIV_CMD_NOTE_OFF_ENV:
    case DIV_CMD_ENV_RELEASE:
    case DIV_CMD_INSTRUMENT:
    case DIV_CMD_PANNING:
    case DIV_CMD_PRE_PORTA:
    case DIV_CMD_HINT_VIBRATO:
    case DIV_CMD_HINT_VIBRATO_RANGE:
    case DIV_CMD_HINT_VIBRATO_SHAPE:
    case DIV_CMD_HINT_PITCH:
    case DIV_CMD_HINT_ARPEGGIO:
    case DIV_CMD_HINT_VOLUME:
    case DIV_CMD_HINT_PORTA:
    case DIV_CMD_HINT_VOL_SLIDE:
    case DIV_CMD_HINT_VOL_SLIDE_TARGET:
    case DIV_CMD_HINT_LEGATO:
      w->writeC((unsigned char)c.cmd+0xb4);
      break;
    default:
      return; // quit for now... we'll implement this later
      w->writeC(0xf2); // unoptimized extended command
      w->writeC(c.cmd);
      break;
  }
  switch (c.cmd) {
    case DIV_CMD_HINT_LEGATO:
      if (c.value==DIV_NOTE_NULL) {
        w->writeC(0xff);
      } else {
        w->writeC(c.value+60);
      }
      break;
    case DIV_CMD_NOTE_ON:
    case DIV_CMD_NOTE_OFF:
    case DIV_CMD_NOTE_OFF_ENV:
    case DIV_CMD_ENV_RELEASE:
      break;
    case DIV_CMD_INSTRUMENT:
    case DIV_CMD_HINT_VIBRATO_RANGE:
    case DIV_CMD_HINT_VIBRATO_SHAPE:
    case DIV_CMD_HINT_PITCH:
    case DIV_CMD_HINT_VOLUME:
      w->writeC(c.value);
      break;
    case DIV_CMD_PANNING:
    case DIV_CMD_HINT_VIBRATO:
    case DIV_CMD_HINT_ARPEGGIO:
    case DIV_CMD_HINT_PORTA:
      w->writeC(c.value);
      w->writeC(c.value2);
      break;
    case DIV_CMD_PRE_PORTA:
      w->writeC((c.value?0x80:0)|(c.value2?0x40:0));
      break;
    case DIV_CMD_HINT_VOL_SLIDE:
      w->writeS(c.value);
      break;
    case DIV_CMD_HINT_VOL_SLIDE_TARGET:
      w->writeS(c.value);
      w->writeS(c.value2);
      break;
    case DIV_CMD_SAMPLE_MODE:
    case DIV_CMD_SAMPLE_FREQ:
    case DIV_CMD_SAMPLE_BANK:
    case DIV_CMD_SAMPLE_POS:
    case DIV_CMD_SAMPLE_DIR:
    case DIV_CMD_FM_HARD_RESET:
    case DIV_CMD_FM_LFO:
    case DIV_CMD_FM_LFO_WAVE:
    case DIV_CMD_FM_FB:
    case DIV_CMD_FM_EXTCH:
    case DIV_CMD_FM_AM_DEPTH:
    case DIV_CMD_FM_PM_DEPTH:
    case DIV_CMD_STD_NOISE_FREQ:
    case DIV_CMD_STD_NOISE_MODE:
    case DIV_CMD_WAVE:
    case DIV_CMD_GB_SWEEP_TIME:
    case DIV_CMD_GB_SWEEP_DIR:
    case DIV_CMD_PCE_LFO_MODE:
    case DIV_CMD_PCE_LFO_SPEED:
    case DIV_CMD_NES_DMC:
    case DIV_CMD_C64_CUTOFF:
    case DIV_CMD_C64_RESONANCE:
    case DIV_CMD_C64_FILTER_MODE:
    case DIV_CMD_C64_RESET_TIME:
    case DIV_CMD_C64_RESET_MASK:
    case DIV_CMD_C64_FILTER_RESET:
    case DIV_CMD_C64_DUTY_RESET:
    case DIV_CMD_C64_EXTENDED:
    case DIV_CMD_AY_ENVELOPE_SET:
    case DIV_CMD_AY_ENVELOPE_LOW:
    case DIV_CMD_AY_ENVELOPE_HIGH:
    case DIV_CMD_AY_ENVELOPE_SLIDE:
    case DIV_CMD_AY_NOISE_MASK_AND:
    case DIV_CMD_AY_NOISE_MASK_OR:
    case DIV_CMD_AY_AUTO_ENVELOPE:
    case DIV_CMD_FDS_MOD_DEPTH:
    case DIV_CMD_FDS_MOD_HIGH:
    case DIV_CMD_FDS_MOD_LOW:
    case DIV_CMD_FDS_MOD_POS:
    case DIV_CMD_FDS_MOD_WAVE:
    case DIV_CMD_SAA_ENVELOPE:
    case DIV_CMD_AMIGA_FILTER:
    case DIV_CMD_AMIGA_AM:
    case DIV_CMD_AMIGA_PM:
    case DIV_CMD_MACRO_OFF:
    case DIV_CMD_MACRO_ON:
    case DIV_CMD_MACRO_RESTART:
    case DIV_CMD_HINT_ARP_TIME:
      w->writeC(1); // length
      w->writeC(c.value);
      break;
    case DIV_CMD_FM_TL:
    case DIV_CMD_FM_AM:
    case DIV_CMD_FM_AR:
    case DIV_CMD_FM_DR:
    case DIV_CMD_FM_SL:
    case DIV_CMD_FM_D2R:
    case DIV_CMD_FM_RR:
    case DIV_CMD_FM_DT:
    case DIV_CMD_FM_DT2:
    case DIV_CMD_FM_RS:
    case DIV_CMD_FM_KSR:
    case DIV_CMD_FM_VIB:
    case DIV_CMD_FM_SUS:
    case DIV_CMD_FM_WS:
    case DIV_CMD_FM_SSG:
    case DIV_CMD_FM_REV:
    case DIV_CMD_FM_EG_SHIFT:
    case DIV_CMD_FM_MULT:
    case DIV_CMD_FM_FINE:
    case DIV_CMD_AY_IO_WRITE:
    case DIV_CMD_AY_AUTO_PWM:
    case DIV_CMD_SURROUND_PANNING:
      w->writeC(2); // length
      w->writeC(c.value);
      w->writeC(c.value2);
      break;
    case DIV_CMD_C64_FINE_DUTY:
    case DIV_CMD_C64_FINE_CUTOFF:
    case DIV_CMD_LYNX_LFSR_LOAD:
      w->writeC(2); // length
      w->writeS(c.value);
      break;
    case DIV_CMD_FM_FIXFREQ:
      w->writeC(2); // length
      w->writeS((c.value<<12)|(c.value2&0x7ff));
      break;
    case DIV_CMD_NES_SWEEP:
      w->writeC(1); // length
      w->writeC((c.value?8:0)|(c.value2&0x77));
      break;
    default:
      logW("unimplemented command %s!",cmdName[c.cmd]);
      w->writeC(0); // length
      break;
  }
}

using namespace DivCS;

void reloc(unsigned char* buf, size_t len, unsigned int sourceAddr, unsigned int destAddr) {
  unsigned int delta=destAddr-sourceAddr;
  for (size_t i=0; i<len;) {
    int insLen=getInsLength(buf[i]);
    if (insLen<1) {
      logE("INS %x NOT IMPLEMENTED...",buf[i]);
      break;
    }
    switch (buf[i]) {
      case 0xf5: // call
      case 0xfa: { // jmp
        unsigned int addr=buf[i+1]|(buf[i+2]<<8)|(buf[i+3]<<8)|(buf[i+4]<<24);
        addr+=delta;
        buf[i+1]=addr&0xff;
        buf[i+2]=(addr>>8)&0xff;
        buf[i+3]=(addr>>16)&0xff;
        buf[i+4]=(addr>>24)&0xff;
        break;
      }
    }
    i+=insLen;
  }
}

SafeWriter* stripNops(SafeWriter* s) {
  std::unordered_map<unsigned int,unsigned int> addrTable;
  SafeWriter* oldStream=s;
  unsigned char* buf=oldStream->getFinalBuf();
  s=new SafeWriter;
  s->init();

  // prepare address map
  size_t addr=0;
  for (size_t i=0; i<oldStream->size();) {
    int insLen=getInsLength(buf[i]);
    if (insLen<1) {
      logE("INS %x NOT IMPLEMENTED...",buf[i]);
      break;
    }
    addrTable[i]=addr;
    if (buf[i]!=0xf1) addr+=insLen;
    i+=insLen;
  }

  // translate addresses
  for (size_t i=0; i<oldStream->size();) {
    int insLen=getInsLength(buf[i]);
    if (insLen<1) {
      logE("INS %x NOT IMPLEMENTED...",buf[i]);
      break;
    }
    switch (buf[i]) {
      case 0xf5: // call
      case 0xfa: { // jmp
        unsigned int addr=buf[i+1]|(buf[i+2]<<8)|(buf[i+3]<<8)|(buf[i+4]<<24);
        try {
          addr=addrTable[addr];
          buf[i+1]=addr&0xff;
          buf[i+2]=(addr>>8)&0xff;
          buf[i+3]=(addr>>16)&0xff;
          buf[i+4]=(addr>>24)&0xff;
        } catch (std::out_of_range& e) {
          logW("address %x is not mappable!",addr);
        }
        break;
      }
    }
    if (buf[i]!=0xf1) {
      s->write(&buf[i],insLen);
    }
    i+=insLen;
  }
  

  oldStream->finish();
  delete oldStream;
  return s;
}

SafeWriter* findSubBlocks(SafeWriter* stream, std::vector<SafeWriter*>& subBlocks) {
  unsigned char* buf=stream->getFinalBuf();

  for (size_t groupSize=stream->size()>>1; groupSize>=8; groupSize--) {
  //for (size_t groupSize=7; groupSize<=stream->size()>>1; groupSize++) {
    bool foundSomething=false;
    //logD("...try size %d",groupSize);
    for (size_t searchPos=0; (searchPos+groupSize)<stream->size();) {
      const unsigned char* group=&buf[searchPos];
      size_t groupLen=0;
      size_t groupInsCount=0;
      size_t subBlockID=subBlocks.size();
      int insLen=getInsLength(buf[searchPos]);
      bool haveSub=false;
      bool onlyCalls=true;

      if (insLen<1) {
        logE("INS %x NOT IMPLEMENTED...",buf[searchPos]);
        break;
      }

      // register this block
      for (size_t i=0; i<groupSize && i<stream->size();) {
        if (buf[searchPos+i]!=0xf4) onlyCalls=false;
        int insLenI=getInsLength(buf[searchPos+i]);
        if (insLenI<1) {
          logE("INS %x NOT IMPLEMENTED...",buf[searchPos+i]);
          break;
        }
        i+=insLenI;
        if (groupLen+insLenI>groupSize) break;
        groupLen+=insLenI;
        groupInsCount++;
      }

      // don't do anything if we don't have a block large enough
      if (groupLen<5) {
        searchPos+=insLen;
        continue;
      }

      // don't do anything if this is just one or two commands
      if (groupInsCount<3) {
        searchPos+=insLen;
        continue;
      }

      // don't do anything if this block only consists of calls
      if (onlyCalls) {
        logW("nothing but calls.");
        searchPos+=insLen;
        continue;
      }

      // find identical blocks
      for (size_t i=searchPos+groupLen; i+groupLen<stream->size();) {
        int insLenI=getInsLength(buf[i]);
        if (insLenI<1) {
          logE("INS %x NOT IMPLEMENTED...",buf[i]);
          break;
        }

        // compare next block to group
        if (memcmp(&buf[i],group,groupLen)==0) {
          // we have a sub-block
          if (!haveSub) {
            // isolate this sub-block
            SafeWriter* newBlock=new SafeWriter;
            newBlock->init();
            newBlock->write(group,groupLen);
            newBlock->writeC(0xf9); // ret
            subBlocks.push_back(newBlock);
            haveSub=true;
          logD("- SUB %x (size %d):",searchPos,groupLen);
          }
          logD("  - %x",i);
          // insert call
          buf[i]=0xf4;
          buf[i+1]=subBlockID&0xff;
          buf[i+2]=(subBlockID>>8)&0xff;
          buf[i+3]=(subBlockID>>16)&0xff;
          buf[i+4]=(subBlockID>>24)&0xff;

          // replace the rest with nop
          for (size_t j=i+5; j<i+groupLen; j++) {
            buf[j]=0xf1;
          }

          // continue search from end of block
          i+=groupLen;
        } else {
          // next
          i+=insLenI;
        }
      }

      if (haveSub) {
        // insert call on the original block
        buf[searchPos]=0xf4;
        buf[searchPos+1]=subBlockID&0xff;
        buf[searchPos+2]=(subBlockID>>8)&0xff;
        buf[searchPos+3]=(subBlockID>>16)&0xff;
        buf[searchPos+4]=(subBlockID>>24)&0xff;

        // replace the rest with nop
        for (size_t j=searchPos+5; j<searchPos+groupLen; j++) {
          buf[j]=0xf1;
        }

        // skip this block (it's isolated now)
        searchPos+=groupLen;
        foundSomething=true;
      } else {
        // try again somewhere else
        searchPos+=insLen;
      }
    }
    if (foundSomething) {
      stream=stripNops(stream);
      buf=stream->getFinalBuf();
    }
  }
  return stream;
}

SafeWriter* DivEngine::saveCommand() {
  stop();
  repeatPattern=false;
  shallStop=false;
  setOrder(0);
  BUSY_BEGIN_SOFT;
  // determine loop point
  int loopOrder=0;
  int loopRow=0;
  int loopEnd=0;
  walkSong(loopOrder,loopRow,loopEnd);
  logI("loop point: %d %d",loopOrder,loopRow);

  int cmdPopularity[256];
  int delayPopularity[256];

  int sortedCmdPopularity[16];
  int sortedDelayPopularity[16];
  unsigned char sortedCmd[16];
  unsigned char sortedDelay[16];
  
  SafeWriter* chanStream[DIV_MAX_CHANS];
  unsigned int chanStreamOff[DIV_MAX_CHANS];
  std::vector<size_t> tickPos[DIV_MAX_CHANS];
  int loopTick=-1;

  memset(cmdPopularity,0,256*sizeof(int));
  memset(delayPopularity,0,256*sizeof(int));
  memset(chanStream,0,DIV_MAX_CHANS*sizeof(void*));
  memset(chanStreamOff,0,DIV_MAX_CHANS*sizeof(unsigned int));
  memset(sortedCmdPopularity,0,16*sizeof(int));
  memset(sortedDelayPopularity,0,16*sizeof(int));
  memset(sortedCmd,0,16);
  memset(sortedDelay,0,16);

  SafeWriter* w=new SafeWriter;
  w->init();

  // write header
  w->write("FCS",4);
  w->writeI(chans);
  // offsets
  for (int i=0; i<chans; i++) {
    chanStream[i]=new SafeWriter;
    chanStream[i]->init();
    w->writeI(0);
  }
  // preset delays and speed dial
  for (int i=0; i<32; i++) {
    w->writeC(0);
  }

  // play the song ourselves
  bool done=false;
  playSub(false);
  
  int tick=0;
  bool oldCmdStreamEnabled=cmdStreamEnabled;
  cmdStreamEnabled=true;
  double curDivider=divider;

  // PASS 0: play the song and log channel command streams
  while (!done) {
    for (int i=0; i<chans; i++) {
      tickPos[i].push_back(chanStream[i]->tell());
    }
    if (loopTick==-1) {
      if (loopOrder==curOrder && loopRow==curRow) {
        if ((ticks-((tempoAccum+virtualTempoN)/virtualTempoD))<=0) {
          logI("loop is on tick %d",tick);
          loopTick=tick;
          // marker
          for (int i=0; i<chans; i++) {
            chanStream[i]->writeC(0xf0);
            chanStream[i]->writeC(0x00);
            chanStream[i]->writeC(0x00);
            chanStream[i]->writeC(0x00);
          }
        }
      }
    }
    if (nextTick(false,true) || !playing) {
      done=true;
      break;
    }
    // get command stream
    if (curDivider!=divider) {
      curDivider=divider;
      chanStream[0]->writeC(0xfb);
      chanStream[0]->writeI((int)(curDivider*65536));
    }
    for (DivCommand& i: cmdStream) {
      switch (i.cmd) {
        // strip away hinted/useless commands
        case DIV_CMD_GET_VOLUME:
          break;
        case DIV_CMD_VOLUME:
          break;
        case DIV_CMD_NOTE_PORTA:
          break;
        case DIV_CMD_LEGATO:
          break;
        case DIV_CMD_PITCH:
          break;
        case DIV_CMD_PRE_NOTE:
          break;
        default:
          cmdPopularity[i.cmd]++;
          writePackedCommandValues(chanStream[i.chan],i);
          break;
      }
    }
    cmdStream.clear();
    for (int i=0; i<chans; i++) {
      chanStream[i]->writeC(0xfe);
    }
    tick++;
  }
  if (!playing || loopTick<0) {
    for (int i=0; i<chans; i++) {
      chanStream[i]->writeC(0xff);
    }
  } else {
    for (int i=0; i<chans; i++) {
      if ((int)tickPos[i].size()>loopTick) {
        chanStream[i]->writeC(0xfa);
        chanStream[i]->writeI(tickPos[i][loopTick]);
        logD("chan %d loop addr: %x",i,tickPos[i][loopTick]);
      } else {
        logW("chan %d unable to find loop addr!",i);
        chanStream[i]->writeC(0xff);
      }
    }
  }
  logV("%d",tick);
  cmdStreamEnabled=oldCmdStreamEnabled;

  remainingLoops=-1;
  playing=false;
  freelance=false;
  extValuePresent=false;
  BUSY_END;

  // PASS 1: condense delays
  // calculate delay usage
  for (int h=0; h<chans; h++) {
    unsigned char* buf=chanStream[h]->getFinalBuf();
    int delayCount=0;
    for (size_t i=0; i<chanStream[h]->size();) {
      int insLen=getInsLength(buf[i]);
      if (insLen<1) {
        logE("INS %x NOT IMPLEMENTED...",buf[i]);
        break;
      }
      if (buf[i]==0xfe) {
        delayCount++;
      } else {
        if (delayCount>1 && delayCount<=255) {
          delayPopularity[delayCount]++;
        }
        delayCount=0;
      }
      i+=insLen;
    }
  }

  // preset delays
  int sortCand=-1;
  int sortPos=0;
  while (sortPos<16) {
    sortCand=-1;
    for (int i=0; i<256; i++) {
      if (delayPopularity[i]) {
        if (sortCand==-1) {
          sortCand=i;
        } else if (delayPopularity[sortCand]<delayPopularity[i]) {
          sortCand=i;
        }
      }
    }
    if (sortCand==-1) break;

    sortedDelayPopularity[sortPos]=delayPopularity[sortCand];
    sortedDelay[sortPos]=sortCand;
    delayPopularity[sortCand]=0;
    sortPos++;
  }


  // condense delays
  for (int h=0; h<chans; h++) {
    unsigned char* buf=chanStream[h]->getFinalBuf();
    int delayPos=-1;
    int delayCount=0;
    int delayLast=0;
    for (size_t i=0; i<chanStream[h]->size();) {
      int insLen=getInsLength(buf[i]);
      if (insLen<1) {
        logE("INS %x NOT IMPLEMENTED...",buf[i]);
        break;
      }
      if (buf[i]==0xfe) {
        if (delayPos==-1) delayPos=i;
        delayCount++;
        delayLast=i;
      } else {
        // finish the last delay if any
        if (delayPos!=-1) {
          if (delayCount>1) {
            if (delayLast<delayPos) {
              logE("delayLast<delayPos! %d<%d",delayLast,delayPos);
            } else {
              // write condensed delay and fill the rest with nop
              if (delayCount>255) {
                buf[delayPos++]=0xfc;
                buf[delayPos++]=delayCount&0xff;
                buf[delayPos++]=(delayCount>>8)&0xff;
              } else {
                bool foundShort=false;
                for (int j=0; j<16; j++) {
                  if (sortedDelay[j]==delayCount) {
                    buf[delayPos++]=0xe0+j;
                    foundShort=true;
                    break;
                  }
                }
                if (!foundShort) {
                  buf[delayPos++]=0xfd;
                  buf[delayPos++]=delayCount;
                }
              }
              // fill with nop
              for (int j=delayPos; j<=delayLast; j++) {
                buf[j]=0xf1;
              }
            }
          }
          delayPos=-1;
          delayCount=0;
        }
      }
      i+=insLen;
    }
  }

  // PASS 2: remove nop's
  // this includes modifying call addresses to compensate
  for (int h=0; h<chans; h++) {
    chanStream[h]=stripNops(chanStream[h]);
  }

#ifndef DISABLE_BLOCK_SEARCH
  // PASS 3: find sub-blocks and isolate them
  for (int h=0; h<chans; h++) {
    std::vector<SafeWriter*> subBlocks;
    size_t beforeSize=chanStream[h]->size();
    
    // 6 is the minimum size that can be reliably optimized
    logI("finding sub-blocks in chan %d",h);
    chanStream[h]=findSubBlocks(chanStream[h],subBlocks);
    // find sub-blocks within sub-blocks
    size_t subBlocksLast=0;
    size_t subBlocksLen=subBlocks.size();
    logI("finding sub-blocks within sub-blocks",h);
    while (subBlocksLast!=subBlocksLen) {
      logI("got %d blocks... starting from %d",(int)subBlocksLen,(int)subBlocksLast);
      for (size_t i=subBlocksLast; i<subBlocksLen; i++) {
        SafeWriter* newBlock=findSubBlocks(subBlocks[i],subBlocks);
        subBlocks[i]=newBlock;
      }
      subBlocksLast=subBlocksLen;
      subBlocksLen=subBlocks.size();
    }

    // insert sub-blocks and resolve symbols
    logI("%d sub-blocks total",(int)subBlocks.size());
    std::vector<size_t> blockOff;
    chanStream[h]->seek(0,SEEK_END);
    for (size_t i=0; i<subBlocks.size(); i++) {
      SafeWriter* block=subBlocks[i];

      // check whether this block is duplicate
      int dupOf=-1;
      for (size_t j=0; j<i; j++) {
        if (subBlocks[j]->size()==subBlocks[i]->size()) {
          if (memcmp(subBlocks[j]->getFinalBuf(),subBlocks[i]->getFinalBuf(),subBlocks[j]->size())==0) {
            logW("we have one");
            dupOf=j;
            break;
          }
        }
      }

      if (dupOf>=0) {
        // push address of original block (discard duplicate)
        blockOff.push_back(blockOff[dupOf]);
      } else {
        // write sub-block
        blockOff.push_back(chanStream[h]->tell());
        chanStream[h]->write(block->getFinalBuf(),block->size());
      }
    }

    for (SafeWriter* block: subBlocks) {
      block->finish();
      delete block;
    }
    subBlocks.clear();

    // resolve symbols
    unsigned char* buf=chanStream[h]->getFinalBuf();
    for (size_t j=0; j<chanStream[h]->size();) {
      int insLen=getInsLength(buf[j]);
      if (insLen<1) {
        logE("INS %x NOT IMPLEMENTED...",buf[j]);
        break;
      }
      if (buf[j]==0xf4) { // callsym
        unsigned int addr=buf[j+1]|(buf[j+2]<<8)|(buf[j+3]<<8)|(buf[j+4]<<24);
        if (addr<blockOff.size()) {
          // turn it into call
          addr=blockOff[addr];
          buf[j]=0xf5;
          buf[j+1]=addr&0xff;
          buf[j+2]=(addr>>8)&0xff;
          buf[j+3]=(addr>>16)&0xff;
          buf[j+4]=(addr>>24)&0xff;
        } else {
          logE("requested symbol %d is out of bounds!",addr);
        }
      }
      j+=insLen;
    }

    size_t afterSize=chanStream[h]->size();
    logI("(before: %d - after: %d)",(int)beforeSize,(int)afterSize);
  }
#endif

  // PASS 4: remove nop's (again)
  for (int h=0; h<chans; h++) {
    chanStream[h]=stripNops(chanStream[h]);
  }

  // PASS 5: optimize command calls
  /*
  int sortCand=-1;
  int sortPos=0;
  while (sortPos<16) {
    sortCand=-1;
    for (int i=DIV_CMD_SAMPLE_MODE; i<256; i++) {
      if (cmdPopularity[i]) {
        if (sortCand==-1) {
          sortCand=i;
        } else if (cmdPopularity[sortCand]<cmdPopularity[i]) {
          sortCand=i;
        }
      }
    }
    if (sortCand==-1) break;

    sortedCmdPopularity[sortPos]=cmdPopularity[sortCand];
    sortedCmd[sortPos]=sortCand;
    cmdPopularity[sortCand]=0;
    sortPos++;
  }*/

  /*
  for (int i=0; i<chans; i++) {
    // optimize stream
    SafeWriter* oldStream=chanStream[i];
    SafeReader* reader=oldStream->toReader();
    chanStream[i]=new SafeWriter;
    chanStream[i]->init();

    while (1) {
      try {
        unsigned char next=reader->readC();
        switch (next) {
          case 0xb8: // instrument
          case 0xc0: // pre porta
          case 0xc3: // vibrato range
          case 0xc4: // vibrato shape
          case 0xc5: // pitch
          case 0xc7: // volume
          case 0xca: // legato
            chanStream[i]->writeC(next);
            next=reader->readC();
            chanStream[i]->writeC(next);
            break;
          case 0xbe: // panning
          case 0xc2: // vibrato
          case 0xc6: // arpeggio
          case 0xc8: // vol slide
          case 0xc9: // porta
            chanStream[i]->writeC(next);
            next=reader->readC();
            chanStream[i]->writeC(next);
            next=reader->readC();
            chanStream[i]->writeC(next);
            break;
          case 0xf2: { // full command (pre)
            unsigned char cmd=reader->readC();
            bool foundShort=false;
            for (int j=0; j<16; j++) {
              if (sortedCmd[j]==cmd) {
                chanStream[i]->writeC(0xd0+j);
                foundShort=true;
                break;
              }
            }
            if (!foundShort) {
              chanStream[i]->writeC(0xf7); // full command
              chanStream[i]->writeC(cmd);
            }

            unsigned char cmdLen=reader->readC();
            logD("cmdLen: %d",cmdLen);
            for (unsigned char j=0; j<cmdLen; j++) {
              next=reader->readC();
              chanStream[i]->writeC(next);
            }
            break;
          }
          case 0xfb: // tick rate
            chanStream[i]->writeC(next);
            next=reader->readC();
            chanStream[i]->writeC(next);
            next=reader->readC();
            chanStream[i]->writeC(next);
            next=reader->readC();
            chanStream[i]->writeC(next);
            next=reader->readC();
            chanStream[i]->writeC(next);
            break;
          case 0xfc: { // 16-bit wait
            unsigned short delay=reader->readS();
            bool foundShort=false;
            for (int j=0; j<16; j++) {
              if (sortedDelay[j]==delay) {
                chanStream[i]->writeC(0xe0+j);
                foundShort=true;
                break;
              }
            }
            if (!foundShort) {
              chanStream[i]->writeC(next);
              chanStream[i]->writeS(delay);
            }
            break;
          }
          case 0xfd: { // 8-bit wait
            unsigned char delay=reader->readC();
            bool foundShort=false;
            for (int j=0; j<16; j++) {
              if (sortedDelay[j]==delay) {
                chanStream[i]->writeC(0xe0+j);
                foundShort=true;
                break;
              }
            }
            if (!foundShort) {
              chanStream[i]->writeC(next);
              chanStream[i]->writeC(delay);
            }
            break;
          }
          default:
            chanStream[i]->writeC(next);
            break;
        }
      } catch (EndOfFileException& e) {
        break;
      }
    }

    oldStream->finish();
    delete oldStream;
  }*/

  // write results
  for (int i=0; i<chans; i++) {
    chanStreamOff[i]=w->tell();
    logI("- %d: off %x size %ld",i,chanStreamOff[i],chanStream[i]->size());
    reloc(chanStream[i]->getFinalBuf(),chanStream[i]->size(),0,w->tell());
    w->write(chanStream[i]->getFinalBuf(),chanStream[i]->size());
    chanStream[i]->finish();
    delete chanStream[i];
  }

  w->seek(8,SEEK_SET);
  for (int i=0; i<chans; i++) {
    w->writeI(chanStreamOff[i]);
  }

  logD("delay popularity:");
  for (int i=0; i<16; i++) {
    w->writeC(sortedDelay[i]);
    if (sortedDelayPopularity[i]) logD("- %d: %d",sortedDelay[i],sortedDelayPopularity[i]);
  }

  logD("command popularity:");
  for (int i=0; i<16; i++) {
    w->writeC(sortedCmd[i]);
    if (sortedCmdPopularity[i]) logD("- %s: %d",cmdName[sortedCmd[i]],sortedCmdPopularity[i]);
  }

  return w;
}
