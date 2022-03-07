/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2022 tildearrow and contributors
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

#ifndef _VERA_H
#define _VERA_H
#include "../dispatch.h"
#include "../instrument.h"
#include "../macroInt.h"

class DivPlatformVERA: public DivDispatch {
  protected:
    struct Channel {
      int freq, baseFreq, pitch, note;
      unsigned char ins;
      bool active, freqChanged, inPorta;
      int vol, outVol;
      unsigned accum;
      int noiseval;
      DivMacroInt std;

      struct PCMChannel {
        int sample;
        int out_l, out_r;
        unsigned pos;
        unsigned len;
        unsigned char freq, pan;
        PCMChannel(): sample(-1), out_l(0), out_r(0), pos(0), len(0), freq(0), pan(3) {}
      } pcm;
      Channel(): freq(0), baseFreq(0), pitch(0), note(0), ins(-1), active(false), freqChanged(false), inPorta(false), vol(0), outVol(0), accum(0), noiseval(0) {}
    };
    Channel chan[17];
    bool isMuted[17];
    unsigned noiseState, noiseOut;
    unsigned char regPool[66];
  
    int calcNoteFreq(int ch, int note);
    friend void putDispatchChan(void*,int,int);
  
  public:
    void acquire(short* bufL, short* bufR, size_t start, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    void reset();
    void tick();
    void muteChannel(int ch, bool mute);
    void notifyInsDeletion(void* ins);
    bool isStereo();
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    const char* getEffectName(unsigned char effect);
    int init(DivEngine* parent, int channels, int sugRate, unsigned int flags);
    ~DivPlatformVERA();
};
#endif
