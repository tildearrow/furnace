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

#include "blip_buf.h"
#include "engine.h"
#include "platform/genesis.h"
#include "platform/genesisext.h"
#include "platform/msm5232.h"
#include "platform/msm6258.h"
#include "platform/msm6295.h"
#include "platform/namcowsg.h"
#include "platform/sms.h"
#include "platform/opll.h"
#include "platform/gb.h"
#include "platform/pce.h"
#include "platform/nes.h"
#include "platform/c64.h"
#include "platform/arcade.h"
#include "platform/t6w28.h"
#include "platform/tx81z.h"
#include "platform/ym2203.h"
#include "platform/ym2203ext.h"
#include "platform/ym2608.h"
#include "platform/ym2608ext.h"
#include "platform/ym2610.h"
#include "platform/ym2610ext.h"
#include "platform/ym2610b.h"
#include "platform/ym2610bext.h"
#include "platform/ay.h"
#include "platform/ay8930.h"
#include "platform/opl.h"
#include "platform/tia.h"
#include "platform/saa.h"
#include "platform/amiga.h"
#include "platform/pcspkr.h"
#include "platform/pokemini.h"
#include "platform/segapcm.h"
#include "platform/qsound.h"
#include "platform/vera.h"
#include "platform/x1_010.h"
#include "platform/su.h"
#include "platform/swan.h"
#include "platform/lynx.h"
#include "platform/pokey.h"
#include "platform/zxbeeper.h"
#include "platform/zxbeeperquadtone.h"
#include "platform/bubsyswsg.h"
#include "platform/n163.h"
#include "platform/pet.h"
#include "platform/pong.h"
#include "platform/vic20.h"
#include "platform/vrc6.h"
#include "platform/fds.h"
#include "platform/mmc5.h"
#include "platform/es5506.h"
#include "platform/scc.h"
#include "platform/ymz280b.h"
#include "platform/rf5c68.h"
#include "platform/snes.h"
#include "platform/vb.h"
#include "platform/k007232.h"
#include "platform/ga20.h"
#include "platform/supervision.h"
#include "platform/scvwave.h"
#include "platform/scvtone.h"
#include "platform/sm8521.h"
#include "platform/pv1000.h"
#include "platform/k053260.h"
#include "platform/ted.h"
#include "platform/c140.h"
#include "platform/gbadma.h"
#include "platform/gbaminmod.h"
#include "platform/pcmdac.h"
#include "platform/esfm.h"
#include "platform/powernoise.h"
#include "platform/dave.h"
#include "platform/nds.h"
#include "platform/bifurcator.h"
#include "platform/sid2.h"
#include "platform/sid3.h"
#include "platform/dummy.h"
#include "../ta-log.h"
#include "song.h"

void DivDispatchContainer::setRates(double gotRate) {
  int outs=dispatch->getOutputCount();

  for (int i=0; i<outs; i++) {
    if (bb[i]==NULL) continue;
    blip_set_rates(bb[i],dispatch->rate,gotRate);
  }
  rateMemory=gotRate;
}

void DivDispatchContainer::setQuality(bool lowQual, bool dcHiPass) {
  lowQuality=lowQual;
  hiPass=dcHiPass;
  for (int i=0; i<DIV_MAX_OUTPUTS; i++) {
    if (bb[i]==NULL) continue;
    blip_set_dc(bb[i],dcHiPass);
  }
}

void DivDispatchContainer::grow(size_t size) {
  bbInLen=size;
  for (int i=0; i<DIV_MAX_OUTPUTS; i++) {
    if (bbIn[i]!=NULL) {
      delete[] bbIn[i];
      bbIn[i]=new short[bbInLen];
    }
  }
}

#define CHECK_MISSING_BUFS \
  int outs=dispatch->getOutputCount(); \
 \
  /* create missing buffers if any */ \
  bool mustClear=false; \
  for (int i=0; i<outs; i++) { \
    if (bb[i]==NULL) { \
      logV("creating buf %d because it doesn't exist",i); \
      bb[i]=blip_new(bbInLen); \
      if (bb[i]==NULL) { \
        logE("not enough memory!"); \
        return; \
      } \
      blip_set_dc(bb[i],hiPass); \
      blip_set_rates(bb[i],dispatch->rate,rateMemory); \
 \
      if (bbIn[i]==NULL) bbIn[i]=new short[bbInLen]; \
      if (bbOut[i]==NULL) bbOut[i]=new short[bbInLen]; \
      memset(bbIn[i],0,bbInLen*sizeof(short)); \
      memset(bbOut[i],0,bbInLen*sizeof(short)); \
      mustClear=true; \
    } \
  } \
  if (mustClear) clear(); \

void DivDispatchContainer::acquire(size_t count) {
  CHECK_MISSING_BUFS;

  if (dispatch->hasAcquireDirect()) {
    dispatch->acquireDirect(bb,count);
  } else {
    for (int i=0; i<DIV_MAX_OUTPUTS; i++) {
      if (i>=outs) {
        bbInMapped[i]=NULL;
      } else {
        if (bbIn[i]==NULL) {
          bbInMapped[i]=NULL;
        } else {
          bbInMapped[i]=bbIn[i];
        }
      }
    }
    dispatch->acquire(bbInMapped,count);
  }
}

void DivDispatchContainer::flush(size_t offset, size_t count) {
  int outs=dispatch->getOutputCount();

  for (int i=0; i<outs; i++) {
    if (bb[i]==NULL) continue;
    blip_read_samples(bb[i],bbOut[i]+offset,count,0);
  }
}

void DivDispatchContainer::fillBuf(size_t runtotal, size_t offset, size_t size) {
  CHECK_MISSING_BUFS;

  if (!dispatch->hasAcquireDirect()) {
    if (dcOffCompensation && runtotal>0) {
      dcOffCompensation=false;
      if (hiPass) {
        for (int i=0; i<outs; i++) {
          if (bbIn[i]==NULL) continue;
          prevSample[i]=bbIn[i][0];
        }
      }
    }
    if (lowQuality) {
      for (int i=0; i<outs; i++) {
        if (bbIn[i]==NULL) continue;
        if (bb[i]==NULL) continue;
        for (size_t j=0; j<runtotal; j++) {
          if (bbIn[i][j]==temp[i]) continue;
          temp[i]=bbIn[i][j];
          blip_add_delta_fast(bb[i],j,temp[i]-prevSample[i]);
          prevSample[i]=temp[i];
        }
      }
    } else {
      for (int i=0; i<outs; i++) {
        if (bbIn[i]==NULL) continue;
        if (bb[i]==NULL) continue;
        for (size_t j=0; j<runtotal; j++) {
          if (bbIn[i][j]==temp[i]) continue;
          temp[i]=bbIn[i][j];
          blip_add_delta(bb[i],j,temp[i]-prevSample[i]);
          prevSample[i]=temp[i];
        }
      }
    }
  }

  for (int i=0; i<outs; i++) {
    if (bbOut[i]==NULL) continue;
    if (bb[i]==NULL) continue;
    blip_end_frame(bb[i],runtotal);
    blip_read_samples(bb[i],bbOut[i]+offset,size,0);
    dispatch->postProcess(bbOut[i]+offset,i,size,rateMemory);
  }
}

void DivDispatchContainer::clear() {
  for (int i=0; i<DIV_MAX_OUTPUTS; i++) {
    if (bb[i]!=NULL) blip_clear(bb[i]);
    temp[i]=0;
    prevSample[i]=0;
  }

  if (dispatch->getDCOffRequired() && hiPass) {
    dcOffCompensation=true;
  }
}

void DivDispatchContainer::init(DivSystem sys, DivEngine* eng, int chanCount, double gotRate, const DivConfig& flags, bool isRender) {
  // quit if we already initialized
  if (dispatch!=NULL) return;

  // initialize chip
  switch (sys) {
    case DIV_SYSTEM_YMU759:
      dispatch=new DivPlatformOPL;
      ((DivPlatformOPL*)dispatch)->setOPLType(759,false);
      break;
    case DIV_SYSTEM_YM2612:
      dispatch=new DivPlatformGenesis;
      if (isRender) {
        ((DivPlatformGenesis*)dispatch)->setYMFM(eng->getConfInt("ym2612CoreRender",0));
      } else {
        ((DivPlatformGenesis*)dispatch)->setYMFM(eng->getConfInt("ym2612Core",0));
      }
      ((DivPlatformGenesis*)dispatch)->setSoftPCM(false);
      break;
    case DIV_SYSTEM_YM2612_EXT:
      dispatch=new DivPlatformGenesisExt;
      if (isRender) {
        ((DivPlatformGenesisExt*)dispatch)->setYMFM(eng->getConfInt("ym2612CoreRender",0));
      } else {
        ((DivPlatformGenesisExt*)dispatch)->setYMFM(eng->getConfInt("ym2612Core",0));
      }
      ((DivPlatformGenesisExt*)dispatch)->setSoftPCM(false);
      break;
    case DIV_SYSTEM_YM2612_CSM:
      dispatch=new DivPlatformGenesisExt;
      if (isRender) {
        ((DivPlatformGenesisExt*)dispatch)->setYMFM(eng->getConfInt("ym2612CoreRender",0));
      } else {
        ((DivPlatformGenesisExt*)dispatch)->setYMFM(eng->getConfInt("ym2612Core",0));
      }
      ((DivPlatformGenesisExt*)dispatch)->setSoftPCM(false);
      ((DivPlatformGenesisExt*)dispatch)->setCSMChannel(6);
      break;
    case DIV_SYSTEM_YM2612_DUALPCM:
      dispatch=new DivPlatformGenesis;
      if (isRender) {
        ((DivPlatformGenesis*)dispatch)->setYMFM(eng->getConfInt("ym2612CoreRender",0));
      } else {
        ((DivPlatformGenesis*)dispatch)->setYMFM(eng->getConfInt("ym2612Core",0));
      }
      ((DivPlatformGenesis*)dispatch)->setSoftPCM(true);
      break;
    case DIV_SYSTEM_YM2612_DUALPCM_EXT:
      dispatch=new DivPlatformGenesisExt;
      if (isRender) {
        ((DivPlatformGenesisExt*)dispatch)->setYMFM(eng->getConfInt("ym2612CoreRender",0));
      } else {
        ((DivPlatformGenesisExt*)dispatch)->setYMFM(eng->getConfInt("ym2612Core",0));
      }
      ((DivPlatformGenesisExt*)dispatch)->setSoftPCM(true);
      break;
    case DIV_SYSTEM_SMS:
      dispatch=new DivPlatformSMS;
      if (isRender) {
        ((DivPlatformSMS*)dispatch)->setNuked(eng->getConfInt("snCoreRender",0));
      } else {
        ((DivPlatformSMS*)dispatch)->setNuked(eng->getConfInt("snCore",0));
      }
      break;
    case DIV_SYSTEM_GB:
      dispatch=new DivPlatformGB;
      if (isRender) {
        ((DivPlatformGB*)dispatch)->setCoreQuality(eng->getConfInt("gbQualityRender",3));
      } else {
        ((DivPlatformGB*)dispatch)->setCoreQuality(eng->getConfInt("gbQuality",3));
      }
      break;
    case DIV_SYSTEM_PCE:
      dispatch=new DivPlatformPCE;
      break;
    case DIV_SYSTEM_NES:
      dispatch=new DivPlatformNES;
      if (isRender) {
        ((DivPlatformNES*)dispatch)->setNSFPlay(eng->getConfInt("nesCoreRender",0)==1);
      } else {
        ((DivPlatformNES*)dispatch)->setNSFPlay(eng->getConfInt("nesCore",0)==1);
      }
      ((DivPlatformNES*)dispatch)->set5E01(false);
      break;
    case DIV_SYSTEM_C64_6581:
    case DIV_SYSTEM_C64_PCM:
      dispatch=new DivPlatformC64;
      if (isRender) {
        ((DivPlatformC64*)dispatch)->setCore(eng->getConfInt("c64CoreRender",1));
        ((DivPlatformC64*)dispatch)->setCoreQuality(eng->getConfInt("dsidQualityRender",3));
      } else {
        ((DivPlatformC64*)dispatch)->setCore(eng->getConfInt("c64Core",0));
        ((DivPlatformC64*)dispatch)->setCoreQuality(eng->getConfInt("dsidQuality",3));
      }
      ((DivPlatformC64*)dispatch)->setChipModel(true);
      ((DivPlatformC64*)dispatch)->setSoftPCM(sys==DIV_SYSTEM_C64_PCM);
      break;
    case DIV_SYSTEM_C64_8580:
      dispatch=new DivPlatformC64;
      if (isRender) {
        ((DivPlatformC64*)dispatch)->setCore(eng->getConfInt("c64CoreRender",1));
        ((DivPlatformC64*)dispatch)->setCoreQuality(eng->getConfInt("dsidQualityRender",3));
      } else {
        ((DivPlatformC64*)dispatch)->setCore(eng->getConfInt("c64Core",0));
        ((DivPlatformC64*)dispatch)->setCoreQuality(eng->getConfInt("dsidQuality",3));
      }
      ((DivPlatformC64*)dispatch)->setChipModel(false);
      ((DivPlatformC64*)dispatch)->setSoftPCM(false);
      break;
    case DIV_SYSTEM_YM2151:
      dispatch=new DivPlatformArcade;
      if (isRender) {
        ((DivPlatformArcade*)dispatch)->setYMFM(eng->getConfInt("arcadeCoreRender",1)==0);
      } else {
        ((DivPlatformArcade*)dispatch)->setYMFM(eng->getConfInt("arcadeCore",0)==0);
      }
      break;
    case DIV_SYSTEM_YM2610:
    case DIV_SYSTEM_YM2610_FULL:
      dispatch=new DivPlatformYM2610;
      if (isRender) {
        ((DivPlatformYM2610*)dispatch)->setCombo(eng->getConfInt("opnbCoreRender",1));
      } else {
        ((DivPlatformYM2610*)dispatch)->setCombo(eng->getConfInt("opnbCore",1));
      }
      break;
    case DIV_SYSTEM_YM2610_EXT:
    case DIV_SYSTEM_YM2610_FULL_EXT:
      dispatch=new DivPlatformYM2610Ext;
      if (isRender) {
        ((DivPlatformYM2610Ext*)dispatch)->setCombo(eng->getConfInt("opnbCoreRender",1));
      } else {
        ((DivPlatformYM2610Ext*)dispatch)->setCombo(eng->getConfInt("opnbCore",1));
      }
      ((DivPlatformYM2610Ext*)dispatch)->setCSM(0);
      break;
    case DIV_SYSTEM_YM2610_CSM:
      dispatch=new DivPlatformYM2610Ext;
      if (isRender) {
        ((DivPlatformYM2610Ext*)dispatch)->setCombo(eng->getConfInt("opnbCoreRender",1));
      } else {
        ((DivPlatformYM2610Ext*)dispatch)->setCombo(eng->getConfInt("opnbCore",1));
      }
      ((DivPlatformYM2610Ext*)dispatch)->setCSM(1);
      break;
    case DIV_SYSTEM_YM2610B:
      dispatch=new DivPlatformYM2610B;
      if (isRender) {
        ((DivPlatformYM2610B*)dispatch)->setCombo(eng->getConfInt("opnbCoreRender",1));
      } else {
        ((DivPlatformYM2610B*)dispatch)->setCombo(eng->getConfInt("opnbCore",1));
      }
      break;
    case DIV_SYSTEM_YM2610B_EXT:
      dispatch=new DivPlatformYM2610BExt;
      if (isRender) {
        ((DivPlatformYM2610BExt*)dispatch)->setCombo(eng->getConfInt("opnbCoreRender",1));
      } else {
        ((DivPlatformYM2610BExt*)dispatch)->setCombo(eng->getConfInt("opnbCore",1));
      }
      ((DivPlatformYM2610BExt*)dispatch)->setCSM(0);
      break;
    case DIV_SYSTEM_YM2610B_CSM:
      dispatch=new DivPlatformYM2610BExt;
      if (isRender) {
        ((DivPlatformYM2610BExt*)dispatch)->setCombo(eng->getConfInt("opnbCoreRender",1));
      } else {
        ((DivPlatformYM2610BExt*)dispatch)->setCombo(eng->getConfInt("opnbCore",1));
      }
      ((DivPlatformYM2610BExt*)dispatch)->setCSM(1);
      break;
    case DIV_SYSTEM_AMIGA:
      dispatch=new DivPlatformAmiga;
      break;
    case DIV_SYSTEM_AY8910:
      dispatch=new DivPlatformAY8910;
      if (isRender) {
        ((DivPlatformAY8910*)dispatch)->setCore(eng->getConfInt("ayCoreRender",0)==1);
      } else {
        ((DivPlatformAY8910*)dispatch)->setCore(eng->getConfInt("ayCore",0)==1);
      }
      break;
    case DIV_SYSTEM_AY8930:
      dispatch=new DivPlatformAY8930;
      break;
    case DIV_SYSTEM_FDS:
      dispatch=new DivPlatformFDS;
      if (isRender) {
        ((DivPlatformFDS*)dispatch)->setNSFPlay(eng->getConfInt("fdsCoreRender",1)==1);
      } else {
        ((DivPlatformFDS*)dispatch)->setNSFPlay(eng->getConfInt("fdsCore",0)==1);
      }
      break;
    case DIV_SYSTEM_TIA:
      dispatch=new DivPlatformTIA;
      break;
    case DIV_SYSTEM_YM2203:
      dispatch=new DivPlatformYM2203;
      if (isRender) {
        ((DivPlatformYM2203*)dispatch)->setCombo(eng->getConfInt("opn1CoreRender",1));
      } else {
        ((DivPlatformYM2203*)dispatch)->setCombo(eng->getConfInt("opn1Core",1));
      }
      break;
    case DIV_SYSTEM_YM2203_EXT:
      dispatch=new DivPlatformYM2203Ext;
      if (isRender) {
        ((DivPlatformYM2203Ext*)dispatch)->setCombo(eng->getConfInt("opn1CoreRender",1));
      } else {
        ((DivPlatformYM2203Ext*)dispatch)->setCombo(eng->getConfInt("opn1Core",1));
      }
      ((DivPlatformYM2203Ext*)dispatch)->setCSM(0);
      break;
    case DIV_SYSTEM_YM2203_CSM:
      dispatch=new DivPlatformYM2203Ext;
      if (isRender) {
        ((DivPlatformYM2203Ext*)dispatch)->setCombo(eng->getConfInt("opn1CoreRender",1));
      } else {
        ((DivPlatformYM2203Ext*)dispatch)->setCombo(eng->getConfInt("opn1Core",1));
      }
      ((DivPlatformYM2203Ext*)dispatch)->setCSM(1);
      break;
    case DIV_SYSTEM_YM2608:
      dispatch=new DivPlatformYM2608;
      if (isRender) {
        ((DivPlatformYM2608*)dispatch)->setCombo(eng->getConfInt("opnaCoreRender",1));
      } else {
        ((DivPlatformYM2608*)dispatch)->setCombo(eng->getConfInt("opnaCore",1));
      }
      break;
    case DIV_SYSTEM_YM2608_EXT:
      dispatch=new DivPlatformYM2608Ext;
      if (isRender) {
        ((DivPlatformYM2608Ext*)dispatch)->setCombo(eng->getConfInt("opnaCoreRender",1));
      } else {
        ((DivPlatformYM2608Ext*)dispatch)->setCombo(eng->getConfInt("opnaCore",1));
      }
      ((DivPlatformYM2608Ext*)dispatch)->setCSM(0);
      break;
    case DIV_SYSTEM_YM2608_CSM:
      dispatch=new DivPlatformYM2608Ext;
      if (isRender) {
        ((DivPlatformYM2608Ext*)dispatch)->setCombo(eng->getConfInt("opnaCoreRender",1));
      } else {
        ((DivPlatformYM2608Ext*)dispatch)->setCombo(eng->getConfInt("opnaCore",1));
      }
      ((DivPlatformYM2608Ext*)dispatch)->setCSM(1);
      break;
    case DIV_SYSTEM_OPLL:
    case DIV_SYSTEM_OPLL_DRUMS:
    case DIV_SYSTEM_VRC7:
      dispatch=new DivPlatformOPLL;
      if (isRender) {
        ((DivPlatformOPLL*)dispatch)->setCore(eng->getConfInt("opllCoreRender",0));
      } else {
        ((DivPlatformOPLL*)dispatch)->setCore(eng->getConfInt("opllCore",0));
      }
      ((DivPlatformOPLL*)dispatch)->setVRC7(sys==DIV_SYSTEM_VRC7);
      ((DivPlatformOPLL*)dispatch)->setProperDrums(sys==DIV_SYSTEM_OPLL_DRUMS);
      break;
    case DIV_SYSTEM_OPL:
      dispatch=new DivPlatformOPL;
      ((DivPlatformOPL*)dispatch)->setOPLType(1,false);
      if (isRender) {
        ((DivPlatformOPL*)dispatch)->setCore(eng->getConfInt("opl2CoreRender",0));
      } else {
        ((DivPlatformOPL*)dispatch)->setCore(eng->getConfInt("opl2Core",0));
      }
      break;
    case DIV_SYSTEM_OPL_DRUMS:
      dispatch=new DivPlatformOPL;
      ((DivPlatformOPL*)dispatch)->setOPLType(1,true);
      if (isRender) {
        ((DivPlatformOPL*)dispatch)->setCore(eng->getConfInt("opl2CoreRender",0));
      } else {
        ((DivPlatformOPL*)dispatch)->setCore(eng->getConfInt("opl2Core",0));
      }
      break;
    case DIV_SYSTEM_OPL2:
      dispatch=new DivPlatformOPL;
      ((DivPlatformOPL*)dispatch)->setOPLType(2,false);
      if (isRender) {
        ((DivPlatformOPL*)dispatch)->setCore(eng->getConfInt("opl2CoreRender",0));
      } else {
        ((DivPlatformOPL*)dispatch)->setCore(eng->getConfInt("opl2Core",0));
      }
      break;
    case DIV_SYSTEM_OPL2_DRUMS:
      dispatch=new DivPlatformOPL;
      ((DivPlatformOPL*)dispatch)->setOPLType(2,true);
      if (isRender) {
        ((DivPlatformOPL*)dispatch)->setCore(eng->getConfInt("opl2CoreRender",0));
      } else {
        ((DivPlatformOPL*)dispatch)->setCore(eng->getConfInt("opl2Core",0));
      }
      break;
    case DIV_SYSTEM_OPL3:
      dispatch=new DivPlatformOPL;
      ((DivPlatformOPL*)dispatch)->setOPLType(3,false);
      if (isRender) {
        ((DivPlatformOPL*)dispatch)->setCore(eng->getConfInt("opl3CoreRender",0));
      } else {
        ((DivPlatformOPL*)dispatch)->setCore(eng->getConfInt("opl3Core",0));
      }
      break;
    case DIV_SYSTEM_OPL3_DRUMS:
      dispatch=new DivPlatformOPL;
      ((DivPlatformOPL*)dispatch)->setOPLType(3,true);
      if (isRender) {
        ((DivPlatformOPL*)dispatch)->setCore(eng->getConfInt("opl3CoreRender",0));
      } else {
        ((DivPlatformOPL*)dispatch)->setCore(eng->getConfInt("opl3Core",0));
      }
      break;
    case DIV_SYSTEM_Y8950:
      dispatch=new DivPlatformOPL;
      ((DivPlatformOPL*)dispatch)->setOPLType(8950,false);
      if (isRender) {
        ((DivPlatformOPL*)dispatch)->setCore(eng->getConfInt("opl2CoreRender",0));
      } else {
        ((DivPlatformOPL*)dispatch)->setCore(eng->getConfInt("opl2Core",0));
      }
      break;
    case DIV_SYSTEM_Y8950_DRUMS:
      dispatch=new DivPlatformOPL;
      ((DivPlatformOPL*)dispatch)->setOPLType(8950,true);
      if (isRender) {
        ((DivPlatformOPL*)dispatch)->setCore(eng->getConfInt("opl2CoreRender",0));
      } else {
        ((DivPlatformOPL*)dispatch)->setCore(eng->getConfInt("opl2Core",0));
      }
      break;
    case DIV_SYSTEM_OPZ:
      dispatch=new DivPlatformTX81Z;
      break;
    case DIV_SYSTEM_SAA1099: {
      dispatch=new DivPlatformSAA1099;
      if (isRender) {
        ((DivPlatformSAA1099*)dispatch)->setCoreQuality(eng->getConfInt("saaQualityRender",3));
      } else {
        ((DivPlatformSAA1099*)dispatch)->setCoreQuality(eng->getConfInt("saaQuality",3));
      }
      break;
    }
    case DIV_SYSTEM_PCSPKR:
      dispatch=new DivPlatformPCSpeaker;
      break;
    case DIV_SYSTEM_POKEMINI:
      dispatch=new DivPlatformPokeMini;
      break;
    case DIV_SYSTEM_SFX_BEEPER:
      dispatch=new DivPlatformZXBeeper;
      break;
    case DIV_SYSTEM_SFX_BEEPER_QUADTONE:
      dispatch=new DivPlatformZXBeeperQuadTone;
      break;
    case DIV_SYSTEM_LYNX:
      dispatch=new DivPlatformLynx;
      break;
    case DIV_SYSTEM_POKEY:
      dispatch=new DivPlatformPOKEY;
      if (isRender) {
        ((DivPlatformPOKEY*)dispatch)->setAltASAP(eng->getConfInt("pokeyCoreRender",1)==1);
      } else {
        ((DivPlatformPOKEY*)dispatch)->setAltASAP(eng->getConfInt("pokeyCore",1)==1);
      }
      break;
    case DIV_SYSTEM_QSOUND:
      dispatch=new DivPlatformQSound;
      break;
    case DIV_SYSTEM_SEGAPCM:
    case DIV_SYSTEM_SEGAPCM_COMPAT:
      dispatch=new DivPlatformSegaPCM;
      break;
    case DIV_SYSTEM_X1_010:
      dispatch=new DivPlatformX1_010;
      break;
    case DIV_SYSTEM_SWAN:
      dispatch=new DivPlatformSwan;
      break;
    case DIV_SYSTEM_T6W28:
      dispatch=new DivPlatformT6W28;
      break;
    case DIV_SYSTEM_VBOY:
      dispatch=new DivPlatformVB;
      break;
    case DIV_SYSTEM_VERA:
      dispatch=new DivPlatformVERA;
      break;
    case DIV_SYSTEM_BUBSYS_WSG:
      dispatch=new DivPlatformBubSysWSG;
      break;
    case DIV_SYSTEM_N163:
      dispatch=new DivPlatformN163;
      break;
    case DIV_SYSTEM_PET:
      dispatch=new DivPlatformPET;
      break;
    case DIV_SYSTEM_VIC20:
      dispatch=new DivPlatformVIC20;
      break;
    case DIV_SYSTEM_PONG:
      dispatch=new DivPlatformPong;
      break;
    case DIV_SYSTEM_VRC6:
      dispatch=new DivPlatformVRC6;
      break;
    case DIV_SYSTEM_MMC5:
      dispatch=new DivPlatformMMC5;
      break;
    case DIV_SYSTEM_ES5506:
      dispatch=new DivPlatformES5506;
      break;
    case DIV_SYSTEM_SCC:
      dispatch=new DivPlatformSCC;
      ((DivPlatformSCC*)dispatch)->setChipModel(false);
      break;
    case DIV_SYSTEM_SCC_PLUS:
      dispatch=new DivPlatformSCC;
      ((DivPlatformSCC*)dispatch)->setChipModel(true);
      break;
    case DIV_SYSTEM_YMZ280B:
      dispatch=new DivPlatformYMZ280B;
      ((DivPlatformYMZ280B*)dispatch)->setChipModel(280);
      break;
    case DIV_SYSTEM_RF5C68:
      dispatch=new DivPlatformRF5C68;
      break;
    case DIV_SYSTEM_SOUND_UNIT:
      dispatch=new DivPlatformSoundUnit;
      break;
    case DIV_SYSTEM_MSM6258:
      dispatch=new DivPlatformMSM6258;
      break;
    case DIV_SYSTEM_MSM6295:
      dispatch=new DivPlatformMSM6295;
      break;
    case DIV_SYSTEM_MSM5232:
      dispatch=new DivPlatformMSM5232;
      break;
    case DIV_SYSTEM_NAMCO:
      dispatch=new DivPlatformNamcoWSG;
      // Pac-Man
      ((DivPlatformNamcoWSG*)dispatch)->setDeviceType(1);
      break;
    case DIV_SYSTEM_NAMCO_15XX:
      dispatch=new DivPlatformNamcoWSG;
      ((DivPlatformNamcoWSG*)dispatch)->setDeviceType(15);
      break;
    case DIV_SYSTEM_NAMCO_CUS30:
      dispatch=new DivPlatformNamcoWSG;
      ((DivPlatformNamcoWSG*)dispatch)->setDeviceType(30);
      break;
    case DIV_SYSTEM_SNES:
      dispatch=new DivPlatformSNES;
      break;
    case DIV_SYSTEM_K007232:
      dispatch=new DivPlatformK007232;
      break;
    case DIV_SYSTEM_GA20:
      dispatch=new DivPlatformGA20;
      break;
    case DIV_SYSTEM_SUPERVISION:
      dispatch=new DivPlatformSupervision;
      break;
    case DIV_SYSTEM_UPD1771C:
      dispatch=new DivPlatformSCVWave;
      break;
    case DIV_SYSTEM_UPD1771C_TONE:
      dispatch=new DivPlatformSCVTone;
      break;
    case DIV_SYSTEM_SM8521:
      dispatch=new DivPlatformSM8521;
      break;
    case DIV_SYSTEM_PV1000:
      dispatch=new DivPlatformPV1000;
      break;
    case DIV_SYSTEM_K053260:
      dispatch=new DivPlatformK053260;
      break;
    case DIV_SYSTEM_TED:
      dispatch=new DivPlatformTED;
      break;
    case DIV_SYSTEM_C140:
      dispatch=new DivPlatformC140;
      ((DivPlatformC140*)dispatch)->set219(false);
      break;
    case DIV_SYSTEM_C219:
      dispatch=new DivPlatformC140;
      ((DivPlatformC140*)dispatch)->set219(true);
      break;
    case DIV_SYSTEM_GBA_DMA:
      dispatch=new DivPlatformGBADMA;
      break;
    case DIV_SYSTEM_GBA_MINMOD:
      dispatch=new DivPlatformGBAMinMod;
      break;
    case DIV_SYSTEM_BIFURCATOR:
      dispatch=new DivPlatformBifurcator;
      break;
    case DIV_SYSTEM_PCM_DAC:
      dispatch=new DivPlatformPCMDAC;
      break;
    case DIV_SYSTEM_ESFM:
      dispatch=new DivPlatformESFM;
      if (isRender) {
        ((DivPlatformESFM*)dispatch)->setFast(eng->getConfInt("esfmCoreRender",0));
      } else {
        ((DivPlatformESFM*)dispatch)->setFast(eng->getConfInt("esfmCore",0));
      }
      break;
    case DIV_SYSTEM_POWERNOISE:
      dispatch=new DivPlatformPowerNoise;
      if (isRender) {
        ((DivPlatformPowerNoise*)dispatch)->setCoreQuality(eng->getConfInt("pnQualityRender",3));
      } else {
        ((DivPlatformPowerNoise*)dispatch)->setCoreQuality(eng->getConfInt("pnQuality",3));
      }
      break;
    case DIV_SYSTEM_DAVE:
      dispatch=new DivPlatformDave;
      break;
    case DIV_SYSTEM_NDS:
      dispatch=new DivPlatformNDS;
      break;
    case DIV_SYSTEM_5E01:
      dispatch=new DivPlatformNES;
      if (isRender) {
        ((DivPlatformNES*)dispatch)->setNSFPlay(eng->getConfInt("nesCoreRender",0)==1);
      } else {
        ((DivPlatformNES*)dispatch)->setNSFPlay(eng->getConfInt("nesCore",0)==1);
      }
      ((DivPlatformNES*)dispatch)->set5E01(true);
      break;
    case DIV_SYSTEM_SID2:
      dispatch=new DivPlatformSID2;
      break;
    case DIV_SYSTEM_SID3:
      dispatch=new DivPlatformSID3;
      break;
    case DIV_SYSTEM_OPL4:
      dispatch=new DivPlatformOPL;
      ((DivPlatformOPL*)dispatch)->setOPLType(4,false);
      if (isRender) {
        ((DivPlatformOPL*)dispatch)->setCore(eng->getConfInt("opl4CoreRender",0));
      } else {
        ((DivPlatformOPL*)dispatch)->setCore(eng->getConfInt("opl4Core",0));
      }
      break;
    case DIV_SYSTEM_OPL4_DRUMS:
      dispatch=new DivPlatformOPL;
      ((DivPlatformOPL*)dispatch)->setOPLType(4,true);
      if (isRender) {
        ((DivPlatformOPL*)dispatch)->setCore(eng->getConfInt("opl4CoreRender",0));
      } else {
        ((DivPlatformOPL*)dispatch)->setCore(eng->getConfInt("opl4Core",0));
      }
      break;
    case DIV_SYSTEM_DUMMY:
      dispatch=new DivPlatformDummy;
      break;
    default:
      logW("this system is not supported yet! using dummy platform.");
      dispatch=new DivPlatformDummy;
      break;
  }
  dispatch->init(eng,chanCount,gotRate,flags);

  // initialize output buffers
  int outs=dispatch->getOutputCount();
  bbInLen=32768;

  for (int i=0; i<outs; i++) {
    bb[i]=blip_new(bbInLen);
    if (bb[i]==NULL) {
      logE("not enough memory!");
      return;
    }

    bbIn[i]=new short[bbInLen];
    bbOut[i]=new short[bbInLen];
    memset(bbIn[i],0,bbInLen*sizeof(short));
    memset(bbOut[i],0,bbInLen*sizeof(short));
    blip_set_dc(bb[i],hiPass);
  }
}

void DivDispatchContainer::quit() {
  if (dispatch==NULL) return;
  dispatch->quit();
  delete dispatch;
  dispatch=NULL;

  for (int i=0; i<DIV_MAX_OUTPUTS; i++) {
    if (bbOut[i]!=NULL) {
      delete[] bbOut[i];
      bbOut[i]=NULL;
    }
    if (bbIn[i]!=NULL) {
      delete[] bbIn[i];
      bbIn[i]=NULL;
    }
    if (bb[i]!=NULL) {
      blip_delete(bb[i]);
      bb[i]=NULL;
    }
  }
  bbInLen=0;
}
