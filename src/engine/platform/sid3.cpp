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

#include "sid3.h"
#include "../engine.h"
#include "IconsFontAwesome4.h"
#include <math.h>
#include "../../ta-log.h"

#define rWrite(a,v) if (!skipRegisterWrites) {writes.push(QueuedWrite(a,v)); if (dumpWrites) {addWrite(a,v);} }

#define CHIP_FREQBASE 524288*64
#define CHIP_DIVIDER 1

#define CURRENT_FREQ_IN_HZ() ((double)chipClock / pow(2.0, (double)SID3_ACC_BITS) * (double)chan[i].freq)
#define c_5_FREQ() (parent->song.tuning / pow(2, (12.0 * 9.0 + 9.0) / 12.0))
#define FREQ_FOR_NOTE(note) (c_5_FREQ() * pow(2, (double)note / 12.0))

const char* regCheatSheetSID3[]={
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

typedef struct
{
  uint32_t LFSRmask;
  float freqScaling;
} noiseFreqData;

const noiseFreqData noiseInterestingWavesData[] = 
{
  {524288, 523.0f / 1675.0f}, //wave very close to SID2 noise mode 1 wave
  {541065280U, 523.0f / 104.0f / 2.0f}, //smth like vocals
  {2068, 1.0f}, //wave very close to SID2 noise mode 3 wave
  {66, 523.0f / 540.0f / 2.0f}, //wave very close to SID2 noise mode 2 wave

  {0, 0.0f}, //end marker
};

const char** DivPlatformSID3::getRegisterSheet() {
  return regCheatSheetSID3;
}

void DivPlatformSID3::acquire(short** buf, size_t len) 
{
  for (int i=0; i<SID3_NUM_CHANNELS; i++) {
    oscBuf[i]->begin(len);
  }

  for (size_t i=0; i<len; i++) 
  {
    if (chan[SID3_NUM_CHANNELS - 1].pcm && chan[SID3_NUM_CHANNELS - 1].dacSample!=-1)
    {
      chan[SID3_NUM_CHANNELS - 1].dacPeriod+=chan[SID3_NUM_CHANNELS - 1].dacRate;
      if (chan[SID3_NUM_CHANNELS - 1].dacPeriod>rate) 
      {
        DivSample* s=parent->getSample(chan[SID3_NUM_CHANNELS - 1].dacSample);
        if (s->samples<=0 || chan[SID3_NUM_CHANNELS - 1].dacPos>=s->samples) 
        {
          chan[SID3_NUM_CHANNELS - 1].dacSample=-1;
          continue;
        }

        int dacData=s->data16[chan[SID3_NUM_CHANNELS - 1].dacPos] + 32767;
        chan[SID3_NUM_CHANNELS - 1].dacOut=CLAMP(dacData,0,65535);
        updateSample = true;

        if (sampleTick > 2)
        {
            sampleTick = 0;
        }

        chan[SID3_NUM_CHANNELS - 1].dacPos++;
        if (s->isLoopable() && chan[SID3_NUM_CHANNELS - 1].dacPos>=(unsigned int)s->loopEnd) 
        {
          chan[SID3_NUM_CHANNELS - 1].dacPos=s->loopStart;
        } 
        else if (chan[SID3_NUM_CHANNELS - 1].dacPos>=s->samples) 
        {
          chan[SID3_NUM_CHANNELS - 1].dacSample=-1;
        }
        chan[SID3_NUM_CHANNELS - 1].dacPeriod-=rate;
      }
      while (chan[SID3_NUM_CHANNELS - 1].dacPeriod > rate)
      {
          chan[SID3_NUM_CHANNELS - 1].dacPeriod -= rate;
      }
    }

    sampleTick++;

    if(chan[SID3_NUM_CHANNELS - 1].pcm)
    {
      if(sampleTick == 2 && updateSample)
      {
        if (!isMuted[SID3_NUM_CHANNELS - 1]) 
        {
          sid3_write(sid3, SID3_REGISTER_STREAMED_SAMPLE_HIGH + (SID3_NUM_CHANNELS - 1) * SID3_REGISTERS_PER_CHANNEL, chan[SID3_NUM_CHANNELS - 1].dacOut >> 8);
          regPool[SID3_REGISTER_STREAMED_SAMPLE_HIGH + (SID3_NUM_CHANNELS - 1) * SID3_REGISTERS_PER_CHANNEL]=chan[SID3_NUM_CHANNELS - 1].dacOut >> 8;
        } 
        else 
        {
          sid3_write(sid3, SID3_REGISTER_STREAMED_SAMPLE_HIGH + (SID3_NUM_CHANNELS - 1) * SID3_REGISTERS_PER_CHANNEL, 32768 >> 8);
          regPool[SID3_REGISTER_STREAMED_SAMPLE_HIGH + (SID3_NUM_CHANNELS - 1) * SID3_REGISTERS_PER_CHANNEL]=32768 >> 8;
        }
      }
      else if(sampleTick == 3 && updateSample)
      {
        if (!isMuted[SID3_NUM_CHANNELS - 1]) 
        {
          sid3_write(sid3, SID3_REGISTER_STREAMED_SAMPLE_LOW + (SID3_NUM_CHANNELS - 1) * SID3_REGISTERS_PER_CHANNEL, chan[SID3_NUM_CHANNELS - 1].dacOut & 0xff);
          regPool[SID3_REGISTER_STREAMED_SAMPLE_LOW + (SID3_NUM_CHANNELS - 1) * SID3_REGISTERS_PER_CHANNEL]=chan[SID3_NUM_CHANNELS - 1].dacOut & 0xff;
        }
        else 
        {
          sid3_write(sid3, SID3_REGISTER_STREAMED_SAMPLE_LOW + (SID3_NUM_CHANNELS - 1) * SID3_REGISTERS_PER_CHANNEL, 32768 & 0xff);
          regPool[SID3_REGISTER_STREAMED_SAMPLE_LOW + (SID3_NUM_CHANNELS - 1) * SID3_REGISTERS_PER_CHANNEL]=32768 & 0xff;
        }

        sampleTick = 0;
        updateSample = false;
      }
      else
      {
        if (!writes.empty()) 
        {
          QueuedWrite w=writes.front();
          sid3_write(sid3, w.addr, w.val);
          regPool[w.addr % SID3_NUM_REGISTERS]=w.val;
          writes.pop();
        }
      }
    }
    else
    {
      if (!writes.empty()) 
      {
        QueuedWrite w=writes.front();
        sid3_write(sid3, w.addr, w.val);
        regPool[w.addr % SID3_NUM_REGISTERS]=w.val;
        writes.pop();
      }
    }
    
    sid3_clock(sid3);

    buf[0][i]=sid3->output_l;
    buf[1][i]=sid3->output_r;

    if (++writeOscBuf>=8) 
    {
      writeOscBuf=0;

      for(int j = 0; j < SID3_NUM_CHANNELS - 1; j++)
      {
        oscBuf[j]->putSample(i,sid3->muted[j] ? 0 : (sid3->channel_output[j] / 4));
      }

      oscBuf[SID3_NUM_CHANNELS - 1]->putSample(i,sid3->muted[SID3_NUM_CHANNELS - 1] ? 0 : (sid3->wave_channel_output / 4));
    }
  }

  for (int i=0; i<SID3_NUM_CHANNELS; i++) {
    oscBuf[i]->end(len);
  }
}

void DivPlatformSID3::updateFlags(int channel, bool gate) 
{
  rWrite(SID3_REGISTER_FLAGS + channel * SID3_REGISTERS_PER_CHANNEL, (gate ? 1 : 0) |
  (chan[channel].ring ? SID3_CHAN_ENABLE_RING_MOD : 0) |
  (chan[channel].sync ? SID3_CHAN_ENABLE_HARD_SYNC : 0) |
  (chan[channel].phase ? SID3_CHAN_ENABLE_PHASE_MOD : 0) |
  (chan[channel].phaseReset ? SID3_CHAN_PHASE_RESET : 0) |
  (chan[channel].envReset ? SID3_CHAN_ENV_RESET : 0) |
  (chan[channel].phaseResetNoise ? SID3_CHAN_NOISE_PHASE_RESET : 0) |
  (chan[channel].oneBitNoise ? SID3_CHAN_1_BIT_NOISE : 0));
}

void DivPlatformSID3::updateFilter(int channel, int filter) 
{
  rWrite(SID3_REGISTER_FILT_MODE + filter * SID3_REGISTERS_PER_FILTER + channel*SID3_REGISTERS_PER_CHANNEL,
    chan[channel].filt[filter].mode | (chan[channel].filt[filter].enabled ? SID3_FILTER_ENABLE : 0));

  rWrite(SID3_REGISTER_FILT_CUTOFF_HIGH + filter * SID3_REGISTERS_PER_FILTER + channel*SID3_REGISTERS_PER_CHANNEL,
    chan[channel].filt[filter].cutoff >> 8);
  rWrite(SID3_REGISTER_FILT_CUTOFF_LOW + filter * SID3_REGISTERS_PER_FILTER + channel*SID3_REGISTERS_PER_CHANNEL,
    chan[channel].filt[filter].cutoff & 0xff);

  rWrite(SID3_REGISTER_FILT_RESONANCE + filter * SID3_REGISTERS_PER_FILTER + channel*SID3_REGISTERS_PER_CHANNEL,
    chan[channel].filt[filter].resonance);

  rWrite(SID3_REGISTER_FILT_DISTORTION + filter * SID3_REGISTERS_PER_FILTER + channel*SID3_REGISTERS_PER_CHANNEL,
    chan[channel].filt[filter].distortion_level);

  rWrite(SID3_REGISTER_FILT_CONNECTION + filter * SID3_REGISTERS_PER_FILTER + channel*SID3_REGISTERS_PER_CHANNEL,
    chan[channel].filt[filter].filter_matrix);

  rWrite(SID3_REGISTER_FILT_OUTPUT_VOLUME + filter * SID3_REGISTERS_PER_FILTER + channel*SID3_REGISTERS_PER_CHANNEL,
    chan[channel].filt[filter].output_volume);
}

void DivPlatformSID3::updateFreq(int channel) 
{
  rWrite(SID3_REGISTER_FREQ_HIGH + channel*SID3_REGISTERS_PER_CHANNEL,(chan[channel].freq >> 16) & 0xff);
  rWrite(SID3_REGISTER_FREQ_MID + channel*SID3_REGISTERS_PER_CHANNEL,(chan[channel].freq >> 8) & 0xff);
  rWrite(SID3_REGISTER_FREQ_LOW + channel*SID3_REGISTERS_PER_CHANNEL,chan[channel].freq & 0xff);
}

void DivPlatformSID3::updateNoiseFreq(int channel) 
{
  rWrite(SID3_REGISTER_NOISE_FREQ_HIGH + channel*SID3_REGISTERS_PER_CHANNEL,(chan[channel].noiseFreq >> 16) & 0xff);
  rWrite(SID3_REGISTER_NOISE_FREQ_MID + channel*SID3_REGISTERS_PER_CHANNEL,(chan[channel].noiseFreq >> 8) & 0xff);
  rWrite(SID3_REGISTER_NOISE_FREQ_LOW + channel*SID3_REGISTERS_PER_CHANNEL,chan[channel].noiseFreq & 0xff);
}

void DivPlatformSID3::updateNoiseLFSRMask(int channel) 
{
  rWrite(SID3_REGISTER_NOISE_LFSR_HIGHEST + channel*SID3_REGISTERS_PER_CHANNEL,(chan[channel].noiseLFSRMask >> 24) & 0xff);
  rWrite(SID3_REGISTER_NOISE_LFSR_HIGH + channel*SID3_REGISTERS_PER_CHANNEL,(chan[channel].noiseLFSRMask >> 16) & 0xff);
  rWrite(SID3_REGISTER_NOISE_LFSR_MID + channel*SID3_REGISTERS_PER_CHANNEL,(chan[channel].noiseLFSRMask >> 8) & 0xff);
  rWrite(SID3_REGISTER_NOISE_LFSR_LOW + channel*SID3_REGISTERS_PER_CHANNEL,chan[channel].noiseLFSRMask & 0xff);
}

void DivPlatformSID3::updateDuty(int channel) 
{
  rWrite(SID3_REGISTER_PW_HIGH + channel*SID3_REGISTERS_PER_CHANNEL,(chan[channel].duty >> 8) & 0xff);
  rWrite(SID3_REGISTER_PW_LOW + channel*SID3_REGISTERS_PER_CHANNEL,chan[channel].duty & 0xff);
}

void DivPlatformSID3::updateEnvelope(int channel) 
{
  rWrite(SID3_REGISTER_ADSR_A + channel * SID3_REGISTERS_PER_CHANNEL, chan[channel].attack); //attack
  rWrite(SID3_REGISTER_ADSR_D + channel * SID3_REGISTERS_PER_CHANNEL, chan[channel].decay); //decay
  rWrite(SID3_REGISTER_ADSR_S + channel * SID3_REGISTERS_PER_CHANNEL, chan[channel].sustain); //sustain
  rWrite(SID3_REGISTER_ADSR_SR + channel * SID3_REGISTERS_PER_CHANNEL, chan[channel].sr); //sr
  rWrite(SID3_REGISTER_ADSR_R + channel * SID3_REGISTERS_PER_CHANNEL, chan[channel].release); //release
}

void DivPlatformSID3::updatePanning(int channel) 
{
  rWrite(SID3_REGISTER_PAN_LEFT + channel*SID3_REGISTERS_PER_CHANNEL,chan[channel].panLeft);
  rWrite(SID3_REGISTER_PAN_RIGHT + channel*SID3_REGISTERS_PER_CHANNEL,chan[channel].panRight);
}

void DivPlatformSID3::updateWave()
{
  int channel = SID3_NUM_CHANNELS - 1;

  for(int i = 0; i < 256; i++)
  {
    uint8_t val = ws.output[i & 255];
    rWrite(SID3_REGISTER_PW_HIGH + channel*SID3_REGISTERS_PER_CHANNEL,i);
    rWrite(SID3_REGISTER_PW_LOW + channel*SID3_REGISTERS_PER_CHANNEL,val);
  }
}

void DivPlatformSID3::tick(bool sysTick) 
{
  bool doUpdateWave = false;

  for (int i=0; i<SID3_NUM_CHANNELS; i++) 
  {
    chan[i].std.next();

    DivInstrument* ins=parent->getIns(chan[i].ins,DIV_INS_SID3);

    bool panChanged = false;
    bool flagsChanged = false;
    bool envChanged = false;

    if(sysTick)
    {
      if(chan[i].pw_slide != 0)
      {
        chan[i].duty -= chan[i].pw_slide;
        chan[i].duty = CLAMP(chan[i].duty, 0, 0xffff);
        updateDuty(i);
      }
      for(int j = 0; j < SID3_NUM_FILTERS; j++) //filters' slides
      {
        if(chan[i].filt[j].cutoff_slide != 0)
        {
          chan[i].filt[j].cutoff += chan[i].filt[j].cutoff_slide;
          chan[i].filt[j].cutoff = CLAMP(chan[i].filt[j].cutoff, 0, 0xffff);
          updateFilter(i, j);
        }
      }

      if(chan[i].phase_reset_counter >= 0)
      {
        if(chan[i].phase_reset_counter == 0)
        {
          chan[i].phaseReset = true;
          flagsChanged = true;
        }
        chan[i].phase_reset_counter--;
      }
      if(chan[i].noise_phase_reset_counter >= 0)
      {
        if(chan[i].noise_phase_reset_counter == 0)
        {
          chan[i].phaseResetNoise = true;
          flagsChanged = true;
        }
        chan[i].noise_phase_reset_counter--;
      }
      if(chan[i].envelope_reset_counter >= 0)
      {
        if(chan[i].envelope_reset_counter == 0)
        {
          chan[i].envReset = true;
          flagsChanged = true;
        }
        chan[i].envelope_reset_counter--;
      }
    }

    if (chan[i].std.vol.had) 
    {
      chan[i].outVol=VOL_SCALE_LINEAR(chan[i].vol&255,MIN(255,chan[i].std.vol.val),255);
      rWrite(13 + i * SID3_REGISTERS_PER_CHANNEL, chan[i].outVol);
    }
    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        chan[i].baseFreq=NOTE_FREQUENCY(parent->calcArp(chan[i].note,chan[i].std.arp.val));
      }
      chan[i].freqChanged=true;
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
    if (chan[i].std.duty.had) {
      DivInstrument* ins=parent->getIns(chan[i].ins,DIV_INS_SID3);
      if (ins->sid3.dutyIsAbs) {
        chan[i].duty=chan[i].std.duty.val;
      } else {
        chan[i].duty-=chan[i].std.duty.val;
      }
      chan[i].duty&=65535;
      updateDuty(i);
    }
    if (chan[i].std.wave.had) {
      DivInstrument* ins=parent->getIns(chan[i].ins,DIV_INS_SID3);

      if(i == SID3_NUM_CHANNELS - 1 && ins->sid3.doWavetable)
      {
        chan[i].wavetable = chan[i].std.wave.val & 0xff;
        ws.changeWave1(chan[i].wavetable, true);
        doUpdateWave = true;
      }
      else
      {
        chan[i].wave = chan[i].std.wave.val & 0xff;
        rWrite(SID3_REGISTER_WAVEFORM + i * SID3_REGISTERS_PER_CHANNEL, chan[i].wave);
      }
    }
    if (chan[i].std.alg.had) { //special wave
      chan[i].special_wave = chan[i].std.alg.val & 0xff;
      rWrite(SID3_REGISTER_SPECIAL_WAVE + i * SID3_REGISTERS_PER_CHANNEL, chan[i].special_wave);
    }
    if (chan[i].std.op[3].am.had) { //noise arpeggio
      //chan[i].handleArpNoise(0);
      chan[i].noiseFreqChanged = true;
    }
    chan[i].handleArpNoise(0);
    chan[i].handlePitchNoise();
    if (chan[i].std.op[0].ar.had) { //noise pitch
      //chan[i].handlePitchNoise();
      chan[i].noiseFreqChanged = true;
    }
    if (chan[i].std.panL.had) {
      panChanged = true;
      chan[i].panLeft = chan[i].std.panL.val & 0xff;
    }
    if (chan[i].std.panR.had) {
      panChanged = true;
      chan[i].panRight = chan[i].std.panR.val & 0xff;
    }
    if (chan[i].std.op[2].ar.had) { //channel signal inversion
      chan[i].phaseInv = chan[i].std.op[2].ar.val & 3;
      rWrite(SID3_REGISTER_PHASE_INVERSION + i * SID3_REGISTERS_PER_CHANNEL, chan[i].phaseInv);
    }
    if (chan[i].std.op[0].am.had) { //key on/off
      chan[i].gate = chan[i].std.op[0].am.val & 1;
      flagsChanged = true;
    }
    if (chan[i].std.ex1.had) { //ring mod, hard sync, phase mod
      chan[i].phase = chan[i].std.ex1.val & 1;
      chan[i].sync = chan[i].std.ex1.val & 2;
      chan[i].ring = chan[i].std.ex1.val & 4;
      flagsChanged = true;
    }
    if (chan[i].std.ams.had) { //hard sync source
      chan[i].syncSrc = chan[i].std.ams.val & 0xff;
      rWrite(SID3_REGISTER_SYNC_SRC + i * SID3_REGISTERS_PER_CHANNEL, chan[i].syncSrc);
    }
    if (chan[i].std.fms.had) { //ring mod source
      chan[i].ringSrc = chan[i].std.fms.val & 0xff;
      rWrite(SID3_REGISTER_RING_MOD_SRC + i * SID3_REGISTERS_PER_CHANNEL, chan[i].ringSrc);
    }
    if (chan[i].std.fb.had) { //phase mod source
      chan[i].phaseSrc = chan[i].std.fb.val & 0xff;
      rWrite(SID3_REGISTER_PHASE_MOD_SRC + i * SID3_REGISTERS_PER_CHANNEL, chan[i].phaseSrc);
    }
    if (chan[i].std.op[3].ar.had) { //feedback
      chan[i].feedback = chan[i].std.op[3].ar.val & 0xff;
      rWrite(SID3_REGISTER_FEEDBACK + i * SID3_REGISTERS_PER_CHANNEL, chan[i].feedback);
    }
    if (chan[i].std.phaseReset.had) {
      chan[i].phaseReset = chan[i].std.phaseReset.val & 1;

      if(chan[i].phaseReset)
      {
        flagsChanged = true;
      }

      if (chan[i].pcm) 
      {
        if (chan[i].active && chan[i].dacSample>=0 && chan[i].dacSample<parent->song.sampleLen) 
        {
          chan[i].dacPos=0;
          chan[i].dacPeriod=0;
        }
      }
    }
    if (chan[i].std.op[1].am.had) { //noise phase reset
      chan[i].phaseResetNoise = chan[i].std.op[1].am.val & 1;

      if(chan[i].phaseResetNoise)
      {
        flagsChanged = true;
      }
    }
    if (chan[i].std.op[2].am.had) { //envelope reset
      chan[i].envReset = chan[i].std.op[2].am.val & 1;

      if(chan[i].envReset)
      {
        flagsChanged = true;
      }
    }
    if (chan[i].std.ex2.had) { //attack
      chan[i].attack = chan[i].std.ex2.val & 0xff;
      envChanged = true;
    }
    if (chan[i].std.ex3.had) { //decay
      chan[i].decay = chan[i].std.ex3.val & 0xff;
      envChanged = true;
    }
    if (chan[i].std.ex4.had) { //sustain
      chan[i].sustain = chan[i].std.ex4.val & 0xff;
      envChanged = true;
    }
    if (chan[i].std.ex5.had) { //sustain rate
      chan[i].sr = chan[i].std.ex5.val & 0xff;
      envChanged = true;
    }
    if (chan[i].std.ex6.had) { //release
      chan[i].release = chan[i].std.ex6.val & 0xff;
      envChanged = true;
    }
    if (chan[i].std.ex7.had) { //noise LFSR feedback bits
      chan[i].noiseLFSRMask = chan[i].std.ex7.val & 0x3fffffff;
      updateNoiseLFSRMask(i);
      chan[i].noiseFreqChanged = true;
    }
    if (chan[i].std.op[1].ar.had) { //1-bit noise / PCM mode for wavetable chan
      if(i == SID3_NUM_CHANNELS - 1) //wave chan
      {
        rWrite(SID3_REGISTER_WAVEFORM + i * SID3_REGISTERS_PER_CHANNEL, chan[i].std.op[1].ar.val & 1);
      }
      else
      {
        if((uint32_t)chan[i].oneBitNoise != (chan[i].std.op[1].ar.val & 1))
        {
          chan[i].oneBitNoise = chan[i].std.op[1].ar.val & 1;
          flagsChanged = true;
        }
      }
    }
    if (chan[i].std.ex8.had) { //wave mix mode
      chan[i].mix_mode = chan[i].std.ex8.val & 0xff;
      rWrite(SID3_REGISTER_MIXMODE + i * SID3_REGISTERS_PER_CHANNEL, chan[i].mix_mode); //mixmode
    }

    for(int j = 0; j < SID3_NUM_FILTERS; j++) //filter macros
    {
      DivMacroInt::IntOp* op = &chan[i].std.op[j];
      DivPlatformSID3::Channel::Filter* ch_filt = &chan[i].filt[j];
      DivInstrumentSID3::Filter* ins_filt = &ins->sid3.filt[j];

      bool doUpdateFilter = false;

      if (op->d2r.had) { //cutoff
        if (ins_filt->absoluteCutoff) {
          ch_filt->cutoff=op->d2r.val;
        } else {
          ch_filt->cutoff+=op->d2r.val;
        }
        ch_filt->cutoff&=65535;
        doUpdateFilter = true;
      }
      if (op->dam.had) { //resonance
        ch_filt->resonance=op->dam.val & 0xff;
        doUpdateFilter = true;
      }
      if (op->dr.had) { //filter toggle
        ch_filt->enabled=op->dr.val & 1;
        doUpdateFilter = true;
      }
      if (op->dt2.had) { //distortion level
        ch_filt->distortion_level=op->dt2.val & 0xff;
        doUpdateFilter = true;
      }
      if (op->dt.had) { //output volume
        ch_filt->output_volume=op->dt.val & 0xff;
        doUpdateFilter = true;
      }
      if (op->dvb.had) { //connect to channel input
        ch_filt->mode &= ~SID3_FILTER_CHANNEL_INPUT;
        ch_filt->mode |= (op->dvb.val & 1) ? SID3_FILTER_CHANNEL_INPUT : 0;
        doUpdateFilter = true;
      }
      if (op->egt.had) { //connect to channel output
        ch_filt->mode &= ~SID3_FILTER_OUTPUT;
        ch_filt->mode |= (op->egt.val & 1) ? SID3_FILTER_OUTPUT : 0;
        doUpdateFilter = true;
      }
      if (op->ksl.had) { //connection matrix row
        ch_filt->filter_matrix=op->ksl.val & 0xf;
        doUpdateFilter = true;
      }
      if (op->ksr.had) { //filter mode
        ch_filt->mode &= ~(SID3_FILTER_LP | SID3_FILTER_HP | SID3_FILTER_BP);
        if(op->ksr.val & 1) ch_filt->mode |= SID3_FILTER_LP;
        if(op->ksr.val & 2) ch_filt->mode |= SID3_FILTER_BP;
        if(op->ksr.val & 4) ch_filt->mode |= SID3_FILTER_HP;
        doUpdateFilter = true;
      }

      if(doUpdateFilter)
      {
        updateFilter(i, j);
      }
    }

    if(panChanged)
    {
      updatePanning(i);
    }
    if(flagsChanged)
    {
      updateFlags(i, chan[i].gate);

      chan[i].phaseReset = false;
      chan[i].phaseResetNoise = false;
      chan[i].envReset = false;
    }
    if(envChanged)
    {
      updateEnvelope(i);
    }

    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) 
    {
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,false,2,chan[i].pitch2,chipClock,CHIP_FREQBASE);

      if (chan[i].keyOn) 
      {
        DivInstrument* ins=parent->getIns(chan[i].ins,DIV_INS_SID3);
        if(i == SID3_NUM_CHANNELS - 1)
        {
          if(ins->sid3.doWavetable && !ins->amiga.useSample)
          {
            rWrite(SID3_REGISTER_WAVEFORM + i * SID3_REGISTERS_PER_CHANNEL, 0); //wave channel mode
          }
          else
          {
            rWrite(SID3_REGISTER_WAVEFORM + i * SID3_REGISTERS_PER_CHANNEL, 1); //wave channel mode
          }
        }
        else
        {
          rWrite(SID3_REGISTER_WAVEFORM + i * SID3_REGISTERS_PER_CHANNEL, chan[i].wave);
        }
        
        rWrite(SID3_REGISTER_SPECIAL_WAVE + i * SID3_REGISTERS_PER_CHANNEL, chan[i].special_wave); //special wave

        rWrite(SID3_REGISTER_ADSR_VOL + i * SID3_REGISTERS_PER_CHANNEL, chan[i].outVol); //set volume
        rWrite(SID3_REGISTER_MIXMODE + i * SID3_REGISTERS_PER_CHANNEL, chan[i].mix_mode); //mixmode

        rWrite(SID3_REGISTER_RING_MOD_SRC + i * SID3_REGISTERS_PER_CHANNEL, chan[i].ringSrc); //ring mod source
        rWrite(SID3_REGISTER_SYNC_SRC + i * SID3_REGISTERS_PER_CHANNEL, chan[i].syncSrc); //hard sync source
        rWrite(SID3_REGISTER_PHASE_MOD_SRC + i * SID3_REGISTERS_PER_CHANNEL, chan[i].phaseSrc); //phase mod source

        rWrite(SID3_REGISTER_PHASE_INVERSION + i * SID3_REGISTERS_PER_CHANNEL, chan[i].phaseInv); //signal inversion
        rWrite(SID3_REGISTER_FEEDBACK + i * SID3_REGISTERS_PER_CHANNEL, chan[i].feedback); //feedback

        updateEnvelope(i);

        updateFlags(i, false); //gate off TODO: make it properly?
        updateFlags(i, true); //gate on
        chan[i].gate = true;
      }
      if (chan[i].keyOff) 
      {
        updateFlags(i, false); //gate off
        chan[i].gate = false;
      }

      if (chan[i].freq<0) chan[i].freq=0;
      if (chan[i].freq>0xffffff) chan[i].freq=0xffffff;

      updateFreq(i);
      
      for(int j = 0; j < SID3_NUM_FILTERS; j++)
      {
        bool doUpdateFilter = false;

        if(chan[i].filt[j].bindCutoffToNote && (!ins->sid3.filt[j].bindCutoffOnNote || chan[i].keyOn))
        {
          double scaling = CURRENT_FREQ_IN_HZ() / FREQ_FOR_NOTE(chan[i].filt[j].bindCutoffToNoteCenter) - 1.0;
          if (chan[i].filt[j].bindCutoffToNoteDir)
          {
            scaling *= -1.0;
          }

          int cutoff = ins->sid3.filt[j].cutoff + (int)(scaling * (double)chan[i].filt[j].bindCutoffToNoteStrength * 80.0);

          if(cutoff > 0xffff) cutoff = 0xffff;
          if(cutoff < 0) cutoff = 0;

          chan[i].filt[j].cutoff = cutoff;
          doUpdateFilter = true;
        }

        if(chan[i].filt[j].bindResonanceToNote && (!ins->sid3.filt[j].bindResonanceOnNote || chan[i].keyOn))
        {
          double scaling = CURRENT_FREQ_IN_HZ() / FREQ_FOR_NOTE(chan[i].filt[j].bindResonanceToNoteCenter) - 1.0;
          if (chan[i].filt[j].bindResonanceToNoteDir)
          {
            scaling *= -1.0;
          }

          int res = ins->sid3.filt[j].resonance + (int)(scaling * (double)chan[i].filt[j].bindResonanceToNoteStrength * 80.0 / 256.0);

          if(res > 0xff) res = 0xff;
          if(res < 0) res = 0;

          chan[i].filt[j].resonance = res;
          doUpdateFilter = true;
        }

        if(doUpdateFilter)
        {
          updateFilter(i, j);
        }
      }

      if (chan[i].pcm && i == SID3_NUM_CHANNELS - 1) {
        double off=1.0;
        if (chan[i].dacSample>=0 && chan[i].dacSample<parent->song.sampleLen) {
          DivSample* s=parent->getSample(chan[i].dacSample);
          if (s->centerRate<1) {
            off=1.0;
          } else {
            off=(double)s->centerRate/parent->getCenterRate();
          }
        }
        chan[i].dacRate=chan[i].freq*(off / 32.0)*(double)chipClock/1000000.0;
      }

      chan[i].noiseFreqChanged = true;

      if(chan[i].independentNoiseFreq)
      {
        chan[i].noise_pitch2 = chan[i].pitch2;
      }

      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }

    if(chan[i].noiseFreqChanged)
    {
      if(chan[i].independentNoiseFreq)
      {
        chan[i].noiseFreq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].noise_fixedArp?chan[i].noise_baseNoteOverride:chan[i].noise_arpOff,chan[i].noise_fixedArp,false,2,chan[i].noise_pitch2,chipClock,CHIP_FREQBASE);
      }
      else
      {
        chan[i].noiseFreq = chan[i].freq;
      }

      bool found = false;
      int index = 0;
      while(noiseInterestingWavesData[index].LFSRmask != 0 && !found)
      {
        if(noiseInterestingWavesData[index].LFSRmask == chan[i].noiseLFSRMask)
        {
          chan[i].noiseFreq *= noiseInterestingWavesData[index].freqScaling;
          found = true;
        }
        index++;
      }

      if (chan[i].noiseFreq<0) chan[i].noiseFreq=0;
      if (chan[i].noiseFreq>0xffffff) chan[i].noiseFreq=0xffffff;

      updateNoiseFreq(i);

      chan[i].noiseFreqChanged = false;
    }
  }

  if (chan[SID3_NUM_CHANNELS - 1].active && !chan[SID3_NUM_CHANNELS - 1].pcm) 
  {
    if (ws.tick()) 
    {
      doUpdateWave = true;
    }
  }

  if(doUpdateWave)
  {
    updateWave();
    doUpdateWave = false;
  }
}

int DivPlatformSID3::dispatch(DivCommand c) {
  if (c.chan>SID3_NUM_CHANNELS - 1) return 0;

  bool updEnv = false;
  DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_SID3);
  int filter = 0;

  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_SID3);
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;

      if (ins->amiga.useSample)
      {
        chan[c.chan].pcm=true;
      }
      else
      {
        chan[c.chan].pcm=false;
      }

      if (chan[c.chan].pcm && c.chan == SID3_NUM_CHANNELS - 1) 
      {
        if (ins->amiga.useSample) 
        {
          if (skipRegisterWrites) break;
          if (c.value!=DIV_NOTE_NULL) {
            chan[c.chan].dacSample=ins->amiga.getSample(c.value);
            chan[c.chan].sampleNote=c.value;
            c.value=ins->amiga.getFreq(c.value);
            chan[c.chan].sampleNoteDelta=c.value-chan[c.chan].sampleNote;
          } else if (chan[c.chan].sampleNote!=DIV_NOTE_NULL) {
            chan[c.chan].dacSample=ins->amiga.getSample(chan[c.chan].sampleNote);
            c.value=ins->amiga.getFreq(chan[c.chan].sampleNote);
          }
          if (chan[c.chan].dacSample<0 || chan[c.chan].dacSample>=parent->song.sampleLen) 
          {
            chan[c.chan].dacSample=-1;
            break;
          }
          chan[c.chan].dacPos=0;
          chan[c.chan].dacPeriod=0;
          if (c.value!=DIV_NOTE_NULL) 
          {
            chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
            chan[c.chan].freqChanged=true;
            chan[c.chan].note=c.value;
          }
          chan[c.chan].active=true;
          chan[c.chan].macroInit(ins);
          if (!parent->song.compatFlags.brokenOutVol && !chan[c.chan].std.vol.will) {
            chan[c.chan].outVol=chan[c.chan].vol;
          }
          //chan[c.chan].keyOn=true;
        }
      }

      if (chan[c.chan].insChanged) 
      {
        chan[c.chan].wave = (ins->sid3.triOn ? SID3_WAVE_TRIANGLE : 0) | (ins->sid3.sawOn ? SID3_WAVE_SAW : 0) |
            (ins->sid3.pulseOn ? SID3_WAVE_PULSE : 0) | (ins->sid3.noiseOn ? SID3_WAVE_NOISE : 0) | (ins->sid3.specialWaveOn ? SID3_WAVE_SPECIAL : 0); //waveform
        chan[c.chan].special_wave = ins->sid3.special_wave; //special wave

        chan[c.chan].attack=ins->sid3.a;
        chan[c.chan].decay=ins->sid3.d;
        chan[c.chan].sustain=ins->sid3.s;
        chan[c.chan].sr=ins->sid3.sr;
        chan[c.chan].release=ins->sid3.r;

        chan[c.chan].sync = ins->sid3.oscSync;
        chan[c.chan].ring = ins->sid3.ringMod;
        chan[c.chan].phase = ins->sid3.phase_mod;
        chan[c.chan].oneBitNoise = ins->sid3.oneBitNoise;

        chan[c.chan].mix_mode = ins->sid3.mixMode;

        chan[c.chan].ringSrc = ins->sid3.ring_mod_source;
        chan[c.chan].syncSrc = ins->sid3.sync_source;
        chan[c.chan].phaseSrc = ins->sid3.phase_mod_source;

        chan[c.chan].independentNoiseFreq = ins->sid3.separateNoisePitch;

        chan[c.chan].phaseInv = ins->sid3.phaseInv;
        chan[c.chan].feedback = ins->sid3.feedback;

        for(int j = 0; j < SID3_NUM_FILTERS; j++)
        {
          if(ins->sid3.filt[j].init)
          {
            chan[c.chan].filt[j].cutoff = ins->sid3.filt[j].cutoff;
            chan[c.chan].filt[j].resonance = ins->sid3.filt[j].resonance;
            chan[c.chan].filt[j].distortion_level = ins->sid3.filt[j].distortion_level;
            chan[c.chan].filt[j].enabled = ins->sid3.filt[j].enabled;
            chan[c.chan].filt[j].filter_matrix = ins->sid3.filt[j].filter_matrix;
            chan[c.chan].filt[j].mode = ins->sid3.filt[j].mode;
            chan[c.chan].filt[j].output_volume = ins->sid3.filt[j].output_volume;

            chan[c.chan].filt[j].bindCutoffToNote = ins->sid3.filt[j].bindCutoffToNote;
            chan[c.chan].filt[j].bindCutoffToNoteStrength = ins->sid3.filt[j].bindCutoffToNoteStrength;
            chan[c.chan].filt[j].bindCutoffToNoteCenter = ins->sid3.filt[j].bindCutoffToNoteCenter;
            chan[c.chan].filt[j].bindCutoffToNoteDir = ins->sid3.filt[j].bindCutoffToNoteDir;

            chan[c.chan].filt[j].bindResonanceToNote = ins->sid3.filt[j].bindResonanceToNote;
            chan[c.chan].filt[j].bindResonanceToNoteStrength = ins->sid3.filt[j].bindResonanceToNoteStrength;
            chan[c.chan].filt[j].bindResonanceToNoteCenter = ins->sid3.filt[j].bindResonanceToNoteCenter;
            chan[c.chan].filt[j].bindResonanceToNoteDir = ins->sid3.filt[j].bindResonanceToNoteDir;

            updateFilter(c.chan, j);
          }
        }

        if(c.chan == SID3_NUM_CHANNELS - 1)
        {
          if(!chan[c.chan].pcm)
          {
            ws.changeWave1(chan[c.chan].wavetable, false);
            ws.init(ins,256,255,chan[c.chan].insChanged);
          }
        }
      }
      if(ins->sid3.resetDuty||chan[c.chan].insChanged)
      {
        chan[c.chan].duty=ins->sid3.duty;
        updateDuty(c.chan);
      }
      if (chan[c.chan].insChanged) {
        chan[c.chan].insChanged=false;
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
          rWrite(SID3_REGISTER_ADSR_VOL + c.chan * SID3_REGISTERS_PER_CHANNEL, chan[c.chan].vol);
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
    case DIV_CMD_LEGATO:
      chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.compatFlags.resetMacroOnPorta || parent->song.compatFlags.preNoteNoEffect) {
          chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_SID3));
          chan[c.chan].keyOn=true;
        }
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.compatFlags.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) chan[c.chan].baseFreq=NOTE_FREQUENCY(chan[c.chan].note);
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_PANNING: {
      bool updPan = false;
      if (!chan[c.chan].std.panL.has) 
      {
        chan[c.chan].panLeft = c.value;
        updPan = true;
      }
      if (!chan[c.chan].std.panR.has) 
      {
        chan[c.chan].panRight = c.value2;
        updPan = true;
      }
      if(updPan)
      {
        updatePanning(c.chan);
      }
      break;
    }
    case DIV_CMD_GET_VOLMAX:
      return SID3_MAX_VOL;
      break;
    case DIV_CMD_WAVE:
      if(c.chan == SID3_NUM_CHANNELS - 1 && ins->sid3.doWavetable)
      {
        chan[c.chan].wavetable = c.value & 0xff;
        ws.changeWave1(chan[c.chan].wave);
      }
      else
      {
        chan[c.chan].wave = c.value & 0xff;
        rWrite(SID3_REGISTER_WAVEFORM + c.chan * SID3_REGISTERS_PER_CHANNEL, chan[c.chan].wave);
      }
      break;
    case DIV_CMD_SID3_SPECIAL_WAVE:
      chan[c.chan].special_wave = c.value % SID3_NUM_SPECIAL_WAVES;
      rWrite(SID3_REGISTER_SPECIAL_WAVE + c.chan * SID3_REGISTERS_PER_CHANNEL, chan[c.chan].special_wave);
      break;
    case DIV_CMD_C64_EXTENDED:
      chan[c.chan].ring = c.value & 1;
      chan[c.chan].sync = c.value & 2;
      chan[c.chan].phase = c.value & 4;
      updateFlags(c.chan, chan[c.chan].gate);
      break;
    case DIV_CMD_C64_DUTY_RESET:
      if (c.value&15) {
        DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_SID3);
        chan[c.chan].duty=ins->sid3.duty;
        updateDuty(c.chan);
      }
      chan[c.chan].resetDuty=c.value>>4;
      break;
    case DIV_CMD_SID3_RING_MOD_SRC:
      chan[c.chan].ringSrc = c.value % (SID3_NUM_CHANNELS + 1);
      rWrite(SID3_REGISTER_RING_MOD_SRC + c.chan * SID3_REGISTERS_PER_CHANNEL, chan[c.chan].ringSrc);
      break;
    case DIV_CMD_SID3_HARD_SYNC_SRC:
      chan[c.chan].syncSrc = c.value % SID3_NUM_CHANNELS;
      rWrite(SID3_REGISTER_SYNC_SRC + c.chan * SID3_REGISTERS_PER_CHANNEL, chan[c.chan].syncSrc);
      break;
    case DIV_CMD_SID3_PHASE_MOD_SRC:
      chan[c.chan].phaseSrc = c.value % SID3_NUM_CHANNELS;
      rWrite(SID3_REGISTER_PHASE_MOD_SRC + c.chan * SID3_REGISTERS_PER_CHANNEL, chan[c.chan].phaseSrc);
      break;
    case DIV_CMD_FM_AR:
      chan[c.chan].attack = c.value & 0xff;
      updEnv = true;
      break;
    case DIV_CMD_FM_DR:
      chan[c.chan].decay = c.value & 0xff;
      updEnv = true;
      break;
    case DIV_CMD_FM_SL:
      chan[c.chan].sustain = c.value & 0xff;
      updEnv = true;
      break;
    case DIV_CMD_FM_D2R:
      chan[c.chan].sr = c.value & 0xff;
      updEnv = true;
      break;
    case DIV_CMD_FM_RR:
      chan[c.chan].release = c.value & 0xff;
      updEnv = true;
      break;
    case DIV_CMD_SID3_WAVE_MIX:
      chan[c.chan].mix_mode = c.value % 5;
      rWrite(SID3_REGISTER_MIXMODE + c.chan * SID3_REGISTERS_PER_CHANNEL, chan[c.chan].mix_mode);
      break;
    case DIV_CMD_SID3_LFSR_FEEDBACK_BITS:
      chan[c.chan].noiseLFSRMask &= ~(0xffU << (8 * c.value2));
      chan[c.chan].noiseLFSRMask |= ((c.value & (c.value2 == 3 ? 0x3f : 0xff)) << (8 * c.value2));
      updateNoiseLFSRMask(c.chan);
      chan[c.chan].noiseFreqChanged = true;
      break;
    case DIV_CMD_SID3_1_BIT_NOISE:
      if(c.chan == SID3_NUM_CHANNELS - 1) //wave chan
      {
        rWrite(SID3_REGISTER_WAVEFORM + c.chan * SID3_REGISTERS_PER_CHANNEL, c.value & 1); //PCM mode
      }
      else
      {
        if((uint32_t)chan[c.chan].oneBitNoise != (c.value & 1))
        {
          chan[c.chan].oneBitNoise = c.value & 1;
          updateFlags(c.chan, chan[c.chan].gate);
        }
      }
      break;
    case DIV_CMD_C64_FINE_DUTY:
      chan[c.chan].duty = (c.value & 0xfff) << 4;
      updateDuty(c.chan);
      break;
    case DIV_CMD_FM_FB:
      chan[c.chan].feedback = c.value & 0xff;
      rWrite(SID3_REGISTER_FEEDBACK + c.chan * SID3_REGISTERS_PER_CHANNEL, chan[c.chan].feedback);
      break;
    case DIV_CMD_SID3_CHANNEL_INVERSION:
      chan[c.chan].phaseInv = c.value & 3;
      rWrite(SID3_REGISTER_PHASE_INVERSION + c.chan * SID3_REGISTERS_PER_CHANNEL, chan[c.chan].phaseInv);
      break;
    case DIV_CMD_C64_FINE_CUTOFF:
      chan[c.chan].filt[c.value2].cutoff = (c.value & 0xfff) << 4;
      updateFilter(c.chan, c.value2);
      break;
    case DIV_CMD_C64_RESONANCE:
      chan[c.chan].filt[c.value2].resonance = c.value & 0xff;
      updateFilter(c.chan, c.value2);
      break;
    case DIV_CMD_SID3_FILTER_OUTPUT_VOLUME:
      chan[c.chan].filt[c.value2].output_volume = c.value & 0xff;
      updateFilter(c.chan, c.value2);
      break;
    case DIV_CMD_SID3_FILTER_DISTORTION:
      chan[c.chan].filt[c.value2].distortion_level = c.value & 0xff;
      updateFilter(c.chan, c.value2);
      break;
    case DIV_CMD_C64_FILTER_MODE:
      chan[c.chan].filt[(c.value >> 4) & 3].mode &= ~(SID3_FILTER_LP | SID3_FILTER_HP | SID3_FILTER_BP);
      if(c.value & 1) chan[c.chan].filt[(c.value >> 4) & 3].mode |= SID3_FILTER_LP;
      if(c.value & 2) chan[c.chan].filt[(c.value >> 4) & 3].mode |= SID3_FILTER_BP;
      if(c.value & 4) chan[c.chan].filt[(c.value >> 4) & 3].mode |= SID3_FILTER_HP;
      updateFilter(c.chan, (c.value >> 4) & 3);
      break;
    case DIV_CMD_SID3_FILTER_CONNECTION:
      chan[c.chan].filt[(c.value >> 4) & 3].mode &= ~(SID3_FILTER_CHANNEL_INPUT | SID3_FILTER_OUTPUT);
      if(c.value & 1) chan[c.chan].filt[(c.value >> 4) & 3].mode |= SID3_FILTER_CHANNEL_INPUT;
      if(c.value & 2) chan[c.chan].filt[(c.value >> 4) & 3].mode |= SID3_FILTER_OUTPUT;
      updateFilter(c.chan, (c.value >> 4) & 3);
      break;
    case DIV_CMD_SID3_FILTER_MATRIX:
      chan[c.chan].filt[(c.value >> 4) & 3].filter_matrix = c.value & 0xf;
      updateFilter(c.chan, (c.value >> 4) & 3);
      break;
    case DIV_CMD_SID3_FILTER_ENABLE:
      chan[c.chan].filt[(c.value >> 4) & 3].enabled = c.value & 1;
      updateFilter(c.chan, (c.value >> 4) & 3);
      break;
    case DIV_CMD_C64_PW_SLIDE:
      chan[c.chan].pw_slide = c.value * c.value2 * 16;
      break;
    case DIV_CMD_C64_CUTOFF_SLIDE:
      filter = abs(c.value2) - 1;
      chan[c.chan].filt[filter].cutoff_slide = c.value * (c.value2 > 0 ? 1 : -1) * 16;
      break;
    case DIV_CMD_SID3_PHASE_RESET:
      chan[c.chan].phase_reset_counter = c.value;
      break;
    case DIV_CMD_SID3_NOISE_PHASE_RESET:
      chan[c.chan].noise_phase_reset_counter = c.value;
      break;
    case DIV_CMD_SID3_ENVELOPE_RESET:
      chan[c.chan].envelope_reset_counter = c.value;
      break;
    case DIV_CMD_SID3_CUTOFF_SCALING:
      chan[c.chan].filt[(c.value >> 4) & 3].bindCutoffToNote = c.value & 1;
      chan[c.chan].filt[(c.value >> 4) & 3].bindCutoffToNoteDir = c.value & 2;
      chan[c.chan].freqChanged = true;
      break;
    case DIV_CMD_SID3_RESONANCE_SCALING:
      chan[c.chan].filt[(c.value >> 4) & 3].bindResonanceToNote = c.value & 1;
      chan[c.chan].filt[(c.value >> 4) & 3].bindResonanceToNoteDir = c.value & 2;
      chan[c.chan].freqChanged = true;
      break;
    case DIV_CMD_SAMPLE_POS:
      chan[c.chan].dacPos=c.value;
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

  if(updEnv)
  {
    updateEnvelope(c.chan);
  }
  return 1;
}

void DivPlatformSID3::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  sid3_set_is_muted(sid3,ch,mute);
}

void DivPlatformSID3::forceIns() {
  for (int i=0; i<SID3_NUM_CHANNELS; i++) {
    chan[i].insChanged=true;
    if (chan[i].active) {
      chan[i].keyOn=true;
      chan[i].freqChanged=true;
    }
    //updateFilter(i);
  }
}

void DivPlatformSID3::notifyInsChange(int ins) {
  for (int i=0; i<SID3_NUM_CHANNELS; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformSID3::notifyWaveChange(int wave) 
{
  if (chan[SID3_NUM_CHANNELS - 1].wavetable==wave)
  {
    ws.changeWave1(wave, false);
    updateWave();
  }
}

void DivPlatformSID3::notifyInsDeletion(void* ins) {
  for (int i=0; i<SID3_NUM_CHANNELS; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void* DivPlatformSID3::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformSID3::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

DivChannelModeHints DivPlatformSID3::getModeHints(int ch) {
  DivChannelModeHints ret;
  ret.count=1;
  ret.hint[0]=ICON_FA_BELL_SLASH_O;
  ret.type[0]=0;
  if (!chan[ch].gate) {
    ret.type[0]=4;
  }

  return ret;
}

DivDispatchOscBuffer* DivPlatformSID3::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned short DivPlatformSID3::getPan(int ch) {
  return (chan[ch].panLeft<<8)|chan[ch].panRight;
}

unsigned char* DivPlatformSID3::getRegisterPool() {
  return regPool;
}

int DivPlatformSID3::getRegisterPoolSize() {
  return SID3_NUM_REGISTERS;
}

float DivPlatformSID3::getPostAmp() {
  return 1.0f;
}

void DivPlatformSID3::reset() {
  while (!writes.empty()) writes.pop();
  for (int i=0; i<SID3_NUM_CHANNELS; i++) {
    chan[i]=DivPlatformSID3::Channel();
    chan[i].std.setEngine(parent);
    chan[i].vol = SID3_MAX_VOL;

    for(int j = 0; j < SID3_NUM_FILTERS; j++)
    {
      chan[i].filt[j].enabled = false;
      updateFilter(i, j);

      chan[i].filt[j].cutoff_slide = 0;
    }

    chan[i].panLeft = chan[i].panRight = 0xff;

    chan[i].freq = chan[i].noiseFreq = 0;
    updatePanning(i);

    chan[i].noiseLFSRMask = 1 | (1 << 23) | (1 << 25) | (1 << 29); //https://docs.amd.com/v/u/en-US/xapp052 for 30 bits: 30, 6, 4, 1; but inverted since our LFSR is moving in different direction

    chan[i].pw_slide = 0;

    chan[i].phase_reset_counter = -1;
    chan[i].noise_phase_reset_counter = -1;
    chan[i].envelope_reset_counter = -1;
  }

  sampleTick = 0;
  updateSample = false;

  ws.setEngine(parent);
  ws.init(NULL,256,255,false);

  sid3_reset(sid3);
  memset(regPool,0,SID3_NUM_REGISTERS);
}

int DivPlatformSID3::getOutputCount() {
  return 2;
}

bool DivPlatformSID3::hasSoftPan(int ch) {
  return true;
}

bool DivPlatformSID3::getDCOffRequired()
{
  return false;
}

void DivPlatformSID3::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformSID3::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

void DivPlatformSID3::setFlags(const DivConfig& flags) {
  chipClock=1000000;
  CHECK_CUSTOM_CLOCK;

  quarterClock=flags.getBool("quarterClock",false);

  if(quarterClock && chipClock >= 1000000 && !parent->isExporting())
  {
    chipClock /= 4;
  }

  rate=chipClock;
  sid3_set_clock_rate(sid3, chipClock);
  for (int i=0; i<SID3_NUM_CHANNELS; i++) {
    oscBuf[i]->setRate(rate);
  }
}

void DivPlatformSID3::getPaired(int ch, std::vector<DivChannelPair>& ret)
{
  if(chan[ch].phase)
  {
    ret.push_back(DivChannelPair(_("phase"), chan[ch].phaseSrc));
  }
  if(chan[ch].ring)
  {
    if(chan[ch].ringSrc == SID3_NUM_CHANNELS)
    {
      ret.push_back(DivChannelPair(_("ring"), ch));
    }
    else
    {
      ret.push_back(DivChannelPair(_("ring"), chan[ch].ringSrc));
    }
  }
  if(chan[ch].sync)
  {
    ret.push_back(DivChannelPair(_("sync"), chan[ch].syncSrc));
  }
}

int DivPlatformSID3::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  writeOscBuf=0;
  
  for (int i=0; i<SID3_NUM_CHANNELS; i++) 
  {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }

  sid3 = sid3_create();

  setFlags(flags);

  reset();

  return SID3_NUM_CHANNELS;
}

void DivPlatformSID3::quit() {
  for (int i=0; i<SID3_NUM_CHANNELS; i++) 
  {
    delete oscBuf[i];
  }
  if (sid3!=NULL)
  {
    sid3_free(sid3);
    sid3 = NULL;
  }
}

DivPlatformSID3::~DivPlatformSID3() {
}
