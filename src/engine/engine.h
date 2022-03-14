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

#ifndef _ENGINE_H
#define _ENGINE_H
#include "song.h"
#include "dispatch.h"
#include "dataErrors.h"
#include "safeWriter.h"
#include "../audio/taAudio.h"
#include "blip_buf.h"
#include <thread>
#include <mutex>
#include <map>
#include <queue>

#define addWarning(x) \
  if (warnings.empty()) { \
    warnings+=x; \
  } else { \
    warnings+=(String("\n")+x); \
  }

#define DIV_VERSION "dev66"
#define DIV_ENGINE_VERSION 66

enum DivStatusView {
  DIV_STATUS_NOTHING=0,
  DIV_STATUS_PATTERN,
  DIV_STATUS_COMMANDS
};

enum DivAudioEngines {
  DIV_AUDIO_JACK=0,
  DIV_AUDIO_SDL=1,
  DIV_AUDIO_NULL=2,
  DIV_AUDIO_DUMMY=3
};

enum DivAudioExportModes {
  DIV_EXPORT_MODE_ONE=0,
  DIV_EXPORT_MODE_MANY_SYS,
  DIV_EXPORT_MODE_MANY_CHAN
};

enum DivHaltPositions {
  DIV_HALT_NONE=0,
  DIV_HALT_TICK,
  DIV_HALT_ROW,
  DIV_HALT_PATTERN,
  DIV_HALT_BREAKPOINT
};

struct DivChannelState {
  std::vector<DivDelayedCommand> delayed;
  int note, oldNote, lastIns, pitch, portaSpeed, portaNote;
  int volume, volSpeed, cut, rowDelay, volMax;
  int delayOrder, delayRow, retrigSpeed, retrigTick;
  int vibratoDepth, vibratoRate, vibratoPos, vibratoDir, vibratoFine;
  int tremoloDepth, tremoloRate, tremoloPos;
  unsigned char arp, arpStage, arpTicks;
  bool doNote, legato, portaStop, keyOn, keyOff, nowYouCanStop, stopOnOff, arpYield, delayLocked, inPorta, scheduledSlideReset, shorthandPorta, noteOnInhibit;

  DivChannelState():
    note(-1),
    oldNote(-1),
    lastIns(-1),
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
    scheduledSlideReset(false),
    shorthandPorta(false),
    noteOnInhibit(false) {}
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
  void flush(size_t count);
  void fillBuf(size_t runtotal, size_t offset, size_t size);
  void clear();
  void init(DivSystem sys, DivEngine* eng, int chanCount, double gotRate, unsigned int flags);
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
  bool halted;
  bool forceMono;
  bool cmdStreamEnabled;
  int ticks, curRow, curOrder, remainingLoops, nextSpeed, divider;
  int cycles, clockDrift, stepPlay;
  int changeOrd, changePos, totalSeconds, totalTicks, totalTicksR, totalCmds, lastCmds, cmdsPerSecond, globalPitch;
  unsigned char extValue;
  unsigned char speed1, speed2;
  DivStatusView view;
  DivHaltPositions haltOn;
  DivChannelState chan[DIV_MAX_CHANS];
  DivAudioEngines audioEngine;
  DivAudioExportModes exportMode;
  std::map<String,String> conf;
  std::queue<DivNoteEvent> pendingNotes;
  bool isMuted[DIV_MAX_CHANS];
  std::mutex isBusy;
  String configPath;
  String configFile;
  String lastError;
  String warnings;
  std::vector<String> audioDevs;
  std::vector<DivCommand> cmdStream;

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

  DivSystem systemFromFile(unsigned char val);
  unsigned char systemToFile(DivSystem val);
  int dispatchCmd(DivCommand c);
  void processRow(int i, bool afterDelay);
  void nextOrder();
  void nextRow();
  void performVGMWrite(SafeWriter* w, DivSystem sys, DivRegWrite& write, int streamOff, double* loopTimer, double* loopFreq, int* loopSample, bool isSecond);
  // returns true if end of song.
  bool nextTick(bool noAccum=false);
  bool perSystemEffect(int ch, unsigned char effect, unsigned char effectVal);
  bool perSystemPostEffect(int ch, unsigned char effect, unsigned char effectVal);
  void recalcChans();
  void renderSamples();
  void reset();
  void playSub(bool preserveDrift, int goalRow=0);

  bool loadDMF(unsigned char* file, size_t len);
  bool loadFur(unsigned char* file, size_t len);
  bool loadMod(unsigned char* file, size_t len);

  bool initAudioBackend();
  bool deinitAudioBackend();

  void exchangeIns(int one, int two);

  public:
    DivSong song;
    DivSystem sysOfChan[DIV_MAX_CHANS];
    int dispatchOfChan[DIV_MAX_CHANS];
    int dispatchChanOfChan[DIV_MAX_CHANS];
    bool keyHit[DIV_MAX_CHANS];
    float* oscBuf[2];
    float oscSize;

    void runExportThread();
    void nextBuf(float** in, float** out, int inChans, int outChans, unsigned int size);
    DivInstrument* getIns(int index);
    DivWavetable* getWave(int index);
    DivSample* getSample(int index);
    // start fresh
    void createNew(const int* description);
    // load a file.
    bool load(unsigned char* f, size_t length);
    // save as .dmf.
    SafeWriter* saveDMF(unsigned char version);
    // save as .fur.
    SafeWriter* saveFur();
    // build a ROM file (TODO).
    // specify system to build ROM for.
    SafeWriter* buildROM(int sys);
    // dump to VGM.
    SafeWriter* saveVGM(bool* sysToExport=NULL, bool loop=true);
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

    // returns whether a system is VGM compatible
    bool isVGMExportable(DivSystem which);

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

    // calculate base frequency/period
    int calcBaseFreq(double clock, double divider, int note, bool period);

    // calculate frequency/period
    int calcFreq(int base, int pitch, bool period=false, int octave=0);

    // find song loop position
    void walkSong(int& loopOrder, int& loopRow, int& loopEnd);

    // play
    void play();

    // play to row
    void playToRow(int row);

    // play by one row
    void stepOne(int row);

    // stop
    void stop();

    // reset playback state
    void syncReset();

    // trigger sample preview
    void previewSample(int sample, int note=-1);
    void stopSamplePreview();

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

    // get song system name
    const char* getSongSystemName();

    // get sys name
    const char* getSystemName(DivSystem sys);

    // get sys chips
    const char* getSystemChips(DivSystem sys);

    // get japanese system name
    const char* getSystemNameJ(DivSystem sys);
    
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

    // unmute all
    void unmuteAll();

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

    // is stepping
    bool isStepping();

    // is exporting
    bool isExporting();

    // add instrument
    int addInstrument(int refChan=0);

    // add instrument from file
    bool addInstrumentFromFile(const char* path);

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

    // deep clone orders
    void deepCloneOrder(bool where);

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

    // set system flags
    void setSysFlags(int system, unsigned int flags, bool restart);

    // set Hz
    void setSongRate(int hz, bool pal);

    // set remaining loops. -1 means loop forever.
    void setLoops(int loops);

    // get channel state
    DivChannelState* getChanState(int chan);

    // get dispatch channel state
    void* getDispatchChanState(int chan);
    
    // get register pool
    unsigned char* getRegisterPool(int sys, int& size, int& depth);

    // enable command stream dumping
    void enableCommandStream(bool enable);

    // get command stream
    void getCommandStream(std::vector<DivCommand>& where);

    // set the audio system.
    void setAudio(DivAudioEngines which);

    // set the view mode.
    void setView(DivStatusView which);

    // get available audio devices
    std::vector<String>& getAudioDevices();

    // rescan audio devices
    void rescanAudioDevices();

    // set the console mode.
    void setConsoleMode(bool enable);
    
    // get metronome
    bool getMetronome();

    // set metronome
    void setMetronome(bool enable);

    // halt now
    void halt();

    // resume from halt
    void resume();

    // halt on next something
    void haltWhen(DivHaltPositions when);

    // is engine halted
    bool isHalted();

    // get register cheatsheet
    const char** getRegisterSheet(int sys);

    // public render samples
    void renderSamplesP();

    // change system
    void changeSystem(int index, DivSystem which);

    // add system
    bool addSystem(DivSystem which);

    // remove system
    bool removeSystem(int index);
    
    // write to register on system
    void poke(int sys, unsigned int addr, unsigned short val);

    // write to register on system
    void poke(int sys, std::vector<DivRegWrite>& wlist);

    // get last error
    String getLastError();

    // get warnings
    String getWarnings();
    
    // switch master
    bool switchMaster();

    // get audio desc want
    TAAudioDesc& getAudioDescWant();

    // get audio desc
    TAAudioDesc& getAudioDescGot();

    // init dispatch
    void initDispatch();

    // quit dispatch
    void quitDispatch();

    // initialize the engine. optionally provide an output file name.
    bool init();

    // terminate the engine.
    bool quit();

    unsigned char* adpcmAMem;
    size_t adpcmAMemLen;
    unsigned char* adpcmBMem;
    size_t adpcmBMemLen;
    unsigned char* qsoundMem;
    size_t qsoundMemLen;
    unsigned char* qsoundAMem;
    size_t qsoundAMemLen;
    unsigned char* dpcmMem;
    size_t dpcmMemLen;
    unsigned char* x1_010Mem;
    size_t x1_010MemLen;

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
      halted(false),
      forceMono(false),
      cmdStreamEnabled(false),
      ticks(0),
      curRow(0),
      curOrder(0),
      remainingLoops(-1),
      nextSpeed(3),
      divider(60),
      cycles(0),
      clockDrift(0),
      stepPlay(0),
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
      haltOn(DIV_HALT_NONE),
      audioEngine(DIV_AUDIO_NULL),
      samp_bbInLen(0),
      samp_temp(0),
      samp_prevSample(0),
      metroTick(NULL),
      metroTickLen(0),
      metroFreq(0),
      metroPos(0),
      metroAmp(0.0f),
      totalProcessed(0),
      oscBuf{NULL,NULL},
      oscSize(1),
      adpcmAMem(NULL),
      adpcmAMemLen(0),
      adpcmBMem(NULL),
      adpcmBMemLen(0),
      qsoundMem(NULL),
      qsoundMemLen(0),
      qsoundAMem(NULL),
      qsoundAMemLen(0),
      dpcmMem(NULL),
      dpcmMemLen(0) {}
};
#endif
