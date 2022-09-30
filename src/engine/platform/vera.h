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

struct VERA_PSG;
struct VERA_PCM;

class DivPlatformVERA: public DivDispatch {
  protected:
    struct Channel {
      int freq, baseFreq, pitch, pitch2, note, ins;
      unsigned char pan;
      bool active, freqChanged, inPorta;
      int vol, outVol;
      unsigned accum;
      int noiseval;
      DivMacroInt std;

      struct PCMChannel {
        int sample;
        unsigned pos;
        unsigned len;
        unsigned char freq;
        bool depth16;
        PCMChannel(): sample(-1), pos(0), len(0), freq(0), depth16(false) {}
      } pcm;
      // somebody please split this into multiple lines!
      void macroInit(DivInstrument* which) {
        std.init(which);
        pitch2=0;
      }
      Channel(): freq(0), baseFreq(0), pitch(0), pitch2(0), note(0), ins(-1), pan(0), active(false), freqChanged(false), inPorta(false), vol(0), outVol(0), accum(0), noiseval(0) {}
    };
    Channel chan[17];
    DivDispatchOscBuffer* oscBuf[17];
    bool isMuted[17];
    unsigned char regPool[67];
    struct VERA_PSG* psg;
    struct VERA_PCM* pcm;
  
    int calcNoteFreq(int ch, int note);
    friend void putDispatchChip(void*,int);
    friend void putDispatchChan(void*,int,int);
  
  public:
    void acquire(short* bufL, short* bufR, size_t start, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    DivMacroInt* getChanMacroInt(int ch);
    DivDispatchOscBuffer* getOscBuffer(int chan);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    void reset();
    void tick(bool sysTick=true);
    void muteChannel(int ch, bool mute);
    void notifyInsDeletion(void* ins);
    float getPostAmp();
    bool isStereo();
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    ~DivPlatformVERA();
};
#endif
