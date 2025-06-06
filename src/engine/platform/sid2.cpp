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

// thanks LTVA

#include "sid2.h"
#include "../engine.h"
#include "IconsFontAwesome4.h"
#include <math.h>
#include "../../ta-log.h"

#define rWrite(a,v) if (!skipRegisterWrites) {writes.push(QueuedWrite(a,v)); if (dumpWrites) {addWrite(a,v);} }

#define CHIP_FREQBASE 524288

const char* regCheatSheetSID2[]={
  "FreqL0", "00",
  "FreqH0", "01",
  "PWL0", "02",
  "PWH0Vol", "03",
  "Control0", "04",
  "AtkDcy0", "05",
  "StnRis0", "06",
  "FreqL1", "07",
  "FreqH1", "08",
  "PWL1", "09",
  "PWH1Vol", "0A",
  "Control1", "0B",
  "AtkDcy1", "0C",
  "StnRis1", "0D",
  "FreqL2", "0E",
  "FreqH2", "0F",
  "PWL2", "10",
  "PWH2Vol", "11",
  "Control2", "12",
  "AtkDcy2", "13",
  "StnRis2", "14",

  "FCL0Ctrl", "15",
  "FCH0", "16",
  "FilterRes0", "17",

  "FCL1Ctrl", "18",
  "FCH1", "19",
  "FilterRes1", "1A",

  "FCL2Ctrl", "1B",
  "FCH2", "1C",
  "FilterRes2", "1D",

  "NoiModeFrMSB01", "1E",
  "WaveMixModeFrMSB2", "1F",
  NULL
};

const char** DivPlatformSID2::getRegisterSheet() {
  return regCheatSheetSID2;
}

void DivPlatformSID2::acquire(short** buf, size_t len) {
  for (int i=0; i<3; i++) {
    oscBuf[i]->begin(len);
  }

  for (size_t i=0; i<len; i++) 
  {
    if (!writes.empty()) 
    {
      QueuedWrite w=writes.front();
      sid2->write(w.addr,w.val);
      regPool[w.addr&0x1f]=w.val;
      writes.pop();
    }
    
    sid2->clock();
    buf[0][i]=sid2->output();
    if (++writeOscBuf>=16) 
    {
      writeOscBuf=0;

      for(int j = 0; j < 3; j++)
      {
        int co=sid2->chan_out[j]>>2;
        if (co<-32768) co=-32768;
        if (co>32767) co=32767;
        oscBuf[j]->putSample(i,co);
      }
    }
  }

  for (int i=0; i<3; i++) {
    oscBuf[i]->end(len);
  }
}

void DivPlatformSID2::updateFilter(int channel) 
{
  rWrite(0x15 + 3 * channel,(chan[channel].filtCut&15) | ((chan[channel].filtControl & 7) << 4) | (chan[channel].filter << 7));
  rWrite(0x16 + 3 * channel,(chan[channel].filtCut >> 4));
  rWrite(0x17 + 3 * channel,chan[channel].filtRes);
}

void DivPlatformSID2::tick(bool sysTick) {
  for (int _i=0; _i<3; _i++) {
    int i=chanOrder[_i];

    bool willUpdateFilter = false;

    chan[i].std.next();

    if (sysTick) {
      if (chan[i].pw_slide!=0) {
        chan[i].duty-=chan[i].pw_slide;
        chan[i].duty=CLAMP(chan[i].duty,0,0xfff);
        rWrite(i*7+2,chan[i].duty&0xff);
        rWrite(i*7+3,(chan[i].duty>>8)|(chan[i].outVol<<4));
      }
      if (chan[i].cutoff_slide!=0) {
        chan[i].filtCut+=chan[i].cutoff_slide;
        chan[i].filtCut=CLAMP(chan[i].filtCut,0,0xfff);
        updateFilter(i);
      }
    }

    if (chan[i].std.vol.had) {
      chan[i].outVol=VOL_SCALE_LINEAR(chan[i].vol&15,MIN(15,chan[i].std.vol.val),15);
      rWrite(i*7+3,(chan[i].duty>>8) | (chan[i].outVol << 4));
    }

    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        chan[i].baseFreq=NOTE_FREQUENCY(parent->calcArp(chan[i].note,chan[i].std.arp.val));
      }
      chan[i].freqChanged=true;
    }
    if (chan[i].std.duty.had) {
      DivInstrument* ins=parent->getIns(chan[i].ins,DIV_INS_SID2);
      if (ins->c64.dutyIsAbs) {
        chan[i].duty=chan[i].std.duty.val;
      } else {
        chan[i].duty-=chan[i].std.duty.val;
      }
      chan[i].duty&=4095;
      rWrite(i*7+2,chan[i].duty&0xff);
      rWrite(i*7+3,(chan[i].duty>>8) | (chan[i].outVol << 4));
    }
    if (chan[i].std.wave.had) {
      chan[i].wave=chan[i].std.wave.val;
      rWrite(i*7+4,(chan[i].wave<<4)|0|(chan[i].ring<<2)|(chan[i].sync<<1)|(int)(chan[i].active && chan[i].gate));

      chan[i].freqChanged=true; //to update freq (if only noise was enabled/disabled)
    }
    if (chan[i].std.fms.had) {
      chan[i].noise_mode=chan[i].std.fms.val;
      rWrite(0x1e, (chan[0].noise_mode) | (chan[1].noise_mode << 2) | (chan[2].noise_mode << 4) | ((chan[0].freq >> 16) << 6) | ((chan[1].freq >> 16) << 7));

      chan[i].freqChanged=true; //to update freq (if only noise was enabled and periodic noise mode is set)
    }
    if (chan[i].std.ams.had) {
      chan[i].mix_mode=chan[i].std.ams.val;
      rWrite(0x1f, (chan[0].mix_mode) | (chan[1].mix_mode << 2) | (chan[2].mix_mode << 4) | ((chan[2].freq >> 16) << 6));
    }
    if (chan[i].std.pitch.had) {
      if (chan[i].std.pitch.mode) {
        chan[i].pitch2+=chan[i].std.pitch.val;
        CLAMP_VAR(chan[i].pitch2,-65535,65535);
      } else {
        chan[i].pitch2=chan[i].std.pitch.val;
      }
      chan[i].freqChanged=true;
    }
    if (chan[i].std.alg.had) { // new cutoff macro
      DivInstrument* ins=parent->getIns(chan[i].ins,DIV_INS_SID2);
      if (ins->c64.filterIsAbs) {
          chan[i].filtCut=MIN(4095,chan[i].std.alg.val);
      } else {
          chan[i].filtCut+=chan[i].std.alg.val;
        if (chan[i].filtCut > 4095) chan[i].filtCut = 4095;
        if (chan[i].filtCut<0) chan[i].filtCut=0;
      }
      willUpdateFilter=true;
    }
    if (chan[i].std.ex1.had) {
      chan[i].filtControl=chan[i].std.ex1.val&15;
      willUpdateFilter=true;
    }
    if (chan[i].std.ex2.had) {
      chan[i].filtRes=chan[i].std.ex2.val&255;
      willUpdateFilter=true;
    }
    if (chan[i].std.ex3.had) {
      chan[i].filter=(chan[i].std.ex3.val&1);
      willUpdateFilter=true;
    }
    if (chan[i].std.ex4.had) {
      chan[i].gate=chan[i].std.ex4.val&1;
      chan[i].sync=chan[i].std.ex4.val&2;
      chan[i].ring=chan[i].std.ex4.val&4;
      chan[i].freqChanged=true;
      rWrite(i*7+4,(chan[i].wave<<4)|0|(chan[i].ring<<2)|(chan[i].sync<<1)|(int)(chan[i].active && chan[i].gate));
    }

    if (chan[i].std.phaseReset.had) {
      chan[i].test=(chan[i].std.phaseReset.val&1);
      rWrite(i*7+4,(chan[i].wave<<4)|(chan[i].test << 3)|(chan[i].ring<<2)|(chan[i].sync<<1)|(int)(chan[i].active && chan[i].gate));
      rWrite(i*7+4,(chan[i].wave<<4)|0|(chan[i].ring<<2)|(chan[i].sync<<1)|(int)(chan[i].active && chan[i].gate));
      chan[i].test = false;
    }

    if (chan[i].std.ex5.had) {
      chan[i].attack=chan[i].std.ex5.val&15;
      rWrite(i*7+5,(chan[i].attack<<4)|(chan[i].decay));
    }

    if (chan[i].std.ex6.had) {
      chan[i].decay=chan[i].std.ex6.val&15;
      rWrite(i*7+5,(chan[i].attack<<4)|(chan[i].decay));
    }

    if (chan[i].std.ex7.had) {
      chan[i].sustain=chan[i].std.ex7.val&15;
      rWrite(i*7+6,(chan[i].sustain<<4)|(chan[i].release));
    }

    if (chan[i].std.ex8.had) {
      chan[i].release=chan[i].std.ex8.val&15;
      rWrite(i*7+6,(chan[i].sustain<<4)|(chan[i].release));
    }

    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,false,8,chan[i].pitch2,chipClock,CHIP_FREQBASE);
      if (chan[i].freq<0) chan[i].freq=0;
      if (chan[i].freq>0x1ffff) chan[i].freq=0x1ffff;
      if (chan[i].keyOn) 
      {
        if(!chan[i].resetMask)
        {
          chan[i].gate = true;
          
          rWrite(i*7+4,(chan[i].wave<<4)|0|(chan[i].ring<<2)|(chan[i].sync<<1)|(0));
          rWrite(i*7+4,(chan[i].wave<<4)|0|(chan[i].ring<<2)|(chan[i].sync<<1)|(chan[i].gate?1:0));
          rWrite(i*7+5,(chan[i].attack<<4)|(chan[i].decay));
          rWrite(i*7+6,(chan[i].sustain<<4)|(chan[i].release));
        }

        rWrite(i*7+3, (chan[i].duty>>8) | (chan[i].outVol << 4)); //set volume

        rWrite(0x1e, (chan[0].noise_mode) | (chan[1].noise_mode << 2) | (chan[2].noise_mode << 4) | ((chan[0].freq >> 16) << 6) | ((chan[1].freq >> 16) << 7));
        rWrite(0x1f, (chan[0].mix_mode) | (chan[1].mix_mode << 2) | (chan[2].mix_mode << 4) | ((chan[2].freq >> 16) << 6));
      }
      if (chan[i].keyOff) 
      {
        chan[i].gate = false;
        rWrite(i*7+5,(chan[i].attack<<4)|(chan[i].decay));
        rWrite(i*7+6,(chan[i].sustain<<4)|(chan[i].release));
        rWrite(i*7+4,(chan[i].wave<<4)|0|(chan[i].ring<<2)|(chan[i].sync<<1)|0);
      }

      if(chan[i].wave == 0x8) //if we have noise (noise only, since tone frequency would be wrong)
      {
        if(chan[i].noise_mode == 1) //1st short noise
        {
          chan[i].freq = (int)((double)chan[i].freq * 523.25 / 349.0); //these numbers and later determined by spectrum analyzer peak of looped noise signal at known frequency (frequency known for tone that would play if tone wave was enabled)
        }

        if(chan[i].noise_mode == 2) //2nd short noise
        {
          chan[i].freq = (int)((double)chan[i].freq * 523.25 / 270.0);
        }

        if(chan[i].noise_mode == 3) //3rd short noise
        {
          chan[i].freq = (int)((double)chan[i].freq * 523.25 / 133.0);
        }
      }

      if (chan[i].freq<0) chan[i].freq=0;
      if (chan[i].freq>0x1ffff) chan[i].freq=0x1ffff;

      rWrite(i*7,chan[i].freq&0xff);
      rWrite(i*7+1,chan[i].freq>>8);
      rWrite(0x1e, (chan[0].noise_mode) | (chan[1].noise_mode << 2) | (chan[2].noise_mode << 4) | ((chan[0].freq >> 16) << 6) | ((chan[1].freq >> 16) << 7));
      rWrite(0x1f, (chan[0].mix_mode) | (chan[1].mix_mode << 2) | (chan[2].mix_mode << 4) | ((chan[2].freq >> 16) << 6));
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }

    if (willUpdateFilter) updateFilter(i);
  }
}

int DivPlatformSID2::dispatch(DivCommand c) {
  if (c.chan>2) return 0;
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_SID2);
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].test=false;

      if (((chan[c.chan].insChanged || chan[c.chan].resetDuty || ins->std.waveMacro.len>0) && ins->c64.resetDuty) || chan[c.chan].resetDuty) {
        chan[c.chan].duty=ins->c64.duty;
        rWrite(c.chan*7+2,chan[c.chan].duty&0xff);
        rWrite(c.chan*7+3,(chan[c.chan].duty>>8) | (chan[c.chan].outVol << 4));
      }
      if (chan[c.chan].insChanged) {
        chan[c.chan].wave=(ins->c64.noiseOn<<3)|(ins->c64.pulseOn<<2)|(ins->c64.sawOn<<1)|(int)(ins->c64.triOn);
        chan[c.chan].attack=ins->c64.a;
        chan[c.chan].decay=(ins->c64.s==15)?0:ins->c64.d;
        chan[c.chan].sustain=ins->c64.s;
        chan[c.chan].release=ins->c64.r;
        chan[c.chan].ring=ins->c64.ringMod;
        chan[c.chan].sync=ins->c64.oscSync;

        chan[c.chan].noise_mode = ins->sid2.noiseMode;
        chan[c.chan].mix_mode = ins->sid2.mixMode;
      }
      if (chan[c.chan].insChanged || chan[c.chan].resetFilter) {
        chan[c.chan].filter=ins->c64.toFilter;
        if (ins->c64.initFilter) {
          chan[c.chan].filtCut=ins->c64.cut;
          chan[c.chan].filtRes=ins->c64.res;
          chan[c.chan].filtControl=(int)(ins->c64.lp)|(ins->c64.bp<<1)|(ins->c64.hp<<2);
        }
        updateFilter(c.chan);
      }
      if (chan[c.chan].insChanged) {
        chan[c.chan].insChanged=false;
      }
      if (keyPriority) {
        if (chanOrder[1]==c.chan) {
          chanOrder[1]=chanOrder[2];
          chanOrder[2]=c.chan;
        } else if (chanOrder[0]==c.chan) {
          chanOrder[0]=chanOrder[1];
          chanOrder[1]=chanOrder[2];
          chanOrder[2]=c.chan;
        }
      }
      chan[c.chan].macroInit(ins);
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].active=false;
      chan[c.chan].keyOff=true;
      chan[c.chan].keyOn=false;
      //chan[c.chan].macroInit(NULL);
      break;
    case DIV_CMD_NOTE_OFF_ENV:
      chan[c.chan].active=false;
      chan[c.chan].keyOff=true;
      chan[c.chan].keyOn=false;
      chan[c.chan].std.release();
      break;
    case DIV_CMD_ENV_RELEASE:
      chan[c.chan].std.release();
      break;
    case DIV_CMD_INSTRUMENT:
      if (chan[c.chan].ins!=c.value || c.value2==1) {
        chan[c.chan].insChanged=true;
        chan[c.chan].ins=c.value;
      }
      break;
    case DIV_CMD_VOLUME:
      if (chan[c.chan].vol!=c.value) {
        chan[c.chan].vol=c.value;
        if (!chan[c.chan].std.vol.has) {
          chan[c.chan].outVol=c.value;
          chan[c.chan].vol=chan[c.chan].outVol;
          rWrite(c.chan*7+3,(chan[c.chan].duty>>8) | (chan[c.chan].vol << 4));
        }
      }
      break;
    case DIV_CMD_GET_VOLUME:
      if (chan[c.chan].std.vol.has) {
        return chan[c.chan].vol;
      }
      return chan[c.chan].outVol;
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
      if (return2) {
        chan[c.chan].inPorta=false;
        return 2;
      }
      break;
    }
    case DIV_CMD_C64_FINE_DUTY:
      chan[c.chan].duty=c.value;
      rWrite(c.chan*7+2,chan[c.chan].duty&0xff);
      rWrite(c.chan*7+3,(chan[c.chan].duty>>8) | (chan[c.chan].outVol << 4));
      break;
    case DIV_CMD_WAVE:
      chan[c.chan].wave=c.value;
      rWrite(c.chan*7+4,(chan[c.chan].wave<<4)|0|(chan[c.chan].ring<<2)|(chan[c.chan].sync<<1)|(int)(chan[c.chan].active && chan[c.chan].gate));
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_LEGATO:
      chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta || parent->song.preNoteNoEffect) {
          chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_SID2));
          chan[c.chan].keyOn=true;
        }
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) chan[c.chan].baseFreq=NOTE_FREQUENCY(chan[c.chan].note);
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 15;
      break;
    case DIV_CMD_C64_FINE_CUTOFF:
      chan[c.chan].filtCut=c.value;
      updateFilter(c.chan);
      break;
    case DIV_CMD_C64_RESONANCE:
      if (c.value>255) c.value=255;
      chan[c.chan].filtRes=c.value;
      updateFilter(c.chan);
      break;
    case DIV_CMD_C64_FILTER_MODE:
      chan[c.chan].filtControl=c.value&7;
      updateFilter(c.chan);
      break;
    case DIV_CMD_C64_RESET_MASK:
      chan[c.chan].resetMask=c.value;
      break;
    case DIV_CMD_C64_FILTER_RESET:
      if (c.value&15) {
        DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_SID2);
        if (ins->c64.initFilter) {
          chan[c.chan].filtCut=ins->c64.cut;
          updateFilter(c.chan);
        }
      }
      chan[c.chan].resetFilter=c.value>>4;
      break;
    case DIV_CMD_C64_DUTY_RESET:
      if (c.value&15) {
        DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_SID2);
        chan[c.chan].duty=ins->c64.duty;
        rWrite(c.chan*7+2,chan[c.chan].duty&0xff);
        rWrite(c.chan*7+3,(chan[c.chan].duty>>8) | (chan[c.chan].outVol << 4));
      }
      chan[c.chan].resetDuty=c.value>>4;
      break;
    case DIV_CMD_C64_EXTENDED:
      switch (c.value>>4) {
        case 0:
          chan[c.chan].attack=c.value&15;
          rWrite(c.chan*7+5,(chan[c.chan].attack<<4)|(chan[c.chan].decay));
          break;
        case 1:
          chan[c.chan].decay=c.value&15;
          rWrite(c.chan*7+5,(chan[c.chan].attack<<4)|(chan[c.chan].decay));
          break;
        case 2:
          chan[c.chan].sustain=c.value&15;
          rWrite(c.chan*7+6,(chan[c.chan].sustain<<4)|(chan[c.chan].release));
          break;
        case 3:
          chan[c.chan].release=c.value&15;
          rWrite(c.chan*7+6,(chan[c.chan].sustain<<4)|(chan[c.chan].release));
          break;
        case 4:
          chan[c.chan].ring=c.value;
          rWrite(c.chan*7+4,(chan[c.chan].wave<<4)|0|(chan[c.chan].ring<<2)|(chan[c.chan].sync<<1)|(int)(chan[c.chan].active && chan[c.chan].gate));
          break;
        case 5:
          chan[c.chan].sync=c.value;
          rWrite(c.chan*7+4,(chan[c.chan].wave<<4)|0|(chan[c.chan].ring<<2)|(chan[c.chan].sync<<1)|(int)(chan[c.chan].active && chan[c.chan].gate));
          break;
        case 6:
          chan[c.chan].filtControl = c.value;
          updateFilter(c.chan);
          break;
        case 7:
          chan[c.chan].mix_mode=(c.value & 3);
          rWrite(0x1f, (chan[0].mix_mode) | (chan[1].mix_mode << 2) | (chan[2].mix_mode << 4) | ((chan[2].freq >> 16) << 6));
          break;
        case 8:
          chan[c.chan].noise_mode=(c.value & 3);
          chan[c.chan].freqChanged = true;
          rWrite(0x1e, (chan[0].noise_mode) | (chan[1].noise_mode << 2) | (chan[2].noise_mode << 4) | ((chan[0].freq >> 16) << 6) | ((chan[1].freq >> 16) << 7));
          break;
        case 9: //phase reset
          chan[c.chan].test=true;
          rWrite(c.chan*7+4,(chan[c.chan].wave<<4)|(chan[c.chan].test << 3)|(chan[c.chan].ring<<2)|(chan[c.chan].sync<<1)|(int)(chan[c.chan].active && chan[c.chan].gate));
          rWrite(c.chan*7+4,(chan[c.chan].wave<<4)|0|(chan[c.chan].ring<<2)|(chan[c.chan].sync<<1)|(int)(chan[c.chan].active && chan[c.chan].gate));
          chan[c.chan].test = false;
          break;
        case 0xa: //envelope on/off
          chan[c.chan].gate=(c.value & 1);
          rWrite(c.chan*7+4,(chan[c.chan].wave<<4)|0|(chan[c.chan].ring<<2)|(chan[c.chan].sync<<1)|(int)(chan[c.chan].active && chan[c.chan].gate));
          break;
        case 0xb: //filter on/off
          chan[c.chan].filter=(c.value & 1);
          updateFilter(c.chan);
          break;
      }
      break;
    case DIV_CMD_C64_PW_SLIDE:
      chan[c.chan].pw_slide=c.value*c.value2;
      break;
    case DIV_CMD_C64_CUTOFF_SLIDE:
      chan[c.chan].cutoff_slide=c.value*c.value2;
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

void DivPlatformSID2::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  sid2->set_is_muted(ch,mute);
}

void DivPlatformSID2::forceIns() {
  for (int i=0; i<3; i++) {
    chan[i].insChanged=true;
    if (chan[i].active) {
      chan[i].keyOn=true;
      chan[i].freqChanged=true;
    }
    updateFilter(i);
  }
}

void DivPlatformSID2::notifyInsChange(int ins) {
  for (int i=0; i<3; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformSID2::notifyInsDeletion(void* ins) {
  for (int i=0; i<3; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void* DivPlatformSID2::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformSID2::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

void DivPlatformSID2::getPaired(int ch, std::vector<DivChannelPair>& ret) {
  if (chan[ch].ring) {
    if (ch==0) {
      ret.push_back(DivChannelPair(_("ring"),2));
    } else {
      ret.push_back(DivChannelPair(_("ring"),(ch-1)%3));
    }
  }
  if (chan[ch].sync) {
    if (ch==0) {
      ret.push_back(DivChannelPair(_("sync"),2));
    } else {
      ret.push_back(DivChannelPair(_("sync"),(ch-1)%3));
    }
  }
}

DivChannelModeHints DivPlatformSID2::getModeHints(int ch) {
  DivChannelModeHints ret;
  ret.count=1;
  ret.hint[0]=ICON_FA_BELL_SLASH_O;
  ret.type[0]=0;
  if (ch==2 && (chan[ch].filtControl & 8)) {
    ret.type[0]=7;
  } else if (!chan[ch].gate) {
    ret.type[0]=4;
  }

  return ret;
}

DivDispatchOscBuffer* DivPlatformSID2::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformSID2::getRegisterPool() {
  return regPool;
}

int DivPlatformSID2::getRegisterPoolSize() {
  return 32;
}

bool DivPlatformSID2::getDCOffRequired() {
  return true;
}

bool DivPlatformSID2::getWantPreNote() {
  return true;
}

bool DivPlatformSID2::isVolGlobal() {
  return true;
}

float DivPlatformSID2::getPostAmp() {
  return 1.0f;
}

void DivPlatformSID2::reset() {
  while (!writes.empty()) writes.pop();
  for (int i=0; i<3; i++) {
    chan[i]=DivPlatformSID2::Channel();
    chan[i].std.setEngine(parent);
    fakeLow[i]=0;
    fakeBand[i]=0;
    chan[i].vol = 15;

    chan[i].filtControl = 7;
    chan[i].filtRes = 0;
    chan[i].filtCut = 4095;

    chan[i].noise_mode = 0;

    rWrite(0x3 + 7 * i,0xf0);

    chan[i].cutoff_slide = 0;
    chan[i].pw_slide = 0;
  }

  sid2->reset();
  memset(regPool,0,32);

  chanOrder[0]=0;
  chanOrder[1]=1;
  chanOrder[2]=2;
}

void DivPlatformSID2::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformSID2::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

void DivPlatformSID2::setFlags(const DivConfig& flags) {
  switch (flags.getInt("clockSel",0)) {
    case 0x0: // NTSC C64
      chipClock=COLOR_NTSC*2.0/7.0;
      break;
    case 0x1: // PAL C64
      chipClock=COLOR_PAL*2.0/9.0;
      break;
    case 0x2: // SSI 2001
    default:
      chipClock=14318180.0/16.0;
      break;
  }
  CHECK_CUSTOM_CLOCK;
  rate=chipClock;
  for (int i=0; i<3; i++) {
    oscBuf[i]->setRate(rate);
  }
  keyPriority=flags.getBool("keyPriority",true);

  // init fake filter table
  // taken from dSID
  double cutRatio=-2.0*3.14*(12500.0/256.0)/(double)oscBuf[0]->rate;

  for (int i=0; i<4095; i++) 
  {
    double c=(double)i/16.0+0.2;
    c=1-exp(c*cutRatio);
    fakeCutTable[i]=c;
  }
}

int DivPlatformSID2::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  needInitTables=true;
  writeOscBuf=0;
  
  for (int i=0; i<3; i++) 
  {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }

  sid2=new SID2;

  sid2->set_chip_model(MOS8580_2);

  setFlags(flags);

  reset();

  return 3;
}

void DivPlatformSID2::quit() {
  for (int i=0; i<3; i++) 
  {
    delete oscBuf[i];
  }
  if (sid2!=NULL) delete sid2;
}

DivPlatformSID2::~DivPlatformSID2() {
}
