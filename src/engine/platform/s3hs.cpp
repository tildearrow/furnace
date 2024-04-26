#include "s3hs.h"
#include "../engine.h"
#include <stdio.h>
#include <math.h>


#define CHIP_FREQBASE 48000
#define rWrite(a,v) if (!skipRegisterWrites) {doWrite(a,v); if(a>=0x400000 && a<0x400400) {regPool[(a-0x400000)]=v;} if (dumpWrites) {addWrite(a,v);} }

unsigned int s3hs_chanaddrs_freq[12] = {0x400000,0x400040,0x400080,0x4000c0,0x400100,0x400140,0x400180,0x4001c0,0x400200,0x400230,0x400260,0x400290};
unsigned int s3hs_chanaddrs_volume[12] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x400202,0x400232,0x400262,0x400292};

void DivPlatformS3HS::doWrite(unsigned int addr, unsigned char data) {
  cpt->ram_poke(cpt->ram,(int)addr,(S3HS_sound::Byte)data);
}

void DivPlatformS3HS::updateWave(int ch) {
  if (ch>=4 && ch<=5)
  {
    for (int j=0;j<32;j++) {
      //rWrite(0x400210+32*(ch-4)+j,chan[ch].ws.output[j]);
    }
  }
}

void DivPlatformS3HS::acquire(short** buf, size_t len) {
  int chanOut, chanOutL, chanOutR;
  std::vector<std::vector<std::vector<int16_t>>> output = cpt->AudioCallBack(len);
  for (size_t i=0; i<len; i++) {
    int out = 0;
    int outL = 0;
    int outR = 0;
    for (unsigned char j=0; j<chans; j++) {
      if (true) {
        if (!isMuted[j]) {
          chanOut=(signed short)(std::max(output[0][j][i],output[1][j][i]));
          chanOutL=(signed short)((output[0][j][i]));
          chanOutR=(signed short)((output[1][j][i]));
          oscBuf[j]->data[oscBuf[j]->needle++]=(short)((double)chanOut/2);
          out+=(int)((double)chanOut/(double)(j>8&&chan[j].pcm?4:4));
          outL+=(int)((double)chanOutL/(double)(j>8&&chan[j].pcm?4:4));
          outR+=(int)((double)chanOutR/(double)(j>8&&chan[j].pcm?4:4));
        } else {
          oscBuf[j]->data[oscBuf[j]->needle++]=0;
        }
        chan[j].pos+=chan[j].freq;
      } else {
        oscBuf[j]->data[oscBuf[j]->needle++]=0;
      }
    }
    if (out<-32768) out=-32768;
    if (out>32767) out=32767;
    if (outL<-32768) outL=-32768;
    if (outL>32767) outL=32767;
    if (outR<-32768) outR=-32768;
    if (outR>32767) outR=32767;
    buf[0][i]=outL;
    buf[1][i]=outR;
  }
}

unsigned short DivPlatformS3HS::getPan(int ch) {
  return (chan[ch].pan>>4)<<8|(chan[ch].pan&0xf);
}

int DivPlatformS3HS::getOutputCount() {
  return 2;
}

void DivPlatformS3HS::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
}

void DivPlatformS3HS::tick(bool sysTick) {
  for (unsigned char i=0; i<chans; i++) {
    chan[i].std.next();
    
    if (chan[i].std.vol.had) {
      if(i>=8 && i<=11) {
      chan[i].outVol=(MIN(255,chan[i].std.vol.val)*(chan[i].vol))/(256);
      if (chan[i].outVol<0) chan[i].outVol=0;
      if (chan[i].outVol>255) chan[i].outVol=255;
      } else {
        chan[i].outVol=parent->getIns(chan[i].ins,DIV_INS_S3HS)->cpt.op1v;
      }
      if (chan[i].resVol!=chan[i].outVol) {
        chan[i].resVol=chan[i].outVol;
        if (!isMuted[i]) {
          chan[i].volumeChanged=true;
        }
      }
    }
    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        chan[i].baseFreq=NOTE_FREQUENCY(parent->calcArp(chan[i].note,chan[i].std.arp.val));
      }
      chan[i].freqChanged=true;
    }
    if (chan[i].std.wave.had) {
      if (chan[i].wave!=chan[i].std.wave.val || chan[i].ws.activeChanged()) {
        chan[i].wave=chan[i].std.wave.val;
        chan[i].ws.changeWave1(chan[i].wave);
        chan[i].waveUpdated=true;
        
      }
    }
    if (chan[i].std.pitch.had) {
      if (chan[i].std.pitch.mode) {
        chan[i].pitch+=chan[i].std.pitch.val;
        CLAMP_VAR(chan[i].pitch,-32768,32767);
      } else {
        chan[i].pitch=chan[i].std.pitch.val;
      }
      chan[i].freqChanged=true;
    }
    if (chan[i].std.duty.had) {
      if (i>=8 && i<=11) {
        rWrite(0x40008a+i-4,chan[i].std.duty.val*2);
      }
    }
    if (chan[i].std.ex1.had) {
      if (i>=8 && i<=11) {
        rWrite(0x40008c+i-4,chan[i].std.ex1.val);
      }
    }
    if (chan[i].waveUpdated) {
      if (i>=8 && i<=11)
      {
        for (int j=0;j<32;j++) {
        //rWrite(0x400090+32*(i-4)+j,chan[i].ws.output[j]);
        }
      }
      if (chan[i].active) {
        if (chan[i].ws.tick()) {
          updateWave(i);
        }
        chan[i].freqChanged=true;
      }
      chan[i].waveChanged=false;
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      //DivInstrument* ins=parent->getIns(chan[i].ins,DIV_INS_SU);
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,0,2,chan[i].pitch2,chipClock,CHIP_FREQBASE)/16;
      if (chan[i].pcm && i>=8 && i<=11) {
        DivSample* sample=parent->getSample(chan[i].sample);
        if (sample!=NULL) {
          double off=0.5;
          if (sample->centerRate<1) {
            off=0.5;
          } else {
            off=(double)sample->centerRate/(8363.0*2);
          }
          chan[i].freq=(double)chan[i].freq*off;
        }
      } else {
          chan[i].freqChanged=false;
          chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,0,2,chan[i].pitch2,chipClock,CHIP_FREQBASE)/16;
      }
      if (chan[i].freq<0) chan[i].freq=0;
      if (chan[i].freq>65535) chan[i].freq=65535;
      rWrite(s3hs_chanaddrs_freq[i],chan[i].freq>>8);
      rWrite(s3hs_chanaddrs_freq[i]+1,chan[i].freq&0xff);
    }
    if (chan[i].keyOn) {
      if (i>=8 && i<=11) {
        if (chan[i].pcm) {
          int sNum=chan[i].sample;
          DivSample* sample=parent->getSample(sNum);
          if (sample!=NULL && sNum>=0 && sNum<parent->song.sampleLen) {
            unsigned int sampleEnd=sampleOffSU[sNum]+(sample->getLoopEndPosition());
            unsigned int off=sampleOffSU[sNum]+chan[i].hasOffset;
            chan[i].hasOffset=0;
            if (sampleEnd>=getSampleMemCapacity(0)) sampleEnd=getSampleMemCapacity(0)-1;
            rWrite(0x400210+48*(i-8),(off>>16));
            rWrite(0x400211+48*(i-8),off>>8&0xff);
            rWrite(0x400212+48*(i-8),off&0xff);
            rWrite(0x400213+48*(i-8),(sampleEnd>>16));
            rWrite(0x400214+48*(i-8),sampleEnd>>8&0xff);
            rWrite(0x400215+48*(i-8),sampleEnd&0xff);
            if (sample->isLoopable()) {
              unsigned int sampleLoop=sampleOffSU[sNum]+sample->getLoopStartPosition();
              if (sampleLoop>=getSampleMemCapacity(0)) sampleLoop=getSampleMemCapacity(0)-1;
              rWrite(0x400216+48*(i-8),(sampleLoop>>16));
              rWrite(0x400217+48*(i-8),sampleLoop>>8&0xff);
              rWrite(0x400218+48*(i-8),sampleLoop&0xff);
              chan[i].pcmLoop=true;
            } else {
              chan[i].pcmLoop=false;
            }
          }
        }
      }
    }
    if (chan[i].pcm && i>=8 && i<=11) {
      //rWrite(0x40008a+i-4,4);
    }
    rWrite(s3hs_chanaddrs_volume[i],chan[i].outVol);
  }
}

void* DivPlatformS3HS::getChanState(int ch) {
  return &chan[ch];
}

DivDispatchOscBuffer* DivPlatformS3HS::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformS3HS::getRegisterPool() {
  return regPool;
}

int DivPlatformS3HS::getRegisterPoolSize() {
  return 704;
}

int DivPlatformS3HS::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON:
      {
      DivInstrument* ins = parent->getIns(chan[c.chan].ins,DIV_INS_S3HS);
      if (chan[c.chan].pcm && !(ins->type==DIV_INS_AMIGA || ins->amiga.useSample)) {
        chan[c.chan].pcm=(ins->type==DIV_INS_AMIGA || ins->amiga.useSample);
      }
      chan[c.chan].pcm=(ins->type==DIV_INS_AMIGA || ins->amiga.useSample);
      if (chan[c.chan].pcm) {
        if (c.value!=DIV_NOTE_NULL) {
          chan[c.chan].sample=ins->amiga.getSample(c.value);
          chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value);
          chan[c.chan].freqChanged=true;
        }
      } else {
        if (c.value!=DIV_NOTE_NULL) {
          chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value);
          chan[c.chan].freqChanged=true;
        }
      }
      chan[c.chan].macroInit(ins);
      if (c.chan<8)
      {
        cpt->resetGate(c.chan);
        rWrite(0x400002 + 64*c.chan,ins->s3hs.op2fu);
        rWrite(0x400003 + 64*c.chan,ins->s3hs.op2fl);
        rWrite(0x400004 + 64*c.chan,ins->s3hs.op3fu);
        rWrite(0x400005 + 64*c.chan,ins->s3hs.op3fl);
        rWrite(0x400006 + 64*c.chan,ins->s3hs.op4fu);
        rWrite(0x400007 + 64*c.chan,ins->s3hs.op4fl);
        rWrite(0x400008 + 64*c.chan,ins->s3hs.op5fu);
        rWrite(0x400009 + 64*c.chan,ins->s3hs.op5fl);
        rWrite(0x40000a + 64*c.chan,ins->s3hs.op6fu);
        rWrite(0x40000b + 64*c.chan,ins->s3hs.op6fl);
        rWrite(0x40000c + 64*c.chan,ins->s3hs.op7fu);
        rWrite(0x40000d + 64*c.chan,ins->s3hs.op7fl);
        rWrite(0x40000e + 64*c.chan,ins->s3hs.op8fu);
        rWrite(0x40000f + 64*c.chan,ins->s3hs.op8fl);
        rWrite(0x400010 + 64*c.chan,ins->s3hs.op1v);
        rWrite(0x400011 + 64*c.chan,ins->s3hs.op2v);
        rWrite(0x400012 + 64*c.chan,ins->s3hs.op3v);
        rWrite(0x400013 + 64*c.chan,ins->s3hs.op4v);
        rWrite(0x400014 + 64*c.chan,ins->s3hs.op5v);
        rWrite(0x400015 + 64*c.chan,ins->s3hs.op6v);
        rWrite(0x400016 + 64*c.chan,ins->s3hs.op7v);
        rWrite(0x400017 + 64*c.chan,ins->s3hs.op8v);
        rWrite(0x400018 + 64*c.chan,ins->s3hs.op1w*16+ins->s3hs.op2w);
        rWrite(0x400019 + 64*c.chan,ins->s3hs.op3w*16+ins->s3hs.op4w);
        rWrite(0x40001a + 64*c.chan,ins->s3hs.op5w*16+ins->s3hs.op6w);
        rWrite(0x40001b + 64*c.chan,ins->s3hs.op7w*16+ins->s3hs.op8w);
        rWrite(0x400020 + 64*c.chan,ins->s3hs.op1a);
        rWrite(0x400021 + 64*c.chan,ins->s3hs.op1d);
        rWrite(0x400022 + 64*c.chan,ins->s3hs.op1s);
        rWrite(0x400023 + 64*c.chan,ins->s3hs.op1r);
        rWrite(0x400024 + 64*c.chan,ins->s3hs.op2a);
        rWrite(0x400025 + 64*c.chan,ins->s3hs.op2d);
        rWrite(0x400026 + 64*c.chan,ins->s3hs.op2s);
        rWrite(0x400027 + 64*c.chan,ins->s3hs.op2r);
        rWrite(0x400028 + 64*c.chan,ins->s3hs.op3a);
        rWrite(0x400029 + 64*c.chan,ins->s3hs.op3d);
        rWrite(0x40002a + 64*c.chan,ins->s3hs.op3s);
        rWrite(0x40002b + 64*c.chan,ins->s3hs.op3r);
        rWrite(0x40002c + 64*c.chan,ins->s3hs.op4a);
        rWrite(0x40002d + 64*c.chan,ins->s3hs.op4d);
        rWrite(0x40002e + 64*c.chan,ins->s3hs.op4s);
        rWrite(0x40002f + 64*c.chan,ins->s3hs.op4r);
        rWrite(0x400030 + 64*c.chan,ins->s3hs.op5a);
        rWrite(0x400031 + 64*c.chan,ins->s3hs.op5d);
        rWrite(0x400032 + 64*c.chan,ins->s3hs.op5s);
        rWrite(0x400033 + 64*c.chan,ins->s3hs.op5r);
        rWrite(0x400034 + 64*c.chan,ins->s3hs.op6a);
        rWrite(0x400035 + 64*c.chan,ins->s3hs.op6d);
        rWrite(0x400036 + 64*c.chan,ins->s3hs.op6s);
        rWrite(0x400037 + 64*c.chan,ins->s3hs.op6r);
        rWrite(0x400038 + 64*c.chan,ins->s3hs.op7a);
        rWrite(0x400039 + 64*c.chan,ins->s3hs.op7d);
        rWrite(0x40003a + 64*c.chan,ins->s3hs.op7s);
        rWrite(0x40003b + 64*c.chan,ins->s3hs.op7r);
        rWrite(0x40003c + 64*c.chan,ins->s3hs.op8a);
        rWrite(0x40003d + 64*c.chan,ins->s3hs.op8d);
        rWrite(0x40003e + 64*c.chan,ins->s3hs.op8s);
        rWrite(0x40003f + 64*c.chan,ins->s3hs.op8r);
        rWrite(0x40001c + 64*c.chan,ins->s3hs.mode);

      } else {
      
        cpt->wtSync(c.chan-8);
      }
      if (chan[c.chan].insChanged) {
        if (!parent->song.brokenOutVol && !chan[c.chan].std.vol.will) {
          chan[c.chan].outVol=chan[c.chan].vol;
        }
        if (chan[c.chan].wave<0) {
          chan[c.chan].wave=0;
          chan[c.chan].ws.changeWave1(chan[c.chan].wave);
          chan[c.chan].waveUpdated=true;
        }
        
        chan[c.chan].insChanged=false;
      }
      chan[c.chan].ws.init(ins,32,255,chan[c.chan].insChanged);
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].amp=(signed char)255;
      if (c.chan<8)
      {
        rWrite(0x40001e + 64*c.chan,1);
      }  
      break;
      }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].active=false;
      chan[c.chan].keyOff=true;
      if (c.chan<8)
      {
        rWrite(0x40001e + 64*c.chan,0);
      }  
      break;
    case DIV_CMD_VOLUME:
      chan[c.chan].vol=c.value;
      //chan[c.chan].vol=255;
      if (chan[c.chan].vol>255) chan[c.chan].vol=255;
      break;
    case DIV_CMD_GET_VOLUME:
      return chan[c.chan].vol;
      break;
    case DIV_CMD_INSTRUMENT:
      if (chan[c.chan].ins!=c.value || c.value2==1) {
        chan[c.chan].ins=c.value;
        chan[c.chan].insChanged=true;
      }
      break;
    case DIV_CMD_PITCH:
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=NOTE_FREQUENCY(c.value2);
      bool return2=false;
      if (destFreq>chan[c.chan].baseFreq) {
        chan[c.chan].baseFreq+=c.value;
        if (chan[c.chan].baseFreq>=destFreq) {
          chan[c.chan].baseFreq=destFreq;
          return2=true;
        }
      } else {
        chan[c.chan].baseFreq-=c.value;
        if (chan[c.chan].baseFreq<=destFreq) {
          chan[c.chan].baseFreq=destFreq;
          return2=true;
        }
      }
      chan[c.chan].freqChanged=true;
      if (return2) return 2;
      break;
    }
    case DIV_CMD_LEGATO:
      chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value);
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 255;
      break;
    case DIV_CMD_WAVE:
      if (chan[c.chan].wave!=c.value) {
        chan[c.chan].wave=c.value;
        chan[c.chan].ws.changeWave1(chan[c.chan].wave);
        chan[c.chan].waveUpdated=true;
      }
    case DIV_CMD_PANNING: {
      int tmp=0;
      chan[c.chan].pan=(c.value>>4)*16+(c.value2>>4);
      rWrite((c.chan<8)?0x40001d+64*c.chan:0x400207+48*(c.chan-8),chan[c.chan].pan);
      break;
    }
  break;
    default:
      break;
  }
  return 1;
}

void DivPlatformS3HS::notifyWaveChange(int wave) {
  for (int i=8; i<12; i++) {
    if (chan[i].wave==wave) {
      chan[i].ws.changeWave1(wave);
    }
    chan[i].waveUpdated=true;
  }
}

void DivPlatformS3HS::notifyInsDeletion(void* ins) {
  for (int i=0; i<12; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

DivMacroInt* DivPlatformS3HS::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

void DivPlatformS3HS::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformS3HS::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

const void* DivPlatformS3HS::getSampleMem(int index) {
  return (index==0)?sampleMem:NULL;
}

size_t DivPlatformS3HS::getSampleMemCapacity(int index) {
  return (index==0)?((sampleMemSize)-((initIlSize&64)?((1+(initIlSize&63))<<7):0)):0;
}

size_t DivPlatformS3HS::getSampleMemUsage(int index) {
  return (index==0)?sampleMemLen:0;
}

const DivMemoryComposition* DivPlatformS3HS::getMemCompo(int index) {
  if (index!=0) return NULL;
  return &memCompo;
}

bool DivPlatformS3HS::isSampleLoaded(int index, int sample) {
  if (index!=0) return false;
  if (sample<0 || sample>255) return false;
  return sampleLoaded[sample];
}

void DivPlatformS3HS::renderSamples(int sysID) {
  memset(sampleMem,0,sampleMemSize);
  memset(sampleOffSU,0,256*sizeof(unsigned int));
  memset(sampleLoaded,0,256*sizeof(bool));

  memCompo=DivMemoryComposition();
  memCompo.name="Sample RAM";

  size_t memPos=0;
  for (int i=0; i<parent->song.sampleLen; i++) {
    DivSample* s=parent->song.sample[i];
    if (s->data8==NULL) continue;
    if (!s->renderOn[0][sysID]) {
      sampleOffSU[i]=0;
      continue;
    }
    
    int paddedLen=s->length8;
    if (memPos>=getSampleMemCapacity(0)) {
      logW("out of PCM memory for sample %d!",i);
      break;
    }
    if (memPos+paddedLen>=getSampleMemCapacity(0)) {
      memcpy(sampleMem+memPos,s->data8,getSampleMemCapacity(0)-memPos);
      logW("out of PCM memory for sample %d!",i);
    } else {
      memcpy(sampleMem+memPos,s->data8,paddedLen);
      sampleLoaded[i]=true;
    }
    sampleOffSU[i]=memPos;
    memCompo.entries.push_back(DivMemoryEntry(DIV_MEMORY_SAMPLE,"Sample",i,memPos,memPos+paddedLen));
    memPos+=paddedLen;
  }
  sampleMemLen=memPos;

  for (int i=0; i<sampleMemSize; i++) {
    cpt->ram_poke(cpt->ram,0x0+i,(S3HS_sound::Byte)((sampleMem[i]+128)&0xff));
  }

  memCompo.used=sampleMemLen;
  memCompo.capacity=sampleMemSize;
}

void DivPlatformS3HS::reset() {
  cpt->ram_boot(cpt->ram,cpt->vram);
  cpt->initSound();
  for (int i=0; i<chans; i++) {
    chan[i]=DivPlatformS3HS::Channel();
    chan[i].vol=0xff;
    chan[i].std.setEngine(parent);
    chan[i].ws.setEngine(parent);
    chan[i].ws.init(NULL,32,255,false);
  }
  for (int i=0; i<sampleMemSize; i++) {
    cpt->ram_poke(cpt->ram,0x0+i,(S3HS_sound::Byte)((sampleMem[i]+128)&0xff));
  }
}

int DivPlatformS3HS::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<channels; i++) {
    isMuted[i]=false;
    if (i<channels) {
      oscBuf[i]=new DivDispatchOscBuffer;
      oscBuf[i]->rate=48000;
      
    }
  }
  
  sampleMemSize=0x400000;// 4096KB (4194304 bytes)
  sampleMem=new unsigned char[sampleMemSize];
  memset(sampleMem,0,sampleMemSize); 
  rate=48000;
  chipClock=48000;
  chans=channels;
  cpt = new S3HS_sound();
  memset(regPool,0,704);
  reset();
  return channels;
}

void DivPlatformS3HS::quit() {
  for (int i=0; i<chans; i++) {
    delete oscBuf[i];
  }
  delete cpt;
}

DivPlatformS3HS::~DivPlatformS3HS() {
}
