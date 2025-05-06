/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2025 tildearrow and contributors
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
#include "config.h"
#include "instrument.h"
#include "song.h"
#include "dispatch.h"
#include "effect.h"
#include "export.h"
#include "dataErrors.h"
#include "safeWriter.h"
#include "cmdStream.h"
#include "../audio/taAudio.h"
#include "blip_buf.h"
#include <functional>
#include <initializer_list>
#include <thread>
#include "../fixedQueue.h"

class DivWorkPool;

#define addWarning(x) \
  if (warnings.empty()) { \
    warnings+=x; \
  } else { \
    warnings+=(String("\n")+x); \
  }

#define BUSY_BEGIN softLocked=false; isBusy.lock();
#define BUSY_BEGIN_SOFT softLocked=true; isBusy.lock();
#define BUSY_END isBusy.unlock(); softLocked=false;

#define EXTERN_BUSY_BEGIN e->softLocked=false; e->isBusy.lock();
#define EXTERN_BUSY_BEGIN_SOFT e->softLocked=true; e->isBusy.lock();
#define EXTERN_BUSY_END e->isBusy.unlock(); e->softLocked=false;

#define DIV_UNSTABLE

#define DIV_VERSION "dev229"
#define DIV_ENGINE_VERSION 229
// for imports
#define DIV_VERSION_MOD 0xff01
#define DIV_VERSION_FC 0xff02
#define DIV_VERSION_S3M 0xff03
#define DIV_VERSION_FTM 0xff04
#define DIV_VERSION_TFE 0xff05
#define DIV_VERSION_XM 0xff06
#define DIV_VERSION_IT 0xff07

enum DivStatusView {
  DIV_STATUS_NOTHING=0,
  DIV_STATUS_PATTERN,
  DIV_STATUS_COMMANDS
};

enum DivAudioEngines {
  DIV_AUDIO_JACK=0,
  DIV_AUDIO_SDL=1,
  DIV_AUDIO_PORTAUDIO=2,
  DIV_AUDIO_PIPE=3,

  DIV_AUDIO_NULL=126,
  DIV_AUDIO_DUMMY=127
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

enum DivMIDIModes {
  DIV_MIDI_MODE_OFF=0,
  DIV_MIDI_MODE_NOTE,
  DIV_MIDI_MODE_LIGHT_SHOW
};

enum DivAudioExportFormats {
  DIV_EXPORT_FORMAT_S16=0,
  DIV_EXPORT_FORMAT_F32
};

struct DivAudioExportOptions {
  DivAudioExportModes mode;
  DivAudioExportFormats format;
  int sampleRate;
  int chans;
  int loops;
  double fadeOut;
  int orderBegin, orderEnd;
  bool channelMask[DIV_MAX_CHANS];
  DivAudioExportOptions():
    mode(DIV_EXPORT_MODE_ONE),
    format(DIV_EXPORT_FORMAT_S16),
    sampleRate(44100),
    chans(2),
    loops(0),
    fadeOut(0.0),
    orderBegin(-1),
    orderEnd(-1) {
    for (int i=0; i<DIV_MAX_CHANS; i++) {
      channelMask[i]=true;
    }
  }
};

struct DivChannelState {
  std::vector<DivDelayedCommand> delayed;
  int note, oldNote, lastIns, pitch, portaSpeed, portaNote;
  int volume, volSpeed, volSpeedTarget, cut, volCut, legatoDelay, legatoTarget, rowDelay, volMax;
  int delayOrder, delayRow, retrigSpeed, retrigTick;
  int vibratoDepth, vibratoRate, vibratoPos, vibratoPosGiant, vibratoShape, vibratoFine;
  int tremoloDepth, tremoloRate, tremoloPos;
  int panDepth, panRate, panPos, panSpeed;
  int sampleOff;
  unsigned char arp, arpStage, arpTicks, panL, panR, panRL, panRR, lastVibrato, lastPorta, cutType;
  bool doNote, legato, portaStop, keyOn, keyOff, nowYouCanStop, stopOnOff, releasing;
  bool arpYield, delayLocked, inPorta, scheduledSlideReset, shorthandPorta, wasShorthandPorta, noteOnInhibit, resetArp, sampleOffSet;
  bool wentThroughNote, goneThroughNote;

  int midiNote, curMidiNote, midiPitch;
  size_t midiAge;
  bool midiAftertouch;

  DivChannelState():
    note(-1),
    oldNote(-1),
    lastIns(-1),
    pitch(0),
    portaSpeed(-1),
    portaNote(-1),
    volume(0x7f00),
    volSpeed(0),
    volSpeedTarget(-1),
    cut(-1),
    volCut(-1),
    legatoDelay(-1),
    legatoTarget(0),
    rowDelay(0),
    volMax(0),
    delayOrder(0),
    delayRow(0),
    retrigSpeed(0),
    retrigTick(0),
    vibratoDepth(0),
    vibratoRate(0),
    vibratoPos(0),
    vibratoPosGiant(0),
    vibratoShape(0),
    vibratoFine(15),
    tremoloDepth(0),
    tremoloRate(0),
    tremoloPos(0),
    panDepth(0),
    panRate(0),
    panPos(0),
    panSpeed(0),
    sampleOff(0),
    arp(0),
    arpStage(-1),
    arpTicks(1),
    panL(255),
    panR(255),
    panRL(0),
    panRR(0),
    lastVibrato(0),
    lastPorta(0),
    cutType(0),
    doNote(false),
    legato(false),
    portaStop(false),
    keyOn(false),
    keyOff(false),
    nowYouCanStop(true),
    stopOnOff(false),
    releasing(false),
    arpYield(false),
    delayLocked(false),
    inPorta(false),
    scheduledSlideReset(false),
    shorthandPorta(false),
    wasShorthandPorta(false),
    noteOnInhibit(false),
    resetArp(false),
    sampleOffSet(false),
    wentThroughNote(false),
    goneThroughNote(false),
    midiNote(-1),
    curMidiNote(-1),
    midiPitch(-1),
    midiAge(0),
    midiAftertouch(false) {}
};

struct DivNoteEvent {
  signed char channel;
  short ins;
  signed char note, volume;
  bool on, nop, insChange, fromMIDI;
  DivNoteEvent(int c, int i, int n, int v, bool o, bool ic=false, bool fm=false):
    channel(c),
    ins(i),
    note(n),
    volume(v),
    on(o),
    nop(false),
    insChange(ic),
    fromMIDI(fm) {}
  DivNoteEvent():
    channel(-1),
    ins(0),
    note(0),
    volume(-1),
    on(false),
    nop(true),
    insChange(false),
    fromMIDI(false) {}
};

struct DivDispatchContainer {
  DivDispatch* dispatch;
  blip_buffer_t* bb[DIV_MAX_OUTPUTS];
  size_t bbInLen, runtotal, runLeft, runPos, lastAvail;
  int temp[DIV_MAX_OUTPUTS], prevSample[DIV_MAX_OUTPUTS];
  short* bbInMapped[DIV_MAX_OUTPUTS];
  short* bbIn[DIV_MAX_OUTPUTS];
  short* bbOut[DIV_MAX_OUTPUTS];
  bool lowQuality, dcOffCompensation, hiPass;
  double rateMemory;

  // used in multi-thread
  int cycles;
  unsigned int size;

  void setRates(double gotRate);
  void setQuality(bool lowQual, bool dcHiPass);
  void grow(size_t size);
  void acquire(size_t count);
  void flush(size_t offset, size_t count);
  void fillBuf(size_t runtotal, size_t offset, size_t size);
  void clear();
  void init(DivSystem sys, DivEngine* eng, int chanCount, double gotRate, const DivConfig& flags, bool isRender=false);
  void quit();
  DivDispatchContainer():
    dispatch(NULL),
    bbInLen(0),
    runtotal(0),
    runLeft(0),
    runPos(0),
    lastAvail(0),
    lowQuality(false),
    dcOffCompensation(false),
    hiPass(true),
    rateMemory(0.0),
    cycles(0),
    size(0) {
    memset(bb,0,DIV_MAX_OUTPUTS*sizeof(blip_buffer_t*));
    memset(temp,0,DIV_MAX_OUTPUTS*sizeof(int));
    memset(prevSample,0,DIV_MAX_OUTPUTS*sizeof(int));
    memset(bbIn,0,DIV_MAX_OUTPUTS*sizeof(short*));
    memset(bbInMapped,0,DIV_MAX_OUTPUTS*sizeof(short*));
    memset(bbOut,0,DIV_MAX_OUTPUTS*sizeof(short*));
  }
};

struct DivEffectContainer {
  DivEffect* effect;
  float* in[DIV_MAX_OUTPUTS];
  float* out[DIV_MAX_OUTPUTS];
  size_t inLen, outLen;

  void preAcquire(size_t count);
  void acquire(size_t count);
  bool init(DivEffectType effectType, DivEngine* eng, double rate, unsigned short version, const unsigned char* data, size_t len);
  void quit();
  DivEffectContainer():
    effect(NULL),
    inLen(0),
    outLen(0) {
    memset(in,0,DIV_MAX_OUTPUTS*sizeof(float*));
    memset(out,0,DIV_MAX_OUTPUTS*sizeof(float*));
  }
};

typedef int EffectValConversion(unsigned char,unsigned char);

struct EffectHandler {
  DivDispatchCmds dispatchCmd;
  const char* description;
  EffectValConversion* val;
  EffectValConversion* val2;
  EffectHandler(
    DivDispatchCmds dispatchCmd_,
    const char* description_,
    EffectValConversion val_=NULL,
    EffectValConversion val2_=NULL
  ):
  dispatchCmd(dispatchCmd_),
  description(description_),
  val(val_),
  val2(val2_) {}
};

struct DivDoNotHandleEffect {
};

typedef std::unordered_map<unsigned char,const EffectHandler> EffectHandlerMap;

struct DivSysDef {
  const char* name;
  const char* nameJ;
  const char* description;
  unsigned char id;
  unsigned char id_DMF;
  int channels;
  bool isFM, isSTD, isCompound;
  // width 0: variable
  // height 0: no wavetable support
  unsigned short waveWidth, waveHeight;
  unsigned int vgmVersion;
  unsigned int sampleFormatMask;
  const char* chanNames[DIV_MAX_CHANS];
  const char* chanShortNames[DIV_MAX_CHANS];
  int chanTypes[DIV_MAX_CHANS];
  // 0: primary
  // 1: alternate (usually PCM)
  DivInstrumentType chanInsType[DIV_MAX_CHANS][2];
  const EffectHandlerMap effectHandlers;
  const EffectHandlerMap postEffectHandlers;
  const EffectHandlerMap preEffectHandlers;
  DivSysDef(
    const char* sysName, const char* sysNameJ, unsigned char fileID, unsigned char fileID_DMF, int chans,
    bool isFMChip, bool isSTDChip, unsigned int vgmVer, bool compound, unsigned int formatMask, unsigned short waveWid, unsigned short waveHei,
    const char* desc,
    std::initializer_list<const char*> chNames,
    std::initializer_list<const char*> chShortNames,
    std::initializer_list<int> chTypes,
    std::initializer_list<DivInstrumentType> chInsType1,
    std::initializer_list<DivInstrumentType> chInsType2={},
    const EffectHandlerMap fxHandlers_={},
    const EffectHandlerMap postFxHandlers_={},
    const EffectHandlerMap preFxHandlers_={}):
    name(sysName),
    nameJ(sysNameJ),
    description(desc),
    id(fileID),
    id_DMF(fileID_DMF),
    channels(chans),
    isFM(isFMChip),
    isSTD(isSTDChip),
    isCompound(compound),
    waveWidth(waveWid),
    waveHeight(waveHei),
    vgmVersion(vgmVer),
    sampleFormatMask(formatMask),
    effectHandlers(fxHandlers_),
    postEffectHandlers(postFxHandlers_),
    preEffectHandlers(preFxHandlers_) {
    memset(chanNames,0,DIV_MAX_CHANS*sizeof(void*));
    memset(chanShortNames,0,DIV_MAX_CHANS*sizeof(void*));
    memset(chanTypes,0,DIV_MAX_CHANS*sizeof(int));
    for (int i=0; i<DIV_MAX_CHANS; i++) {
      chanInsType[i][0]=DIV_INS_NULL;
      chanInsType[i][1]=DIV_INS_NULL;
    }

    int index=0;
    for (const char* i: chNames) {
      chanNames[index++]=i;
      if (index>=DIV_MAX_CHANS) break;
    }

    index=0;
    for (const char* i: chShortNames) {
      chanShortNames[index++]=i;
      if (index>=DIV_MAX_CHANS) break;
    }

    index=0;
    for (int i: chTypes) {
      chanTypes[index++]=i;
      if (index>=DIV_MAX_CHANS) break;
    }

    index=0;
    for (DivInstrumentType i: chInsType1) {
      chanInsType[index++][0]=i;
      if (index>=DIV_MAX_CHANS) break;
    }

    index=0;
    for (DivInstrumentType i: chInsType2) {
      chanInsType[index++][1]=i;
      if (index>=DIV_MAX_CHANS) break;
    }
  }
};

enum DivChanTypes {
  DIV_CH_FM=0,
  DIV_CH_PULSE=1,
  DIV_CH_NOISE=2,
  DIV_CH_WAVE=3,
  DIV_CH_PCM=4,
  DIV_CH_OP=5
};

extern const char* cmdName[];

class DivEngine {
  DivDispatchContainer disCont[DIV_MAX_CHIPS];
  TAAudio* output;
  TAAudioDesc want, got;
  String exportPath;
  std::thread* exportThread;
  int chans;
  bool configLoaded;
  bool active;
  bool lowQuality;
  bool dcHiPass;
  bool playing;
  bool freelance;
  bool shallStop, shallStopSched;
  bool endOfSong;
  bool consoleMode;
  bool disableStatusOut;
  bool extValuePresent;
  bool repeatPattern;
  bool metronome;
  bool exporting;
  bool stopExport;
  bool halted;
  bool forceMono;
  bool clampSamples;
  bool cmdStreamEnabled;
  bool softLocked;
  bool firstTick;
  bool skipping;
  bool midiIsDirect;
  bool midiIsDirectProgram;
  bool lowLatency;
  bool systemsRegistered;
  bool romExportsRegistered;
  bool hasLoadedSomething;
  bool midiOutClock;
  bool midiOutTime;
  bool midiOutProgramChange;
  int midiOutMode;
  int midiOutTimeRate;
  float midiVolExp;
  int softLockCount;
  int subticks, ticks, curRow, curOrder, prevRow, prevOrder, remainingLoops, totalLoops, lastLoopPos, exportLoopCount, curExportChan, nextSpeed, prevSpeed, elapsedBars, elapsedBeats, curSpeed;
  size_t curSubSongIndex;
  size_t bufferPos;
  double divider;
  int cycles;
  double clockDrift;
  int midiClockCycles;
  double midiClockDrift;
  int midiTimeCycles;
  double midiTimeDrift;
  int stepPlay;
  int changeOrd, changePos, totalSeconds, totalTicks, totalTicksR, curMidiClock, curMidiTime, totalCmds, lastCmds, cmdsPerSecond, globalPitch;
  double totalTicksOff;
  int curMidiTimePiece, curMidiTimeCode;
  unsigned char extValue, pendingMetroTick;
  DivGroovePattern speeds;
  short virtualTempoN, virtualTempoD;
  short tempoAccum;
  DivStatusView view;
  DivHaltPositions haltOn;
  DivChannelState chan[DIV_MAX_CHANS];
  DivAudioEngines audioEngine;
  DivAudioExportModes exportMode;
  DivAudioExportFormats exportFormat;
  double exportFadeOut;
  bool isFadingOut;
  int exportOutputs;
  bool exportChannelMask[DIV_MAX_CHANS];
  DivConfig conf;
  FixedQueue<DivNoteEvent,8192> pendingNotes;
  // bitfield
  unsigned char walked[8192];
  bool isMuted[DIV_MAX_CHANS];
  std::mutex isBusy, saveLock, playPosLock;
  String configPath;
  String configFile;
  String lastError;
  String warnings;
  std::vector<String> audioDevs;
  std::vector<String> midiIns;
  std::vector<String> midiOuts;
  std::vector<DivCommand> cmdStream;
  std::vector<DivInstrumentType> possibleInsTypes;
  std::vector<DivEffectContainer> effectInst;
  std::vector<int> curChanMask;
  static DivSysDef* sysDefs[DIV_MAX_CHIP_DEFS];
  static DivSystem sysFileMapFur[DIV_MAX_CHIP_DEFS];
  static DivSystem sysFileMapDMF[DIV_MAX_CHIP_DEFS];
  static DivROMExportDef* romExportDefs[DIV_ROM_MAX];

  DivCSPlayer* cmdStreamInt;

  struct SamplePreview {
    double rate;
    int sample;
    int wave;
    int pos;
    int pBegin, pEnd;
    int rateMul, posSub;
    bool dir;
    SamplePreview():
      rate(0.0),
      sample(-1),
      wave(-1),
      pos(0),
      pBegin(-1),
      pEnd(-1),
      rateMul(1),
      posSub(0),
      dir(false) {}
  } sPreview;

  short vibTable[64];
  short tremTable[128];
  int reversePitchTable[4096];
  int pitchTable[4096];
  short effectSlotMap[4096];
  int midiBaseChan;
  bool midiPoly;
  bool midiDebug;
  size_t midiAgeCounter;

  blip_buffer_t* samp_bb;
  size_t samp_bbInLen;
  int samp_temp, samp_prevSample;
  short* samp_bbIn;
  short* samp_bbOut;
  unsigned char* metroTick;
  size_t metroTickLen;
  float* metroBuf;
  size_t metroBufLen;
  float metroFreq, metroPos;
  float metroAmp;
  float metroVol;
  float previewVol;

  size_t totalProcessed;

  unsigned int renderPoolThreads;
  DivWorkPool* renderPool;

  // MIDI stuff
  std::function<int(const TAMidiMessage&)> midiCallback=[](const TAMidiMessage&) -> int {return -2;};

  void processRowPre(int i);
  void processRow(int i, bool afterDelay);
  void nextOrder();
  void nextRow();
  void performVGMWrite(SafeWriter* w, DivSystem sys, DivRegWrite& write, int streamOff, double* loopTimer, double* loopFreq, int* loopSample, bool* sampleDir, bool isSecond, int* pendingFreq, int* playingSample, int* setPos, unsigned int* sampleOff8, unsigned int* sampleLen8, size_t bankOffset, bool directStream, bool* sampleStoppable, bool dpcm07, DivDispatch** writeNES);
  // returns true if end of song.
  bool nextTick(bool noAccum=false, bool inhibitLowLat=false);
  bool perSystemEffect(int ch, unsigned char effect, unsigned char effectVal);
  bool perSystemPostEffect(int ch, unsigned char effect, unsigned char effectVal);
  bool perSystemPreEffect(int ch, unsigned char effect, unsigned char effectVal);
  void recalcChans();
  void reset();
  void playSub(bool preserveDrift, int goalRow=0);
  void runMidiClock(int totalCycles=1);
  void runMidiTime(int totalCycles=1);
  bool shallSwitchCores();

  void testFunction();

  bool loadDMF(unsigned char* file, size_t len);
  bool loadFur(unsigned char* file, size_t len, int variantID=0);
  bool loadMod(unsigned char* file, size_t len);
  bool loadS3M(unsigned char* file, size_t len);
  bool loadXM(unsigned char* file, size_t len);
  bool loadIT(unsigned char* file, size_t len);
  bool loadFTM(unsigned char* file, size_t len, bool dnft, bool dnftSig, bool eft);
  bool loadFC(unsigned char* file, size_t len);
  bool loadTFMv1(unsigned char* file, size_t len);
  bool loadTFMv2(unsigned char* file, size_t len);

  void loadDMP(SafeReader& reader, std::vector<DivInstrument*>& ret, String& stripPath);
  void loadTFI(SafeReader& reader, std::vector<DivInstrument*>& ret, String& stripPath);
  void loadVGI(SafeReader& reader, std::vector<DivInstrument*>& ret, String& stripPath);
  void loadS3I(SafeReader& reader, std::vector<DivInstrument*>& ret, String& stripPath);
  void loadSBI(SafeReader& reader, std::vector<DivInstrument*>& ret, String& stripPath);
  void loadOPLI(SafeReader& reader, std::vector<DivInstrument*>& ret, String& stripPath);
  void loadOPNI(SafeReader& reader, std::vector<DivInstrument*>& ret, String& stripPath);
  void loadY12(SafeReader& reader, std::vector<DivInstrument*>& ret, String& stripPath);
  void loadBNK(SafeReader& reader, std::vector<DivInstrument*>& ret, String& stripPath);
  void loadGYB(SafeReader& reader, std::vector<DivInstrument*>& ret, String& stripPath);
  void loadOPM(SafeReader& reader, std::vector<DivInstrument*>& ret, String& stripPath);
  void loadFF(SafeReader& reader, std::vector<DivInstrument*>& ret, String& stripPath);
  void loadWOPL(SafeReader& reader, std::vector<DivInstrument*>& ret, String& stripPath);
  void loadWOPN(SafeReader& reader, std::vector<DivInstrument*>& ret, String& stripPath);
 
 //sample banks
  void loadP(SafeReader& reader, std::vector<DivSample*>& ret, String& stripPath);
  void loadPPC(SafeReader& reader, std::vector<DivSample*>& ret, String& stripPath);
  void loadPPS(SafeReader& reader, std::vector<DivSample*>& ret, String& stripPath);
  void loadPVI(SafeReader& reader, std::vector<DivSample*>& ret, String& stripPath);
  void loadPDX(SafeReader& reader, std::vector<DivSample*>& ret, String& stripPath);
  void loadPZI(SafeReader& reader, std::vector<DivSample*>& ret, String& stripPath);
  void loadP86(SafeReader& reader, std::vector<DivSample*>& ret, String& stripPath);



  int loadSampleROM(String path, ssize_t expectedSize, unsigned char*& ret);

  bool initAudioBackend();
  bool deinitAudioBackend(bool dueToSwitchMaster=false);

  void registerSystems();
  void registerROMExports();
  void initSongWithDesc(const char* description, bool inBase64=true, bool oldVol=false);

  void exchangeIns(int one, int two);
  void exchangeWave(int one, int two);
  void exchangeSample(int one, int two);

  void swapChannels(int src, int dest);
  void stompChannel(int ch);

  // recalculate patchbay (UNSAFE)
  void recalcPatchbay();

  // change song (UNSAFE)
  void changeSong(size_t songIndex);

  void swapSystemUnsafe(int src, int dest, bool preserveOrder=true);

  // move an asset
  void moveAsset(std::vector<DivAssetDir>& dir, int before, int after);

  // remove an asset
  void removeAsset(std::vector<DivAssetDir>& dir, int entry);

  // read/write asset dir
  void putAssetDirData(SafeWriter* w, std::vector<DivAssetDir>& dir);
  DivDataErrors readAssetDirData(SafeReader& reader, std::vector<DivAssetDir>& dir);

  // add every export method here
  friend class DivROMExport;
  friend class DivExportAmigaValidation;
  friend class DivExportSAPR;
  friend class DivExportTiuna;
  friend class DivExportZSM;

  public:
    DivSong song;
    DivOrders* curOrders;
    DivChannelData* curPat;
    DivSubSong* curSubSong;
    DivInstrument* tempIns;
    DivSystem sysOfChan[DIV_MAX_CHANS];
    int dispatchOfChan[DIV_MAX_CHANS];
    int dispatchChanOfChan[DIV_MAX_CHANS];
    int dispatchFirstChan[DIV_MAX_CHANS];
    bool keyHit[DIV_MAX_CHANS];
    float* oscBuf[DIV_MAX_OUTPUTS];
    float oscSize;
    int oscReadPos, oscWritePos;
    int tickMult;
    int lastNBIns, lastNBOuts, lastNBSize;
    std::atomic<size_t> processTime;

    void runExportThread();
    void nextBuf(float** in, float** out, int inChans, int outChans, unsigned int size);
    DivInstrument* getIns(int index, DivInstrumentType fallbackType=DIV_INS_FM);
    DivWavetable* getWave(int index);
    DivSample* getSample(int index);
    DivDispatch* getDispatch(int index);
    // parse old system setup description
    String decodeSysDesc(String desc);
    // start fresh
    void createNew(const char* description, String sysName, bool inBase64=true);
    void createNewFromDefaults();
    // load a file.
    bool load(unsigned char* f, size_t length, const char* nameHint=NULL);
    // play a binary command stream.
    bool playStream(unsigned char* f, size_t length);
    // get the playing stream.
    DivCSPlayer* getStreamPlayer();
    // destroy command stream player.
    bool killStream();
    // save as .dmf.
    SafeWriter* saveDMF(unsigned char version);
    // save as .fur.
    // if notPrimary is true then the song will not be altered
    SafeWriter* saveFur(bool notPrimary=false, bool newPatternFormat=true);
    // return a ROM exporter.
    DivROMExport* buildROM(DivROMExportOptions sys);
    // dump to VGM.
    // set trailingTicks to:
    // - 0 to add one tick of trailing
    // - x to add x+1 ticks of trailing
    // - -1 to auto-determine trailing
    // - -2 to add a whole loop of trailing
    SafeWriter* saveVGM(bool* sysToExport=NULL, bool loop=true, int version=0x171, bool patternHints=false, bool directStream=false, int trailingTicks=-1, bool dpcm07=false);
    // dump to TIunA.
    SafeWriter* saveTiuna(const bool* sysToExport, const char* baseLabel, int firstBankSize, int otherBankSize);
    // dump command stream.
    SafeWriter* saveCommand(DivCSProgress* progress=NULL, DivCSOptions options=DivCSOptions());
    // export to text
    SafeWriter* saveText(bool separatePatterns=true);
    // export to an audio file
    bool saveAudio(const char* path, DivAudioExportOptions options);
    // wait for audio export to finish
    void waitAudioFile();
    // stop audio file export
    bool haltAudioFile();
    // return back to playback cores if necessary
    void finishAudioFile();
    // notify instrument parameter change
    void notifyInsChange(int ins);
    // notify wavetable change
    void notifyWaveChange(int wave);

    // dispatch a command
    int dispatchCmd(DivCommand c);

    // get system IDs
    static DivSystem systemFromFileFur(unsigned char val);
    static unsigned char systemToFileFur(DivSystem val);
    static DivSystem systemFromFileDMF(unsigned char val);
    static unsigned char systemToFileDMF(DivSystem val);

    // convert old flags
    static void convertOldFlags(unsigned int oldFlags, DivConfig& newFlags, DivSystem sys);

    // check whether an asset directory is complete (UNSAFE)
    void checkAssetDir(std::vector<DivAssetDir>& dir, size_t entries);

    // benchmark (returns time in seconds)
    double benchmarkPlayback();
    double benchmarkSeek();

    // returns the minimum VGM version which may carry the specified system, or 0 if none.
    int minVGMVersion(DivSystem which);

    // determine and setup config dir
    void initConfDir();

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

    // get config object
    DivConfig& getConfObject();

    // set a config value
    void setConf(String key, bool value);
    void setConf(String key, int value);
    void setConf(String key, float value);
    void setConf(String key, double value);
    void setConf(String key, const char* value);
    void setConf(String key, String value);

    // get whether config value exists
    bool hasConf(String key);

    // reset all settings
    void factoryReset();

    // calculate base frequency/period
    double calcBaseFreq(double clock, double divider, int note, bool period);

    // calculate base frequency in f-num/block format
    int calcBaseFreqFNumBlock(double clock, double divider, int note, int bits, int fixedBlock);

    // calculate frequency/period
    int calcFreq(int base, int pitch, int arp, bool arpFixed, bool period=false, int octave=0, int pitch2=0, double clock=1.0, double divider=1.0, int blockBits=0, int fixedBlock=0);

    // calculate arpeggio
    int calcArp(int note, int arp, int offset=0);

    // convert panning formats
    int convertPanSplitToLinear(unsigned int val, unsigned char bits, int range);
    int convertPanSplitToLinearLR(unsigned char left, unsigned char right, int range);
    unsigned int convertPanLinearToSplit(int val, unsigned char bits, int range);

    // find song loop position
    void walkSong(int& loopOrder, int& loopRow, int& loopEnd);

    // find song length in rows (up to specified loop point), and find length of every order
    void findSongLength(int loopOrder, int loopRow, double fadeoutLen, int& rowsForFadeout, bool& hasFFxx, std::vector<int>& orders, int& length);

    // play (returns whether successful)
    bool play();

    // play to row (returns whether successful)
    bool playToRow(int row);

    // play by one row
    void stepOne(int row);

    // stop
    void stop();

    // reset playback state
    void syncReset();

    // get C-4 rate for samples
    double getCenterRate();

    // sample preview query
    bool isPreviewingSample();
    int getSamplePreviewSample();
    int getSamplePreviewPos();
    double getSamplePreviewRate();

    // set sample preview volume (1.0 = 100%)
    void setSamplePreviewVol(float vol);

    // trigger sample preview
    void previewSample(int sample, int note=-1, int pStart=-1, int pEnd=-1);
    void stopSamplePreview();

    // trigger wave preview
    void previewWave(int wave, int note);
    void stopWavePreview();

    // trigger sample preview
    void previewSampleNoLock(int sample, int note=-1, int pStart=-1, int pEnd=-1);
    void stopSamplePreviewNoLock();

    // trigger wave preview
    void previewWaveNoLock(int wave, int note);
    void stopWavePreviewNoLock();

    // get config path
    String getConfigPath();

    // get sys channel count
    int getChannelCount(DivSystem sys);

    // get channel count
    int getTotalChannelCount();

    // get instrument types available for use
    std::vector<DivInstrumentType>& getPossibleInsTypes();

    // get effect description
    const char* getEffectDesc(unsigned char effect, int chan, bool notNull=false);

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

    // get alternate instrument type
    DivInstrumentType getPreferInsSecondType(int ch);

    // get song system name
    String getSongSystemLegacyName(DivSong& ds, bool isMultiSystemAcceptable=true);

    // get sys name
    const char* getSystemName(DivSystem sys);

    // get japanese system name
    const char* getSystemNameJ(DivSystem sys);

    // get sys definition
    const DivSysDef* getSystemDef(DivSystem sys);

    // get ROM export definition
    const DivROMExportDef* getROMExportDef(DivROMExportOptions opt);
    // check whether ROM export option is viable for current song
    bool isROMExportViable(DivROMExportOptions opt);

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

    // map MIDI velocity to volume
    int mapVelocity(int ch, float vel);

    // map volume to gain
    float getGain(int ch, int vol);

    // get current order
    unsigned char getOrder();

    // get current row
    int getRow();

    // synchronous get order/row
    void getPlayPos(int& order, int& row);
    void getPlayPosTick(int& order, int& row, int& tick, int& speed);

    // get beat/bar
    int getElapsedBars();
    int getElapsedBeats();

    // get current subsong
    size_t getCurrentSubSong();

    // get speeds
    const DivGroovePattern& getSpeeds();

    // get Hz
    float getHz();

    // get current Hz
    float getCurHz();

    // get virtual tempo
    short getVirtualTempoN();
    short getVirtualTempoD();

    // tell engine about virtual tempo changes
    void virtualTempoChanged();

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

    // dump song info to stdout
    void dumpSongInfo();

    // is playing
    bool isPlaying();

    // is running
    bool isRunning();

    // is stepping
    bool isStepping();

    // is exporting
    bool isExporting();

    // get how many loops is left
    void getLoopsLeft(int& loops);

    // get how many loops in total export needs to do
    void getTotalLoops(int& loops);

    // get current position in song
    void getCurSongPos(int& row, int& order);

    // get how many files export needs to create
    void getTotalAudioFiles(int& files);

    // get which file is processed right now (progress for e.g. per-channel export)
    void getCurFileIndex(int& file);

    // get fadeout state
    bool getIsFadingOut();

    // add instrument
    int addInstrument(int refChan=0, DivInstrumentType fallbackType=DIV_INS_STD);

    // add instrument from pointer
    int addInstrumentPtr(DivInstrument* which);

    // get instrument from file
    // if the returned vector is empty then there was an error.
    std::vector<DivInstrument*> instrumentFromFile(const char* path, bool loadAssets=true, bool readInsName=true);

    // load temporary instrument
    void loadTempIns(DivInstrument* which);

    // delete instrument
    void delInstrument(int index);
    void delInstrumentUnsafe(int index);

    // add wavetable
    int addWave();

    // add wavetable from pointer
    int addWavePtr(DivWavetable* which);

    // get wavetable from file
    DivWavetable* waveFromFile(const char* path, bool loadRaw=true);

    // delete wavetable
    void delWave(int index);
    void delWaveUnsafe(int index);

    // add sample
    int addSample();

    // add sample from pointer
    int addSamplePtr(DivSample* which);

    // get sample from file
    //DivSample* sampleFromFile(const char* path);
    std::vector<DivSample*> sampleFromFile(const char* path);

    // get raw sample
    DivSample* sampleFromFileRaw(const char* path, DivSampleDepth depth, int channels, bool bigEndian, bool unsign, bool swapNibbles, int rate);

    // delete sample
    void delSample(int index);
    void delSampleUnsafe(int index, bool render=true);

    // add order
    void addOrder(int pos, bool duplicate, bool where);

    // deep clone orders
    void deepCloneOrder(int pos, bool where);

    // delete order
    void deleteOrder(int pos);

    // move order up
    void moveOrderUp(int& pos);

    // move order down
    void moveOrderDown(int& pos);

    // move thing up
    bool moveInsUp(int which);
    bool moveWaveUp(int which);
    bool moveSampleUp(int which);

    // move thing down
    bool moveInsDown(int which);
    bool moveWaveDown(int which);
    bool moveSampleDown(int which);

    // swap things
    bool swapInstruments(int a, int b);
    bool swapWaves(int a, int b);
    bool swapSamples(int a, int b);

    // automatic patchbay
    void autoPatchbay();
    void autoPatchbayP();

    // connect in patchbay
    // returns false if connection already made
    bool patchConnect(unsigned int src, unsigned int dest);

    // disconnect in patchbay
    // returns false if connection doesn't exist
    bool patchDisconnect(unsigned int src, unsigned int dest);

    // disconnect all in patchbay
    void patchDisconnectAll(unsigned int portSet);

    // play note
    void noteOn(int chan, int ins, int note, int vol=-1);

    // stop note
    void noteOff(int chan);

    // returns whether it could
    bool autoNoteOn(int chan, int ins, int note, int vol=-1);
    void autoNoteOff(int chan, int note, int vol=-1);
    void autoNoteOffAll();

    // set whether autoNoteIn is mono or poly
    void setAutoNotePoly(bool poly);

    // go to order
    void setOrder(unsigned char order);

    // update system flags
    void updateSysFlags(int system, bool restart, bool render);

    // set Hz
    void setSongRate(float hz);

    // set remaining loops. -1 means loop forever.
    void setLoops(int loops);

    // get channel state
    DivChannelState* getChanState(int chan);

    // get dispatch channel state
    void* getDispatchChanState(int chan);

    // get channel pairs
    void getChanPaired(int chan, std::vector<DivChannelPair>& ret);

    // get channel mode hints
    DivChannelModeHints getChanModeHints(int chan);

    // get register pool
    unsigned char* getRegisterPool(int sys, int& size, int& depth);

    // get macro interpreter
    DivMacroInt* getMacroInt(int chan);

    // get channel panning
    unsigned short getChanPan(int chan);

    // get sample position
    DivSamplePos getSamplePos(int chan);

    // get osc buffer
    DivDispatchOscBuffer* getOscBuffer(int chan);

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

    // get available MIDI inputs
    std::vector<String>& getMidiIns();

    // get available MIDI inputs
    std::vector<String>& getMidiOuts();

    // rescan audio devices
    void rescanAudioDevices();

    /** rescan midi devices */
    void rescanMidiDevices();

    // set the console mode.
    void setConsoleMode(bool enable, bool statusOut=true);

    // get metronome
    bool getMetronome();

    // set metronome
    void setMetronome(bool enable);

    // set metronome volume (1.0 = 100%)
    void setMetronomeVol(float vol);

    // get buffer position
    int getBufferPos();

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

    // load sample ROMs
    int loadSampleROMs();

    // get the sample format mask
    unsigned int getSampleFormatMask();

    // UNSAFE render samples - only execute when locked
    void renderSamples(int whichSample=-1);

    // public render samples
    // values for whichSample
    // -2: don't render anything - just update chip sample memory
    // -1: render all samples
    // >=0: render specific sample
    void renderSamplesP(int whichSample=-1);

    // public swap channels
    void swapChannelsP(int src, int dest);

    // public change song
    void changeSongP(size_t index);

    // add subsong
    int addSubSong();

    // duplicate subsong
    int duplicateSubSong(int index);

    // remove subsong
    bool removeSubSong(int index);

    // move subsong
    void moveSubSongUp(size_t index);
    void moveSubSongDown(size_t index);

    // clear all subsong data
    void clearSubSongs();

    // optimize assets
    void delUnusedIns();
    void delUnusedWaves();
    void delUnusedSamples();

    // change system
    bool changeSystem(int index, DivSystem which, bool preserveOrder=true);

    // add system
    bool addSystem(DivSystem which);

    // duplicate system
    bool duplicateSystem(int index, bool pat=true, bool end=false);

    // remove system
    bool removeSystem(int index, bool preserveOrder=true);

    // move system
    bool swapSystem(int src, int dest, bool preserveOrder=true);

    // add effect
    bool addEffect(DivEffectType which);

    // remove effect
    bool removeEffect(int index);

    // write to register on system
    void poke(int sys, unsigned int addr, unsigned short val);

    // write to register on system
    void poke(int sys, std::vector<DivRegWrite>& wlist);

    // get last error
    String getLastError();

    // get warnings
    String getWarnings();

    // get debug info
    String getPlaybackDebugInfo();

    // switch master
    bool switchMaster(bool full=false);

    // set MIDI base channel
    void setMidiBaseChan(int chan);

    // set MIDI direct channel map
    void setMidiDirect(bool value);

    // set MIDI direct program change
    void setMidiDirectProgram(bool value);

    // set MIDI volume curve exponent
    void setMidiVolExp(float value);

    // set MIDI input callback
    // if the specified function returns -2, note feedback will be inhibited.
    void setMidiCallback(std::function<int(const TAMidiMessage&)> what);

    // send MIDI message
    bool sendMidiMessage(TAMidiMessage& msg);

    // enable MIDI debug
    void setMidiDebug(bool enable);

    // perform secure/sync operation
    void synchronized(const std::function<void()>& what);

    // perform secure/sync operation (soft)
    void synchronizedSoft(const std::function<void()>& what);

    // perform secure/sync song operation
    void lockSave(const std::function<void()>& what);

    // perform secure/sync song operation (and lock audio too)
    void lockEngine(const std::function<void()>& what);

    // get audio desc want
    TAAudioDesc& getAudioDescWant();

    // get audio desc
    TAAudioDesc& getAudioDescGot();

    // init dispatch
    void initDispatch(bool isRender=false);

    // quit dispatch
    void quitDispatch();

    // pre-pre-initialize the engine.
    bool prePreInit();

    // pre-initialize the engine. returns whether Furnace should run in safe mode.
    bool preInit(bool noSafeMode=true);

    // initialize the engine.
    bool init();

    // confirm that the engine is running (delete safe mode file).
    void everythingOK();

    // terminate the engine.
    bool quit(bool saveConfig=true);

    unsigned char* yrw801ROM;
    unsigned char* tg100ROM;
    unsigned char* mu5ROM;

    DivEngine():
      output(NULL),
      exportThread(NULL),
      chans(0),
      configLoaded(false),
      active(false),
      lowQuality(false),
      dcHiPass(true),
      playing(false),
      freelance(false),
      shallStop(false),
      shallStopSched(false),
      endOfSong(false),
      consoleMode(false),
      disableStatusOut(false),
      extValuePresent(false),
      repeatPattern(false),
      metronome(false),
      exporting(false),
      stopExport(false),
      halted(false),
      forceMono(false),
      cmdStreamEnabled(false),
      softLocked(false),
      firstTick(false),
      skipping(false),
      midiIsDirect(false),
      midiIsDirectProgram(false),
      lowLatency(false),
      systemsRegistered(false),
      romExportsRegistered(false),
      hasLoadedSomething(false),
      midiOutClock(false),
      midiOutTime(false),
      midiOutProgramChange(false),
      midiOutMode(DIV_MIDI_MODE_NOTE),
      midiOutTimeRate(0),
      midiVolExp(2.0f), // General MIDI standard
      softLockCount(0),
      subticks(0),
      ticks(0),
      curRow(0),
      curOrder(0),
      prevRow(0),
      prevOrder(0),
      remainingLoops(-1),
      totalLoops(0),
      lastLoopPos(0),
      exportLoopCount(0),
      curExportChan(0),
      nextSpeed(3),
      prevSpeed(6),
      elapsedBars(0),
      elapsedBeats(0),
      curSpeed(0),
      curSubSongIndex(0),
      bufferPos(0),
      divider(60),
      cycles(0),
      clockDrift(0),
      midiClockCycles(0),
      midiClockDrift(0),
      midiTimeCycles(0),
      midiTimeDrift(0),
      stepPlay(0),
      changeOrd(-1),
      changePos(0),
      totalSeconds(0),
      totalTicks(0),
      totalTicksR(0),
      curMidiClock(0),
      curMidiTime(0),
      totalCmds(0),
      lastCmds(0),
      cmdsPerSecond(0),
      globalPitch(0),
      totalTicksOff(0.0),
      curMidiTimePiece(0),
      curMidiTimeCode(0),
      extValue(0),
      pendingMetroTick(0),
      virtualTempoN(150),
      virtualTempoD(150),
      tempoAccum(0),
      view(DIV_STATUS_NOTHING),
      haltOn(DIV_HALT_NONE),
      audioEngine(DIV_AUDIO_NULL),
      exportMode(DIV_EXPORT_MODE_ONE),
      exportFormat(DIV_EXPORT_FORMAT_S16),
      exportFadeOut(0.0),
      isFadingOut(false),
      exportOutputs(2),
      cmdStreamInt(NULL),
      midiBaseChan(0),
      midiPoly(true),
      midiDebug(false),
      midiAgeCounter(0),
      samp_bb(NULL),
      samp_bbInLen(0),
      samp_temp(0),
      samp_prevSample(0),
      samp_bbIn(NULL),
      samp_bbOut(NULL),
      metroTick(NULL),
      metroTickLen(0),
      metroBuf(NULL),
      metroBufLen(0),
      metroFreq(0),
      metroPos(0),
      metroAmp(0.0f),
      metroVol(1.0f),
      previewVol(1.0f),
      totalProcessed(0),
      renderPoolThreads(0),
      renderPool(NULL),
      curOrders(NULL),
      curPat(NULL),
      tempIns(NULL),
      oscSize(1),
      oscReadPos(0),
      oscWritePos(0),
      tickMult(1),
      lastNBIns(0),
      lastNBOuts(0),
      lastNBSize(0),
      processTime(0),
      yrw801ROM(NULL),
      tg100ROM(NULL),
      mu5ROM(NULL) {
      memset(isMuted,0,DIV_MAX_CHANS*sizeof(bool));
      memset(keyHit,0,DIV_MAX_CHANS*sizeof(bool));
      memset(dispatchFirstChan,0,DIV_MAX_CHANS*sizeof(int));
      memset(dispatchChanOfChan,0,DIV_MAX_CHANS*sizeof(int));
      memset(dispatchOfChan,0,DIV_MAX_CHANS*sizeof(int));
      memset(sysOfChan,0,DIV_MAX_CHANS*sizeof(int));
      memset(vibTable,0,64*sizeof(short));
      memset(tremTable,0,128*sizeof(short));
      memset(reversePitchTable,0,4096*sizeof(int));
      memset(pitchTable,0,4096*sizeof(int));
      memset(effectSlotMap,-1,4096*sizeof(short));
      memset(sysDefs,0,DIV_MAX_CHIP_DEFS*sizeof(void*));
      memset(romExportDefs,0,DIV_ROM_MAX*sizeof(void*));
      memset(walked,0,8192);
      memset(oscBuf,0,DIV_MAX_OUTPUTS*(sizeof(float*)));
      memset(exportChannelMask,1,DIV_MAX_CHANS*sizeof(bool));

      for (int i=0; i<DIV_MAX_CHIP_DEFS; i++) {
        sysFileMapFur[i]=DIV_SYSTEM_NULL;
        sysFileMapDMF[i]=DIV_SYSTEM_NULL;
      }

      changeSong(0);
    }
};
#endif
