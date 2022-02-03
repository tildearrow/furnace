#ifndef _SMS_H
#define _SMS_H

#include "../dispatch.h"
#include "../macroInt.h"
#include "sound/sn76496.h"

class DivPlatformSMS: public DivDispatch {
  struct Channel {
    int freq, baseFreq, pitch, note, actualNote;
    unsigned char ins;
    bool active, insChanged, freqChanged, keyOn, keyOff;
    signed char vol, outVol;
    DivMacroInt std;
    Channel():
      freq(0),
      baseFreq(0),
      pitch(0),
      note(0),
      actualNote(0),
      ins(-1),
      active(false),
      insChanged(true),
      freqChanged(false),
      keyOn(false),
      keyOff(false),
      vol(15),
      outVol(15) {}
  };
  Channel chan[4];
  bool isMuted[4];
  unsigned char snNoiseMode;
  bool updateSNMode;
  bool isRealSN;
  sn76496_base_device* sn;
  friend void putDispatchChan(void*,int,int);
  public:
    int acquireOne();
    void acquire(short* bufL, short* bufR, size_t start, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    void reset();
    void forceIns();
    void tick();
    void muteChannel(int ch, bool mute);
    bool keyOffAffectsArp(int ch);
    bool keyOffAffectsPorta(int ch);
    int getPortaFloor(int ch);
    void setFlags(unsigned int flags);
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    int init(DivEngine* parent, int channels, int sugRate, unsigned int flags);
    void quit();
    ~DivPlatformSMS();
};

#endif
