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

#ifndef _KLATTSCH_H
#define _KLATTSCH_H

#include "../dispatch.h"
#include <klattsch/banks.hpp>
#include <memory>
#include <vector>

#define KLATTSCH_MAX_CHANS 16

class DivPlatformKlattsch: public DivDispatch {
  struct Channel: public SharedChannel {
    enum class QueuedEventKind {
      None,
      Glide,
      Stop,
    };

    std::unique_ptr<klattsch::Synth> synth;
    klattsch::ScheduleEvent queuedEvent;
    QueuedEventKind queuedEventKind;
    // Direct 12xx-17xx parameters dispatched during the current tracker tick.
    // Row effects arrive before note-on, so key-on must reapply these after the
    // phoneme initializes the synth.
    klattsch::ParamUpdate directUpdate;
    // Transition-sensitive targets dispatched during the current tracker tick.
    // A later 11xx reapplies them before audio renders so column order does not
    // change the transition duration.
    klattsch::ParamUpdate pendingTransitionUpdate;
    int phonemeIndex;
    bool phonemeChanged;
    int transitionTicks;
    bool noteActive;
    // -1 uses the phoneme default; tilt itself may be negative.
    float ovVoicing, ovAspiration, ovTilt, ovEffort, ovGain;
    // vibrato depth is in Hz; tremolo depth is 0-1; rates are in Hz.
    float vibDepth, vibRate, tremDepth, tremRate;
    float bwScale;
    float formantShift;
    // direct formant targets survive shifts until the next phoneme or note-on.
    bool formantOverridden[3];
    float panL, panR;
    Channel(bool linear=true):
      SharedChannel(15,linear),
      queuedEvent{},
      queuedEventKind(QueuedEventKind::None),
      phonemeIndex(6),  // AH by default
      phonemeChanged(false),
      transitionTicks(0),
      noteActive(false),
      ovVoicing(-1.0f),
      ovAspiration(-1.0f),
      ovTilt(-1.0f),
      ovEffort(-1.0f),
      ovGain(-1.0f),
      vibDepth(0.0f),
      vibRate(5.0f),
      tremDepth(0.0f),
      tremRate(5.0f),
      bwScale(1.0f),
      formantShift(1.0f),
      panL(1.0f),
      panR(1.0f) {
        formantOverridden[0]=false;
        formantOverridden[1]=false;
        formantOverridden[2]=false;
      }
  };
  Channel chan[KLATTSCH_MAX_CHANS];
  DivDispatchOscBuffer* oscBuf[KLATTSCH_MAX_CHANS];
  bool isMuted[KLATTSCH_MAX_CHANS];
  int chans;
  const klattsch::PhonemeBank* bank;
  std::vector<float> renderScratch;
  std::vector<float> mixScratch;
  std::vector<float> mixScratchR;
  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);

  const klattsch::PhonemeRecord* phonemeAt(int idx) const;
  klattsch::Params shapedParams(const klattsch::Params& src, const Channel& ch, float f0Hz);
  void applyPhoneme(Channel& ch, const klattsch::PhonemeRecord& rec, float f0Hz, unsigned int transitionSamples);
  int baseFreqForNote(int note) const;
  void setBaseNote(Channel& ch, int note);
  void convertBaseFreqMode(Channel& ch, bool raw);
  float effectiveF0(Channel& ch);
  float samplesPerTick();
  unsigned int transitionSamples(const Channel& ch);
  unsigned int msToSamples(float ms);
  unsigned int slotSamples();
  void setTarget(Channel& ch, const klattsch::ParamUpdate& update, unsigned int transitionSamples, bool sticky);
  void rememberTransitionUpdate(Channel& ch, const klattsch::ParamUpdate& update);
  void setEffectTarget(Channel& ch, const klattsch::ParamUpdate& update, bool sticky);
  void scaleTargets(Channel& ch, const klattsch::ParamId* ids, size_t count, float ratio);
  void pushImmediate(Channel& ch, klattsch::ParamId id, float value, bool sticky=false);
  void pushDirect(Channel& ch, klattsch::ParamId id, float value);
  void applyDirectUpdate(Channel& ch, const klattsch::ParamUpdate& update, unsigned int transitionSamples);
  void pushVoiceParam(Channel& ch, klattsch::ParamId id, float overrideValue);

  public:
    void acquire(short** buf, size_t len);
    int dispatch(DivCommand c);
    SharedChannel* getChanState(int chan);
    DivMacroInt* getChanMacroInt(int ch);
    DivDispatchOscBuffer* getOscBuffer(int chan);
    int getOutputCount();
    bool hasSoftPan(int ch);
    unsigned short getPan(int chan);
    void reset();
    void forceIns();
    void tick(bool sysTick=true);
    void muteChannel(int ch, bool mute);
    bool keyOffAffectsArp(int ch);
    void setFlags(const DivConfig& flags);
    void notifyInsDeletion(void* ins);
    void notifyPitchTable(int sample=-1);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    ~DivPlatformKlattsch();
};

#endif
