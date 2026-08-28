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
// this is a transcription tool, not an arrangement tool: notes, timing,
// velocity (scaled by the CC7 and CC11 envelopes) and program changes are
// preserved on a Generic PCM DAC (up to 128 channels, one per simultaneous
// MIDI voice), but no samples are created and no GM timbre is emulated.
// see doc/2-interface/formats.md.

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

// the order matters: within one tick, note-offs are applied before controller
// changes, and controller changes before note-ons. that way a note starting on
// the same tick as a CC picks up the new value, and a legato retrigger does not
// read as a false polyphony collision
enum DivMIDIEventType {
  DIV_MIDI_NOTE_OFF=0,
  DIV_MIDI_CC,
  DIV_MIDI_NOTE_ON
};

struct DivMIDIEvent {
  int tick, track, channel;
  unsigned char type;
  // note/vel for DIV_MIDI_NOTE_ON and DIV_MIDI_NOTE_OFF
  short note, vel;
  // program in effect on this channel when the note started (-1 if none)
  short program;
  // controller number and its new value, for DIV_MIDI_CC
  unsigned char cc;
  short ccVal;
};

struct DivMIDITempoEvent {
  int tick, tempo;
};

struct DivMIDITimeSigEvent {
  int tick, numer, denom;
};

struct DivMIDIPart {
  int track, channel;
  int poolStart, poolSize;
  String name;
};

struct MIDIInvalidException {
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

static int midiGCD(int a, int b) {
  if (a<0) a=-a;
  if (b<0) b=-b;
  while (b!=0) {
    int t=b;
    b=a%b;
    a=t;
  }
  return (a<1)?1:a;
}

// best rational approximation of x with numerator and denominator in 1..limit.
// continued fractions, with the semiconvergent check so we do not fall back to a
// much worse convergent when the next one overflows the limit.
static void midiBestRational(double x, int limit, int& p, int& q) {
  if (x<=0.0) {
    p=1;
    q=1;
    return;
  }
  int p0=0, q0=1, p1=1, q1=0;
  double val=x;
  for (int i=0; i<32; i++) {
    double a=floor(val);
    int ai=(a>(double)limit)?limit:(int)a;
    int p2=ai*p1+p0;
    int q2=ai*q1+q0;

    if (p2>limit || q2>limit) {
      // the next convergent does not fit - try the best semiconvergent instead
      int k=ai;
      if (p1>0 && (limit-p0)/p1<k) k=(limit-p0)/p1;
      if (q1>0 && (limit-q0)/q1<k) k=(limit-q0)/q1;
      if (k>0) {
        int semiP=k*p1+p0;
        int semiQ=k*q1+q0;
        if (fabs((double)semiP/(double)semiQ-x)<fabs((double)p1/(double)q1-x)) {
          p1=semiP;
          q1=semiQ;
        }
      }
      break;
    }
    p0=p1;
    q0=q1;
    p1=p2;
    q1=q2;
    double frac=val-a;
    if (frac<1e-9) break;
    val=1.0/frac;
  }
  if (p1<1 || q1<1) {
    p=1;
    q=1;
    return;
  }
  p=p1;
  q=q1;
}

// the default tick rate, which groove approximation mode never touches - there
// the base tempo rides in the groove instead
#define MIDI_BASE_HZ 60.0

// the ceiling the Speed window enforces on the tick rate
#define MIDI_MAX_HZ 999.0

// solve the average ticks per row into a groove of at most 16 integer entries,
// spreading the remainder one row at a time (Bresenham) instead of front-loading it
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

// velocity, CC7 (channel volume) and CC11 (expression) multiply together, the
// same way they do on a MIDI synth. any of the three can be turned off in the
// import dialog; with all of them off the column sits at maximum
static short midiVolumeOf(short vel, short cc7, short cc11, int maxVol, bool useVel, bool useCC7, bool useCC11) {
  double scale=1.0;
  if (useVel) scale*=(double)vel/127.0;
  if (useCC7) scale*=(double)cc7/127.0;
  if (useCC11) scale*=(double)cc11/127.0;
  return (short)lround(scale*(double)maxVol);
}

// pick the value that is in effect for the longest, not the one that appears most
template<typename K> static K midiMostCommonByDuration(const std::vector<std::pair<int, K> >& events, int songEndTicks, K fallback) {
  if (events.empty()) return fallback;
  std::map<K, int64_t> weight;
  for (size_t i=0; i<events.size(); i++) {
    int start=events[i].first;
    int end=(i+1<events.size())?events[i+1].first:songEndTicks;
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

#ifdef DIV_MIDI_SELFTEST
#include <cassert>
#include <cstdlib>
static void midiSelfTest() {
  // varlen round-trip, the canonical SMF spec table
  struct VarLenCase {
    unsigned char bytes[4];
    int len;
    unsigned int expect;
  };
  VarLenCase cases[6]={
    {{0x00, 0, 0, 0}, 1, 0x00},
    {{0x7f, 0, 0, 0}, 1, 0x7f},
    {{0x81, 0x00, 0, 0}, 2, 0x80},
    {{0xc0, 0x00, 0, 0}, 2, 0x2000},
    {{0xff, 0x7f, 0, 0}, 2, 0x3fff},
    {{0xff, 0xff, 0xff, 0x7f}, 4, 0x0fffffff}
  };
  for (int i=0; i<6; i++) {
    SafeReader r(cases[i].bytes,cases[i].len);
    assert(midiReadVarLen(r)==cases[i].expect);
  }

  int p, q;
  midiBestRational(0.8,255,p,q);
  assert(p==4 && q==5);
  midiBestRational(14.0/15.0,255,p,q);
  assert(p==14 && q==15);

  // never worse than an exhaustive search over the same limit
  for (int i=0; i<1000; i++) {
    double x=1.0/16.0+((double)rand()/(double)RAND_MAX)*(16.0-1.0/16.0);
    midiBestRational(x,255,p,q);
    double bruteErr=fabs(1.0-x);
    for (int qq=1; qq<=255; qq++) {
      int pp=CLAMP((int)lround(x*qq),1,255);
      double e=fabs((double)pp/(double)qq-x);
      if (e<bruteErr) bruteErr=e;
    }
    assert(fabs((double)p/(double)q-x)<=bruteErr+1e-12);
  }

  DivGroovePattern g;
  // 90 BPM at 8 rows per quarter is exactly 5 ticks per row
  midiComputeBaseGroove(8,666667,g);
  assert(g.len==1 && g.val[0]==5);
  // 120 BPM at 4 rows per quarter is 7.5, so 7 8
  midiComputeBaseGroove(4,500000,g);
  assert(g.len==2 && g.val[0]+g.val[1]==15);
  midiComputeBaseGroove(16,500000,g);
  assert(g.len==8 && g.val[0]+g.val[1]+g.val[2]+g.val[3]+g.val[4]+g.val[5]+g.val[6]+g.val[7]==15);

  for (int Ri=0; Ri<6; Ri++) {
    static const int rCand[6]={4, 8, 12, 16, 24, 32};
    for (int bpm=40; bpm<=240; bpm++) {
      int tempo0=(int)lround(60000000.0/(double)bpm);
      double rowsPerSecond=(double)rCand[Ri]*1000000.0/(double)tempo0;
      if (rowsPerSecond>MIDI_BASE_HZ) continue;
      midiComputeBaseGroove(rCand[Ri],tempo0,g);
      assert(g.len>=1 && g.len<=16);
      double sum=0.0;
      for (int i=0; i<g.len; i++) {
        assert(g.val[i]>=1);
        sum+=(double)g.val[i];
      }
      double gotRows=MIDI_BASE_HZ/(sum/(double)g.len);
      assert(fabs(gotRows-rowsPerSecond)/rowsPerSecond<0.04);
    }
  }
}
#endif

bool DivEngine::loadMIDI(unsigned char* file, size_t len) {
  bool success=false;
  SafeReader reader=SafeReader(file,len);
  warnings="";

#ifdef DIV_MIDI_SELFTEST
  midiSelfTest();
#endif

  try {
    DivSong ds;
    ds.version=DIV_VERSION_MIDI;

    reader.seek(4,SEEK_SET);
    int headerLen=reader.readI_BE();
    if (headerLen<6) throw EndOfFileException(&reader,reader.size());
    int format=reader.readS_BE();
    unsigned short numTracks=(unsigned short)reader.readS_BE();
    unsigned short division=(unsigned short)reader.readS_BE();
    if (headerLen>6) midiSkip(reader,(size_t)(headerLen-6));
    logD("MIDI import: format %d, %d tracks, division %d",format,numTracks,division);

    if (format==2) {
      lastError="MIDI format 2 is not supported";
      throw MIDIInvalidException();
    }

    int TPQN=1;
    if (division&0x8000) {
      // SMPTE: ticks are absolute time. pin a nominal 120 BPM to get an
      // equivalent ticks-per-quarter, so the rest of the importer does not care
      int fps=-(signed char)((division>>8)&0xff);
      int tpf=division&0xff;
      if (fps<=0 || tpf<=0) {
        lastError="invalid SMPTE division";
        throw MIDIInvalidException();
      }
      TPQN=(int)lround((double)fps*(double)tpf*0.5);
      if (TPQN<1) TPQN=1;
    } else {
      TPQN=division;
      if (TPQN<1) TPQN=1;
    }

    std::vector<DivMIDIEvent> events;
    std::vector<DivMIDITempoEvent> tempoEvents;
    std::vector<DivMIDITimeSigEvent> timeSigEvents;
    std::vector<String> trackNames;

    while (reader.tell()+8<=reader.size()) {
      unsigned char chunkID[4];
      reader.read(chunkID,4);
      int chunkLenS=reader.readI_BE();
      if (chunkLenS<0) throw EndOfFileException(&reader,reader.size());
      size_t chunkLen=(size_t)chunkLenS;
      if (chunkLen>reader.size()-reader.tell()) throw EndOfFileException(&reader,reader.size());
      size_t chunkEnd=reader.tell()+chunkLen;

      if (memcmp(chunkID,"MTrk",4)!=0) {
        midiSkip(reader,chunkLen);
        continue;
      }

      int trackIndex=(int)trackNames.size();
      trackNames.push_back(String(""));

      int runningTick=0;
      unsigned char runningStatus=0;
      short curProgram[16];
      for (int i=0; i<16; i++) curProgram[i]=-1;

      while (reader.tell()<chunkEnd) {
        runningTick+=(int)midiReadVarLen(reader);

        unsigned char b=(unsigned char)reader.readC();
        unsigned char status;
        unsigned char data1;
        bool haveData1=false;

        if (b&0x80) {
          status=b;
          if (status==0xff) {
            unsigned char metaType=(unsigned char)reader.readC();
            unsigned int metaLen=midiReadVarLen(reader);
            switch (metaType) {
              case 0x03: // track name
                if (trackNames[trackIndex].empty()) {
                  trackNames[trackIndex]=reader.readString((size_t)metaLen);
                } else {
                  midiSkip(reader,metaLen);
                }
                break;
              case 0x51: // set tempo
                if (metaLen>=3) {
                  unsigned char t0=(unsigned char)reader.readC();
                  unsigned char t1=(unsigned char)reader.readC();
                  unsigned char t2=(unsigned char)reader.readC();
                  int tempo=(t0<<16)|(t1<<8)|t2;
                  if (tempo>0) tempoEvents.push_back({runningTick, tempo});
                  if (metaLen>3) midiSkip(reader,metaLen-3);
                } else {
                  midiSkip(reader,metaLen);
                }
                break;
              case 0x58: // time signature
                if (metaLen>=2) {
                  int numer=(unsigned char)reader.readC();
                  int denomPow=(unsigned char)reader.readC();
                  if (denomPow>10) denomPow=10;
                  if (metaLen>2) midiSkip(reader,metaLen-2);
                  if (numer>0) timeSigEvents.push_back({runningTick, numer, 1<<denomPow});
                } else {
                  midiSkip(reader,metaLen);
                }
                break;
              default: // unknown meta event - skip it, do not fail the load
                midiSkip(reader,metaLen);
                break;
            }
            continue;
          } else if (status==0xf0 || status==0xf7) {
            // SysEx - skip by length rather than scanning for the terminator
            unsigned int sysexLen=midiReadVarLen(reader);
            midiSkip(reader,sysexLen);
            continue;
          } else {
            runningStatus=status;
            haveData1=false;
          }
        } else {
          status=runningStatus;
          data1=b;
          haveData1=true;
        }

        if (status<0x80) continue;
        if (status>=0xf0) break;

        unsigned char type=status&0xf0;
        unsigned char channel=status&0x0f;
        if (!haveData1) data1=(unsigned char)reader.readC();
        unsigned char data2=0;
        if (type!=0xc0 && type!=0xd0) data2=(unsigned char)reader.readC();

        switch (type) {
          case 0x80: // note off
            events.push_back({runningTick, trackIndex, (int)channel, DIV_MIDI_NOTE_OFF, (short)(data1&0x7f), (short)0, (short)-1, 0, (short)0});
            break;
          case 0x90: // note on - velocity 0 means off
            if (data2==0) {
              events.push_back({runningTick, trackIndex, (int)channel, DIV_MIDI_NOTE_OFF, (short)(data1&0x7f), (short)0, (short)-1, 0, (short)0});
            } else {
              events.push_back({runningTick, trackIndex, (int)channel, DIV_MIDI_NOTE_ON, (short)(data1&0x7f), (short)data2, curProgram[channel], 0, (short)0});
            }
            break;
          case 0xb0: // control change - only CC7 and CC11 are kept
            if (data1==7 || data1==11) {
              events.push_back({runningTick, trackIndex, (int)channel, DIV_MIDI_CC, (short)0, (short)0, (short)-1, data1, (short)(data2&0x7f)});
            }
            break;
          case 0xc0: // program change
            curProgram[channel]=(short)data1;
            break;
          default: // everything else (other CC, pitch bend, aftertouch) is discarded
            break;
        }
      }

      reader.seek((ssize_t)chunkEnd,SEEK_SET);
    }

    if (events.empty()) {
      lastError="no notes found in MIDI file";
      throw MIDIInvalidException();
    }

    // see DivMIDIEventType for why the type is the tiebreaker
    std::stable_sort(events.begin(),events.end(),[](const DivMIDIEvent& a, const DivMIDIEvent& b) -> bool {
      if (a.tick!=b.tick) return a.tick<b.tick;
      return a.type<b.type;
    });
    std::stable_sort(tempoEvents.begin(),tempoEvents.end(),[](const DivMIDITempoEvent& a, const DivMIDITempoEvent& b) -> bool {
      return a.tick<b.tick;
    });
    std::stable_sort(timeSigEvents.begin(),timeSigEvents.end(),[](const DivMIDITimeSigEvent& a, const DivMIDITimeSigEvent& b) -> bool {
      return a.tick<b.tick;
    });

    int songEndTicks=0;
    for (DivMIDIEvent& e: events) {
      if (e.tick>songEndTicks) songEndTicks=e.tick;
    }
    for (DivMIDITempoEvent& e: tempoEvents) {
      if (e.tick>songEndTicks) songEndTicks=e.tick;
    }
    for (DivMIDITimeSigEvent& e: timeSigEvents) {
      if (e.tick>songEndTicks) songEndTicks=e.tick;
    }
    if (songEndTicks<TPQN) songEndTicks=TPQN;

    std::vector<std::pair<int, int> > tempoPairs;
    for (DivMIDITempoEvent& e: tempoEvents) tempoPairs.push_back({e.tick, e.tempo});
    int tempo0=midiMostCommonByDuration<int>(tempoPairs,songEndTicks,500000);

    std::vector<std::pair<int, std::pair<int, int> > > sigPairs;
    for (DivMIDITimeSigEvent& e: timeSigEvents) sigPairs.push_back({e.tick, {e.numer, e.denom}});
    std::pair<int, int> baseSig=midiMostCommonByDuration<std::pair<int, int> >(sigPairs,songEndTicks,{4, 4});
    int numer=baseSig.first;
    int denom=baseSig.second;
    bool timeSigChanges=false;
    for (DivMIDITimeSigEvent& e: timeSigEvents) {
      if (e.numer!=numer || e.denom!=denom) {
        timeSigChanges=true;
        break;
      }
    }

    // rows per quarter note. the smallest one that captures the groove keeps
    // patterns short; 12 and 24 are there so triplets are not mangled
    static const int midiRCandidates[6]={4, 8, 12, 16, 24, 32};
    int totalNoteOns=0;
    for (DivMIDIEvent& e: events) {
      if (e.type==DIV_MIDI_NOTE_ON) totalNoteOns++;
    }

    // a row cannot be shorter than one tick, so at a fixed tick rate this caps
    // how fine the grid gets. in base tempo mode the tick rate follows the song,
    // so there is nothing to cap against
    double rMax=midiImportBaseTempo?1e9:(MIDI_BASE_HZ*(double)tempo0/1000000.0);
    int R=4;
    for (int ci=0; ci<6; ci++) {
      if (midiRCandidates[ci]>16) break;
      if ((double)midiRCandidates[ci]<=rMax) R=midiRCandidates[ci];
    }
    if (totalNoteOns>0) {
      bool foundR=false;
      double lastPct=0.0;
      for (int ci=0; ci<6; ci++) {
        int cand=midiRCandidates[ci];
        if ((double)cand>rMax) break;
        int onGrid=0;
        for (DivMIDIEvent& e: events) {
          if (e.type!=DIV_MIDI_NOTE_ON) continue;
          double scaled=(double)e.tick*(double)cand/(double)TPQN;
          if (fabs(scaled-(double)llround(scaled))<0.05) onGrid++;
        }
        double pct=(double)onGrid/(double)totalNoteOns;
        lastPct=pct;
        if (pct>=0.95) {
          R=cand;
          foundR=true;
          logI("MIDI import: R=%d (%.1f%% of notes on-grid)",R,pct*100.0);
          break;
        }
      }
      if (!foundR) {
        logI("MIDI import: R auto-detect found no good fit (best %.1f%% on-grid), falling back to %d",lastPct*100.0,R);
      }
    }

    int rowsPerBar=R*numer*4/denom;
    if (rowsPerBar<1) rowsPerBar=1;
    int barsPerPattern=64/rowsPerBar;
    if (barsPerPattern<1) barsPerPattern=1;
    int patLen=rowsPerBar*barsPerPattern;
    if (patLen>DIV_MAX_ROWS) patLen=DIV_MAX_ROWS;
    if (patLen<1) patLen=1;

    DivGroovePattern baseGroove;
    double songHz=MIDI_BASE_HZ;
    bool tooFast=false;
    if (midiImportBaseTempo) {
      // solve the tick rate instead of the groove. the speed stays flat, so
      // the tempo is exact and the Base Tempo field reads the song's own BPM
      // (hz*2.5 = R*BPM*speed/24, which is BPM itself whenever R*speed is 24)
      double rowsPerSecond=(double)R*1000000.0/(double)tempo0;
      int speed=6;
      while (speed>1 && rowsPerSecond*(double)speed>MIDI_MAX_HZ) speed--;
      songHz=rowsPerSecond*(double)speed;
      if (songHz>MIDI_MAX_HZ) {
        songHz=MIDI_MAX_HZ;
        tooFast=true;
      }
      if (songHz<1.0) songHz=1.0;
      baseGroove.len=1;
      for (int i=0; i<16; i++) baseGroove.val[i]=(unsigned short)speed;
      logI("MIDI import: base tempo mode - tick rate %g Hz, speed %d",songHz,speed);
    } else {
      midiComputeBaseGroove(R,tempo0,baseGroove);
      tooFast=(rMax<4.0);
    }

    int totalRows=(int)ceil((double)songEndTicks*(double)R/(double)TPQN)+1;
    if (totalRows<1) totalRows=1;
    int ordersLen=(int)ceil((double)totalRows/(double)patLen);
    if (ordersLen<1) ordersLen=1;
    bool truncated=false;
    if (ordersLen>DIV_MAX_PATTERNS) {
      ordersLen=DIV_MAX_PATTERNS;
      truncated=true;
    }
    int maxRow=ordersLen*patLen-1;

    // a part is a distinct (track, channel) pair - this handles both type 0
    // (one track, 16 channels) and type 1 (one part per track) with no user input
    std::vector<DivMIDIPart> parts;
    std::map<std::pair<int, int>, int> partIndexOf;
    std::vector<int> curActive, maxActive;
    std::vector<short> firstProgram;

    // only notes create parts. a controller change on a channel that never
    // plays anything has nothing to apply to
    for (DivMIDIEvent& e: events) {
      if (e.type==DIV_MIDI_CC) continue;
      std::pair<int, int> key={e.track, e.channel};
      std::map<std::pair<int, int>, int>::iterator it=partIndexOf.find(key);
      int pi;
      if (it==partIndexOf.end()) {
        pi=(int)parts.size();
        partIndexOf[key]=pi;
        DivMIDIPart p;
        p.track=e.track;
        p.channel=e.channel;
        p.poolStart=0;
        p.poolSize=0;
        parts.push_back(p);
        curActive.push_back(0);
        maxActive.push_back(0);
        firstProgram.push_back(-1);
      } else {
        pi=it->second;
      }
      if (e.type==DIV_MIDI_NOTE_ON) {
        curActive[pi]++;
        if (curActive[pi]>maxActive[pi]) maxActive[pi]=curActive[pi];
        if (firstProgram[pi]<0 && e.program>=0) firstProgram[pi]=e.program;
      } else if (curActive[pi]>0) {
        curActive[pi]--;
      }
    }
    if (parts.empty()) {
      lastError="no notes found in MIDI file";
      throw MIDIInvalidException();
    }
    // each part gets a pool sized to its own worst-case polyphony, so in the
    // common case nothing is ever stolen
    for (size_t i=0; i<parts.size(); i++) parts[i].poolSize=MAX(1,maxActive[i]);

    int totalVoices=0;
    for (DivMIDIPart& p: parts) totalVoices+=p.poolSize;
    int stealCount=0;
    if (totalVoices>DIV_MAX_CHANS) {
      double scale=(double)DIV_MAX_CHANS/(double)totalVoices;
      for (DivMIDIPart& p: parts) p.poolSize=MAX(1,(int)floor((double)p.poolSize*scale));
    }
    int cursor=0;
    for (DivMIDIPart& p: parts) {
      if (cursor>=DIV_MAX_CHANS) {
        p.poolStart=DIV_MAX_CHANS-1;
        p.poolSize=1;
      } else {
        int sz=MIN(p.poolSize,DIV_MAX_CHANS-cursor);
        if (sz<1) sz=1;
        p.poolStart=cursor;
        p.poolSize=sz;
        cursor+=sz;
      }
    }
    int numChans=CLAMP(cursor,1,DIV_MAX_CHANS);

    ds.systemLen=1;
    ds.system[0]=DIV_SYSTEM_PCM_DAC;
    ds.systemChans[0]=(unsigned short)numChans;
    ds.systemVol[0]=1.0f;
    ds.systemPan[0]=0.0f;
    ds.systemName="Generic PCM DAC";

    // read the instrument type off the chip definition rather than hardcoding
    // it - getPreferInsType() would read the dispatch of the song being replaced
    int chanDefIdx=0;
    DivInstrumentType pcmInsType=DivEngine::getSystemDef(ds.system[0])->getChanDef(chanDefIdx).insType[0];

    for (size_t i=0; i<parts.size(); i++) {
      DivMIDIPart& p=parts[i];
      if (!trackNames[p.track].empty()) {
        p.name=trackNames[p.track];
      } else if (p.channel==9) {
        p.name="Standard Drum Kit";
      } else if (firstProgram[i]>=0 && firstProgram[i]<128) {
        p.name=midiGMInstrumentNames[firstProgram[i]];
      } else {
        p.name=fmt::sprintf("Channel %d",p.channel+1);
      }
    }

    DivSubSong* sub=ds.subsong[0];
    sub->speeds=baseGroove;
    sub->hz=(float)songHz;
    sub->virtualTempoN=150;
    sub->virtualTempoD=150;
    sub->patLen=patLen;
    sub->ordersLen=ordersLen;
    // hilightA drives the displayed BPM, so it has to be the beat division
    sub->hilightA=(unsigned char)CLAMP(R,1,255);
    sub->hilightB=(unsigned char)CLAMP(rowsPerBar,1,255);

    for (int ch=0; ch<numChans; ch++) {
      for (int o=0; o<ordersLen; o++) sub->orders.ord[ch][o]=(unsigned char)o;
    }
    for (DivMIDIPart& p: parts) {
      for (int j=0; j<p.poolSize; j++) {
        sub->chanName[p.poolStart+j]=fmt::sprintf("%s %d",p.name,j+1);
      }
    }

    std::map<std::pair<int, int>, int> insMap;
    bool voiceActive[DIV_MAX_CHANS];
    short voiceNote[DIV_MAX_CHANS];
    int voiceStartTick[DIV_MAX_CHANS];
    int voiceStartRow[DIV_MAX_CHANS];
    short lastVol[DIV_MAX_CHANS];
    bool chanUsed[DIV_MAX_CHANS];
    for (int i=0; i<DIV_MAX_CHANS; i++) {
      voiceActive[i]=false;
      voiceNote[i]=-1;
      voiceStartTick[i]=0;
      voiceStartRow[i]=0;
      lastVol[i]=-1;
      chanUsed[i]=false;
    }

    // PCM DAC's volMax is 255, so 7-bit velocity survives essentially losslessly
    int maxVol=ds.systemFlags[0].getInt("volMax",255);
    int retimedCount=0, droppedCount=0;

    // running controller state per part. GM says both default to 100, but a
    // file that never sends them shouldn't come in quieter than one that sends
    // 127, so start open
    std::vector<short> partCC7(parts.size(),127);
    std::vector<short> partCC11(parts.size(),127);
    // velocity of the note each voice is currently holding, so a controller
    // change can recompute that voice's volume without losing the velocity
    short voiceVel[DIV_MAX_CHANS];
    for (int i=0; i<DIV_MAX_CHANS; i++) voiceVel[i]=0;

    for (DivMIDIEvent& ev: events) {
      std::map<std::pair<int, int>, int>::iterator partIt=partIndexOf.find({ev.track, ev.channel});
      if (partIt==partIndexOf.end()) continue;
      int pi=partIt->second;
      DivMIDIPart& part=parts[pi];

      int totalRow=(int)llround((double)ev.tick*(double)R/(double)TPQN);
      if (totalRow<0) totalRow=0;
      if (totalRow>maxRow) totalRow=maxRow;
      int order=totalRow/patLen;
      int localRow=totalRow%patLen;

      if (ev.type==DIV_MIDI_CC) {
        if (ev.cc==7) partCC7[pi]=ev.ccVal;
        if (ev.cc==11) partCC11[pi]=ev.ccVal;
        // follow the envelope: retarget every voice this part is currently
        // holding. the volume column applies on a row with no note, so this is
        // just a write at the quantized row
        for (int j=0; j<part.poolSize; j++) {
          int ch=part.poolStart+j;
          if (!voiceActive[ch]) continue;
          short vol=midiVolumeOf(voiceVel[ch],partCC7[pi],partCC11[pi],maxVol,midiImportVelocity,midiImportCC7,midiImportCC11);
          if (vol==lastVol[ch]) continue;
          DivPattern* ccPat=sub->pat[ch].getPattern(order,true);
          ccPat->newData[localRow][DIV_PAT_VOL]=vol;
          lastVol[ch]=vol;
        }
        continue;
      }

      if (ev.type==DIV_MIDI_NOTE_ON) {
        int chosen=-1;
        for (int j=0; j<part.poolSize; j++) {
          int ch=part.poolStart+j;
          if (!voiceActive[ch]) {
            chosen=ch;
            break;
          }
        }
        if (chosen<0) {
          // no free voice - steal the oldest one. only reachable past 128 voices
          int oldest=-1, oldestTick=INT_MAX;
          for (int j=0; j<part.poolSize; j++) {
            int ch=part.poolStart+j;
            if (voiceActive[ch] && voiceStartTick[ch]<oldestTick) {
              oldestTick=voiceStartTick[ch];
              oldest=ch;
            }
          }
          chosen=(oldest>=0)?oldest:part.poolStart;
          stealCount++;
        }

        DivPattern* pat=sub->pat[chosen].getPattern(order,true);
        // MIDI 60 (C4) maps to Furnace 108, which is C-4
        pat->newData[localRow][DIV_PAT_NOTE]=(short)(ev.note+48);

        // key on the timbre, not the part - two tracks on the same GM program
        // share one instrument. drums are their own namespace (program 0 is a
        // kit on channel 10, a piano everywhere else)
        std::pair<int, int> insKey={(part.channel==9)?1:0, (int)ev.program};
        std::map<std::pair<int, int>, int>::iterator insIt=insMap.find(insKey);
        int insIndex;
        if (insIt==insMap.end()) {
          insIndex=(int)ds.ins.size();
          DivInstrument* ins=new DivInstrument;
          ins->type=pcmInsType;
          if (ev.program<0) {
            ins->name="Default";
          } else if (part.channel==9) {
            ins->name=(ev.program==0)?String("Standard Drum Kit"):fmt::sprintf("Drum Kit %d",ev.program);
          } else if (ev.program<128) {
            ins->name=midiGMInstrumentNames[ev.program];
          } else {
            ins->name=fmt::sprintf("Program %d",ev.program);
          }
          ds.ins.push_back(ins);
          insMap[insKey]=insIndex;
        } else {
          insIndex=insIt->second;
        }
        pat->newData[localRow][DIV_PAT_INS]=insIndex;

        short vol=midiVolumeOf(ev.vel,partCC7[pi],partCC11[pi],maxVol,midiImportVelocity,midiImportCC7,midiImportCC11);
        if (vol!=lastVol[chosen]) {
          pat->newData[localRow][DIV_PAT_VOL]=vol;
          lastVol[chosen]=vol;
        }

        voiceActive[chosen]=true;
        voiceNote[chosen]=ev.note;
        voiceVel[chosen]=ev.vel;
        voiceStartTick[chosen]=ev.tick;
        voiceStartRow[chosen]=totalRow;
        chanUsed[chosen]=true;
      } else {
        int found=-1;
        for (int j=0; j<part.poolSize; j++) {
          int ch=part.poolStart+j;
          if (voiceActive[ch] && voiceNote[ch]==ev.note) {
            found=ch;
            break;
          }
        }
        if (found>=0) {
          // a note-off landing on its own note-on's row gets pushed one row down,
          // unless that row already has a note - the retrigger covers it anyway
          int offRow=totalRow;
          bool pushed=false;
          if (offRow==voiceStartRow[found]) {
            offRow=MIN(offRow+1,maxRow);
            pushed=true;
          }
          DivPattern* offPat=sub->pat[found].getPattern(offRow/patLen,true);
          int offLocalRow=offRow%patLen;
          if (pushed && offPat->newData[offLocalRow][DIV_PAT_NOTE]!=-1) {
            droppedCount++;
          } else {
            offPat->newData[offLocalRow][DIV_PAT_NOTE]=DIV_NOTE_OFF;
            if (pushed) retimedCount++;
          }
          voiceActive[found]=false;
        }
      }
    }

    for (int ch=0; ch<numChans; ch++) {
      if (!chanUsed[ch]) sub->chanShow[ch]=false;
    }

    // hz is fixed for the whole subsong, so later tempos are expressed as an
    // exact ratio against the base one via virtual tempo
    bool anyTempoChange=false;
    bool approxTempoUsed=false;
    int curTempoVal=tempo0;
    for (DivMIDITempoEvent& te: tempoEvents) {
      if (te.tempo==curTempoVal) continue;

      int totalRow=(int)llround((double)te.tick*(double)R/(double)TPQN);
      if (totalRow<0) totalRow=0;
      if (totalRow>maxRow) totalRow=maxRow;

      int g=midiGCD(tempo0,te.tempo);
      int N=tempo0/g;
      int D=te.tempo/g;
      if (N>255 || D>255) {
        midiBestRational((double)tempo0/(double)te.tempo,255,N,D);
        approxTempoUsed=true;
      }
      N=CLAMP(N,1,255);
      D=CLAMP(D,1,255);

      DivPattern* pat0=sub->pat[0].getPattern(totalRow/patLen,true);
      int localRow=totalRow%patLen;
      pat0->newData[localRow][DIV_PAT_FX(0)]=0xfd;
      pat0->newData[localRow][DIV_PAT_FXVAL(0)]=N;
      pat0->newData[localRow][DIV_PAT_FX(1)]=0xfe;
      pat0->newData[localRow][DIV_PAT_FXVAL(1)]=D;

      curTempoVal=te.tempo;
      anyTempoChange=true;
    }
    if (anyTempoChange && sub->pat[0].effectCols<2) sub->pat[0].effectCols=2;

    ds.insLen=(int)ds.ins.size();

    sub->removeUnusedPatterns();
    sub->optimizePatterns();
    sub->rearrangePatterns();

    ds.recalcChans();

    addWarning("no samples were created - assign samples to the instruments, or change the chip, to hear anything");
    if (retimedCount>0) {
      addWarning(fmt::sprintf("%d notes were re-timed by quantization",retimedCount));
    }
    if (droppedCount>0) {
      addWarning(fmt::sprintf("%d note-offs were dropped due to quantization",droppedCount));
    }
    if (stealCount>0) {
      addWarning(fmt::sprintf("%d voice steals (more than %d simultaneous notes)",stealCount,DIV_MAX_CHANS));
    }
    if (timeSigChanges) {
      addWarning("time signature changes are not supported; bars will drift after the first change");
    }
    if (truncated) {
      addWarning("song truncated to 256 orders");
    }
    if (tooFast) {
      if (midiImportBaseTempo) {
        addWarning("Song is too fast for the maximum tick rate of 999Hz; it will play back slower than the MIDI");
      } else {
        addWarning("Song is too fast for the default tick rate; it will play back slower than the MIDI (raise the tick rate, or import again with Base Tempo, to fix)");
      }
    }
    if (approxTempoUsed) {
      addWarning("a tempo change ratio did not fit exactly and was approximated");
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
    // lastError is already set at the throw site
  }

  delete[] file;
  return success;
}
