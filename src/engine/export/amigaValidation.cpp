/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2023 tildearrow and contributors
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

std::vector<DivROMExportOutput> DivExportAmigaValidation::go(DivEngine* e) {
  std::vector<DivROMExportOutput> ret;
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

  // wave.bin
  SafeWriter* wave=new SafeWriter;
  wave->init();
  for (int i=0; i<32; i++) {
    wave->writeC(i<<3);
  }

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
      break;
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
          seq->writeS_BE(j.val);
        } else { // loc
          seq->writeC((j.addr&3)<<4);
          seq->writeC(j.val>>16);
          seq->writeC(j.val>>8);
          seq->writeC(j.val);
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

  // finish
  ret.push_back(DivROMExportOutput("sample.bin",sample));
  ret.push_back(DivROMExportOutput("wave.bin",wave));
  ret.push_back(DivROMExportOutput("seq.bin",seq));

  return ret;
}
