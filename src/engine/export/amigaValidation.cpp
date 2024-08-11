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

#include "amigaValidation.h"
#include "../engine.h"
#include "../platform/amiga.h"

struct WaveEntry {
  unsigned int pos;
  short width;
  signed char data[256];
  WaveEntry():
    pos(0),
    width(32) {
    memset(data,0,256);
  }
};

struct SampleBookEntry {
  unsigned int loc;
  unsigned short len;
  SampleBookEntry():
    loc(0),
    len(0) {}
};

void DivExportAmigaValidation::run() {
  std::vector<WaveEntry> waves;
  std::vector<SampleBookEntry> sampleBook;
  unsigned int wavesDataPtr=0;
  WaveEntry curWaveState[4];
  unsigned int sampleBookLoc=0;

  DivPlatformAmiga* amiga=(DivPlatformAmiga*)e->getDispatch(0);

  e->stop();
  e->repeatPattern=false;
  e->setOrder(0);
  EXTERN_BUSY_BEGIN_SOFT;

  // determine loop point
  int loopOrder=0;
  int loopRow=0;
  int loopEnd=0;
  e->walkSong(loopOrder,loopRow,loopEnd);

  e->curOrder=0;
  e->freelance=false;
  e->playing=false;
  e->extValuePresent=false;
  e->remainingLoops=-1;

  // play the song ourselves
  bool done=false;

  // sample.bin
  SafeWriter* sample=new SafeWriter;
  sample->init();
  for (int i=0; i<256; i++) {
    sample->writeI(0);
  }
  sample->write(&((const unsigned char*)amiga->getSampleMem(0))[0x400],amiga->getSampleMemUsage(0)-0x400);
  if (sample->tell()&1) sample->writeC(0);

  // seq.bin
  SafeWriter* seq=new SafeWriter;
  seq->init();

  amiga->toggleRegisterDump(true);

  // write song data
  e->playSub(false);
  size_t songTick=0;
  size_t lastTick=0;
  //bool writeLoop=false;
  int loopPos=-1;
  for (int i=0; i<e->chans; i++) {
    e->chan[i].wentThroughNote=false;
    e->chan[i].goneThroughNote=false;
  }
  while (!done) {
    if (loopPos==-1) {
      if (loopOrder==e->curOrder && loopRow==e->curRow && e->ticks==1) {
        //writeLoop=true;
      }
    }
    if (e->nextTick(false,true)) {
      done=true;
      amiga->getRegisterWrites().clear();
      if (lastTick!=songTick) {
        int delta=songTick-lastTick;
        if (delta==1) {
          seq->writeC(0xf1);
        } else if (delta<256) {
          seq->writeC(0xf2);
          seq->writeC(delta-1);
        } else if (delta<32768) {
          seq->writeC(0xf3);
          seq->writeS_BE(delta-1);
        }
        lastTick=songTick;
      }
      break;
    }
    // check wavetable changes
    for (int i=0; i<4; i++) {
      if (amiga->chan[i].useWave) {
        if ((amiga->chan[i].audLen*2)!=curWaveState[i].width || memcmp(curWaveState[i].data,&(((signed char*)amiga->getSampleMem())[i<<8]),amiga->chan[i].audLen*2)!=0) {
          curWaveState[i].width=amiga->chan[i].audLen*2;
          memcpy(curWaveState[i].data,&(((signed char*)amiga->getSampleMem())[i<<8]),amiga->chan[i].audLen*2);

          int waveNum=-1;
          for (size_t j=0; j<waves.size(); j++) {
            if (waves[j].width!=curWaveState[i].width) continue;
            if (memcmp(waves[j].data,curWaveState[i].data,curWaveState[i].width)==0) {
              waveNum=j;
              break;
            }
          }
          
          if (waveNum==-1) {
            // write new wavetable
            waveNum=(int)waves.size();
            curWaveState[i].pos=wavesDataPtr;
            waves.push_back(curWaveState[i]);
            wavesDataPtr+=curWaveState[i].width;
          }

          if (waveNum<256) {
            seq->writeC((i<<4)|3);
            seq->writeC(waveNum);
          } else if (waveNum<65536) {
            seq->writeC((i<<4)|4);
            seq->writeS_BE(waveNum);
          } else {
            seq->writeC((i<<4)|1);
            seq->writeC(waves[waveNum].pos>>16);
            seq->writeC(waves[waveNum].pos>>8);
            seq->writeC(waves[waveNum].pos);
            seq->writeS_BE(waves[waveNum].width);
          }
        }
      }
    }

    // get register writes
    std::vector<DivRegWrite>& writes=amiga->getRegisterWrites();
    for (DivRegWrite& j: writes) {
      if (lastTick!=songTick) {
        int delta=songTick-lastTick;
        if (delta==1) {
          seq->writeC(0xf1);
        } else if (delta<256) {
          seq->writeC(0xf2);
          seq->writeC(delta-1);
        } else if (delta<32768) {
          seq->writeC(0xf3);
          seq->writeS_BE(delta-1);
        }
        lastTick=songTick;
      }
      if (j.addr>=0x200) { // direct loc/len change
        if (j.addr&4) { // len
          int sampleBookIndex=-1;
          for (size_t i=0; i<sampleBook.size(); i++) {
            if (sampleBook[i].loc==sampleBookLoc && sampleBook[i].len==(j.val&0xffff)) {
              sampleBookIndex=i;
              break;
            }
          }
          if (sampleBookIndex==-1) {
            if (sampleBook.size()<256) {
              SampleBookEntry e;
              e.loc=sampleBookLoc;
              e.len=j.val&0xffff;
              sampleBookIndex=sampleBook.size();
              sampleBook.push_back(e);
            }
          }

          if (sampleBookIndex==-1) {
            seq->writeC((j.addr&3)<<4);
            seq->writeC(sampleBookLoc>>16);
            seq->writeC(sampleBookLoc>>8);
            seq->writeC(sampleBookLoc);
            seq->writeS_BE(j.val);
          } else {
            seq->writeC(((j.addr&3)<<4)|2);
            seq->writeC(sampleBookIndex);
          }
        } else { // loc
          sampleBookLoc=j.val;
        }
      } else if (j.addr<0xa0) {
        // don't write INTENA
        if ((j.addr&15)!=10) {
          seq->writeC(0xf0|(j.addr&15));
          seq->writeS_BE(j.val);
        }
      } else if ((j.addr&15)!=0 && (j.addr&15)!=2 && (j.addr&15)!=4) {
        seq->writeC(j.addr-0xa0);
        if ((j.addr&15)==8) {
          seq->writeC(j.val);
        } else {
          seq->writeS_BE(j.val);
        }
      }
    }
    writes.clear();

    songTick++;
  }
  // end of song
  seq->writeC(0xff);

  amiga->toggleRegisterDump(false);

  e->remainingLoops=-1;
  e->playing=false;
  e->freelance=false;
  e->extValuePresent=false;

  EXTERN_BUSY_END;

  // wave.bin
  SafeWriter* wave=new SafeWriter;
  wave->init();
  for (WaveEntry& i: waves) {
    wave->write(i.data,i.width);
  }
  
  // sbook.bin
  SafeWriter* sbook=new SafeWriter;
  sbook->init();
  for (SampleBookEntry& i: sampleBook) {
    // 8 bytes per entry
    sbook->writeI_BE(i.loc);
    sbook->writeI_BE(i.len);
  }

  // wbook.bin
  SafeWriter* wbook=new SafeWriter;
  wbook->init();
  for (WaveEntry& i: waves) {
    wbook->writeC(i.width);
    wbook->writeC(i.pos>>16);
    wbook->writeC(i.pos>>8);
    wbook->writeC(i.pos);
  }

  // finish
  output.reserve(5);
  output.push_back(DivROMExportOutput("sbook.bin",sbook));
  output.push_back(DivROMExportOutput("wbook.bin",wbook));
  output.push_back(DivROMExportOutput("sample.bin",sample));
  output.push_back(DivROMExportOutput("wave.bin",wave));
  output.push_back(DivROMExportOutput("seq.bin",seq));

  running=false;
}

bool DivExportAmigaValidation::go(DivEngine* eng) {
  e=eng;
  running=true;
  exportThread=new std::thread(&DivExportAmigaValidation::run,this);
  return true;
}

void DivExportAmigaValidation::wait() {
  if (exportThread!=NULL) {
    exportThread->join();
    delete exportThread;
  }
}

void DivExportAmigaValidation::abort() {
  wait();
}

bool DivExportAmigaValidation::isRunning() {
  return running;
}
