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

#ifndef _AY8930_H
#define _AY8930_H
#include "../dispatch.h"
#include "../macroInt.h"
#include <queue>
#include "sound/ay8910.h"

class DivPlatformAY8930: public DivDispatch {
  protected:
    struct Channel {
      struct Envelope {
        unsigned char mode;
        unsigned short period;
        short slideLow;
        short slide;
        Envelope():
          mode(0),
          period(0),
          slideLow(0),
          slide(0) {}
      } envelope;

      struct PSGMode {
        unsigned char tone: 1;
        unsigned char noise: 1;
        unsigned char envelope: 1;
        unsigned char dac: 1;

        unsigned char getTone() {
          return dac?0:(tone<<0);
        }

        unsigned char getNoise() {
          return dac?0:(noise<<1);
        }

        unsigned char getEnvelope() {
          return dac?0:(envelope<<2);
        }

        PSGMode& operator=(unsigned char s) {
          tone=(s>>0)&1;
          noise=(s>>1)&1;
          envelope=(s>>2)&1;
          dac=(s>>3)&1;
          return *this;
        }

        PSGMode():
          tone(1),
          noise(0),
          envelope(0),
          dac(0) {}
      } psgMode;

      struct DAC {
        int sample, rate, period, pos, out;
        unsigned char furnaceDAC: 1;

        DAC():
          sample(-1),
          rate(0),
          period(0),
          pos(0),
          out(0),
          furnaceDAC(0) {}
      } dac;

      int freq, baseFreq, note, pitch, pitch2;
      int ins;
      unsigned char autoEnvNum, autoEnvDen, duty;
      signed char konCycles;
      bool active, insChanged, freqChanged, keyOn, keyOff, portaPause, inPorta;
      int vol, outVol;
      DivMacroInt std;
      void macroInit(DivInstrument* which) {
        std.init(which);
        pitch2=0;
      }
      Channel():
        envelope(Envelope()),
        psgMode(PSGMode()),
        dac(DAC()),
        freq(0),
        baseFreq(0),
        note(0),
        pitch(0),
        pitch2(0),
        ins(-1),
        autoEnvNum(0),
        autoEnvDen(0),
        duty(4),
        active(false),
        insChanged(true),
        freqChanged(false),
        keyOn(false),
        keyOff(false),
        portaPause(false),
        inPorta(false),
        vol(0),
        outVol(31) {}
    };
    Channel chan[3];
    bool isMuted[3];
    struct QueuedWrite {
      unsigned short addr;
      unsigned char val;
      bool addrOrVal;
      QueuedWrite(unsigned short a, unsigned char v): addr(a), val(v), addrOrVal(false) {}
    };
    std::queue<QueuedWrite> writes;
    ay8930_device* ay;
    DivDispatchOscBuffer* oscBuf[3];
    unsigned char regPool[32];
    unsigned char ayNoiseAnd, ayNoiseOr;
    unsigned char stereoSep;
    bool bank;

    unsigned char sampleBank;

    int delay;

    bool extMode, stereo, clockSel;
    bool ioPortA, ioPortB;
    unsigned char portAVal, portBVal;
  
    short oldWrites[32];
    short pendingWrites[32];
    short* ayBuf[3];
    size_t ayBufLen;

    void runDAC();
    void checkWrites();
    void updateOutSel(bool immediate=false);
    void immWrite(unsigned char a, unsigned char v);
  
    friend void putDispatchChip(void*,int);
    friend void putDispatchChan(void*,int,int);
  
  public:
    void acquire(short* bufL, short* bufR, size_t start, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    DivDispatchOscBuffer* getOscBuffer(int chan);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    void reset();
    void forceIns();
    void tick(bool sysTick=true);
    void muteChannel(int ch, bool mute);
    void setFlags(unsigned int flags);
    bool isStereo();
    bool keyOffAffectsArp(int ch);
    DivMacroInt* getChanMacroInt(int ch);
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    int init(DivEngine* parent, int channels, int sugRate, unsigned int flags);
    void quit();
};
#endif
