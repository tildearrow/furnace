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
uint8_t ES5503_wave_lengths_convert_back(uint32_t len)
{
  if((int)len <= ES5503_wave_lengths[0]) return 0;

  for(int j = 0; j < DivInstrumentES5503::DIV_ES5503_WAVE_LENGTH_MAX - 1; j++)
  {
    if((int)len > ES5503_wave_lengths[j] && (int)len <= ES5503_wave_lengths[j + 1])
    {
      return j + 1;
    }
  }

  return 0;
}

int jump_blocks(int actual_size)
{
  for(int i = 0; i < DivInstrumentES5503::DIV_ES5503_WAVE_LENGTH_MAX; i++)
  {
    if(actual_size < ES5503_wave_lengths[i])
    {
      return 1 << i;
    }
  }

  return 0;
}

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
  es5503.fill_audio_buffer(buf, len);

  while (!writes.empty()) { //do register writes
    QueuedWrite w=writes.front();
    if(w.addr < 0x100)
    {
      es5503.write((uint8_t)w.addr,(uint8_t)w.val);
    }
    
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
  reserved_blocks=flags.getInt("reserveBlocks",0);

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

void DivPlatformES5503::writeSampleMemoryByte(unsigned int address, unsigned char value)
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
    for (unsigned int i=0; i<chan[ch].wave_size; i++)
    {
      uint8_t val = chan[ch].ws.output[i & 255];
      if (val == 0) val = 1;
      writeSampleMemoryByte(chan[ch].wave_pos + i, val); //if using synthesized wavetable, avoid zeros so wave can loop
    }

    if (dumpWrites) addWrite(0xfffe0000 | chan[ch].wave_size, chan[ch].wave_pos >> 8);
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
    if (chan[i].std.get_div_macro_struct(DIV_MACRO_VOL)->had) 
    {
      chan[i].outVol=VOL_SCALE_LINEAR(chan[i].vol&255,MIN(255,chan[i].std.get_div_macro_struct(DIV_MACRO_VOL)->val),255);

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

    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.get_div_macro_struct(DIV_MACRO_ARP)->had) {
      chan[i].freqChanged=true;
    }

    if (chan[i].std.get_div_macro_struct(DIV_MACRO_PITCH)->had) {
      if (chan[i].std.get_div_macro_struct(DIV_MACRO_PITCH)->mode) {
        chan[i].pitch2+=chan[i].std.get_div_macro_struct(DIV_MACRO_PITCH)->val;
        CLAMP_VAR(chan[i].pitch2,-32768,32767);
      } else {
        chan[i].pitch2=chan[i].std.get_div_macro_struct(DIV_MACRO_PITCH)->val;
      }
      chan[i].freqChanged=true;
    }

    if (chan[i].std.get_div_macro_struct(DIV_MACRO_DUTY)->had) {
      chan[i].osc_mode = chan[i].std.get_div_macro_struct(DIV_MACRO_DUTY)->val & 3;
      if(chan[i].softpan_channel)
      {
        rWrite(0xA0 + i, (chan[i].osc_mode << 1) | (chan[i].output << 4)); //update osc. mode, do not disturb the osc
        rWrite(0xA0 + i + 1, (chan[i].osc_mode << 1) | (chan[i + 1].output << 4));
      }

      else
      {
        rWrite(0xA0 + i, (chan[i].osc_mode << 1) | (chan[i].output << 4));
      }
    }

    if (chan[i].std.get_div_macro_struct(DIV_MACRO_WAVE)->had && !chan[i].pcm) {
      if (chan[i].wave!=chan[i].std.get_div_macro_struct(DIV_MACRO_WAVE)->val || chan[i].ws.activeChanged()) {
        chan[i].wave=chan[i].std.get_div_macro_struct(DIV_MACRO_WAVE)->val;
        chan[i].ws.changeWave1(chan[i].wave);
        if (!chan[i].keyOff) chan[i].keyOn=true;
      }
    }

    if (chan[i].std.get_div_macro_struct(DIV_MACRO_PHASE_RESET)->had && chan[i].std.get_div_macro_struct(DIV_MACRO_PHASE_RESET)->val == 1) {
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

    if (chan[i].std.get_div_macro_struct(DIV_MACRO_EX1)->had) {
      if(chan[i].softpan_channel)
      {
        rWrite(0x80 + i, chan[i].std.get_div_macro_struct(DIV_MACRO_EX1)->val);
        rWrite(0x80 + i + 1, chan[i].std.get_div_macro_struct(DIV_MACRO_EX1)->val);
      }

      else
      {
        rWrite(0x80 + i, chan[i].std.get_div_macro_struct(DIV_MACRO_EX1)->val);
      }
    }

    if (chan[i].std.get_div_macro_struct(DIV_MACRO_EX2)->had) {
      chan[i].output = chan[i].std.get_div_macro_struct(DIV_MACRO_EX2)->val;
      rWrite(0xa0 + i, (chan[i].osc_mode << 1) | (chan[i].output << 4));
    }

    if (chan[i].std.get_div_macro_struct(DIV_MACRO_PAN_LEFT)->had && chan[i].softpan_channel) {
      chan[i].panleft = chan[i].std.get_div_macro_struct(DIV_MACRO_PAN_LEFT)->val;
      uint8_t temp = chan[i].outVol * chan[i].panleft / 255;
      rWrite(0x40 + i, isMuted[i] ? 0 : (temp));
    }

    if (chan[i].std.get_div_macro_struct(DIV_MACRO_PAN_RIGHT)->had && chan[i].softpan_channel) {
      chan[i].panright = chan[i].std.get_div_macro_struct(DIV_MACRO_PAN_RIGHT)->val;
      uint8_t temp = chan[i].outVol * chan[i].panright / 255;
      rWrite(0x40 + i + 1, isMuted[i] ? 0 : (temp));
    }

    if (chan[i].active && !chan[i].pcm) {
      if (chan[i].ws.tick()) {
        updateWave(i);
      }
    }

    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      DivInstrument* ins=parent->getIns(chan[i].ins,DIV_INS_ES5503);
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,false,0,chan[i].pitch2,(double)chipClock,CHIP_FREQBASE * 130.81 * 2 / 211.0); //TODO: why freq calc is wrong?
      if (chan[i].freq<0) chan[i].freq=0;
      if (chan[i].freq>0xffff) chan[i].freq=0xffff;

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
          uint8_t temp = chan[i].outVol * chan[i].panleft / 255;
          rWrite(0x40+i, isMuted[i] ? 0 : temp); //set volume
          if(chan[i].pcm || chan[i].wavetable_block != -1)
          {
            rWrite(0x80+i, chan[i].wave_pos >> 8); //set wave pos
            rWrite(0xa0+i, (chan[i].osc_mode << 1) | (chan[i].output << 4));
          }

          rWrite(0xc0+i, (ES5503_wave_lengths_convert_back(chan[i].wave_size) << 3) | chan[i].address_bus_res); //set wave len

          temp = chan[i].outVol * chan[i].panright / 255;
          rWrite(0x40+i+1, isMuted[i] ? 0 : temp); //set volume

          if(chan[i].pcm || chan[i].wavetable_block != -1)
          {
            rWrite(0x80+i+1, chan[i].wave_pos >> 8); //set wave pos
            rWrite(0xa0+i+1, (chan[i].osc_mode << 1) | (chan[i + 1].output << 4));
          }

          rWrite(0xc0+i+1, (ES5503_wave_lengths_convert_back(chan[i].wave_size) << 3) | chan[i].address_bus_res); //set wave len

          if(chan[i].pcm || chan[i].wavetable_block != -1)
          {
            if(ins->es5503.phase_reset_on_start)
            {
              rWrite(0xA0 + i, (chan[i].osc_mode << 1) | 1 | (chan[i].output << 4)); //writing 1 resets acc
              rWrite(0xA0 + i, (chan[i].osc_mode << 1) | (chan[i].output << 4)); //writing 0 forces the reset
              rWrite(0xA0 + i + 1, (chan[i].osc_mode << 1) | 1 | (chan[i + 1].output << 4)); //writing 1 resets acc
              rWrite(0xA0 + i + 1, (chan[i].osc_mode << 1) | (chan[i + 1].output << 4)); //writing 0 forces the reset
            }
          }
        }

        else
        {
          rWrite(0x40+i, isMuted[i] ? 0 : chan[i].outVol); //set volume

          if(chan[i].pcm || chan[i].wavetable_block != -1)
          {
            rWrite(0x80+i, chan[i].wave_pos >> 8); //set wave pos
            rWrite(0xa0+i, (chan[i].osc_mode << 1) | (chan[i].output << 4));
          }

          rWrite(0xc0+i, (ES5503_wave_lengths_convert_back(chan[i].wave_size) << 3) | chan[i].address_bus_res); //set wave len

          if(chan[i].pcm || chan[i].wavetable_block != -1)
          {
            if(ins->es5503.phase_reset_on_start)
            {
              rWrite(0xA0 + i, (chan[i].osc_mode << 1) | 1 | (chan[i].output << 4)); //writing 1 resets acc
              rWrite(0xA0 + i, (chan[i].osc_mode << 1) | (chan[i].output << 4)); //writing 0 forces the reset
            }
          }
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

        if (chan[i].keyOff) chan[i].softpan_channel = false;
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

      chan[c.chan].osc_mode = ins->es5503.initial_osc_mode;

      if(chan[c.chan].wavetable_block != -1) //free block to hopefully properly reallocate it afterwards (see below)
      {
        wavetable_block_occupied[chan[c.chan].wavetable_block] = false;
        chan[c.chan].wavetable_block = -1;
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

        if(chan[c.chan].sample >= 0)
        {
          chan[c.chan].wave_size = sampleLengths[chan[c.chan].sample];
          chan[c.chan].wave_pos = sampleOffsets[chan[c.chan].sample];

          chan[c.chan].address_bus_res = ES5503_wave_lengths_convert_back(chan[c.chan].wave_size);
        }

        if(chan[c.chan].softpan_channel)
        {
          chan[c.chan + 1].osc_mode = ins->es5503.initial_osc_mode;

          if(chan[c.chan].sample >= 0)
          {
            chan[c.chan + 1].wave_size = sampleLengths[chan[c.chan].sample];
            chan[c.chan + 1].wave_pos = sampleOffsets[chan[c.chan].sample];

            chan[c.chan + 1].address_bus_res = ES5503_wave_lengths_convert_back(chan[c.chan].wave_size);
          }
        }
      }

      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;

      if(chan[c.chan].softpan_channel)
      {
        chan[c.chan + 1].active=true;
        chan[c.chan + 1].keyOn=true;

        chan[c.chan].output = 0;
        chan[c.chan + 1].output = 1; //force-reset outputs so softpanned channel works as expected
      }

      chan[c.chan].macroInit(ins);
      if (!chan[c.chan].std.get_div_macro_struct(DIV_MACRO_VOL)->will) {
        chan[c.chan].outVol=chan[c.chan].vol;
      }

      if (!chan[c.chan].pcm)
      {
        if (chan[c.chan].wave<0) 
        {
          chan[c.chan].wave=0;
        }

        chan[c.chan].ws.changeWave1(chan[c.chan].wave);

        chan[c.chan].ws.init(ins,256,255,chan[c.chan].insChanged);
        chan[c.chan].insChanged=false;

        chan[c.chan].wave_size = 0;

        bool found_block = false;

        for(int b = 0; b < reserved_blocks; b++)
        {
          if(!wavetable_block_occupied[b])
          {
            chan[c.chan].wave_pos = wavetable_blocks_offsets[b];
            wavetable_block_occupied[b] = true;
            found_block = true;
            chan[c.chan].wave_size = 256;
            chan[c.chan].wavetable_block = b;
            break;
          }
        }

        if(found_block)
        {
          if(chan[c.chan].softpan_channel)
          {
            chan[c.chan + 1].wave_size = 256;
            chan[c.chan + 1].wave_pos = chan[c.chan].wave_pos;
            chan[c.chan + 1].osc_mode = ins->es5503.initial_osc_mode;
          }
        }
      }
      
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].sample=-1;
      chan[c.chan].pcm=false; //???
      chan[c.chan].active=false;
      chan[c.chan].keyOff=true;
      chan[c.chan].macroInit(NULL);

      if(chan[c.chan].wavetable_block != -1)
      {
        wavetable_block_occupied[chan[c.chan].wavetable_block] = false;
        chan[c.chan].wavetable_block = -1;
      }

      if(chan[c.chan].softpan_channel)
      {
        chan[c.chan + 1].active=false;
        chan[c.chan + 1].keyOff=true;
        chan[c.chan + 1].macroInit(NULL);
      }
      break;
    case DIV_CMD_NOTE_OFF_ENV:
    case DIV_CMD_ENV_RELEASE:
      chan[c.chan].std.release();

      if(chan[c.chan].softpan_channel)
      {
        chan[c.chan + 1].std.release();
      }
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
        if (!chan[c.chan].std.get_div_macro_struct(DIV_MACRO_VOL)->has) {
          chan[c.chan].outVol=c.value;
          if (chan[c.chan].active) 
          {
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
      if (chan[c.chan].std.get_div_macro_struct(DIV_MACRO_VOL)->has) {
        return chan[c.chan].vol;
      }
      return chan[c.chan].outVol;
      break;
    case DIV_CMD_PANNING: {
      if(chan[c.chan].softpan_channel)
      {
        chan[c.chan].panleft = c.value;
        chan[c.chan].panright = c.value2;

        uint8_t temp = chan[c.chan].outVol * chan[c.chan].panleft / 255;
        rWrite(0x40+c.chan, isMuted[c.chan] ? 0 : temp);
        temp = chan[c.chan].outVol * chan[c.chan].panright / 255;
        rWrite(0x40+c.chan+1, isMuted[c.chan] ? 0 : temp);
      }
      break;
    }
    case DIV_CMD_PITCH:
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_LEGATO:
      chan[c.chan].baseFreq=NOTE_PERIODIC(c.value+((HACKY_LEGATO_MESS)?(chan[c.chan].std.get_div_macro_struct(DIV_MACRO_ARP)->val):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_ES5503));
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.brokenPortaArp && chan[c.chan].std.get_div_macro_struct(DIV_MACRO_ARP)->will && !NEW_ARP_STRAT) chan[c.chan].baseFreq=NOTE_PERIODIC(chan[c.chan].note);
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
    case DIV_CMD_MACRO_RESTART:
      chan[c.chan].std.restart(c.value);
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformES5503::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;

  if(isMuted[ch])
  {
    rWrite(0x40 + ch, 0); //force mute even for samples, I hope

    if(chan[ch].softpan_channel)
    {
      rWrite(0x40 + ch + 1, 0); //force mute even for samples, I hope
    }
  }

  else
  {
    if(chan[ch].softpan_channel)
    {
      uint8_t temp = chan[ch].outVol * chan[ch].panleft / 255;
      rWrite(0x40 + ch, isMuted[ch] ? 0 : (temp));
      temp = chan[ch].outVol * chan[ch].panright / 255;
      rWrite(0x40 + ch + 1, isMuted[ch] ? 0 : (temp));
    }

    else
    {
      rWrite(0x40 + ch, isMuted[ch] ? 0 : chan[ch].outVol);
    }
  }
}

void DivPlatformES5503::forceIns() {
  for (int i=0; i<32; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
    updateWave(i);

    if(chan[i].softpan_channel)
    {
      uint8_t temp = chan[i].outVol * chan[i].panleft / 255;
      rWrite(0x40+i,isMuted[i]?0:(temp));
      temp = chan[i].outVol * chan[i].panright / 255;
      rWrite(0x40+i+1,isMuted[i]?0:(temp));
    }

    else
    {
      rWrite(0x40+i,isMuted[i]?0:chan[i].outVol);
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

bool DivPlatformES5503::isSampleLoaded(int index, int sample) {
  if (index!=0) return false;
  if (sample<0 || sample>255) return false;
  return sampleLoaded[sample];
}

int DivPlatformES5503::is_enough_continuous_memory(int actualLength)
{
  int num_blocks = actualLength / 256 + 1;
  int result = -1;

  int jumpbl = jump_blocks(actualLength); //restrict possible starting positions to valid ones (depends on sample size)

  for(int i = 0; i < 256; i += jumpbl)
  {
    result = i; //index of 1st block

    for(int j = 0; j < num_blocks; j++)
    {
      if(!free_block[i + j]) //block is already occupied!
      {
        result = -1;
      }
    }

    if(result != -1) return result; //we found the 1st block
  }

  return result; //haven't found continuous memory for this sample so return -1
}

int DivPlatformES5503::count_free_blocks()
{
  int count = 0;

  for(int i = 0; i < 256; i++)
  {
    if(free_block[i])
    {
      count++;
    }
  }

  return count;
}

size_t DivPlatformES5503::getSampleMemUsage(int index) {
  return index == 0 ? (getSampleMemCapacity() - count_free_blocks() * 256) : 0;
}

void DivPlatformES5503::renderSamples(int sysID) {
  memset(es5503.sampleMem,0,getSampleMemCapacity());
  memset(sampleOffsets,0,256*sizeof(uint32_t));
  memset(sampleLoaded,0,256*sizeof(bool));
  memset(free_block,1,256*sizeof(bool));
  memset(sampleLengths,0,256*sizeof(uint32_t));

  memset(wavetable_blocks_offsets,0,32*sizeof(uint32_t));

  for(int size = 7; size > 0; size--) //first we place the longest samples then descend to shorter ones (bc placement limitations for longer samples)
  {
    int maxsize = ES5503_wave_lengths[size];
    int minsize = ES5503_wave_lengths[size - 1];

    for (int i = 0; i < parent->song.sampleLen; i++)
    {
      DivSample* s = parent->song.sample[i];

      if (!s->renderOn[0][sysID])
      {
        sampleOffsets[i] = 0;
        continue;
      }

      int length = s->getLoopEndPosition(DIV_SAMPLE_DEPTH_8BIT);

      if(length < 0) break;

      int actualLength = length + 8; //8 more bytes for trailing zeros
      int start_pos = 0;

      if (size != 1)
      {
        if (actualLength < minsize || actualLength >= maxsize) goto end;
      }

      else //for samples with <= 256 steps length
      {
        if (actualLength >= maxsize) goto end;
      }

      start_pos = is_enough_continuous_memory(actualLength);

      if(start_pos == -1 || reserved_blocks >= count_free_blocks())
      {
        logW("out of ES5503 PCM memory for sample %d!", i);
        break;
      }

      else
      {
        int actual_start_pos = start_pos * 256;

        for(int ss = 0; ss < length; ss++)
        {
          uint8_t val = (uint8_t)s->data8[ss] + 0x80;
          if (val == 0) val = 1;

          es5503.sampleMem[actual_start_pos + ss] = val;
        }

        int num_blocks = actualLength / 256 + 1;

        sampleLengths[i] = actualLength - 8;
        sampleLoaded[i] = true;
        sampleOffsets[i] = actual_start_pos;

        logW("sample %d length %d startpos %d", i, sampleLengths[i], sampleOffsets[i]);

        for(int b = start_pos; b < start_pos + num_blocks; b++)
        {
          free_block[b] = false;
        }
      }

      end:;
    }
  }

  for(int i = 0; i < reserved_blocks; i++)
  {
    int block_index = is_enough_continuous_memory(0);
    wavetable_blocks_offsets[i] = block_index * 256;
    free_block[block_index] = false;
  }
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
  //memset(es5503.sampleMem,0,es5503.sampleMemLen);
  for (uint8_t i=0; i<32; i++) {
    chan[i]=DivPlatformES5503::Channel();
    chan[i].std.setEngine(parent);
    chan[i].ws.setEngine(parent);
    chan[i].ws.init(NULL,256,255,false);

    chan[i].output = (i & 1) ? 1 : 0; //odd channels are left, even are right
    chan[i].softpan_channel = false;

    chan[i].wavetable_block = -1;
  }

  curChan=-1;
  // set volume to zero and reset some other shit
  for(uint8_t i = 0; i < 0x9f; i++)
  {
    es5503.write(i,0); //using es5503.write() instead of rWrite() so these aren't included in vgm (instead we use the addWrite(0xffffffff,0) for VGM export routine)
    //rWrite(i,0);
  }

  for(uint8_t i = 0xc0; i < 0xdf; i++)
  {
    es5503.write(i,0);
  }

  //write to num_osc register and enable all 32 oscillators
  es5503.write(0xe1,62);

  for(uint8_t i = 0xa0; i < 0xbf; i++) //odd channels are left, even are right
  {
    es5503.write(i, (chan[i - 0xa0].output << 4) | 1); //0<<4=left, 1<<4=right, 1=disable oscillator
  }

  memset(wavetable_block_occupied, 0, sizeof(bool) * 32);
}

int DivPlatformES5503::getOutputCount() {
  return mono ? 1 : 8;
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

bool DivPlatformES5503::getLegacyAlwaysSetVolume() {
  return true;
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

DivChannelPair DivPlatformES5503::getPaired(int ch) {
  if (chan[ch].softpan_channel && !(ch&1))
  {
    return DivChannelPair("SoftPan",ch+1);
  }
  return DivChannelPair();
}

void DivPlatformES5503::quit() {
  
  es5503.es5503_core_free();
  for (int i=0; i<32; i++) {
    delete oscBuf[i];
  }
}
