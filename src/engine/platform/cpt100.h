#include "../waveSynth.h"
#include "../dispatch.h"

class DivPlatformCPT100: public DivDispatch {
  struct Channel : public SharedChannel<int> {
    int freq, baseFreq, pitch;
    int wave;
    unsigned short pos;
    bool active, freqChanged;
    unsigned char vol;
    signed char amp;
    signed char modTable[2][32];
    DivWaveSynth ws;
    Channel(): 
      SharedChannel<int>(0),
      freq(0), 
      baseFreq(0), 
      pitch(0), 
      pos(0), 
      active(false), 
      freqChanged(false), 
      vol(0), 
      amp(255),
      wave(-1)
      {
        memset(modTable,0,64);
      }
  };
  unsigned char regPool[208];
  Channel chan[6];
  DivDispatchOscBuffer* oscBuf[6];
  bool isMuted[6];
  unsigned char chans;  
  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);
  public:
    struct QueuedWrite {
    unsigned char addr;
    unsigned char val;
    QueuedWrite(): addr(0), val(0) {}
    QueuedWrite(unsigned char a, unsigned char v): addr(a), val(v) {}
   };
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    void acquire(short** buf, size_t len);
    void muteChannel(int ch, bool mute);
    int dispatch(DivCommand c);
    void notifyInsDeletion(void* ins);
    void* getChanState(int chan);
    DivDispatchOscBuffer* getOscBuffer(int chan);
    void reset();
    void tick(bool sysTick=true);
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    void doWrite(unsigned int addr, unsigned char data);
    ~DivPlatformCPT100();
};
