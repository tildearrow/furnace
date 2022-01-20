#ifndef _ENGINE_H
#define _ENGINE_H
#include "song.h"
#include "dispatch.h"
#include "safeWriter.h"
#include "../audio/taAudio.h"
#include "blip_buf.h"
#include <thread>
#include <mutex>
#include <map>
#include <queue>

#define DIV_VERSION "0.4.5"
#define DIV_ENGINE_VERSION 26

enum DivStatusView {
  DIV_STATUS_NOTHING=0,
  DIV_STATUS_PATTERN,
  DIV_STATUS_COMMANDS
};

enum DivAudioEngines {
  DIV_AUDIO_JACK=0,
  DIV_AUDIO_SDL=1
};

enum DivAudioExportModes {
  DIV_EXPORT_MODE_ONE=0,
  DIV_EXPORT_MODE_MANY_SYS,
  DIV_EXPORT_MODE_MANY_CHAN
};

struct DivChannelState {
  std::vector<DivDelayedCommand> delayed;
  int note, oldNote, pitch, portaSpeed, portaNote;
  int volume, volSpeed, cut, rowDelay, volMax;
  int delayOrder, delayRow, retrigSpeed, retrigTick;
  int vibratoDepth, vibratoRate, vibratoPos, vibratoDir, vibratoFine;
  int tremoloDepth, tremoloRate, tremoloPos;
  unsigned char arp, arpStage, arpTicks;
  bool doNote, legato, portaStop, keyOn, keyOff, nowYouCanStop, stopOnOff, arpYield, delayLocked, inPorta, scheduledSlideReset;

  DivChannelState():
    note(-1),
    oldNote(-1),
    pitch(0),
    portaSpeed(-1),
    portaNote(-1),
    volume(0x7f00),
    volSpeed(0),
    cut(-1),
    rowDelay(0),
    volMax(0),
    delayOrder(0),
    delayRow(0),
    retrigSpeed(0),
    retrigTick(0),
    vibratoDepth(0),
    vibratoRate(0),
    vibratoPos(0),
    vibratoDir(0),
    vibratoFine(15),
    tremoloDepth(0),
    tremoloRate(0),
    tremoloPos(0),
    arp(0),
    arpStage(-1),
    arpTicks(1),
    doNote(false),
    legato(false),
    portaStop(false),
    keyOn(false),
    keyOff(false),
    nowYouCanStop(true),
    stopOnOff(false),
    arpYield(false),
    delayLocked(false),
    inPorta(false),
    scheduledSlideReset(false) {}
};

struct DivNoteEvent {
  int channel, ins, note, volume;
  bool on;
  DivNoteEvent(int c, int i, int n, int v, bool o):
    channel(c),
    ins(i),
    note(n),
    volume(v),
    on(o) {}
};

struct DivDispatchContainer {
  DivDispatch* dispatch;
  blip_buffer_t* bb[2];
  size_t bbInLen;
  int temp[2], prevSample[2];
  short* bbIn[2];
  short* bbOut[2];
  bool lowQuality;

  void setRates(double gotRate);
  void setQuality(bool lowQual);
  void acquire(size_t offset, size_t count);
  void fillBuf(size_t runtotal, size_t size);
  void clear();
  void init(DivSystem sys, DivEngine* eng, int chanCount, double gotRate, bool pal);
  void quit();
  DivDispatchContainer():
    dispatch(NULL),
    bb{NULL,NULL},
    bbInLen(0),
    temp{0,0},
    prevSample{0,0},
    bbIn{NULL,NULL},
    bbOut{NULL,NULL},
    lowQuality(false) {}
};

class DivEngine {
  DivDispatchContainer disCont[32];
  TAAudio* output;
  TAAudioDesc want, got;
  String exportPath;
  std::thread* exportThread;
  int chans;
  bool active;
  bool lowQuality;
  bool playing;
  bool freelance;
  bool speedAB;
  bool endOfSong;
  bool consoleMode;
  bool extValuePresent;
  bool repeatPattern;
  bool metronome;
  bool exporting;
  int ticks, curRow, curOrder, remainingLoops, nextSpeed, divider;
  int cycles, clockDrift;
  int changeOrd, changePos, totalSeconds, totalTicks, totalTicksR, totalCmds, lastCmds, cmdsPerSecond, globalPitch;
  unsigned char extValue;
  unsigned char speed1, speed2;
  DivStatusView view;
  DivChannelState chan[DIV_MAX_CHANS];
  DivAudioEngines audioEngine;
  DivAudioExportModes exportMode;
  std::map<String,String> conf;
  std::queue<DivNoteEvent> pendingNotes;
  bool isMuted[DIV_MAX_CHANS];
  DivSystem sysOfChan[DIV_MAX_CHANS];
  int dispatchOfChan[DIV_MAX_CHANS];
  int dispatchChanOfChan[DIV_MAX_CHANS];
  std::mutex isBusy;
  String configPath;
  String configFile;
  String lastError;

  struct SamplePreview {
    int sample;
    int wave;
    unsigned int pos;
    SamplePreview():
      sample(-1),
      wave(-1),
      pos(0) {}
  } sPreview;

  short vibTable[64];

  blip_buffer_t* samp_bb;
  size_t samp_bbInLen;
  int samp_temp, samp_prevSample;
  short* samp_bbIn;
  short* samp_bbOut;
  unsigned char* metroTick;
  size_t metroTickLen;
  float metroFreq, metroPos;
  float metroAmp;

  size_t totalProcessed;

  private: int* jediTable;

  int dispatchCmd(DivCommand c);
  void processRow(int i, bool afterDelay);
  void nextOrder();
  void nextRow();
  // returns true if end of song.
  bool nextTick(bool noAccum=false);
  bool perSystemEffect(int ch, unsigned char effect, unsigned char effectVal);
  bool perSystemPostEffect(int ch, unsigned char effect, unsigned char effectVal);
  void recalcChans();
  void renderSamples();
  void reset();
  void playSub(bool preserveDrift);

  bool loadDMF(unsigned char* file, size_t len);
  bool loadFur(unsigned char* file, size_t len);

  bool initAudioBackend();
  bool deinitAudioBackend();

  public:
    DivSong song;
    void runExportThread();
    void nextBuf(float** in, float** out, int inChans, int outChans, unsigned int size);
    DivInstrument* getIns(int index);
    DivWavetable* getWave(int index);
    // start fresh
    void createNew();
    // load a file.
    bool load(unsigned char* f, size_t length);
    // save as .dmf.
    SafeWriter* saveDMF();
    // save as .fur.
    SafeWriter* saveFur();
    // build a ROM file (TODO).
    // specify system to build ROM for.
    SafeWriter* buildROM(int sys);
    // dump to VGM (TODO).
    SafeWriter* saveVGM();
    // export to an audio file
    bool saveAudio(const char* path, int loops, DivAudioExportModes mode);
    // wait for audio export to finish
    void waitAudioFile();
    // stop audio file export
    bool haltAudioFile();
    // notify instrument parameter change
    void notifyInsChange(int ins);
    // notify wavetable change
    void notifyWaveChange(int wave);

    // save config
    bool saveConf();

    // load config
    bool loadConf();

    // get a config value
    bool getConfBool(String key, bool fallback);
    int getConfInt(String key, int fallback);
    float getConfFloat(String key, float fallback);
    double getConfDouble(String key, double fallback);
    String getConfString(String key, String fallback);

    // set a config value
    void setConf(String key, bool value);
    void setConf(String key, int value);
    void setConf(String key, float value);
    void setConf(String key, double value);
    void setConf(String key, String value);

    // calculate frequency/period
    int calcFreq(int base, int pitch, bool period=false);

    // play
    void play();

    // stop
    void stop();

    // reset playback state
    void syncReset();

    // trigger sample preview
    void previewSample(int sample);

    // trigger wave preview
    void previewWave(int wave, int note);
    void stopWavePreview();

    // get config path
    String getConfigPath();

    // get sys channel count
    int getChannelCount(DivSystem sys);

    // get channel count
    int getTotalChannelCount();

    // get effect description
    const char* getEffectDesc(unsigned char effect, int chan);

    // get channel type
    // - 0: FM
    // - 1: pulse
    // - 2: noise
    // - 3: wave/other
    // - 4: PCM
    // - 5: FM operator
    int getChannelType(int ch);

    // get preferred instrument type
    DivInstrumentType getPreferInsType(int ch);

    // get sys name
    const char* getSystemName(DivSystem sys);
    
    // convert sample rate format
    int fileToDivRate(int frate);
    int divToFileRate(int drate);

    // get effective sample rate
    int getEffectiveSampleRate(int rate);

    // is FM system
    bool isFMSystem(DivSystem sys);

    // is STD system
    bool isSTDSystem(DivSystem sys);

    // is channel muted
    bool isChannelMuted(int chan);

    // toggle mute
    void toggleMute(int chan);

    // toggle solo
    void toggleSolo(int chan);

    // set mute status
    void muteChannel(int chan, bool mute);

    // get channel name
    const char* getChannelName(int chan);

    // get channel short name
    const char* getChannelShortName(int chan);

    // get channel max volume
    int getMaxVolumeChan(int chan);

    // get current order
    unsigned char getOrder();

    // get current row
    int getRow();

    // get speed 1
    unsigned char getSpeed1();

    // get speed 2
    unsigned char getSpeed2();

    // get Hz
    int getHz();

    // get current Hz
    int getCurHz();

    // get time
    int getTotalTicks(); // 1/1000000th of a second
    int getTotalSeconds();

    // get repeat pattern
    bool getRepeatPattern();

    // set repeat pattern
    void setRepeatPattern(bool value);

    // has ext value
    bool hasExtValue();

    // get ext value
    unsigned char getExtValue();

    // is playing
    bool isPlaying();

    // is exporting
    bool isExporting();

    // add instrument
    int addInstrument(int refChan=0);

    // add instrument from file
    int addInstrumentFromFile(const char* path);

    // delete instrument
    void delInstrument(int index);

    // add wavetable
    int addWave();

    // add wavetable from file
    bool addWaveFromFile(const char* path);

    // delete wavetable
    void delWave(int index);

    // add sample
    int addSample();

    // add sample from file
    bool addSampleFromFile(const char* path);

    // delete sample
    void delSample(int index);

    // add order
    void addOrder(bool duplicate, bool where);

    // delete order
    void deleteOrder();

    // move order up
    void moveOrderUp();

    // move order down
    void moveOrderDown();

    // move thing up
    bool moveInsUp(int which);
    bool moveWaveUp(int which);
    bool moveSampleUp(int which);

    // move thing down
    bool moveInsDown(int which);
    bool moveWaveDown(int which);
    bool moveSampleDown(int which);

    // play note
    void noteOn(int chan, int ins, int note, int vol=-1);

    // stop note
    void noteOff(int chan);

    // go to order
    void setOrder(unsigned char order);

    // set Hz
    void setSongRate(int hz, bool pal);

    // set remaining loops. -1 means loop forever.
    void setLoops(int loops);

    // set the audio system.
    void setAudio(DivAudioEngines which);

    // set the view mode.
    void setView(DivStatusView which);

    // set the console mode.
    void setConsoleMode(bool enable);
    
    // get metronome
    bool getMetronome();

    // set metronome
    void setMetronome(bool enable);

    // public render samples
    void renderSamplesP();

    // change system
    void changeSystem(int index, DivSystem which);

    // add system
    bool addSystem(DivSystem which);

    // remove system
    bool removeSystem(int index);

    // get last error
    String getLastError();
    
    // switch master
    void switchMaster();

    // init dispatch
    void initDispatch();

    // quit dispatch
    void quitDispatch();

    // initialize the engine. optionally provide an output file name.
    bool init();

    // terminate the engine.
    bool quit();

    unsigned char* adpcmMem;

    DivEngine():
      output(NULL),
      exportThread(NULL),
      chans(0),
      active(false),
      lowQuality(false),
      playing(false),
      freelance(false),
      speedAB(false),
      endOfSong(false),
      consoleMode(false),
      extValuePresent(false),
      repeatPattern(false),
      metronome(false),
      exporting(false),
      ticks(0),
      curRow(0),
      curOrder(0),
      remainingLoops(-1),
      nextSpeed(3),
      divider(60),
      cycles(0),
      clockDrift(0),
      changeOrd(-1),
      changePos(0),
      totalSeconds(0),
      totalTicks(0),
      totalTicksR(0),
      totalCmds(0),
      lastCmds(0),
      cmdsPerSecond(0),
      extValue(0),
      speed1(3),
      speed2(3),
      view(DIV_STATUS_NOTHING),
      audioEngine(DIV_AUDIO_SDL),
      samp_bbInLen(0),
      samp_temp(0),
      samp_prevSample(0),
      metroTick(NULL),
      metroTickLen(0),
      metroFreq(0),
      metroPos(0),
      metroAmp(0.0f),
      totalProcessed(0),
      jediTable(NULL),
      adpcmMem(NULL) {}
};
#endif
