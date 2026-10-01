/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2026 tildearrow and contributors
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
#include "sysDef.h"
#include "cmdStream.h"
#include "filePlayer.h"
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

// when defined, the version string will be watermarked on the GUI (top right corner).
// disable on Furnace stable releases.
#define DIV_UNSTABLE

// version string.
// the usual format for stable versions is 0.major.minor[.patch].
// we're not reaching 1.0 until we have export for all major systems.
//
// development/interim versions go by the format version, prepended with "dev".
#define DIV_VERSION "dev255"
// format version.
// this shall be bumped on each file format/breaking change.
#define DIV_ENGINE_VERSION 255
// for imports
#define DIV_VERSION_MOD 0xff01
#define DIV_VERSION_FC 0xff02
#define DIV_VERSION_S3M 0xff03
#define DIV_VERSION_FTM 0xff04
#define DIV_VERSION_TFE 0xff05
#define DIV_VERSION_XM 0xff06
#define DIV_VERSION_IT 0xff07
#define DIV_VERSION_MIDI 0xff08

/**
 * used by the -view flag in command line player.
 */
enum DivStatusView {
  // don't display anything
  DIV_STATUS_NOTHING=0,
  // print the pattern
  DIV_STATUS_PATTERN,
  // print dispatched commands
  DIV_STATUS_COMMANDS
};

/**
 * if you add an audio engine to TAAudio/Furnace, define it here.
 */
enum DivAudioEngines {
  DIV_AUDIO_JACK=0,
  DIV_AUDIO_SDL=1,
  DIV_AUDIO_PORTAUDIO=2,
  DIV_AUDIO_PIPE=3,
  DIV_AUDIO_ASIO=4,

  // these two are special. don't touch them.
  DIV_AUDIO_NULL=126,
  DIV_AUDIO_DUMMY=127
};

/**
 * audio export modes.
 */
enum DivAudioExportModes {
  // export to a single file.
  DIV_EXPORT_MODE_ONE=0,
  // export to multiple files (one per chip).
  DIV_EXPORT_MODE_MANY_SYS,
  // export to multiple files (one per channel).
  DIV_EXPORT_MODE_MANY_CHAN
};

/**
 * the engine is capable of "halting" (a debug feature which pauses playback).
 * this allows you to set when to halt.
 */
enum DivHaltPositions {
  // normal engine operation.
  DIV_HALT_NONE=0,
  // halt on the next tick.
  DIV_HALT_TICK,
  // halt on the next row.
  DIV_HALT_ROW,
  // halt on the next order.
  DIV_HALT_PATTERN,
  // halt on a user-specified breakpoint.
  DIV_HALT_BREAKPOINT
};

/**
 * MIDI output modes.
 */
enum DivMIDIModes {
  // no output - e.g. TX81Z
  DIV_MIDI_MODE_OFF=0,
  // send notes
  DIV_MIDI_MODE_NOTE,
  // this is a remnant of an experiment with a Launchpad.
  DIV_MIDI_MODE_LIGHT_SHOW
};

/**
 * define audio export formats here.
 */
enum DivAudioExportFormats {
  DIV_EXPORT_FORMAT_WAV=0,
  DIV_EXPORT_FORMAT_OPUS,
  DIV_EXPORT_FORMAT_FLAC,
  DIV_EXPORT_FORMAT_VORBIS,
  DIV_EXPORT_FORMAT_MPEG_L3 // MPEG Layer 3 (MP3)
};

/**
 * used by MP3 format.
 */
enum DivAudioExportBitrateModes {
  DIV_EXPORT_BITRATE_CONSTANT=0,
  DIV_EXPORT_BITRATE_VARIABLE,
  DIV_EXPORT_BITRATE_AVERAGE,
};

/**
 * used by WAV format.
 */
enum DivAudioExportWavFormats {
  DIV_EXPORT_WAV_U8=0,
  DIV_EXPORT_WAV_S16,
  DIV_EXPORT_WAV_F32
};

/**
 * this struct encapsulates options for audio export. it is passed to DivEngine::saveAudio().
 */
struct DivAudioExportOptions {
  DivAudioExportModes mode;
  DivAudioExportFormats format;
  DivAudioExportBitrateModes bitRateMode;
  DivAudioExportWavFormats wavFormat;
  int sampleRate;
  // number of channels in the outupt file.
  // only takes effect in single file or per-channel export.
  int chans;
  int loops;
  double fadeOut;
  // this hasn't been implemented yet!
  int orderBegin, orderEnd;
  // set which channels are going to be exported.
  bool channelMask[DIV_MAX_CHANS];
  // bit rate (in bits/second).
  int bitRate;
  // range is 0.0-10.0 if I remember correctly.
  float vbrQuality;
  DivAudioExportOptions():
    mode(DIV_EXPORT_MODE_ONE),
    format(DIV_EXPORT_FORMAT_WAV),
    bitRateMode(DIV_EXPORT_BITRATE_CONSTANT),
    wavFormat(DIV_EXPORT_WAV_S16),
    sampleRate(44100),
    chans(2),
    loops(0),
    fadeOut(0.0),
    orderBegin(-1),
    orderEnd(-1),
    bitRate(128000),
    vbrQuality(6.0f) {
    for (int i=0; i<DIV_MAX_CHANS; i++) {
      channelMask[i]=true;
    }
  }
};

#ifdef WITH_JSON
struct DivJSONExportOptions {
  enum ExportFormat: unsigned char {
    EXPORT_JSON,
    EXPORT_BSON,
    EXPORT_CBOR
  };
  ExportFormat format;
  bool jsonPretty;
  bool exportMetadata, exportChips, exportOrders, exportPatterns, exportInstruments, exportWaves, exportSamples, exportCompatFlags;
  bool optimizePatterns;
  DivJSONExportOptions():
    format(EXPORT_JSON),
    jsonPretty(false),
    exportMetadata(true),
    exportChips(true),
    exportOrders(true),
    exportPatterns(true),
    exportInstruments(true),
    exportWaves(true),
    exportSamples(true),
    exportCompatFlags(false),
    optimizePatterns(true) {}
};
#endif

/**
 * this struct contains playback state for a channel.
 */
struct DivChannelState {
  // currently unused. not sure why it is here.
  std::vector<DivDelayedCommand> delayed;
  // note: the current note, from 0 (C-(-5)) to 179 (B-9).
  // oldNote: the previous note.
  // lastIns: the current instrument. used to prevent duplicate notes during note input.
  // pitch: the current pitch effect's value (E5xx).
  // portaSpeed: slide/portamento speed, in pitch units per tick.
  // portaNote: the slide/portamento target.
  int note, oldNote, lastIns, pitch, portaSpeed, portaNote;
  // volume: 8.8 fixed point number representing channel volume. the integer part is sent to dispatch.
  // volSpeed: volume slide speed, in fractional units per tick.
  // volSpeedTarget: volume slide target. if this is -1, the slide won't stop until it reaches min/max volume.
  // cut: number of remaining ticks for a note cut.
  // volCut: number of remaining ticks for a volume cut (volume set to 0).
  // legatoDelay: how many ticks remain before a quick legato.
  // legatoTarget: quick legato's target note.
  // rowDelay: number of ticks before this row is executed (EDxx effect).
  // volMax: maximum volume of this channel (8.8 fixed point).
  int volume, volSpeed, volSpeedTarget, cut, volCut, legatoDelay, legatoTarget, rowDelay, volMax;
  // delayOrder/delayRow: the order/row to be executed after rowDelay.
  // - this exists because an EDxx effect may exceed the current speed.
  // retrigSpeed: retrigger speed.
  // retrigTick: number of ticks before retrigger.
  int delayOrder, delayRow, retrigSpeed, retrigTick;
  // vibratoDepth: vibrato depth. 15 should be ±1 semitone when vibratoFine is 15.
  // vibratoRate: vibrato rate. one vibrato cycle has a duration of 64 ticks on rate 1.
  // vibratoPos: position in current vibrato cycle.
  // vibratoPosGiant: this one has a period of 512. it is used by the GUI pattern visualizer.
  // vibratoShape: current vibrato shape. this may be one of the following:
  // - 0: sine
  // - 1: sine (up only)
  // - 2: sine (down only)
  // - 3: triangle
  // - 4: ramp up
  // - 5: ramp down
  // - 6: square
  // - 7: random
  // - 8: square up
  // - 9: square down
  // - 10: half sine up
  // - 11: half sine donw
  // vibratoFine: sets the vibrato range. 15 is ±1 semitone at depth 15.
  int vibratoDepth, vibratoRate, vibratoPos, vibratoPosGiant, vibratoShape, vibratoFine;
  // tremoloDepth: depth of tremolo effect. ±128 volume units at depth 15... I think.
  // tremoloRate: tremolo effect rate. one tremolo cycle has a duration of 128 ticks on rate 1.
  // tremoloPos: position in current tremolo cycle.
  int tremoloDepth, tremoloRate, tremoloPos;
  // panDepth: panbrello depth (15 is full left-right).
  // panRate: panbrello rate. one cycle is 256 ticks long at rate 1.
  // panPos: position in current panbrello cycle.
  // panSpeed: pan slide speed. negative is left and positive is right.
  // - panbrello and pan slides may not occur simultaneously.
  int panDepth, panRate, panPos, panSpeed;
  // sample position effects are accumulated here and dispatched after scanning all effects in a row.
  int sampleOff;
  // arp: current arpeggio value.
  // arpStage: current arpeggio note (0: note; 1: note+x; 2: note+y).
  // arpTicks: number of ticks before next arp stage.
  // arpSpeed: current arpeggio speed.
  // panL: left panning.
  // panR: right panning.
  // panRL: left panning (rear).
  // panRR: right panning (rear).
  // lastVibrato: stores the last value of a vibrato effect. it is recalled on a vibrato + vol slide effect.
  // lastPorta: same thing but for portamento. stores the last speed.
  // cutType: ECxx effect type. one of the following:
  // - 0: note off
  // - 1: note release
  // - 2: macro release
  unsigned char arp, arpStage, arpTicks, arpSpeed, panL, panR, panRL, panRR, lastVibrato, lastPorta, cutType;
  // doNote: whether a note is going to occur.
  // legato: whether legato (EAxx) is enabled.
  // portaStop: a compatibility thing.
  // keyOn: note on state.
  // keyOff: note off state.
  // stopOnOff: a compatibility thing.
  // releasing: whether a note release/macro release has occurred.
  bool doNote, legato, portaStop, keyOn, keyOff, stopOnOff, releasing;
  // arpYield: another compatibility thing...
  // delayLocked: oh man
  bool arpYield, delayLocked, inPorta, scheduledSlideReset, shorthandPorta, wasShorthandPorta, noteOnInhibit, resetArp, sampleOffSet;
  // wentThroughNote: whether a note has played on this channel. resets on loop.
  // goneThroughNote: same as wentThroughNote, but doesn't reset on loop.
  // - these two are used to determine loop trail length.
  bool wentThroughNote, goneThroughNote;

  // MIDI state variables.
  int midiNote, curMidiNote, midiPitch;
  size_t midiAge;
  bool midiAftertouch;

  DivChannelState():
    note(-1),
    oldNote(-1),
    lastIns(-1),
    pitch(0),
    portaSpeed(-1),
    portaNote(0),
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
    arpSpeed(1),
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

/**
 * a note preview event.
 */
struct DivNoteEvent {
  signed char channel;
  short ins;
  // we can't save space anymore now that raw notes exist.
  int note;
  // velocity. if set to -1, there isn't.
  signed char volume;
  // on: whether it's a key on event or a key off one.
  // nop: if set, disregard this event.
  // insChange: whether we have an instrument change.
  // fromMIDI: whether the event was caused by MIDI input.
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

/**
 * a DivDispatchContainer handles the audio output portion of a dispatch.
 * it performs resampling as the engine's audio output rate usually differs from that of dispatches.
 */
struct DivDispatchContainer {
  // the dispatch.
  DivDispatch* dispatch;
  // BLIP buffers.
  // the dispatch container will feed them on acquire(), or deliver these to the dispatch if it supports acquireDirect().
  blip_buffer_t* bb[DIV_MAX_OUTPUTS];
  // bbInLen: size of BLIP buffers.
  // runtotal: no longer used.
  // runLeft: no longer used.
  // runPos: no longer used.
  // lastAvail: no longer used. why is it here?
  size_t bbInLen, runtotal, runLeft, runPos, lastAvail;
  // temp: stores the current sample for delta calculation.
  // prevSample: stores the previous sample for delta calculation.
  int temp[DIV_MAX_OUTPUTS], prevSample[DIV_MAX_OUTPUTS];
  // this is the same as bbIn, but has null pointers on unallocated outputs.
  short* bbInMapped[DIV_MAX_OUTPUTS];
  // allocates buffers for DivDispatch::acquire().
  short* bbIn[DIV_MAX_OUTPUTS];
  // allocates buffers for resampled output (using blip_buf).
  short* bbOut[DIV_MAX_OUTPUTS];
  // lowQuality: use fast interpolation (blip_add_delta_fast).
  // dcOffCompensation: getDCOffRequired(). if set, the first sample must be considered as DC offset to avoid clicks on playback start.
  // hiPass: enable a low-frequency high-pass filter to remove DC offset. set in Furnace settings.
  bool lowQuality, dcOffCompensation, hiPass;
  // the last dispatch's rate, used on audio output rate changes.
  double rateMemory;

  // used in multi-thread
  int cycles;
  unsigned int size;

  /**
   * tell this DivDispatchContainer that the output rate has changed and resampling ratio shall be recomputed.
   * @param gotRate the new rate.
   */
  void setRates(double gotRate);
  /**
   * set output quality options.
   * @param lowQual enable low-quality mode.
   * @param dcHiPass set the DC offset compensation option.
   */
  void setQuality(bool lowQual, bool dcHiPass);
  /**
   * re-allocate input/output buffers to fit a requested size.
   * @param size size.
   */
  void grow(size_t size);
  /**
   * call the dispatch's acquire function.
   * alters bbIn unless acquireDirect() is supported.
   * this does not fill output buffers! call fillBuf() after this function.
   * @param count number of requested samples (in dispatch output rate).
   */
  void acquire(size_t count);
  /**
   * flush the output, if any samples remain.
   * read bbOut afterwards.
   * @param offset offset within the output buffer (in engine output rate).
   * @param count number of samples to flush (in engine output rate).
   */
  void flush(size_t offset, size_t count);
  /**
   * resample and populate bbOut.
   * read bbOut afterwards.
   * @param runtotal number of samples in dispatch output rate.
   * @param offset position within bbOut.
   * @param size number of samples to output (in engine output rate).
   */
  void fillBuf(size_t runtotal, size_t offset, size_t size);
  /**
   * empty all buffers and reset blip_buf state.
   */
  void clear();
  /**
   * initialize this DivDispatchContainer.
   * allocates a DivDispatch for a specific chip, binds an engine and sets the resampler up.
   *
   * when adding a new chip, make sure to update this function so it allocates your DivDispatch.
   * @param sys the chip type.
   * @param eng the DivEngine.
   * @param chanCount requested chip channel count (dynamic channel count systems only).
   * @param gotRate the engine output rate.
   * @param flags a reference to a DivConfig containing chip-specific configuration.
   * @param isRender set during audio export. used to select between playback and render cores.
   */
  void init(DivSystem sys, DivEngine* eng, int chanCount, double gotRate, const DivConfig& flags, bool isRender=false);
  /**
   * de-initialize the DivDispatch and this container's state.
   */
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

/**
 * container for an audio effect.
 * a feature that may be implemented in the future.
 */
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

/**
 * options for MIDI import.
 */
struct DivMIDIImportOptions {
  bool useBaseTempo;
  bool importVelocity;
  bool importCC7;
  bool importCC11;
  bool importSustain;
  bool importPan;
  bool importVibrato;
  bool importPitchBend;
  bool splitDrums;
  int quantize;
  int ticksPerRow;
  int patternLen;
  int drumChannel;
  int vibratoRate;
  int vibratoDepth;
  int bendRange;
  DivMIDIImportOptions():
    useBaseTempo(true),
    importVelocity(true),
    importCC7(true),
    importCC11(true),
    importSustain(true),
    importPan(true),
    importVibrato(true),
    importPitchBend(true),
    splitDrums(true),
    quantize(32),
    ticksPerRow(6),
    patternLen(64),
    drumChannel(10),
    vibratoRate(5),
    vibratoDepth(8),
    bendRange(0) {}
};

// command names (this array must be updated in playback.cpp when adding new commands).
extern const char* cmdName[];

/**
 * this is the Furnace engine.
 * it handles file operations, song playback, audio output and more.
 *
 * the Div prefix in these definitions comes from Divorce, the original name of Furnace back in April 2021.
 *
 * Furnace's architecture is mostly monolithic. most components depends on a DivEngine.
 * it is briefly described here.
 *
 * DivEngine (the Furnace engine)
 * - DivSong
 *   - assets (DivInstrument/DivWavetable/DivSample)
 * - DivDispatchContainer
 *   - DivDispatch
 * - audio output
 * - MIDI input
 * - MIDI output
 * - configuration
 * - DivChannelState
 * - playback engine
 * - DivCSPlayer (reference command stream player)
 *
 * I will make a graphical tree soon...
 *
 * currently, it is not possible to have multiple engines running simultaneously. it leads to a bunch of conflicts and anomalies.
 */
class DivEngine {
  // instances of DivDispatchContainer.
  DivDispatchContainer disCont[DIV_MAX_CHIPS];
  // audio output backend.
  TAAudio* output;
  // want: the requested audio output configuration.
  // got: the output configuration that is actually being used.
  TAAudioDesc want, got;
  // audio export path. stored because audio export runs in a thread and may need to reference the name later (e.g. multiple file export).
  String exportPath;
  // the export thread.
  std::thread* exportThread;
  // set by prePreInit() after the config file has been loaded.
  bool configLoaded;
  // whether the engine has been initialized.
  bool active;
  // config setting "audioQuality".
  bool lowQuality;
  // config setting "audioHiPass".
  bool dcHiPass;
  // despite its name, this is set whenever the playback engine should be running.
  bool playing;
  // "jam mode" - when set, the playback engine isn't going to read the pattern.
  // the variable `playing` must be set for this to work.
  bool freelance;
  // shallStop: whether a stop effect (FFxx) has been issued and we have to stop.
  // shallStopSched: stop effects are scheduled so it stops after the row is done (rather than immediately after hitting a stop effect).
  bool shallStop, shallStopSched;
  // set when the song ends or loops.
  bool endOfSong;
  // activated with the -console command line parameter.
  bool consoleMode;
  // activated with the -nostatus command line parameter.
  // requires consoleMode.
  bool disableStatusOut;
  // set when an EExx effect occurs.
  bool extValuePresent;
  // internally used by playSub().
  bool repeatPattern;
  // enables a metronome which triggers on each row highlight.
  bool metronome;
  // set if we're exporting audio.
  std::atomic<bool> exporting;
  // requests the export thread to abort.
  bool stopExport;
  // completely stops all audio output (debug feature).
  bool halted;
  // downmix all channels to mono (accessibility).
  bool forceMono;
  // soft-clip to work around issues with certain audio output drivers.
  bool clampSamples;
  // dump all commands to the cmdStream vector.
  // used by the GUI pattern visualizer and command stream export.
  bool cmdStreamEnabled;
  // whether busy lock mutex is "soft-locked". in this case, the audio thread outputs silence instead of waiting.
  bool softLocked;
  // set during the first tick of a row (for a compatibility flag).
  bool firstTick;
  // whether we are "skipping" (happens during playSub()).
  bool skipping;
  // direct mode outputs channels 0-15 as is to the MIDI output device.
  bool midiIsDirect;
  // whether instrument changes should also be output as program changes.
  bool midiIsDirectProgram;
  // low-latency mode runs the engine at ~1000Hz and mitigates delay in note preview events.
  bool lowLatency;
  // set once registerSystems() has been called.
  bool systemsRegistered;
  // set once registerROMExports() has been called.
  bool romExportsRegistered;
  // it is possible to load a file without fully initializing the engine.
  // this happens when passing a file path in the command line.
  // if this is set, the engine won't attempt to initialize the song with the default system.
  bool hasLoadedSomething;
  // MIDI output state.
  bool midiOutClock;
  bool midiOutTime;
  bool midiOutProgramChange;
  // MIDI output mode. see the DivMIDIModes enum.
  int midiOutMode;
  int midiOutTimeRate;
  float midiVolExp;
  // used to keep track of audio frames elapsed while the busy mutex is soft-locked.
  int softLockCount;
  // subticks: set in low-latency mode. number of sub-ticks before next tick.
  // ticks: number of ticks before next row.
  // curRow: the current row. it usually is one row ahead of the currently playing row.
  // curOrder: same, but for order.
  // prevRow: the actually playing row.
  // prevOrder: the actually playing order.
  // remainingLoops: number of loops before playback stops (set with the -loops command line parameter).
  // totalLoops: number of elapsed loops.
  // lastLoopPos: last position where a loop occurred within the current audio output frame. used to keep audio output length accurate.
  // exportLoopCount: number of loops before audio export finishes.
  // curExportChan: the current channel being exported.
  // nextSpeed: the next speed (in ticks/row).
  // prevSpeed: the currently used speed.
  // elapsedBars: number of elapsed highlight 2 cycles, for the GUI clock.
  // elapsedBeatS: same but for highlight 1.
  // curSpeed: the current speed index.
  int subticks, ticks, curRow, curOrder, prevRow, prevOrder, remainingLoops, totalLoops, lastLoopPos, exportLoopCount, curExportChan, nextSpeed, prevSpeed, elapsedBars, elapsedBeats, curSpeed;
  // the current sub-song index.
  size_t curSubSongIndex;
  // position in current audio frame. used in the audio output callback.
  size_t bufferPos;
  // the tick rate.
  double divider;
  // number of samples before next tick (in engine output rate).
  int cycles;
  // fractional part of the above.
  double clockDrift;
  // number of samples before next MIDI beat clock.
  int midiClockCycles;
  double midiClockDrift;
  // number of samples before next MIDI timecode part.
  int midiTimeCycles;
  double midiTimeDrift;
  // current step play state.
  // - 0: step play is disabled
  // - 1: step play enabled - waiting for next step
  // - 2: step play enabled - next step pending
  int stepPlay;
  // changeOrd: jump to this order after the current row (if not -1).
  // - -2 means "next order".
  // changePos: jump to this row after the current row. changeOrd must be set.
  // totalTicksR: elapsed ticks during playback 
  // curMidiClock: the current MIDI beat clock.
  // curMidiTime: the current MIDI timecode (in frames).
  // totalCmds: how many commands we dispatched so far 
  // lastCmds: the previous value of totalCmds, for...
  // cmdsPerSecond: the command rate, in commands per second.
  int changeOrd, changePos, totalTicksR, curMidiClock, curMidiTime, totalCmds, lastCmds, cmdsPerSecond;
  // current playback time.
  TimeMicros totalTime;
  // fraction of microseconds in playback time.
  double totalTimeDrift;
  // curMidiTimePiece: which byte of MIDI timecode to produce.
  // curMidiTimeCode: current MIDI time code byte.
  int curMidiTimePiece, curMidiTimeCode;
  // extValue: last value of EExx effect.
  // pendingMetroTick: whether the metronome must click. one of the following:
  // - 0: nothing
  // - 1: beat
  // - 2: bar
  unsigned char extValue, pendingMetroTick;
  // the current groove pattern/speed set.
  DivGroovePattern speeds;
  // current virtual tempo numerator/denominator.
  short virtualTempoN, virtualTempoD;
  // virtual tempo accumulator.
  // on each tick, the numerator is added.
  // while it exceeds or meets the denominator, the tick counter is decreased.
  short tempoAccum;
  // current console status view mode.
  DivStatusView view;
  // when to halt (pause playback).
  DivHaltPositions haltOn;
  // playback state for each channel.
  DivChannelState chan[DIV_MAX_CHANS];
  // the current audio backend.
  DivAudioEngines audioEngine;
  // audio export options currently in use.
  DivAudioExportModes exportMode;
  DivAudioExportFormats exportFormat;
  DivAudioExportWavFormats wavFormat;
  DivAudioExportBitrateModes exportBitRateMode;
  // stores the engine's previous output rate.
  // restored after audio export.
  double prevAudioRate;
  double exportFadeOut;
  bool isFadingOut;
  int exportOutputs;
  int exportBitRate;
  float exportVBRQuality;
  bool exportChannelMask[DIV_MAX_CHANS];
  // the current Furnace config is loaded here.
  // use the getConf*() functions to access it, or getConfObject() if you really need the entire config.
  DivConfig conf;
  // note event queue
  FixedQueue<DivNoteEvent,8192> pendingNotes;
  // a bitfield which keeps track of the rows we've "walked" on.
  // used to determine loop point.
  // 256 orders × 256 rows = 65536 bits = 8192 bytes
  unsigned char walked[8192];
  // stores which chsnnels are muted.
  bool isMuted[DIV_MAX_CHANS];
  // isBusy: general busy lock, used by the audio engine.
  // saveLock: save lock, used to prevent concurrent saves (e.g. backup thread).
  // playPosLock: taken when the playback position is changing.
  std::mutex isBusy, saveLock, playPosLock;
  // path to config directory. usually one of the following:
  // - Windows: %USERPROFILE%\AppData\Roaming\furnace
  // - macOS: ~/Library/Application Support/Furnace
  // - Linux/other: ~/.config/furnace
  String configPath;
  // parh to config file.
  String configFile;
  // information about last error.
  String lastError;
  // displays information to the user after an operation (e.g. exporting or loading a file).
  // use the addWarning() macro to append warnings!
  String warnings;
  // list of available audio devices.
  // call rescanAudioDeices() to populate it again.
  std::vector<String> audioDevs;
  // list of MIDI input/output devices.
  // call rescanMidiDevices() to populate it again.
  std::vector<String> midiIns;
  std::vector<String> midiOuts;
  // a dump of all dispatched commands.
  // set cmdStreamEnabled to enable command dumping.
  std::vector<DivCommand> cmdStream;
  // audio effects. not implemented yet.
  std::vector<DivEffectContainer> effectInst;
  // the initial system's channel mask.
  std::vector<int> curChanMask;
  // system definitions.
  // registered in registerSystems(), called by preInit().
  static DivSysDef* sysDefs[DIV_SYSTEM_MAX+1];
  // registered in registerROMExports(), called by preInit().
  static DivROMExportDef* romExportDefs[DIV_ROM_MAX];
  // the current command stream player. NULL if not loaded.
  DivCSPlayer* cmdStreamInt;

  // sample preview state.
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

  // a sine table with range ±127.
  short vibTable[64];
  // a cosine table with range 0-255. I believe.
  short tremTable[128];
  // for audio effects. currently unused.
  short effectSlotMap[4096];
  // MIDI base channel. used during note preview when a channel is not specified.
  int midiBaseChan;
  // polyphonic MIDI note preview.
  bool midiPoly;
  // debug MIDI messages.
  bool midiDebug;
  // used to keep track of the oldest active channel.
  size_t midiAgeCounter;

  // sample preview state.
  blip_buffer_t* samp_bb;
  size_t samp_bbInLen;
  int samp_temp, samp_prevSample;
  short* samp_bbIn;
  short* samp_bbOut;

  // an array which stores where do metronome ticks occur within an audio output frame.
  unsigned char* metroTick;
  // size of the metroTick array.
  size_t metroTickLen;
  // metronome output buffer.
  float* metroBuf;
  size_t metroBufLen;
  float metroFreq, metroPos;
  float metroAmp;
  float metroVol;
  float previewVol;

  // file player output buffer.
  float* filePlayerBuf[DIV_MAX_OUTPUTS];
  size_t filePlayerBufLen;
  // an audio file player instance.
  DivFilePlayer* curFilePlayer;
  // whether the file player should be synchronized to tracker playback.
  bool filePlayerSync;
  // file player cue (start) point.
  TimeMicros filePlayerCue;
  // unused...
  int filePlayerLoopTrail;
  int curFilePlayerTrail;

  // number of samples actually present in the audio output frame.
  size_t totalProcessed;

  // how many threads to use in the render pool.
  unsigned int renderPoolThreads;
  // the render pool runs one thread per dispatch during audio output.
  DivWorkPool* renderPool;

  // MIDI stuff
  // this function provides a mechanism to filter MIDI input messages before they are added to the note preview queue.
  // the function should return an instrument index, which will be used
  // for all forthcoming notes.
  // special values:
  // - -1: don't change
  // - -2: "preview" instrument
  // - -3: cancel event (do not add to pending notes)
  std::function<int(const TAMidiMessage&)> midiCallback=[](const TAMidiMessage&) -> int {return -3;};

  /**
   * INTERNAL ENGINE FUNCTIONS
   *
   * most of these should not be called directly.
   */

  /**
   * called by nextRow() before calling processRow().
   * executes chip pre-effects.
   * @param i the channel to process.
   */
  void processRowPre(int i);
  /**
   * called by nextRow() and nextTick() (after EDxx).
   * processes a channel's cell. notes, instruments, volumes and effects.
   * @param i the channel to process.
   * @param afterDelay must be set to true if this is being called from nextTick(). this function will bail out early if there is an EDxx effect.
   */
  void processRow(int i, bool afterDelay);
  /**
   * jump to the next order, or to an scheduled order (see 0Bxx and 0Dxx effects).
   * called by nextRow().
   */
  void nextOrder();
  /**
   * process the next row.
   * called by nextTick().
   */
  void nextRow();
  /**
   * process the next tick.
   * @param noAccum set during playSub() and "reset" loop modality, ensuring the seek process does not alter the song playback time.
   * @param inhibitLowLat when set, low-latency mode is disregarded. you must set this to true if calling outside nextBuf() (e.g. ROM export).
   * @return whether we reached end of song.
   */
  bool nextTick(bool noAccum=false, bool inhibitLowLat=false);

  /**
   * process per-chip effects.
   * finds an effect, and attempts to execute it.
   * @param ch channel.
   * @param effect effect.
   * @param effectVal effect value.
   * @return whether a chip effect was found and successfully executed. if not, proceed with normal effects.
   */
  bool perSystemEffect(int ch, unsigned char effect, unsigned char effectVal);
  /**
   * process per-chip post-effects.
   * this happens after notes in processRow().
   * @param ch channel.
   * @param effect effect.
   * @param effectVal effect value.
   * @return whether a chip effect was found and successfully executed.
   */
  bool perSystemPostEffect(int ch, unsigned char effect, unsigned char effectVal);
  /**
   * process per-chip pre-effects.
   * this is called by processRowPre().
   * @param ch channel.
   * @param effect effect.
   * @param effectVal effect value.
   * @return whether a chip effect was found and successfully executed.
   */
  bool perSystemPreEffect(int ch, unsigned char effect, unsigned char effectVal);
  /**
   * reset dispatches, song speeds and channel state.
   * this does not stop playback!
   */
  void reset();
  /**
   * this function handles seeking to the current order/row.
   * called during play(), and after end of song in the "reset" loop modality.
   * @param preserveDrift preserves all timings. set to true when handling "reset" loop modality.
   * @param goalRow specify a row to seek to.
   */
  void playSub(bool preserveDrift, int goalRow=0);
  /**
   * runs MIDI beat clock.
   * @param totalCycles how many output samples to run for.
   */
  void runMidiClock(int totalCycles=1);
  /**
   * runs MIDI timecode.
   * @param totalCycles how many output samples to run for.
   */
  void runMidiTime(int totalCycles=1);
  /**
   * currently returns true.
   * @return true.
   */
  bool shallSwitchCores();

  /**
   * this function is a mess.
   * it takes a zillion arguments and writes VGM writes to a SafeWriter.
   *
   * @param SafeWriter pointer to the target SafeWriter.
   * @param sys the chip type.
   * @param write the register write.
   * @param streamOff what?
   */
  void performVGMWrite(SafeWriter* w, DivSystem sys, DivRegWrite& write, int streamOff, double* loopTimer, double* loopFreq, int* loopSample, bool* sampleDir, bool isSecond, int* pendingFreq, int* playingSample, int* setPos, unsigned int* sampleOff8, unsigned int* sampleLen8, size_t bankOffset, bool directStream, bool* sampleStoppable, bool dpcm07, DivDispatch** writeNES, int rateCorrection);

  /**
   * hello, world!
   */
  void testFunction();

  /**
   * these functions are called by load().
   * these load different file formats.
   * @param file a pointer to file data.
   * @param len the file size.
   * @return whether loading was successful. if so, the current DivSong is replaced, playback is stopped and dispatches are initialized.
   */
  bool loadDMF(unsigned char* file, size_t len);
  /**
   * @param variantID Furnace variant ID (to accommodate certain Furnace forks).
   */
  bool loadFur(unsigned char* file, size_t len, int variantID=0);
  bool loadMod(unsigned char* file, size_t len);
  bool loadS3M(unsigned char* file, size_t len);
  bool loadXM(unsigned char* file, size_t len);
  bool loadIT(unsigned char* file, size_t len);
  /**
   * @param dnft set if this is a Dn-FamiTracker module.
   * @param dnftSig set if the module has a Dn-FamiTracker magic/signature.
   * @param eft set if this is an E-FamiTracker module.
   */
  bool loadFTM(unsigned char* file, size_t len, bool dnft, bool dnftSig, bool eft);
  bool loadFC(unsigned char* file, size_t len);
  bool loadTFMv1(unsigned char* file, size_t len);
  bool loadTFMv2(unsigned char* file, size_t len);
  bool loadMIDI(unsigned char* file, size_t len);

  /**
   * these functions load various instrument formats.
   * @param reader a SafeReader to the instrument.
   * @param ret where to place loaded instrument(s).
   * @param stripPath the file name (stripped from the path). this is used to set the instrument name if not store in the instrument.
   */
  void loadDMP(SafeReader& reader, std::vector<DivInstrument*>& ret, String& stripPath);
  void loadTFI(SafeReader& reader, std::vector<DivInstrument*>& ret, String& stripPath);
  void loadVGI(SafeReader& reader, std::vector<DivInstrument*>& ret, String& stripPath);
  void loadEIF(SafeReader& reader, std::vector<DivInstrument*>& ret, String& stripPath);
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

  /**
   * these functions load sample banks.
   * @param reader a SafeReader to the sample bank.
   * @param ret where to place loaded samples.
   * @param stripPath the file name (stripped from the path). this is used as sample names.
   */
  void loadP(SafeReader& reader, std::vector<DivSample*>& ret, String& stripPath);
  void loadPPC(SafeReader& reader, std::vector<DivSample*>& ret, String& stripPath);
  void loadPPS(SafeReader& reader, std::vector<DivSample*>& ret, String& stripPath);
  void loadPVI(SafeReader& reader, std::vector<DivSample*>& ret, String& stripPath);
  void loadPDX(SafeReader& reader, std::vector<DivSample*>& ret, String& stripPath);
  void loadPZI(SafeReader& reader, std::vector<DivSample*>& ret, String& stripPath);
  void loadP86(SafeReader& reader, std::vector<DivSample*>& ret, String& stripPath);

  /**
   * load a sample ROM. currently unused.
   * @param path path to data.
   * @param expectedSize the ROM's size.
   * @param ret ROM will be allocated and this pointer will be set.
   * @return 0 on success or -1 otherwise.
   */
  int loadSampleROM(String path, ssize_t expectedSize, unsigned char*& ret);

  /**
   * initialize the currently selected audio backend.
   * @return whether initialization was successful.
   */
  bool initAudioBackend();
  /**
   * de-initialize the audio backend and clear the audio backend selection.
   * @param dueToSwitchMaster if set, don't reset the audio backend. called on switchMaster().
   * @return whether it was successful.
   */
  bool deinitAudioBackend(bool dueToSwitchMaster=false);

  /**
   * registers all chip definitions.
   * see sysDef.cpp for these.
   */
  void registerSystems();
  /**
   * registers all ROM export definitions.
   * see exportDef.cpp for these.
   */
  void registerROMExports();
  /**
   * initialize the current song with a specified system preset.
   * @param description the system preset.
   * @param inBase64 whether description is a Base64-encoded string.
   * @param oldVol compatibility option that uses old volume/panning range, back when it wasn't a floating point number.
   */
  void initSongWithDesc(const char* description, bool inBase64=true, bool oldVol=false);

  /**
   * swap the index of two instruments in all patterns..
   * called after moving an instrument.
   * @param one the first instrument.
   * @param two the second instrument.
   */
  void exchangeIns(int one, int two);
  /**
   * TODO: this.
   * @param one the first wavetable.
   * @param two the second wavetable.
   */
  void exchangeWave(int one, int two);
  /**
   * swap the index of two samples in all instruments.
   * called after moving a sample.
   * @param one the first sample.
   * @param two the second sample.
   */
  void exchangeSample(int one, int two);

  /**
   * copy a channel's contents to another.
   * @param src the source channel.
   * @param dest the destination channel.
   */
  void copyChannel(int src, int dest);
  /**
   * swap the contents of two channels.
   * @param src the first channel.
   * @param dest the second channel.
   */
  void swapChannels(int src, int dest);
  /**
   * destroy the contents of a channel.
   * @param ch the channel to wipe.
   */
  void stompChannel(int ch);
  /**
   * handle a change in chip channel count (e.g. by adding/changing/removing chips).
   * @param firstChan the first channel of the affected chip.
   * @param before the previous channel count.
   * @param after the new channel count.
   * @return whether we could. usually this fails when the change would exceed the channel limit.
   */
  bool sysChanCountChange(int firstChan, int before, int after);

  // recalculate patchbay (UNSAFE)
  void recalcPatchbay();

  // change song (UNSAFE)
  void changeSong(size_t songIndex);

  // convert legacy sample mode to normal
  // returns whether conversion occurred
  bool convertLegacySampleMode();

  void swapSystemUnsafe(int src, int dest, bool preserveOrder=true);

  // add every export method here
  friend class DivROMExport;
  friend class DivExportAmigaValidation;
  friend class DivExportS98;
  friend class DivExportSAPR;
  friend class DivExportTiuna;
  friend class DivExportZSM;
  friend class DivExportiPod;
  friend class DivExportGRUB;

  public:
    DivSong song;
    DivOrders* curOrders;
    DivChannelData* curPat;
    DivSubSong* curSubSong;
    DivInstrument* tempIns;
    bool keyHit[DIV_MAX_CHANS];
    float* oscBuf[DIV_MAX_OUTPUTS];
    float oscSize;
    int oscReadPos, oscWritePos;
    int tickMult;
    int lastNBIns, lastNBOuts, lastNBSize;
    std::atomic<size_t> processTime;

    float chipPeak[DIV_MAX_CHIPS][DIV_MAX_OUTPUTS];

    // ugh...
    DivMIDIImportOptions midiImportOptions;

    void runExportThread();
    void nextBuf(float** in, float** out, int inChans, int outChans, unsigned int size, bool calledFromExport=false);
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

    // get the audio file player.
    DivFilePlayer* getFilePlayer();
    // get whether the player is synchronized with song playback.
    bool getFilePlayerSync();
    void setFilePlayerSync(bool doSync);
    // get/set file player cue position.
    TimeMicros getFilePlayerCue();
    void setFilePlayerCue(TimeMicros cue);
    // UNSAFE - sync file player to current playback position.
    void syncFilePlayer();

    // save as .dmf.
    SafeWriter* saveDMF(unsigned char version);
    // save as .fur.
    // if notPrimary is true then the song will not be altered
    SafeWriter* saveFur(bool notPrimary=false);
    // return a ROM exporter.
    DivROMExport* buildROM(DivROMExportOptions sys);
    // compile instruments.
    SafeWriter* compileAllIns(int insType);
    // dump to VGM.
    // set trailingTicks to:
    // - 0 to add one tick of trailing
    // - x to add x+1 ticks of trailing
    // - -1 to auto-determine trailing
    // - -2 to add a whole loop of trailing
    SafeWriter* saveVGM(bool* sysToExport=NULL, bool loop=true, int version=0x171, bool patternHints=false, bool directStream=false, int trailingTicks=-1, bool dpcm07=false, int correctedRate=44100);
    // dump to S98.
    SafeWriter* saveS98(float tickRate=0.0f, bool* sysToExport=NULL, bool loop=true, int trailingTicks=-1);
    // dump command stream.
    SafeWriter* saveCommand(DivCSProgress* progress=NULL, DivCSOptions options=DivCSOptions());
    // export to text
    SafeWriter* saveText(bool separatePatterns=true);
#ifdef WITH_JSON
    // export to json
    SafeWriter* saveJSON(DivJSONExportOptions* options);
#endif
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
    // notify sample change
    void notifySampleChange(int sample);
    // notify a change which requires regenerating the pitch table
    void notifyPitchTable(int sample=-1);

    // dispatch a command
    int dispatchCmd(DivCommand c);

    // get system IDs
    static DivSystem systemFromFileFur(unsigned short val);
    static unsigned short systemToFileFur(DivSystem val);
    static DivSystem systemFromFileDMF(unsigned char val);
    static unsigned char systemToFileDMF(DivSystem val);

    // convert old flags
    static void convertOldFlags(unsigned int oldFlags, DivConfig& newFlags, DivSystem sys);

    // benchmark (returns time in seconds)
    double benchmarkPlayback();
    double benchmarkSeek();
    double benchmarkWalk();

    // returns the minimum VGM version which may carry the specified system, or 0 if none.
    int minVGMVersion(DivSystem which);

    // returns whether the S98 format supports this system.
    bool supportedByS98(DivSystem which);

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
    // DEPRECATED. use DivPitchTable instead.
    double calcBaseFreq(double clock, double divider, int note, bool period);

    // calculate base frequency in f-num/block format
    // TODO: get rid of this and use DivPitchTable...
    int calcBaseFreqFNumBlock(double clock, double divider, int note, int bits, int fixedBlock);

    // calculate frequency/period
    // DEPRECATED. use DivPitchTable instead.
    int calcFreq(int base, int pitch, int arp, bool arpFixed, bool period=false, int octave=0, int pitch2=0, double clock=1.0, double divider=1.0, int blockBits=0, int fixedBlock=0);

    // calculate arpeggio
    int calcArp(int note, int arp, int offset=0);

    // convert panning formats
    int convertPanSplitToLinear(unsigned int val, unsigned char bits, int range);
    int convertPanSplitToLinearLR(unsigned char left, unsigned char right, int range);
    unsigned int convertPanLinearToSplit(int val, unsigned char bits, int range);

    // calculate all song timestamps
    void calcSongTimestamps();

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
    static const DivSysDef* getSystemDef(DivSystem sys);

    // get ROM export definition
    const DivROMExportDef* getROMExportDef(DivROMExportOptions opt);
    // check whether ROM export option is viable for current song
    bool isROMExportViable(DivROMExportOptions opt);

    // convert sample rate format
    int fileToDivRate(int frate);
    int divToFileRate(int drate);

    // get effective sample rate
    int getEffectiveSampleRate(int rate);

    // convert between old and new note/octave format
    short splitNoteToNote(short note, short octave);
    void noteToSplitNote(short note, short& outNote, short& outOctave);

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

    // get max frequency/period of a channel
    unsigned int getMaxFreqChan(int ch);

    // get current order
    unsigned char getOrder();

    // get current row
    int getRow();

    // synchronous get order/row
    void getPlayPos(int& order, int& row);
    void getPlayPosTick(int& order, int& row, int& tick, int& speed);

    // get the row speed used for live preview timing
    int getPreviewSpeed();

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
    TimeMicros getCurTime();

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
    bool autoNoteOn(int chan, int ins, int note, int vol=-1, int transpose=0);
    void autoNoteOff(int chan, int note, int vol=-1);
    void autoNoteOffAll();

    // set whether autoNoteIn is mono or poly
    void setAutoNotePoly(bool poly);

    // get next viable channel with an offset
    // chan is the base channel, off is the offset and ins is the instrument.
    int getViableChannel(int chan, int off, int ins);

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
    SharedChannel* getDispatchChanState(int chan);

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

    // public copy channel
    void copyChannelP(int src, int dest);

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

    // set system channel count
    bool setSystemChans(int index, int ch, bool preserveOrder=true);

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
    // if the specified function returns -3, note feedback will be inhibited.
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

    // get audio device status
    TAAudioDeviceStatus getAudioDeviceStatus();

    // acknowledge an audio device status change
    void acceptAudioDeviceStatus();

    // send command to audio backend
    int audioBackendCommand(TAAudioCommand which);

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
      totalTicksR(0),
      curMidiClock(0),
      curMidiTime(0),
      totalCmds(0),
      lastCmds(0),
      cmdsPerSecond(0),
      totalTimeDrift(0.0),
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
      exportFormat(DIV_EXPORT_FORMAT_WAV),
      wavFormat(DIV_EXPORT_WAV_S16),
      exportBitRateMode(DIV_EXPORT_BITRATE_CONSTANT),
      prevAudioRate(44100.0),
      exportFadeOut(0.0),
      isFadingOut(false),
      exportOutputs(2),
      exportBitRate(128000),
      exportVBRQuality(6.0f),
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
      filePlayerBufLen(0),
      curFilePlayer(NULL),
      filePlayerSync(false),
      filePlayerCue(0,0),
      filePlayerLoopTrail(0),
      curFilePlayerTrail(0),
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
      memset(vibTable,0,64*sizeof(short));
      memset(tremTable,0,128*sizeof(short));
      memset(effectSlotMap,-1,4096*sizeof(short));
      memset(sysDefs,0,DIV_SYSTEM_MAX*sizeof(void*));
      memset(romExportDefs,0,DIV_ROM_MAX*sizeof(void*));
      memset(walked,0,8192);
      memset(oscBuf,0,DIV_MAX_OUTPUTS*(sizeof(float*)));
      memset(exportChannelMask,1,DIV_MAX_CHANS*sizeof(bool));
      memset(chipPeak,0,DIV_MAX_CHIPS*DIV_MAX_OUTPUTS*sizeof(float));
      memset(filePlayerBuf,0,DIV_MAX_OUTPUTS*sizeof(float));

      changeSong(0);
    }
};
#endif
