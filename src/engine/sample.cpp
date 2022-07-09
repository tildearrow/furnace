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
#include <math.h>
#include <string.h>
#ifdef HAVE_SNDFILE
#include "sfWrapper.h"
#endif
#include "filter.h"

extern "C" {
#include "../../extern/adpcm/bs_codec.h"
#include "../../extern/adpcm/oki_codec.h"
#include "../../extern/adpcm/yma_codec.h"
#include "../../extern/adpcm/ymb_codec.h"
#include "../../extern/adpcm/ymz_codec.h"
}

DivSampleHistory::~DivSampleHistory() {
  if (data!=NULL) delete[] data;
}

bool DivSample::isLoopable() {
  return (loopStart>=0 && loopStart<loopEnd) && (loopEnd>loopStart && loopEnd<=(int)samples);
}

unsigned int DivSample::getEndPosition(DivSampleDepth depth) {
  int end=loopEnd;
  unsigned int len=samples;
  switch (depth) {
    case DIV_SAMPLE_DEPTH_1BIT:
      end=(loopEnd+7)/8;
      len=length1;
      break;
    case DIV_SAMPLE_DEPTH_1BIT_DPCM:
      end=(loopEnd+7)/8;
      len=lengthDPCM;
      break;
    case DIV_SAMPLE_DEPTH_YMZ_ADPCM:
      end=(loopEnd+1)/2;
      len=lengthZ;
      break;
    case DIV_SAMPLE_DEPTH_QSOUND_ADPCM:
      end=(loopEnd+1)/2;
      len=lengthQSoundA;
      break;
    case DIV_SAMPLE_DEPTH_ADPCM_A:
      end=(loopEnd+1)/2;
      len=lengthA;
      break;
    case DIV_SAMPLE_DEPTH_ADPCM_B:
      end=(loopEnd+1)/2;
      len=lengthB;
      break;
    case DIV_SAMPLE_DEPTH_8BIT:
      end=loopEnd;
      len=length8;
      break;
    case DIV_SAMPLE_DEPTH_BRR:
      end=9*((loopEnd+15)/16);
      len=lengthBRR;
      break;
    case DIV_SAMPLE_DEPTH_VOX:
      end=(loopEnd+1)/2;
      len=lengthVOX;
      break;
    case DIV_SAMPLE_DEPTH_16BIT:
      end=loopEnd*2;
      len=length16;
      break;
    default:
      break;
  }
  return isLoopable()?end:len;
}

void DivSample::setSampleCount(unsigned int count) {
  samples=count;
  if ((!isLoopable()) || loopEnd<0 || loopEnd>(int)samples) loopEnd=samples;
}

bool DivSample::save(const char* path) {
#ifndef HAVE_SNDFILE
  logE("Furnace was not compiled with libsndfile!");
  return false;
#else
  SNDFILE* f;
  SF_INFO si;
  SFWrapper sfWrap;
  memset(&si,0,sizeof(SF_INFO));

  if (length16<1) return false;

  si.channels=1;
  si.samplerate=rate;
  switch (depth) {
    case DIV_SAMPLE_DEPTH_8BIT: // 8-bit
      si.format=SF_FORMAT_PCM_U8|SF_FORMAT_WAV;
      break;
    default: // 16-bit
      si.format=SF_FORMAT_PCM_16|SF_FORMAT_WAV;
      break;
  }

  f=sfWrap.doOpen(path,SFM_WRITE,&si);

  if (f==NULL) {
    logE("could not open wave file for saving! %s",sf_error_number(sf_error(f)));
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
  if(isLoopable())
  {
    inst.loop_count = 1;
    inst.loops[0].mode = SF_LOOP_FORWARD;
    inst.loops[0].start = loopStart;
    inst.loops[0].end = loopEnd;
  }
  sf_command(f, SFC_SET_INSTRUMENT, &inst, sizeof(inst));

  switch (depth) {
    case DIV_SAMPLE_DEPTH_8BIT: {
      // convert from signed to unsigned
      unsigned char* buf=new unsigned char[length8];
      for (size_t i=0; i<length8; i++) {
        buf[i]=data8[i]^0x80;
      }
      sf_write_raw(f,buf,length8);
      delete[] buf;
      break;
    }
    default:
      sf_write_raw(f,data16,length16);
      break;
  }

  sfWrap.doClose();

  return true;
#endif
}

// 16-bit memory is padded to 512, to make things easier for ADPCM-A/B.
bool DivSample::initInternal(DivSampleDepth d, int count) {
  switch (d) {
    case DIV_SAMPLE_DEPTH_1BIT: // 1-bit
      if (data1!=NULL) delete[] data1;
      length1=(count+7)/8;
      data1=new unsigned char[length1];
      memset(data1,0,length1);
      break;
    case DIV_SAMPLE_DEPTH_1BIT_DPCM: // DPCM
      if (dataDPCM!=NULL) delete[] dataDPCM;
      lengthDPCM=(count+7)/8;
      dataDPCM=new unsigned char[lengthDPCM];
      memset(dataDPCM,0,lengthDPCM);
      break;
    case DIV_SAMPLE_DEPTH_YMZ_ADPCM: // YMZ ADPCM
      if (dataZ!=NULL) delete[] dataZ;
      lengthZ=(count+1)/2;
      // for padding AICA sample
      dataZ=new unsigned char[(lengthZ+3)&(~0x03)];
      memset(dataZ,0,(lengthZ+3)&(~0x03));
      break;
    case DIV_SAMPLE_DEPTH_QSOUND_ADPCM: // QSound ADPCM
      if (dataQSoundA!=NULL) delete[] dataQSoundA;
      lengthQSoundA=(count+1)/2;
      dataQSoundA=new unsigned char[lengthQSoundA];
      memset(dataQSoundA,0,lengthQSoundA);
      break;
    case DIV_SAMPLE_DEPTH_ADPCM_A: // ADPCM-A
      if (dataA!=NULL) delete[] dataA;
      lengthA=(count+1)/2;
      dataA=new unsigned char[(lengthA+255)&(~0xff)];
      memset(dataA,0,(lengthA+255)&(~0xff));
      break;
    case DIV_SAMPLE_DEPTH_ADPCM_B: // ADPCM-B
      if (dataB!=NULL) delete[] dataB;
      lengthB=(count+1)/2;
      dataB=new unsigned char[(lengthB+255)&(~0xff)];
      memset(dataB,0,(lengthB+255)&(~0xff));
      break;
    case DIV_SAMPLE_DEPTH_8BIT: // 8-bit
      if (data8!=NULL) delete[] data8;
      length8=count;
      // for padding X1-010 sample
      data8=new signed char[(count+4095)&(~0xfff)];
      memset(data8,0,(count+4095)&(~0xfff));
      break;
    case DIV_SAMPLE_DEPTH_BRR: // BRR
      if (dataBRR!=NULL) delete[] dataBRR;
      lengthBRR=9*((count+15)/16);
      dataBRR=new unsigned char[lengthBRR];
      memset(dataBRR,0,lengthBRR);
      break;
    case DIV_SAMPLE_DEPTH_VOX: // VOX
      if (dataVOX!=NULL) delete[] dataVOX;
      lengthVOX=(count+1)/2;
      dataVOX=new unsigned char[lengthVOX];
      memset(dataVOX,0,lengthVOX);
      break;
    case DIV_SAMPLE_DEPTH_16BIT: // 16-bit
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
  setSampleCount(count);
  return true;
}

bool DivSample::resize(unsigned int count) {
  if (depth==DIV_SAMPLE_DEPTH_8BIT) {
    if (data8!=NULL) {
      signed char* oldData8=data8;
      data8=NULL;
      initInternal(DIV_SAMPLE_DEPTH_8BIT,count);
      memcpy(data8,oldData8,MIN(count,samples));
      delete[] oldData8;
    } else {
      initInternal(DIV_SAMPLE_DEPTH_8BIT,count);
    }
    setSampleCount(count);
    return true;
  } else if (depth==DIV_SAMPLE_DEPTH_16BIT) {
    if (data16!=NULL) {
      short* oldData16=data16;
      data16=NULL;
      initInternal(DIV_SAMPLE_DEPTH_16BIT,count);
      memcpy(data16,oldData16,sizeof(short)*MIN(count,samples));
      delete[] oldData16;
    } else {
      initInternal(DIV_SAMPLE_DEPTH_16BIT,count);
    }
    setSampleCount(count);
    return true;
  }
  return false;
}

bool DivSample::strip(unsigned int begin, unsigned int end) {
  if (begin>samples) begin=samples;
  if (end>samples) end=samples;
  int count=samples-(end-begin);
  if (count<=0) return resize(0);
  if (depth==DIV_SAMPLE_DEPTH_8BIT) {
    if (data8!=NULL) {
      signed char* oldData8=data8;
      data8=NULL;
      initInternal(DIV_SAMPLE_DEPTH_8BIT,count);
      if (begin>0) {
        memcpy(data8,oldData8,begin);
      }
      if (samples-end>0) {
        memcpy(data8+begin,oldData8+end,samples-end);
      }
      delete[] oldData8;
    } else {
      // do nothing
      return true;
    }
    setSampleCount(count);
    return true;
  } else if (depth==DIV_SAMPLE_DEPTH_16BIT) {
    if (data16!=NULL) {
      short* oldData16=data16;
      data16=NULL;
      initInternal(DIV_SAMPLE_DEPTH_16BIT,count);
      if (begin>0) {
        memcpy(data16,oldData16,sizeof(short)*begin);
      }
      if (samples-end>0) {
        memcpy(&(data16[begin]),&(oldData16[end]),sizeof(short)*(samples-end));
      }
      delete[] oldData16;
    } else {
      // do nothing
      return true;
    }
    setSampleCount(count);
    return true;
  }
  return false;
}

bool DivSample::trim(unsigned int begin, unsigned int end) {
  int count=end-begin;
  if (count==0) return true;
  if (begin==0 && end==samples) return true;
  if (depth==DIV_SAMPLE_DEPTH_8BIT) {
    if (data8!=NULL) {
      signed char* oldData8=data8;
      data8=NULL;
      initInternal(DIV_SAMPLE_DEPTH_8BIT,count);
      memcpy(data8,oldData8+begin,count);
      delete[] oldData8;
    } else {
      // do nothing
      return true;
    }
    setSampleCount(count);
    return true;
  } else if (depth==DIV_SAMPLE_DEPTH_16BIT) {
    if (data16!=NULL) {
      short* oldData16=data16;
      data16=NULL;
      initInternal(DIV_SAMPLE_DEPTH_16BIT,count);
      memcpy(data16,&(oldData16[begin]),sizeof(short)*count);
      delete[] oldData16;
    } else {
      // do nothing
      return true;
    }
    setSampleCount(count);
    return true;
  }
  return false;
}

bool DivSample::insert(unsigned int pos, unsigned int length) {
  unsigned int count=samples+length;
  if (depth==DIV_SAMPLE_DEPTH_8BIT) {
    if (data8!=NULL) {
      signed char* oldData8=data8;
      data8=NULL;
      initInternal(DIV_SAMPLE_DEPTH_8BIT,count);
      if (pos>0) {
        memcpy(data8,oldData8,pos);
      }
      if (count-pos-length>0) {
        memcpy(data8+pos+length,oldData8+pos,count-pos-length);
      }
      delete[] oldData8;
    } else {
      initInternal(DIV_SAMPLE_DEPTH_8BIT,count);
    }
    setSampleCount(count);
    return true;
  } else if (depth==DIV_SAMPLE_DEPTH_16BIT) {
    if (data16!=NULL) {
      short* oldData16=data16;
      data16=NULL;
      initInternal(DIV_SAMPLE_DEPTH_16BIT,count);
      if (pos>0) {
        memcpy(data16,oldData16,sizeof(short)*pos);
      }
      if (count-pos-length>0) {
        memcpy(&(data16[pos+length]),&(oldData16[pos]),sizeof(short)*(count-pos-length));
      }
      delete[] oldData16;
    } else {
      initInternal(DIV_SAMPLE_DEPTH_16BIT,count);
    }
    setSampleCount(count);
    return true;
  }
  return false;
}

#define RESAMPLE_BEGIN \
  if (samples<1) return true; \
  int finalCount=(double)samples*(r/(double)rate); \
  signed char* oldData8=data8; \
  short* oldData16=data16; \
  if (depth==DIV_SAMPLE_DEPTH_16BIT) { \
    if (data16!=NULL) { \
      data16=NULL; \
      initInternal(DIV_SAMPLE_DEPTH_16BIT,finalCount); \
    } \
  } else if (depth==DIV_SAMPLE_DEPTH_8BIT) {  \
    if (data8!=NULL) { \
      data8=NULL; \
      initInternal(DIV_SAMPLE_DEPTH_8BIT,finalCount); \
    } \
  } else { \
    return false; \
  }

#define RESAMPLE_END \
  if (loopStart>=0) loopStart=(double)loopStart*(r/(double)rate); \
  if (loopEnd>=0) loopEnd=(double)loopEnd*(r/(double)rate); \
  centerRate=(int)((double)centerRate*(r/(double)rate)); \
  rate=r; \
  samples=finalCount; \
  if (depth==DIV_SAMPLE_DEPTH_16BIT) { \
    delete[] oldData16; \
  } else if (depth==DIV_SAMPLE_DEPTH_8BIT) { \
    delete[] oldData8; \
  }

bool DivSample::resampleNone(double r) {
  RESAMPLE_BEGIN;

  if (depth==DIV_SAMPLE_DEPTH_16BIT) {
    for (int i=0; i<finalCount; i++) {
      unsigned int pos=(unsigned int)((double)i*((double)rate/r));
      if (pos>=samples) {
        data16[i]=0;
      } else {
        data16[i]=oldData16[pos];
      }
    }
  } else if (depth==DIV_SAMPLE_DEPTH_8BIT) {
    for (int i=0; i<finalCount; i++) {
      unsigned int pos=(unsigned int)((double)i*((double)rate/r));
      if (pos>=samples) {
        data8[i]=0;
      } else {
        data8[i]=oldData8[pos];
      }
    }
  }

  RESAMPLE_END;
  return true;
}

bool DivSample::resampleLinear(double r) {
  RESAMPLE_BEGIN;

  double posFrac=0;
  unsigned int posInt=0;
  double factor=(double)rate/r;

  if (depth==DIV_SAMPLE_DEPTH_16BIT) {
    for (int i=0; i<finalCount; i++) {
      short s1=(posInt>=samples)?0:oldData16[posInt];
      short s2=(posInt+1>=samples)?((loopStart>=0 && loopStart<(int)samples)?oldData16[loopStart]:0):oldData16[posInt+1];

      data16[i]=s1+(float)(s2-s1)*posFrac;

      posFrac+=factor;
      while (posFrac>=1.0) {
        posFrac-=1.0;
        posInt++;
      }
    }
  } else if (depth==DIV_SAMPLE_DEPTH_8BIT) {
    for (int i=0; i<finalCount; i++) {
      short s1=(posInt>=samples)?0:oldData8[posInt];
      short s2=(posInt+1>=samples)?((loopStart>=0 && loopStart<(int)samples)?oldData8[loopStart]:0):oldData8[posInt+1];

      data8[i]=s1+(float)(s2-s1)*posFrac;

      posFrac+=factor;
      while (posFrac>=1.0) {
        posFrac-=1.0;
        posInt++;
      }
    }
  }

  RESAMPLE_END;
  return true;
}

bool DivSample::resampleCubic(double r) {
  RESAMPLE_BEGIN;

  double posFrac=0;
  unsigned int posInt=0;
  double factor=(double)rate/r;
  float* cubicTable=DivFilterTables::getCubicTable();

  if (depth==DIV_SAMPLE_DEPTH_16BIT) {
    for (int i=0; i<finalCount; i++) {
      unsigned int n=((unsigned int)(posFrac*1024.0))&1023;
      float* t=&cubicTable[n<<2];
      float s0=(posInt<1)?0:oldData16[posInt-1];
      float s1=(posInt>=samples)?0:oldData16[posInt];
      float s2=(posInt+1>=samples)?((loopStart>=0 && loopStart<(int)samples)?oldData16[loopStart]:0):oldData16[posInt+1];
      float s3=(posInt+2>=samples)?((loopStart>=0 && loopStart<(int)samples)?oldData16[loopStart]:0):oldData16[posInt+2];

      float result=s0*t[0]+s1*t[1]+s2*t[2]+s3*t[3];
      if (result<-32768) result=-32768;
      if (result>32767) result=32767;
      data16[i]=result;

      posFrac+=factor;
      while (posFrac>=1.0) {
        posFrac-=1.0;
        posInt++;
      }
    }
  } else if (depth==DIV_SAMPLE_DEPTH_8BIT) {
    for (int i=0; i<finalCount; i++) {
      unsigned int n=((unsigned int)(posFrac*1024.0))&1023;
      float* t=&cubicTable[n<<2];
      float s0=(posInt<1)?0:oldData8[posInt-1];
      float s1=(posInt>=samples)?0:oldData8[posInt];
      float s2=(posInt+1>=samples)?((loopStart>=0 && loopStart<(int)samples)?oldData8[loopStart]:0):oldData8[posInt+1];
      float s3=(posInt+2>=samples)?((loopStart>=0 && loopStart<(int)samples)?oldData8[loopStart]:0):oldData8[posInt+2];

      float result=s0*t[0]+s1*t[1]+s2*t[2]+s3*t[3];
      if (result<-128) result=-128;
      if (result>127) result=127;
      data8[i]=result;

      posFrac+=factor;
      while (posFrac>=1.0) {
        posFrac-=1.0;
        posInt++;
      }
    }
  }

  RESAMPLE_END;
  return true;
}

bool DivSample::resampleBlep(double r) {
  RESAMPLE_BEGIN;

  double posFrac=0;
  unsigned int posInt=0;
  double factor=r/(double)rate;
  float* sincITable=DivFilterTables::getSincIntegralTable();
  float s[16];

  memset(s,0,16*sizeof(float));

  if (depth==DIV_SAMPLE_DEPTH_16BIT) {
    memset(data16,0,finalCount*sizeof(short));
    for (int i=0; i<finalCount; i++) {
      if (posInt<samples) {
        int result=data16[i]+oldData16[posInt];
        if (result<-32768) result=-32768;
        if (result>32767) result=32767;
        data16[i]=result;
      }
      
      posFrac+=1.0;
      while (posFrac>=1.0) {
        unsigned int n=((unsigned int)(posFrac*8192.0))&8191;
        posFrac-=factor;
        posInt++;

        float* t1=&sincITable[(8191-n)<<3];
        float* t2=&sincITable[n<<3];
        float delta=oldData16[posInt]-oldData16[posInt-1];

        for (int j=0; j<8; j++) {
          if (i-j>0) {
            float result=data16[i-j]+t1[j]*-delta;
            if (result<-32768) result=-32768;
            if (result>32767) result=32767;
            data16[i-j]=result;
          }
          if (i+j+1<finalCount) {
            float result=data16[i+j+1]+t2[j]*delta;
            if (result<-32768) result=-32768;
            if (result>32767) result=32767;
            data16[i+j+1]=result;
          }
        }
      }
    }
  } else if (depth==DIV_SAMPLE_DEPTH_8BIT) {
    memset(data8,0,finalCount);
    for (int i=0; i<finalCount; i++) {
      if (posInt<samples) {
        int result=data8[i]+oldData8[posInt];
        if (result<-128) result=-128;
        if (result>127) result=127;
        data8[i]=result;
      }
      
      posFrac+=1.0;
      while (posFrac>=1.0) {
        unsigned int n=((unsigned int)(posFrac*8192.0))&8191;
        posFrac-=factor;
        posInt++;

        float* t1=&sincITable[(8191-n)<<3];
        float* t2=&sincITable[n<<3];
        float delta=oldData8[posInt]-oldData8[posInt-1];

        for (int j=0; j<8; j++) {
          if (i-j>0) {
            float result=data8[i-j]+t1[j]*-delta;
            if (result<-128) result=-128;
            if (result>127) result=127;
            data8[i-j]=result;
          }
          if (i+j+1<finalCount) {
            float result=data8[i+j+1]+t2[j]*delta;
            if (result<-128) result=-128;
            if (result>127) result=127;
            data8[i+j+1]=result;
          }
        }
      }
    }
  }

  RESAMPLE_END;
  return true;
}

bool DivSample::resampleSinc(double r) {
  RESAMPLE_BEGIN;

  double posFrac=0;
  unsigned int posInt=0;
  double factor=(double)rate/r;
  float* sincTable=DivFilterTables::getSincTable();
  float s[16];

  memset(s,0,16*sizeof(float));

  if (depth==DIV_SAMPLE_DEPTH_16BIT) {
    for (int i=0; i<finalCount+8; i++) {
      unsigned int n=((unsigned int)(posFrac*8192.0))&8191;
      float result=0;
      float* t1=&sincTable[(8191-n)<<3];
      float* t2=&sincTable[n<<3];
      for (int j=0; j<8; j++) {
        result+=s[j]*t2[7-j];
        result+=s[8+j]*t1[j];
      }
      if (result<-32768) result=-32768;
      if (result>32767) result=32767;
      if (i>=8) {
        data16[i-8]=result;
      }
      
      posFrac+=factor;
      while (posFrac>=1.0) {
        posFrac-=1.0;
        posInt++;

        for (int j=0; j<15; j++) s[j]=s[j+1];
        s[15]=(posInt>=samples)?0:oldData16[posInt];
      }
    }
  } else if (depth==DIV_SAMPLE_DEPTH_8BIT) {
    for (int i=0; i<finalCount+8; i++) {
      unsigned int n=((unsigned int)(posFrac*8192.0))&8191;
      float result=0;
      float* t1=&sincTable[(8191-n)<<3];
      float* t2=&sincTable[n<<3];
      for (int j=0; j<8; j++) {
        result+=s[j]*t2[7-j];
        result+=s[8+j]*t1[j];
      }
      if (result<-128) result=-128;
      if (result>127) result=127;
      if (i>=8) {
        data8[i-8]=result;
      }
      
      posFrac+=factor;
      while (posFrac>=1.0) {
        posFrac-=1.0;
        posInt++;

        for (int j=0; j<15; j++) s[j]=s[j+1];
        s[15]=(posInt>=samples)?0:oldData8[posInt];
      }
    }
  }

  RESAMPLE_END;
  return true;
}

bool DivSample::resample(double r, int filter) {
  if (depth!=DIV_SAMPLE_DEPTH_8BIT && depth!=DIV_SAMPLE_DEPTH_16BIT) return false;
  switch (filter) {
    case DIV_RESAMPLE_NONE:
      return resampleNone(r);
      break;
    case DIV_RESAMPLE_LINEAR:
      return resampleLinear(r);
      break;
    case DIV_RESAMPLE_CUBIC:
      return resampleCubic(r);
      break;
    case DIV_RESAMPLE_BLEP:
      return resampleBlep(r);
      break;
    case DIV_RESAMPLE_SINC:
      return resampleSinc(r);
      break;
    case DIV_RESAMPLE_BEST:
      if (r>rate) {
        return resampleSinc(r);
      } else {
        return resampleBlep(r);
      }
      break;
  }
  return false;
}

void DivSample::render() {
  // step 1: convert to 16-bit if needed
  if (depth!=DIV_SAMPLE_DEPTH_16BIT) {
    if (!initInternal(DIV_SAMPLE_DEPTH_16BIT,samples)) return;
    switch (depth) {
      case DIV_SAMPLE_DEPTH_1BIT: // 1-bit
        for (unsigned int i=0; i<samples; i++) {
          data16[i]=((data1[i>>3]>>(i&7))&1)?0x7fff:-0x7fff;
        }
        break;
      case DIV_SAMPLE_DEPTH_1BIT_DPCM: { // DPCM
        int accum=0;
        for (unsigned int i=0; i<samples; i++) {
          accum+=((dataDPCM[i>>3]>>(i&7))&1)?1:-1;
          if (accum>63) accum=63;
          if (accum<-64) accum=-64;
          data16[i]=accum*512;
        }
        break;
      }
      case DIV_SAMPLE_DEPTH_YMZ_ADPCM: // YMZ ADPCM
        ymz_decode(dataZ,data16,samples);
        break;
      case DIV_SAMPLE_DEPTH_QSOUND_ADPCM: // QSound ADPCM
        bs_decode(dataQSoundA,data16,samples);
        break;
      case DIV_SAMPLE_DEPTH_ADPCM_A: // ADPCM-A
        yma_decode(dataA,data16,samples);
        break;
      case DIV_SAMPLE_DEPTH_ADPCM_B: // ADPCM-B
        ymb_decode(dataB,data16,samples);
        break;
      case DIV_SAMPLE_DEPTH_8BIT: // 8-bit PCM
        for (unsigned int i=0; i<samples; i++) {
          data16[i]=data8[i]<<8;
        }
        break;
      case DIV_SAMPLE_DEPTH_BRR: // BRR
        // TODO!
        break;
      case DIV_SAMPLE_DEPTH_VOX: // VOX
        oki_decode(dataVOX,data16,samples);
        break;
      default:
        return;
    }
  }

  // step 2: render to other formats
  if (depth!=DIV_SAMPLE_DEPTH_1BIT) { // 1-bit
    if (!initInternal(DIV_SAMPLE_DEPTH_1BIT,samples)) return;
    for (unsigned int i=0; i<samples; i++) {
      if (data16[i]>0) {
        data1[i>>3]|=1<<(i&7);
      }
    }
  }
  if (depth!=DIV_SAMPLE_DEPTH_1BIT_DPCM) { // DPCM
    if (!initInternal(DIV_SAMPLE_DEPTH_1BIT_DPCM,samples)) return;
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
  if (depth!=DIV_SAMPLE_DEPTH_YMZ_ADPCM) { // YMZ ADPCM
    if (!initInternal(DIV_SAMPLE_DEPTH_YMZ_ADPCM,samples)) return;
    ymz_encode(data16,dataZ,(samples+7)&(~0x7));
  }
  if (depth!=DIV_SAMPLE_DEPTH_QSOUND_ADPCM) { // QSound ADPCM
    if (!initInternal(DIV_SAMPLE_DEPTH_QSOUND_ADPCM,samples)) return;
    bs_encode(data16,dataQSoundA,samples);
  }
  // TODO: pad to 256.
  if (depth!=DIV_SAMPLE_DEPTH_ADPCM_A) { // ADPCM-A
    if (!initInternal(DIV_SAMPLE_DEPTH_ADPCM_A,samples)) return;
    yma_encode(data16,dataA,(samples+511)&(~0x1ff));
  }
  if (depth!=DIV_SAMPLE_DEPTH_ADPCM_B) { // ADPCM-B
    if (!initInternal(DIV_SAMPLE_DEPTH_ADPCM_B,samples)) return;
    ymb_encode(data16,dataB,(samples+511)&(~0x1ff));
  }
  if (depth!=DIV_SAMPLE_DEPTH_8BIT) { // 8-bit PCM
    if (!initInternal(DIV_SAMPLE_DEPTH_8BIT,samples)) return;
    for (unsigned int i=0; i<samples; i++) {
      data8[i]=data16[i]>>8;
    }
  }
  // TODO: BRR!
  if (depth!=DIV_SAMPLE_DEPTH_VOX) { // VOX
    if (!initInternal(DIV_SAMPLE_DEPTH_VOX,samples)) return;
    oki_encode(data16,dataVOX,samples);
  }
}

void* DivSample::getCurBuf() {
  switch (depth) {
    case DIV_SAMPLE_DEPTH_1BIT:
      return data1;
    case DIV_SAMPLE_DEPTH_1BIT_DPCM:
      return dataDPCM;
    case DIV_SAMPLE_DEPTH_YMZ_ADPCM:
      return dataZ;
    case DIV_SAMPLE_DEPTH_QSOUND_ADPCM:
      return dataQSoundA;
    case DIV_SAMPLE_DEPTH_ADPCM_A:
      return dataA;
    case DIV_SAMPLE_DEPTH_ADPCM_B:
      return dataB;
    case DIV_SAMPLE_DEPTH_8BIT:
      return data8;
    case DIV_SAMPLE_DEPTH_BRR:
      return dataBRR;
    case DIV_SAMPLE_DEPTH_VOX:
      return dataVOX;
    case DIV_SAMPLE_DEPTH_16BIT:
      return data16;
    default:
      return NULL;
  }
  return NULL;
}

unsigned int DivSample::getCurBufLen() {
  switch (depth) {
    case DIV_SAMPLE_DEPTH_1BIT:
      return length1;
    case DIV_SAMPLE_DEPTH_1BIT_DPCM:
      return lengthDPCM;
    case DIV_SAMPLE_DEPTH_YMZ_ADPCM:
      return lengthZ;
    case DIV_SAMPLE_DEPTH_QSOUND_ADPCM:
      return lengthQSoundA;
    case DIV_SAMPLE_DEPTH_ADPCM_A:
      return lengthA;
    case DIV_SAMPLE_DEPTH_ADPCM_B:
      return lengthB;
    case DIV_SAMPLE_DEPTH_8BIT:
      return length8;
    case DIV_SAMPLE_DEPTH_BRR:
      return lengthBRR;
    case DIV_SAMPLE_DEPTH_VOX:
      return lengthVOX;
    case DIV_SAMPLE_DEPTH_16BIT:
      return length16;
    default:
      return 0;
  }
  return 0;
}

DivSampleHistory* DivSample::prepareUndo(bool data, bool doNotPush) {
  DivSampleHistory* h;
  if (data) {
    unsigned char* duplicate;
    if (getCurBuf()==NULL) {
      duplicate=NULL;
    } else {
      duplicate=new unsigned char[getCurBufLen()];
      memcpy(duplicate,getCurBuf(),getCurBufLen());
    }
    h=new DivSampleHistory(duplicate,getCurBufLen(),samples,depth,rate,centerRate,loopStart,loopEnd);
  } else {
    h=new DivSampleHistory(depth,rate,centerRate,loopStart,loopEnd);
  }
  if (!doNotPush) {
    while (!redoHist.empty()) {
      DivSampleHistory* h=redoHist.back();
      delete h;
      redoHist.pop_back();
    }
    if (undoHist.size()>100) undoHist.pop_front();
    undoHist.push_back(h);
  }
  return h;
}

#define applyHistory \
  depth=h->depth; \
  if (h->hasSample) { \
    initInternal(h->depth,h->samples); \
    samples=h->samples; \
\
    if (h->length!=getCurBufLen()) logW("undo buffer length not equal to current buffer length! %d != %d",h->length,getCurBufLen()); \
\
    void* buf=getCurBuf(); \
\
    if (buf!=NULL && h->data!=NULL) { \
      memcpy(buf,h->data,h->length); \
    } \
  } \
  rate=h->rate; \
  centerRate=h->centerRate; \
  loopStart=h->loopStart; \
  loopEnd=h->loopEnd;


int DivSample::undo() {
  if (undoHist.empty()) return 0;
  DivSampleHistory* h=undoHist.back();
  DivSampleHistory* redo=prepareUndo(h->hasSample,true);

  int ret=h->hasSample?2:1;

  applyHistory;

  redoHist.push_back(redo);
  delete h;
  undoHist.pop_back();
  return ret;
}

int DivSample::redo() {
  if (redoHist.empty()) return 0;
  DivSampleHistory* h=redoHist.back();
  DivSampleHistory* undo=prepareUndo(h->hasSample,true);

  int ret=h->hasSample?2:1;

  applyHistory;

  undoHist.push_back(undo);
  delete h;
  redoHist.pop_back();
  return ret;
}

DivSample::~DivSample() {
  while (!undoHist.empty()) {
    DivSampleHistory* h=undoHist.back();
    delete h;
    undoHist.pop_back();
  }
  while (!redoHist.empty()) {
    DivSampleHistory* h=redoHist.back();
    delete h;
    redoHist.pop_back();
  }
  if (data8) delete[] data8;
  if (data16) delete[] data16;
  if (data1) delete[] data1;
  if (dataDPCM) delete[] dataDPCM;
  if (dataZ) delete[] dataZ;
  if (dataQSoundA) delete[] dataQSoundA;
  if (dataA) delete[] dataA;
  if (dataB) delete[] dataB;
  if (dataBRR) delete[] dataBRR;
  if (dataVOX) delete[] dataVOX;
}
