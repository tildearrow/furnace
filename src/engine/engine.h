#ifndef _ENGINE_H
#define _ENGINE_H
#include "song.h"
#include "dispatch.h"
#include "../audio/taAudio.h"
#include "blip_buf.h"

enum DivStatusView {
  DIV_STATUS_NOTHING=0,
  DIV_STATUS_PATTERN,
  DIV_STATUS_COMMANDS
};

struct DivChannelState {
  std::vector<DivDelayedCommand> delayed;
  int note, pitch, portaSpeed, portaNote;
  int volume, volSpeed, cut, rowDelay, volMax;
  int vibratoDepth, vibratoRate, vibratoPos, vibratoDir, vibratoFine;
  int tremoloDepth, tremoloRate, tremoloPos;
  unsigned char arp, arpStage, arpTicks;
  bool doNote, legato, portaStop, keyOn, nowYouCanStop, stopOnOff, arpYield;

  DivChannelState():
    note(-1),
    pitch(0),
    portaSpeed(-1),
    portaNote(-1),
    volume(0x7f00),
    volSpeed(0),
    cut(-1),
    rowDelay(0),
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
    doNote(false), legato(false), portaStop(false), keyOn(false), nowYouCanStop(true), stopOnOff(false), arpYield(false) {}
};

class DivEngine {
  DivDispatch* dispatch;
  TAAudio* output;
  TAAudioDesc want, got;
  int chans;
  bool playing;
  bool speedAB;
  int ticks, cycles, curRow, curOrder;
  int changeOrd, changePos, totalTicks, totalCmds, lastCmds, cmdsPerSecond;
  DivStatusView view;
  DivChannelState chan[17];

  short vibTable[64];

  blip_buffer_t* bb[2];
  int temp[2], prevSample[2];
  short* bbOut[2];

  int dispatchCmd(DivCommand c);
  void processRow(int i, bool afterDelay);
  void nextOrder();
  void nextRow();
  void nextTick();
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
    bool save(FILE* f);

    // play
    void play();

    // initialize the engine.
    bool init();

    DivEngine():
      chans(0),
      playing(false),
      speedAB(false),
      ticks(0),
      cycles(0),
      curRow(-1),
      curOrder(0),
      changeOrd(-1),
      changePos(0),
      totalTicks(0),
      totalCmds(0),
      lastCmds(0),
      cmdsPerSecond(0),
      view(DIV_STATUS_PATTERN),
      temp{0,0},
      prevSample{0,0} {}
};
#endif
