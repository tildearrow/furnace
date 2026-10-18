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

#include "br811.h"
#include "../engine.h"
#include <math.h>

// divisor de afinacao do BR811: frequencia = clock / (114 * (periodo + 1))
#define CHIP_DIVIDER 114
// o chip e emulado a clock/57 (duas amostras por passo de 114 ciclos)
#define RATE_DIVIDER 57
#define BR811_CLOCK 6420000

const char* regCheatSheetBR811[]={
  "CH1_Control", "00",
  "CH1_PeriodLow", "01",
  "CH1_PeriodHigh_Noise", "02",
  "CH2_Control", "03",
  "CH2_PeriodLow", "04",
  "CH2_PeriodHigh_Noise", "05",
  "CH3_Control", "06",
  "CH3_PeriodLow", "07",
  "CH3_PeriodHigh_Noise", "08",
  "CH4_Control", "09",
  "CH4_PeriodLow", "0A",
  "CH4_PeriodHigh_Noise", "0B",
  "CH5_Control", "0C",
  "CH5_PeriodLow", "0D",
  "CH5_PeriodHigh_Noise", "0E",
  "PCM_AddrLow", "0F",
  "PCM_AddrHigh", "10",
  "PCM_Length", "11",
  "PCM_Control", "12",
  NULL
};

const char** DivPlatformBR811::getRegisterSheet() {
  return regCheatSheetBR811;
}

// escreve um registrador do chip e guarda uma copia no regPool
void DivPlatformBR811::chipWrite(int ch, int reg, unsigned char val) {
  regPool[ch*3+reg]=val;
  chip.write(ch,reg,val);
}

void DivPlatformBR811::chipWritePcm(int reg, unsigned char val) {
  regPool[15+reg]=val;
  chip.writePcm(reg,val);
}

// inicia o sample do canal 5 (PCM de 7 bits, da ROM do cartucho)
void DivPlatformBR811::pcmStart(int ch) {
  int sNum=chan[ch].sample;
  DivSample* sample=parent->getSample(sNum);
  if (sample==NULL || sNum<0 || sNum>=parent->song.sampleLen) return;
  if (!sampleLoaded[sNum] || sampleLenBytes[sNum]==0) return;
  unsigned int off=sampleOff[sNum];
  unsigned int blocks=sampleLenBytes[sNum]>>4; // tamanho em blocos de 16 bytes
  chipWritePcm(br811::PCM_CTRL,0); // para antes, para recomecar do inicio
  chipWritePcm(br811::PCM_ADDR_LO,off&0xff);
  chipWritePcm(br811::PCM_ADDR_HI,(off>>8)&0xff);
  chipWritePcm(br811::PCM_LENGTH,(blocks>0)?(blocks-1):0);
  chipWritePcm(br811::PCM_CTRL,br811::PCM_CTRL_PLAY|(sample->loop?br811::PCM_CTRL_LOOP:0));
  pcmActive=true;
}

void DivPlatformBR811::pcmStop() {
  chipWritePcm(br811::PCM_CTRL,0);
  pcmActive=false;
}

// fonte (0-7) -> modo do chip e taxa de ruido
// 0 = Onda A, 1 = Onda B, 2..7 = ruido: /512, /1024, /2048, vinculado, /256, /128
static const unsigned char sourceNoiseRate[8]={
  0, 0, br811::NOISE_DIV512, br811::NOISE_DIV1024, br811::NOISE_DIV2048, br811::NOISE_LINKED, br811::NOISE_DIV256, br811::NOISE_DIV128
};

void DivPlatformBR811::commit(int ch) {
  int src=chan[ch].source&7;
  int mode=(src==0)?br811::MODE_WAVE_A:((src==1)?br811::MODE_WAVE_B:br811::MODE_NOISE);
  if (ch==4 && chan[ch].pcm) mode=br811::MODE_PCM;
  if (ch==4 && !chan[ch].pcm && pcmActive) pcmStop();
  int vol=(chan[ch].active && !isMuted[ch])?chan[ch].outVol:0;
  if (vol<0) vol=0;
  if (vol>15) vol=15;
  int period=chan[ch].period;
  if (period<0) period=0;
  if (period>2047) period=2047;

  chipWrite(ch,1,period&0xff);
  chipWrite(ch,2,((period>>8)&7)|(sourceNoiseRate[src]<<3));
  chipWrite(ch,0,(mode<<6)|vol);
  chan[ch].dirty=false;
}

void DivPlatformBR811::acquire(short** buf, size_t len) {
  for (int i=0; i<5; i++) {
    oscBuf[i]->begin(len);
  }

  const size_t BLOCK=256;
  float mix[BLOCK];
  float chOut[5][BLOCK];
  float* chPtr[5]={chOut[0],chOut[1],chOut[2],chOut[3],chOut[4]};

  size_t done=0;
  while (done<len) {
    size_t n=len-done;
    if (n>BLOCK) n=BLOCK;
    chip.render(mix,(int)n,chPtr);
    for (size_t k=0; k<n; k++) {
      buf[0][done+k]=(short)(mix[k]*32767.0f);
      for (int i=0; i<5; i++) {
        oscBuf[i]->putSample(done+k,(short)(chOut[i][k]*16384.0f));
      }
    }
    done+=n;
  }

  for (int i=0; i<5; i++) {
    oscBuf[i]->end(len);
  }
}

void DivPlatformBR811::tick(bool sysTick) {
  for (int i=0; i<5; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=VOL_SCALE_LINEAR(chan[i].vol&15,MIN(15,chan[i].std.vol.val),15);
      chan[i].dirty=true;
    }
    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had && !chan[i].rawFreq) {
      if (!chan[i].inPorta) {
        chan[i].baseFreq=chan[i].calcBaseFreq(parent->calcArp(chan[i].note,chan[i].std.arp.val));
      }
      chan[i].freqChanged=true;
    }
    if (chan[i].std.duty.had) {
      chan[i].source=chan[i].std.duty.val&7;
      chan[i].dirty=true;
    }
    if (chan[i].std.pitch.had) {
      if (chan[i].std.pitch.mode) {
        chan[i].pitch2+=chan[i].std.pitch.val;
        CLAMP_VAR(chan[i].pitch2,-32768,32767);
      } else {
        chan[i].pitch2=chan[i].std.pitch.val;
      }
      chan[i].freqChanged=true;
    }
    bool wasKeyOn=chan[i].keyOn;
    bool wasKeyOff=chan[i].keyOff;
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      chan[i].freq=chan[i].calcFreq();
      if (!chan[i].rawFreq) {
        // a tabela de afinacao entrega (periodo + 1); o registrador guarda o periodo
        chan[i].freq--;
        if (chan[i].freq<0) chan[i].freq=0;
        if (chan[i].freq>2047) chan[i].freq=2047;
      }
      chan[i].period=chan[i].freq;
      chan[i].keyOn=false;
      chan[i].keyOff=false;
      chan[i].freqChanged=false;
      chan[i].dirty=true;
    }
    if (chan[i].dirty) commit(i);
    if (i==4 && chan[i].pcm) {
      if (wasKeyOn) pcmStart(i);
      if (wasKeyOff) pcmStop();
    }
  }
}

int DivPlatformBR811::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_BR811);
      // canal 5: instrumento BR811 com "Use sample" ligado = PCM
      chan[c.chan].pcm=(c.chan==4 && (ins->type==DIV_INS_AMIGA || ins->amiga.useSample));
      if (chan[c.chan].pcm && c.value!=DIV_NOTE_NULL) {
        chan[c.chan].sample=ins->amiga.getSample(c.value);
        c.value=ins->amiga.getFreq(c.value);
      }
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=chan[c.chan].calcBaseFreq(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].macroInit(ins);
      if (!parent->song.compatFlags.brokenOutVol && !chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
      }
      chan[c.chan].dirty=true;
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].active=false;
      chan[c.chan].keyOff=true;
      chan[c.chan].macroInit(NULL);
      chan[c.chan].dirty=true;
      break;
    case DIV_CMD_NOTE_OFF_ENV:
    case DIV_CMD_ENV_RELEASE:
      chan[c.chan].std.release();
      break;
    case DIV_CMD_INSTRUMENT:
      if (chan[c.chan].ins!=c.value || c.value2==1) {
        chan[c.chan].ins=c.value;
      }
      break;
    case DIV_CMD_VOLUME:
      if (chan[c.chan].vol!=c.value) {
        chan[c.chan].vol=c.value;
        if (!chan[c.chan].std.vol.has) {
          chan[c.chan].outVol=c.value;
        }
        if (chan[c.chan].active) chan[c.chan].dirty=true;
      }
      break;
    case DIV_CMD_GET_VOLUME:
      return chan[c.chan].vol;
      break;
    case DIV_CMD_PITCH:
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_STD_NOISE_MODE: // efeito 20xx: escolhe a fonte do som (0-7)
      if (c.value>=0 && c.value<8) {
        chan[c.chan].source=c.value;
        chan[c.chan].dirty=true;
      }
      break;
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=chan[c.chan].calcBaseFreq(c.value2);
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
      if (return2) {
        chan[c.chan].inPorta=false;
        return 2;
      }
      break;
    }
    case DIV_CMD_LEGATO:
      chan[c.chan].baseFreq=chan[c.chan].calcBaseFreq(c.value+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.compatFlags.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_BR811));
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.compatFlags.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) chan[c.chan].baseFreq=chan[c.chan].calcBaseFreq(chan[c.chan].note);
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 15;
      break;
    case DIV_CMD_MACRO_OFF:
      chan[c.chan].std.mask(c.value,true);
      break;
    case DIV_CMD_MACRO_ON:
      chan[c.chan].std.mask(c.value,false);
      break;
    case DIV_CMD_MACRO_RESTART:
      chan[c.chan].std.restart(c.value);
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformBR811::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  chan[ch].dirty=true;
}

bool DivPlatformBR811::keyOffAffectsArp(int ch) {
  return true;
}

void DivPlatformBR811::forceIns() {
  for (int i=0; i<5; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
    chan[i].dirty=true;
  }
}

SharedChannel* DivPlatformBR811::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformBR811::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

DivDispatchOscBuffer* DivPlatformBR811::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformBR811::getRegisterPool() {
  return regPool;
}

int DivPlatformBR811::getRegisterPoolSize() {
  return 19;
}

void DivPlatformBR811::reset() {
  memset(regPool,0,19);
  chip.reset();
  pcmActive=false;
  for (int i=0; i<5; i++) {
    chan[i]=Channel(parent->song.compatFlags.linearPitch);
    chan[i].pitchTable=&pitchTable;
    chan[i].std.setEngine(parent);
  }
}

int DivPlatformBR811::getOutputCount() {
  return 1;
}

bool DivPlatformBR811::getDCOffRequired() {
  // a saida do chip e unipolar (0 a max)
  return true;
}

void DivPlatformBR811::notifyInsDeletion(void* ins) {
  for (int i=0; i<5; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformBR811::notifyPitchTable(int sample) {
  pitchTable.init(parent->song.tuning,chipClock,CHIP_DIVIDER,2048,true,parent->song.compatFlags.linearPitch);
}

unsigned int DivPlatformBR811::getMaxFreq(int ch) {
  return 2047;
}

void DivPlatformBR811::setFlags(const DivConfig& flags) {
  // o clock do BR811 e fixo: o mesmo da CPU RS04 do Ompreza (6,42 MHz)
  chipClock=BR811_CLOCK;
  rate=chipClock/RATE_DIVIDER;
  chip.setSampleRate(rate);
  for (int i=0; i<5; i++) {
    oscBuf[i]->setRate(rate);
  }

  notifyPitchTable();
}

void DivPlatformBR811::poke(unsigned int addr, unsigned short val) {
  if (addr>=19) return;
  if (addr>=15) {
    chipWritePcm(addr-15,val);
    return;
  }
  chipWrite(addr/3,addr%3,val);
}

void DivPlatformBR811::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) poke(i.addr,i.val);
}

int DivPlatformBR811::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<5; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  sampleMemLen=0;
  memset(sampleMem,0x40,65536);
  pcmActive=false;
  // o chip le os samples da "ROM do cartucho"
  chip.setRomReader([this](uint16_t addr) -> uint8_t { return sampleMem[addr]; });
  setFlags(flags);
  reset();
  return 5;
}

void DivPlatformBR811::quit() {
  for (int i=0; i<5; i++) {
    delete oscBuf[i];
  }
}

const void* DivPlatformBR811::getSampleMem(int index) {
  return index==0?sampleMem:NULL;
}

size_t DivPlatformBR811::getSampleMemCapacity(int index) {
  return index==0?65536:0;
}

size_t DivPlatformBR811::getSampleMemUsage(int index) {
  return index==0?sampleMemLen:0;
}

bool DivPlatformBR811::isSampleLoaded(int index, int sample) {
  if (index!=0) return false;
  if (sample<0 || sample>=getMaxSamples(index)) return false;
  return sampleLoaded[sample];
}

const DivMemoryComposition* DivPlatformBR811::getMemCompo(int index) {
  if (index!=0) return NULL;
  return &memCompo;
}

// monta a "ROM": samples de 7 bits, alinhados em blocos de 16 bytes, no maximo 4096 bytes cada
void DivPlatformBR811::renderSamples(int sysID) {
  memset(sampleMem,0x40,65536); // 0x40 = silencio no PCM de 7 bits
  memset(sampleLoaded,0,32768*sizeof(bool));

  memCompo=DivMemoryComposition();
  memCompo.name="Sample ROM";

  size_t memPos=0;
  for (int i=0; i<parent->song.sampleLen; i++) {
    DivSample* s=parent->song.sample[i];
    sampleLenBytes[i]=0;
    if (!s->renderOn[0][sysID]) {
      sampleOff[i]=0;
      continue;
    }
    size_t paddedLen=(s->length8+15)&(~15);
    if (paddedLen>4096) paddedLen=4096;
    if (paddedLen==0) {
      sampleOff[i]=0;
      continue;
    }
    if (memPos+paddedLen>65536) {
      logW("out of memory for sample %d!",i);
      break;
    }
    size_t len=MIN((size_t)s->length8,paddedLen);
    for (size_t k=0; k<len; k++) {
      sampleMem[memPos+k]=(unsigned char)(((int)s->data8[k]+128)>>1);
    }
    sampleOff[i]=memPos;
    sampleLenBytes[i]=paddedLen;
    sampleLoaded[i]=true;
    memCompo.entries.push_back(DivMemoryEntry(DIV_MEMORY_SAMPLE,"Sample",i,memPos,memPos+paddedLen));
    memPos+=paddedLen;
  }
  sampleMemLen=memPos;

  memCompo.capacity=65536;
  memCompo.used=sampleMemLen;
}

DivPlatformBR811::DivPlatformBR811() {
  sampleMem=new unsigned char[65536];
  sampleOff=new unsigned int[32768];
  sampleLenBytes=new unsigned int[32768];
  sampleLoaded=new bool[32768];
  memset(sampleOff,0,32768*sizeof(unsigned int));
  memset(sampleLenBytes,0,32768*sizeof(unsigned int));
  memset(sampleLoaded,0,32768*sizeof(bool));
}

DivPlatformBR811::~DivPlatformBR811() {
  delete[] sampleMem;
  delete[] sampleOff;
  delete[] sampleLenBytes;
  delete[] sampleLoaded;
}
