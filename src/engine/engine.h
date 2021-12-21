#ifndef _ENGINE_H
#define _ENGINE_H
#include "song.h"
#include "dispatch.h"
#include "safeWriter.h"
#include "../audio/taAudio.h"
#include "blip_buf.h"
#include <mutex>
#include <map>

#define DIV_VERSION "0.1"
#define DIV_ENGINE_VERSION 11

enum DivStatusView {
  DIV_STATUS_NOTHING=0,
  DIV_STATUS_PATTERN,
  DIV_STATUS_COMMANDS
};

enum DivAudioEngines {
  DIV_AUDIO_JACK=0,
  DIV_AUDIO_SDL
};

struct DivChannelState {
  std::vector<DivDelayedCommand> delayed;
  int note, oldNote, pitch, portaSpeed, portaNote;
  int volume, volSpeed, cut, rowDelay, volMax;
  int delayOrder, delayRow;
  int vibratoDepth, vibratoRate, vibratoPos, vibratoDir, vibratoFine;
  int tremoloDepth, tremoloRate, tremoloPos;
  unsigned char arp, arpStage, arpTicks;
  bool doNote, legato, portaStop, keyOn, nowYouCanStop, stopOnOff, arpYield, delayLocked, inPorta;

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
    nowYouCanStop(true),
    stopOnOff(false),
    arpYield(false),
    delayLocked(false),
    inPorta(false) {}
};

class DivEngine {
  DivDispatch* dispatch;
  TAAudio* output;
  TAAudioDesc want, got;
  int chans;
  bool active;
  bool playing;
  bool speedAB;
  bool endOfSong;
  bool consoleMode;
  bool extValuePresent;
  bool repeatPattern;
  int ticks, cycles, curRow, curOrder, remainingLoops, nextSpeed, clockDrift;
  int changeOrd, changePos, totalTicks, totalCmds, lastCmds, cmdsPerSecond;
  unsigned char extValue;
  unsigned char speed1, speed2;
  DivStatusView view;
  DivChannelState chan[17];
  DivAudioEngines audioEngine;
  std::map<String,String> conf;
  bool isMuted[17];
  std::mutex isBusy;
  String configPath;
  String configFile;
  String lastError;

  struct SamplePreview {
    int sample;
    unsigned int pos;
    SamplePreview():
      sample(-1),
      pos(0) {}
  } sPreview;

  short vibTable[64];

  blip_buffer_t* bb[3];
  size_t bbInLen;
  int temp[3], prevSample[3];
  short* bbIn[3];
  short* bbOut[3];

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
  void renderSamples();
  void reset();
  void playSub(bool preserveDrift);

  public:
    DivSong song;
    void nextBuf(float** in, float** out, int inChans, int outChans, unsigned int size);
    DivInstrument* getIns(int index);
    DivWavetable* getWave(int index);
    // load a .dmf.
    bool load(unsigned char* f, size_t length);
    // save as .dmf.
    SafeWriter* save();

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

    // play
    void play();

    // stop
    void stop();

    // reset playback state
    void syncReset();

    // trigger sample preview
    void previewSample(int sample);

    // get config path
    String getConfigPath();

    // get sys channel count
    int getChannelCount(DivSystem sys);

    // get sys name
    const char* getSystemName(DivSystem sys);

    // is FM system
    bool isFMSystem(DivSystem sys);

    // is STD system
    bool isSTDSystem(DivSystem sys);

    // get wave resolution
    int getWaveRes(DivSystem sys);

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

    // get max STD volume
    int getMaxVolume();

    // get max STD duty
    int getMaxDuty();

    // get max STD wave
    int getMaxWave();

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

    // get time
    int getTotalTicks();

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

    // add instrument
    int addInstrument();

    // delete instrument
    void delInstrument(int index);

    // add wavetable
    int addWave();

    // delete wavetable
    void delWave(int index);

    // add sample
    int addSample();

    // add sample from file
    bool addSampleFromFile(const char* path);

    // delete sample
    void delSample(int index);

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

    // public render samples
    void renderSamplesP();

    // change system
    void changeSystem(DivSystem which);

    // get last error
    String getLastError();

    // init dispatch
    void initDispatch();

    // quit dispatch
    void quitDispatch();

    // initialize the engine. optionally provide an output file name.
    bool init(String outName="");

    // terminate the engine.
    bool quit();

    unsigned char* adpcmMem;

    DivEngine():
      chans(0),
      active(false),
      playing(false),
      speedAB(false),
      endOfSong(false),
      consoleMode(false),
      extValuePresent(false),
      repeatPattern(false),
      ticks(0),
      cycles(0),
      curRow(0),
      curOrder(0),
      remainingLoops(-1),
      nextSpeed(3),
      clockDrift(0),
      changeOrd(-1),
      changePos(0),
      totalTicks(0),
      totalCmds(0),
      lastCmds(0),
      cmdsPerSecond(0),
      extValue(0),
      speed1(3),
      speed2(3),
      view(DIV_STATUS_NOTHING),
      audioEngine(DIV_AUDIO_SDL),
      bbInLen(0),
      temp{0,0},
      prevSample{0,0},
      totalProcessed(0),
      jediTable(NULL),
      adpcmMem(NULL) {}
};
#endif
