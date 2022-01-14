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
#include "platform/dummy.h"
#include "../ta-log.h"

void DivDispatchContainer::setRates(double gotRate) {
  blip_set_rates(bb[0],dispatch->rate,gotRate);
  blip_set_rates(bb[1],dispatch->rate,gotRate);
}

void DivDispatchContainer::acquire(size_t offset, size_t count) {
  dispatch->acquire(bbIn[0],bbIn[1],offset,count);
}

void DivDispatchContainer::fillBuf(size_t runtotal, size_t size) {
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

  blip_end_frame(bb[0],runtotal);
  blip_read_samples(bb[0],bbOut[0],size,0);

  if (dispatch->isStereo()) {
    blip_end_frame(bb[1],runtotal);
    blip_read_samples(bb[1],bbOut[1],size,0);
  }
}

void DivDispatchContainer::clear() {
  blip_clear(bb[0]);
  blip_clear(bb[1]);
  temp[0]=0;
  temp[1]=0;
  prevSample[0]=0;
  prevSample[1]=0;
}

void DivDispatchContainer::init(DivSystem sys, DivEngine* eng, int chanCount, double gotRate, bool pal) {
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
      break;
    case DIV_SYSTEM_GENESIS_EXT:
      dispatch=new DivPlatformGenesisExt;
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
      ((DivPlatformArcade*)dispatch)->setYMFM(true);
      break;
    case DIV_SYSTEM_YM2610:
      dispatch=new DivPlatformYM2610;
      break;
    case DIV_SYSTEM_YM2610_EXT:
      dispatch=new DivPlatformYM2610Ext;
      break;
    case DIV_SYSTEM_AY8910:
      dispatch=new DivPlatformAY8910;
      break;
    case DIV_SYSTEM_AY8930:
      dispatch=new DivPlatformAY8930;
      break;
    default:
      logW("this system is not supported yet! using dummy platform.\n");
      dispatch=new DivPlatformDummy;
      break;
  }
  dispatch->init(eng,chanCount,gotRate,pal);
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
