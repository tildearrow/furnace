#include "cpt100.h"
#include "../engine.h"
#include <stdio.h>
#include <math.h>


#define CHIP_FREQBASE 48000
#define rWrite(a,v) if (!skipRegisterWrites) {doWrite(a,v); if(a>=0x10000 && a<0x100d0) {regPool[(a-0x10000)]=v;} if (dumpWrites) {addWrite(a,v);} }

unsigned int chanaddrs_freq[6] = {0x10000,0x10010,0x10020,0x10030,0x10084,0x10086};
unsigned int chanaddrs_volume[6] = {0x10009,0x10019,0x10029,0x10039,0x10088,0x10089};

void DivPlatformCPT100::doWrite(unsigned int addr, unsigned char data) {
  cpt->ram_poke(cpt->ram,(int)addr,(Cpt100_sound::Byte)data);
}

void DivPlatformCPT100::updateWave(int ch) {
  if (ch>=4 && ch<=5)
  {
    for (int j=0;j<32;j++) {
      rWrite(0x10090+32*(ch-4)+j,chan[ch].ws.output[j]);
    }
  }
}

void DivPlatformCPT100::acquire(short** buf, size_t len) {
  int chanOut;
  std::vector<std::vector<int16_t>> output = cpt->AudioCallBack(len);
  for (size_t i=0; i<len; i++) {
    int out=0;
    for (unsigned char j=0; j<chans; j++) {
      if (true) {
        if (!isMuted[j]) {
          chanOut=(signed short)(output[j][i]);
          oscBuf[j]->data[oscBuf[j]->needle++]=(short)((double)chanOut/2);
          out+=(int)((double)chanOut/(double)(j>4&&chan[j].pcm?1.5:5));
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
    buf[0][i]=out;
  }
}

void DivPlatformCPT100::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
}

void DivPlatformCPT100::tick(bool sysTick) {
  for (unsigned char i=0; i<chans; i++) {
    chan[i].std.next();
    
    if (chan[i].std.vol.had) {
      if(i>=4 && i<=5) {
      chan[i].outVol=(MIN(255,chan[i].std.vol.val)*(chan[i].vol))/(chan[i].pcm?16:256);
      if (chan[i].outVol<0) chan[i].outVol=0;
      if (chan[i].outVol>255) chan[i].outVol=255;
      } else {
        chan[i].outVol=parent->getIns(chan[i].ins,DIV_INS_CPT100)->cpt.op1v;
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
      if (i>=4 && i<=5) {
        rWrite(0x1008a+i-4,chan[i].std.duty.val*2);
      }
    }
    if (chan[i].std.ex1.had) {
      if (i>=4 && i<=5) {
        rWrite(0x1008c+i-4,chan[i].std.ex1.val);
      }
    }
    if (chan[i].waveUpdated) {
      if (i>=4 && i<=5)
      {
        for (int j=0;j<32;j++) {
        rWrite(0x10090+32*(i-4)+j,chan[i].ws.output[j]);
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
      if (chan[i].pcm) {
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
      rWrite(chanaddrs_freq[i],chan[i].freq>>8);
      rWrite(chanaddrs_freq[i]+1,chan[i].freq&0xff);
    }
    if (chan[i].keyOn) {
      if (i>=4 && i<=5) {
        if (chan[i].pcm) {
          int sNum=chan[i].sample;
          DivSample* sample=parent->getSample(sNum);
          if (sample!=NULL && sNum>=0 && sNum<parent->song.sampleLen) {
            unsigned int sampleEnd=sampleOffSU[sNum]+(sample->getLoopEndPosition());
            unsigned int off=sampleOffSU[sNum]+chan[i].hasOffset;
            chan[i].hasOffset=0;
            if (sampleEnd>=getSampleMemCapacity(0)) sampleEnd=getSampleMemCapacity(0)-1;
            rWrite(0x10090+32*(i-4),(off>>16)+2);
            rWrite(0x10091+32*(i-4),off>>8&0xff);
            rWrite(0x10092+32*(i-4),off&0xff);
            rWrite(0x10093+32*(i-4),(sampleEnd>>16)+2);
            rWrite(0x10094+32*(i-4),sampleEnd>>8&0xff);
            rWrite(0x10095+32*(i-4),sampleEnd&0xff);
            if (sample->isLoopable()) {
              unsigned int sampleLoop=sampleOffSU[sNum]+sample->getLoopStartPosition();
              if (sampleLoop>=getSampleMemCapacity(0)) sampleLoop=getSampleMemCapacity(0)-1;
              rWrite(0x10096+32*(i-4),(sampleLoop>>16)+2);
              rWrite(0x10097+32*(i-4),sampleLoop>>8&0xff);
              rWrite(0x10098+32*(i-4),sampleLoop&0xff);
              chan[i].pcmLoop=true;
            } else {
              chan[i].pcmLoop=false;
            }
          }
        }
      }
    }
    if (chan[i].pcm && i>=4 && i<=5) {
      rWrite(0x1008a+i-4,4);
    }
    rWrite(chanaddrs_volume[i],chan[i].outVol);
  }
}

void* DivPlatformCPT100::getChanState(int ch) {
  return &chan[ch];
}

DivDispatchOscBuffer* DivPlatformCPT100::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformCPT100::getRegisterPool() {
  return regPool;
}

int DivPlatformCPT100::getRegisterPoolSize() {
  return 208;
}

int DivPlatformCPT100::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON:
      {
      DivInstrument* ins = parent->getIns(chan[c.chan].ins,DIV_INS_CPT100);
      if (chan[c.chan].pcm && !(ins->amiga.useSample)) {
        chan[c.chan].pcm=(ins->amiga.useSample);
      }
      chan[c.chan].pcm=(ins->amiga.useSample);
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
      if (c.chan<4)
      {
        cpt->resetGate(c.chan);
        rWrite(0x10002 + 16*c.chan,ins->cpt.op2f*16);
        rWrite(0x10003 + 16*c.chan,ins->cpt.op3f*16);
        rWrite(0x10004 + 16*c.chan,ins->cpt.op4f*16);
        rWrite(0x10009 + 16*c.chan,ins->cpt.op1v);
        rWrite(0x1000a + 16*c.chan,ins->cpt.op2v);
        rWrite(0x1000b + 16*c.chan,ins->cpt.op3v);
        rWrite(0x1000c + 16*c.chan,ins->cpt.op4v);
        rWrite(0x10040 + 16*c.chan,ins->cpt.op1a);
        rWrite(0x10041 + 16*c.chan,ins->cpt.op1d);
        rWrite(0x10042 + 16*c.chan,ins->cpt.op1s);
        rWrite(0x10043 + 16*c.chan,ins->cpt.op1r);
        rWrite(0x10044 + 16*c.chan,ins->cpt.op2a);
        rWrite(0x10045 + 16*c.chan,ins->cpt.op2d);
        rWrite(0x10046 + 16*c.chan,ins->cpt.op2s);
        rWrite(0x10047 + 16*c.chan,ins->cpt.op2r);
        rWrite(0x10048 + 16*c.chan,ins->cpt.op3a);
        rWrite(0x10049 + 16*c.chan,ins->cpt.op3d);
        rWrite(0x1004a + 16*c.chan,ins->cpt.op3s);
        rWrite(0x1004b + 16*c.chan,ins->cpt.op3r);
        rWrite(0x1004c + 16*c.chan,ins->cpt.op4a);
        rWrite(0x1004d + 16*c.chan,ins->cpt.op4d);
        rWrite(0x1004e + 16*c.chan,ins->cpt.op4s);
        rWrite(0x1004f + 16*c.chan,ins->cpt.op4r);
      } else {
      
        cpt->wtSync(c.chan-4);
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
      rWrite(0x10080 + c.chan,1);
      break;
      }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].active=false;
      chan[c.chan].keyOff=true;
      rWrite(0x10080 + c.chan,0);
      break;
    case DIV_CMD_VOLUME:
      chan[c.chan].vol=c.value;
      //chan[c.chan].vol=255;
      if (chan[c.chan].vol>16) chan[c.chan].vol=16;
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
      return 16;
      break;
    case DIV_CMD_WAVE:
      if (chan[c.chan].wave!=c.value) {
        chan[c.chan].wave=c.value;
        chan[c.chan].ws.changeWave1(chan[c.chan].wave);
        chan[c.chan].waveUpdated=true;
      }
  break;
    default:
      break;
  }
  return 1;
}

void DivPlatformCPT100::notifyWaveChange(int wave) {
  for (int i=5; i<6; i++) {
    if (chan[i].wave==wave) {
      chan[i].ws.changeWave1(wave);
    }
    chan[i].waveUpdated=true;
  }
}

void DivPlatformCPT100::notifyInsDeletion(void* ins) {
  for (int i=0; i<6; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

DivMacroInt* DivPlatformCPT100::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

void DivPlatformCPT100::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformCPT100::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

const void* DivPlatformCPT100::getSampleMem(int index) {
  return (index==0)?sampleMem:NULL;
}

size_t DivPlatformCPT100::getSampleMemCapacity(int index) {
  return (index==0)?((sampleMemSize)-((initIlSize&64)?((1+(initIlSize&63))<<7):0)):0;
}

size_t DivPlatformCPT100::getSampleMemUsage(int index) {
  return (index==0)?sampleMemLen:0;
}

bool DivPlatformCPT100::isSampleLoaded(int index, int sample) {
  if (index!=0) return false;
  if (sample<0 || sample>255) return false;
  return sampleLoaded[sample];
}

void DivPlatformCPT100::renderSamples(int sysID) {
  memset(sampleMem,0,sampleMemSize);
  memset(sampleOffSU,0,256*sizeof(unsigned int));
  memset(sampleLoaded,0,256*sizeof(bool));

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
    memPos+=paddedLen;
  }
  sampleMemLen=memPos;

  for (int i=0; i<sampleMemSize; i++) {
    cpt->ram_poke(cpt->ram,0x20000+i,(Cpt100_sound::Byte)((sampleMem[i]+128)&0xff));
  }
}

void DivPlatformCPT100::reset() {
  cpt->ram_boot(cpt->ram,cpt->vram);
  cpt->initSound();
  for (int i=0; i<chans; i++) {
    chan[i]=DivPlatformCPT100::Channel();
    chan[i].vol=0xff;
    chan[i].std.setEngine(parent);
    chan[i].ws.setEngine(parent);
    chan[i].ws.init(NULL,32,255,false);
  }
}

int DivPlatformCPT100::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
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
  
  sampleMemSize=0x60000;// 384KB (393216 bytes)
  sampleMem=new unsigned char[sampleMemSize];
  memset(sampleMem,0,sampleMemSize); 
  rate=48000;
  chipClock=48000;
  chans=channels;
  cpt = new Cpt100_sound();
  memset(regPool,0,208);
  reset();
  return channels;
}

void DivPlatformCPT100::quit() {
  for (int i=0; i<chans; i++) {
    delete oscBuf[i];
  }
  delete cpt;
}

DivPlatformCPT100::~DivPlatformCPT100() {
}
