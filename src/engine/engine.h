#ifndef _ENGINE_H
#define _ENGINE_H
#include "song.h"
#include "dispatch.h"
#include "safeWriter.h"
#include "../audio/taAudio.h"
#include "blip_buf.h"
#include <mutex>

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
  int note, pitch, portaSpeed, portaNote;
  int volume, volSpeed, cut, rowDelay, volMax;
  int delayOrder, delayRow;
  int vibratoDepth, vibratoRate, vibratoPos, vibratoDir, vibratoFine;
  int tremoloDepth, tremoloRate, tremoloPos;
  unsigned char arp, arpStage, arpTicks;
  bool doNote, legato, portaStop, keyOn, nowYouCanStop, stopOnOff, arpYield, delayLocked;

  DivChannelState():
    note(-1),
    pitch(0),
    portaSpeed(-1),
    portaNote(-1),
    volume(0x7f00),
    volSpeed(0),
    cut(-1),
    rowDelay(0),
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
    delayLocked(false) {}
};

class DivEngine {
  DivDispatch* dispatch;
  TAAudio* output;
  TAAudioDesc want, got;
  int chans;
  bool playing;
  bool speedAB;
  bool endOfSong;
  int ticks, cycles, curRow, curOrder, remainingLoops, nextSpeed, clockDrift;
  int changeOrd, changePos, totalTicks, totalCmds, lastCmds, cmdsPerSecond;
  DivStatusView view;
  DivChannelState chan[17];
  DivAudioEngines audioEngine;
  std::mutex isBusy;

  short vibTable[64];

  blip_buffer_t* bb[2];
  size_t bbInLen;
  int temp[2], prevSample[2];
  short* bbIn[2];
  short* bbOut[2];

  size_t totalProcessed;

  private: int* jediTable;

  int dispatchCmd(DivCommand c);
  void processRow(int i, bool afterDelay);
  void nextOrder();
  void nextRow();
  // returns true if end of song.
  bool nextTick();
  bool perSystemEffect(int ch, unsigned char effect, unsigned char effectVal);
  bool perSystemPostEffect(int ch, unsigned char effect, unsigned char effectVal);
  void renderSamples();

  public:
    DivSong song;
    void nextBuf(float** in, float** out, int inChans, int outChans, unsigned int size);
    DivInstrument* getIns(int index);
    DivWavetable* getWave(int index);
    // load a .dmf.
    bool load(void* f, size_t length);
    // save as .dmf.
    SafeWriter* save();

    // play
    void play();

    // stop
    void stop();

    // reset playback state
    void reset();

    // get sys channel count
    int getChannelCount(DivSystem sys);

    // is FM system
    bool isFMSystem(DivSystem sys);

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

    // is playing
    bool isPlaying();

    // go to order
    void setOrder(unsigned char order);

    // set remaining loops. -1 means loop forever.
    void setLoops(int loops);

    // set the audio system.
    void setAudio(DivAudioEngines which);

    // set the view mode.
    void setView(DivStatusView which);

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
      playing(false),
      speedAB(false),
      endOfSong(false),
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
