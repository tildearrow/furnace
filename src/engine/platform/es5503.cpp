/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2023 tildearrow and contributors
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

#include "es5503.h"
#include "../engine.h"
#include "furIcons.h"
#include <math.h>

#define my_min(a, b) ((a) >= (b) ? (b) : (a))

const int ES5503_wave_lengths[DivInstrumentES5503::DIV_ES5503_WAVE_LENGTH_MAX] = {256, 512, 1024, 2048, 4096, 8192, 16384, 32768};

//#define rWrite(a,v) pendingWrites[a]=v;
#define rWrite(a,v) if (!skipRegisterWrites) {writes.push(QueuedWrite(a,v)); if (dumpWrites) {addWrite(a,v);} }

#define CHIP_DIVIDER (double)(chipClock / rate)

#define CHIP_FREQBASE (894886.0)

const char* regCheatSheetES5503[]={
  "CHx_FreqL", "x",
  "CHx_FreqH", "20+x",
  "CHx_Vol", "40+x",
  "CHx_CurrSample", "60+x",
  "CHx_WaveAddress", "80+x",
  "CHx_Control", "A0+x",
  "CHx_WaveSize", "C0+x",
  "Osc_interrupt", "E0",
  "Osc_enable", "E1",
  "A/D_conversion_result", "E2",
  NULL
};


const char** DivPlatformES5503::getRegisterSheet() {
  return regCheatSheetES5503;
}

void DivPlatformES5503::acquire(short** buf, size_t len) { //the function where we actually fill the fucking audio buffer!!!!!
  es5503.fill_audio_buffer(buf[0], buf[1], len);

  while (!writes.empty()) { //do register writes
      QueuedWrite w=writes.front();
      es5503.write(w.addr,w.val);
      regPool[w.addr]=w.val;
      writes.pop();
    }
}

void DivPlatformES5503::setFlags(const DivConfig& flags) {
  chipClock=7159090U; //approx. 894886 * 8 Hz on Apple IIGS

  CHECK_CUSTOM_CLOCK;

  es5503.es5503_core_init(chipClock, this->oscBuf, 32);

  rate=(chipClock / 8) / (es5503.oscsenabled + 2); //26320 Hz for Apple IIGS card with all oscillators enabled

  for (int i=0; i<32; i++) {
    oscBuf[i]->rate=rate;
  }

  mono=flags.getBool("monoOutput",false);

  es5503.mono = mono;
}

void DivPlatformES5503::changeNumOscs(uint8_t num_oscs)
{
  es5503.update_num_osc(oscBuf, num_oscs);
  rWrite(0xe1, (num_oscs - 1) * 2);
  rate=(chipClock / 8) / (es5503.oscsenabled + 2);

  for (int i=0; i<32; i++) {
    oscBuf[i]->rate=rate;
  }
}

void DivPlatformES5503::writeSampleMemoryByte(int address, unsigned char value)
{
  if(es5503.sampleMem)
  {
    es5503.sampleMem[address] = value;
  }
}

void DivPlatformES5503::updateWave(int ch)
{
  if(!chan[ch].pcm)
  {
    for (int i=0; i<chan[ch].wave_size; i++)
    {
      uint8_t val = chan[ch].ws.output[i & 255];
      if (val == 0) val = 1;
      writeSampleMemoryByte(chan[ch].wave_pos + i, val); //if using synthesized wavetable, avoid zeros so wave can loop
    }

    if (dumpWrites) addWrite(0xfffe0000 | chan[ch].wave_size, chan[ch].wave_pos >> 8);
  }

  else
  {
    DivSample* s=parent->getSample(chan[ch].sample);

    if(!s->data8) return;

    int i=0;

    for (i=0; i<my_min(s->length8, chan[ch].wave_size); i++)
    {
      uint8_t val = (uint8_t)s->data8[i] + 0x80;
      if (val == 0) val = 1;
      writeSampleMemoryByte(chan[ch].wave_pos + i, val); //avoid zeros so wave can loop and does not halt
    }

    for (; i < my_min(s->length8 + 8, chan[ch].wave_size); i++) //write at least 8 zeros if wave size allows to guarantee oneshot samples are halted if they are shorter than wave size
    {
      writeSampleMemoryByte(chan[ch].wave_pos + i, 0);
    }

    if (dumpWrites) addWrite(0xfffe0000|my_min(s->length8 + 8, chan[ch].wave_size), chan[ch].wave_pos >> 8);
  }
  
  //chan[ch].antiClickWavePos&=31;
  if (chan[ch].active) {
    if(chan[ch].softpan_channel)
    {
      uint8_t temp = isMuted[ch] ? 0 : chan[ch].outVol*chan[ch].panleft / 255;
      rWrite(0x40+ch,temp);
      temp = isMuted[ch] ? 0 : chan[ch].outVol*chan[ch].panright / 255;
      rWrite(0x40+ch+1,temp);
    }

    else
    {
      rWrite(0x40+ch,isMuted[ch] ? 0 : chan[ch].outVol);
    }
  }
}

void DivPlatformES5503::tick(bool sysTick) {
  for(int i = 0; i < 32; i++)
  {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=VOL_SCALE_LINEAR(chan[i].vol&255,MIN(255,chan[i].std.vol.val),255);
      if (chan[i].pcm) {
        // ignore for now
      } else {
        if(chan[i].softpan_channel)
        {
          uint8_t temp = chan[i].outVol * chan[i].panleft / 255;
          rWrite(0x40 + i, isMuted[i] ? 0 : (temp));
          temp = chan[i].outVol * chan[i].panright / 255;
          rWrite(0x40 + i + 1, isMuted[i] ? 0 : (temp));
        }

        else
        {
          rWrite(0x40 + i, isMuted[i] ? 0 : chan[i].outVol);
        }
      }
    }

    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      chan[i].freqChanged=true;
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

    if (chan[i].std.duty.had && !chan[i].pcm) {
      chan[i].osc_mode = chan[i].std.duty.val & 3;
      if(chan[i].softpan_channel)
      {
        rWrite(0xA0 + i, (chan[i].osc_mode << 1) | (chan[i].output << 4)); //update osc. mode, do not disturb the osc
      }

      else
      {
        rWrite(0xA0 + i, (chan[i].osc_mode << 1) | (chan[i].output << 4));
        rWrite(0xA0 + i + 1, (chan[i].osc_mode << 1) | (chan[i + 1].output << 4));
      }
    }

    if (chan[i].std.wave.had && !chan[i].pcm) {
      if (chan[i].wave!=chan[i].std.wave.val || chan[i].ws.activeChanged()) {
        chan[i].wave=chan[i].std.wave.val;
        chan[i].ws.changeWave1(chan[i].wave);
        if (!chan[i].keyOff) chan[i].keyOn=true;
      }
    }

    if (chan[i].std.phaseReset.had && !chan[i].pcm && chan[i].std.phaseReset.val == 1) {
      if(chan[i].softpan_channel)
      {
        rWrite(0xA0 + i, (chan[i].osc_mode << 1) | 1 | (chan[i].output << 4)); //writing 1 resets acc
        rWrite(0xA0 + i, (chan[i].osc_mode << 1) | (chan[i].output << 4)); //writing 0 forces the reset
        rWrite(0xA0 + i + 1, (chan[i].osc_mode << 1) | 1 | (chan[i + 1].output << 4)); //writing 1 resets acc
        rWrite(0xA0 + i + 1, (chan[i].osc_mode << 1) | (chan[i + 1].output << 4)); //writing 0 forces the reset
      }

      else
      {
        rWrite(0xA0 + i, (chan[i].osc_mode << 1) | 1 | (chan[i].output << 4)); //writing 1 resets acc
        rWrite(0xA0 + i, (chan[i].osc_mode << 1) | (chan[i].output << 4)); //writing 0 forces the reset
      }
    }

    if (chan[i].std.ex1.had) {
      if(chan[i].softpan_channel)
      {
        rWrite(0x80 + i, chan[i].std.ex1.val);
        rWrite(0x80 + i + 1, chan[i].std.ex1.val);
      }

      else
      {
        rWrite(0x80 + i, chan[i].std.ex1.val);
      }
    }

    if (chan[i].std.ex2.had) {
      chan[i].output = chan[i].std.ex2.val;
      rWrite(0xa0 + i, (chan[i].osc_mode << 1) | (chan[i].output << 4));
    }

    if (chan[i].std.panL.had && chan[i].softpan_channel) {
      chan[i].panleft = chan[i].std.panL.val;
      uint8_t temp = chan[i].outVol * chan[i].panleft / 255;
      rWrite(0x40 + i, isMuted[i] ? 0 : (temp));
    }

    if (chan[i].std.panR.had && chan[i].softpan_channel) {
      chan[i].panright = chan[i].std.panR.val;
      uint8_t temp = chan[i].outVol * chan[i].panright / 255;
      rWrite(0x40 + i + 1, isMuted[i] ? 0 : (temp));
    }

    if (chan[i].active && !chan[i].pcm) {
      if (chan[i].ws.tick() || (chan[i].std.phaseReset.had && chan[i].std.phaseReset.val==1)) {
        updateWave(i);
      }
    }

    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      DivInstrument* ins=parent->getIns(chan[i].ins,DIV_INS_ES5503);
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,false,0,chan[i].pitch2,(double)chipClock /** (32 + 2) / (es5503.oscsenabled + 2)*/,CHIP_FREQBASE * 130.81 * 2 / 211.0); //TODO: why freq calc is wrong?
      if (chan[i].freq<0) chan[i].freq=0;
      if (chan[i].freq>0xffff) chan[i].freq=0xffff;
      if (chan[i].pcm) {
        double off=1.0;
        if (chan[i].sample>=0) {
          DivSample* s=parent->getSample(chan[i].sample);
          if (s->centerRate<1) {
            off=1.0;
          } else {
            off=8363.0/(double)s->centerRate;
          }
        }
        //chan[i].dacRate=((double)chipClock/2)/MAX(1,off*chan[i].freq);
        //if (dumpWrites) addWrite(0xffff0001+(i<<8),chan[i].dacRate);
      }

      if(chan[i].softpan_channel)
      {
        chan[i + 1].freq = chan[i].freq;
        rWrite(i, chan[i].freq&0xff);
        rWrite(0x20+i, chan[i].freq>>8);
        rWrite(i+1, chan[i].freq&0xff);
        rWrite(0x20+i+1, chan[i].freq>>8);
      }

      else
      {
        rWrite(i, chan[i].freq&0xff);
        rWrite(0x20+i, chan[i].freq>>8);
      }

      if (chan[i].keyOn) {
        if(chan[i].softpan_channel)
        {
          rWrite(0xA0+i, 0 | (ins->es5503.initial_osc_mode << 1) | (chan[i].output << 4));
          rWrite(0xA0+i+1, 0 | (ins->es5503.initial_osc_mode << 1) | (chan[i + 1].output << 4));
        }

        else
        {
          rWrite(0xA0+i, 0 | (ins->es5503.initial_osc_mode << 1) | (chan[i].output << 4)); //reset halt bit and set oscillator mode
        }
      }
      if (chan[i].keyOff) {
        if(chan[i].softpan_channel)
        {
          rWrite(0xA0+i, 1 | (chan[i].output << 4)); //halt oscillator
          rWrite(0xA0+i+1, 1 | (chan[i + 1].output << 4)); //halt oscillator
        }

        else
        {
          rWrite(0xA0+i, 1 | (chan[i].output << 4)); //halt oscillator
        }
      }

      if(chan[i].softpan_channel)
      {
        if (chan[i].keyOn) chan[i+1].keyOn=false;
        if (chan[i].keyOff) chan[i+1].keyOff=false;
      }

      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
  }
}

int DivPlatformES5503::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_ES5503);
      chan[c.chan].softpan_channel = ins->es5503.softpan_virtual_channel && !(c.chan & 1); //only works on odd channel
      chan[c.chan].macroVolMul=ins->type==DIV_INS_AMIGA?64:255;
      if (ins->type==DIV_INS_AMIGA || ins->amiga.useSample) {
        chan[c.chan].pcm=true;
      }
      else
      {
        chan[c.chan].pcm=false;
        chan[c.chan].address_bus_res = 0b000;
      }
      if (chan[c.chan].pcm) {
        if (ins->type==DIV_INS_AMIGA || ins->amiga.useSample) {
          if (c.value!=DIV_NOTE_NULL) {
            chan[c.chan].sample=ins->amiga.getSample(c.value);
            c.value=ins->amiga.getFreq(c.value);
          }
          if (chan[c.chan].sample<0) {
            chan[c.chan].sample=-1;
            break;
          } else {
             
          }
        }
        //break;

        chan[c.chan].wave_pos = ins->es5503.wavePos << 8;
        chan[c.chan].osc_mode = ins->es5503.initial_osc_mode;
        chan[c.chan].wave_size = ES5503_wave_lengths[ins->es5503.waveLen&7];

        if(chan[c.chan].softpan_channel)
        {
          chan[c.chan + 1].wave_pos = ins->es5503.wavePos << 8;
          chan[c.chan + 1].osc_mode = ins->es5503.initial_osc_mode;
          chan[c.chan + 1].wave_size = ES5503_wave_lengths[ins->es5503.waveLen&7];
        }

        if(chan[c.chan].wave_pos != chan[c.chan].previous_sample_pos || chan[c.chan].sample != chan[c.chan].previous_sample)
        {
          updateWave(c.chan);
        }

        chan[c.chan].previous_sample_pos = chan[c.chan].wave_pos;
        chan[c.chan].previous_sample = chan[c.chan].sample;

        if(chan[c.chan].wave_size >= 1024)
        {
          chan[c.chan].address_bus_res = 0b011;
        }
      }
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      //chWrite(c.chan,0x04,0x80|chan[c.chan].vol);
      if(chan[c.chan].softpan_channel)
      {
        chan[c.chan + 1].active=true;
        chan[c.chan + 1].keyOn=true;

        chan[c.chan].output = 0;
        chan[c.chan + 1].output = 1; //force-reset outputs so softpanned channel works as expected

        uint8_t temp = chan[c.chan].vol * chan[c.chan].panleft / 255;
        rWrite(0x40+c.chan, isMuted[c.chan] ? 0 : temp); //set volume
        rWrite(0x80+c.chan, ins->es5503.wavePos); //set wave pos
        rWrite(0xa0+c.chan, (ins->es5503.initial_osc_mode << 1) | (chan[c.chan].output << 4));
        rWrite(0xc0+c.chan, (ins->es5503.waveLen << 3) | chan[c.chan].address_bus_res /*lowest acc resolution*/); //set wave len

        temp = chan[c.chan].vol * chan[c.chan].panright / 255;
        rWrite(0x40+c.chan+1, isMuted[c.chan] ? 0 : temp); //set volume
        rWrite(0x80+c.chan+1, ins->es5503.wavePos); //set wave pos
        rWrite(0xa0+c.chan+1, (ins->es5503.initial_osc_mode << 1) | (chan[c.chan].output << 4));
        rWrite(0xc0+c.chan+1, (ins->es5503.waveLen << 3) | chan[c.chan].address_bus_res /*lowest acc resolution*/); //set wave len

        if(ins->es5503.phase_reset_on_start)
        {
          rWrite(0xA0 + c.chan, (chan[c.chan].osc_mode << 1) | 1 | (chan[c.chan].output << 4)); //writing 1 resets acc
          rWrite(0xA0 + c.chan, (chan[c.chan].osc_mode << 1) | (chan[c.chan].output << 4)); //writing 0 forces the reset
          rWrite(0xA0 + c.chan + 1, (chan[c.chan].osc_mode << 1) | 1 | (chan[c.chan + 1].output << 4)); //writing 1 resets acc
          rWrite(0xA0 + c.chan + 1, (chan[c.chan].osc_mode << 1) | (chan[c.chan + 1].output << 4)); //writing 0 forces the reset
        }
      }
      
      else
      {
        rWrite(0x40+c.chan, isMuted[c.chan] ? 0 : chan[c.chan].vol); //set volume
        rWrite(0x80+c.chan, ins->es5503.wavePos); //set wave pos
        rWrite(0xa0+c.chan, (ins->es5503.initial_osc_mode << 1) | (chan[c.chan].output << 4));
        rWrite(0xc0+c.chan, (ins->es5503.waveLen << 3) | chan[c.chan].address_bus_res /*lowest acc resolution*/); //set wave len

        if(ins->es5503.phase_reset_on_start)
        {
          rWrite(0xA0 + c.chan, (chan[c.chan].osc_mode << 1) | 1 | (chan[c.chan].output << 4)); //writing 1 resets acc
          rWrite(0xA0 + c.chan, (chan[c.chan].osc_mode << 1) | (chan[c.chan].output << 4)); //writing 0 forces the reset
        }
      }

      if (!chan[c.chan].pcm)
      {
        chan[c.chan].wave_pos = ins->es5503.wavePos << 8;
        chan[c.chan].osc_mode = ins->es5503.initial_osc_mode;
        chan[c.chan].wave_size = ES5503_wave_lengths[ins->es5503.waveLen&7];

        if(chan[c.chan].softpan_channel)
        {
          chan[c.chan + 1].wave_pos = ins->es5503.wavePos << 8;
          chan[c.chan + 1].osc_mode = ins->es5503.initial_osc_mode;
          chan[c.chan + 1].wave_size = ES5503_wave_lengths[ins->es5503.waveLen&7];
        }
      }

      chan[c.chan].macroInit(ins);
      if (!chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
      }

      if (!chan[c.chan].pcm)
      {
        if (chan[c.chan].wave<0) 
        {
          chan[c.chan].wave=0;
          chan[c.chan].ws.changeWave1(chan[c.chan].wave);
        }

        chan[c.chan].ws.init(ins,256,255,chan[c.chan].insChanged);
        chan[c.chan].insChanged=false;
      }
      
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].sample=-1;
      //if (dumpWrites) addWrite(0xffff0002+(c.chan<<8),0);
      chan[c.chan].pcm=false;
      chan[c.chan].active=false;
      chan[c.chan].keyOff=true;
      chan[c.chan].macroInit(NULL);
      break;
    case DIV_CMD_NOTE_OFF_ENV:
    case DIV_CMD_ENV_RELEASE:
      chan[c.chan].std.release();
      break;
    case DIV_CMD_INSTRUMENT:
      if (chan[c.chan].ins!=c.value || c.value2==1) {
        chan[c.chan].ins=c.value;
        chan[c.chan].insChanged=true;
      }
      break;
    case DIV_CMD_VOLUME:
      if (chan[c.chan].vol!=c.value) {
        chan[c.chan].vol=c.value;
        if (!chan[c.chan].std.vol.has) {
          chan[c.chan].outVol=c.value;
          if (chan[c.chan].active && !chan[c.chan].pcm) {
            if(chan[c.chan].softpan_channel)
            {
              uint8_t temp = chan[c.chan].outVol * chan[c.chan].panleft / 255;
              rWrite(0x40 + c.chan, isMuted[c.chan] ? 0 : temp);
              temp = chan[c.chan].outVol * chan[c.chan].panright / 255;
              rWrite(0x40 + c.chan + 1, isMuted[c.chan] ? 0 : temp);
            }

            else
            {
              rWrite(0x40 + c.chan, isMuted[c.chan] ? 0 : chan[c.chan].outVol);
            }
          }
        }
      }
      break;
    case DIV_CMD_GET_VOLUME:
      if (chan[c.chan].std.vol.has) {
        return chan[c.chan].vol;
      }
      return chan[c.chan].outVol;
      break;
    case DIV_CMD_PANNING: {
      if(chan[c.chan].softpan_channel)
      {
        chan[c.chan].panleft = c.value;
        chan[c.chan].panright = c.value2;

        uint8_t temp = chan[c.chan].vol * chan[c.chan].panleft / 255;
        rWrite(0x40+c.chan, isMuted[c.chan] ? 0 : temp);
        temp = chan[c.chan].vol * chan[c.chan].panright / 255;
        rWrite(0x40+c.chan+1, isMuted[c.chan] ? 0 : temp);
      }
      break;
    }
    case DIV_CMD_PITCH:
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_LEGATO:
      chan[c.chan].baseFreq=NOTE_PERIODIC(c.value+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_ES5503));
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) chan[c.chan].baseFreq=NOTE_PERIODIC(chan[c.chan].note);
      chan[c.chan].inPorta=c.value;
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
      if (return2) {
        chan[c.chan].inPorta=false;
        return 2;
      }
      break;
    }
    case DIV_CMD_ES5503_NUM_ENABLED_OSC: {
      if(c.value >= 2 && c.value <= 32)
      {
        changeNumOscs(c.value);
      }
      break;
    }
    case DIV_CMD_ES5503_OSC_OUTPUT: {
      if(c.value <= 7)
      {
        chan[c.chan].output = c.value;
        rWrite(0xa0+c.chan, (chan[c.chan].osc_mode << 1) | (chan[c.chan].output << 4));
      }
      break;
    }
    case DIV_CMD_ES5503_WAVE_LENGTH: {
      if(c.value <= 7)
      {
        chan[c.chan].wave_size = ES5503_wave_lengths[c.value];
        rWrite(0xc0+c.chan, (c.value << 3) | chan[c.chan].address_bus_res /*lowest acc resolution*/); //set wave len

        if(chan[c.chan].softpan_channel)
        {
          chan[c.chan + 1].wave_size = ES5503_wave_lengths[c.value];
          rWrite(0xc0+c.chan+1, (c.value << 3) | chan[c.chan].address_bus_res /*lowest acc resolution*/); //set wave len
        }
      }
      break;
    }
    case DIV_CMD_ES5503_WAVE_POS: {
      chan[c.chan].wave_pos = c.value << 8;
      rWrite(0x80+c.chan, c.value);

      if(chan[c.chan].softpan_channel)
      {
        chan[c.chan + 1].wave_pos = c.value << 8;
        rWrite(0x80+c.chan+1, c.value);
      }
      break;
    }
    case DIV_CMD_ES5503_OSC_MODE: {
      if(c.value <= 3)
      {
        chan[c.chan].osc_mode = c.value;
        rWrite(0xa0+c.chan, (chan[c.chan].osc_mode << 1) | (chan[c.chan].output << 4));

        if(chan[c.chan].softpan_channel)
        {
          chan[c.chan+1].osc_mode = c.value;
          rWrite(0xa0+c.chan+1, (chan[c.chan].osc_mode << 1) | (chan[c.chan+1].output << 4)); //don't forget to preserve "slave" chan output
        }
      }
      break;
    }
    case DIV_CMD_GET_VOLMAX:
      return 255;
      break;
    case DIV_CMD_MACRO_OFF:
      chan[c.chan].std.mask(c.value,true);
      break;
    case DIV_CMD_MACRO_ON:
      chan[c.chan].std.mask(c.value,false);
      break;
    case DIV_ALWAYS_SET_VOLUME:
      return 1;
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformES5503::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  /*chWrite(ch,0x05,isMuted[ch]?0:chan[ch].pan);
  if (!isMuted[ch] && (chan[ch].pcm && chan[ch].dacSample!=-1)) {
    chWrite(ch,0x04,parent->song.disableSampleMacro?0xdf:(0xc0|chan[ch].outVol));
    chWrite(ch,0x06,chan[ch].dacOut&0x1f);
  }*/
}

void DivPlatformES5503::forceIns() {
  for (int i=0; i<32; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
    updateWave(i);

    if(chan[i].softpan_channel)
    {
      uint8_t temp = chan[i].vol * chan[i].panleft / 255;
      rWrite(0x40+i,isMuted[i]?0:(temp));
      temp = chan[i].vol * chan[i].panright / 255;
      rWrite(0x40+i+1,isMuted[i]?0:(temp));
    }

    else
    {
      rWrite(0x40+i,isMuted[i]?0:chan[i].vol);
    }
  }
}

void* DivPlatformES5503::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformES5503::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

const void* DivPlatformES5503::getSampleMem(int index) {
  return index == 0 ? es5503.sampleMem : NULL;
}

size_t DivPlatformES5503::getSampleMemCapacity(int index)
{
  return index == 0 ? 65536 : 0;
}

size_t DivPlatformES5503::getSampleMemUsage(int index)
{
  return index == 0 ? 65536 : 0;
}

DivDispatchOscBuffer* DivPlatformES5503::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformES5503::getRegisterPool() {
  return regPool;
}

int DivPlatformES5503::getRegisterPoolSize() {
  return 256;
}

void DivPlatformES5503::reset() {
  writes.clear();
  memset(regPool,0,256);
  if (dumpWrites) {
    addWrite(0xffffffff,0); //do reset
  }
  memset(es5503.sampleMem,0,es5503.sampleMemLen);
  for (uint8_t i=0; i<32; i++) {
    chan[i]=DivPlatformES5503::Channel();
    chan[i].std.setEngine(parent);
    chan[i].ws.setEngine(parent);
    chan[i].ws.init(NULL,256,255,false);

    chan[i].output = (i & 1) ? 1 : 0; //odd channels are left, even are right
  }

  curChan=-1;
  // set volume to zero and reset some other shit
  for(uint8_t i = 0; i < 0x9f; i++)
  {
    rWrite(i,0);
  }

  for(uint8_t i = 0xc0; i < 0xdf; i++)
  {
    rWrite(i,0);
  }

  //write to num_osc register and enable all 32 oscillators
  rWrite(0xe1,62);

  for(uint8_t i = 0xa0; i < 0xbf; i++) //odd channels are left, even are right
  {
    rWrite(i, (chan[i - 0xa0].output << 4)); //0=left, 1=right
  }
}

int DivPlatformES5503::getOutputCount() {
  return mono ? 1 : 2;
}

bool DivPlatformES5503::keyOffAffectsArp(int ch) {
  return true;
}

void DivPlatformES5503::notifyWaveChange(int wave) {
  for (int i=0; i<32; i++) {
    if (chan[i].wave==wave) {
      chan[i].ws.changeWave1(wave);
      updateWave(i);
    }
  }
}

void DivPlatformES5503::notifyInsChange(int ins) {
  for (int i=0; i<32; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformES5503::notifyInsDeletion(void* ins) {
  for (int i=0; i<32; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformES5503::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformES5503::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

int DivPlatformES5503::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  memset(this->regPool, 0, sizeof(this->regPool[0]) * 256);
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<32; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  setFlags(flags);
  reset();
  return 32;
}

void DivPlatformES5503::quit() {
  for (int i=0; i<32; i++) {
    delete oscBuf[i];
  }
  es5503.es5503_core_free();
}

DivPlatformES5503::~DivPlatformES5503() {
}
