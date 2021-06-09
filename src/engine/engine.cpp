#include "engine.h"
#include "safeReader.h"
#include "../ta-log.h"
#include "../audio/sdl.h"
#ifdef HAVE_JACK
#include "../audio/jack.h"
#endif
#include "platform/genesis.h"
#include "platform/genesisext.h"
#include "platform/sms.h"
#include "platform/gb.h"
#include "platform/pce.h"
#include "platform/dummy.h"
#include <math.h>
#include <zlib.h>

void process(void* u, float** in, float** out, int inChans, int outChans, unsigned int size) {
  ((DivEngine*)u)->nextBuf(in,out,inChans,outChans,size);
}

#define DIV_READ_SIZE 131072
#define DIV_DMF_MAGIC ".DelekDefleMask."

struct InflateBlock {
  unsigned char* buf;
  size_t len;
  size_t blockSize;
  InflateBlock(size_t s) {
    buf=new unsigned char[s];
    len=s;
    blockSize=0;
  }
  ~InflateBlock() {
    delete[] buf;
    len=0;
  }
};

DivSystem systemFromFile(unsigned char val) {
  switch (val) {
    case 0x01:
      return DIV_SYSTEM_YMU759;
    case 0x02:
      return DIV_SYSTEM_GENESIS;
    case 0x03:
      return DIV_SYSTEM_SMS;
    case 0x04:
      return DIV_SYSTEM_GB;
    case 0x05:
      return DIV_SYSTEM_PCE;
    case 0x06:
      return DIV_SYSTEM_NES;
    case 0x07:
      return DIV_SYSTEM_C64_8580;
    case 0x08:
      return DIV_SYSTEM_ARCADE;
    case 0x09:
      return DIV_SYSTEM_YM2610;
    case 0x42:
      return DIV_SYSTEM_GENESIS_EXT;
    case 0x47:
      return DIV_SYSTEM_C64_6581;
    case 0x49:
      return DIV_SYSTEM_YM2610_EXT;
  }
  return DIV_SYSTEM_NULL;
}

int getChannelCount(DivSystem sys) {
  switch (sys) {
    case DIV_SYSTEM_NULL:
      return 0;
    case DIV_SYSTEM_YMU759:
      return 17;
    case DIV_SYSTEM_GENESIS:
      return 10;
    case DIV_SYSTEM_SMS:
    case DIV_SYSTEM_GB:
      return 4;
    case DIV_SYSTEM_PCE:
      return 6;
    case DIV_SYSTEM_NES:
      return 5;
    case DIV_SYSTEM_C64_6581:
    case DIV_SYSTEM_C64_8580:
      return 3;
    case DIV_SYSTEM_ARCADE:
    case DIV_SYSTEM_GENESIS_EXT:
    case DIV_SYSTEM_YM2610:
      return 13;      
    case DIV_SYSTEM_YM2610_EXT:
      return 16;
  }
  return 0;
}

bool DivEngine::load(void* f, size_t slen) {
  unsigned char* file;
  size_t len;
  if (slen<16) {
    logE("too small!");
    return false;
  }
  if (memcmp(f,DIV_DMF_MAGIC,16)!=0) {
    logD("loading as zlib...\n");
    // try zlib
    z_stream zl;
    memset(&zl,0,sizeof(z_stream));

    zl.avail_in=slen;
    zl.next_in=(Bytef*)f;
    zl.zalloc=NULL;
    zl.zfree=NULL;
    zl.opaque=NULL;

    int nextErr;
    nextErr=inflateInit(&zl);
    if (nextErr!=Z_OK) {
      if (zl.msg==NULL) {
        logE("zlib error: unknown! %d\n",nextErr);
      } else {
        logE("zlib error: %s\n",zl.msg);
      }
      return false;
    }

    std::vector<InflateBlock*> blocks;
    while (true) {
      InflateBlock* ib=new InflateBlock(DIV_READ_SIZE);
      zl.next_out=ib->buf;
      zl.avail_out=ib->len;

      nextErr=inflate(&zl,Z_SYNC_FLUSH);
      if (nextErr!=Z_OK && nextErr!=Z_STREAM_END) {
        if (zl.msg==NULL) {
          logE("zlib error: unknown error! %d\n",nextErr);
        } else {
          logE("zlib inflate: %s\n",zl.msg);
        }
        for (InflateBlock* i: blocks) delete i;
        blocks.clear();
        delete ib;
        return false;
      }
      ib->blockSize=ib->len-zl.avail_out;
      blocks.push_back(ib);
      if (nextErr==Z_STREAM_END) {
        break;
      }
    }
    nextErr=inflateEnd(&zl);
    if (nextErr!=Z_OK) {
      if (zl.msg==NULL) {
        logE("zlib end error: unknown error! %d\n",nextErr);
      } else {
        logE("zlib end: %s\n",zl.msg);
      }
      for (InflateBlock* i: blocks) delete i;
      blocks.clear();
      return false;
    }

    size_t finalSize=0;
    size_t curSeek=0;
    for (InflateBlock* i: blocks) {
      finalSize+=i->blockSize;
    }
    if (finalSize<1) {
      logE("compressed too small!\n");
      for (InflateBlock* i: blocks) delete i;
      blocks.clear();
      return false;
    }
    file=new unsigned char[finalSize];
    for (InflateBlock* i: blocks) {
      memcpy(&file[curSeek],i->buf,i->blockSize);
      curSeek+=i->blockSize;
      delete i;
    }
    blocks.clear();
    len=finalSize;
  } else {
    logD("loading as uncompressed\n");
    file=(unsigned char*)f;
    len=slen;
  }
  if (memcmp(file,DIV_DMF_MAGIC,16)!=0) {
    logE("not a valid module!\n");
    return false;
  }
  SafeReader reader=SafeReader(file,len);
  try {
    DivSong ds;

    ds.nullWave.len=32;
    for (int i=0; i<32; i++) {
      ds.nullWave.data[i]=15;
    }

    if (!reader.seek(16,SEEK_SET)) {
      logE("premature end of file!");
      return false;
    }
    ds.version=reader.readC();
    logI("module version %d (0x%.2x)\n",ds.version,ds.version);
    unsigned char sys=0;
    if (ds.version<0x09) {
      // V E R S I O N  -> 3 <-
      // AWESOME
      ds.system=DIV_SYSTEM_YMU759;
    } else {
      sys=reader.readC();
      ds.system=systemFromFile(sys);
    }
    if (ds.system==DIV_SYSTEM_NULL) {
      logE("invalid system 0x%.2x!",sys);
      return false;
    }
    
    if (ds.system==DIV_SYSTEM_YMU759 && ds.version<0x10) { // TODO
      ds.vendor=reader.readString((unsigned char)reader.readC());
      ds.carrier=reader.readString((unsigned char)reader.readC());
      ds.category=reader.readString((unsigned char)reader.readC());
      ds.name=reader.readString((unsigned char)reader.readC());
      ds.author=reader.readString((unsigned char)reader.readC());
      ds.writer=reader.readString((unsigned char)reader.readC());
      ds.composer=reader.readString((unsigned char)reader.readC());
      ds.arranger=reader.readString((unsigned char)reader.readC());
      ds.copyright=reader.readString((unsigned char)reader.readC());
      ds.manGroup=reader.readString((unsigned char)reader.readC());
      ds.manInfo=reader.readString((unsigned char)reader.readC());
      ds.createdDate=reader.readString((unsigned char)reader.readC());
      ds.revisionDate=reader.readString((unsigned char)reader.readC());
      logI("%s by %s\n",ds.name.c_str(),ds.author.c_str());
      logI("has YMU-specific data:\n");
      logI("- carrier: %s\n",ds.carrier.c_str());
      logI("- category: %s\n",ds.category.c_str());
      logI("- vendor: %s\n",ds.vendor.c_str());
      logI("- writer: %s\n",ds.writer.c_str());
      logI("- composer: %s\n",ds.composer.c_str());
      logI("- arranger: %s\n",ds.arranger.c_str());
      logI("- copyright: %s\n",ds.copyright.c_str());
      logI("- management group: %s\n",ds.manGroup.c_str());
      logI("- management info: %s\n",ds.manInfo.c_str());
      logI("- created on: %s\n",ds.createdDate.c_str());
      logI("- revision date: %s\n",ds.revisionDate.c_str());
    } else {
      ds.name=reader.readString((unsigned char)reader.readC());
      ds.author=reader.readString((unsigned char)reader.readC());
      logI("%s by %s\n",ds.name.c_str(),ds.author.c_str());
    }

    logI("reading module data...\n");
    if (ds.version>0x0c) {
      ds.hilightA=reader.readC();
      ds.hilightB=reader.readC();
    }

    ds.timeBase=reader.readC();
    ds.speed1=reader.readC();
    if (ds.version>0x03) {
      ds.speed2=reader.readC();
      ds.pal=reader.readC();
      ds.customTempo=reader.readC();
    } else {
      ds.speed2=ds.speed1;
    }
    if (ds.version>0x0a) {
      String hz=reader.readString(3);
      if (ds.customTempo) {
        ds.hz=std::stoi(hz);
      }
    }
    // TODO
    if (ds.version>0x17) {
      ds.patLen=reader.readI();
    } else {
      ds.patLen=(unsigned char)reader.readC();
    }
    ds.ordersLen=(unsigned char)reader.readC();

    if (ds.version<20 && ds.version>3) {
      ds.arpLen=reader.readC();
    } else {
      ds.arpLen=1;
    }

    logI("reading pattern matrix (%d)...\n",ds.ordersLen);
    for (int i=0; i<getChannelCount(ds.system); i++) {
      for (int j=0; j<ds.ordersLen; j++) {
        ds.orders.ord[i][j]=reader.readC();
        if (ds.orders.ord[i][j]>ds.ordersLen) {
          logW("pattern %d exceeds order count %d!\n",ds.orders.ord[i][j],ds.ordersLen);
        }
      }
    }

    if (ds.version>0x03) {
      ds.insLen=reader.readC();
    } else {
      ds.insLen=16;
    }
    logI("reading instruments (%d)...\n",ds.insLen);
    for (int i=0; i<ds.insLen; i++) {
      DivInstrument* ins=new DivInstrument;
      if (ds.version>0x03) {
        ins->name=reader.readString((unsigned char)reader.readC());
      }
      logD("%d name: %s\n",i,ins->name.c_str());
      if (ds.version<0x0b) {
        // instruments in ancient versions were all FM or STD.
        ins->mode=1;
      } else {
        ins->mode=reader.readC();
      }

      if (ins->mode) { // FM
        if (ds.system!=DIV_SYSTEM_GENESIS &&
            ds.system!=DIV_SYSTEM_GENESIS_EXT &&
            ds.system!=DIV_SYSTEM_ARCADE &&
            ds.system!=DIV_SYSTEM_YM2610 &&
            ds.system!=DIV_SYSTEM_YM2610_EXT &&
            ds.system!=DIV_SYSTEM_YMU759) {
          logE("FM instrument in non-FM system. oopsie?\n");
          return false;
        }
        ins->fm.alg=reader.readC();
        if (ds.version<0x13) {
          reader.readC();
        }
        ins->fm.fb=reader.readC();
        if (ds.version<0x13) {
          reader.readC();
        }
        ins->fm.fms=reader.readC();
        if (ds.version<0x13) {
          reader.readC();
          ins->fm.ops=2+reader.readC()*2;
          if (ds.system!=DIV_SYSTEM_YMU759) ins->fm.ops=4;
        } else {
          ins->fm.ops=4;
        }
        if (ins->fm.ops!=2 && ins->fm.ops!=4) {
          logE("invalid op count %d. did we read it wrong?\n",ins->fm.ops);
          return false;
        }
        ins->fm.ams=reader.readC();

        for (int j=0; j<ins->fm.ops; j++) {
          ins->fm.op[j].am=reader.readC();
          ins->fm.op[j].ar=reader.readC();
          if (ds.version<0x13) {
            ins->fm.op[j].dam=reader.readC();
          }
          ins->fm.op[j].dr=reader.readC();
          if (ds.version<0x13) {
            ins->fm.op[j].dvb=reader.readC();
            ins->fm.op[j].egt=reader.readC();
            ins->fm.op[j].ksl=reader.readC();
            if (ds.version<0x11) { // TODO: don't know when did this change
              ins->fm.op[j].ksr=reader.readC();
            }
          }
          ins->fm.op[j].mult=reader.readC();
          ins->fm.op[j].rr=reader.readC();
          ins->fm.op[j].sl=reader.readC();
          if (ds.version<0x13) {
            ins->fm.op[j].sus=reader.readC();
          }
          ins->fm.op[j].tl=reader.readC();
          if (ds.version<0x13) {
            ins->fm.op[j].vib=reader.readC();
            ins->fm.op[j].ws=reader.readC();
          } else {
            ins->fm.op[j].dt2=reader.readC();
          }
          if (ds.version>0x03) {
            ins->fm.op[j].rs=reader.readC();
            ins->fm.op[j].dt=reader.readC();
            ins->fm.op[j].d2r=reader.readC();
            ins->fm.op[j].ssgEnv=reader.readC();
          }

          logD("OP%d: AM %d AR %d DAM %d DR %d DVB %d EGT %d KSL %d MULT %d RR %d SL %d SUS %d TL %d VIB %d WS %d RS %d DT %d D2R %d SSG-EG %d\n",j,
               ins->fm.op[j].am,
               ins->fm.op[j].ar,
               ins->fm.op[j].dam,
               ins->fm.op[j].dr,
               ins->fm.op[j].dvb,
               ins->fm.op[j].egt,
               ins->fm.op[j].ksl,
               ins->fm.op[j].mult,
               ins->fm.op[j].rr,
               ins->fm.op[j].sl,
               ins->fm.op[j].sus,
               ins->fm.op[j].tl,
               ins->fm.op[j].vib,
               ins->fm.op[j].ws,
               ins->fm.op[j].rs,
               ins->fm.op[j].dt,
               ins->fm.op[j].d2r,
               ins->fm.op[j].ssgEnv
               );
        }
      } else { // STD
        if (ds.system!=DIV_SYSTEM_GB || ds.version<0x12) {
          ins->std.volMacroLen=reader.readC();
          for (int j=0; j<ins->std.volMacroLen; j++) {
            if (ds.version<0x0e) {
              ins->std.volMacro[j]=reader.readC();
            } else {
              ins->std.volMacro[j]=reader.readI();
            }
          }
          if (ins->std.volMacroLen>0) {
            ins->std.volMacroLoop=reader.readC();
          }
        }

        ins->std.arpMacroLen=reader.readC();
        for (int j=0; j<ins->std.arpMacroLen; j++) {
          if (ds.version<0x0e) {
            ins->std.arpMacro[j]=reader.readC();
          } else {
            ins->std.arpMacro[j]=reader.readI();
          }
        }
        if (ins->std.arpMacroLen>0) {
          ins->std.arpMacroLoop=reader.readC();
        }
        if (ds.version>0x0f) { // TODO
          ins->std.arpMacroMode=reader.readC();
        }

        ins->std.dutyMacroLen=reader.readC();
        for (int j=0; j<ins->std.dutyMacroLen; j++) {
          if (ds.version<0x0e) {
            ins->std.dutyMacro[j]=reader.readC();
          } else {
            ins->std.dutyMacro[j]=reader.readI();
          }
        }
        if (ins->std.dutyMacroLen>0) {
          ins->std.dutyMacroLoop=reader.readC();
        }

        ins->std.waveMacroLen=reader.readC();
        for (int j=0; j<ins->std.waveMacroLen; j++) {
          if (ds.version<0x0e) {
            ins->std.waveMacro[j]=reader.readC();
          } else {
            ins->std.waveMacro[j]=reader.readI();
          }
        }
        if (ins->std.waveMacroLen>0) {
          ins->std.waveMacroLoop=reader.readC();
        }

        if (ds.system==DIV_SYSTEM_C64_6581 || ds.system==DIV_SYSTEM_C64_8580) {
          ins->c64.triOn=reader.readC();
          ins->c64.sawOn=reader.readC();
          ins->c64.pulseOn=reader.readC();
          ins->c64.noiseOn=reader.readC();

          ins->c64.a=reader.readC();
          ins->c64.d=reader.readC();
          ins->c64.s=reader.readC();
          ins->c64.r=reader.readC();

          ins->c64.duty=reader.readC();

          ins->c64.ringMod=reader.readC();
          ins->c64.oscSync=reader.readC();
          ins->c64.toFilter=reader.readC();
          if (ds.version<0x11) {
            ins->c64.volIsCutoff=reader.readI();
          } else {
            ins->c64.volIsCutoff=reader.readC();
          }
          ins->c64.initFilter=reader.readC();

          ins->c64.res=reader.readC();
          ins->c64.cut=reader.readC();
          ins->c64.hp=reader.readC();
          ins->c64.lp=reader.readC();
          ins->c64.bp=reader.readC();
          ins->c64.ch3off=reader.readC();
        }

        if (ds.system==DIV_SYSTEM_GB && ds.version>0x11) {
          ins->gb.envVol=reader.readC();
          ins->gb.envDir=reader.readC();
          ins->gb.envLen=reader.readC();
          ins->gb.soundLen=reader.readC();

          logD("GB data: vol %d dir %d len %d sl %d\n",ins->gb.envVol,ins->gb.envDir,ins->gb.envLen,ins->gb.soundLen);
        }
      }

      ds.ins.push_back(ins);
    }

    if (ds.version>0x0b) {
      ds.waveLen=(unsigned char)reader.readC();
      logI("reading wavetables (%d)...\n",ds.waveLen);
      for (int i=0; i<ds.waveLen; i++) {
        DivWavetable* wave=new DivWavetable;
        wave->len=(unsigned char)reader.readI();
        if (wave->len>32) {
          logE("invalid wave length %d. are we doing something wrong?\n",wave->len);
          return false;
        }
        logD("%d length %d\n",i,wave->len);
        for (int j=0; j<wave->len; j++) {
          if (ds.version<0x0e) {
            wave->data[j]=reader.readC();
          } else {
            wave->data[j]=reader.readI();
          }
        }
        ds.wave.push_back(wave);
      }
    }

    logI("reading patterns (%d channels, %d orders)...\n",getChannelCount(ds.system),ds.ordersLen);
    for (int i=0; i<getChannelCount(ds.system); i++) {
      DivChannelData* chan=new DivChannelData;
      if (ds.version<0x0a) {
        chan->effectRows=1;
      } else {
        chan->effectRows=reader.readC();
      }
      logD("%d fx rows: %d\n",i,chan->effectRows);
      if (chan->effectRows>4 || chan->effectRows<1) {
        logE("invalid effect row count %d. are you sure everything is ok?\n",chan->effectRows);
        return false;
      }
      for (int j=0; j<ds.ordersLen; j++) {
        DivPattern* pat=new DivPattern;
        for (int k=0; k<ds.patLen; k++) {
          // note
          pat->data[k][0]=reader.readS();
          // octave
          pat->data[k][1]=reader.readS();
          if (ds.system==DIV_SYSTEM_SMS && ds.version<0x0e && pat->data[k][1]>0) {
            // apparently it was up one octave before
            pat->data[k][1]--;
          } else if (ds.system==DIV_SYSTEM_GENESIS && ds.version<0x0e && pat->data[k][1]>0 && i>5) {
            // ditto
            pat->data[k][1]--;
          }
          // volume
          pat->data[k][3]=reader.readS();
          if (ds.version<0x0a) {
            // back then volume was stored as 00-ff instead of 00-7f/0-f
            if (i>5) {
              pat->data[k][3]>>=4;
            } else {
              pat->data[k][3]>>=1;
            }
          }
          for (int l=0; l<chan->effectRows; l++) {
            // effect
            pat->data[k][4+(l<<1)]=reader.readS();
            pat->data[k][5+(l<<1)]=reader.readS();
          }
          // instrument
          pat->data[k][2]=reader.readS();
        }
        chan->data.push_back(pat);
      }
      ds.pat.push_back(chan);
    }

    ds.sampleLen=reader.readC();
    logI("reading samples (%d)...\n",ds.sampleLen);
    if (ds.version<0x0b && ds.sampleLen>0) { // TODO what is this for?
      reader.readC();
    }
    for (int i=0; i<ds.sampleLen; i++) {
      DivSample* sample=new DivSample;
      sample->length=reader.readI();
      if (sample->length<0) {
        logE("invalid sample length %d. are we doing something wrong?\n",sample->length);
        return false;
      }
      if (ds.version>0x16) {
        sample->name=reader.readString((unsigned char)reader.readC());
      } else {
        sample->name="";
      }
      logD("%d name %s (%d)\n",i,sample->name.c_str(),sample->length);
      if (ds.version<0x0b) {
        sample->rate=4;
        sample->pitch=0;
        sample->vol=0;
      } else {
        sample->rate=reader.readC();
        sample->pitch=reader.readC();
        sample->vol=reader.readC();
      }
      if (ds.version>0x15) {
        sample->depth=reader.readC();
      } else {
        sample->depth=16;
      }
      if (sample->length>0) {
        if (ds.version<0x0b) {
          sample->data=new short[1+(sample->length/2)];
          reader.read(sample->data,sample->length);
          sample->length/=2;
        } else {
          sample->data=new short[sample->length];
          reader.read(sample->data,sample->length*2);
        }
      }
      ds.sample.push_back(sample);
    }

    if (reader.tell()<reader.size()) {
      logW("premature end of song (we are at %x, but size is %x)\n",reader.tell(),reader.size());
    }

    song=ds;
    chans=getChannelCount(song.system);
    renderSamples();
  } catch (EndOfFileException e) {
    logE("premature end of file!\n");
    return false;
  }
  return true;
}

static double samplePitches[11]={
  0.1666666666, 0.2, 0.25, 0.333333333, 0.5,
  1,
  2, 3, 4, 5, 6
};

void DivEngine::renderSamples() {
  for (int i=0; i<song.sampleLen; i++) {
    DivSample* s=song.sample[i];
    if (s->rendLength!=0) delete[] s->rendData;
    s->rendLength=(double)s->length/samplePitches[s->pitch];
    s->rendData=new short[s->rendLength];
    int k=0;
    float mult=(float)(s->vol+100)/150.0f;
    for (double j=0; j<s->length; j+=samplePitches[s->pitch]) {
      if (k>=s->rendLength) {
        break;
      }
      if (s->depth==8) {
        float next=(float)(s->data[(unsigned int)j]-0x80)*mult;
        s->rendData[k++]=fmin(fmax(next,-128),127);
      } else {
        float next=(float)s->data[(unsigned int)j]*mult;
        s->rendData[k++]=fmin(fmax(next,-32768),32767);
      }
    }
  }
}

DivInstrument* DivEngine::getIns(int index) {
  if (index<0 || index>=song.insLen) return &song.nullIns;
  return song.ins[index];
}

DivWavetable* DivEngine::getWave(int index) {
  if (index<0 || index>=song.waveLen) {
    if (song.waveLen>0) {
      return song.wave[0];
    } else {
      return &song.nullWave;
    }
  }
  return song.wave[index];
}

void DivEngine::play() {
  
}

void DivEngine::setAudio(DivAudioEngines which) {
  audioEngine=which;
}

void DivEngine::setView(DivStatusView which) {
  view=which;
}

bool DivEngine::init() {
  switch (audioEngine) {
    case DIV_AUDIO_JACK:
#ifndef HAVE_JACK
      logE("Furnace was not compiled with JACK support!\n");
      return false;
#else
      output=new TAAudioJACK;
#endif
      break;
    case DIV_AUDIO_SDL:
      output=new TAAudioSDL;
      break;
    default:
      logE("invalid audio engine!\n");
      return false;
  }
  want.bufsize=1024;
  want.rate=44100;
  want.fragments=2;
  want.inChans=0;
  want.outChans=2;
  want.outFormat=TA_AUDIO_FORMAT_F32;
  want.name="Furnace";

  output->setCallback(process,this);

  logI("initializing audio.\n");
  if (!output->init(want,got)) {
    logE("error while initializing audio!\n");
    return false;
  }

  bb[0]=blip_new(32768);
  if (bb[0]==NULL) {
    logE("not enough memory!\n");
    return false;
  }

  bb[1]=blip_new(32768);
  if (bb[1]==NULL) {
    logE("not enough memory!\n");
    return false;
  }
  
  bbOut[0]=new short[got.bufsize];
  bbOut[1]=new short[got.bufsize];

  for (int i=0; i<64; i++) {
    vibTable[i]=127*sin(((double)i/64.0)*(2*M_PI));
  }

  switch (song.system) {
    case DIV_SYSTEM_GENESIS:
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
    default:
      logW("this system is not supported yet! using dummy platform.\n");
      dispatch=new DivPlatformDummy;
      break;
  }
  dispatch->init(this,getChannelCount(song.system),got.rate);

  blip_set_rates(bb[0],dispatch->rate,got.rate);
  blip_set_rates(bb[1],dispatch->rate,got.rate);

  for (int i=0; i<chans; i++) {
    chan[i].volMax=(dispatch->dispatch(DivCommand(DIV_CMD_GET_VOLMAX,i))<<8)|0xff;
    chan[i].volume=chan[i].volMax;
  }

  if (!output->setRun(true)) {
    logE("error while activating!\n");
    return false;
  }
  return true;
}
