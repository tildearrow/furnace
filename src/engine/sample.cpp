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

#include "sample.h"
#include "../ta-log.h"
#include "../fileutils.h"
#include <math.h>
#include <string.h>
#ifdef HAVE_SNDFILE
#include "sfWrapper.h"
#endif
#include "filter.h"
#include "bsr.h"

extern "C" {
#include "../../extern/adpcm/bs_codec.h"
#include "../../extern/adpcm/oki_codec.h"
#include "../../extern/adpcm/yma_codec.h"
#include "../../extern/adpcm/ymb_codec.h"
#include "../../extern/adpcm/ymz_codec.h"
}
#include "../../extern/adpcm-xq-s/adpcm-lib.h"
#include "brrUtils.h"

DivSampleHistory::~DivSampleHistory() {
  if (data!=NULL) delete[] data;
}

void DivSample::putSampleData(SafeWriter* w) {
  size_t blockStartSeek, blockEndSeek;

  w->write("SMP2",4);
  blockStartSeek=w->tell();
  w->writeI(0);

  w->writeString(name,false);
  w->writeI(samples);
  w->writeI(rate);
  w->writeI(centerRate);
  w->writeC(depth);
  w->writeC(loopMode);
  w->writeC(brrEmphasis);
  w->writeC((dither?1:0)|(brrNoFilter?2:0));
  w->writeI(loop?loopStart:-1);
  w->writeI(loop?loopEnd:-1);

  for (int i=0; i<DIV_MAX_SAMPLE_TYPE; i++) {
    unsigned int out=0;
    for (int j=0; j<DIV_MAX_CHIPS; j++) {
      if (renderOn[i][j]) out|=1<<j;
    }
    w->writeI(out);
  }

#ifdef TA_BIG_ENDIAN
  // store 16-bit samples as little-endian
  if (depth==DIV_SAMPLE_DEPTH_16BIT) {
    unsigned char* sampleBuf=(unsigned char*)getCurBuf();
    size_t bufLen=getCurBufLen();
    for (size_t i=0; i<bufLen; i+=2) {
      w->writeC(sampleBuf[i+1]);
      w->writeC(sampleBuf[i]);
    }
  } else {
    w->write(getCurBuf(),getCurBufLen());
  }
#else
  w->write(getCurBuf(),getCurBufLen());
#endif

  blockEndSeek=w->tell();
  w->seek(blockStartSeek,SEEK_SET);
  w->writeI(blockEndSeek-blockStartSeek-4);
  w->seek(0,SEEK_END);
}

// Delek why
static double samplePitchesSD[11]={
  0.1666666666, 0.2, 0.25, 0.333333333, 0.5,
  1,
  2, 3, 4, 5, 6
};

DivDataErrors DivSample::readSampleData(SafeReader& reader, short version) {
  int vol=0;
  int pitch=0;
  char magic[4];

  reader.read(magic,4);
  if (memcmp(magic,"SMPL",4)!=0 && memcmp(magic,"SMP2",4)!=0) {
    logV("header is invalid: %c%c%c%c",magic[0],magic[1],magic[2],magic[3]);
    return DIV_DATA_INVALID_HEADER;
  }
  bool isNewSample=(memcmp(magic,"SMP2",4)==0);
  reader.readI();
  if (!isNewSample) logV("(old sample)");

  name=reader.readString();
  samples=reader.readI();
  if (!isNewSample) {
    loopEnd=samples;
  }
  rate=reader.readI();

  if (isNewSample) {
    centerRate=reader.readI();
    depth=(DivSampleDepth)reader.readC();
    if (version>=123) {
      loopMode=(DivSampleLoopMode)reader.readC();
    } else {
      loopMode=DIV_SAMPLE_LOOP_FORWARD;
      reader.readC();
    }

    if (version>=129) {
      brrEmphasis=reader.readC();
    } else {
      reader.readC();
    }
    if (version>=159) {
      signed char c=reader.readC();
      dither=c&1;
      if (version>=213) brrNoFilter=c&2;
    } else {
      reader.readC();
    }

    loopStart=reader.readI();
    loopEnd=reader.readI();
    loop=(loopStart>=0)&&(loopEnd>=0);

    for (int i=0; i<DIV_MAX_SAMPLE_TYPE; i++) {
      unsigned int outMask=(unsigned int)reader.readI();
      for (int j=0; j<DIV_MAX_CHIPS; j++) {
        renderOn[i][j]=outMask&(1<<j);
      }
    }
  } else {
    if (version<58) {
      vol=reader.readS();
      pitch=reader.readS();
    } else {
      reader.readI();
    }
    depth=(DivSampleDepth)reader.readC();

    // reserved
    reader.readC();

    // while version 32 stored this value, it was unused.
    if (version>=38) {
      centerRate=(unsigned short)reader.readS();
    } else {
      reader.readS();
    }

    if (version>=19) {
      loopStart=reader.readI();
      loop=(loopStart>=0)&&(loopEnd>=0);
    } else {
      reader.readI();
    }
  }

  if (version>=58) { // modern sample
    init(samples);
    reader.read(getCurBuf(),getCurBufLen());
#ifdef TA_BIG_ENDIAN
    // convert 16-bit samples to big-endian
    if (depth==DIV_SAMPLE_DEPTH_16BIT) {
      unsigned char* sampleBuf=(unsigned char*)getCurBuf();
      size_t sampleBufLen=getCurBufLen();
      for (size_t pos=0; pos<sampleBufLen; pos+=2) {
        sampleBuf[pos]^=sampleBuf[pos+1];
        sampleBuf[pos+1]^=sampleBuf[pos];
        sampleBuf[pos]^=sampleBuf[pos+1];
      }
    }
#endif
  } else { // legacy sample
    int length=samples;
    short* data=new short[length];
    reader.read(data,2*length);

#ifdef TA_BIG_ENDIAN
    // convert 16-bit samples to big-endian
    for (int pos=0; pos<length; pos++) {
      data[pos]=((unsigned short)data[pos]>>8)|((unsigned short)data[pos]<<8);
    }
#endif

    if (pitch!=5) {
      logD("scaling from %d...",pitch);
    }

    // render data
    if (depth!=DIV_SAMPLE_DEPTH_8BIT && depth!=DIV_SAMPLE_DEPTH_16BIT) {
      logW("sample depth is wrong! (%d)",(int)depth);
      depth=DIV_SAMPLE_DEPTH_16BIT;
    }
    samples=(double)samples/samplePitchesSD[pitch];
    init(samples);

    unsigned int k=0;
    float mult=(float)(vol)/50.0f;
    for (double j=0; j<length; j+=samplePitchesSD[pitch]) {
      if (k>=samples) {
        break;
      }
      if (depth==DIV_SAMPLE_DEPTH_8BIT) {
        float next=(float)(data[(unsigned int)j]-0x80)*mult;
        data8[k++]=fmin(fmax(next,-128),127);
      } else {
        float next=(float)data[(unsigned int)j]*mult;
        data16[k++]=fmin(fmax(next,-32768),32767);
      }
    }

    delete[] data;
  }

  return DIV_DATA_SUCCESS;
}

bool DivSample::isLoopable() {
  return loop && ((loopStart>=0 && loopStart<loopEnd) && (loopEnd>loopStart && loopEnd<=(int)samples));
}

int DivSample::getSampleOffset(int offset, int length, DivSampleDepth depth) {
  if ((length==0) || (offset==length)) {
    int off=offset;
    switch (depth) {
      case DIV_SAMPLE_DEPTH_1BIT:
        off=(offset+7)/8;
        break;
      case DIV_SAMPLE_DEPTH_1BIT_DPCM:
        off=(offset+7)/8;
        break;
      case DIV_SAMPLE_DEPTH_YMZ_ADPCM:
        off=(offset+1)/2;
        break;
      case DIV_SAMPLE_DEPTH_QSOUND_ADPCM:
        off=(offset+1)/2;
        break;
      case DIV_SAMPLE_DEPTH_ADPCM_A:
        off=(offset+1)/2;
        break;
      case DIV_SAMPLE_DEPTH_ADPCM_B:
        off=(offset+1)/2;
        break;
      case DIV_SAMPLE_DEPTH_ADPCM_K:
        off=(offset+1)/2;
        break;
      case DIV_SAMPLE_DEPTH_8BIT:
        off=offset;
        break;
      case DIV_SAMPLE_DEPTH_BRR:
        off=9*((offset+15)/16);
        break;
      case DIV_SAMPLE_DEPTH_VOX:
        off=(offset+1)/2;
        break;
      case DIV_SAMPLE_DEPTH_MULAW:
        off=offset;
        break;
      case DIV_SAMPLE_DEPTH_C219:
        off=offset;
        break;
      case DIV_SAMPLE_DEPTH_IMA_ADPCM:
        off=(offset+1)/2;
        break;
      case DIV_SAMPLE_DEPTH_12BIT:
        off=((offset*3)+1)/2;
        break;
      case DIV_SAMPLE_DEPTH_16BIT:
        off=offset*2;
        break;
      default:
        break;
    }
    return off;
  } else {
    int off=offset;
    int len=length;
    switch (depth) {
      case DIV_SAMPLE_DEPTH_1BIT:
        off=(offset+7)/8;
        len=(length+7)/8;
        break;
      case DIV_SAMPLE_DEPTH_1BIT_DPCM:
        off=(offset+7)/8;
        len=(length+7)/8;
        break;
      case DIV_SAMPLE_DEPTH_YMZ_ADPCM:
        off=(offset+1)/2;
        len=(length+1)/2;
        break;
      case DIV_SAMPLE_DEPTH_QSOUND_ADPCM:
        off=(offset+1)/2;
        len=(length+1)/2;
        break;
      case DIV_SAMPLE_DEPTH_ADPCM_A:
        off=(offset+1)/2;
        len=(length+1)/2;
        break;
      case DIV_SAMPLE_DEPTH_ADPCM_B:
        off=(offset+1)/2;
        len=(length+1)/2;
        break;
      case DIV_SAMPLE_DEPTH_ADPCM_K:
        off=(offset+1)/2;
        len=(length+1)/2;
        break;
      case DIV_SAMPLE_DEPTH_8BIT:
        off=offset;
        len=length;
        break;
      case DIV_SAMPLE_DEPTH_BRR:
        off=9*((offset+15)/16);
        len=9*((length+15)/16);
        break;
      case DIV_SAMPLE_DEPTH_VOX:
        off=(offset+1)/2;
        len=(length+1)/2;
        break;
      case DIV_SAMPLE_DEPTH_MULAW:
        off=offset;
        len=length;
        break;
      case DIV_SAMPLE_DEPTH_C219:
        off=offset;
        len=length;
        break;
      case DIV_SAMPLE_DEPTH_IMA_ADPCM:
        off=(offset+1)/2;
        len=(length+1)/2;
        break;
      case DIV_SAMPLE_DEPTH_12BIT:
        off=((offset*3)+1)/2;
        len=((length*3)+1)/2;
        break;
      case DIV_SAMPLE_DEPTH_16BIT:
        off=offset*2;
        len=length*2;
        break;
      default:
        break;
    }
    return isLoopable()?off:len;
  }
}

int DivSample::getLoopStartPosition(DivSampleDepth depth) {
  return getSampleOffset(loopStart,0,depth);
}

int DivSample::getLoopEndPosition(DivSampleDepth depth) {
  return getSampleOffset(loopEnd,samples,depth);
}

int DivSample::getEndPosition(DivSampleDepth depth) {
  int off=samples;
  switch (depth) {
    case DIV_SAMPLE_DEPTH_1BIT:
      off=length1;
      break;
    case DIV_SAMPLE_DEPTH_1BIT_DPCM:
      off=lengthDPCM;
      break;
    case DIV_SAMPLE_DEPTH_YMZ_ADPCM:
      off=lengthZ;
      break;
    case DIV_SAMPLE_DEPTH_QSOUND_ADPCM:
      off=lengthQSoundA;
      break;
    case DIV_SAMPLE_DEPTH_ADPCM_A:
      off=lengthA;
      break;
    case DIV_SAMPLE_DEPTH_ADPCM_B:
      off=lengthB;
      break;
    case DIV_SAMPLE_DEPTH_ADPCM_K:
      off=lengthK;
      break;
    case DIV_SAMPLE_DEPTH_8BIT:
      off=length8;
      break;
    case DIV_SAMPLE_DEPTH_BRR:
      off=lengthBRR;
      break;
    case DIV_SAMPLE_DEPTH_VOX:
      off=lengthVOX;
      break;
    case DIV_SAMPLE_DEPTH_MULAW:
      off=lengthMuLaw;
      break;
    case DIV_SAMPLE_DEPTH_C219:
      off=lengthC219;
      break;
    case DIV_SAMPLE_DEPTH_IMA_ADPCM:
      off=lengthIMA;
      break;
    case DIV_SAMPLE_DEPTH_12BIT:
      off=length12;
      break;
    case DIV_SAMPLE_DEPTH_16BIT:
      off=length16;
      break;
    default:
      break;
  }
  return off;
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
  si.samplerate=centerRate;
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
  // TODO: fix
  short pitch = (0x3c * 100) + 50 - (log2((double)centerRate/8363.0) * 12.0 * 100.0);
  inst.basenote = pitch / 100;
  inst.detune = 50 - (pitch % 100);
  inst.velocity_hi = 0x7f;
  inst.key_hi = 0x7f;
  if(isLoopable())
  {
    inst.loop_count = 1;
    inst.loops[0].mode = (int)loopMode+SF_LOOP_FORWARD;
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
      sf_writef_short(f,data16,samples);
      break;
  }

  sfWrap.doClose();

  return true;
#endif
}

bool DivSample::saveRaw(const char* path) {
  if (samples<1) {
    logE("sample is empty though!");
    return false;
  }

  FILE* f=ps_fopen(path,"wb");
  if (f==NULL) {
    logE("could not save sample: %s!",strerror(errno));
    return false;
  }
  if (depth==DIV_SAMPLE_DEPTH_BRR) {
    if (isLoopable()) {
      unsigned short loopPos=getLoopStartPosition(DIV_SAMPLE_DEPTH_BRR);
      fputc(loopPos&0xff,f);
      fputc(loopPos>>8,f);
    } else {
      fputc(0,f);
      fputc(0,f);
    }
  }

  if (fwrite(getCurBuf(),1,getCurBufLen(),f)!=getCurBufLen()) {
    logW("did not write entire instrument!");
  }
  fclose(f);
  return true;
}

// 16-bit memory is padded to 512, to make things easier for ADPCM-A/B.
bool DivSample::initInternal(DivSampleDepth d, int count) {
  logV("initInternal(%d,%d)",(int)d,count);
  switch (d) {
    case DIV_SAMPLE_DEPTH_1BIT: // 1-bit
      if (data1!=NULL) delete[] data1;
      length1=(count+7)/8;
      data1=new unsigned char[length1];
      memset(data1,0,length1);
      break;
    case DIV_SAMPLE_DEPTH_1BIT_DPCM: // DPCM
      if (dataDPCM!=NULL) delete[] dataDPCM;
      lengthDPCM=1+((((count-1)/8)+15)&(~15));
      dataDPCM=new unsigned char[lengthDPCM];
      memset(dataDPCM,0xaa,lengthDPCM);
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
      memset(dataA,0x80,(lengthA+255)&(~0xff));
      break;
    case DIV_SAMPLE_DEPTH_ADPCM_B: // ADPCM-B
      if (dataB!=NULL) delete[] dataB;
      lengthB=(count+1)/2;
      dataB=new unsigned char[(lengthB+255)&(~0xff)];
      memset(dataB,0x80,(lengthB+255)&(~0xff));
      break;
    case DIV_SAMPLE_DEPTH_ADPCM_K: // K05 ADPCM
      if (dataK!=NULL) delete[] dataK;
      lengthK=(count+1)/2;
      dataK=new unsigned char[(lengthK+255)&(~0xff)];
      memset(dataK,0,(lengthK+255)&(~0xff));
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
      dataBRR=new unsigned char[lengthBRR+9];
      memset(dataBRR,0,lengthBRR+9);
      break;
    case DIV_SAMPLE_DEPTH_VOX: // VOX
      if (dataVOX!=NULL) delete[] dataVOX;
      lengthVOX=(count+1)/2;
      dataVOX=new unsigned char[lengthVOX];
      memset(dataVOX,0,lengthVOX);
      break;
    case DIV_SAMPLE_DEPTH_MULAW: // 8-bit µ-law
      if (dataMuLaw!=NULL) delete[] dataMuLaw;
      lengthMuLaw=count;
      dataMuLaw=new unsigned char[(count+4095)&(~0xfff)];
      memset(dataMuLaw,0,(count+4095)&(~0xfff));
      break;
    case DIV_SAMPLE_DEPTH_C219: // 8-bit C219 "μ-law"
      if (dataC219!=NULL) delete[] dataC219;
      lengthC219=count;
      dataC219=new unsigned char[(count+4095)&(~0xfff)];
      memset(dataC219,0,(count+4095)&(~0xfff));
      break;
    case DIV_SAMPLE_DEPTH_IMA_ADPCM: // IMA ADPCM
      if (dataIMA!=NULL) delete[] dataIMA;
      lengthIMA=4+((count+1)/2);
      dataIMA=new unsigned char[lengthIMA];
      memset(dataIMA,0,lengthIMA);
      break;
    case DIV_SAMPLE_DEPTH_12BIT: // 12-bit PCM (MultiPCM)
      if (data12!=NULL) delete[] data12;
      length12=((count*3)+1)/2;
      data12=new unsigned char[length12];
      memset(data12,0,length12);
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
  if (count<=0) {
    loopStart=-1;
    loopEnd=-1;
    loop=false;
    return resize(0);
  }
  if (loopStart>(int)begin && loopEnd<(int)end) {
    loopStart=-1;
    loopEnd=-1;
    loop=false;
  } else {
    if (loopStart<(int)end && loopStart>(int)begin) {
      loopStart=end;
    }
    if (loopStart>(int)begin && loopEnd>(int)begin) {
      loopStart-=end-begin;
      loopEnd-=end-begin;
      if (loopEnd<0) loopEnd=0;
      if (loopStart<0) loopStart=0;
    } else if (loopEnd>(int)begin) {
      loopEnd=begin;
    }
  }
  if (loopStart>loopEnd) {
    loopStart=-1;
    loopEnd=-1;
    loop=false;
  }
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
  if (((int)begin<loopStart && (int)end<loopStart) || ((int)begin>loopEnd && (int)end>loopEnd)) {
    loopStart=-1;
    loopEnd=-1;
    loop=false;
  } else {
    loopStart-=begin;
    loopEnd-=begin;
    if (loopStart<0) loopStart=0;
    if (loopEnd>count) loopEnd=count;
  }
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

void DivSample::convert(DivSampleDepth newDepth, unsigned int formatMask) {
  render(formatMask|(1U<<newDepth));
  depth=newDepth;
  switch (depth) {
    case DIV_SAMPLE_DEPTH_1BIT:
      setSampleCount((samples+7)&(~7));
      break;
    case DIV_SAMPLE_DEPTH_1BIT_DPCM:
      if (samples) {
        setSampleCount((1+((((samples-1)/8)+15)&(~15)))<<3);
      } else {
        setSampleCount(8);
      }
      break;
    case DIV_SAMPLE_DEPTH_YMZ_ADPCM:
      setSampleCount(((lengthZ+3)&(~0x03))*2);
      break;
    case DIV_SAMPLE_DEPTH_QSOUND_ADPCM: // QSound ADPCM
      setSampleCount((samples+1)&(~1));
      break;
    case DIV_SAMPLE_DEPTH_ADPCM_A: // ADPCM-A
      setSampleCount((samples+1)&(~1));
      break;
    case DIV_SAMPLE_DEPTH_ADPCM_B: // ADPCM-B
      setSampleCount((samples+1)&(~1));
      break;
    case DIV_SAMPLE_DEPTH_ADPCM_K: // K05 ADPCM
      setSampleCount((samples+1)&(~1));
      break;
    case DIV_SAMPLE_DEPTH_BRR: // BRR
      setSampleCount(16*(lengthBRR/9));
      break;
    case DIV_SAMPLE_DEPTH_VOX: // VOX
      setSampleCount((samples+1)&(~1));
      break;
    default:
      break;
  }
  render(formatMask|(1U<<newDepth));
}

#define RESAMPLE_BEGIN \
  if (samples<1) return true; \
  int finalCount=(double)samples*(tRate/sRate); \
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
  if (loopStart>=0) loopStart=(double)loopStart*(tRate/sRate); \
  if (loopEnd>=0) loopEnd=(double)loopEnd*(tRate/sRate); \
  centerRate=(int)((double)centerRate*(tRate/sRate)); \
  rate=(int)((double)rate*(tRate/sRate)); \
  samples=finalCount; \
  if (depth==DIV_SAMPLE_DEPTH_16BIT) { \
    delete[] oldData16; \
  } else if (depth==DIV_SAMPLE_DEPTH_8BIT) { \
    delete[] oldData8; \
  }

bool DivSample::resampleNone(double sRate, double tRate) {
  RESAMPLE_BEGIN;

  if (depth==DIV_SAMPLE_DEPTH_16BIT) {
    for (int i=0; i<finalCount; i++) {
      unsigned int pos=(unsigned int)((double)i*(sRate/tRate));
      if (pos>=samples) {
        data16[i]=0;
      } else {
        data16[i]=oldData16[pos];
      }
    }
  } else if (depth==DIV_SAMPLE_DEPTH_8BIT) {
    for (int i=0; i<finalCount; i++) {
      unsigned int pos=(unsigned int)((double)i*(sRate/tRate));
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

bool DivSample::resampleLinear(double sRate, double tRate) {
  RESAMPLE_BEGIN;

  double posFrac=0;
  unsigned int posInt=0;
  double factor=sRate/tRate;

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

bool DivSample::resampleCubic(double sRate, double tRate) {
  RESAMPLE_BEGIN;

  double posFrac=0;
  unsigned int posInt=0;
  double factor=sRate/tRate;
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

bool DivSample::resampleBlep(double sRate, double tRate) {
  RESAMPLE_BEGIN;

  double posFrac=0;
  unsigned int posInt=0;
  double factor=tRate/sRate;
  float* sincITable=DivFilterTables::getSincIntegralTable();

  float* floatData=new float[finalCount];
  memset(floatData,0,finalCount*sizeof(float));

  if (depth==DIV_SAMPLE_DEPTH_16BIT) {
    memset(data16,0,finalCount*sizeof(short));
    for (int i=0; i<finalCount; i++) {
      if (posInt<samples) {
        data16[i]=oldData16[posInt];
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
            floatData[i-j]+=t1[j]*-delta;
          }
          if (i+j+1<finalCount) {
            floatData[i+j+1]+=t2[j]*delta;
          }
        }
      }
    }
    for (int i=0; i<finalCount; i++) {
      float result=floatData[i]+data16[i];
      if (result<-32768) result=-32768;
      if (result>32767) result=32767;
      data16[i]=round(result);
    }
  } else if (depth==DIV_SAMPLE_DEPTH_8BIT) {
    memset(data8,0,finalCount);
    for (int i=0; i<finalCount; i++) {
      if (posInt<samples) {
        data8[i]=oldData8[posInt];
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
            floatData[i-j]+=t1[j]*-delta;
          }
          if (i+j+1<finalCount) {
            floatData[i+j+1]+=t2[j]*delta;
          }
        }
      }
    }
    for (int i=0; i<finalCount; i++) {
      float result=floatData[i]+data8[i];
      if (result<-128) result=-128;
      if (result>127) result=127;
      data8[i]=round(result);
    }
  }
  delete[] floatData;

  RESAMPLE_END;
  return true;
}

bool DivSample::resampleSinc(double sRate, double tRate) {
  RESAMPLE_BEGIN;

  double posFrac=0;
  unsigned int posInt=0;
  double factor=sRate/tRate;
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

bool DivSample::resample(double sRate, double tRate, int filter) {
  if (depth!=DIV_SAMPLE_DEPTH_8BIT && depth!=DIV_SAMPLE_DEPTH_16BIT) return false;
  switch (filter) {
    case DIV_RESAMPLE_NONE:
      return resampleNone(sRate,tRate);
      break;
    case DIV_RESAMPLE_LINEAR:
      return resampleLinear(sRate,tRate);
      break;
    case DIV_RESAMPLE_CUBIC:
      return resampleCubic(sRate,tRate);
      break;
    case DIV_RESAMPLE_BLEP:
      return resampleBlep(sRate,tRate);
      break;
    case DIV_RESAMPLE_SINC:
      return resampleSinc(sRate,tRate);
      break;
    case DIV_RESAMPLE_BEST:
      if (tRate>sRate) {
        return resampleSinc(sRate,tRate);
      } else {
        return resampleBlep(sRate,tRate);
      }
      break;
  }
  return false;
}

#define NOT_IN_FORMAT(x) (depth!=x && formatMask&(1U<<(unsigned int)x))

union IntFloat {
  unsigned int i;
  float f;
};

const short c219Table[256]={
  0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 480,
  512, 576, 640, 704, 768, 832, 896, 960, 1024, 1152, 1280, 1408, 1536, 1664, 1792, 1920,
  2048, 2176, 2304, 2432, 2560, 2688, 2816, 2944, 3072, 3200, 3328, 3456, 3584, 3712, 3840, 3968,
  4096, 4352, 4608, 4864, 5120, 5376, 5632, 5888, 6144, 6400, 6656, 6912, 7168, 7424, 7680, 7936,
  8192, 8448, 8704, 8960, 9216, 9472, 9728, 9984, 10240, 10496, 10752, 11008, 11264, 11520, 11776, 12032,
  12288, 12544, 12800, 13056, 13312, 13568, 13824, 14080, 14336, 14592, 14848, 15104, 15360, 15616, 15872, 16128,
  16384, 16640, 16896, 17152, 17408, 17920, 18432, 18944, 19456, 19968, 20480, 20992, 21504, 22016, 22528, 23040,
  23552, 24064, 24576, 25088, 25600, 26112, 26624, 27136, 27648, 28160, 28672, 29184, 29696, 30208, 30720, 31232,
  -32, -64, -96, -128, -160, -192, -224, -256, -288, -320, -352, -384, -416, -448, -480, -512,
  -544, -608, -672, -736, -800, -864, -928, -992, -1056, -1184, -1312, -1440, -1568, -1696, -1824, -1952,
  -2080, -2208, -2336, -2464, -2592, -2720, -2848, -2976, -3104, -3232, -3360, -3488, -3616, -3744, -3872, -4000,
  -4128, -4384, -4640, -4896, -5152, -5408, -5664, -5920, -6176, -6432, -6688, -6944, -7200, -7456, -7712, -7968,
  -8224, -8480, -8736, -8992, -9248, -9504, -9760, -10016, -10272, -10528, -10784, -11040, -11296, -11552, -11808, -12064,
  -12320, -12576, -12832, -13088, -13344, -13600, -13856, -14112, -14368, -14624, -14880, -15136, -15392, -15648, -15904, -16160,
  -16416, -16672, -16928, -17184, -17440, -17952, -18464, -18976, -19488, -20000, -20512, -21024, -21536, -22048, -22560, -23072,
  -23584, -24096, -24608, -25120, -25632, -26144, -26656, -27168, -27680, -28192, -28704, -29216, -29728, -30240, -30752, -31264
};

unsigned char c219HighBitPos[16]={
  0, 0, 0, 0, 0, 0, 0, 0, 0, 16, 24, 24, 48, 48, 48, 48
};

unsigned char c219ShiftToVal[16]={
  5, 5, 5, 5, 5, 5, 5, 5, 5,  6,  7,  7,  8,  8,  8,  8
};

signed char adpcmKTable[16]={
  0, 1, 2, 4, 8, 16, 32, 64, -128, -64, -32, -16, -8, -4, -2, -1
};

void DivSample::render(unsigned int formatMask) {
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
      case DIV_SAMPLE_DEPTH_ADPCM_K: { // K05 ADPCM
        signed char s=0;
        for (unsigned int i=0; i<samples; i++) {
          unsigned char nibble=dataK[i>>1];
          if (i&1) { // TODO: is this right?
            nibble>>=4;
          } else {
            nibble&=15;
          }
          s+=adpcmKTable[nibble];
          data16[i]=s<<8;
        }
        break;
      }
      case DIV_SAMPLE_DEPTH_8BIT: // 8-bit PCM
        for (unsigned int i=0; i<samples; i++) {
          data16[i]=data8[i]<<8;
        }
        break;
      case DIV_SAMPLE_DEPTH_BRR: // BRR
        brrDecode(dataBRR,data16,lengthBRR,brrEmphasis);
        break;
      case DIV_SAMPLE_DEPTH_VOX: // VOX
        oki_decode(dataVOX,data16,samples);
        break;
      case DIV_SAMPLE_DEPTH_MULAW: // 8-bit µ-law PCM
        for (unsigned int i=0; i<samples; i++) {
          IntFloat s;
          s.i=(dataMuLaw[i]^0xff);
          s.i=0x3f800000+(((s.i<<24)&0x80000000)|((s.i&0x7f)<<19));
          data16[i]=(short)(s.f*128.0f);
        }
        break;
      case DIV_SAMPLE_DEPTH_C219: // 8-bit C219 "μ-law" PCM
        for (unsigned int i=0; i<samples; i++) {
          data16[i]=c219Table[dataC219[i]&0x7f];
          if (dataC219[i]&0x80) data16[i]=-data16[i];
        }
        break;
      case DIV_SAMPLE_DEPTH_IMA_ADPCM: // IMA ADPCM
        if (adpcm_decode_block(data16,dataIMA,lengthIMA,samples)==0) logE("oh crap!");
        break;
      case DIV_SAMPLE_DEPTH_12BIT: // 12-bit PCM (MultiPCM)
        for (unsigned int i=0, j=0; i<samples; i+=2, j+=3) {
          data16[i+0]=(data12[j+0]<<8)|(data12[j+1]&0xf0);
          if (i+1<samples) {
            data16[i+1]=(data12[j+2]<<8)|((data12[j+1]<<4)&0xf0);
          }
        }
        break;
      default:
        return;
    }
  }

  // step 2: render to other formats
  if (NOT_IN_FORMAT(DIV_SAMPLE_DEPTH_1BIT)) { // 1-bit
    if (!initInternal(DIV_SAMPLE_DEPTH_1BIT,samples)) return;
    for (unsigned int i=0; i<samples; i++) {
      if (data16[i]>0) {
        data1[i>>3]|=1<<(i&7);
      }
    }
  }
  if (NOT_IN_FORMAT(DIV_SAMPLE_DEPTH_1BIT_DPCM)) { // DPCM
    if (!initInternal(DIV_SAMPLE_DEPTH_1BIT_DPCM,samples)) return;
    int accum=63;
    int next=63;
    
    for (unsigned int i=0; (i<samples && (i>>3)<lengthDPCM); i++) {
      next=((unsigned short)(data16[i]^0x8000))>>9;
      if (next>accum) {
        dataDPCM[i>>3]|=1<<(i&7);
        accum++;
      } else {
        dataDPCM[i>>3]&=~(1<<(i&7));
        accum--;
      }
      if (accum<0) accum=0;
      if (accum>127) accum=127;
    }
  }
  if (NOT_IN_FORMAT(DIV_SAMPLE_DEPTH_YMZ_ADPCM)) { // YMZ ADPCM
    if (!initInternal(DIV_SAMPLE_DEPTH_YMZ_ADPCM,samples)) return;
    ymz_encode(data16,dataZ,(samples+7)&(~0x7));
  }
  if (NOT_IN_FORMAT(DIV_SAMPLE_DEPTH_QSOUND_ADPCM)) { // QSound ADPCM
    if (!initInternal(DIV_SAMPLE_DEPTH_QSOUND_ADPCM,samples)) return;
    bs_encode(data16,dataQSoundA,samples);
  }
  // TODO: pad to 256.
  if (NOT_IN_FORMAT(DIV_SAMPLE_DEPTH_ADPCM_A)) { // ADPCM-A
    if (!initInternal(DIV_SAMPLE_DEPTH_ADPCM_A,samples)) return;
    yma_encode(data16,dataA,(samples+511)&(~0x1ff));
  }
  if (NOT_IN_FORMAT(DIV_SAMPLE_DEPTH_ADPCM_B)) { // ADPCM-B
    if (!initInternal(DIV_SAMPLE_DEPTH_ADPCM_B,samples)) return;
    ymb_encode(data16,dataB,(samples+511)&(~0x1ff));
  }
  if (NOT_IN_FORMAT(DIV_SAMPLE_DEPTH_ADPCM_K)) { // K05 ADPCM
    if (!initInternal(DIV_SAMPLE_DEPTH_ADPCM_K,samples)) return;
    signed char accum=0;
    unsigned char out=0;
    for (unsigned int i=0; i<samples; i++) {
      signed char target=data16[i]>>8;
      short delta=target-accum;
      unsigned char next=0;

      if (delta!=0) {
        int b=bsr((delta>=0)?delta:-delta);
        if (delta>=0) {
          if (b>7) b=7;
          next=b&15;

          // test previous
          if (next>1) {
            const signed char t1=accum+adpcmKTable[next];
            const signed char t2=accum+adpcmKTable[next-1];
            const signed char d1=((t1-target)<0)?(target-t1):(t1-target);
            const signed char d2=((t2-target)<0)?(target-t2):(t2-target);

            if (d2<d1) next--;
          }
        } else {
          if (b>8) b=8;
          next=(16-b)&15;

          // test next
          if (next<15) {
            const signed char t1=accum+adpcmKTable[next];
            const signed char t2=accum+adpcmKTable[next+1];
            const signed char d1=((t1-target)<0)?(target-t1):(t1-target);
            const signed char d2=((t2-target)<0)?(target-t2):(t2-target);

            if (d2<d1) next++;
          }
        }

        /*if (accum+adpcmKTable[next]>=128 || accum+adpcmKTable[next]<-128) {
          if (delta>=0) {
            next--;
          } else {
            next++;
            if (next>15) next=15;
          }
        }*/
      }

      out>>=4;
      out|=next<<4;
      accum+=adpcmKTable[next];

      if (i&1) {
        dataK[i>>1]=out;
        out=0;
      }
    }
  }
  if (NOT_IN_FORMAT(DIV_SAMPLE_DEPTH_8BIT)) { // 8-bit PCM
    if (!initInternal(DIV_SAMPLE_DEPTH_8BIT,samples)) return;
    if (dither) {
      unsigned short lfsr=0x6438;
      unsigned short lfsr1=0x1283;
      signed char errorLast=0;
      signed char errorCur=0;
      for (unsigned int i=0; i<samples; i++) {
        signed char val=CLAMP(data16[i]+128,-32768,32767)>>8;
        errorLast=errorCur;
        errorCur=(val<<8)-data16[i];
        data8[i]=CLAMP(val-((((errorLast+errorCur)>>1)+(lfsr&0xff))>>8),-128,127);
        lfsr=(lfsr<<1)|(((lfsr>>1)^(lfsr>>2)^(lfsr>>4)^(lfsr>>15))&1);
        lfsr1=(lfsr1<<1)|(((lfsr1>>1)^(lfsr1>>2)^(lfsr1>>4)^(lfsr1>>15))&1);
      }
    } else {
      for (unsigned int i=0; i<samples; i++) {
        data8[i]=data16[i]>>8;
      }
    }
  }
  if (NOT_IN_FORMAT(DIV_SAMPLE_DEPTH_BRR)) { // BRR
    int sampleCount=loop?loopEnd:samples;
    if (!initInternal(DIV_SAMPLE_DEPTH_BRR,sampleCount)) return;
    brrEncode(data16,dataBRR,sampleCount,loop?loopStart:-1,brrEmphasis,brrNoFilter);
  }
  if (NOT_IN_FORMAT(DIV_SAMPLE_DEPTH_VOX)) { // VOX
    if (!initInternal(DIV_SAMPLE_DEPTH_VOX,samples)) return;
    oki_encode(data16,dataVOX,samples);
  }
  if (NOT_IN_FORMAT(DIV_SAMPLE_DEPTH_MULAW)) { // µ-law
    if (!initInternal(DIV_SAMPLE_DEPTH_MULAW,samples)) return;
    for (unsigned int i=0; i<samples; i++) {
      IntFloat s;
      s.f=data16[i];
      s.i&=0x7fffffff;
      if (s.f>32639.0f) s.f=32639.0f;
      s.f/=128.0f;
      s.f+=1.0f;
      s.i-=0x3f800000;
      dataMuLaw[i]=(((data16[i]<0)?0x80:0)|(s.i&0x03f80000)>>19)^0xff;
    }
  }
  if (NOT_IN_FORMAT(DIV_SAMPLE_DEPTH_C219)) { // C219
    if (!initInternal(DIV_SAMPLE_DEPTH_C219,samples)) return;
    for (unsigned int i=0; i<samples; i++) {
      short s=data16[i];
      unsigned char x=0;
      bool negate=s&0x8000;
      if (negate) {
        s^=0xffff;
      }
      if (s==0) {
        x=0;
      } else if (s>17152) { // 100+
        x=((s-17152)>>9)+100;
      } else {
        int b=bsr(s)-1;
        x=((s-(c219Table[c219HighBitPos[b]]))>>c219ShiftToVal[b])+c219HighBitPos[b];
      }
      if (x>127) x=127;
      dataC219[i]=x|(negate?0x80:0);
    }
  }
  if (NOT_IN_FORMAT(DIV_SAMPLE_DEPTH_IMA_ADPCM)) { // IMA ADPCM
    if (!initInternal(DIV_SAMPLE_DEPTH_IMA_ADPCM,samples)) return;
    int delta[2];
    delta[0]=0;
    delta[1]=0;

    void* codec=adpcm_create_context(1,1,NOISE_SHAPING_OFF,delta);
    if (codec==NULL) {
      logE("oh no IMA encoder could not be created!");
    } else {
      size_t whyPointer=0;
      adpcm_encode_block(codec,dataIMA,&whyPointer,data16,samples);
      if (whyPointer!=lengthIMA) logW("IMA length mismatch! %d -> %d!=%d",(int)samples,(int)whyPointer,(int)lengthIMA);

      adpcm_free_context(codec);
    }
  }
  if (NOT_IN_FORMAT(DIV_SAMPLE_DEPTH_12BIT)) { // 12-bit PCM (MultiPCM)
    if (!initInternal(DIV_SAMPLE_DEPTH_12BIT,samples)) return;
    for (unsigned int i=0, j=0; i<samples; i+=2, j+=3) {
      data12[j+0]=data16[i+0]>>8;
      data12[j+1]=((data16[i+0]>>4)&0xf)|(i+1<samples?(data16[i+1]>>4)&0xf:0);
      if (i+1<samples) {
        data12[j+2]=data16[i+1]>>8;
      }
    }
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
    case DIV_SAMPLE_DEPTH_ADPCM_K:
      return dataK;
    case DIV_SAMPLE_DEPTH_8BIT:
      return data8;
    case DIV_SAMPLE_DEPTH_BRR:
      return dataBRR;
    case DIV_SAMPLE_DEPTH_VOX:
      return dataVOX;
    case DIV_SAMPLE_DEPTH_MULAW:
      return dataMuLaw;
    case DIV_SAMPLE_DEPTH_C219:
      return dataC219;
    case DIV_SAMPLE_DEPTH_IMA_ADPCM:
      return dataIMA;
    case DIV_SAMPLE_DEPTH_12BIT:
      return data12;
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
    case DIV_SAMPLE_DEPTH_ADPCM_K:
      return lengthK;
    case DIV_SAMPLE_DEPTH_8BIT:
      return length8;
    case DIV_SAMPLE_DEPTH_BRR:
      return lengthBRR;
    case DIV_SAMPLE_DEPTH_VOX:
      return lengthVOX;
    case DIV_SAMPLE_DEPTH_MULAW:
      return lengthMuLaw;
    case DIV_SAMPLE_DEPTH_C219:
      return lengthC219;
    case DIV_SAMPLE_DEPTH_IMA_ADPCM:
      return lengthIMA;
    case DIV_SAMPLE_DEPTH_12BIT:
      return length12;
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
    h=new DivSampleHistory(duplicate,getCurBufLen(),samples,depth,rate,centerRate,loopStart,loopEnd,loop,brrEmphasis,brrNoFilter,dither,loopMode);
  } else {
    h=new DivSampleHistory(depth,rate,centerRate,loopStart,loopEnd,loop,brrEmphasis,brrNoFilter,dither,loopMode);
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
  loopEnd=h->loopEnd; \
  loop=h->loop; \
  brrEmphasis=h->brrEmphasis; \
  brrNoFilter=h->brrNoFilter; \
  dither=h->dither; \
  loopMode=h->loopMode;


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
  if (dataK) delete[] dataK;
  if (dataBRR) delete[] dataBRR;
  if (dataVOX) delete[] dataVOX;
  if (dataMuLaw) delete[] dataMuLaw;
  if (dataC219) delete[] dataC219;
  if (dataIMA) delete[] dataIMA;
  if (data12) delete[] data12;
}
