#ifndef _LYNX_H
#define _LYNX_H

#include "../dispatch.h"
#include "../macroInt.h"
#include "sound/lynx/Mikey.hpp"

class DivPlatformLynx: public DivDispatch {

  struct MikeyFreqDiv {
    uint8_t clockDivider;
    uint8_t backup;

    MikeyFreqDiv(int frequency);
  };

  struct MikeyDuty {
    uint8_t int_feedback7;
    uint8_t feedback;

    MikeyDuty(int duty);
  };

  struct Channel {
    DivMacroInt std;
    MikeyFreqDiv fd;
    MikeyDuty duty;
    int baseFreq, pitch, note, actualNote, lfsr;
    unsigned char ins;
    bool active, insChanged, freqChanged, keyOn, keyOff, inPorta;
    signed char vol, outVol;
    Channel():
      std(),
      fd(0),
      duty(0),
      baseFreq(0),
      pitch(0),
      note(0),
      actualNote(0),
      lfsr(-1),
      ins(-1),
      active(false),
      insChanged(true),
      freqChanged(false),
      keyOn(false),
      keyOff(false),
      inPorta(false),
      vol(127),
      outVol(127) {}
  };
  Channel chan[4];
  bool isMuted[4];
  std::unique_ptr<Lynx::Mikey> mikey;
  friend void putDispatchChan(void*,int,int);
  public:
    void acquire(short* bufL, short* bufR, size_t start, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    void reset();
    void forceIns();
    void tick();
    void muteChannel(int ch, bool mute);
    bool keyOffAffectsArp(int ch);
    bool keyOffAffectsPorta(int ch);
    //int getPortaFloor(int ch);
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    const char* getEffectName( unsigned char effect );
    int init(DivEngine* parent, int channels, int sugRate, unsigned int flags);
    void quit();
    ~DivPlatformLynx();
};

#endif
