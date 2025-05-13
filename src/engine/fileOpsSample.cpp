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
#include "../fileutils.h"
#ifdef HAVE_SNDFILE
#include "sfWrapper.h"
#endif

std::vector<DivSample*> DivEngine::sampleFromFile(const char* path) {
  std::vector<DivSample*> ret;

  if (song.sample.size()>=256) {
    lastError="too many samples!";
    return ret;
  }
  BUSY_BEGIN;
  warnings="";

  const char* pathRedux=strrchr(path,DIR_SEPARATOR);
  if (pathRedux==NULL) {
    pathRedux=path;
  } else {
    pathRedux++;
  }
  String stripPath;
  const char* pathReduxEnd=strrchr(pathRedux,'.');
  if (pathReduxEnd==NULL) {
    stripPath=pathRedux;
  } else {
    for (const char* i=pathRedux; i!=pathReduxEnd && (*i); i++) {
      stripPath+=*i;
    }
  }

  const char* ext=strrchr(path,'.');
  if (ext!=NULL) {
    String extS;
    for (; *ext; ext++) {
      char i=*ext;
      if (i>='A' && i<='Z') {
        i+='a'-'A';
      }
      extS+=i;
    }

    if(extS == ".pps" || extS == ".ppc" || extS == ".pvi" ||
      extS == ".pdx" || extS == ".pzi" || extS == ".p86" ||
      extS == ".p") //sample banks!
    {
      String stripPath;
      const char* pathReduxEnd=strrchr(pathRedux,'.');
      if (pathReduxEnd==NULL) {
        stripPath=pathRedux;
      } else {
        for (const char* i=pathRedux; i!=pathReduxEnd && (*i); i++) {
          stripPath+=*i;
        }
      }

      FILE* f=ps_fopen(path,"rb");
      if (f==NULL) {
        lastError=strerror(errno);
        return ret;
      }
      unsigned char* buf;
      ssize_t len;
      if (fseek(f,0,SEEK_END)!=0) {
        lastError=strerror(errno);
        fclose(f);
        return ret;
      }
      len=ftell(f);
      if (len<0) {
        lastError=strerror(errno);
        fclose(f);
        return ret;
      }
      if (len==(SIZE_MAX>>1)) {
        lastError=strerror(errno);
        fclose(f);
        return ret;
      }
      if (len==0) {
        lastError=strerror(errno);
        fclose(f);
        return ret;
      }
      if (fseek(f,0,SEEK_SET)!=0) {
        lastError=strerror(errno);
        fclose(f);
        return ret;
      }
      buf=new unsigned char[len];
      if (fread(buf,1,len,f)!=(size_t)len) {
        logW("did not read entire sample bank file buffer!");
        lastError=_("did not read entire sample bank file!");
        delete[] buf;
        fclose(f);
        return ret;
      }
      fclose(f);

      SafeReader reader = SafeReader(buf,len);

      if(extS == ".pps")
      {
        loadPPS(reader,ret,stripPath);
      }
      if(extS == ".ppc")
      {
        loadPPC(reader,ret,stripPath);
      }
      if(extS == ".pvi")
      {
        loadPVI(reader,ret,stripPath);
      }
      if(extS == ".pdx")
      {
        loadPDX(reader,ret,stripPath);
      }
      if(extS == ".pzi")
      {
        loadPZI(reader,ret,stripPath);
      }
      if(extS == ".p86")
      {
        loadP86(reader,ret,stripPath);
      }
      if(extS == ".p")
      {
        loadP(reader,ret,stripPath);
      }

      if((int)ret.size() > 0)
      {
        int counter = 0;

        for(DivSample* s: ret)
        {
          s->name = fmt::sprintf("%s sample %d", stripPath, counter);
          counter++;
        }
      }

      delete[] buf; //done with buffer
      BUSY_END;
      return ret;
    }

    if (extS==".dmc" || extS==".brr") { // read as .dmc or .brr
      size_t len=0;
      DivSample* sample=new DivSample;
      sample->name=stripPath;

      FILE* f=ps_fopen(path,"rb");
      if (f==NULL) {
        BUSY_END;
        lastError=fmt::sprintf("could not open file! (%s)",strerror(errno));
        delete sample;
        return ret;
      }

      if (fseek(f,0,SEEK_END)<0) {
        fclose(f);
        BUSY_END;
        lastError=fmt::sprintf("could not get file length! (%s)",strerror(errno));
        delete sample;
        return ret;
      }

      len=ftell(f);

      if (len==0) {
        fclose(f);
        BUSY_END;
        lastError="file is empty!";
        delete sample;
        return ret;
      }

      if (len==(SIZE_MAX>>1)) {
        fclose(f);
        BUSY_END;
        lastError="file is invalid!";
        delete sample;
        return ret;
      }

      if (fseek(f,0,SEEK_SET)<0) {
        fclose(f);
        BUSY_END;
        lastError=fmt::sprintf("could not seek to beginning of file! (%s)",strerror(errno));
        delete sample;
        return ret;
      }

      if (extS==".dmc") {
        sample->rate=33144;
        sample->centerRate=33144;
        sample->depth=DIV_SAMPLE_DEPTH_1BIT_DPCM;
        sample->init(len*8);
      } else if (extS==".brr") {
        sample->rate=32000;
        sample->centerRate=32000;
        sample->depth=DIV_SAMPLE_DEPTH_BRR;
        sample->init(16*(len/9));
      } else {
        fclose(f);
        BUSY_END;
        lastError="wait... is that right? no I don't think so...";
        delete sample;
        return ret;
      }

      unsigned char* dataBuf=sample->dataDPCM;
      if (extS==".brr") {
        dataBuf=sample->dataBRR;
        if ((len%9)==2) {
          // read loop position
          unsigned short loopPos=0;
          logD("BRR file has loop position");
          if (fread(&loopPos,1,2,f)!=2) {
            logW("could not read loop position! %s",strerror(errno));
          } else {
#ifdef TA_BIG_ENDIAN
            loopPos=(loopPos>>8)|(loopPos<<8);
#endif
            sample->loopStart=16*(loopPos/9);
            sample->loopEnd=sample->samples;
            sample->loop=true;
            sample->loopMode=DIV_SAMPLE_LOOP_FORWARD;
          }
          len-=2;
          if (len==0) {
            fclose(f);
            BUSY_END;
            lastError="BRR sample is empty!";
            delete sample;
            return ret;
          }
        } else if ((len%9)!=0) {
          fclose(f);
          BUSY_END;
          lastError="possibly corrupt BRR sample!";
          delete sample;
          return ret;
        }
      }

      if (fread(dataBuf,1,len,f)==0) {
        fclose(f);
        BUSY_END;
        lastError=fmt::sprintf("could not read file! (%s)",strerror(errno));
        delete sample;
        return ret;
      }
      fclose(f);
      BUSY_END;
      ret.push_back(sample);
      return ret;
    }
  }

#ifndef HAVE_SNDFILE
  lastError="Furnace was not compiled with libsndfile!";
  return ret;
#else
  SF_INFO si;
  SFWrapper sfWrap;
  memset(&si,0,sizeof(SF_INFO));
  SNDFILE* f=sfWrap.doOpen(path,SFM_READ,&si);
  if (f==NULL) {
    BUSY_END;
    int err=sf_error(NULL);
    if (err==SF_ERR_SYSTEM) {
      lastError=fmt::sprintf("could not open file! (%s %s)",sf_error_number(err),strerror(errno));
    } else {
      lastError=fmt::sprintf("could not open file! (%s)\nif this is raw sample data, you may import it by right-clicking the Load Sample icon and selecting \"import raw\".",sf_error_number(err));
    }
    return ret;
  }
  if (si.frames>16777215) {
    lastError="this sample is too big! max sample size is 16777215.";
    sfWrap.doClose();
    BUSY_END;
    return ret;
  }
  void* buf=NULL;
  sf_count_t sampleLen=sizeof(short);
  if ((si.format&SF_FORMAT_SUBMASK)==SF_FORMAT_PCM_U8) {
    logD("sample is 8-bit unsigned");
    buf=new unsigned char[si.channels*si.frames];
    sampleLen=sizeof(unsigned char);
  } else if ((si.format&SF_FORMAT_SUBMASK)==SF_FORMAT_FLOAT)  {
    logD("sample is 32-bit float");
    buf=new float[si.channels*si.frames];
    sampleLen=sizeof(float);
  } else {
    logD("sample is 16-bit signed");
    buf=new short[si.channels*si.frames];
    sampleLen=sizeof(short);
  }
  if ((si.format&SF_FORMAT_SUBMASK)==SF_FORMAT_PCM_U8 || (si.format&SF_FORMAT_SUBMASK)==SF_FORMAT_FLOAT) {
    if (sf_read_raw(f,buf,si.frames*si.channels*sampleLen)!=(si.frames*si.channels*sampleLen)) {
      logW("sample read size mismatch!");
    }
  } else {
    if (sf_read_short(f,(short*)buf,si.frames*si.channels)!=(si.frames*si.channels)) {
      logW("sample read size mismatch!");
    }
  }
  DivSample* sample=new DivSample;
  int sampleCount=(int)song.sample.size();
  sample->name=stripPath;

  int index=0;
  if ((si.format&SF_FORMAT_SUBMASK)==SF_FORMAT_PCM_U8) {
    sample->depth=DIV_SAMPLE_DEPTH_8BIT;
  } else {
    sample->depth=DIV_SAMPLE_DEPTH_16BIT;
  }
  sample->init(si.frames);
  if ((si.format&SF_FORMAT_SUBMASK)==SF_FORMAT_PCM_U8) {
    for (int i=0; i<si.frames*si.channels; i+=si.channels) {
      int averaged=0;
      for (int j=0; j<si.channels; j++) {
        averaged+=((int)((unsigned char*)buf)[i+j])-128;
      }
      averaged/=si.channels;
      sample->data8[index++]=averaged;
    }
    delete[] (unsigned char*)buf;
  } else if ((si.format&SF_FORMAT_SUBMASK)==SF_FORMAT_FLOAT)  {
    for (int i=0; i<si.frames*si.channels; i+=si.channels) {
      float averaged=0.0f;
      for (int j=0; j<si.channels; j++) {
        averaged+=((float*)buf)[i+j];
      }
      averaged/=si.channels;
      averaged*=32767.0;
      if (averaged<-32768.0) averaged=-32768.0;
      if (averaged>32767.0) averaged=32767.0;
      sample->data16[index++]=averaged;
    }
    delete[] (float*)buf;
  } else {
    for (int i=0; i<si.frames*si.channels; i+=si.channels) {
      int averaged=0;
      for (int j=0; j<si.channels; j++) {
        averaged+=((short*)buf)[i+j];
      }
      averaged/=si.channels;
      sample->data16[index++]=averaged;
    }
    delete[] (short*)buf;
  }

  sample->rate=si.samplerate;
  if (sample->rate<4000) sample->rate=4000;
  if (sample->rate>96000) sample->rate=96000;
  sample->centerRate=si.samplerate;

  SF_INSTRUMENT inst;
  if (sf_command(f, SFC_GET_INSTRUMENT, &inst, sizeof(inst)) == SF_TRUE)
  {
    // There's no documentation on libsndfile detune range, but the code
    // implies -50..50. Yet when loading a file you can get a >50 value.
    // disabled for now
    /*
    if(inst.detune > 50)
      inst.detune = inst.detune - 100;
    short pitch = ((0x3c-inst.basenote)*100) + inst.detune;
    sample->centerRate=si.samplerate*pow(2.0,pitch/(12.0 * 100.0));
    */
    if(inst.loop_count && inst.loops[0].mode >= SF_LOOP_FORWARD)
    {
      sample->loop=true;
      sample->loopMode=(DivSampleLoopMode)(inst.loops[0].mode-SF_LOOP_FORWARD);
      sample->loopStart=inst.loops[0].start;
      sample->loopEnd=inst.loops[0].end;
      if(inst.loops[0].end < (unsigned int)sampleCount)
        sampleCount=inst.loops[0].end;
    }
    else
      sample->loop=false;
  }

  if (sample->centerRate<100) sample->centerRate=100;
  if (sample->centerRate>384000) sample->centerRate=384000;
  sfWrap.doClose();
  BUSY_END;
  ret.push_back(sample);
  return ret;
#endif
}

DivSample* DivEngine::sampleFromFileRaw(const char* path, DivSampleDepth depth, int channels, bool bigEndian, bool unsign, bool swapNibbles, int rate) {
  if (song.sample.size()>=256) {
    lastError="too many samples!";
    return NULL;
  }
  if (channels<1) {
    channels=1;
  }
  if (depth!=DIV_SAMPLE_DEPTH_8BIT && depth!=DIV_SAMPLE_DEPTH_16BIT) {
    if (channels!=1) {
      channels=1;
    }
  }
  BUSY_BEGIN;
  warnings="";

  const char* pathRedux=strrchr(path,DIR_SEPARATOR);
  if (pathRedux==NULL) {
    pathRedux=path;
  } else {
    pathRedux++;
  }
  String stripPath;
  const char* pathReduxEnd=strrchr(pathRedux,'.');
  if (pathReduxEnd==NULL) {
    stripPath=pathRedux;
  } else {
    for (const char* i=pathRedux; i!=pathReduxEnd && (*i); i++) {
      stripPath+=*i;
    }
  }

  size_t len=0;
  size_t lenDivided=0;
  DivSample* sample=new DivSample;
  sample->name=stripPath;

  FILE* f=ps_fopen(path,"rb");
  if (f==NULL) {
    BUSY_END;
    lastError=fmt::sprintf("could not open file! (%s)",strerror(errno));
    delete sample;
    return NULL;
  }

  if (fseek(f,0,SEEK_END)<0) {
    fclose(f);
    BUSY_END;
    lastError=fmt::sprintf("could not get file length! (%s)",strerror(errno));
    delete sample;
    return NULL;
  }

  len=ftell(f);

  if (len==0) {
    fclose(f);
    BUSY_END;
    lastError="file is empty!";
    delete sample;
    return NULL;
  }

  if (len==(SIZE_MAX>>1)) {
    fclose(f);
    BUSY_END;
    lastError="file is invalid!";
    delete sample;
    return NULL;
  }

  if (fseek(f,0,SEEK_SET)<0) {
    fclose(f);
    BUSY_END;
    lastError=fmt::sprintf("could not seek to beginning of file! (%s)",strerror(errno));
    delete sample;
    return NULL;
  }

  lenDivided=len/channels;

  unsigned int samples=lenDivided;
  switch (depth) {
    case DIV_SAMPLE_DEPTH_1BIT:
    case DIV_SAMPLE_DEPTH_1BIT_DPCM:
      samples=lenDivided*8;
      break;
    case DIV_SAMPLE_DEPTH_YMZ_ADPCM:
    case DIV_SAMPLE_DEPTH_QSOUND_ADPCM:
    case DIV_SAMPLE_DEPTH_ADPCM_A:
    case DIV_SAMPLE_DEPTH_ADPCM_B:
    case DIV_SAMPLE_DEPTH_ADPCM_K:
    case DIV_SAMPLE_DEPTH_VOX:
      samples=lenDivided*2;
      break;
    case DIV_SAMPLE_DEPTH_IMA_ADPCM:
      samples=(lenDivided-4)*2;
      break;
    case DIV_SAMPLE_DEPTH_8BIT:
    case DIV_SAMPLE_DEPTH_MULAW:
    case DIV_SAMPLE_DEPTH_C219:
      samples=lenDivided;
      break;
    case DIV_SAMPLE_DEPTH_BRR:
      samples=16*((lenDivided+8)/9);
      break;
    case DIV_SAMPLE_DEPTH_12BIT:
      samples=(2+(lenDivided*2))/3;
      break;
    case DIV_SAMPLE_DEPTH_16BIT:
      samples=(lenDivided+1)/2;
      break;
    default:
      break;
  }

  if (samples>16777215) {
    fclose(f);
    BUSY_END;
    lastError="this sample is too big! max sample size is 16777215.";
    delete sample;
    return NULL;
  }

  sample->rate=rate;
  sample->centerRate=rate;
  sample->depth=depth;
  sample->init(samples);

  unsigned char* buf=new unsigned char[len];
  if (fread(buf,1,len,f)==0) {
    fclose(f);
    BUSY_END;
    lastError=fmt::sprintf("could not read file! (%s)",strerror(errno));
    delete[] buf;
    delete sample;
    return NULL;
  }

  fclose(f);

  // import sample
  size_t pos=0;
  if (depth==DIV_SAMPLE_DEPTH_16BIT) {
    for (unsigned int i=0; i<samples; i++) {
      int accum=0;
      for (int j=0; j<channels; j++) {
        if (pos+1>=len) break;
        if (bigEndian) {
          accum+=(short)(((short)((buf[pos]<<8)|buf[pos+1]))^(unsign?0x8000:0));
        } else {
          accum+=(short)(((short)(buf[pos]|(buf[pos+1]<<8)))^(unsign?0x8000:0));
        }
        pos+=2;
      }
      accum/=channels;
      sample->data16[i]=accum;
    }
  } else if (depth==DIV_SAMPLE_DEPTH_8BIT) {
    for (unsigned int i=0; i<samples; i++) {
      int accum=0;
      for (int j=0; j<channels; j++) {
        if (pos>=len) break;
        accum+=(signed char)(buf[pos++]^(unsign?0x80:0));
      }
      accum/=channels;
      sample->data8[i]=accum;
    }
    if (bigEndian) {
      for (unsigned int i=0; (i+1)<samples; i+=2) {
        sample->data8[i]^=sample->data8[i^1];
        sample->data8[i^1]^=sample->data8[i];
        sample->data8[i]^=sample->data8[i^1];
      }
    }
  } else {
    memcpy(sample->getCurBuf(),buf,len);
  }
  delete[] buf;

  if (swapNibbles) {
    unsigned char* b=(unsigned char*)sample->getCurBuf();
    switch (depth) {
      case DIV_SAMPLE_DEPTH_1BIT:
      case DIV_SAMPLE_DEPTH_1BIT_DPCM:
        // reverse bit order
        for (unsigned int i=0; i<sample->getCurBufLen(); i++) {
          b[i]=(
            ((b[i]&128)?1:0)|
            ((b[i]&64)?2:0)|
            ((b[i]&32)?4:0)|
            ((b[i]&16)?8:0)|
            ((b[i]&8)?16:0)|
            ((b[i]&4)?32:0)|
            ((b[i]&2)?64:0)|
            ((b[i]&1)?128:0)
          );
        }
        break;
      case DIV_SAMPLE_DEPTH_YMZ_ADPCM:
      case DIV_SAMPLE_DEPTH_QSOUND_ADPCM:
      case DIV_SAMPLE_DEPTH_ADPCM_A:
      case DIV_SAMPLE_DEPTH_ADPCM_B:
      case DIV_SAMPLE_DEPTH_ADPCM_K:
      case DIV_SAMPLE_DEPTH_VOX:
        // swap nibbles
        for (unsigned int i=0; i<sample->getCurBufLen(); i++) {
          b[i]=(b[i]<<4)|(b[i]>>4);
        }
        break;
      case DIV_SAMPLE_DEPTH_MULAW:
        // Namco to G.711
        // Namco: smmmmxxx
        // G.711: sxxxmmmm (^0xff)
        for (unsigned int i=0; i<sample->getCurBufLen(); i++) {
          b[i]=(((b[i]&7)<<4)|(((b[i]>>3)&15)^((b[i]&0x80)?15:0))|(b[i]&0x80))^0xff;
        }
        break;
      default:
        break;
    }
  }

  BUSY_END;
  return sample;
}
