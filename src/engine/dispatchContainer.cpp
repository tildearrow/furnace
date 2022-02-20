/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2022 tildearrow and contributors
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

#include "blip_buf.h"
#include "engine.h"
#include "platform/genesis.h"
#include "platform/genesisext.h"
#include "platform/sms.h"
#include "platform/gb.h"
#include "platform/pce.h"
#include "platform/nes.h"
#include "platform/c64.h"
#include "platform/arcade.h"
#include "platform/ym2610.h"
#include "platform/ym2610ext.h"
#include "platform/ay.h"
#include "platform/ay8930.h"
#include "platform/tia.h"
#include "platform/saa.h"
#include "platform/amiga.h"
#include "platform/dummy.h"
#include "platform/lynx.h"
#include "../ta-log.h"
#include "song.h"

void DivDispatchContainer::setRates(double gotRate) {
  blip_set_rates(bb[0],dispatch->rate,gotRate);
  blip_set_rates(bb[1],dispatch->rate,gotRate);
}

void DivDispatchContainer::setQuality(bool lowQual) {
  lowQuality=lowQual;
}

void DivDispatchContainer::acquire(size_t offset, size_t count) {
  dispatch->acquire(bbIn[0],bbIn[1],offset,count);
}

void DivDispatchContainer::flush(size_t count) {
  blip_read_samples(bb[0],bbOut[0],count,0);

  if (dispatch->isStereo()) {
    blip_read_samples(bb[1],bbOut[1],count,0);
  }
}

void DivDispatchContainer::fillBuf(size_t runtotal, size_t offset, size_t size) {
  if (lowQuality) {
    for (size_t i=0; i<runtotal; i++) {
      temp[0]=bbIn[0][i];
      blip_add_delta_fast(bb[0],i,temp[0]-prevSample[0]);
      prevSample[0]=temp[0];
    }

    if (dispatch->isStereo()) for (size_t i=0; i<runtotal; i++) {
      temp[1]=bbIn[1][i];
      blip_add_delta_fast(bb[1],i,temp[1]-prevSample[1]);
      prevSample[1]=temp[1];
    }
  } else {
    for (size_t i=0; i<runtotal; i++) {
      temp[0]=bbIn[0][i];
      blip_add_delta(bb[0],i,temp[0]-prevSample[0]);
      prevSample[0]=temp[0];
    }

    if (dispatch->isStereo()) for (size_t i=0; i<runtotal; i++) {
      temp[1]=bbIn[1][i];
      blip_add_delta(bb[1],i,temp[1]-prevSample[1]);
      prevSample[1]=temp[1];
    }
  }

  blip_end_frame(bb[0],runtotal);
  blip_read_samples(bb[0],bbOut[0]+offset,size,0);
  /*if (totalRead<(int)size && totalRead>0) {
    for (size_t i=totalRead; i<size; i++) {
      bbOut[0][i]=bbOut[0][totalRead-1];//bbOut[0][totalRead];
    }
  }*/

  if (dispatch->isStereo()) {
    blip_end_frame(bb[1],runtotal);
    blip_read_samples(bb[1],bbOut[1]+offset,size,0);
  }
}

void DivDispatchContainer::clear() {
  blip_clear(bb[0]);
  blip_clear(bb[1]);
  temp[0]=0;
  temp[1]=0;
  prevSample[0]=0;
  prevSample[1]=0;
  // run for one cycle to determine DC offset
  // TODO: SAA1099 doesn't like that
  /*dispatch->acquire(bbIn[0],bbIn[1],0,1);
  temp[0]=bbIn[0][0];
  temp[1]=bbIn[1][0];
  prevSample[0]=temp[0];
  prevSample[1]=temp[1];*/
}

void DivDispatchContainer::init(DivSystem sys, DivEngine* eng, int chanCount, double gotRate, unsigned int flags) {
  if (dispatch!=NULL) return;

  bb[0]=blip_new(32768);
  if (bb[0]==NULL) {
    logE("not enough memory!\n");
    return;
  }

  bb[1]=blip_new(32768);
  if (bb[1]==NULL) {
    logE("not enough memory!\n");
    return;
  }

  bbOut[0]=new short[32768];
  bbOut[1]=new short[32768];
  bbIn[0]=new short[32768];
  bbIn[1]=new short[32768];
  bbInLen=32768;

  switch (sys) {
    case DIV_SYSTEM_GENESIS:
    case DIV_SYSTEM_YM2612:
      dispatch=new DivPlatformGenesis;
      ((DivPlatformGenesis*)dispatch)->setYMFM(eng->getConfInt("ym2612Core",0));
      break;
    case DIV_SYSTEM_GENESIS_EXT:
      dispatch=new DivPlatformGenesisExt;
      ((DivPlatformGenesisExt*)dispatch)->setYMFM(eng->getConfInt("ym2612Core",0));
      break;
    case DIV_SYSTEM_SMS:
      dispatch=new DivPlatformSMS;
      break;
    case DIV_SYSTEM_GB:
      dispatch=new DivPlatformGB;
      break;
    case DIV_SYSTEM_PCE:
      dispatch=new DivPlatformPCE;
      break;
    case DIV_SYSTEM_NES:
      dispatch=new DivPlatformNES;
      break;
    case DIV_SYSTEM_C64_6581:
      dispatch=new DivPlatformC64;
      ((DivPlatformC64*)dispatch)->setChipModel(true);
      break;
    case DIV_SYSTEM_C64_8580:
      dispatch=new DivPlatformC64;
      ((DivPlatformC64*)dispatch)->setChipModel(false);
      break;
    case DIV_SYSTEM_ARCADE:
    case DIV_SYSTEM_YM2151:
      dispatch=new DivPlatformArcade;
      ((DivPlatformArcade*)dispatch)->setYMFM(eng->getConfInt("arcadeCore",0)==0);
      break;
    case DIV_SYSTEM_YM2610:
    case DIV_SYSTEM_YM2610_FULL:
      dispatch=new DivPlatformYM2610;
      break;
    case DIV_SYSTEM_YM2610_EXT:
    case DIV_SYSTEM_YM2610_FULL_EXT:
      dispatch=new DivPlatformYM2610Ext;
      break;
    case DIV_SYSTEM_AMIGA:
      dispatch=new DivPlatformAmiga;
      break;
    case DIV_SYSTEM_AY8910:
      dispatch=new DivPlatformAY8910;
      break;
    case DIV_SYSTEM_AY8930:
      dispatch=new DivPlatformAY8930;
      break;
    case DIV_SYSTEM_TIA:
      dispatch=new DivPlatformTIA;
      break;
    case DIV_SYSTEM_SAA1099: {
      int saaCore=eng->getConfInt("saaCore",0);
      if (saaCore<0 || saaCore>2) saaCore=0;
      dispatch=new DivPlatformSAA1099;
      ((DivPlatformSAA1099*)dispatch)->setCore((DivSAACores)saaCore);
      break;
    }
    case DIV_SYSTEM_LYNX:
      dispatch = new DivPlatformLynx;
      break;
    default:
      logW("this system is not supported yet! using dummy platform.\n");
      dispatch=new DivPlatformDummy;
      break;
  }
  dispatch->init(eng,chanCount,gotRate,flags);
}

void DivDispatchContainer::quit() {
  if (dispatch==NULL) return;
  dispatch->quit();
  delete dispatch;
  dispatch=NULL;

  delete[] bbOut[0];
  delete[] bbOut[1];
  delete[] bbIn[0];
  delete[] bbIn[1];
  bbInLen=0;
  blip_delete(bb[0]);
  blip_delete(bb[1]);
}
