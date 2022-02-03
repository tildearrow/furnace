#ifndef _GENESIS_H
#define _GENESIS_H
#include "../dispatch.h"
#include <queue>
#include "../../../extern/Nuked-OPN2/ym3438.h"
#include "sound/ymfm/ymfm_opn.h"

#include "sms.h"

class DivYM2612Interface: public ymfm::ymfm_interface {

};

class DivPlatformGenesis: public DivDispatch {
  protected:
    struct Channel {
      DivInstrumentFM state;
      DivMacroInt std;
      unsigned char freqH, freqL;
      int freq, baseFreq, pitch, note;
      unsigned char ins;
      bool active, insChanged, freqChanged, keyOn, keyOff, portaPause, furnaceDac, inPorta;
      int vol, outVol;
      unsigned char pan;
      Channel():
        freqH(0),
        freqL(0),
        freq(0),
        baseFreq(0),
        pitch(0),
        note(0),
        ins(-1),
        active(false),
        insChanged(true),
        freqChanged(false),
        keyOn(false),
        keyOff(false),
        portaPause(false),
        furnaceDac(false),
        inPorta(false),
        vol(0),
        pan(3) {}
    };
    Channel chan[10];
    bool isMuted[10];
    struct QueuedWrite {
      unsigned short addr;
      unsigned char val;
      bool addrOrVal;
      QueuedWrite(unsigned short a, unsigned char v): addr(a), val(v), addrOrVal(false) {}
    };
    std::queue<QueuedWrite> writes;
    ym3438_t fm;
    DivPlatformSMS psg;
    int psgClocks;
    int psgOut;
    int delay;
    unsigned char lastBusy;

    ymfm::ym2612* fm_ymfm;
    ymfm::ym2612::output_data out_ymfm;
    DivYM2612Interface iface;
  
    bool dacMode;
    int dacPeriod;
    int dacRate;
    unsigned int dacPos;
    int dacSample;
    unsigned char sampleBank;
    unsigned char lfoValue;

    bool extMode, useYMFM;
    bool ladder;
  
    short oldWrites[512];
    short pendingWrites[512];

    int octave(int freq);
    int toFreq(int freq);

    friend void putDispatchChan(void*,int,int);

    void acquire_nuked(short* bufL, short* bufR, size_t start, size_t len);
    void acquire_ymfm(short* bufL, short* bufR, size_t start, size_t len);
  
  public:
    void acquire(short* bufL, short* bufR, size_t start, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    void reset();
    void forceIns();
    void tick();
    void muteChannel(int ch, bool mute);
    bool isStereo();
    void setYMFM(bool use);
    bool keyOffAffectsArp(int ch);
    bool keyOffAffectsPorta(int ch);
    void toggleRegisterDump(bool enable);
    void setFlags(unsigned int flags);
    void notifyInsChange(int ins);
    void notifyInsDeletion(void* ins);
    int getPortaFloor(int ch);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    int init(DivEngine* parent, int channels, int sugRate, unsigned int flags);
    void quit();
    ~DivPlatformGenesis();
};
#endif
