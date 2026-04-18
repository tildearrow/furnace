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

// pce.cpp: DivPlatformPCE code.

#include "pce.h"
#include "../engine.h"
#include "furIcons.h"
#include <math.h>

// the following macros are used to add into the reg write queue:
// rWrite(): schedule a direct write to some register.
// chWrite(): write to a channel's registers.
// - this writes to the channel select register first (if needed).
// observe how we honor skipRegisterWrites (set during playback skip).

//#define rWrite(a,v) pendingWrites[a]=v;
#define rWrite(a,v) if (!skipRegisterWrites) {writes.push(QueuedWrite(a,v)); if (dumpWrites) {addWrite(a,v);} }
#define chWrite(c,a,v) \
  if (!skipRegisterWrites) { \
    if (curChan!=c) { \
      curChan=c; \
      rWrite(0,curChan); \
    } \
    regPool[16+((c)<<4)+((a)&0x0f)]=v; \
    rWrite(a,v); \
  }

// use CHIP_DIVIDER for period-based chips.
// use CHIP_FREQBASE for accumulator/frequency-based chips.
// adjust until C-4 sounds like C-4.
// this must be defined at the beginning!
#define CHIP_DIVIDER 64

// our register cheatsheet, displayed in the debug menu.
// the format is "RegName", "Address"
const char* regCheatSheetPCE[]={
  "Select", "0",
  "MasterVol", "1",
  "FreqL", "2",
  "FreqH", "3",
  "DataCtl", "4",
  "ChanVol", "5",
  "WaveCtl", "6",
  "NoiseCtl", "7",
  "LFOFreq", "8",
  "LFOCtl", "9",
  NULL
};

const char** DivPlatformPCE::getRegisterSheet() {
  return regCheatSheetPCE;
}

// this function is called by the engine during mixing.
// it runs emulation code, flushes the reg write queue and fills in audio buffers.
// since this DivPlatform supports acquireDirect(), this function is empty.
// however I will make sure to put something in here soon so you get an idea of how should it be.
void DivPlatformPCE::acquire(short** buf, size_t len) {
}

// this function is called by the engine during mixing.
// the DivDispatch must inform the engine that acquireDirect() is implemented. see hasAcquireDirect().
//
// the difference is that you get a direct pointer to the blip_buffer. this allows optimizations to take place.
// if your emulation code cannot be optimized and always outputs something each sample, use acquire() instead.
void DivPlatformPCE::acquireDirect(blip_buffer_t** bb, size_t len) {
  // prepare chan osc buffers
  for (int i=0; i<6; i++) {
    oscBuf[i]->begin(len);
    // in this case I've modified the emulator so that it takes advantage of the osc buffers and uses them.
    pce->channel[i].oscBuf=oscBuf[i];
  }

  // pass our blip_buffers to the emulator.
  pce->bb[0]=bb[0];
  pce->bb[1]=bb[1];

  // now we can start emulation.
  // reset the emulator's "timestamp" and begin...
  size_t pos=0;
  pce->ResetTS(pos);

  // flush the write queue
  while (!writes.empty()) {
    QueuedWrite w=writes.front();
    pce->Write(pos,w.addr,w.val);
    regPool[w.addr&0x0f]=w.val;
    writes.pop();
  }

  // begin the buffer filling process
  for (size_t h=0; h<len;) {
    // calculate how many samples we can advance.
    // the initial value is the number of samples left to fill.
    // the reason why don't we just run the emulator until the end is because we support software PCM,
    // so there may be a sample change at some point.
    int advance=len-h;
    // we use an heuristic to determine how many samples remain until one of the channels' software PCM state ticks.
    int remainTime=0;
    for (int i=0; i<6; i++) {
      // only if we have a sample
      if (chan[i].pcm && chan[i].dacSample!=-1) {
        // prevent a division by zero
        if (chan[i].dacRate<=0) continue;
        // calculate remaining time and reduce advance if needed
        remainTime=(rate-chan[i].dacPeriod+chan[i].dacRate-1)/chan[i].dacRate;
        if (remainTime<advance) advance=remainTime;
        if (remainTime<1) advance=1;
      }
    }

    // PCM part
    // this is a software PCM implementation. it uses PCE's direct mode to output a 5-bit sample.
    for (int i=0; i<6; i++) {
      // run a channel
      if (chan[i].pcm && chan[i].dacSample!=-1) {
        // increase the accumulator (multiplied by advance)
        chan[i].dacPeriod+=chan[i].dacRate*advance;
        // if our accumulator is high enough, we need to output the next sample
        if (chan[i].dacPeriod>=rate) {
          DivSample* s=parent->getSample(chan[i].dacSample);
          // if we're out of range, stop the sample
          if (s->samples<=0 || chan[i].dacPos>=s->samples) {
            chan[i].dacSample=-1;
            continue;
          }
          // disable the output I believe
          chWrite(i,0x07,0);
          // prepare our sample (take a signed 8-bit sample and make it unsigned 5-bit)
          signed char dacData=((signed char)((unsigned char)s->data8[chan[i].dacPos]^0x80))>>3;
          chan[i].dacOut=CLAMP(dacData,-16,15);
          if (!isMuted[i]) {
            // now write the sample
            chWrite(i,0x04,parent->song.compatFlags.disableSampleMacro?0xdf:(0xc0|chan[i].outVol));
            chWrite(i,0x06,chan[i].dacOut&0x1f);
          } else {
            // we're muted - write silence
            chWrite(i,0x04,0xc0);
            chWrite(i,0x06,0x10);
          }
          // advance current sample position
          chan[i].dacPos++;
          // loop the sample if we've reached the end
          // otherwise stop it
          if (s->isLoopable() && chan[i].dacPos>=(unsigned int)s->loopEnd) {
            chan[i].dacPos=s->loopStart;
          } else if (chan[i].dacPos>=s->samples) {
            chan[i].dacSample=-1;
          }
          // adjust the accumulator
          chan[i].dacPeriod-=rate;
        }
      }
    }
  
    // PCE part
    // move the current position
    pos+=advance;

    // flush register writes
    // the emulator's Write function will run the output as needed
    while (!writes.empty()) {
      QueuedWrite w=writes.front();
      pce->Write(pos,w.addr,w.val);
      regPool[w.addr&0x0f]=w.val;
      writes.pop();
    }

    // move the position in our buffer
    h+=advance;
  }
  // tell the emulator to output until the end of this frame
  pce->Update(pos);

  // tell our chan osc buffers we're done
  for (int i=0; i<6; i++) {
    oscBuf[i]->end(len);
  }
}

// this function updates the waveform of a channel.
void DivPlatformPCE::updateWave(int ch) {
  // if we're in PCM mode, schedule a wave update.
  // we can't change the waveform while on it.
  if (chan[ch].pcm) {
    chan[ch].deferredWaveUpdate=true;
    return;
  }

  // turn the channel off, and prepare it for loading a waveform
  chWrite(ch,0x04,0x5f);
  chWrite(ch,0x04,0x1f);
  // write the new waveform
  // we take the wave synth's output. this also manages our waveforms when disabled.
  // the waveform is shifted by the predicted current position (if anti-click is enabled).
  for (int i=0; i<32; i++) {
    chWrite(ch,0x06,chan[ch].ws.output[(i+chan[ch].antiClickWavePos)&31]);
  }
  chan[ch].antiClickWavePos&=31;
  // re-enable output if the channel is active
  if (chan[ch].active) {
    chWrite(ch,0x04,0x80|chan[ch].outVol);
  }
  // acknowledge a pending wave update
  if (chan[ch].deferredWaveUpdate) {
    chan[ch].deferredWaveUpdate=false;
  }
}

// this table is here for compatibility.
// in DefleMask, 12 preset noise pitches are mapped to each octave.
// TODO: in octave 6 the noise table changes to a tonal one
static unsigned char noiseFreq[12]={
  4,13,15,18,21,23,25,27,29,31,0,2  
};

// this is called on every engine tick.
// it runs macros and updates frequencies/note status.
void DivPlatformPCE::tick(bool sysTick) {
  for (int i=0; i<6; i++) {
    // anti-click
    // on each song tick, we estimate the current wave position:
    // 1. calculate clocks per tick
    // 2. divide this by the current period
    // 3. add this to the wave position accumulator
    if (antiClickEnabled && sysTick && chan[i].freq>0) {
      chan[i].antiClickPeriodCount+=((chipClock>>1)/MAX(parent->getCurHz(),1.0f));
      chan[i].antiClickWavePos+=chan[i].antiClickPeriodCount/chan[i].freq;
      chan[i].antiClickPeriodCount%=chan[i].freq;
    }

    // run the macro interpreter, and process its results
    chan[i].std.next();
    // volume macro
    if (chan[i].std.vol.had) {
      // PCE volume is logarithmic.
      // we use BROKEN for compatibility with DefleMask (which implement volume calc in an odd way).
      chan[i].outVol=VOL_SCALE_LOG_BROKEN(chan[i].vol&31,MIN(31,chan[i].std.vol.val),31);
      // write the output volume, setting the DAC flag appropriately.
      if (chan[i].pcm) {
        chWrite(i,0x04,0xc0|chan[i].outVol);
      } else {
        chWrite(i,0x04,0x80|chan[i].outVol);
      }
    }
    // noise mode macro (last two channels only)
    // usually we put this after checking the arp macro, but noise mode may influence frequency calculation, so we do this first.
    if (chan[i].std.duty.had && i>=4) {
      chan[i].noise=chan[i].std.duty.val;
      chan[i].freqChanged=true;
    }
    // arp macro
    // on linear pitch, we can use the new arp strategy helper
    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      // handle noise (mapped to current note)
      // and calculate the new frequency
      if (!chan[i].inPorta) {
        int noiseSeek=parent->calcArp(chan[i].note,chan[i].std.arp.val);
        chan[i].baseFreq=NOTE_PERIODIC(noiseSeek);
        if (noiseSeek<0) noiseSeek=0;
        chan[i].noiseSeek=noiseSeek;
      }
      // prepare for a freq recalculation
      chan[i].freqChanged=true;
    }
    // wave macro (wave mode only)
    if (chan[i].std.wave.had && !chan[i].pcm) {
      // only if the macro's value has changed
      if (chan[i].wave!=chan[i].std.wave.val || chan[i].ws.activeChanged()) {
        // tell the wave synth to perform a wave change
        chan[i].wave=chan[i].std.wave.val;
        chan[i].ws.changeWave1(chan[i].wave);
        if (!chan[i].keyOff) chan[i].keyOn=true;
      }
    }
    // pan left macro
    if (chan[i].std.panL.had) {
      // update left portion of pan register
      chan[i].pan&=0x0f;
      chan[i].pan|=(chan[i].std.panL.val&15)<<4;
    }
    // pan right macro
    if (chan[i].std.panR.had) {
      // update right portion of pan register
      chan[i].pan&=0xf0;
      chan[i].pan|=chan[i].std.panR.val&15;
    }
    // if either macro is active, perform register write.
    if (chan[i].std.panL.had || chan[i].std.panR.had) {
      chWrite(i,0x05,isMuted[i]?0:chan[i].pan);
    }
    // pitch macro
    if (chan[i].std.pitch.had) {
      if (chan[i].std.pitch.mode) {
        // macro is relative (add to the previous value)
        chan[i].pitch2+=chan[i].std.pitch.val;
        // limit the range to a 16-bit number
        CLAMP_VAR(chan[i].pitch2,-32768,32767);
      } else {
        // macro is absolute
        chan[i].pitch2=chan[i].std.pitch.val;
      }
      // prepare for a freq recalculation
      chan[i].freqChanged=true;
    }
    // phase reset macro
    if (chan[i].std.phaseReset.had && chan[i].std.phaseReset.val==1) {
      // in PCM mode, we restart the sample.
      if (chan[i].pcm) {
        if (chan[i].active && chan[i].dacSample>=0 && chan[i].dacSample<parent->song.sampleLen) {
          if (chan[i].setPos) {
            chan[i].setPos=false;
          } else {
            chan[i].dacPos=0;
          }
          chan[i].dacPeriod=0;
          chWrite(i,0x04,parent->song.compatFlags.disableSampleMacro?0xdf:(0xc0|chan[i].vol));
          // tell the engine we're playing a software PCM sample
          if (dumpWrites) addWrite(0xffff0000+(i<<8),chan[i].dacSample);
          chan[i].keyOn=true;
        }
      }
      // to accomplish a phase reset with anti-click enable, we simply reset anti-click state.
      chan[i].antiClickWavePos=0;
      chan[i].antiClickPeriodCount=0;
    }
    // check whether we should update the waveform:
    // - if we changed the wave
    // - if the wave synth has a new result
    // - if phase reset has occurred
    // - if a wave update is pending (because a wave change occurred while we were in PCM mode)
    if (chan[i].active) {
      if (chan[i].ws.tick() || (chan[i].std.phaseReset.had && chan[i].std.phaseReset.val==1) || chan[i].deferredWaveUpdate) {
        updateWave(i);
      }
    }

    // final frequency calculation
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      // this function here calculates the frequency.
      // it looks terrible I know. I gotta refactor this...
      //DivInstrument* ins=parent->getIns(chan[i].ins,DIV_INS_PCE);
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,true,0,chan[i].pitch2,chipClock,CHIP_DIVIDER);
      // in PCM mode, a multiplier is applied depending on the sample's rate.
      if (chan[i].pcm) {
        double off=1.0;
        if (chan[i].dacSample>=0 && chan[i].dacSample<parent->song.sampleLen) {
          DivSample* s=parent->getSample(chan[i].dacSample);
          if (s->centerRate<1) {
            off=1.0;
          } else {
            off=parent->getCenterRate()/(double)s->centerRate;
          }
        }
        // we set the accumulator rate
        chan[i].dacRate=(double)chipClock/(4*MAX(1,off*chan[i].freq));
        // if we're dumping writes, tell the engine we have a software PCM rate change
        if (dumpWrites) addWrite(0xffff0001+(i<<8),chan[i].dacRate);
      }
      // clamp the final period...
      if (chan[i].freq<1) chan[i].freq=1;
      if (chan[i].freq>4096) chan[i].freq=4096;
      // ...and write it
      chWrite(i,0x02,chan[i].freq&0xff);
      chWrite(i,0x03,(chan[i].freq>>8)&0xf);

      // if we're in noise mode, calculate the noise pitch value.
      if (i>=4) {
        int noiseSeek=(chan[i].fixedArp?chan[i].baseNoteOverride:(chan[i].note+chan[i].arpOff))+chan[i].pitch2;
        if (!parent->song.compatFlags.properNoiseLayout && noiseSeek<0) noiseSeek=0;
        if (!NEW_ARP_STRAT) {
          noiseSeek=chan[i].noiseSeek;
        }
        // write the noise frequency
        // in compatible noise layout, we read from the table. otherwise just write the note AND 31.
        chWrite(i,0x07,chan[i].noise?(0x80|(parent->song.compatFlags.properNoiseLayout?(noiseSeek&31):noiseFreq[noiseSeek%12])):0);
      }
      // this code is unused...
      if (chan[i].keyOn) {
        //rWrite(16+i*5,0x80);
        //chWrite(i,0x04,0x80|chan[i].vol);
      }
      // silence the channel upon note off
      if (chan[i].keyOff) {
        chWrite(i,0x04,0);
      }
      // acknowledge changes
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
  }
  // if we have a scheduled LFO register update, do it now.
  if (updateLFO) {
    rWrite(0x08,lfoSpeed); // TODO: 0 is treated to 256 in chip
    rWrite(0x09,lfoMode);
    updateLFO=false;
  }
}

// this function handles incoming commands.
int DivPlatformPCE::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_PCE);
      // set the volume macro range
      chan[c.chan].macroVolMul=ins->type==DIV_INS_AMIGA?64:31;
      if (ins->type==DIV_INS_AMIGA || ins->amiga.useSample) {
        // enter PCM mode if we found a Generic Sample instrument or sample mode is on
        chan[c.chan].pcm=true;
      } else {
        // get out of sample mode and reset its state
        chan[c.chan].pcm=false;
        chan[c.chan].sampleNote=DIV_NOTE_NULL;
        chan[c.chan].sampleNoteDelta=0;
        // let the engine know we stopped software sample mode
        if (dumpWrites) addWrite(0xffff0002+(c.chan<<8),0);
      }
      // I've split the note handler into two sections. this here handles notes in PCM mode.
      if (chan[c.chan].pcm) {
        // ignore notes when seeking
        if (skipRegisterWrites) break;
        // a "null" note is a special type of note that occurs during the retrigger effect.
        // when that happens, the current frequency/note should not change.
        // this allows you to slide while retriggering.
        if (c.value!=DIV_NOTE_NULL) {
          // set the current note, and remap it using the sample map.
          // this is important for the next part.
          chan[c.chan].dacSample=ins->amiga.getSample(c.value);
          chan[c.chan].sampleNote=c.value;
          c.value=ins->amiga.getFreq(c.value);
          chan[c.chan].sampleNoteDelta=c.value-chan[c.chan].sampleNote;
        } else if (chan[c.chan].sampleNote!=DIV_NOTE_NULL) {
          // if we have a null note, use the previous note as long as we've played a note before.
          chan[c.chan].dacSample=ins->amiga.getSample(chan[c.chan].sampleNote);
          c.value=ins->amiga.getFreq(chan[c.chan].sampleNote);
        }
        // now ready the sample
        // if it doesn't exist or is out of range, stop playing.
        if (chan[c.chan].dacSample<0 || chan[c.chan].dacSample>=parent->song.sampleLen) {
          chan[c.chan].dacSample=-1;
          if (dumpWrites) addWrite(0xffff0002+(c.chan<<8),0);
          break;
        } else {
           // start PCM mode and tell the engine we're playing a sample
           if (dumpWrites) {
             chWrite(c.chan,0x04,parent->song.compatFlags.disableSampleMacro?0xdf:(0xc0|chan[c.chan].vol));
             addWrite(0xffff0000+(c.chan<<8),chan[c.chan].dacSample);
           }
        }
        // reset sample position.
        // we acknowledge a sample offset/position command (DIV_CMD_SAMPLE_POS).
        // we use setPos because you may have placed a sample position effect alongside a note.
        if (chan[c.chan].setPos) {
          chan[c.chan].setPos=false;
        } else {
          chan[c.chan].dacPos=0;
        }
        // reset the sample playback accunulator
        chan[c.chan].dacPeriod=0;
        // set the frequency
        if (c.value!=DIV_NOTE_NULL) {
          chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
          chan[c.chan].freqChanged=true;
          chan[c.chan].note=c.value;
        }
        chan[c.chan].active=true;
        // start the macro interpreter
        chan[c.chan].macroInit(ins);
        // update the output volume if we don't have a volume macro
        if (!parent->song.compatFlags.brokenOutVol && !chan[c.chan].std.vol.will) {
          chan[c.chan].outVol=chan[c.chan].vol;
        }
        break;
      }
      // this here is the normal mode note handler...
      // we reset the sample note
      chan[c.chan].sampleNote=DIV_NOTE_NULL;
      chan[c.chan].sampleNoteDelta=0;
      // update frequency and note
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
        // noiseSeek is a variable which will be ANDed with 31 to calculate the noise pitch in noise mode.
        chan[c.chan].noiseSeek=c.value;
        if (chan[c.chan].noiseSeek<0) chan[c.chan].noiseSeek=0;
      }
      // perform a key on and write the current volume
      // volume will be overridden by the volume macro (if present)
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chWrite(c.chan,0x04,0x80|chan[c.chan].vol);
      // start the macro interpreter
      chan[c.chan].macroInit(ins);
      // update the output volume if we don't have a volume macro
      // I know I know, the volume write should have been done here...
      if (!parent->song.compatFlags.brokenOutVol && !chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
      }
      // if we don't have a selected waveform, select the first wave.
      if (chan[c.chan].wave<0) {
        chan[c.chan].wave=0;
        chan[c.chan].ws.changeWave1(chan[c.chan].wave);
      }
      // initialize the wave synth/wave engine
      chan[c.chan].ws.init(ins,32,31,chan[c.chan].insChanged);
      // acknowledge an instrument change.
      // PCE doesn't have any special instrument features, so this goes unused.
      chan[c.chan].insChanged=false;
      break;
    }
    case DIV_CMD_NOTE_OFF:
      // note off - disable everything.
      chan[c.chan].dacSample=-1;
      if (dumpWrites) addWrite(0xffff0002+(c.chan<<8),0);
      chan[c.chan].pcm=false;
      chan[c.chan].active=false;
      chan[c.chan].keyOff=true;
      // reset the macro interpreter
      chan[c.chan].macroInit(NULL);
      break;
    case DIV_CMD_NOTE_OFF_ENV:
    case DIV_CMD_ENV_RELEASE:
      // the handler for these two commands is the same as PCE doesn't have hardware envelope.
      // on a chip with hardware envelopes, NOTE_OFF_ENV should trigger hardware release.

      // tell the macro int we're in release
      chan[c.chan].std.release();
      break;
    case DIV_CMD_INSTRUMENT:
      // instrument change
      if (chan[c.chan].ins!=c.value || c.value2==1) {
        chan[c.chan].ins=c.value;
        chan[c.chan].insChanged=true;
      }
      break;
    case DIV_CMD_VOLUME:
      // we only process volume changes when the volume differs.
      if (chan[c.chan].vol!=c.value) {
        chan[c.chan].vol=c.value;
        // if we don't have a volume macro, update the output volume and write it.
        if (!chan[c.chan].std.vol.has) {
          chan[c.chan].outVol=c.value;
          if (chan[c.chan].active) {
            if (chan[c.chan].pcm) {
              chWrite(c.chan,0x04,0xc0|chan[c.chan].outVol);
            } else {
              chWrite(c.chan,0x04,0x80|chan[c.chan].outVol);
            }
          }
        }
      }
      break;
    case DIV_CMD_GET_VOLUME:
      // this command is used to get the current volume.
      // it exists mostly for compatibility.
      if (chan[c.chan].std.vol.has) {
        return chan[c.chan].vol;
      }
      return chan[c.chan].outVol;
      break;
    case DIV_CMD_PITCH:
      // set the pitch
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_WAVE:
      // change the current waveform
      // we use wave synth to do this
      chan[c.chan].wave=c.value;
      chan[c.chan].ws.changeWave1(chan[c.chan].wave);
      // there's a key on here for some reason...
      chan[c.chan].keyOn=true;
      break;
    // chip-specific command handlers go here.
    case DIV_CMD_PCE_LFO_MODE:
      if (c.value==0) {
        lfoMode=0;
      } else {
        lfoMode=c.value;
      }
      // LFO writes are done in tick() to avoid duplicate writes and misbehavior.
      updateLFO=true;
      break;
    case DIV_CMD_PCE_LFO_SPEED:
      lfoSpeed=255-c.value;
      updateLFO=true;
      break;
    // end of chip-specific command handlers
    case DIV_CMD_NOTE_PORTA: {
      // NOTE_PORTA is sent by the engine on each song tick until we tell it the slide/portamento has reached its target.
      // the return values are:
      // - 0: can't do it - sorry
      // - 1: OK - we're sliding
      // - 2: target reached - stop sliding

      // the target frequency.
      // this is where we use sampleNoteDelta, to compensate for mapped note being different from the played note.
      int destFreq=NOTE_PERIODIC(c.value2+chan[c.chan].sampleNoteDelta);
      bool return2=false;
      // move towards target frequency
      if (destFreq>chan[c.chan].baseFreq) {
        chan[c.chan].baseFreq+=c.value;
        if (chan[c.chan].baseFreq>=destFreq) {
          // stop at the target frequency
          chan[c.chan].baseFreq=destFreq;
          return2=true;
        }
      } else {
        chan[c.chan].baseFreq-=c.value;
        // stop at the target frequency
        if (chan[c.chan].baseFreq<=destFreq) {
          chan[c.chan].baseFreq=destFreq;
          return2=true;
        }
      }
      chan[c.chan].freqChanged=true;
      if (return2) {
        // exit portamento and tell the engine we're done
        chan[c.chan].inPorta=false;
        return 2;
      }
      break;
    }
    case DIV_CMD_STD_NOISE_MODE:
      // noise mode handler
      chan[c.chan].noise=c.value;
      chan[c.chan].freqChanged=true;
      break;
    // sample command handlers
    case DIV_CMD_SAMPLE_MODE:
      // this is unused now that legacy sample mode is removed...
      chan[c.chan].pcm=c.value;
      break;
    case DIV_CMD_SAMPLE_POS:
      // set the position.
      // since samples are done in software, we don't have to set any registers.
      chan[c.chan].dacPos=c.value;
      chan[c.chan].setPos=true;
      break;
    case DIV_CMD_PANNING: {
      // PCE has stereo output. accept panning commands.
      chan[c.chan].pan=(c.value&0xf0)|(c.value2>>4);
      chWrite(c.chan,0x05,isMuted[c.chan]?0:chan[c.chan].pan);
      break;
    }
    case DIV_CMD_LEGATO:
      // legato is kind of weird.
      // we change the base freq and the note, but we must offset it by the legato value or something like that...
      // i don't even know why. it must be from an old era....
      chan[c.chan].baseFreq=NOTE_PERIODIC(c.value+chan[c.chan].sampleNoteDelta+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      // this command is sent by the engine when a slide is about to begin (or end).
      if (chan[c.chan].active && c.value2) {
        // replicate Defle's behavior with portamento - it restarts macros...
        if (parent->song.compatFlags.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_PCE));
      }
      // reset the base freq to the current note if we can't use the new arp strategy.
      // the arp macro cannot run during a pitch slide in that case.
      if (!chan[c.chan].inPorta && c.value && !parent->song.compatFlags.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) chan[c.chan].baseFreq=NOTE_PERIODIC(chan[c.chan].note);
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_GET_VOLMAX:
      // maximum volume for PCE.
      return 31;
      break;
    // macro commands
    case DIV_CMD_MACRO_OFF:
      chan[c.chan].std.mask(c.value,true);
      break;
    case DIV_CMD_MACRO_ON:
      chan[c.chan].std.mask(c.value,false);
      break;
    case DIV_CMD_MACRO_RESTART:
      chan[c.chan].std.restart(c.value);
      break;
    // ignore unsupported commands
    default:
      break;
  }
  return 1;
}

void DivPlatformPCE::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  // there are two ways to handle channel muting:
  // - mute it in the emulator
  // - set the channel volume to 0
  // in this case we use the latter.
  chWrite(ch,0x05,isMuted[ch]?0:chan[ch].pan);
  if (!isMuted[ch] && (chan[ch].pcm && chan[ch].dacSample!=-1)) {
    chWrite(ch,0x04,parent->song.compatFlags.disableSampleMacro?0xdf:(0xc0|chan[ch].outVol));
    chWrite(ch,0x06,chan[ch].dacOut&0x1f);
  }
}

void DivPlatformPCE::forceIns() {
  // forceIns is called after a playback seek.
  // it should force-update channel state to match.
  for (int i=0; i<6; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
    updateWave(i);
    chWrite(i,0x05,isMuted[i]?0:chan[i].pan);
  }
}

SharedChannel* DivPlatformPCE::getChanState(int ch) {
  // return our Channel struct. used in the debug menu.
  return &chan[ch];
}

DivMacroInt* DivPlatformPCE::getChanMacroInt(int ch) {
  // return our macro interpreter.
  // used in the GUI for displaying macro positions.
  return &chan[ch].std;
}

unsigned short DivPlatformPCE::getPan(int ch) {
  // return current panning.
  // left in top 8 bits, and right in the bottom 8 ones.
  return ((chan[ch].pan&0xf0)<<4)|(chan[ch].pan&15);
}

void DivPlatformPCE::getPaired(int ch, std::vector<DivChannelPair>& ret) {
  // return a DivChannelPair for the channel that supports LFO (it pairs with the first channel).
  if (ch==1 && lfoMode>0) {
    ret.push_back(DivChannelPair(_("mod"),0));
  }
}

DivChannelModeHints DivPlatformPCE::getModeHints(int ch) {
  // the last two channels may be set to noise mode. we let the GUI show the user whether noise mode is on.
  DivChannelModeHints ret;
  if (ch<4) return ret;
  ret.count=1;
  ret.hint[0]=ICON_FUR_NOISE;
  ret.type[0]=0;

  if (chan[ch].noise) ret.type[0]=4;
  
  return ret;
}

DivSamplePos DivPlatformPCE::getSamplePos(int ch) {
  // used in the sample editor to display currently playing sample positions.
  if (ch>=6) return DivSamplePos();
  if (!chan[ch].pcm) return DivSamplePos();
  return DivSamplePos(
    chan[ch].dacSample,
    chan[ch].dacPos,
    chan[ch].dacRate
  );
}

DivDispatchOscBuffer* DivPlatformPCE::getOscBuffer(int ch) {
  // guess what this is for.
  return oscBuf[ch];
}

// the following two functions deal with volume conversions.
// PCE volume is logarithmic, so we must imement these.

// this maps a floating point "velocity" (0.0-1.0) to a chip volume.
int DivPlatformPCE::mapVelocity(int ch, float vel) {
  return round(31.0*pow(vel,0.22));
}

// this does the opposite - get the actual volume of a chip volume step.
float DivPlatformPCE::getGain(int ch, int vol) {
  if (vol==0) return 0;
  return 1.0/pow(10.0,(float)(31-vol)*3.0/20.0);
}

// return the register pool for display in the register view.
unsigned char* DivPlatformPCE::getRegisterPool() {
  return regPool;
}

// 112 - 16 global and 16 for each channel.
int DivPlatformPCE::getRegisterPoolSize() {
  return 112;
}

void DivPlatformPCE::reset() {
  // reset everything to initial state.

  // empty the register write queue
  writes.clear();
  // reset the register pool
  memset(regPool,0,128);
  // initialize channels
  for (int i=0; i<6; i++) {
    chan[i]=DivPlatformPCE::Channel();
    // bind engines for the macro interpreter and wave synth!
    chan[i].std.setEngine(parent);
    chan[i].ws.setEngine(parent);
    chan[i].ws.init(NULL,32,31,false);
  }
  // if we're dumping writes, tell the engine a reset has occurred.
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }
  // reset the emulator
  pce->Power(0);
  // reset chip state
  curChan=-1;
  lfoMode=0;
  lfoSpeed=255;
  // set global volume
  rWrite(0,0);
  rWrite(0x01,0xff);
  // ensure the LFO values are written on the first tick
  updateLFO=true;
  // set per-channel initial panning
  for (int i=0; i<6; i++) {
    chWrite(i,0x05,isMuted[i]?0:chan[i].pan);
  }
}

// this function doesn't have to be implemented if the chip only supports mono audio.
int DivPlatformPCE::getOutputCount() {
  return 2;
}

// this provides a hint to the GUI whether the 80xx effect (panning from left to right) is useful.
bool DivPlatformPCE::hasSoftPan(int ch) {
  return true;
}

// this is here for compatibility I believe.
// reset arpeggio on note off.
bool DivPlatformPCE::keyOffAffectsArp(int ch) {
  return true;
}

// this function lets the engine know that the dispatch supports acquireDirect().
// during mixing, the engine calls either acquire() or acquireDirect() to get an audio buffer from the dispatches.
// acquireDirect() is a version of acquire() where we get a direct blip_buffer instead of a 16-bit PCM buffer, allowing for optimizations.
bool DivPlatformPCE::hasAcquireDirect() {
  return true;
}

// this function is called whenever a waveform is modified.
void DivPlatformPCE::notifyWaveChange(int wave) {
  for (int i=0; i<6; i++) {
    if (chan[i].wave==wave) {
      chan[i].ws.changeWave1(wave);
      updateWave(i);
    }
  }
}

// this here is called when an instrument has been deleted.
// it tells the macro interpreter to stop 
void DivPlatformPCE::notifyInsDeletion(void* ins) {
  for (int i=0; i<6; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

// called on init() and by the engine when chip configuration changes.
void DivPlatformPCE::setFlags(const DivConfig& flags) {
  // set the chip clock
  if (flags.getInt("clockSel",0)) { // technically there is no PAL PC Engine but oh well...
    chipClock=COLOR_PAL*8.0/5.0;
  } else {
    chipClock=COLOR_NTSC*2.0;
  }
  // this macro should be called once chipClock is set.
  CHECK_CUSTOM_CLOCK;
  antiClickEnabled=!flags.getBool("noAntiClick",false);

  // set the output sample rate and forward this rate to the osc buffers
  rate=chipClock;
  for (int i=0; i<6; i++) {
    oscBuf[i]->setRate(rate);
  }

  // we recreate the emulator because the chip type is set in the constructor...
  if (pce!=NULL) {
    delete pce;
    pce=NULL;
  }
  pce=new PCE_PSG(flags.getInt("chipType",0)?PCE_PSG::REVISION_HUC6280A:PCE_PSG::REVISION_HUC6280);
}

void DivPlatformPCE::poke(unsigned int addr, unsigned short val) {
  // used in the debug menu. writes to a register.
  rWrite(addr,val);
}

void DivPlatformPCE::poke(std::vector<DivRegWrite>& wlist) {
  // same thing as before, but takes in a list of writes.
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

int DivPlatformPCE::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  // set the parent engine and initialize sutff.
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  updateLFO=false;
  // reset mute state and allocate our osc buffers
  for (int i=0; i<6; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  // the emulator will be allocated in setFlags().
  pce=NULL;
  setFlags(flags);
  reset();
  // return the number of channels this chip supports.
  return 6;
}

void DivPlatformPCE::quit() {
  // deallocate everything.
  for (int i=0; i<6; i++) {
    delete oscBuf[i];
  }
  if (pce!=NULL) {
    delete pce;
    pce=NULL;
  }
}

DivPlatformPCE::~DivPlatformPCE() {
}
