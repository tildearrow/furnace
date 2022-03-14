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

#include "sample.h"
#include "../ta-log.h"
#include <string.h>
#include <sndfile.h>
#include <math.h>

extern "C" {
#include "../../extern/adpcm/bs_codec.h"
#include "../../extern/adpcm/oki_codec.h"
#include "../../extern/adpcm/yma_codec.h"
#include "../../extern/adpcm/ymb_codec.h"
#include "../../extern/adpcm/ymz_codec.h"
}

bool DivSample::save(const char* path) {
  SNDFILE* f;
  SF_INFO si;
  memset(&si,0,sizeof(SF_INFO));

  if (length16<1) return false;

  si.channels=1;
  si.samplerate=rate;
  si.format=SF_FORMAT_PCM_16|SF_FORMAT_WAV;

  f=sf_open(path,SFM_WRITE,&si);

  if (f==NULL) {
    logE("could not open wave file for saving! %s\n",sf_error_number(sf_error(f)));
    return false;
  }

  SF_INSTRUMENT inst;
  memset(&inst, 0, sizeof(inst));
  inst.gain = 1;
  short pitch = (0x3c * 100) + 50 - (log2((double)centerRate/rate) * 12.0 * 100.0);
  inst.basenote = pitch / 100;
  inst.detune = 50 - (pitch % 100);
  inst.velocity_hi = 0x7f;
  inst.key_hi = 0x7f;
  if(loopStart != -1)
  {
    inst.loop_count = 1;
    inst.loops[0].mode = SF_LOOP_FORWARD;
    inst.loops[0].start = loopStart;
    inst.loops[0].end = samples;
  }
  sf_command(f, SFC_SET_INSTRUMENT, &inst, sizeof(inst));

  sf_write_short(f,data16,length16);

  sf_close(f);

  return true;
}

// 16-bit memory is padded to 512, to make things easier for ADPCM-A/B.
bool DivSample::initInternal(unsigned char d, int count) {
  switch (d) {
    case 0: // 1-bit
      if (data1!=NULL) delete[] data1;
      length1=(count+7)/8;
      data1=new unsigned char[length1];
      memset(data1,0,length1);
      break;
    case 1: // DPCM
      if (dataDPCM!=NULL) delete[] dataDPCM;
      lengthDPCM=(count+7)/8;
      dataDPCM=new unsigned char[lengthDPCM];
      memset(dataDPCM,0,lengthDPCM);
      break;
    case 4: // QSound ADPCM
      if (dataQSoundA!=NULL) delete[] dataQSoundA;
      lengthQSoundA=(count+1)/2;
      dataQSoundA=new unsigned char[lengthQSoundA];
      memset(dataQSoundA,0,lengthQSoundA);
      break;
    case 5: // ADPCM-A
      if (dataA!=NULL) delete[] dataA;
      lengthA=(count+1)/2;
      dataA=new unsigned char[(lengthA+255)&(~0xff)];
      memset(dataA,0,(lengthA+255)&(~0xff));
      break;
    case 6: // ADPCM-B
      if (dataB!=NULL) delete[] dataB;
      lengthB=(count+1)/2;
      dataB=new unsigned char[(lengthB+255)&(~0xff)];
      memset(dataB,0,(lengthB+255)&(~0xff));
      break;
    case 7: // X68000 ADPCM
      if (dataX68!=NULL) delete[] dataX68;
      lengthX68=(count+1)/2;
      dataX68=new unsigned char[lengthX68];
      memset(dataX68,0,lengthX68);
      break;
    case 8: // 8-bit
      if (data8!=NULL) delete[] data8;
      length8=count;
      // for padding X1-010 sample
      data8=new signed char[(count+4095)&(~0xfff)];
      memset(data8,0,(count+4095)&(~0xfff));
      break;
    case 9: // BRR
      if (dataBRR!=NULL) delete[] dataBRR;
      lengthBRR=9*((count+15)/16);
      dataBRR=new unsigned char[lengthBRR];
      memset(dataBRR,0,lengthBRR);
      break;
    case 10: // VOX
      if (dataVOX!=NULL) delete[] dataVOX;
      lengthVOX=(count+1)/2;
      dataVOX=new unsigned char[lengthVOX];
      memset(dataVOX,0,lengthVOX);
      break;
    case 16: // 16-bit
      if (data16!=NULL) delete[] data16;
      length16=count*2;
      data16=new short[(count+511)&(~0x1ff)];
      memset(data16,0,((count+511)&(~0x1ff))*sizeof(short));
      break;
    default:
      return false;
  }
  return true;
}

bool DivSample::init(unsigned int count) {
  if (!initInternal(depth,count)) return false;
  samples=count;
  return true;
}

void DivSample::render() {
  // step 1: convert to 16-bit if needed
  if (depth!=16) {
    if (!initInternal(16,samples)) return;
    switch (depth) {
      case 0: // 1-bit
        for (unsigned int i=0; i<samples; i++) {
          data16[i]=((data1[i>>3]>>(i&7))&1)?0x7fff:-0x7fff;
        }
        break;
      case 1: { // DPCM
        int accum=0;
        for (unsigned int i=0; i<samples; i++) {
          accum+=((data1[i>>3]>>(i&7))&1)?1:-1;
          if (accum>63) accum=63;
          if (accum<-64) accum=-64;
          data16[i]=accum*512;
        }
        break;
      }
      case 4: // QSound ADPCM
        bs_decode(dataQSoundA,data16,samples);
        break;
      case 5: // ADPCM-A
        yma_decode(dataA,data16,samples);
        break;
      case 6: // ADPCM-B
        ymb_decode(dataB,data16,samples);
        break;
      case 7: // X6800 ADPCM
        oki6258_decode(dataX68,data16,samples);
        break;
      case 8: // 8-bit PCM
        for (unsigned int i=0; i<samples; i++) {
          data16[i]=data8[i]<<8;
        }
        break;
      case 9: // BRR
        // TODO!
        break;
      case 10: // VOX
        oki_decode(dataVOX,data16,samples);
        break;
      default:
        return;
    }
  }

  // step 2: render to other formats
  if (depth!=0) { // 1-bit
    if (!initInternal(0,samples)) return;
    for (unsigned int i=0; i<samples; i++) {
      if (data16[i]>0) {
        data1[i>>3]|=1<<(i&7);
      }
    }
  }
  if (depth!=1) { // DPCM
    if (!initInternal(1,samples)) return;
    int accum=63;
    for (unsigned int i=0; i<samples; i++) {
      int next=((unsigned short)(data16[i]^0x8000))>>9;
      if (next>accum) {
        dataDPCM[i>>3]|=1<<(i&7);
        accum++;
      } else {
        accum--;
      }
      if (accum<0) accum=0;
      if (accum>127) accum=127;
    }
  }
  if (depth!=4) { // QSound ADPCM
    if (!initInternal(4,samples)) return;
    bs_encode(data16,dataQSoundA,samples);
  }
  // TODO: pad to 256.
  if (depth!=5) { // ADPCM-A
    if (!initInternal(5,samples)) return;
    yma_encode(data16,dataA,(samples+511)&(~0x1ff));
  }
  if (depth!=6) { // ADPCM-B
    if (!initInternal(6,samples)) return;
    ymb_encode(data16,dataB,(samples+511)&(~0x1ff));
  }
  if (depth!=7) { // X68000 ADPCM
    if (!initInternal(7,samples)) return;
    oki6258_encode(data16,dataX68,samples);
  }
  if (depth!=8) { // 8-bit PCM
    if (!initInternal(8,samples)) return;
    for (unsigned int i=0; i<samples; i++) {
      data8[i]=data16[i]>>8;
    }
  }
  // TODO: BRR!
  if (depth!=10) { // VOX
    if (!initInternal(10,samples)) return;
    oki_encode(data16,dataVOX,samples);
  }
}

void* DivSample::getCurBuf() {
  switch (depth) {
    case 0:
      return data1;
    case 1:
      return dataDPCM;
    case 4:
      return dataQSoundA;
    case 5:
      return dataA;
    case 6:
      return dataB;
    case 7:
      return dataX68;
    case 8:
      return data8;
    case 9:
      return dataBRR;
    case 10:
      return dataVOX;
    case 16:
      return data16;
  }
  return NULL;
}

unsigned int DivSample::getCurBufLen() {
  switch (depth) {
    case 0:
      return length1;
    case 1:
      return lengthDPCM;
    case 4:
      return lengthQSoundA;
    case 5:
      return lengthA;
    case 6:
      return lengthB;
    case 7:
      return lengthX68;
    case 8:
      return length8;
    case 9:
      return lengthBRR;
    case 10:
      return lengthVOX;
    case 16:
      return length16;
  }
  return 0;
}

DivSample::~DivSample() {
  if (data8) delete[] data8;
  if (data16) delete[] data16;
  if (data1) delete[] data1;
  if (dataDPCM) delete[] dataDPCM;
  if (dataQSoundA) delete[] dataQSoundA;
  if (dataA) delete[] dataA;
  if (dataB) delete[] dataB;
  if (dataX68) delete[] dataX68;
  if (dataBRR) delete[] dataBRR;
  if (dataVOX) delete[] dataVOX;
}
