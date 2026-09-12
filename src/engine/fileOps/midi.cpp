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

// TODO: this code needs to be partially rewritten. a message from its author follows.
//
// Yes, I used Claude Opus for questions and code implementation
// 
// Before committing + pushing, the code is personally reviewed by me and tested to ensure the code generated is quality code, and that it follows the contributing rules
// 
// I honestly left the disclosure out because I didn't know how people would react with MIDI import action since there was some long heated discussions in the past, and right now AI-assisted contributions are also having very heated discussions so i wanted to pick only 1 struggle, even tho it will make me look like a asshole
// 
// but in future contributions (if i make them) I will disclosure the usage of LMMs if they are used

#include "fileOpsCommon.h"
#include <algorithm>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <map>
#include <vector>

static const char* midiGMInstrumentNames[128]={
  "Acoustic Grand Piano", "Bright Acoustic Piano", "Electric Grand Piano", "Honky-tonk Piano",
  "Electric Piano 1", "Electric Piano 2", "Harpsichord", "Clavinet",
  "Celesta", "Glockenspiel", "Music Box", "Vibraphone",
  "Marimba", "Xylophone", "Tubular Bells", "Dulcimer",
  "Drawbar Organ", "Percussive Organ", "Rock Organ", "Church Organ",
  "Reed Organ", "Accordion", "Harmonica", "Tango Accordion",
  "Acoustic Guitar (nylon)", "Acoustic Guitar (steel)", "Electric Guitar (jazz)", "Electric Guitar (clean)",
  "Electric Guitar (muted)", "Overdriven Guitar", "Distortion Guitar", "Guitar Harmonics",
  "Acoustic Bass", "Electric Bass (finger)", "Electric Bass (pick)", "Fretless Bass",
  "Slap Bass 1", "Slap Bass 2", "Synth Bass 1", "Synth Bass 2",
  "Violin", "Viola", "Cello", "Contrabass",
  "Tremolo Strings", "Pizzicato Strings", "Orchestral Harp", "Timpani",
  "String Ensemble 1", "String Ensemble 2", "Synth Strings 1", "Synth Strings 2",
  "Choir Aahs", "Voice Oohs", "Synth Voice", "Orchestra Hit",
  "Trumpet", "Trombone", "Tuba", "Muted Trumpet",
  "French Horn", "Brass Section", "Synth Brass 1", "Synth Brass 2",
  "Soprano Sax", "Alto Sax", "Tenor Sax", "Baritone Sax",
  "Oboe", "English Horn", "Bassoon", "Clarinet",
  "Piccolo", "Flute", "Recorder", "Pan Flute",
  "Blown Bottle", "Shakuhachi", "Whistle", "Ocarina",
  "Lead 1 (square)", "Lead 2 (sawtooth)", "Lead 3 (calliope)", "Lead 4 (chiff)",
  "Lead 5 (charang)", "Lead 6 (voice)", "Lead 7 (fifths)", "Lead 8 (bass + lead)",
  "Pad 1 (new age)", "Pad 2 (warm)", "Pad 3 (polysynth)", "Pad 4 (choir)",
  "Pad 5 (bowed)", "Pad 6 (metallic)", "Pad 7 (halo)", "Pad 8 (sweep)",
  "FX 1 (rain)", "FX 2 (soundtrack)", "FX 3 (crystal)", "FX 4 (atmosphere)",
  "FX 5 (brightness)", "FX 6 (goblins)", "FX 7 (echoes)", "FX 8 (sci-fi)",
  "Sitar", "Banjo", "Shamisen", "Koto",
  "Kalimba", "Bag pipe", "Fiddle", "Shanai",
  "Tinkle Bell", "Agogo", "Steel Drums", "Woodblock",
  "Taiko Drum", "Melodic Tom", "Synth Drum", "Reverse Cymbal",
  "Guitar Fret Noise", "Breath Noise", "Seashore", "Bird Tweet",
  "Telephone Ring", "Helicopter", "Applause", "Gunshot"
};

#define MIDI_DRUM_FIRST 35
#define MIDI_DRUM_LAST 81
static const char* midiGMDrumNames[MIDI_DRUM_LAST-MIDI_DRUM_FIRST+1]={
  "Acoustic Bass Drum", "Bass Drum 1", "Side Stick", "Acoustic Snare",
  "Hand Clap", "Electric Snare", "Low Floor Tom", "Closed Hi-hat",
  "High Floor Tom", "Pedal Hi-hat", "Low Tom", "Open Hi-hat",
  "Low-Mid Tom", "Hi-Mid Tom", "Crash Cymbal 1", "High Tom",
  "Ride Cymbal 1", "Chinese Cymbal", "Ride Bell", "Tambourine",
  "Splash Cymbal", "Cowbell", "Crash Cymbal 2", "Vibraslap",
  "Ride Cymbal 2", "Hi Bongo", "Low Bongo", "Mute Hi Conga",
  "Open Hi Conga", "Low Conga", "High Timbale", "Low Timbale",
  "High Agogo", "Low Agogo", "Cabasa", "Maracas",
  "Short Whistle", "Long Whistle", "Short Guiro", "Long Guiro",
  "Claves", "Hi Wood Block", "Low Wood Block", "Mute Cuica",
  "Open Cuica", "Mute Triangle", "Open Triangle"
};

struct MIDIInvalidException {
};

#define MIDI_MAX_HZ 999.0

#define MIDI_BASE_HZ 60.0

static void midiComputeBaseGroove(int R, int tempo, DivGroovePattern& groove) {
  double rowsPerSecond=(double)R*1000000.0/(double)tempo;
  if (rowsPerSecond<=0.0) rowsPerSecond=8.0;
  double avgSpeed=MIDI_BASE_HZ/rowsPerSecond;
  if (avgSpeed<1.0) avgSpeed=1.0;
  if (avgSpeed>512.0) avgSpeed=512.0;

  int bestLen=1;
  int bestSum=round(avgSpeed);
  double bestErr=-1.0;
  for (int len=1; len<=16; len++) {
    if (R%len) continue;
    int sum=round(avgSpeed*(double)len);
    if (sum<len) sum=len;
    if (sum>512*len) sum=512*len;
    double err=fabs((double)sum/(double)len-avgSpeed);
    if (bestErr<0.0 || err<bestErr-1.0e-12) {
      bestErr=err;
      bestLen=len;
      bestSum=sum;
      if (err<1.0e-12) break;
    }
  }

  groove.len=(unsigned short)bestLen;
  for (int i=0; i<16; i++) {
    int j=(i<bestLen)?i:(bestLen-1);
    groove.val[i]=(unsigned short)((int64_t)bestSum*(j+1)/bestLen-(int64_t)bestSum*j/bestLen);
  }
}

static bool midiGrooveEq(const DivGroovePattern& a, const DivGroovePattern& b) {
  if (a.len!=b.len) return false;
  for (int i=0; i<a.len; i++) {
    if (a.val[i]!=b.val[i]) return false;
  }
  return true;
}

#define MIDI_NOTE_BIAS 48
#define MIDI_DRUM_NOTE (60+MIDI_NOTE_BIAS)

#define MIDI_FX_NOTE_DELAY 0
#define MIDI_FX_NOTE_CUT 1
#define MIDI_FX_PITCH 2
#define MIDI_FX_VIBRATO 3
#define MIDI_FX_PAN 4

#define MIDI_FX_NO_RETRIGGER 5
#define MIDI_FX_MAX 6

struct DivMIDIChanState {
  int program;
  int volume;
  int expression;
  bool sustain;
  bool monoMode;
  int pitchBend;
  int bendSemis, bendFine, bendRangeCents;
  int rpnMSB, rpnLSB;
  int pan;
  int modulation;
  int noteOn[128];
  DivMIDIChanState():
    program(0),
    volume(127),
    expression(127),
    sustain(false),
    monoMode(false),
    pitchBend(0),
    bendSemis(2),
    bendFine(0),
    bendRangeCents(200),
    rpnMSB(0),
    rpnLSB(0),
    pan(64),
    modulation(0) {
    for (int i=0; i<128; i++) noteOn[i]=-1;
  }
};

struct DivMIDIModChanState {
  int midiCh;
  int note;
  int vel;
  int64_t age;
  bool sustained;
  int noteRow, prevNoteRow;
  int noteDelay;
  int baseNote;
  int curNote;
  int pitchApplied;
  bool noRetrigger;
  int panApplied;
  int vibratoApplied;
  DivMIDIModChanState():
    midiCh(-1),
    note(-1),
    vel(0),
    age(0),
    sustained(false),
    noteRow(-1),
    prevNoteRow(-1),
    noteDelay(0),
    baseNote(-1),
    curNote(-1),
    pitchApplied(0),
    noRetrigger(false),
    panApplied(-1),
    vibratoApplied(0) {}
  bool rowTaken(int row) const {
    return noteRow==row || prevNoteRow==row;
  }
};

struct DivMIDIPart {
  int track, channel;
  int firstProgram;
  std::vector<int> voices;
  String name;
  DivMIDIPart():
    track(0),
    channel(0),
    firstProgram(-1) {}
};

struct DivMIDITrackState {
  SafeReader* r;
  int64_t nextEvent;
  unsigned char runningStatus;
  int midiBaseChannel;
  bool finished;
  DivMIDITrackState():
    r(NULL),
    nextEvent(0),
    runningStatus(0),
    midiBaseChannel(0),
    finished(false) {}
};

struct DivMIDITempoEvent {
  int64_t tick;
  int tempo, order, row;
};

static unsigned int midiReadVarLen(SafeReader& r) {
  unsigned int value=0;
  unsigned char b=0;
  int count=0;
  do {
    b=(unsigned char)r.readC();
    value=(value<<7)|(unsigned int)(b&0x7f);
    count++;
  } while ((b&0x80) && count<5);
  return value;
}

static void midiSkip(SafeReader& r, size_t n) {
  if (n==0) return;
  if (!r.seek(n,SEEK_CUR)) throw EndOfFileException(&r,r.size());
}

static int64_t midiMulDivR(int64_t a, int64_t b, int64_t c) {
  if (c<1) c=1;
  const int64_t n=a*b;
  return (n>=0)?((n+c/2)/c):((n-c/2)/c);
}

template<typename K> static K midiMostCommonByDuration(const std::vector<std::pair<int64_t,K>>& events, int64_t songEndTicks, K fallback) {
  if (events.empty()) return fallback;
  std::map<K,int64_t> weight;
  for (size_t i=0; i<events.size(); i++) {
    int64_t start=events[i].first;
    int64_t end=(i+1<events.size())?events[i+1].first:songEndTicks;
    int64_t dur=end-start;
    if (dur<0) dur=0;
    weight[events[i].second]+=dur;
  }
  K best=events[0].second;
  int64_t bestW=-1;
  for (typename std::map<K,int64_t>::iterator kv=weight.begin(); kv!=weight.end(); kv++) {
    if (kv->second>bestW) {
      bestW=kv->second;
      best=kv->first;
    }
  }
  return best;
}

static short midiVolumeOf(int vel, int cc7, int cc11, bool useVel, bool useCC7, bool useCC11) {
  double scale=1.0;
  if (useVel) scale*=(double)vel/127.0;
  if (useCC7) scale*=(double)cc7/127.0;
  if (useCC11) scale*=(double)cc11/127.0;
  int v=round(scale*127.0);
  return (short)CLAMP(v,0,127);
}

static void midiWriteFx(short* row, unsigned char& effectCols, int role, unsigned char fx, unsigned char val) {
  row[DIV_PAT_FX(role)]=(short)fx;
  row[DIV_PAT_FXVAL(role)]=(short)val;
  if (effectCols<(unsigned char)(role+1)) effectCols=(unsigned char)(role+1);
}

static void midiCompactFx(DivSubSong* sub, int numChans, int ordersLen, int patLen) {
  for (int ch=0; ch<numChans; ch++) {
    bool roleUsed[MIDI_FX_MAX];
    for (int i=0; i<MIDI_FX_MAX; i++) roleUsed[i]=false;
    for (int o=0; o<ordersLen; o++) {
      DivPattern* p=sub->pat[ch].data[o];
      if (p==NULL) continue;
      for (int k=0; k<patLen; k++) {
        for (int role=0; role<MIDI_FX_MAX; role++) {
          if (p->newData[k][DIV_PAT_FX(role)]!=-1) roleUsed[role]=true;
        }
      }
    }

    int newLane[MIDI_FX_MAX];
    int laneCount=0;
    bool identity=true;
    for (int role=0; role<MIDI_FX_MAX; role++) {
      if (!roleUsed[role]) {
        newLane[role]=-1;
        continue;
      }
      newLane[role]=laneCount;
      if (newLane[role]!=role) identity=false;
      laneCount++;
    }

    if (!identity) {
      for (int o=0; o<ordersLen; o++) {
        DivPattern* p=sub->pat[ch].data[o];
        if (p==NULL) continue;
        for (int k=0; k<patLen; k++) {
          short fx[MIDI_FX_MAX], val[MIDI_FX_MAX];
          for (int role=0; role<MIDI_FX_MAX; role++) {
            fx[role]=p->newData[k][DIV_PAT_FX(role)];
            val[role]=p->newData[k][DIV_PAT_FXVAL(role)];
            p->newData[k][DIV_PAT_FX(role)]=-1;
            p->newData[k][DIV_PAT_FXVAL(role)]=-1;
          }
          for (int role=0; role<MIDI_FX_MAX; role++) {
            if (newLane[role]<0) continue;
            p->newData[k][DIV_PAT_FX(newLane[role])]=fx[role];
            p->newData[k][DIV_PAT_FXVAL(newLane[role])]=val[role];
          }
        }
      }
    }
    sub->pat[ch].effectCols=(unsigned char)MAX(1,laneCount);
  }
}

static void midiResolveVibratoRate(DivSubSong* sub, int numChans, int ordersLen, int patLen, double songHz) {
  for (int ch=0; ch<numChans; ch++) {
    for (int o=0; o<ordersLen; o++) {
      DivPattern* p=sub->pat[ch].data[o];
      if (p==NULL) continue;
      for (int k=0; k<patLen; k++) {
        if (p->newData[k][DIV_PAT_FX(MIDI_FX_VIBRATO)]!=0x04) continue;
        short& v=p->newData[k][DIV_PAT_FXVAL(MIDI_FX_VIBRATO)];
        if (v<=0) continue;
        const int speed=CLAMP((int)lround((double)((v>>4)&15)*64.0/songHz),1,15);
        v=(short)((speed<<4)|(v&15));
      }
    }
  }
}

static int midiBendToPitch(int bend, int rangeCents) {
  return (int)midiMulDivR(bend,(int64_t)rangeCents*128,8192LL*100);
}

static unsigned char midiModTo04xy(int mod, int maxDepth, int rateHz) {
  const int depth=(int)midiMulDivR(mod,CLAMP(maxDepth,1,15),127);
  if (depth<1) return 0;
  return (unsigned char)((CLAMP(rateHz,1,15)<<4)|depth);
}

static unsigned char midiPanTo80xx(int pan) {
  return (unsigned char)((pan>=127)?255:(pan*2));
}

static int midiWritePitchSlide(short* row, unsigned char& effectCols, int steps) {
  short& fx=row[DIV_PAT_FX(MIDI_FX_PITCH)];
  short& val=row[DIV_PAT_FXVAL(MIDI_FX_PITCH)];
  const int prev=(fx==0xf1)?val:((fx==0xf2)?-val:0);
  const int total=CLAMP(prev+steps,-255,255);
  if (total!=0) {
    midiWriteFx(row,effectCols,MIDI_FX_PITCH,(total>0)?0xf1:0xf2,(unsigned char)((total>0)?total:-total));
  } else {
    fx=-1;
    val=-1;
  }
  return total-prev;
}

static bool midiSplitBend(int want, int cur, int slideSpeed, int& newNote, int& steps) {
  const int reach=255*slideSpeed;
  if (want-cur<=reach && cur-want<=reach) {
    newNote=-1;
    steps=CLAMP((want-cur)/slideSpeed,-255,255);
    return false;
  }
  newNote=CLAMP((want+64)>>7,0,179);
  steps=CLAMP((want-(newNote<<7))/slideSpeed,-255,255);
  return true;
}

static int midiAllocVoice(DivMIDIPart& part, const std::vector<DivMIDIModChanState>& chans, int& nextChan, int voiceChans, int note, bool monoMode, int totalRow, int& outRow, int& nudgeCount, int& crowdedCount, int& stealCount) {
  outRow=totalRow;

  for (size_t i=0; i<part.voices.size(); i++) {
    int ch=part.voices[i];
    if (chans[ch].rowTaken(totalRow)) continue;
    if (chans[ch].note==note || (monoMode && chans[ch].note!=-1)) return ch;
  }

  int sounding=0;
  for (size_t i=0; i<part.voices.size(); i++) {
    if (chans[part.voices[i]].note!=-1) sounding++;
  }
  if (sounding>=(int)part.voices.size() && nextChan<voiceChans) {
    int ch=nextChan++;
    part.voices.push_back(ch);
    return ch;
  }

  for (size_t i=0; i<part.voices.size(); i++) {
    int ch=part.voices[i];
    if (chans[ch].note==-1 && !chans[ch].rowTaken(totalRow)) return ch;
  }
  for (size_t i=0; i<part.voices.size(); i++) {
    int ch=part.voices[i];
    if (!chans[ch].rowTaken(totalRow)) return ch;
  }

  for (size_t i=0; i<part.voices.size(); i++) {
    int ch=part.voices[i];
    if (chans[ch].note==-1 && !chans[ch].rowTaken(totalRow+1)) {
      outRow=totalRow+1;
      nudgeCount++;
      return ch;
    }
  }

  if (nextChan<voiceChans) {
    crowdedCount++;
    int ch=nextChan++;
    part.voices.push_back(ch);
    return ch;
  }

  stealCount++;
  int oldest=part.voices[0];
  for (size_t i=0; i<part.voices.size(); i++) {
    int ch=part.voices[i];
    if (chans[ch].age<chans[oldest].age) oldest=ch;
  }
  return oldest;
}

bool DivEngine::loadMIDI(unsigned char* file, size_t len) {
  bool success=false;
  SafeReader reader=SafeReader(file,len);
  std::vector<DivMIDITrackState> tracks;
  warnings="";

  try {
    DivSong ds;
    ds.version=DIV_VERSION_MIDI;

    const int drumChannel=(midiImportOptions.drumChannel>=1 && midiImportOptions.drumChannel<=16)?(midiImportOptions.drumChannel-1):-1;
    const int slideSpeed=(ds.compatFlags.linearPitch && ds.compatFlags.pitchSlideSpeed>0)?ds.compatFlags.pitchSlideSpeed:1;

    reader.seek(4,SEEK_SET);
    int headerLen=reader.readI_BE();
    if (headerLen<6) throw EndOfFileException(&reader,reader.size());
    int format=reader.readS_BE();
    int numTracks=(unsigned short)reader.readS_BE();
    unsigned short division=(unsigned short)reader.readS_BE();
    if (headerLen>6) midiSkip(reader,(size_t)(headerLen-6));
    logD("MIDI import: format %d, %d tracks, division %d",format,numTracks,division);

    if (numTracks<1) {
      lastError="MIDI file has no tracks";
      throw MIDIInvalidException();
    }
    if (format==2) {
      lastError="MIDI format 2 is not supported";
      throw MIDIInvalidException();
    }

    int ppqn=division;
    if (division&0x8000) {
      int frames=256-((division>>8)&0xff);
      int subFrames=division&0xff;
      ppqn=frames*subFrames/2;
    }
    if (ppqn<1) ppqn=96;

    std::vector<String> trackNames;
    while (reader.tell()+8<=reader.size() && (int)tracks.size()<numTracks) {
      char chunkID[4];
      reader.read(chunkID,4);
      int chunkLenS=reader.readI_BE();
      if (chunkLenS<0) throw EndOfFileException(&reader,reader.size());
      size_t chunkLen=(size_t)chunkLenS;
      if (chunkLen>reader.size()-reader.tell()) chunkLen=reader.size()-reader.tell();
      size_t chunkStart=reader.tell();
      midiSkip(reader,chunkLen);
      if (strncmp(chunkID,"MTrk",4)!=0) continue;

      DivMIDITrackState ts;
      ts.r=new SafeReader(file+chunkStart,chunkLen);
      try {
        ts.nextEvent=(int64_t)midiReadVarLen(*ts.r);
      } catch (EndOfFileException& e) {
        ts.finished=true;
        ts.nextEvent=INT64_MAX;
      }
      tracks.push_back(ts);
      trackNames.push_back("");
    }
    numTracks=(int)tracks.size();
    if (numTracks<1) {
      lastError="MIDI file has no tracks";
      throw MIDIInvalidException();
    }

    const int tempoChan=DIV_MAX_CHANS-1;
    const int voiceChans=DIV_MAX_CHANS-1;

    ds.systemLen=1;
    ds.system[0]=DIV_SYSTEM_DUMMY;
    ds.systemVol[0]=1.0f;
    ds.systemPan[0]=0.0f;
    ds.systemFlags[0].set("volMax",127);

    int chanDefIdx=0;
    DivInstrumentType pcmInsType=getSystemDef(ds.system[0])->getChanDef(chanDefIdx).insType[0];

    DivSubSong* sub=ds.subsong[0];
    sub->patLen=midiImportOptions.patternLen;

    DivMIDIChanState midiChan[16];
    if (midiImportOptions.bendRange>0) {
      for (size_t i=0; i<16; i++) midiChan[i].bendRangeCents=midiImportOptions.bendRange*100;
    }
    std::vector<DivMIDIModChanState> modChan(DIV_MAX_CHANS);
    std::vector<DivMIDITempoEvent> tempoEvents;
    std::map<std::pair<int,int>,int> insMap;

    std::vector<DivMIDIPart> parts;
    std::map<std::pair<int,int>,int> partIndexOf;
    int nextChan=0;

    int maxOrd=0;
    int64_t songEndTicks=0;
    int stealCount=0, zeroLenNotes=0, partOverflow=0;
    int retimedCount=0, droppedCount=0, tempoClamped=0, grooveOverflow=0;
    int nudgeCount=0, crowdedCount=0;
    int bendClamped=0;
    int timeSigNumer=4, timeSigDenom=4;
    bool haveTimeSig=false, timeSigChanges=false;
    bool truncated=false, pitchBendSeen=false, rpnSeen=false;

    int finishedTracks=0;
    while (finishedTracks<numTracks) {
      int t=-1;
      int64_t tick=INT64_MAX;
      for (int i=0; i<numTracks; i++) {
        if (!tracks[i].finished && tracks[i].nextEvent<tick) {
          tick=tracks[i].nextEvent;
          t=i;
        }
      }
      if (t<0) break;
      DivMIDITrackState& tr=tracks[t];
      SafeReader& r=*tr.r;

      int64_t modTicks=midiMulDivR(tick,(int64_t)midiImportOptions.quantize*(int64_t)midiImportOptions.ticksPerRow,(int64_t)ppqn*4);
      int64_t totalRow64=modTicks/midiImportOptions.ticksPerRow;
      int delay=(int)(modTicks%midiImportOptions.ticksPerRow);
      int64_t ord64=totalRow64/midiImportOptions.patternLen;
      if (ord64>=DIV_MAX_PATTERNS) {
        truncated=true;
        break;
      }
      const int totalRow=(int)totalRow64;
      const int ord=(int)ord64;
      const int row=(int)(totalRow64%midiImportOptions.patternLen);

      if (ord>maxOrd) maxOrd=ord;
      if (tick>songEndTicks) songEndTicks=tick;

      bool endTrack=false;
      try {
        unsigned char data1=(unsigned char)r.readC();
        if (data1==0xff) {
          unsigned char metaType=(unsigned char)r.readC();
          unsigned int metaLen=midiReadVarLen(r);
          size_t metaEnd=r.tell()+(size_t)metaLen;
          if (metaEnd>r.size()) metaEnd=r.size();
          switch (metaType) {
            case 0x03:
              if (metaLen>0 && trackNames[t].empty()) {
                trackNames[t]=r.readString((size_t)metaLen);
                if (ds.name.empty()) ds.name=trackNames[t];
              }
              break;
            case 0x21:
              if (metaLen>=1) tr.midiBaseChannel=((unsigned char)r.readC())*16;
              break;
            case 0x2f:
              endTrack=true;
              break;
            case 0x51:
              if (metaLen>=3) {
                unsigned char t0=(unsigned char)r.readC();
                unsigned char t1=(unsigned char)r.readC();
                unsigned char t2=(unsigned char)r.readC();
                int tempo=(t0<<16)|(t1<<8)|t2;
                if (tempo>0) tempoEvents.push_back({tick,tempo,ord,row});
              }
              break;
            case 0x58:
              if (metaLen>=2) {
                int numer=(unsigned char)r.readC();
                int denomPow=(unsigned char)r.readC();
                if (denomPow>10) denomPow=10;
                if (numer>0) {
                  if (!haveTimeSig) {
                    timeSigNumer=numer;
                    timeSigDenom=1<<denomPow;
                    haveTimeSig=true;
                  } else if (numer!=timeSigNumer || (1<<denomPow)!=timeSigDenom) {
                    timeSigChanges=true;
                  }
                }
              }
              break;
            default:
              break;
          }

          if (!r.seek(metaEnd,SEEK_SET)) throw EndOfFileException(&r,r.size());
        } else {
          unsigned char command=tr.runningStatus;
          if (data1&0x80) {
            command=data1;
            if (data1<0xf0) {
              tr.runningStatus=data1;
              data1=(unsigned char)r.readC();
            }
          }

          if (command<0x80) throw EndOfFileException(&r,r.size());
          const int midiCh=((command&0x0f)+tr.midiBaseChannel)%16;
          DivMIDIChanState& mc=midiChan[midiCh];
          const bool isDrum=(midiCh==drumChannel);

          auto noteOff=[&](int note) {
            if (note<0 || note>127) return;
            int ch=mc.noteOn[note];
            if (ch<0) return;
            if (mc.sustain && midiImportOptions.importSustain) {
              modChan[ch].sustained=true;
              return;
            }
            modChan[ch].note=-1;
            modChan[ch].sustained=false;
            mc.noteOn[note]=-1;

            short* cell=sub->pat[ch].getPattern(ord,true)->newData[row];
            if (cell[DIV_PAT_NOTE]==-1) {
              cell[DIV_PAT_NOTE]=DIV_NOTE_OFF;
              if (midiImportOptions.useBaseTempo && delay!=0) midiWriteFx(cell,sub->pat[ch].effectCols,MIDI_FX_NOTE_DELAY,0xed,(unsigned char)delay);
              return;
            }
            if (cell[DIV_PAT_NOTE]>=DIV_NOTE_RAW || isDrum) return;

            if (midiImportOptions.useBaseTempo) {
              int cut=delay-modChan[ch].noteDelay;
              if (cut<1) {
                cut=1;
                zeroLenNotes++;
              }
              midiWriteFx(cell,sub->pat[ch].effectCols,MIDI_FX_NOTE_CUT,0xec,(unsigned char)cut);
            } else {
              int offRow=totalRow+1;
              int offOrd=offRow/midiImportOptions.patternLen;
              if (offOrd>=DIV_MAX_PATTERNS) return;
              short* offCell=sub->pat[ch].getPattern(offOrd,true)->newData[offRow%midiImportOptions.patternLen];
              if (offCell[DIV_PAT_NOTE]==-1) {
                offCell[DIV_PAT_NOTE]=DIV_NOTE_OFF;
                retimedCount++;
              } else {
                droppedCount++;
              }
            }
          };

          auto emitBend=[&](int ch, int startRow, int target) {
            for (int fr=startRow;; fr++) {
              const int want=modChan[ch].baseNote*128+target;
              int cur=modChan[ch].curNote*128+modChan[ch].pitchApplied;
              if (want==cur) break;

              const int bOrd=fr/midiImportOptions.patternLen;
              if (bOrd>=DIV_MAX_PATTERNS) break;
              if (bOrd>maxOrd) maxOrd=bOrd;
              short* bendCell=sub->pat[ch].getPattern(bOrd,true)->newData[fr%midiImportOptions.patternLen];

              int newNote, steps;
              if (midiSplitBend(want,cur,slideSpeed,newNote,steps)) {
                if (bendCell[DIV_PAT_NOTE]==-1) {
                  bendCell[DIV_PAT_NOTE]=(short)newNote;
                  modChan[ch].curNote=newNote;
                  modChan[ch].pitchApplied=0;
                  if (!modChan[ch].noRetrigger) {
                    midiWriteFx(bendCell,sub->pat[ch].effectCols,MIDI_FX_NO_RETRIGGER,0xea,1);
                    modChan[ch].noRetrigger=true;
                  }

                  bendCell[DIV_PAT_FX(MIDI_FX_PITCH)]=-1;
                  bendCell[DIV_PAT_FXVAL(MIDI_FX_PITCH)]=-1;
                  steps=CLAMP((want-(newNote<<7))/slideSpeed,-255,255);
                } else {

                  steps=CLAMP((want-cur)/slideSpeed,-255,255);
                }
              }

              const int applied=midiWritePitchSlide(bendCell,sub->pat[ch].effectCols,steps);
              modChan[ch].pitchApplied+=applied*slideSpeed;
              if (applied==steps || applied==0) break;
              if (fr==startRow) bendClamped++;
            }
          };

          auto applyBend=[&](int newBend) {
            mc.pitchBend=newBend;
            if (isDrum) return;
            const int newTarget=midiBendToPitch(newBend,mc.bendRangeCents);
            for (int n=0; n<128; n++) {
              int ch=mc.noteOn[n];
              if (ch>=0) emitBend(ch,totalRow,newTarget);
            }
          };

          auto applyPan=[&]() {
            const unsigned char panVal=midiPanTo80xx(mc.pan);
            for (int n=0; n<128; n++) {
              int ch=mc.noteOn[n];
              if (ch<0 || modChan[ch].panApplied==mc.pan) continue;
              modChan[ch].panApplied=mc.pan;
              midiWriteFx(sub->pat[ch].getPattern(ord,true)->newData[row],sub->pat[ch].effectCols,MIDI_FX_PAN,0x80,panVal);
            }
          };

          auto applyVibrato=[&]() {
            const unsigned char vibVal=midiModTo04xy(mc.modulation,midiImportOptions.vibratoDepth,midiImportOptions.vibratoRate);
            for (int n=0; n<128; n++) {
              int ch=mc.noteOn[n];
              if (ch<0 || modChan[ch].vibratoApplied==(int)vibVal) continue;
              modChan[ch].vibratoApplied=vibVal;
              midiWriteFx(sub->pat[ch].getPattern(ord,true)->newData[row],sub->pat[ch].effectCols,MIDI_FX_VIBRATO,0x04,vibVal);
            }
          };

          switch (command&0xf0) {
            case 0x80:
            case 0x90: {
              int note=data1&0x7f;
              unsigned char data2=(unsigned char)r.readC();
              if (data2>0 && (command&0xf0)==0x90) {
                std::pair<int,int> partKey=std::make_pair(t,midiCh);
                std::map<std::pair<int,int>,int>::iterator partIt=partIndexOf.find(partKey);
                int pi;
                if (partIt==partIndexOf.end()) {
                  pi=(int)parts.size();
                  DivMIDIPart np;
                  np.track=t;
                  np.channel=midiCh;
                  np.firstProgram=mc.program;
                  if (nextChan<voiceChans) {
                    np.voices.push_back(nextChan++);
                  } else {
                    np.voices.push_back(voiceChans-1);
                    partOverflow++;
                  }
                  parts.push_back(np);
                  partIndexOf[partKey]=pi;
                } else {
                  pi=partIt->second;
                }
                DivMIDIPart& part=parts[pi];
                int placeRow=totalRow;
                int ch=midiAllocVoice(part,modChan,nextChan,voiceChans,note,mc.monoMode,totalRow,placeRow,nudgeCount,crowdedCount,stealCount);
                int placeOrd=placeRow/midiImportOptions.patternLen;
                if (placeOrd>=DIV_MAX_PATTERNS) {
                  placeRow=totalRow;
                  placeOrd=ord;
                }
                const int placeLocalRow=placeRow%midiImportOptions.patternLen;

                const int placeDelay=(placeRow==totalRow)?delay:0;
                if (placeOrd>maxOrd) maxOrd=placeOrd;

                int oldNote=modChan[ch].note;
                if (oldNote>=0 && oldNote!=note && modChan[ch].midiCh>=0) {
                  midiChan[modChan[ch].midiCh].noteOn[oldNote]=-1;
                }

                modChan[ch].midiCh=midiCh;
                modChan[ch].note=note;
                modChan[ch].vel=data2;
                modChan[ch].age=tick;
                modChan[ch].sustained=false;
                modChan[ch].prevNoteRow=modChan[ch].noteRow;
                modChan[ch].noteRow=placeRow;
                modChan[ch].noteDelay=placeDelay;
                mc.noteOn[note]=ch;
                if (part.name.empty() && !trackNames[t].empty()) part.name=trackNames[t];

                short* cell=sub->pat[ch].getPattern(placeOrd,true)->newData[placeLocalRow];

                int outNote=(isDrum && midiImportOptions.splitDrums)?MIDI_DRUM_NOTE:CLAMP(note+MIDI_NOTE_BIAS,0,179);
                modChan[ch].baseNote=outNote;
                modChan[ch].curNote=outNote;
                modChan[ch].pitchApplied=0;

                int seedSteps=0;
                if (!isDrum && midiImportOptions.importPitchBend && mc.pitchBend!=0) {
                  const int want=outNote*128+midiBendToPitch(mc.pitchBend,mc.bendRangeCents);
                  int newNote;
                  if (midiSplitBend(want,outNote*128,slideSpeed,newNote,seedSteps)) {
                    outNote=newNote;
                    modChan[ch].curNote=newNote;

                    cell[DIV_PAT_FX(MIDI_FX_PITCH)]=-1;
                    cell[DIV_PAT_FXVAL(MIDI_FX_PITCH)]=-1;
                    seedSteps=CLAMP((want-(newNote<<7))/slideSpeed,-255,255);
                  }
                }
                cell[DIV_PAT_NOTE]=(short)outNote;

                if (modChan[ch].noRetrigger) {
                  midiWriteFx(cell,sub->pat[ch].effectCols,MIDI_FX_NO_RETRIGGER,0xea,0);
                  modChan[ch].noRetrigger=false;
                }

                std::pair<int,int> insKey;
                if (isDrum) {
                  insKey=std::make_pair(1+mc.program,midiImportOptions.splitDrums?note:-1);
                } else {
                  insKey=std::make_pair(0,mc.program);
                }
                std::map<std::pair<int,int>,int>::iterator insIt=insMap.find(insKey);
                int insIndex;
                if (insIt==insMap.end()) {
                  insIndex=(int)ds.ins.size();
                  DivInstrument* ins=new DivInstrument;
                  ins->type=pcmInsType;
                  if (insKey.first>0) {
                    int kit=insKey.first-1;
                    if (insKey.second<0) {
                      ins->name=(kit==0)?"Standard Drum Kit":fmt::sprintf("Drum Kit %d",kit);
                    } else {
                      if (insKey.second>=MIDI_DRUM_FIRST && insKey.second<=MIDI_DRUM_LAST) {
                        ins->name=midiGMDrumNames[insKey.second-MIDI_DRUM_FIRST];
                      } else {
                        ins->name=fmt::sprintf("Drum %d",insKey.second);
                      }
                      if (kit!=0) ins->name+=fmt::sprintf(" (Kit %d)",kit);
                    }
                    ins->std.dutyMacro.len=1;
                    ins->std.dutyMacro.val[0]=1;
                  } else {
                    ins->name=midiGMInstrumentNames[insKey.second&0x7f];
                    ins->std.dutyMacro.len=1;
                    ins->std.dutyMacro.val[0]=0;
                  }
                  ds.ins.push_back(ins);
                  insMap[insKey]=insIndex;
                } else {
                  insIndex=insIt->second;
                }
                cell[DIV_PAT_INS]=(short)insIndex;
                cell[DIV_PAT_VOL]=midiVolumeOf(data2,mc.volume,mc.expression,midiImportOptions.importVelocity,midiImportOptions.importCC7,midiImportOptions.importCC11);
                if (midiImportOptions.useBaseTempo && placeDelay!=0) midiWriteFx(cell,sub->pat[ch].effectCols,MIDI_FX_NOTE_DELAY,0xed,(unsigned char)placeDelay);

                if (seedSteps!=0) {
                  modChan[ch].pitchApplied=midiWritePitchSlide(cell,sub->pat[ch].effectCols,seedSteps)*slideSpeed;
                }

                if (midiImportOptions.importPan && mc.pan!=64) {
                  modChan[ch].panApplied=mc.pan;
                  midiWriteFx(cell,sub->pat[ch].effectCols,MIDI_FX_PAN,0x80,midiPanTo80xx(mc.pan));
                }

                if (midiImportOptions.importVibrato) {
                  const unsigned char vibVal=midiModTo04xy(mc.modulation,midiImportOptions.vibratoDepth,midiImportOptions.vibratoRate);
                  if (modChan[ch].vibratoApplied!=(int)vibVal) {
                    modChan[ch].vibratoApplied=vibVal;
                    midiWriteFx(cell,sub->pat[ch].effectCols,MIDI_FX_VIBRATO,0x04,vibVal);
                  }
                }
              } else {
                noteOff(note);
              }
              break;
            }
            case 0xa0:
              midiSkip(r,1);
              break;
            case 0xb0: {
              unsigned char data2=(unsigned char)r.readC();
              switch (data1) {
                case 7:
                case 11: {
                  if (data1==7) mc.volume=data2&0x7f; else mc.expression=data2&0x7f;

                  for (int n=0; n<128; n++) {
                    int ch=mc.noteOn[n];
                    if (ch<0) continue;
                    short vol=midiVolumeOf(modChan[ch].vel,mc.volume,mc.expression,midiImportOptions.importVelocity,midiImportOptions.importCC7,midiImportOptions.importCC11);
                    sub->pat[ch].getPattern(ord,true)->newData[row][DIV_PAT_VOL]=vol;
                  }
                  break;
                }
                case 1:
                  if (midiImportOptions.importVibrato) {
                    mc.modulation=data2&0x7f;
                    applyVibrato();
                  }
                  break;
                case 10:
                  if (midiImportOptions.importPan) {
                    mc.pan=data2&0x7f;
                    applyPan();
                  }
                  break;
                case 64:
                  mc.sustain=(data2>=0x40);
                  if (data2<0x40) {
                    for (int n=0; n<128; n++) {
                      int ch=mc.noteOn[n];
                      if (ch>=0 && modChan[ch].sustained) noteOff(n);
                    }
                  }
                  break;
                case 120:
                case 123:
                  mc.sustain=false;
                  for (int n=0; n<128; n++) noteOff(n);
                  break;
                case 121:
                  mc.volume=127;
                  mc.expression=127;
                  mc.sustain=false;
                  mc.monoMode=false;
                  mc.modulation=0;

                  if (midiImportOptions.importPitchBend) applyBend(0);
                  if (midiImportOptions.importVibrato) applyVibrato();
                  break;
                case 126:
                  if (data2==0) mc.monoMode=true;
                  break;
                case 127:
                  mc.monoMode=false;
                  break;
                case 98:
                case 99:
                  mc.rpnMSB=0x7f;
                  mc.rpnLSB=0x7f;
                  break;
                case 100:
                  mc.rpnLSB=data2&0x7f;
                  break;
                case 101:
                  mc.rpnMSB=data2&0x7f;
                  break;
                case 6:
                case 38:
                  if (mc.rpnMSB==0 && mc.rpnLSB==0) {
                    rpnSeen=true;
                    if (data1==6) {
                      mc.bendSemis=data2&0x7f;
                      mc.bendFine=0;
                    } else {
                      mc.bendFine=data2&0x7f;
                    }
                    if (midiImportOptions.bendRange==0) {
                      mc.bendRangeCents=mc.bendSemis*100+mc.bendFine;
                      if (midiImportOptions.importPitchBend) applyBend(mc.pitchBend);
                    }
                  }
                  break;
                default:
                  break;
              }
              break;
            }
            case 0xc0:
              mc.program=data1&0x7f;
              break;
            case 0xd0:
              break;
            case 0xe0:
              pitchBendSeen=true;
              if (midiImportOptions.importPitchBend) {
                applyBend(((((int)((unsigned char)r.readC())&0x7f)<<7)|((int)data1&0x7f))-8192);
              } else {
                midiSkip(r,1);
              }
              break;
            case 0xf0:
              if (command==0xf0 || command==0xf7) {
                unsigned int sysexLen=midiReadVarLen(r);
                midiSkip(r,sysexLen);
              } else if (command==0xf2) {
                midiSkip(r,2);
              } else if (command==0xf1 || command==0xf3) {
                midiSkip(r,1);
              }
              break;
            default:
              break;
          }
        }

        if (!endTrack) {
          if (r.tell()>=r.size()) {
            endTrack=true;
          } else {
            tr.nextEvent+=(int64_t)midiReadVarLen(r);
          }
        }
      } catch (EndOfFileException& e) {
        endTrack=true;
      }

      if (endTrack) {
        tr.finished=true;
        tr.nextEvent=INT64_MAX;
        finishedTracks++;
      }
    }

    if (ds.ins.empty()) {
      lastError="no notes found in MIDI file";
      throw MIDIInvalidException();
    }

    std::vector<std::pair<int64_t,int>> tempoPairs;
    for (DivMIDITempoEvent& e: tempoEvents) tempoPairs.push_back(std::make_pair(e.tick,e.tempo));
    int tempo0=midiMostCommonByDuration<int>(tempoPairs,songEndTicks+1,500000);

    double rowsPerSecond=(double)midiImportOptions.quantize*1000000.0/(4.0*(double)tempo0);
    bool tooFast=false;
    double songHz=MIDI_BASE_HZ;
    if (midiImportOptions.useBaseTempo) {
      songHz=rowsPerSecond*(double)midiImportOptions.ticksPerRow;
      if (songHz>MIDI_MAX_HZ) {
        songHz=MIDI_MAX_HZ;
        tooFast=true;
      }
      if (songHz<1.0) songHz=1.0;
      sub->speeds.len=1;
      for (int i=0; i<16; i++) sub->speeds.val[i]=(unsigned short)midiImportOptions.ticksPerRow;
    } else {
      songHz=MIDI_BASE_HZ;
      midiComputeBaseGroove(MAX(1,midiImportOptions.quantize/4),tempo0,sub->speeds);
      tooFast=(rowsPerSecond>MIDI_BASE_HZ);
    }
    logI("MIDI import: quantize %d, %d ticks/row, %d rows/pattern, %s, tick rate %g Hz",midiImportOptions.quantize,midiImportOptions.ticksPerRow,midiImportOptions.patternLen,midiImportOptions.useBaseTempo?"base tempo":"groove approximation",songHz);

    sub->hz=(float)songHz;

    bool anyTempoChange=false;
    int curTempoVal=tempo0;
    std::map<int,int> grooveOf;
    if (!midiImportOptions.useBaseTempo) {
      ds.grooves.push_back(sub->speeds);
      grooveOf[tempo0]=0;
    }
    int curGroove=0;
    for (DivMIDITempoEvent& te: tempoEvents) {
      if (te.tempo==curTempoVal) continue;
      if (te.order>maxOrd) continue;
      curTempoVal=te.tempo;

      int gi=0;
      if (!midiImportOptions.useBaseTempo) {
        std::map<int,int>::iterator gIt=grooveOf.find(te.tempo);
        if (gIt==grooveOf.end()) {
          DivGroovePattern g;
          midiComputeBaseGroove(MAX(1,midiImportOptions.quantize/4),te.tempo,g);
          gi=-1;
          for (size_t gj=0; gj<ds.grooves.size(); gj++) {
            if (midiGrooveEq(ds.grooves[gj],g)) {
              gi=(int)gj;
              break;
            }
          }
          if (gi<0) {
            if (ds.grooves.size()>=256) {
              grooveOverflow++;
              continue;
            }
            gi=(int)ds.grooves.size();
            ds.grooves.push_back(g);
          }
          grooveOf[te.tempo]=gi;
        } else {
          gi=gIt->second;
        }

        if (gi==curGroove) continue;
        curGroove=gi;
      }

      short* cell=sub->pat[tempoChan].getPattern(te.order,true)->newData[te.row];
      if (midiImportOptions.useBaseTempo) {
        double hz=(double)midiImportOptions.quantize*1000000.0/(4.0*(double)te.tempo)*(double)midiImportOptions.ticksPerRow;
        int hzI=round(hz);
        if (hzI<1) hzI=1;
        if (hzI>1023) {
          hzI=1023;
          tempoClamped++;
        }
        midiWriteFx(cell,sub->pat[tempoChan].effectCols,0,(unsigned char)(0xc0|((hzI>>8)&3)),(unsigned char)(hzI&0xff));
      } else {
        midiWriteFx(cell,sub->pat[tempoChan].effectCols,0,0x09,(unsigned char)gi);
      }
      anyTempoChange=true;
    }

    int rowsPerBeat=midiImportOptions.quantize/4;
    if (rowsPerBeat<1) rowsPerBeat=1;
    int rowsPerBar=round((double)midiImportOptions.quantize*(double)timeSigNumer/(double)timeSigDenom);
    if (rowsPerBar<1) rowsPerBar=rowsPerBeat;
    sub->hilightA=(unsigned char)CLAMP(rowsPerBeat,1,255);
    sub->hilightB=(unsigned char)CLAMP(rowsPerBar,1,255);

    std::vector<int> partOrder;
    for (size_t i=0; i<parts.size(); i++) partOrder.push_back((int)i);
    std::stable_sort(partOrder.begin(),partOrder.end(),[&parts](int a, int b) -> bool {
      if (parts[a].channel!=parts[b].channel) return parts[a].channel<parts[b].channel;
      return parts[a].track<parts[b].track;
    });

    std::vector<int> newChanOf(nextChan,-1);
    int sortCursor=0;
    for (size_t i=0; i<partOrder.size(); i++) {
      DivMIDIPart& part=parts[partOrder[i]];
      for (size_t j=0; j<part.voices.size(); j++) {
        if (part.voices[j]<nextChan && newChanOf[part.voices[j]]<0) newChanOf[part.voices[j]]=sortCursor++;
      }
    }

    for (int i=0; i<nextChan; i++) {
      if (newChanOf[i]<0) newChanOf[i]=sortCursor++;
    }
    if (sortCursor==nextChan) {
      std::vector<DivChannelData> shuffled(nextChan);
      for (int i=0; i<nextChan; i++) shuffled[newChanOf[i]]=sub->pat[i];
      for (int i=0; i<nextChan; i++) sub->pat[i]=shuffled[i];
      for (size_t i=0; i<parts.size(); i++) {
        for (size_t j=0; j<parts[i].voices.size(); j++) {
          int v=parts[i].voices[j];
          if (v<nextChan) parts[i].voices[j]=newChanOf[v];
        }
      }
    }

    for (size_t i=0; i<parts.size(); i++) {
      DivMIDIPart& part=parts[i];
      if (!part.name.empty()) continue;
      if (part.channel==drumChannel) {
        part.name=(part.firstProgram<=0)?String("Standard Drum Kit"):fmt::sprintf("Drum Kit %d",part.firstProgram);
      } else if (part.firstProgram>=0 && part.firstProgram<128) {
        part.name=midiGMInstrumentNames[part.firstProgram];
      } else {
        part.name=fmt::sprintf("Channel %d",part.channel+1);
      }
    }
    for (size_t i=0; i<parts.size(); i++) {
      DivMIDIPart& part=parts[i];
      if (part.voices.size()>1) {
        for (size_t j=0; j<part.voices.size(); j++) {
          sub->chanName[part.voices[j]]=fmt::sprintf("MIDI CH%02d | %s | [Polyphony: %d]",part.channel+1,part.name,(int)j+1);
        }
      } else {
        sub->chanName[part.voices[0]]=fmt::sprintf("MIDI CH%02d | %s",part.channel+1,part.name);
      }
    }

    int numChans=nextChan;
    if (anyTempoChange) {
      if (nextChan!=tempoChan) {
        sub->pat[nextChan].wipePatterns();
        sub->pat[nextChan]=sub->pat[tempoChan];
        for (int k=0; k<DIV_MAX_PATTERNS; k++) sub->pat[tempoChan].data[k]=NULL;
        sub->pat[tempoChan].effectCols=1;
      }
      sub->chanName[nextChan]="Tempo";
      numChans=nextChan+1;
    }
    if (numChans<1) numChans=1;
    for (int i=numChans; i<DIV_MAX_CHANS; i++) sub->pat[i].wipePatterns();
    ds.systemChans[0]=(unsigned short)numChans;

    int ordersLen=CLAMP(maxOrd+1,1,DIV_MAX_PATTERNS);
    sub->ordersLen=ordersLen;
    for (int ch=0; ch<numChans; ch++) {
      for (int o=0; o<ordersLen; o++) sub->orders.ord[ch][o]=(unsigned char)o;
    }

    ds.insLen=(int)ds.ins.size();

    logI("MIDI import: %d parts, %d channels, %d instruments",(int)parts.size(),numChans,ds.insLen);

    if (midiImportOptions.importVibrato) midiResolveVibratoRate(sub,nextChan,ordersLen,midiImportOptions.patternLen,songHz);

    midiCompactFx(sub,nextChan,ordersLen,midiImportOptions.patternLen);

    sub->removeUnusedPatterns();
    sub->optimizePatterns();

    ds.recalcChans();

    if (retimedCount>0) {
      addWarning(fmt::sprintf("%d Note-offs were moved a row later by quantization",retimedCount));
    }
    if (tempoClamped>0) {
      addWarning(fmt::sprintf("%d Tempo changes were faster than the 1023Hz tick rate limit; lower Ticks/Row or Quantize",tempoClamped));
    }
    if (grooveOverflow>0) {
      addWarning(fmt::sprintf("%d Tempo changes did not fit in the 256-groove table and were left out",grooveOverflow));
    }
    if (droppedCount>0) {
      logD("MIDI import: %d note-offs fell on a retrigger row and were left out",droppedCount);
    }
    if (stealCount>0) {
      addWarning(fmt::sprintf("%d Voice steals (more than %d simultaneous notes)",stealCount,voiceChans));
    }
    if (crowdedCount>0) {
      addWarning(fmt::sprintf("%d Notes needed an extra channel because too many landed on one row; raise Quantize for a finer grid",crowdedCount));
    }
    if (nudgeCount>0) {
      logD("MIDI import: %d notes were moved to the next row to share a column instead of taking a new one",nudgeCount);
    }
    if (partOverflow>0) {
      addWarning(fmt::sprintf("%d Parts had no channel left and share the last one",partOverflow));
    }
    if (zeroLenNotes>0) {
      addWarning(fmt::sprintf("%d Notes were shorter than one tracker tick; raise Ticks/Row for finer timing",zeroLenNotes));
    }
    if (timeSigChanges) {
      addWarning("Time signature changes are not supported; bars will drift after the first change");
    }
    if (truncated) {
      addWarning(fmt::sprintf("Song truncated to %d patterns; raise Pattern Length or lower Quantize",DIV_MAX_PATTERNS));
    }
    if (tooFast) {
      if (midiImportOptions.useBaseTempo) {
        addWarning(fmt::sprintf("Song is too fast for the maximum tick rate of %gHz; it will play back slower than the MIDI. lower Ticks/Row or Quantize",MIDI_MAX_HZ));
      } else {
        addWarning(fmt::sprintf("Song is too fast for Groove Approximation, which holds the tick rate at %gHz; it will play back slower than the MIDI. lower Quantize, or import with Base Tempo",MIDI_BASE_HZ));
      }
    }
    if (bendClamped>0) {
      addWarning(fmt::sprintf("%d Pitch bends could not land on their target row (the row was already taken, or the note range ran out) and were spread over the rows after them",bendClamped));
    }
    if (pitchBendSeen && !midiImportOptions.importPitchBend) {
      addWarning("This file has pitch bend, but importing it is turned off");
    }
    if (pitchBendSeen && midiImportOptions.importPitchBend && !rpnSeen && midiImportOptions.bendRange==0) {
      addWarning("This file bends without saying how far, so the General MIDI default of 2 semitones was used. if the bends sound too shallow, set Bend Range in the import options to whatever made the file");
    }

    if (active) quitDispatch();
    BUSY_BEGIN_SOFT;
    saveLock.lock();
    song.unload();
    song=ds;
    hasLoadedSomething=true;
    changeSong(0);
    saveLock.unlock();
    BUSY_END;
    if (active) {
      initDispatch();
      BUSY_BEGIN;
      renderSamples();
      reset();
      BUSY_END;
    }
    success=true;
  } catch (EndOfFileException& e) {
    lastError="premature end of file";
  } catch (MIDIInvalidException& e) {
  }

  for (DivMIDITrackState& tr: tracks) {
    if (tr.r!=NULL) delete tr.r;
  }
  delete[] file;
  return success;
}
