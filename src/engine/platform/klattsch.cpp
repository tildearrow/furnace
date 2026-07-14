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

#include "klattsch.h"
#include "../engine.h"

#include <cmath>

#define KLATTSCH_RATE 48000
// leave headroom for four-voice unisons.
#define KLATTSCH_OUT_SCALE 0.26f
#define KLATTSCH_MIX_CLIP_THRESHOLD 0.85f

static float furnaceNoteToHz(float note, float tuning) {
  // dispatch notes are offset by 60 from the note shown in the GUI.
  return tuning*std::pow(2.0f,(note-117.0f)/12.0f);
}

static float softClipMix(float x) {
  const float magnitude=std::fabs(x);
  if (magnitude<=KLATTSCH_MIX_CLIP_THRESHOLD) return x;
  const float excess=magnitude-KLATTSCH_MIX_CLIP_THRESHOLD;
  const float clipped=KLATTSCH_MIX_CLIP_THRESHOLD+
    (1.0f-KLATTSCH_MIX_CLIP_THRESHOLD)*excess/(excess+1.0f);
  return std::copysign(clipped,x);
}

static klattsch::ParamUpdate changedParams(const klattsch::Params& from, const klattsch::Params& to) {
  const klattsch::ParamUpdate fromUpdate=klattsch::paramsToFullUpdate(from);
  const klattsch::ParamUpdate toUpdate=klattsch::paramsToFullUpdate(to);
  klattsch::ParamUpdate changed;
  for (size_t i=0; i<klattsch::kNumParams; i++) {
    if (fromUpdate.values[i]!=toUpdate.values[i]) {
      changed.mask|=(uint32_t{1}<<i);
      changed.values[i]=toUpdate.values[i];
    }
  }
  return changed;
}

const klattsch::PhonemeRecord* DivPlatformKlattsch::phonemeAt(int idx) const {
  if (bank==NULL || idx<0 || (size_t)idx>=bank->count) return NULL;
  return &bank->records[idx];
}

klattsch::Params DivPlatformKlattsch::shapedParams(const klattsch::Params& src, const Channel& ch, float f0Hz) {
  klattsch::Params p=src;
  p.f0=f0Hz;
  p.f1*=ch.formantShift;
  p.f2*=ch.formantShift;
  p.f3*=ch.formantShift;
  p.bw1*=ch.formantShift*ch.bwScale;
  p.bw2*=ch.formantShift*ch.bwScale;
  p.bw3*=ch.formantShift*ch.bwScale;
  if (ch.ovVoicing>=0.0f) p.voicing=ch.ovVoicing;
  if (ch.ovAspiration>=0.0f) p.aspiration=ch.ovAspiration;
  if (ch.ovTilt>-1.0f) p.tilt=ch.ovTilt;
  if (ch.ovEffort>=0.0f) p.effort=ch.ovEffort;
  if (ch.ovGain>=0.0f) p.gain=ch.ovGain;
  p.vibratoDepth=ch.vibDepth;
  p.vibratoRate=ch.vibRate;
  p.tremoloDepth=ch.tremDepth;
  p.tremoloRate=ch.tremRate;
  return p;
}

unsigned int DivPlatformKlattsch::msToSamples(float ms) {
  return MAX((unsigned int)(ms*(float)KLATTSCH_RATE/1000.0f),1u);
}

unsigned int DivPlatformKlattsch::slotSamples() {
  const int speed=parent->getPreviewSpeed();
  const float ticks=(float)MAX(speed,1);
  return MAX((unsigned int)(ticks*samplesPerTick()),1u);
}

void DivPlatformKlattsch::applyPhoneme(Channel& ch, const klattsch::PhonemeRecord& rec, float f0Hz, unsigned int txnSamples) {
  ch.formantOverridden[0]=false;
  ch.formantOverridden[1]=false;
  ch.formantOverridden[2]=false;
  if (!ch.synth) return;
  ch.synth->clearSchedule();
  ch.queuedEventKind=Channel::QueuedEventKind::None;
  // follow the reference sequencer's closure/burst timing for stops.
  if (rec.isStop) {
    const float slotMs=1000.0f*(float)slotSamples()/(float)KLATTSCH_RATE;
    const float burstMs=MIN(25.0f,slotMs*0.3f);
    const float silenceMs=slotMs-burstMs;
    klattsch::ParamUpdate closure;
    closure.set(klattsch::ParamId::A1,0.0f);
    closure.set(klattsch::ParamId::A2,0.0f);
    closure.set(klattsch::ParamId::A3,0.0f);
    ch.synth->setTarget(closure,msToSamples(MIN(20.0f,silenceMs*0.4f)));
    ch.queuedEvent.atSample=msToSamples(silenceMs);
    ch.queuedEvent.transitionSamples=msToSamples(MIN(5.0f,burstMs*0.2f));
    ch.queuedEvent.target=klattsch::paramsToFullUpdate(shapedParams(rec.params,ch,f0Hz));
    ch.queuedEventKind=Channel::QueuedEventKind::Stop;
    ch.synth->setScheduleView(&ch.queuedEvent,1);
    return;
  }
  // glide records use Txx for both the onset and terminal target.
  // Txx=0 applies only the onset.
  if (rec.hasGlide && txnSamples>1) {
    const klattsch::Params onset=shapedParams(rec.params,ch,f0Hz);
    const klattsch::Params terminal=shapedParams(rec.glideTo,ch,f0Hz);
    ch.synth->setTarget(klattsch::paramsToFullUpdate(onset),txnSamples);
    ch.queuedEvent.atSample=txnSamples;
    ch.queuedEvent.transitionSamples=txnSamples;
    ch.queuedEvent.target=changedParams(onset,terminal);
    if (ch.queuedEvent.target.mask!=0) {
      ch.queuedEventKind=Channel::QueuedEventKind::Glide;
      ch.synth->setScheduleView(&ch.queuedEvent,1);
    }
  } else {
    ch.synth->setTarget(klattsch::paramsToFullUpdate(shapedParams(rec.params,ch,f0Hz)),MAX(txnSamples,1u));
  }
}

int DivPlatformKlattsch::baseFreqForNote(int note) const {
  return (note&DIV_NOTE_RAW_FLAG)?(note&(~DIV_NOTE_RAW_FLAG)):(note<<7);
}

void DivPlatformKlattsch::setBaseNote(Channel& ch, int note) {
  ch.rawFreq=note&DIV_NOTE_RAW_FLAG;
  ch.baseFreq=baseFreqForNote(note);
}

float DivPlatformKlattsch::effectiveF0(Channel& ch) {
  if (ch.rawFreq) return (float)(ch.baseFreq+ch.pitch2);
  float n;
  if (ch.fixedArp) {
    n=(float)ch.baseNoteOverride;
  } else {
    n=(float)ch.baseFreq/128.0f+(float)ch.arpOff;
  }
  n+=(float)(ch.pitch+ch.pitch2)/128.0f;
  return furnaceNoteToHz(n,parent->song.tuning);
}

void DivPlatformKlattsch::convertBaseFreqMode(Channel& ch, bool raw) {
  if (ch.rawFreq==raw) return;
  const float currentHz=effectiveF0(ch);
  if (raw) {
    // raw mode applies pitch2 as a linear-Hz offset.
    ch.baseFreq=(int)std::lround(currentHz)-(int)ch.pitch2;
    ch.rawFreq=true;
    return;
  }

  // Normal mode uses 1/128-semitone base units. Preserve the current audible
  // pitch while changing domains, accounting for the offsets effectiveF0 adds.
  if (currentHz>0.0f && parent->song.tuning>0.0f) {
    const float note=117.0f+12.0f*std::log2(currentHz/parent->song.tuning);
    ch.baseFreq=(int)std::lround(note*128.0f)-(ch.arpOff<<7)-ch.pitch-ch.pitch2;
  } else {
    ch.baseFreq=0;
  }
  ch.rawFreq=false;
}

float DivPlatformKlattsch::samplesPerTick() {
  const float tempoN=(float)MAX((int)parent->getVirtualTempoN(),1);
  const float tempoD=(float)MAX((int)parent->getVirtualTempoD(),1);
  const float ticksPerSec=MAX(1.0f,parent->getCurHz()*tempoN/tempoD);
  return (float)KLATTSCH_RATE/ticksPerSec;
}

unsigned int DivPlatformKlattsch::transitionSamples(const Channel& ch) {
  return MAX((unsigned int)(ch.transitionTicks*samplesPerTick()),1u);
}

void DivPlatformKlattsch::setTarget(Channel& ch, const klattsch::ParamUpdate& update, unsigned int txnSamples, bool sticky) {
  if (!ch.synth) return;
  ch.synth->setTarget(update,txnSamples);
  if (ch.queuedEventKind==Channel::QueuedEventKind::Glide) {
    ch.queuedEvent.target.mask&=~update.mask;
    if (ch.queuedEvent.target.mask==0) {
      ch.synth->clearSchedule();
      ch.queuedEventKind=Channel::QueuedEventKind::None;
    }
  } else if (sticky && ch.queuedEventKind==Channel::QueuedEventKind::Stop) {
    for (size_t i=0; i<klattsch::kNumParams; i++) {
      const uint32_t bit=uint32_t{1}<<i;
      if (update.mask&bit) {
        ch.queuedEvent.target.mask|=bit;
        ch.queuedEvent.target.values[i]=update.values[i];
      }
    }
  }
}

void DivPlatformKlattsch::rememberTransitionUpdate(Channel& ch, const klattsch::ParamUpdate& update) {
  for (size_t i=0; i<klattsch::kNumParams; i++) {
    const uint32_t bit=uint32_t{1}<<i;
    if (update.mask&bit) {
      ch.pendingTransitionUpdate.mask|=bit;
      ch.pendingTransitionUpdate.values[i]=update.values[i];
    }
  }
}

void DivPlatformKlattsch::setEffectTarget(Channel& ch, const klattsch::ParamUpdate& update, bool sticky) {
  rememberTransitionUpdate(ch,update);
  setTarget(ch,update,transitionSamples(ch),sticky);
}

void DivPlatformKlattsch::scaleTargets(Channel& ch, const klattsch::ParamId* ids, size_t count, float ratio) {
  if (!ch.synth) return;
  klattsch::ParamUpdate u;
  for (size_t i=0; i<count; i++) {
    const klattsch::ParamId id=ids[i];
    u.set(id,ch.synth->target(id)*ratio);
    if (ch.queuedEventKind!=Channel::QueuedEventKind::None && ch.queuedEvent.target.has(id)) {
      const size_t paramIndex=static_cast<size_t>(id);
      ch.queuedEvent.target.values[paramIndex]*=ratio;
    }
  }
  rememberTransitionUpdate(ch,u);
  ch.synth->setTarget(u,transitionSamples(ch));
}

void DivPlatformKlattsch::pushImmediate(Channel& ch, klattsch::ParamId id, float value, bool sticky) {
  klattsch::ParamUpdate u;
  u.set(id,value);
  setEffectTarget(ch,u,sticky);
}

void DivPlatformKlattsch::pushDirect(Channel& ch, klattsch::ParamId id, float value) {
  ch.directUpdate.set(id,value);
  pushImmediate(ch,id,value,true);
}

void DivPlatformKlattsch::applyDirectUpdate(Channel& ch, const klattsch::ParamUpdate& update, unsigned int txnSamples) {
  if (update.has(klattsch::ParamId::F1)) ch.formantOverridden[0]=true;
  if (update.has(klattsch::ParamId::F2)) ch.formantOverridden[1]=true;
  if (update.has(klattsch::ParamId::F3)) ch.formantOverridden[2]=true;
  setTarget(ch,update,txnSamples,true);
}

void DivPlatformKlattsch::pushVoiceParam(Channel& ch, klattsch::ParamId id, float overrideValue) {
  float value=overrideValue;
  if (value<=-1.0f) {
    if (!ch.noteActive) return;
    const klattsch::PhonemeRecord* rec=phonemeAt(ch.phonemeIndex);
    if (rec==NULL) return;
    switch (id) {
      case klattsch::ParamId::Voicing: value=rec->params.voicing; break;
      case klattsch::ParamId::Aspiration: value=rec->params.aspiration; break;
      case klattsch::ParamId::Tilt: value=rec->params.tilt; break;
      case klattsch::ParamId::Effort: value=rec->params.effort; break;
      case klattsch::ParamId::Gain: value=rec->params.gain; break;
      default: return;
    }
  }
  pushImmediate(ch,id,value,true);
}

void DivPlatformKlattsch::acquire(short** buf, size_t len) {
  if (renderScratch.size()<len) renderScratch.resize(len);
  if (mixScratch.size()<len) mixScratch.resize(len);
  if (mixScratchR.size()<len) mixScratchR.resize(len);
  std::fill(mixScratch.begin(),mixScratch.begin()+len,0.0f);
  std::fill(mixScratchR.begin(),mixScratchR.begin()+len,0.0f);
  for (int ch=0; ch<chans; ch++) {
    oscBuf[ch]->begin(len);
    if (chan[ch].synth) {
      chan[ch].synth->process(renderScratch.data(),len);
      const float volScale=((float)chan[ch].outVol/15.0f)*KLATTSCH_OUT_SCALE;
      const float pl=chan[ch].panL, pr=chan[ch].panR;
      for (size_t i=0; i<len; i++) {
        float s=renderScratch[i]*volScale;
        oscBuf[ch]->putSample(i,(short)CLAMP((int)(s*32767.0f),-32768,32767));
        if (!isMuted[ch]) {
          mixScratch[i]+=s*pl;
          mixScratchR[i]+=s*pr;
        }
      }
    }
    oscBuf[ch]->end(len);
  }
  for (size_t i=0; i<len; i++) {
    buf[0][i]=(short)CLAMP((int)(softClipMix(mixScratch[i])*32767.0f),-32768,32767);
    buf[1][i]=(short)CLAMP((int)(softClipMix(mixScratchR[i])*32767.0f),-32768,32767);
  }
}

void DivPlatformKlattsch::tick(bool sysTick) {
  for (int i=0; i<chans; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=(chan[i].vol*MIN(chan[i].std.vol.val,15))/15;
    }
    if (!chan[i].rawFreq) {
      if (NEW_ARP_STRAT) {
        chan[i].handleArp();
      } else if (chan[i].std.arp.had) {
        if (!chan[i].inPorta) {
          chan[i].baseFreq=parent->calcArp(chan[i].note,chan[i].std.arp.val)<<7;
          chan[i].freqChanged=true;
        }
      }
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
    if (chan[i].std.panL.had) {
      chan[i].panL=(float)((chan[i].std.panL.val&0x7f)*2)/255.0f;
    }
    if (chan[i].std.panR.had) {
      chan[i].panR=(float)((chan[i].std.panR.val&0x7f)*2)/255.0f;
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff || chan[i].phonemeChanged) {
      const unsigned int txnSamples=transitionSamples(chan[i]);
      // use a fixed short release so Txx cannot make note-off sluggish.
      const unsigned int releaseSamples=(unsigned int)(KLATTSCH_RATE*0.030f);
      const float f0Hz=effectiveF0(chan[i]);

      const bool applyDirect=!chan[i].keyOff && (chan[i].keyOn || chan[i].phonemeChanged);
      if (chan[i].keyOn) {
        const klattsch::PhonemeRecord* rec=phonemeAt(chan[i].phonemeIndex);
        if (rec) {
          applyPhoneme(chan[i],*rec,f0Hz,txnSamples);
        }
        chan[i].noteActive=true;
        chan[i].keyOn=false;
      } else if (chan[i].keyOff) {
        if (chan[i].synth) {
          chan[i].synth->clearSchedule();
          chan[i].queuedEventKind=Channel::QueuedEventKind::None;
          klattsch::ParamUpdate u;
          u.set(klattsch::ParamId::Voicing,0.0f);
          u.set(klattsch::ParamId::A1,0.0f);
          u.set(klattsch::ParamId::A2,0.0f);
          u.set(klattsch::ParamId::A3,0.0f);
          u.set(klattsch::ParamId::Aspiration,0.0f);
          chan[i].synth->setTarget(u,MAX(releaseSamples,1u));
        }
        chan[i].noteActive=false;
        chan[i].keyOff=false;
      } else if (chan[i].noteActive && chan[i].synth) {
        if (chan[i].phonemeChanged) {
          const klattsch::PhonemeRecord* rec=phonemeAt(chan[i].phonemeIndex);
          if (rec) applyPhoneme(chan[i],*rec,f0Hz,txnSamples);
        } else {
          klattsch::ParamUpdate u;
          u.set(klattsch::ParamId::F0,f0Hz);
          setTarget(chan[i],u,txnSamples,true);
        }
      }
      if (applyDirect && chan[i].directUpdate.mask!=0) {
        applyDirectUpdate(chan[i],chan[i].directUpdate,txnSamples);
      }
      chan[i].freqChanged=false;
      chan[i].phonemeChanged=false;
    }
    chan[i].directUpdate.clear();
    chan[i].pendingTransitionUpdate.clear();
  }
}

int DivPlatformKlattsch::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON:
      if (c.value!=DIV_NOTE_NULL) {
        setBaseNote(chan[c.chan],c.value);
        chan[c.chan].note=c.value;
        chan[c.chan].freqChanged=true;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOff=false;
      chan[c.chan].keyOn=true;
      chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_STD));
      if (!parent->song.compatFlags.brokenOutVol && !chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
      }
      break;
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].active=false;
      chan[c.chan].keyOn=false;
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
      }
      break;
    case DIV_CMD_VOLUME:
      if (chan[c.chan].vol!=c.value) {
        chan[c.chan].vol=c.value;
        if (!chan[c.chan].std.vol.has) {
          chan[c.chan].outVol=c.value;
        }
      }
      break;
    case DIV_CMD_GET_VOLUME:
      return chan[c.chan].vol;
    case DIV_CMD_PITCH:
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_NOTE_PORTA: {
      Channel& ch=chan[c.chan];
      const bool targetRaw=c.value2&DIV_NOTE_RAW_FLAG;
      const bool modeChanged=ch.rawFreq!=targetRaw;
      convertBaseFreqMode(ch,targetRaw);
      if (modeChanged) ch.note=c.value2;
      const int destFreq=baseFreqForNote(c.value2);
      bool return2=false;
      if (destFreq>ch.baseFreq) {
        ch.baseFreq+=c.value;
        if (ch.baseFreq>=destFreq) {
          ch.baseFreq=destFreq;
          return2=true;
        }
      } else {
        ch.baseFreq-=c.value;
        if (ch.baseFreq<=destFreq) {
          ch.baseFreq=destFreq;
          return2=true;
        }
      }
      ch.freqChanged=true;
      if (return2) {
        ch.inPorta=false;
        return 2;
      }
      break;
    }
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.compatFlags.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_STD));
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.compatFlags.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) setBaseNote(chan[c.chan],chan[c.chan].note);
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_LEGATO:
      chan[c.chan].note=c.value;
      setBaseNote(chan[c.chan],c.value);
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_PANNING:
      chan[c.chan].panL=(float)c.value/255.0f;
      chan[c.chan].panR=(float)c.value2/255.0f;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 15;
    case DIV_CMD_MACRO_OFF:
      chan[c.chan].std.mask(c.value,true);
      break;
    case DIV_CMD_MACRO_ON:
      chan[c.chan].std.mask(c.value,false);
      break;
    case DIV_CMD_MACRO_RESTART:
      chan[c.chan].std.restart(c.value);
      break;
    case DIV_CMD_KLATTSCH_PHONEME: {
      const klattsch::PhonemeRecord* rec=phonemeAt(c.value);
      if (rec) {
        chan[c.chan].phonemeIndex=c.value;
        // Defer application until tick(), after every effect column on the row
        // has supplied its transition time and direct parameter overrides.
        chan[c.chan].phonemeChanged=true;
      }
      break;
    }
    case DIV_CMD_KLATTSCH_TRANSITION: {
      Channel& ch=chan[c.chan];
      ch.transitionTicks=c.value&0xff;
      if (ch.synth && ch.pendingTransitionUpdate.mask!=0) {
        // No audio is rendered between row effects, so setting the same targets
        // again replaces their stale ramp without producing an intermediate one.
        ch.synth->setTarget(ch.pendingTransitionUpdate,transitionSamples(ch));
      }
      break;
    }
    case DIV_CMD_KLATTSCH_FORMANT: {
      // c.value: formant index 0-2. c.value2: effect value.
      // F1 has 10 Hz resolution (max 2550 Hz); F2/F3 have 16 Hz (max 4080 Hz).
      const float freq=(c.value==0)?(c.value2*10.0f):(c.value2*16.0f);
      static const klattsch::ParamId ids[3]={klattsch::ParamId::F1,klattsch::ParamId::F2,klattsch::ParamId::F3};
      if (c.value>=0 && c.value<3) {
        chan[c.chan].formantOverridden[c.value]=true;
        pushDirect(chan[c.chan],ids[c.value],freq);
      }
      break;
    }
    case DIV_CMD_KLATTSCH_AMP: {
      static const klattsch::ParamId ids[3]={klattsch::ParamId::A1,klattsch::ParamId::A2,klattsch::ParamId::A3};
      if (c.value>=0 && c.value<3) pushDirect(chan[c.chan],ids[c.value],(float)(c.value2&0xff)/255.0f);
      break;
    }
    case DIV_CMD_KLATTSCH_VOICING:
      chan[c.chan].ovVoicing=(c.value==0xff)?-1.0f:((float)c.value/255.0f);
      pushVoiceParam(chan[c.chan],klattsch::ParamId::Voicing,chan[c.chan].ovVoicing);
      break;
    case DIV_CMD_KLATTSCH_ASPIRATION:
      chan[c.chan].ovAspiration=(c.value==0xff)?-1.0f:((float)c.value/255.0f);
      pushVoiceParam(chan[c.chan],klattsch::ParamId::Aspiration,chan[c.chan].ovAspiration);
      break;
    case DIV_CMD_KLATTSCH_TILT:
      if ((c.value&0xff)==0xff) {
        chan[c.chan].ovTilt=-1.0f;
      } else {
        const int byteValue=c.value&0xff;
        const int signedValue=(byteValue<0x80)?byteValue:(byteValue-0x100);
        chan[c.chan].ovTilt=0.95f*(float)signedValue/(float)((signedValue<0)?128:127);
      }
      pushVoiceParam(chan[c.chan],klattsch::ParamId::Tilt,chan[c.chan].ovTilt);
      break;
    case DIV_CMD_KLATTSCH_EFFORT:
      chan[c.chan].ovEffort=(c.value==0xff)?-1.0f:((float)c.value/255.0f);
      pushVoiceParam(chan[c.chan],klattsch::ParamId::Effort,chan[c.chan].ovEffort);
      break;
    case DIV_CMD_KLATTSCH_VIBRATO: {
      chan[c.chan].vibRate=(float)((c.value>>4)&15);
      chan[c.chan].vibDepth=(float)(c.value&15)*4.0f;
      if (chan[c.chan].synth) {
        klattsch::ParamUpdate u;
        u.set(klattsch::ParamId::VibratoRate,chan[c.chan].vibRate);
        u.set(klattsch::ParamId::VibratoDepth,chan[c.chan].vibDepth);
        setEffectTarget(chan[c.chan],u,true);
      }
      break;
    }
    case DIV_CMD_KLATTSCH_TREMOLO: {
      chan[c.chan].tremRate=(float)((c.value>>4)&15);
      chan[c.chan].tremDepth=(float)(c.value&15)/15.0f;
      if (chan[c.chan].synth) {
        klattsch::ParamUpdate u;
        u.set(klattsch::ParamId::TremoloRate,chan[c.chan].tremRate);
        u.set(klattsch::ParamId::TremoloDepth,chan[c.chan].tremDepth);
        setEffectTarget(chan[c.chan],u,true);
      }
      break;
    }
    case DIV_CMD_KLATTSCH_GAIN:
      chan[c.chan].ovGain=(c.value==0)?-1.0f:((float)c.value/16.0f);
      pushVoiceParam(chan[c.chan],klattsch::ParamId::Gain,chan[c.chan].ovGain);
      break;
    case DIV_CMD_KLATTSCH_BW_SCALE: {
      const float oldScale=chan[c.chan].bwScale;
      chan[c.chan].bwScale=(c.value==0)?1.0f:((float)c.value/64.0f);
      if (chan[c.chan].synth && chan[c.chan].noteActive) {
        static const klattsch::ParamId ids[3]={
          klattsch::ParamId::BW1,
          klattsch::ParamId::BW2,
          klattsch::ParamId::BW3,
        };
        scaleTargets(chan[c.chan],ids,3,chan[c.chan].bwScale/oldScale);
      }
      break;
    }
    case DIV_CMD_KLATTSCH_FORMANT_SHIFT: {
      const float oldShift=chan[c.chan].formantShift;
      chan[c.chan].formantShift=(c.value==0)?1.0f:((float)c.value/64.0f);
      if (chan[c.chan].synth && chan[c.chan].noteActive) {
        klattsch::ParamId ids[6];
        size_t count=0;
        if (!chan[c.chan].formantOverridden[0]) ids[count++]=klattsch::ParamId::F1;
        if (!chan[c.chan].formantOverridden[1]) ids[count++]=klattsch::ParamId::F2;
        if (!chan[c.chan].formantOverridden[2]) ids[count++]=klattsch::ParamId::F3;
        ids[count++]=klattsch::ParamId::BW1;
        ids[count++]=klattsch::ParamId::BW2;
        ids[count++]=klattsch::ParamId::BW3;
        scaleTargets(chan[c.chan],ids,count,chan[c.chan].formantShift/oldShift);
      }
      break;
    }
    default:
      break;
  }
  return 1;
}

void DivPlatformKlattsch::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
}

void DivPlatformKlattsch::forceIns() {
  for (int i=0; i<chans; i++) {
    if (chan[i].synth) chan[i].synth->finishPending();
    chan[i].queuedEventKind=Channel::QueuedEventKind::None;
  }
}

SharedChannel* DivPlatformKlattsch::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformKlattsch::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

DivDispatchOscBuffer* DivPlatformKlattsch::getOscBuffer(int ch) {
  return oscBuf[ch];
}

int DivPlatformKlattsch::getOutputCount() {
  return 2;
}

bool DivPlatformKlattsch::hasSoftPan(int ch) {
  return true;
}

unsigned short DivPlatformKlattsch::getPan(int ch) {
  return (((unsigned char)(chan[ch].panL*255.0f))<<8)|((unsigned char)(chan[ch].panR*255.0f));
}

void DivPlatformKlattsch::reset() {
  for (int i=0; i<chans; i++) {
    std::unique_ptr<klattsch::Synth> synth=std::move(chan[i].synth);
    chan[i]=DivPlatformKlattsch::Channel(parent->song.compatFlags.linearPitch);
    chan[i].std.setEngine(parent);
    if (synth) {
      synth->reset();
    } else {
      synth.reset(new klattsch::Synth(KLATTSCH_RATE));
    }
    chan[i].synth=std::move(synth);
  }
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }
}

bool DivPlatformKlattsch::keyOffAffectsArp(int ch) {
  return true;
}

void DivPlatformKlattsch::setFlags(const DivConfig& flags) {
  chipClock=KLATTSCH_RATE;
  rate=KLATTSCH_RATE;
  for (int i=0; i<chans; i++) {
    oscBuf[i]->setRate(rate);
  }
  bank=&klattsch::builtInBanks().resolve(flags.getString("bank","ja-mokhtari-2000"));
}

void DivPlatformKlattsch::notifyInsDeletion(void* ins) {
  for (int i=0; i<chans; i++) {
    chan[i].std.notifyInsDeletion(static_cast<DivInstrument*>(ins));
  }
}

void DivPlatformKlattsch::notifyPitchTable(int sample) {
  for (int i=0; i<chans; i++) {
    if (!chan[i].rawFreq) chan[i].freqChanged=true;
  }
}

void DivPlatformKlattsch::poke(unsigned int addr, unsigned short val) {
}

void DivPlatformKlattsch::poke(std::vector<DivRegWrite>& wlist) {
}

int DivPlatformKlattsch::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  chans=CLAMP(channels,1,KLATTSCH_MAX_CHANS);
  for (int i=0; i<chans; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  setFlags(flags);
  reset();
  return chans;
}

void DivPlatformKlattsch::quit() {
  for (int i=0; i<chans; i++) {
    delete oscBuf[i];
  }
}

DivPlatformKlattsch::~DivPlatformKlattsch() {
}
