/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2026 tildearrow and contributors
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

#define _USE_MATH_DEFINES
#include "gui.h"
#include "../../extern/opn/ym3438.h"
#include "../../extern/opm/opm.h"
#include "../../extern/opl/opl3.h"
#include "../../extern/ESFMu/esfm.h"
extern "C" {
#include "../../extern/Nuked-OPLL/opll.h"
#include "../engine/platform/sound/sgu.h"
}
#include "../engine/platform/sound/ymfm/ymfm_opz.h"
#include "../engine/bsr.h"

#define OPN_WRITE(addr,val) \
  OPN2_Write((ym3438_t*)fmPreviewOPN,0,(addr)); \
  do { \
    OPN2_Clock((ym3438_t*)fmPreviewOPN,out); \
  } while (((ym3438_t*)fmPreviewOPN)->write_busy); \
  OPN2_Write((ym3438_t*)fmPreviewOPN,1,(val)); \
  do { \
    OPN2_Clock((ym3438_t*)fmPreviewOPN,out); \
  } while (((ym3438_t*)fmPreviewOPN)->write_busy);

const unsigned char dtTableFMP[8]={
  7,6,5,0,1,2,3,4
};

void FurnaceGUI::renderFMPreviewOPN(const DivInstrumentFM& params, int pos) {
  if (fmPreviewOPN==NULL) {
    fmPreviewOPN=new ym3438_t;
    pos=0;
  }
  short out[2];
  int aOut=0;
  bool mult0=false;

  if (pos==0) {
    OPN2_Reset((ym3438_t*)fmPreviewOPN);
    OPN2_SetChipType((ym3438_t*)fmPreviewOPN,ym3438_mode_opn);

    // set params
    for (int i=0; i<4; i++) {
      if ((params.op[i].mult&15)==0) {
        mult0=true;
        break;
      }
    }
    for (int i=0; i<4; i++) {
      const DivInstrumentFM::Operator& op=params.op[i];
      unsigned short baseAddr=i*4;
      OPN_WRITE(baseAddr+0x40,op.tl);
      OPN_WRITE(baseAddr+0x30,(op.mult&15)|(dtTableFMP[op.dt&7]<<4));
      OPN_WRITE(baseAddr+0x50,(op.ar&31)|(op.rs<<6));
      OPN_WRITE(baseAddr+0x60,(op.dr&31)|(op.am<<7));
      OPN_WRITE(baseAddr+0x70,op.d2r&31);
      OPN_WRITE(baseAddr+0x80,(op.rr&15)|(op.sl<<4));
      OPN_WRITE(baseAddr+0x90,op.ssgEnv&15);
    }
    OPN_WRITE(0xb0,(params.alg&7)|((params.fb&7)<<3));
    OPN_WRITE(0xb4,0xc0|(params.fms&7)|((params.ams&3)<<4));
    OPN_WRITE(0xa4,mult0?0x1c:0x14); // frequency
    OPN_WRITE(0xa0,0);
    OPN_WRITE(
     0x28,
     (params.op[0].enable?0x10:0)|
     (params.op[2].enable?0x20:0)|
     (params.op[1].enable?0x40:0)|
     (params.op[3].enable?0x80:0)
    ); // key on
  }

  // render
  for (int i=0; i<FM_PREVIEW_SIZE; i++) {
    aOut=0;
    for (int j=0; j<24; j++) {
      OPN2_Clock((ym3438_t*)fmPreviewOPN,out);
    }
    aOut+=((ym3438_t*)fmPreviewOPN)->ch_out[0];
    if (aOut<-32768) aOut=-32768;
    if (aOut>32767) aOut=32767;
    fmPreview[i]=aOut;
  }
}

#define OPM_WRITE(addr,val) \
  OPM_Write((opm_t*)fmPreviewOPM,0,(addr)); \
  do { \
    OPM_Clock((opm_t*)fmPreviewOPM,out,NULL,NULL,NULL); \
    OPM_Clock((opm_t*)fmPreviewOPM,out,NULL,NULL,NULL); \
  } while (((opm_t*)fmPreviewOPM)->write_busy); \
  OPM_Write((opm_t*)fmPreviewOPM,1,(val)); \
  do { \
    OPM_Clock((opm_t*)fmPreviewOPM,out,NULL,NULL,NULL); \
    OPM_Clock((opm_t*)fmPreviewOPM,out,NULL,NULL,NULL); \
  } while (((opm_t*)fmPreviewOPM)->write_busy);

void FurnaceGUI::renderFMPreviewOPM(const DivInstrumentFM& params, int pos) {
  if (fmPreviewOPM==NULL) {
    fmPreviewOPM=new opm_t;
    pos=0;
  }
  int out[2];
  int aOut=0;
  bool mult0=false;

  if (pos==0) {
    OPM_Reset((opm_t*)fmPreviewOPM);

    // set params
    for (int i=0; i<4; i++) {
      if ((params.op[i].mult&15)==0) {
        mult0=true;
        break;
      }
    }
    for (int i=0; i<4; i++) {
      const DivInstrumentFM::Operator& op=params.op[i];
      unsigned short baseAddr=i*8;
      OPM_WRITE(baseAddr+0x40,(op.mult&15)|(dtTableFMP[op.dt&7]<<4));
      OPM_WRITE(baseAddr+0x60,op.tl);
      OPM_WRITE(baseAddr+0x80,(op.ar&31)|(op.rs<<6));
      OPM_WRITE(baseAddr+0xa0,(op.dr&31)|(op.am<<7));
      OPM_WRITE(baseAddr+0xc0,(op.d2r&31)|(op.dt2<<6));
      OPM_WRITE(baseAddr+0xe0,(op.rr&15)|(op.sl<<4));
    }
    OPM_WRITE(0x20,(params.alg&7)|((params.fb&7)<<3)|0xc0);
    OPM_WRITE(0x38,((params.fms&7)<<4)|(params.ams&3));
    OPM_WRITE(0x28,mult0?0x39:0x29); // frequency
    OPM_WRITE(0x30,0xe6);
    OPM_WRITE(
     0x08,
     (params.op[0].enable?0x08:0)|
     (params.op[2].enable?0x10:0)|
     (params.op[1].enable?0x20:0)|
     (params.op[3].enable?0x40:0)
    ); // key on
  }

  // render
  for (int i=0; i<FM_PREVIEW_SIZE; i++) {
    aOut=0;
    for (int j=0; j<32; j++) {
      OPM_Clock((opm_t*)fmPreviewOPM,out,NULL,NULL,NULL);
    }
    aOut+=out[0];
    if (aOut<-32768) aOut=-32768;
    if (aOut>32767) aOut=32767;
    fmPreview[i]=aOut;
  }
}

#define OPLL_WRITE(addr,val) \
  OPLL_Write((opll_t*)fmPreviewOPLL,0,(addr)); \
  for (int _i=0; _i<3; _i++) { \
    OPLL_Clock((opll_t*)fmPreviewOPLL,out); \
  } \
  OPLL_Write((opll_t*)fmPreviewOPLL,1,(val)); \
  for (int _i=0; _i<21; _i++) { \
    OPLL_Clock((opll_t*)fmPreviewOPLL,out); \
  }

void FurnaceGUI::renderFMPreviewOPLL(const DivInstrumentFM& params, int pos) {
  if (fmPreviewOPLL==NULL) {
    fmPreviewOPLL=new opm_t;
    pos=0;
  }
  int out[2];
  int aOut=0;
  bool mult0=false;

  if (pos==0) {
    OPLL_Reset((opll_t*)fmPreviewOPLL,opll_type_ym2413);

    // set params
    const DivInstrumentFM::Operator& mod=params.op[0];
    const DivInstrumentFM::Operator& car=params.op[1];
    if (params.opllPreset==0) {
      for (int i=0; i<2; i++) {
        if ((params.op[i].mult&15)==0) {
          mult0=true;
          break;
        }
      }
      OPLL_WRITE(0x00,(mod.am<<7)|(mod.vib<<6)|((mod.ssgEnv&8)<<2)|(mod.ksr<<4)|(mod.mult));
      OPLL_WRITE(0x01,(car.am<<7)|(car.vib<<6)|((car.ssgEnv&8)<<2)|(car.ksr<<4)|(car.mult));
      OPLL_WRITE(0x02,(mod.ksl<<6)|(mod.tl&63));
      OPLL_WRITE(0x03,(car.ksl<<6)|((params.fms&1)<<4)|((params.ams&1)<<3)|(params.fb&7));
      OPLL_WRITE(0x04,(mod.ar<<4)|(mod.dr));
      OPLL_WRITE(0x05,(car.ar<<4)|(car.dr));
      OPLL_WRITE(0x06,(mod.sl<<4)|(mod.rr));
      OPLL_WRITE(0x07,(car.sl<<4)|(car.rr));
    }
    OPLL_WRITE(0x10,0);
    OPLL_WRITE(0x30,(params.opllPreset<<4)|(car.tl&15));
    OPLL_WRITE(0x20,(params.alg?0x20:0)|(mult0?0x15:0x13));
  }

  // render
  for (int i=0; i<FM_PREVIEW_SIZE; i++) {
    aOut=0;
    for (int j=0; j<36; j++) {
      OPLL_Clock((opll_t*)fmPreviewOPLL,out);
      aOut+=out[0]<<4;
    }
    if (aOut<-32768) aOut=-32768;
    if (aOut>32767) aOut=32767;
    fmPreview[i]=aOut;
  }
}

#define OPL_WRITE(addr,val) \
  OPL3_WriteReg((opl3_chip*)fmPreviewOPL,(addr),(val)); \
  OPL3_Generate4Ch((opl3_chip*)fmPreviewOPL,out);

const unsigned char lPreviewSlots[4]={
  0, 3, 8, 11
};

const unsigned char lOpMap[4]={
  0, 2, 1, 3
};

void FurnaceGUI::renderFMPreviewOPL(const DivInstrumentFM& params, int pos) {
  if (fmPreviewOPL==NULL) {
    fmPreviewOPL=new opl3_chip;
    pos=0;
  }
  short out[4];
  bool mult0=false;

  if (pos==0) {
    OPL3_Reset((opl3_chip*)fmPreviewOPL,49716);

    // set params
    int ops=(params.ops==4)?4:2;
    for (int i=0; i<ops; i++) {
      if ((params.op[i].mult&15)==0) {
        mult0=true;
        break;
      }
    }

    OPL_WRITE(0x01,0x20);
    OPL_WRITE(0x105,1);
    if (ops==4) {
      OPL_WRITE(0x104,1);
    }
    for (int i=0; i<ops; i++) {
      const DivInstrumentFM::Operator& op=params.op[(ops==4)?lOpMap[i]:i];
      unsigned short baseAddr=lPreviewSlots[i];

      OPL_WRITE(baseAddr+0x40,op.tl|(op.ksl<<6));
      OPL_WRITE(baseAddr+0x20,(op.am<<7)|(op.vib<<6)|(op.sus<<5)|(op.ksr<<4)|op.mult);
      OPL_WRITE(baseAddr+0x60,(op.ar<<4)|op.dr);
      OPL_WRITE(baseAddr+0x80,(op.sl<<4)|op.rr);
      OPL_WRITE(baseAddr+0xe0,op.ws&7);
    }

    OPL_WRITE(0xc0,(params.alg&1)|(params.fb<<1)|0x10);
    if (ops==4) {
      OPL_WRITE(0xc3,((params.alg>>1)&1)|(params.fb<<1)|0x10);
    }
    OPL_WRITE(0xa0,0);
    if (ops==4) {
      OPL_WRITE(0xa3,0);
    }
    OPL_WRITE(0xb0,mult0?0x2a:0x26);
    if (ops==4) {
      OPL_WRITE(0xb3,mult0?0x2a:0x26);
    }
  }

  // render
  for (int i=0; i<FM_PREVIEW_SIZE; i++) {
    OPL3_Generate4Ch((opl3_chip*)fmPreviewOPL,out);
    OPL3_Generate4Ch((opl3_chip*)fmPreviewOPL,out);
    fmPreview[i]=CLAMP(out[0]*2,-32768,32767);
  }
}

#define OPZ_WRITE(addr,val) \
  ((ymfm::ym2414*)fmPreviewOPZ)->write(0,(addr)); \
  ((ymfm::ym2414*)fmPreviewOPZ)->write(1,(val)); \
  ((ymfm::ym2414*)fmPreviewOPZ)->generate(&out,1);

void FurnaceGUI::renderFMPreviewOPZ(const DivInstrumentFM& params, int pos) {
  if (fmPreviewOPZ==NULL) {
    fmPreviewOPZInterface=new ymfm::ymfm_interface();
    fmPreviewOPZ=new ymfm::ym2414(*(ymfm::ymfm_interface*)fmPreviewOPZInterface);
    pos=0;
  }
  ymfm::ymfm_output<2> out;
  int aOut=0;
  bool mult0=false;

  if (pos==0) {
    ((ymfm::ym2414*)fmPreviewOPZ)->reset();

    // set params
    for (int i=0; i<4; i++) {
      if ((params.op[i].mult&15)==0) {
        mult0=true;
        break;
      }
    }
    for (int i=0; i<4; i++) {
      const DivInstrumentFM::Operator& op=params.op[i];
      unsigned short baseAddr=i*8;
      OPZ_WRITE(baseAddr+0x40,(op.mult&15)|((op.egt?(op.dt&7):dtTableFMP[op.dt&7])<<4));
      OPZ_WRITE(baseAddr+0x40,(op.dvb&15)|((op.ws&7)<<4)|0x80);
      OPZ_WRITE(baseAddr+0x60,op.tl);
      OPZ_WRITE(baseAddr+0x80,(op.ar&31)|(op.egt<<5)|(op.rs<<6));
      OPZ_WRITE(baseAddr+0xa0,(op.dr&31)|(op.am<<7));
      OPZ_WRITE(baseAddr+0xc0,(op.d2r&31)|(op.dt2<<6));
      OPZ_WRITE(baseAddr+0xc0,(op.dam&7)|(op.ksl<<6)|0x20);
      OPZ_WRITE(baseAddr+0xe0,(op.rr&15)|(op.sl<<4));
    }
    OPZ_WRITE(0x38,((params.fms&7)<<4)|(params.ams&3));
    OPZ_WRITE(0x38,((params.fms2&7)<<4)|(params.ams2&3)|0x84);
    OPZ_WRITE(0x28,mult0?0x39:0x29); // frequency
    OPZ_WRITE(0x30,0xe7);
    OPZ_WRITE(0x20,(params.alg&7)|((params.fb&7)<<3)|0x40); // key on
  }

  // render
  for (int i=0; i<FM_PREVIEW_SIZE; i++) {
    aOut=0;
    ((ymfm::ym2414*)fmPreviewOPZ)->generate(&out,1);
    aOut+=out.data[0];
    if (aOut<-32768) aOut=-32768;
    if (aOut>32767) aOut=32767;
    fmPreview[i]=aOut;
  }
}

#define ESFM_WRITE(addr,val) \
  ESFM_write_reg_buffered_fast((esfm_chip*)fmPreviewESFM,(addr),(val))

void FurnaceGUI::renderFMPreviewESFM(const DivInstrumentFM& params, const DivInstrumentESFM& esfmParams, int pos) {
  if (fmPreviewESFM==NULL) {
    fmPreviewESFM=new esfm_chip;
    pos=0;
  }
  short out[4];
  bool mult0=false;

  if (pos==0) {
    ESFM_init((esfm_chip*)fmPreviewESFM,0);
    // set native mode
    ESFM_WRITE(0x105, 0x80);

    // set params
    for (int i=0; i<4; i++) {
      if ((params.op[i].mult&15)==0) {
        mult0=true;
        break;
      }
    }

    for (int i=0; i<4; i++) {
      const DivInstrumentFM::Operator& op=params.op[i];
      const DivInstrumentESFM::Operator& opE=esfmParams.op[i];
      unsigned short baseAddr=i*8;
      unsigned char freqL, freqH;
      if (opE.fixed) {
        freqL=opE.dt;
        freqH=opE.ct&0x1f;
      } else {
        // perform detune calculation
        int offset=(opE.ct<<7)+opE.dt;
        double fbase=(mult0?2048.0:1024.0)*pow(2.0,(float)offset/(128.0*12.0));
        int bf=round(fbase);
        int block=0;
        if (bf>0x3ff) {
          block=bsr32(bf)-10;
          bf>>=block;
        }
        freqL=bf&0xff;
        freqH=((block&7)<<2)|((bf>>8)&3);
      }

      ESFM_WRITE(baseAddr+0,(op.am<<7)|((op.vib&1)<<6)|((op.sus&1)<<5)|((op.ksr&1)<<4)|(op.mult&0x0f));
      ESFM_WRITE(baseAddr+1,(op.ksl<<6)|(op.tl&0x3f));
      ESFM_WRITE(baseAddr+2,(op.ar<<4)|(op.dr&0x0f));
      ESFM_WRITE(baseAddr+3,(op.sl<<4)|(op.rr&0x0f));

      ESFM_WRITE(baseAddr+4,freqL);
      ESFM_WRITE(baseAddr+5,(opE.delay<<5)|freqH);

      ESFM_WRITE(baseAddr+6,(op.dam<<7)|((op.dvb&1)<<6)|((opE.right&1)<<5)|((opE.left&1)<<4)|((opE.modIn&7)<<1));
      ESFM_WRITE(baseAddr+7,(opE.outLvl<<5)|((i==3?esfmParams.noise:0)<<3)|(op.ws&7));
    }
  }

  // note on
  ESFM_WRITE(0x240, 1);

  // render
  for (int i=0; i<FM_PREVIEW_SIZE; i++) {
    ESFM_generate((esfm_chip*)fmPreviewESFM,out);
    ESFM_generate((esfm_chip*)fmPreviewESFM,out);
    fmPreview[i]=CLAMP(out[0]+out[1],-32768,32767);
  }
}

#define SGU_WRITE(addr,val) \
  SGU_Write((SGU*)fmPreviewSGU,(addr),(val))

void FurnaceGUI::renderFMPreviewSGU(const DivInstrumentFM& params, const DivInstrumentESFM& esfmParams, const DivInstrumentSGU& sguParams, int sampleIdx, int pos) {
  if (fmPreviewSGU==NULL) {
    fmPreviewSGU=new SGU;
    SGU_Init((SGU*)fmPreviewSGU,SGU_PCM_RAM_SIZE);
    pos=0;
  }
  int32_t outL=0;
  int32_t outR=0;
  bool mult0=false;

  if (pos==0) {
    SGU* sgu=(SGU*)fmPreviewSGU;
    SGU_Reset(sgu);

    for (int i=0; i<SGU_OP_PER_CH; i++) {
      if ((params.op[i].mult&15)==0) {
        mult0=true;
        break;
      }
    }

    const unsigned short chBase=0;
    const unsigned short opBase=chBase;
    const unsigned short chRegBase=chBase+(SGU_OP_PER_CH*SGU_OP_REGS);

    bool hasSampleWave=false;
    for (int i=0; i<SGU_OP_PER_CH; i++) {
      const DivInstrumentFM::Operator& op=params.op[i];
      const DivInstrumentESFM::Operator& opE=esfmParams.op[i];
      const DivInstrumentSGU::Operator& opS=sguParams.op[i];
      const unsigned short baseAddr=opBase+(i*SGU_OP_REGS);

      if ((op.ws&7)==SGU_WAVE_SAMPLE) hasSampleWave=true;

      const unsigned char am=op.am&0x01;
      const unsigned char fm=op.vib&0x01;
      const unsigned char fix=opE.fixed&0x01;
      const unsigned char mult=op.mult&0x0f;
      const unsigned char ksl=op.ksl&0x03;
      const unsigned char tl=op.tl&0x7f;
      const unsigned char ar=op.ar&0x1f;
      const unsigned char dr=op.dr&0x1f;
      const unsigned char sl=op.sl&0x0f;
      const unsigned char rr=op.rr&0x0f;
      const unsigned char dt=op.dt&0x07;
      const unsigned char sr=op.d2r&0x1f;
      const unsigned char delay=opE.delay&0x07;
      const unsigned char ksr=op.ksr&0x03;
      const unsigned char wpar=opS.wpar&0x0f;
      const unsigned char dam=op.dam&0x01;
      const unsigned char dvb=op.dvb&0x01;
      const unsigned char sync=opS.sync?1:0;
      const unsigned char ring=opS.ring?1:0;
      const unsigned char modIn=opE.modIn&0x07;
      const unsigned char outLvl=opE.outLvl&0x07;
      const unsigned char wave=op.ws&0x07;

      const unsigned char reg0=(am?SGU_OP0_TRM_BIT:0)
        | (fm?SGU_OP0_VIB_BIT:0)
        | ((ksr<<SGU_OP0_KSR_SHIFT)&SGU_OP0_KSR_MASK)
        | (mult&SGU_OP0_MUL_MASK);
      const unsigned char reg1=((ksl<<SGU_OP1_KSL_SHIFT)&SGU_OP1_KSL_MASK)
        | (tl&SGU_OP1_TL_MASK);
      const unsigned char reg2=((ar<<SGU_OP2_AR_SHIFT)&SGU_OP2_AR_MASK)
        | (dr&SGU_OP2_DR_MASK);
      const unsigned char reg3=((sl<<SGU_OP3_SL_SHIFT)&SGU_OP3_SL_MASK)
        | (rr&SGU_OP3_RR_MASK);
      const unsigned char reg4=((dt<<SGU_OP4_DT_SHIFT)&SGU_OP4_DT_MASK)
        | (sr&SGU_OP4_SR_MASK);
      const unsigned char reg5=((delay<<SGU_OP5_DELAY_SHIFT)&SGU_OP5_DELAY_MASK)
        | (fix?SGU_OP5_FIX_BIT:0)
        | (wpar&SGU_OP5_WPAR_MASK);
      const unsigned char reg6=(dam?SGU_OP6_TRMD_BIT:0)
        | (dvb?SGU_OP6_VIBD_BIT:0)
        | (sync?SGU_OP6_SYNC_BIT:0)
        | (ring?SGU_OP6_RING_BIT:0)
        | ((modIn<<SGU_OP6_MOD_SHIFT)&SGU_OP6_MOD_MASK)
        | ((tl>>SGU_OP1_TL_OFFSET)&SGU_OP6_TL_MSB_BIT);
      const unsigned char reg7=((((op.enable?outLvl:0)<<SGU_OP7_OUT_SHIFT)&SGU_OP7_OUT_MASK))
        | ((ar>>SGU_OP2_AR_OFFSET)?SGU_OP7_AR_MSB_BIT:0)
        | ((dr>>SGU_OP2_DR_OFFSET)?SGU_OP7_DR_MSB_BIT:0)
        | (wave&SGU_OP7_WAVE_MASK);

      SGU_WRITE(baseAddr+0,reg0);
      SGU_WRITE(baseAddr+1,reg1);
      SGU_WRITE(baseAddr+2,reg2);
      SGU_WRITE(baseAddr+3,reg3);
      SGU_WRITE(baseAddr+4,reg4);
      SGU_WRITE(baseAddr+5,reg5);
      SGU_WRITE(baseAddr+6,reg6);
      SGU_WRITE(baseAddr+7,reg7);
    }

    if (hasSampleWave) {
      memset(sgu->pcm,0,SGU_PCM_RAM_SIZE);
      if (sampleIdx>=0 && sampleIdx<e->song.sampleLen) {
        DivSample* s=e->song.sample[sampleIdx];
        if (s!=NULL && s->data8!=NULL && s->length8>0) {
          size_t copyLen=s->length8;
          if (copyLen>(size_t)SGU_PCM_RAM_SIZE) {
            copyLen=SGU_PCM_RAM_SIZE;
          }
          memcpy(sgu->pcm,s->data8,copyLen);
        }
      }
      SGU_WRITE(chRegBase+SGU1_CHN_PCM_RST_L,0);
      SGU_WRITE(chRegBase+SGU1_CHN_PCM_RST_H,0);
    }

    const unsigned short freq=mult0?0x39ac:0x1cd6;
    SGU_WRITE(chRegBase+SGU1_CHN_FREQ_L,freq&0xff);
    SGU_WRITE(chRegBase+SGU1_CHN_FREQ_H,freq>>8);
    SGU_WRITE(chRegBase+SGU1_CHN_VOL,127);
    SGU_WRITE(chRegBase+SGU1_CHN_PAN,0);
    SGU_WRITE(chRegBase+SGU1_CHN_DUTY,63);
    SGU_WRITE(chRegBase+SGU1_CHN_FLAGS1,0);
    SGU_WRITE(chRegBase+SGU1_CHN_FLAGS0,SGU1_FLAGS0_CTL_GATE);
  }

  for (int i=0; i<FM_PREVIEW_SIZE; i++) {
    SGU_NextSample((SGU*)fmPreviewSGU,&outL,&outR);
    SGU_NextSample((SGU*)fmPreviewSGU,&outL,&outR);
    fmPreview[i]=CLAMP(SGU_GetSample((SGU*)fmPreviewSGU,0),-32768,32767);
  }
}

void FurnaceGUI::renderFMPreview(const DivInstrument* ins, int pos) {
  switch (ins->type) {
    case DIV_INS_FM:
      renderFMPreviewOPN(ins->fm,pos);
      break;
    case DIV_INS_OPM:
      renderFMPreviewOPM(ins->fm,pos);
      break;
    case DIV_INS_OPLL:
      renderFMPreviewOPLL(ins->fm,pos);
      break;
    case DIV_INS_OPL:
      renderFMPreviewOPL(ins->fm,pos);
      break;
    case DIV_INS_OPZ:
      renderFMPreviewOPZ(ins->fm,pos);
      break;
    case DIV_INS_ESFM:
      renderFMPreviewESFM(ins->fm,ins->esfm,pos);
      break;
    case DIV_INS_SGU:
      renderFMPreviewSGU(ins->fm,ins->esfm,ins->sgu,ins->amiga.initSample,pos);
      break;
    default:
      break;
  }
}
