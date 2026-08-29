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

// Standard MIDI file (.mid/.midi) importer.
//
// a single merge loop runs over the MTrk chunks in ascending tick order, with
// every position converted straight from its own absolute MIDI tick by
//
//   modTicks = round(tick * quantize * ticksPerRow / (ppqn * 4))
//   row      = modTicks / ticksPerRow
//   delay    = modTicks % ticksPerRow
//
// so error is bounded at half a tracker tick per event and never accumulates.
// the leftover "delay" is not thrown away - it becomes an EDxx note delay, so
// the real timing resolution is quantize*ticksPerRow per whole note, not
// quantize. Furnace's row limit is 256, so patternLen is clamped there and
// the order list, not the pattern, carries the length.
//
// channel allocation: every part - one (track, MIDI channel) pair - owns its
// own contiguous run of Furnace channels and grows a new one whenever its
// polyphony rises, so polyphony is spread sideways rather than dropped, and
// one part's notes never land in another part's columns. only a song that
// has filled all 127 voice channels steals, and then only from the part that
// needs the voice.
//
// this is a transcription tool, not an arrangement tool: notes, timing,
// velocity (scaled by the CC7 and CC11 envelopes), sustain pedal, tempo and
// program changes are preserved on a Generic PCM DAC, but no samples are
// created and no GM timbre is emulated - the instruments come out empty and
// you assign samples to them yourself. see doc/2-interface/formats.md.

#include "fileOpsCommon.h"
#include <algorithm>
#include <climits>
#include <cmath>
#include <map>
#include <vector>

static const char* const midiGMInstrumentNames[128]={
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
static const char* const midiGMDrumNames[MIDI_DRUM_LAST-MIDI_DRUM_FIRST+1]={
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





static void midiComputeBaseGroove(int R, int tempo0, DivGroovePattern& groove) {
  double rowsPerSecond=(double)R*1000000.0/(double)tempo0;
  if (rowsPerSecond<=0.0) rowsPerSecond=8.0;
  double avgSpeed=MIDI_BASE_HZ/rowsPerSecond;
  if (avgSpeed<1.0) avgSpeed=1.0;
  if (avgSpeed>512.0) avgSpeed=512.0;

  int bestLen=1;
  int bestSum=(int)lround(avgSpeed);
  double bestErr=-1.0;
  for (int len=1; len<=16; len++) {
    
    
    
    
    
    if (R%len) continue;
    int sum=(int)lround(avgSpeed*(double)len);
    if (sum<len) sum=len;
    if (sum>512*len) sum=512*len;
    double err=fabs((double)sum/(double)len-avgSpeed);
    if (bestErr<0.0 || err<bestErr-1e-12) {
      bestErr=err;
      bestLen=len;
      bestSum=sum;
      if (err<1e-12) break;
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


struct DivMIDIChanState {
  int program;
  int volume;     
  int expression; 
  bool sustain;   
  bool monoMode;
  int noteOn[128]; 
  DivMIDIChanState():
    program(0),
    volume(127),
    expression(127),
    sustain(false),
    monoMode(false) {
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
  DivMIDIModChanState():
    midiCh(-1),
    note(-1),
    vel(0),
    age(0),
    sustained(false),
    noteRow(-1),
    prevNoteRow(-1),
    noteDelay(0) {}
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
  if (!r.seek((ssize_t)n,SEEK_CUR)) throw EndOfFileException(&r,r.size());
}


static int64_t midiMulDivR(int64_t a, int64_t b, int64_t c) {
  if (c<1) c=1;
  return (a*b+c/2)/c;
}


template<typename K> static K midiMostCommonByDuration(const std::vector<std::pair<int64_t, K> >& events, int64_t songEndTicks, K fallback) {
  if (events.empty()) return fallback;
  std::map<K, int64_t> weight;
  for (size_t i=0; i<events.size(); i++) {
    int64_t start=events[i].first;
    int64_t end=(i+1<events.size())?events[i+1].first:songEndTicks;
    int64_t dur=end-start;
    if (dur<0) dur=0;
    weight[events[i].second]+=dur;
  }
  K best=events[0].second;
  int64_t bestW=-1;
  for (typename std::map<K, int64_t>::iterator kv=weight.begin(); kv!=weight.end(); kv++) {
    if (kv->second>bestW) {
      bestW=kv->second;
      best=kv->first;
    }
  }
  return best;
}




static short midiVolumeOf(int vel, int cc7, int cc11, int maxVol, bool useVel, bool useCC7, bool useCC11) {
  double scale=1.0;
  if (useVel) scale*=(double)vel/127.0;
  if (useCC7) scale*=(double)cc7/127.0;
  if (useCC11) scale*=(double)cc11/127.0;
  int v=(int)lround(scale*(double)maxVol);
  return (short)CLAMP(v,0,maxVol);
}



static bool midiWriteEffect(short* row, unsigned char& effectCols, unsigned char fx, unsigned char val) {
  for (int i=0; i<DIV_MAX_EFFECTS; i++) {
    if (row[DIV_PAT_FX(i)]==-1 || row[DIV_PAT_FX(i)]==(short)fx) {
      row[DIV_PAT_FX(i)]=(short)fx;
      row[DIV_PAT_FXVAL(i)]=(short)val;
      if (effectCols<(unsigned char)(i+1)) effectCols=(unsigned char)(i+1);
      return true;
    }
  }
  return false;
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

    
    const int quantize=CLAMP(midiImportQuantize,4,256);
    const int patLen=CLAMP(midiImportPatternLen,1,DIV_MAX_ROWS);
    const int ticksPerRow=CLAMP(midiImportTicksPerRow,2,16);
    const int drumCh=(midiImportDrumChannel>=1 && midiImportDrumChannel<=16)?(midiImportDrumChannel-1):-1;
    
    
    
    
    
    
    const bool baseTempoMode=midiImportBaseTempo;
    const bool useDelays=baseTempoMode;

    
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
      unsigned char chunkID[4];
      reader.read(chunkID,4);
      int chunkLenS=reader.readI_BE();
      if (chunkLenS<0) throw EndOfFileException(&reader,reader.size());
      size_t chunkLen=(size_t)chunkLenS;
      if (chunkLen>reader.size()-reader.tell()) chunkLen=reader.size()-reader.tell();
      size_t chunkStart=reader.tell();
      midiSkip(reader,chunkLen);
      if (memcmp(chunkID,"MTrk",4)!=0) continue;

      DivMIDITrackState ts;
      ts.r=new SafeReader(file+chunkStart,chunkLen);
      try {
        ts.nextEvent=(int64_t)midiReadVarLen(*ts.r);
      } catch (EndOfFileException& e) {
        ts.finished=true;
        ts.nextEvent=INT64_MAX;
      }
      tracks.push_back(ts);
      trackNames.push_back(String(""));
    }
    numTracks=(int)tracks.size();
    if (numTracks<1) {
      lastError="MIDI file has no tracks";
      throw MIDIInvalidException();
    }

    
    
    
    
    const int tempoChan=DIV_MAX_CHANS-1;
    const int voiceChans=DIV_MAX_CHANS-1;

    ds.systemLen=1;
    ds.system[0]=DIV_SYSTEM_PCM_DAC;
    ds.systemChans[0]=(unsigned short)DIV_MAX_CHANS;
    ds.systemVol[0]=1.0f;
    ds.systemPan[0]=0.0f;
    ds.systemName="Generic PCM DAC";

    
    
    int chanDefIdx=0;
    DivInstrumentType pcmInsType=DivEngine::getSystemDef(ds.system[0])->getChanDef(chanDefIdx).insType[0];
    
    int maxVol=ds.systemFlags[0].getInt("volMax",255);

    DivSubSong* sub=ds.subsong[0];
    sub->patLen=patLen;

    std::vector<DivMIDIChanState> midiChan(16);
    std::vector<DivMIDIModChanState> modChan(DIV_MAX_CHANS);
    std::vector<DivMIDITempoEvent> tempoEvents;
    std::map<std::pair<int, int>, int> insMap;

    
    
    
    std::vector<DivMIDIPart> parts;
    std::map<std::pair<int, int>, int> partIndexOf;
    int nextChan=0;

    int maxOrd=0;
    int64_t songEndTicks=0;
    int stealCount=0, cutMissed=0, zeroLenNotes=0, partOverflow=0;
    int retimedCount=0, droppedCount=0, tempoClamped=0, grooveOverflow=0;
    int nudgeCount=0, crowdedCount=0;
    int timeSigNumer=4, timeSigDenom=4;
    bool haveTimeSig=false, timeSigChanges=false;
    bool truncated=false, pitchBendSeen=false;

    
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

      
      
      int64_t modTicks=midiMulDivR(tick,(int64_t)quantize*(int64_t)ticksPerRow,(int64_t)ppqn*4);
      int64_t totalRow64=modTicks/ticksPerRow;
      int delay=(int)(modTicks%ticksPerRow);
      int64_t ord64=totalRow64/patLen;
      if (ord64>=DIV_MAX_PATTERNS) {
        truncated=true;
        break;
      }
      const int totalRow=(int)totalRow64;
      const int ord=(int)ord64;
      const int row=(int)(totalRow64%patLen);

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
                if (tempo>0) tempoEvents.push_back({tick, tempo, ord, row});
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
          
          
          if (!r.seek((ssize_t)metaEnd,SEEK_SET)) throw EndOfFileException(&r,r.size());
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
          const bool isDrum=(midiCh==drumCh);

          
          
          auto noteOff=[&](int note) {
            if (note<0 || note>127) return;
            int ch=mc.noteOn[note];
            if (ch<0) return;
            if (mc.sustain && midiImportSustain) {
              
              modChan[ch].sustained=true;
              return;
            }
            modChan[ch].note=-1;
            modChan[ch].sustained=false;
            mc.noteOn[note]=-1;

            short* cell=sub->pat[ch].getPattern(ord,true)->newData[row];
            if (cell[DIV_PAT_NOTE]==-1) {
              cell[DIV_PAT_NOTE]=DIV_NOTE_OFF;
              if (useDelays && delay!=0) midiWriteEffect(cell,sub->pat[ch].effectCols,0xed,(unsigned char)delay);
              return;
            }
            
            
            if (cell[DIV_PAT_NOTE]>=DIV_NOTE_RAW || isDrum) return;

            
            
            if (useDelays) {
              
              int cut=delay;
              if (cut<=modChan[ch].noteDelay) {
                cut=modChan[ch].noteDelay+1;
                zeroLenNotes++;
              }
              if (cut<ticksPerRow) {
                if (!midiWriteEffect(cell,sub->pat[ch].effectCols,0xec,(unsigned char)cut)) cutMissed++;
              }
            } else {
              
              
              
              int offRow=totalRow+1;
              int offOrd=offRow/patLen;
              if (offOrd>=DIV_MAX_PATTERNS) return;
              short* offCell=sub->pat[ch].getPattern(offOrd,true)->newData[offRow%patLen];
              if (offCell[DIV_PAT_NOTE]==-1) {
                offCell[DIV_PAT_NOTE]=DIV_NOTE_OFF;
                retimedCount++;
              } else {
                droppedCount++;
              }
            }
          };

          switch (command&0xf0) {
            case 0x80: 
            case 0x90: { 
              int note=data1&0x7f;
              unsigned char data2=(unsigned char)r.readC();
              if (data2>0 && (command&0xf0)==0x90) {
                
                
                std::pair<int, int> partKey=std::make_pair(t,midiCh);
                std::map<std::pair<int, int>, int>::iterator partIt=partIndexOf.find(partKey);
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
                int placeOrd=placeRow/patLen;
                if (placeOrd>=DIV_MAX_PATTERNS) {
                  
                  placeRow=totalRow;
                  placeOrd=ord;
                }
                const int placeLocalRow=placeRow%patLen;
                
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
                
                
                int outNote=(isDrum && midiImportSplitDrums)?MIDI_DRUM_NOTE:CLAMP(note+MIDI_NOTE_BIAS,0,179);
                cell[DIV_PAT_NOTE]=(short)outNote;

                
                
                
                
                std::pair<int, int> insKey;
                if (isDrum) {
                  insKey=std::make_pair(1+mc.program,midiImportSplitDrums?note:-1);
                } else {
                  insKey=std::make_pair(0,mc.program);
                }
                std::map<std::pair<int, int>, int>::iterator insIt=insMap.find(insKey);
                int insIndex;
                if (insIt==insMap.end()) {
                  insIndex=(int)ds.ins.size();
                  DivInstrument* ins=new DivInstrument;
                  ins->type=pcmInsType;
                  if (insKey.first>0) {
                    int kit=insKey.first-1;
                    if (insKey.second<0) {
                      ins->name=(kit==0)?String("Standard Drum Kit"):fmt::sprintf("Drum Kit %d",kit);
                    } else {
                      if (insKey.second>=MIDI_DRUM_FIRST && insKey.second<=MIDI_DRUM_LAST) {
                        ins->name=midiGMDrumNames[insKey.second-MIDI_DRUM_FIRST];
                      } else {
                        ins->name=fmt::sprintf("Drum %d",insKey.second);
                      }
                      if (kit!=0) ins->name+=fmt::sprintf(" (Kit %d)",kit);
                    }
                  } else {
                    ins->name=midiGMInstrumentNames[insKey.second&0x7f];
                  }
                  ds.ins.push_back(ins);
                  insMap[insKey]=insIndex;
                } else {
                  insIndex=insIt->second;
                }
                cell[DIV_PAT_INS]=(short)insIndex;
                cell[DIV_PAT_VOL]=midiVolumeOf(data2,mc.volume,mc.expression,maxVol,midiImportVelocity,midiImportCC7,midiImportCC11);
                if (useDelays && placeDelay!=0) midiWriteEffect(cell,sub->pat[ch].effectCols,0xed,(unsigned char)placeDelay);
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
                    short vol=midiVolumeOf(modChan[ch].vel,mc.volume,mc.expression,maxVol,midiImportVelocity,midiImportCC7,midiImportCC11);
                    sub->pat[ch].getPattern(ord,true)->newData[row][DIV_PAT_VOL]=vol;
                  }
                  break;
                }
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
                  break;
                case 126: 
                  if (data2==0) mc.monoMode=true;
                  break;
                case 127: 
                  mc.monoMode=false;
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
              midiSkip(r,1);
              pitchBendSeen=true;
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

    
    
    
    
    std::vector<std::pair<int64_t, int> > tempoPairs;
    for (DivMIDITempoEvent& e: tempoEvents) tempoPairs.push_back(std::make_pair(e.tick,e.tempo));
    int tempo0=midiMostCommonByDuration<int>(tempoPairs,songEndTicks+1,500000);

    
    double rowsPerSecond=(double)quantize*1000000.0/(4.0*(double)tempo0);
    bool tooFast=false;
    double songHz=MIDI_BASE_HZ;
    if (baseTempoMode) {
      
      
      songHz=rowsPerSecond*(double)ticksPerRow;
      if (songHz>MIDI_MAX_HZ) {
        songHz=MIDI_MAX_HZ;
        tooFast=true;
      }
      if (songHz<1.0) songHz=1.0;
      sub->speeds.len=1;
      for (int i=0; i<16; i++) sub->speeds.val[i]=(unsigned short)ticksPerRow;
    } else {
      
      
      songHz=MIDI_BASE_HZ;
      midiComputeBaseGroove(MAX(1,quantize/4),tempo0,sub->speeds);
      tooFast=(rowsPerSecond>MIDI_BASE_HZ);
    }
    logI("MIDI import: quantize %d, %d ticks/row, %d rows/pattern, %s, tick rate %g Hz",quantize,ticksPerRow,patLen,baseTempoMode?"base tempo":"groove approximation",songHz);

    sub->hz=(float)songHz;

    
    
    
    bool anyTempoChange=false;
    int curTempoVal=tempo0;
    std::map<int, int> grooveOf;
    if (!baseTempoMode) {
      ds.grooves.push_back(sub->speeds);
      grooveOf[tempo0]=0;
    }
    int curGroove=0;
    for (DivMIDITempoEvent& te: tempoEvents) {
      if (te.tempo==curTempoVal) continue;
      if (te.order>maxOrd) continue;
      curTempoVal=te.tempo;

      int gi=0;
      if (!baseTempoMode) {
        std::map<int, int>::iterator gIt=grooveOf.find(te.tempo);
        if (gIt==grooveOf.end()) {
          DivGroovePattern g;
          midiComputeBaseGroove(MAX(1,quantize/4),te.tempo,g);
          
          
          
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
      if (baseTempoMode) {
        
        double hz=(double)quantize*1000000.0/(4.0*(double)te.tempo)*(double)ticksPerRow;
        int hzI=(int)lround(hz);
        if (hzI<1) hzI=1;
        if (hzI>1023) {
          hzI=1023;
          tempoClamped++;
        }
        midiWriteEffect(cell,sub->pat[tempoChan].effectCols,(unsigned char)(0xc0|((hzI>>8)&3)),(unsigned char)(hzI&0xff));
      } else {
        midiWriteEffect(cell,sub->pat[tempoChan].effectCols,0x09,(unsigned char)gi);
      }
      anyTempoChange=true;
    }

    
    
    int rowsPerBeat=quantize/4;
    if (rowsPerBeat<1) rowsPerBeat=1;
    int rowsPerBar=(int)lround((double)quantize*(double)timeSigNumer/(double)timeSigDenom);
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
      if (part.channel==drumCh) {
        part.name=(part.firstProgram<=0)?String("Standard Drum Kit"):fmt::sprintf("Drum Kit %d",part.firstProgram);
      } else if (part.firstProgram>=0 && part.firstProgram<128) {
        part.name=midiGMInstrumentNames[part.firstProgram];
      } else {
        part.name=fmt::sprintf("Channel %d",part.channel+1);
      }
    }
    for (size_t i=0; i<parts.size(); i++) {
      DivMIDIPart& part=parts[i];
      for (size_t j=0; j<part.voices.size(); j++) {
        
        
        
        if (j==0) {
          sub->chanName[part.voices[j]]=fmt::sprintf("MIDI CH%02d | %s",part.channel,part.name);
        } else {
          sub->chanName[part.voices[j]]=fmt::sprintf("MIDI CH%02d | %s | [Polyphony: %d]",part.channel,part.name,(int)j);
        }
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

    sub->removeUnusedPatterns();
    sub->optimizePatterns();
    
    
    
    
    
    
    
    
    

    ds.recalcChans();

    addWarning("no samples were created - assign samples to the instruments, or change the chip, to hear anything");
    if (retimedCount>0) {
      addWarning(fmt::sprintf("%d note-offs were moved a row later by quantization",retimedCount));
    }
    if (tempoClamped>0) {
      addWarning(fmt::sprintf("%d tempo changes were faster than the 1023Hz tick rate limit; lower Ticks/Row or Quantize",tempoClamped));
    }
    if (grooveOverflow>0) {
      addWarning(fmt::sprintf("%d tempo changes did not fit in the 256-groove table and were left out",grooveOverflow));
    }
    if (droppedCount>0) {
      logD("MIDI import: %d note-offs fell on a retrigger row and were left out",droppedCount);
    }
    if (stealCount>0) {
      addWarning(fmt::sprintf("%d voice steals (more than %d simultaneous notes)",stealCount,voiceChans));
    }
    if (crowdedCount>0) {
      addWarning(fmt::sprintf("%d notes needed an extra channel because too many landed on one row; raise Quantize for a finer grid",crowdedCount));
    }
    if (nudgeCount>0) {
      logD("MIDI import: %d notes were moved to the next row to share a column instead of taking a new one",nudgeCount);
    }
    if (partOverflow>0) {
      addWarning(fmt::sprintf("%d parts had no channel left and share the last one",partOverflow));
    }
    if (zeroLenNotes>0) {
      addWarning(fmt::sprintf("%d notes were shorter than one tracker tick; raise Ticks/Row for finer timing",zeroLenNotes));
    }
    if (cutMissed>0) {
      logD("MIDI import: %d note cuts did not fit in the effect columns",cutMissed);
    }
    if (timeSigChanges) {
      addWarning("time signature changes are not supported; bars will drift after the first change");
    }
    if (truncated) {
      addWarning(fmt::sprintf("song truncated to %d patterns; raise Pattern Length or lower Quantize",DIV_MAX_PATTERNS));
    }
    if (tooFast) {
      if (baseTempoMode) {
        addWarning(fmt::sprintf("song is too fast for the maximum tick rate of %gHz; it will play back slower than the MIDI. lower Ticks/Row or Quantize",MIDI_MAX_HZ));
      } else {
        addWarning(fmt::sprintf("song is too fast for Groove Approximation, which holds the tick rate at %gHz; it will play back slower than the MIDI. lower Quantize, or import with Base Tempo",MIDI_BASE_HZ));
      }
    }
    if (pitchBendSeen) {
      addWarning("pitch bend is not imported");
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
