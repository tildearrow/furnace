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
#include "config.h"
#include "instrument.h"
#include "song.h"
#include "dispatch.h"
#include "dataErrors.h"
#include "safeWriter.h"
#include "../audio/taAudio.h"
#include "blip_buf.h"
#include <atomic>
#include <functional>
#include <initializer_list>
#include <thread>
#include <mutex>
#include <map>
#include <unordered_map>
#include <deque>

#define addWarning(x) \
  if (warnings.empty()) { \
    warnings+=x; \
  } else { \
    warnings+=(String("\n")+x); \
  }

#define BUSY_BEGIN softLocked=false; isBusy.lock();
#define BUSY_BEGIN_SOFT softLocked=true; isBusy.lock();
#define BUSY_END isBusy.unlock(); softLocked=false;

#define DIV_VERSION "dev121"
#define DIV_ENGINE_VERSION 121
// for imports
#define DIV_VERSION_MOD 0xff01
#define DIV_VERSION_FC 0xff02

// "Namco C163"
#define DIV_C163_DEFAULT_NAME "Namco 163"

enum DivStatusView {
  DIV_STATUS_NOTHING=0,
  DIV_STATUS_PATTERN,
  DIV_STATUS_COMMANDS
};

enum DivAudioEngines {
  DIV_AUDIO_JACK=0,
  DIV_AUDIO_SDL=1,

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

struct DivChannelState {
  std::vector<DivDelayedCommand> delayed;
  int note, oldNote, lastIns, pitch, portaSpeed, portaNote;
  int volume, volSpeed, cut, rowDelay, volMax;
  int delayOrder, delayRow, retrigSpeed, retrigTick;
  int vibratoDepth, vibratoRate, vibratoPos, vibratoPosGiant, vibratoDir, vibratoFine;
  int tremoloDepth, tremoloRate, tremoloPos;
  unsigned char arp, arpStage, arpTicks, panL, panR;
  bool doNote, legato, portaStop, keyOn, keyOff, nowYouCanStop, stopOnOff;
  bool arpYield, delayLocked, inPorta, scheduledSlideReset, shorthandPorta, wasShorthandPorta, noteOnInhibit, resetArp;

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
    vibratoPosGiant(0),
    vibratoDir(0),
    vibratoFine(15),
    tremoloDepth(0),
    tremoloRate(0),
    tremoloPos(0),
    arp(0),
    arpStage(-1),
    arpTicks(1),
    panL(255),
    panR(255),
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
    wasShorthandPorta(false),
    noteOnInhibit(false),
    resetArp(false),
    midiNote(-1),
    curMidiNote(-1),
    midiPitch(-1),
    midiAge(0),
    midiAftertouch(false) {}
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
  size_t bbInLen, runtotal, runLeft, runPos, lastAvail;
  int temp[2], prevSample[2];
  short* bbIn[2];
  short* bbOut[2];
  bool lowQuality, dcOffCompensation;

  void setRates(double gotRate);
  void setQuality(bool lowQual);
  void acquire(size_t offset, size_t count);
  void flush(size_t count);
  void fillBuf(size_t runtotal, size_t offset, size_t size);
  void clear();
  void init(DivSystem sys, DivEngine* eng, int chanCount, double gotRate, const DivConfig& flags);
  void quit();
  DivDispatchContainer():
    dispatch(NULL),
    bb{NULL,NULL},
    bbInLen(0),
    runtotal(0),
    runLeft(0),
    runPos(0),
    lastAvail(0),
    temp{0,0},
    prevSample{0,0},
    bbIn{NULL,NULL},
    bbOut{NULL,NULL},
    lowQuality(false),
    dcOffCompensation(false) {}
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
  DivSysDef(
    const char* sysName, const char* sysNameJ, unsigned char fileID, unsigned char fileID_DMF, int chans,
    bool isFMChip, bool isSTDChip, unsigned int vgmVer, bool compound, unsigned int formatMask, const char* desc,
    std::initializer_list<const char*> chNames,
    std::initializer_list<const char*> chShortNames,
    std::initializer_list<int> chTypes,
    std::initializer_list<DivInstrumentType> chInsType1,
    std::initializer_list<DivInstrumentType> chInsType2={},
    const EffectHandlerMap fxHandlers_={},
    const EffectHandlerMap postFxHandlers_={}):
    name(sysName),
    nameJ(sysNameJ),
    description(desc),
    id(fileID),
    id_DMF(fileID_DMF),
    channels(chans),
    isFM(isFMChip),
    isSTD(isSTDChip),
    isCompound(compound),
    vgmVersion(vgmVer),
    sampleFormatMask(formatMask),
    effectHandlers(fxHandlers_),
    postEffectHandlers(postFxHandlers_) {
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
  bool shallStop;
  bool speedAB;
  bool endOfSong;
  bool consoleMode;
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
  bool lowLatency;
  bool systemsRegistered;
  bool hasLoadedSomething;
  bool midiOutClock;
  int midiOutMode;
  int softLockCount;
  int subticks, ticks, curRow, curOrder, prevRow, prevOrder, remainingLoops, totalLoops, lastLoopPos, exportLoopCount, nextSpeed;
  size_t curSubSongIndex;
  double divider;
  int cycles;
  double clockDrift;
  int stepPlay;
  int changeOrd, changePos, totalSeconds, totalTicks, totalTicksR, totalCmds, lastCmds, cmdsPerSecond, globalPitch;
  unsigned char extValue;
  unsigned char speed1, speed2;
  short tempoAccum;
  DivStatusView view;
  DivHaltPositions haltOn;
  DivChannelState chan[DIV_MAX_CHANS];
  DivAudioEngines audioEngine;
  DivAudioExportModes exportMode;
  double exportFadeOut;
  DivConfig conf;
  std::deque<DivNoteEvent> pendingNotes;
  // bitfield
  unsigned char walked[8192];
  bool isMuted[DIV_MAX_CHANS];
  std::mutex isBusy, saveLock;
  String configPath;
  String configFile;
  String lastError;
  String warnings;
  std::vector<String> audioDevs;
  std::vector<String> midiIns;
  std::vector<String> midiOuts;
  std::vector<DivCommand> cmdStream;
  std::vector<DivInstrumentType> possibleInsTypes;
  DivSysDef* sysDefs[256];
  DivSystem sysFileMapFur[256];
  DivSystem sysFileMapDMF[256];

  struct SamplePreview {
    double rate;
    int sample;
    int wave;
    int pos;
    int pBegin, pEnd;
    bool dir;
    SamplePreview():
      rate(0.0),
      sample(-1),
      wave(-1),
      pos(0),
      pBegin(-1),
      pEnd(-1),
      dir(false) {}
  } sPreview;

  short vibTable[64];
  int reversePitchTable[4096];
  int pitchTable[4096];
  char c163NameCS[1024];
  int midiBaseChan;
  bool midiPoly;
  size_t midiAgeCounter;

  blip_buffer_t* samp_bb;
  size_t samp_bbInLen;
  int samp_temp, samp_prevSample;
  short* samp_bbIn;
  short* samp_bbOut;
  unsigned char* metroTick;
  size_t metroTickLen;
  float metroFreq, metroPos;
  float metroAmp;
  float metroVol;

  size_t totalProcessed;

  // MIDI stuff
  std::function<int(const TAMidiMessage&)> midiCallback=[](const TAMidiMessage&) -> int {return -2;};

  int dispatchCmd(DivCommand c);
  void processRow(int i, bool afterDelay);
  void nextOrder();
  void nextRow();
  void performVGMWrite(SafeWriter* w, DivSystem sys, DivRegWrite& write, int streamOff, double* loopTimer, double* loopFreq, int* loopSample, bool* sampleDir, bool isSecond);
  // returns true if end of song.
  bool nextTick(bool noAccum=false, bool inhibitLowLat=false);
  bool perSystemEffect(int ch, unsigned char effect, unsigned char effectVal);
  bool perSystemPostEffect(int ch, unsigned char effect, unsigned char effectVal);
  void recalcChans();
  void reset();
  void playSub(bool preserveDrift, int goalRow=0);

  void convertOldFlags(unsigned int oldFlags, DivConfig& newFlags, DivSystem sys);

  bool loadDMF(unsigned char* file, size_t len);
  bool loadFur(unsigned char* file, size_t len);
  bool loadMod(unsigned char* file, size_t len);
  bool loadFTM(unsigned char* file, size_t len);
  bool loadFC(unsigned char* file, size_t len);

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

  int loadSampleROM(String path, ssize_t expectedSize, unsigned char*& ret);

  bool initAudioBackend();
  bool deinitAudioBackend(bool dueToSwitchMaster=false);

  void registerSystems();
  void initSongWithDesc(const char* description);

  void exchangeIns(int one, int two);
  void swapChannels(int src, int dest);
  void stompChannel(int ch);

  // change song (UNSAFE)
  void changeSong(size_t songIndex);

  public:
    DivSong song;
    DivOrders* curOrders;
    DivChannelData* curPat;
    DivSubSong* curSubSong;
    DivInstrument* tempIns;
    DivSystem sysOfChan[DIV_MAX_CHANS];
    int dispatchOfChan[DIV_MAX_CHANS];
    int dispatchChanOfChan[DIV_MAX_CHANS];
    bool keyHit[DIV_MAX_CHANS];
    float* oscBuf[2];
    float oscSize;
    int oscReadPos, oscWritePos;
    int tickMult;
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
    void createNew(const char* description, String sysName);
    // load a file.
    bool load(unsigned char* f, size_t length);
    // save as .dmf.
    SafeWriter* saveDMF(unsigned char version);
    // save as .fur.
    // if notPrimary is true then the song will not be altered
    SafeWriter* saveFur(bool notPrimary=false);
    // build a ROM file (TODO).
    // specify system to build ROM for.
    SafeWriter* buildROM(int sys);
    // dump to VGM.
    SafeWriter* saveVGM(bool* sysToExport=NULL, bool loop=true, int version=0x171, bool patternHints=false);
    // dump to ZSM.
    SafeWriter* saveZSM(unsigned int zsmrate=60, bool loop=true);
    // dump command stream.
    SafeWriter* saveCommand(bool binary=false);
    // export to an audio file
    bool saveAudio(const char* path, int loops, DivAudioExportModes mode, double fadeOutTime=0.0);
    // wait for audio export to finish
    void waitAudioFile();
    // stop audio file export
    bool haltAudioFile();
    // notify instrument parameter change
    void notifyInsChange(int ins);
    // notify wavetable change
    void notifyWaveChange(int wave);

    // get system IDs
    DivSystem systemFromFileFur(unsigned char val);
    unsigned char systemToFileFur(DivSystem val);
    DivSystem systemFromFileDMF(unsigned char val);
    unsigned char systemToFileDMF(DivSystem val);

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

    // set a config value
    void setConf(String key, bool value);
    void setConf(String key, int value);
    void setConf(String key, float value);
    void setConf(String key, double value);
    void setConf(String key, const char* value);
    void setConf(String key, String value);

    // calculate base frequency/period
    double calcBaseFreq(double clock, double divider, int note, bool period);

    // calculate base frequency in f-num/block format
    int calcBaseFreqFNumBlock(double clock, double divider, int note, int bits);

    // calculate frequency/period
    int calcFreq(int base, int pitch, bool period=false, int octave=0, int pitch2=0, double clock=1.0, double divider=1.0, int blockBits=0);

    // calculate arpeggio
    int calcArp(int note, int arp, int offset=0);

    // convert panning formats
    int convertPanSplitToLinear(unsigned int val, unsigned char bits, int range);
    int convertPanSplitToLinearLR(unsigned char left, unsigned char right, int range);
    unsigned int convertPanLinearToSplit(int val, unsigned char bits, int range);

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

    // sample preview query
    bool isPreviewingSample();
    int getSamplePreviewPos();
    double getSamplePreviewRate();

    // trigger sample preview
    void previewSample(int sample, int note=-1, int pStart=-1, int pEnd=-1);
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

    // get current subsong
    size_t getCurrentSubSong();

    // get speed 1
    unsigned char getSpeed1();

    // get speed 2
    unsigned char getSpeed2();

    // get Hz
    float getHz();

    // get current Hz
    float getCurHz();

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

    // is running
    bool isRunning();

    // is stepping
    bool isStepping();

    // is exporting
    bool isExporting();

    // add instrument
    int addInstrument(int refChan=0);

    // add instrument from pointer
    int addInstrumentPtr(DivInstrument* which);

    // get instrument from file
    // if the returned vector is empty then there was an error.
    std::vector<DivInstrument*> instrumentFromFile(const char* path);

    // load temporary instrument
    void loadTempIns(DivInstrument* which);

    // delete instrument
    void delInstrument(int index);

    // add wavetable
    int addWave();

    // add wavetable from pointer
    int addWavePtr(DivWavetable* which);

    // get wavetable from file
    DivWavetable* waveFromFile(const char* path, bool loadRaw=true);

    // delete wavetable
    void delWave(int index);

    // add sample
    int addSample();

    // add sample from pointer
    int addSamplePtr(DivSample* which);

    // get sample from file
    DivSample* sampleFromFile(const char* path);

    // get raw sample
    DivSample* sampleFromFileRaw(const char* path, DivSampleDepth depth, int channels, bool bigEndian, bool unsign);

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

    void autoNoteOn(int chan, int ins, int note, int vol=-1);
    void autoNoteOff(int chan, int note, int vol=-1);
    void autoNoteOffAll();

    // set whether autoNoteIn is mono or poly
    void setAutoNotePoly(bool poly);

    // go to order
    void setOrder(unsigned char order);

    // update system flags
    void updateSysFlags(int system, bool restart);

    // set Hz
    void setSongRate(float hz, bool pal);

    // set remaining loops. -1 means loop forever.
    void setLoops(int loops);

    // get channel state
    DivChannelState* getChanState(int chan);

    // get dispatch channel state
    void* getDispatchChanState(int chan);

    // get register pool
    unsigned char* getRegisterPool(int sys, int& size, int& depth);

    // get macro interpreter
    DivMacroInt* getMacroInt(int chan);

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

    // set the console mode.
    void setConsoleMode(bool enable);

    // get metronome
    bool getMetronome();

    // set metronome
    void setMetronome(bool enable);

    // set metronome volume (1.0 = 100%)
    void setMetronomeVol(float vol);

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
    void renderSamples();

    // public render samples
    void renderSamplesP();

    // public swap channels
    void swapChannelsP(int src, int dest);

    // public change song
    void changeSongP(size_t index);

    // add subsong
    int addSubSong();

    // remove subsong
    bool removeSubSong(int index);

    // move subsong
    void moveSubSongUp(size_t index);
    void moveSubSongDown(size_t index);

    // clear all subsong data
    void clearSubSongs();

    // change system
    void changeSystem(int index, DivSystem which, bool preserveOrder=true);

    // add system
    bool addSystem(DivSystem which);

    // remove system
    bool removeSystem(int index, bool preserveOrder=true);

    // move system
    bool swapSystem(int src, int dest, bool preserveOrder=true);

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
    bool switchMaster();

    // set MIDI base channel
    void setMidiBaseChan(int chan);

    // set MIDI direct channel map
    void setMidiDirect(bool value);

    // set MIDI input callback
    // if the specified function returns -2, note feedback will be inhibited.
    void setMidiCallback(std::function<int(const TAMidiMessage&)> what);

    // send MIDI message
    bool sendMidiMessage(TAMidiMessage& msg);

    // perform secure/sync operation
    void synchronized(const std::function<void()>& what);

    // perform secure/sync song operation
    void lockSave(const std::function<void()>& what);

    // perform secure/sync song operation (and lock audio too)
    void lockEngine(const std::function<void()>& what);

    // get audio desc want
    TAAudioDesc& getAudioDescWant();

    // get audio desc
    TAAudioDesc& getAudioDescGot();

    // init dispatch
    void initDispatch();

    // quit dispatch
    void quitDispatch();

    // initialize the engine.
    bool init();

    // terminate the engine.
    bool quit();

    unsigned char* yrw801ROM;
    unsigned char* tg100ROM;
    unsigned char* mu5ROM;

    DivEngine():
      output(NULL),
      exportThread(NULL),
      chans(0),
      active(false),
      lowQuality(false),
      playing(false),
      freelance(false),
      shallStop(false),
      speedAB(false),
      endOfSong(false),
      consoleMode(false),
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
      lowLatency(false),
      systemsRegistered(false),
      hasLoadedSomething(false),
      midiOutClock(false),
      midiOutMode(DIV_MIDI_MODE_NOTE),
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
      nextSpeed(3),
      curSubSongIndex(0),
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
      globalPitch(0),
      extValue(0),
      speed1(3),
      speed2(3),
      tempoAccum(0),
      view(DIV_STATUS_NOTHING),
      haltOn(DIV_HALT_NONE),
      audioEngine(DIV_AUDIO_NULL),
      exportMode(DIV_EXPORT_MODE_ONE),
      exportFadeOut(0.0),
      midiBaseChan(0),
      midiPoly(true),
      midiAgeCounter(0),
      samp_bb(NULL),
      samp_bbInLen(0),
      samp_temp(0),
      samp_prevSample(0),
      samp_bbIn(NULL),
      samp_bbOut(NULL),
      metroTick(NULL),
      metroTickLen(0),
      metroFreq(0),
      metroPos(0),
      metroAmp(0.0f),
      metroVol(1.0f),
      totalProcessed(0),
      curOrders(NULL),
      curPat(NULL),
      tempIns(NULL),
      oscBuf{NULL,NULL},
      oscSize(1),
      oscReadPos(0),
      oscWritePos(0),
      tickMult(1),
      processTime(0),
      yrw801ROM(NULL),
      tg100ROM(NULL),
      mu5ROM(NULL) {
      memset(isMuted,0,DIV_MAX_CHANS*sizeof(bool));
      memset(keyHit,0,DIV_MAX_CHANS*sizeof(bool));
      memset(dispatchChanOfChan,0,DIV_MAX_CHANS*sizeof(int));
      memset(dispatchOfChan,0,DIV_MAX_CHANS*sizeof(int));
      memset(sysOfChan,0,DIV_MAX_CHANS*sizeof(int));
      memset(vibTable,0,64*sizeof(short));
      memset(reversePitchTable,0,4096*sizeof(int));
      memset(pitchTable,0,4096*sizeof(int));
      memset(sysDefs,0,256*sizeof(void*));
      memset(walked,0,8192);

      for (int i=0; i<256; i++) {
        sysFileMapFur[i]=DIV_SYSTEM_NULL;
        sysFileMapDMF[i]=DIV_SYSTEM_NULL;
      }

      changeSong(0);
    }
};
#endif
