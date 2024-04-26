#include "sound/s3hs88pwn4/sound.cpp"
#include "../waveSynth.h"
#include "../dispatch.h"

class DivPlatformS3HS: public DivDispatch {
  struct Channel : public SharedChannel<int> {
    int freq, baseFreq, pitch;
    int wave, sample;
    unsigned short pos;
    bool active, freqChanged;
    bool volumeChanged;
    bool waveChanged, waveUpdated;
    bool pcm;
    unsigned char vol;
    unsigned char outVol, resVol;
    signed char amp;
    bool pcmLoop;
    int hasOffset;
    int pan;
    DivWaveSynth ws;
    Channel(): 
      SharedChannel<int>(0),
      freq(0), 
      baseFreq(0), 
      pitch(0), 
      wave(-1),
      sample(0),
      pos(0), 
      active(false), 
      freqChanged(false),  
      volumeChanged(false),
      waveChanged(false),
      waveUpdated(false),
      pcm(false),
      vol(0),
      outVol(0),
      resVol(0),
      amp(0),
      pcmLoop(false),
      hasOffset(0),
      pan(0){}
  };
  S3HS_sound* cpt;
  unsigned char regPool[704];
  DivMemoryComposition memCompo;
  Channel chan[12];
  DivDispatchOscBuffer* oscBuf[12];
  bool isMuted[12];
  unsigned char chans;  
  unsigned int sampleMemSize;
  unsigned char ilCtrl, ilSize, fil1;
  unsigned char initIlCtrl, initIlSize, initFil1;
  bool sampleLoaded[256];
  unsigned char* sampleMem;
  size_t sampleMemLen;
  unsigned int sampleOffSU[256];
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
    void updateWave(int ch);
    int getRegisterPoolSize();
    void acquire(short** buf, size_t len);
    void muteChannel(int ch, bool mute);
    int dispatch(DivCommand c);
    DivMacroInt* getChanMacroInt(int ch);
    void* getChanState(int chan);
    DivDispatchOscBuffer* getOscBuffer(int chan);
    void notifyWaveChange(int wave);
    void notifyInsDeletion(void* ins);
    int getOutputCount();
    unsigned short getPan(int ch);
    void reset();
    void tick(bool sysTick=true);
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const void* getSampleMem(int index);
    size_t getSampleMemCapacity(int index);
    size_t getSampleMemUsage(int index);
    const DivMemoryComposition* getMemCompo(int index);
    bool isSampleLoaded(int index, int sample);
    void renderSamples(int chipID);
    void doWrite(unsigned int addr, unsigned char data);
    ~DivPlatformS3HS();
};
