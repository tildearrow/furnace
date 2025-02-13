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

#include "fileOpsCommon.h"

unsigned char fcXORTriangle[32]={
  0xc0, 0xc0, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8, 0x00, 0xf8, 0xf0, 0xe8, 0xe0, 0xd8, 0xd0, 0xc8,
  0xc0, 0xb8, 0xb0, 0xa8, 0xa0, 0x98, 0x90, 0x88, 0x80, 0x88, 0x90, 0x98, 0xa0, 0xa8, 0xb0, 0xb8
};

unsigned char fcCustom1[32]={
  0x45, 0x45, 0x79, 0x7d, 0x7a, 0x77, 0x70, 0x66, 0x61, 0x58, 0x53, 0x4d, 0x2c, 0x20, 0x18, 0x12,
  0x04, 0xdb, 0xd3, 0xcd, 0xc6, 0xbc, 0xb5, 0xae, 0xa8, 0xa3, 0x9d, 0x99, 0x93, 0x8e, 0x8b, 0x8a
};

unsigned char fcCustom2[32]={
  0x45, 0x45, 0x79, 0x7d, 0x7a, 0x77, 0x70, 0x66, 0x5b, 0x4b, 0x43, 0x37, 0x2c, 0x20, 0x18, 0x12,
  0x04, 0xf8, 0xe8, 0xdb, 0xcf, 0xc6, 0xbe, 0xb0, 0xa8, 0xa4, 0x9e, 0x9a, 0x95, 0x94, 0x8d, 0x83
};

unsigned char fcTinyTriangle[16]={
  0x00, 0x00, 0x40, 0x60, 0x7f, 0x60, 0x40, 0x20, 0x00, 0xe0, 0xc0, 0xa0, 0x80, 0xa0, 0xc0, 0xe0
};

void generateFCPresetWave(int index, DivWavetable* wave) {
  wave->max=255;
  wave->len=32;

  switch (index) {
    case 0x00: case 0x01: case 0x02: case 0x03:
    case 0x04: case 0x05: case 0x06: case 0x07:
    case 0x08: case 0x09: case 0x0a: case 0x0b:
    case 0x0c: case 0x0d: case 0x0e: case 0x0f:
      // XOR triangle
      for (int i=0; i<32; i++) {
        wave->data[i]=(unsigned char)((fcXORTriangle[i]^0x80)^(((index+15)<i)?0x87:0x00));
      }
      break;
    case 0x10: case 0x11: case 0x12: case 0x13:
    case 0x14: case 0x15: case 0x16: case 0x17:
    case 0x18: case 0x19: case 0x1a: case 0x1b:
    case 0x1c: case 0x1d: case 0x1e: case 0x1f:
      // pulse
      for (int i=0; i<32; i++) {
        wave->data[i]=(index>i)?0x01:0xff;
      }
      break;
    case 0x20: case 0x21: case 0x22: case 0x23:
    case 0x24: case 0x25: case 0x26: case 0x27:
      // tiny pulse
      for (int i=0; i<32; i++) {
        wave->data[i]=((index-0x18)>(i&15))?0x01:0xff;
      }
      break;
    case 0x28:
    case 0x2e:
      // saw
      for (int i=0; i<32; i++) {
        wave->data[i]=i<<3;
      }
      break;
    case 0x29:
    case 0x2f:
      // tiny saw
      for (int i=0; i<32; i++) {
        wave->data[i]=(i<<4)&0xff;
      }
      break;
    case 0x2a:
      // custom 1
      for (int i=0; i<32; i++) {
        wave->data[i]=fcCustom1[i]^0x80;
      }
      break;
    case 0x2b:
      // custom 2
      for (int i=0; i<32; i++) {
        wave->data[i]=fcCustom2[i]^0x80;
      }
      break;
    case 0x2c: case 0x2d:
      // tiny triangle
      for (int i=0; i<32; i++) {
        wave->data[i]=fcTinyTriangle[i&15]^0x80;
      }
      break;
    default:
      for (int i=0; i<32; i++) {
        wave->data[i]=i;
      }
      break;
  }
}

bool DivEngine::loadFC(unsigned char* file, size_t len) {
  struct InvalidHeaderException {};
  bool success=false;
  char magic[4]={0,0,0,0};
  SafeReader reader=SafeReader(file,len);
  warnings="";
  bool isFC14=false;
  unsigned int patPtr, freqMacroPtr, volMacroPtr, samplePtr, wavePtr;
  unsigned int seqLen, patLen, freqMacroLen, volMacroLen, sampleLen;

  unsigned char waveLen[80];
  //unsigned char waveLoopLen[40];

  struct FCSequence {
    unsigned char pat[4];
    signed char transpose[4];
    signed char offsetIns[4];
    unsigned char speed;
  };
  std::vector<FCSequence> seq;
  struct FCPattern {
    unsigned char note[32];
    unsigned char val[32];
  };
  std::vector<FCPattern> pat;
  struct FCMacro {
    unsigned char val[64];
  };
  std::vector<FCMacro> freqMacros;
  std::vector<FCMacro> volMacros;

  struct FCSample {
    unsigned short loopLen, len, loopStart;
  } sample[10];

  try {
    DivSong ds;
    ds.tuning=436.0;
    ds.version=DIV_VERSION_FC;
    //ds.linearPitch=0;
    //ds.pitchMacroIsLinear=false;
    //ds.noSlidesOnFirstTick=true;
    //ds.rowResetsArpPos=true;
    ds.pitchSlideSpeed=8;
    ds.ignoreJumpAtEnd=false;

    // load here
    if (!reader.seek(0,SEEK_SET)) {
      logE("premature end of file!");
      lastError="incomplete file";
      delete[] file;
      return false;
    }
    reader.read(magic,4);

    if (memcmp(magic,DIV_FC13_MAGIC,4)==0) {
      isFC14=false;
    } else if (memcmp(magic,DIV_FC14_MAGIC,4)==0) {
      isFC14=true;
    } else {
      logW("the magic isn't complete");
      throw EndOfFileException(&reader,reader.tell());
    }

    ds.systemLen=1;
    ds.system[0]=DIV_SYSTEM_AMIGA;
    ds.systemVol[0]=1.0f;
    ds.systemPan[0]=0;
    ds.systemFlags[0].set("clockSel",1); // PAL
    ds.systemFlags[0].set("stereoSep",80);
    ds.systemName="Amiga";

    seqLen=reader.readI_BE();
    if (seqLen%13) {
      logW("sequence length is not multiple of 13 (%d)",seqLen);
      //throw EndOfFileException(&reader,reader.tell());
    }
    patPtr=reader.readI_BE();
    patLen=reader.readI_BE();
    if (patLen%64) {
      logW("pattern length is not multiple of 64 (%d)",patLen);
      throw EndOfFileException(&reader,reader.tell());
    }
    freqMacroPtr=reader.readI_BE();
    freqMacroLen=reader.readI_BE();
    if (freqMacroLen%64) {
      logW("freq sequence length is not multiple of 64 (%d)",freqMacroLen);
      //throw EndOfFileException(&reader,reader.tell());
    }
    volMacroPtr=reader.readI_BE();
    volMacroLen=reader.readI_BE();
    if (volMacroLen%64) {
      logW("vol sequence length is not multiple of 64 (%d)",volMacroLen);
      //throw EndOfFileException(&reader,reader.tell());
    }
    samplePtr=reader.readI_BE();
    if (isFC14) {
      wavePtr=reader.readI_BE(); // wave len
      sampleLen=0;
    } else {
      sampleLen=reader.readI_BE();
      wavePtr=0;
    }

    logD("patPtr: %x",patPtr);
    logD("patLen: %d",patLen);
    logD("freqMacroPtr: %x",freqMacroPtr);
    logD("freqMacroLen: %d",freqMacroLen);
    logD("volMacroPtr: %x",volMacroPtr);
    logD("volMacroLen: %d",volMacroLen);
    logD("samplePtr: %x",samplePtr);
    if (isFC14) {
      logD("wavePtr: %x",wavePtr);
    } else {
      logD("sampleLen: %d",sampleLen);
    }

    // sample info
    logD("samples: (%x)",reader.tell());
    for (int i=0; i<10; i++) {
      sample[i].len=reader.readS_BE();
      sample[i].loopStart=reader.readS_BE();
      sample[i].loopLen=reader.readS_BE();

      logD("- %d: %d (%d, %d)",i,sample[i].len,sample[i].loopStart,sample[i].loopLen);
    }

    // wavetable lengths
    if (isFC14) {
      logD("wavetables:");
      for (int i=0; i<80; i++) {
        waveLen[i]=(unsigned char)reader.readC();

        logD("- %d: %.4x",i,waveLen[i]);
      }
    }

    // sequences
    seqLen/=13;
    logD("reading sequences... (%d)",seqLen);
    seq.reserve(seqLen);
    for (unsigned int i=0; i<seqLen; i++) {
      FCSequence s;
      for (int j=0; j<4; j++) {
        s.pat[j]=reader.readC();
        s.transpose[j]=reader.readC();
        s.offsetIns[j]=reader.readC();
      }
      s.speed=reader.readC();
      seq.push_back(s);
      logV(
        "%.2x | %.2x%.2x%.2x %.2x%.2x%.2x %.2x%.2x%.2x %.2x%.2x%.2x | %.2x",
        i,
        s.pat[0],s.transpose[0],s.offsetIns[0],
        s.pat[1],s.transpose[1],s.offsetIns[1],
        s.pat[2],s.transpose[2],s.offsetIns[2],
        s.pat[3],s.transpose[3],s.offsetIns[3],
        s.speed
      );
    }

    // patterns
    if (!reader.seek(patPtr,SEEK_SET)) {
      logE("premature end of file!");
      lastError="incomplete file";
      delete[] file;
      return false;
    }
    patLen/=64;
    logD("reading patterns... (%d)",patLen);
    pat.reserve(patLen);
    for (unsigned int i=0; i<patLen; i++) {
      FCPattern p;
      logV("- pattern %d",i);
      for (int j=0; j<32; j++) {
        p.note[j]=reader.readC();
        p.val[j]=reader.readC();
        //logV("%.2x | %.2x %.2x",j,p.note[j],p.val[j]);
      }
      pat.push_back(p);
    }

    // freq sequences
    if (!reader.seek(freqMacroPtr,SEEK_SET)) {
      logE("premature end of file!");
      lastError="incomplete file";
      delete[] file;
      return false;
    }
    freqMacroLen/=64;
    logD("reading freq sequences... (%d)",freqMacroLen);
    freqMacros.reserve(freqMacroLen);
    for (unsigned int i=0; i<freqMacroLen; i++) {
      FCMacro m;
      reader.read(m.val,64);
      freqMacros.push_back(m);
    }

    // vol sequences
    if (!reader.seek(volMacroPtr,SEEK_SET)) {
      logE("premature end of file!");
      lastError="incomplete file";
      delete[] file;
      return false;
    }
    volMacroLen/=64;
    logD("reading volume sequences... (%d)",volMacroLen);
    volMacros.reserve(volMacroLen);
    for (unsigned int i=0; i<volMacroLen; i++) {
      FCMacro m;
      reader.read(m.val,64);
      volMacros.push_back(m);
    }

    // samples
    if (!reader.seek(samplePtr,SEEK_SET)) {
      logE("premature end of file!");
      lastError="incomplete file";
      delete[] file;
      return false;
    }
    logD("reading samples...");
    ds.sample.reserve(10);
    for (int i=0; i<10; i++) {
      DivSample* s=new DivSample;
      s->depth=DIV_SAMPLE_DEPTH_8BIT;
      if (sample[i].len>0) {
        s->init(sample[i].len*2);
      }
      s->name=fmt::sprintf("Sample %d",i+1);
      if (sample[i].loopLen>1) {
        s->loopStart=sample[i].loopStart;
        s->loopEnd=sample[i].loopStart+(sample[i].loopLen*2);
        s->loop=(s->loopStart>=0)&&(s->loopEnd>=0);
      }
      reader.read(s->data8,sample[i].len*2);
      ds.sample.push_back(s);
    }
    ds.sampleLen=(int)ds.sample.size();

    // wavetables
    if (isFC14) {
      if (!reader.seek(wavePtr,SEEK_SET)) {
        logE("premature end of file!");
        lastError="incomplete file";
        delete[] file;
        return false;
      }
      logD("reading wavetables...");
      ds.wave.reserve(80);
      for (int i=0; i<80; i++) {
        DivWavetable* w=new DivWavetable;
        w->min=0;
        w->max=255;
        w->len=MIN(256,waveLen[i]*2);

        for (int i=0; i<256; i++) {
          w->data[i]=128;
        }
        
        if (waveLen[i]>0) {
          signed char* waveArray=new signed char[waveLen[i]*2];
          reader.read(waveArray,waveLen[i]*2);
          int howMany=waveLen[i]*2;
          if (howMany>256) howMany=256;
          for (int i=0; i<howMany; i++) {
            w->data[i]=waveArray[i]+128;
          }
          delete[] waveArray;
        } else {
          logV("empty wave %d",i);
          generateFCPresetWave(i,w);
        }

        ds.wave.push_back(w);
      }
    } else {
      // generate preset waves
      ds.wave.reserve(48);
      for (int i=0; i<48; i++) {
        DivWavetable* w=new DivWavetable;
        generateFCPresetWave(i,w);
        ds.wave.push_back(w);
      }
    }
    ds.waveLen=(int)ds.wave.size();

    // convert
    ds.subsong[0]->ordersLen=seqLen;
    ds.subsong[0]->patLen=32;
    ds.subsong[0]->hz=50;
    ds.subsong[0]->pat[3].effectCols=3;
    ds.subsong[0]->speeds.val[0]=3;
    ds.subsong[0]->speeds.len=1;

    int lastIns[4];
    int lastNote[4];
    signed char lastTranspose[4];
    bool isSliding[4];

    memset(lastIns,-1,4*sizeof(int));
    memset(lastNote,-1,4*sizeof(int));
    memset(lastTranspose,0,4);
    memset(isSliding,0,4*sizeof(bool));

    for (unsigned int i=0; i<seqLen; i++) {
      for (int j=0; j<4; j++) {
        ds.subsong[0]->orders.ord[j][i]=i;
        DivPattern* p=ds.subsong[0]->pat[j].getPattern(i,true);
        if (j==3 && seq[i].speed) {
          p->data[0][6]=0x0f;
          p->data[0][7]=seq[i].speed;
        }

        bool ignoreNext=false;

        for (int k=0; k<32; k++) {
          FCPattern& fp=pat[seq[i].pat[j]];
          if (fp.note[k]>0 && fp.note[k]<0x49) {
            lastNote[j]=fp.note[k];
            short note=(fp.note[k]+seq[i].transpose[j])%12;
            short octave=2+((fp.note[k]+seq[i].transpose[j])/12);
            if (fp.note[k]>=0x3d) octave-=6;
            if (note==0) {
              note=12;
              octave--;
            }
            octave&=0xff;
            p->data[k][0]=note;
            p->data[k][1]=octave;
            if (isSliding[j]) {
              isSliding[j]=false;
              p->data[k][4]=2;
              p->data[k][5]=0;
            }
          } else if (fp.note[k]==0x49) {
            if (k>0) {
              p->data[k-1][4]=0x0d;
              p->data[k-1][5]=0;
            }
          } else if (k==0 && lastTranspose[j]!=seq[i].transpose[j]) {
            p->data[0][2]=lastIns[j];
            p->data[0][4]=0x03;
            p->data[0][5]=0xff;
            lastTranspose[j]=seq[i].transpose[j];

            short note=(lastNote[j]+seq[i].transpose[j])%12;
            short octave=2+((lastNote[j]+seq[i].transpose[j])/12);
            if (lastNote[j]>=0x3d) octave-=6;
            if (note==0) {
              note=12;
              octave--;
            }
            octave&=0xff;
            p->data[k][0]=note;
            p->data[k][1]=octave;
          }
          if (fp.val[k]) {
            if (ignoreNext) {
              ignoreNext=false;
            } else {
              if (fp.val[k]==0xf0) {
                p->data[k][0]=100;
                p->data[k][1]=0;
                p->data[k][2]=-1;
              } else if (fp.val[k]&0xe0) {
                if (fp.val[k]&0x40) {
                  p->data[k][4]=2;
                  p->data[k][5]=0;
                  isSliding[j]=false;
                } else if (fp.val[k]&0x80) {
                  isSliding[j]=true;
                  if (k<31) {
                    if (fp.val[k+1]&0x20) {
                      p->data[k][4]=2;
                      p->data[k][5]=fp.val[k+1]&0x1f;
                    } else {
                      p->data[k][4]=1;
                      p->data[k][5]=fp.val[k+1]&0x1f;
                    }
                    ignoreNext=true;
                  } else {
                    p->data[k][4]=2;
                    p->data[k][5]=0;
                  }
                }
              } else {
                p->data[k][2]=(fp.val[k]+seq[i].offsetIns[j])&0x3f;
                lastIns[j]=p->data[k][2];
              }
            }
          } else if (fp.note[k]>0 && fp.note[k]<0x49) {
            p->data[k][2]=seq[i].offsetIns[j];
            lastIns[j]=p->data[k][2];
          }
        }
      }
    }

    // convert instruments
    for (unsigned int i=0; i<volMacroLen; i++) {
      DivInstrument* ins=new DivInstrument;
      FCMacro& m=volMacros[i];
      
      ins->type=DIV_INS_AMIGA;
      ins->name=fmt::sprintf("Instrument %d",i);
      ins->amiga.useWave=true;
      unsigned char seqSpeed=m.val[0];
      unsigned char freqMacro=m.val[1];
      unsigned char vibSpeed=m.val[2];
      unsigned char vibDepth=m.val[3];
      unsigned char vibDelay=m.val[4];

      unsigned char lastVal=m.val[5];

      signed char loopMap[64];
      memset(loopMap,-1,64);

      signed char loopMapFreq[64];
      memset(loopMapFreq,-1,64);

      signed char loopMapWave[64];
      memset(loopMapWave,-1,64);

      // volume sequence
      ins->std.volMacro.len=0;
      ds.ins.reserve(64 - 5);
      for (int j=5; j<64; j++) {
        loopMap[j]=ins->std.volMacro.len;
        if (m.val[j]==0xe1) { // end
          break;
        } else if (m.val[j]==0xe0) { // loop
          if (++j>=64) break;
          ins->std.volMacro.loop=loopMap[m.val[j]&63];
          break;
        } else if (m.val[j]==0xe8) { // sustain
          if (++j>=64) break;
          unsigned char susTime=m.val[j];
          // TODO: <= or <?
          for (int k=0; k<=susTime; k++) {
            ins->std.volMacro.val[ins->std.volMacro.len]=lastVal;
            if (++ins->std.volMacro.len>=255) break;
          }
          if (ins->std.volMacro.len>=255) break;
        } else if (m.val[j]==0xe9 || m.val[j]==0xea) { // volume slide
          if (++j>=64) break;
          signed char slideStep=m.val[j];
          if (++j>=64) break;
          unsigned char slideTime=m.val[j];
          // TODO: <= or <?
          for (int k=0; k<=slideTime; k++) {
            if (slideStep>0) {
              lastVal+=slideStep;
              if (lastVal>63) lastVal=63;
            } else {
              if (-slideStep>lastVal) {
                lastVal=0;
              } else {
                lastVal-=slideStep;
              }
            }
            ins->std.volMacro.val[ins->std.volMacro.len]=lastVal;
            if (++ins->std.volMacro.len>=255) break;
          }
        } else {
          // TODO: replace with upcoming macro speed
          for (int k=0; k<MAX(1,seqSpeed); k++) {
            ins->std.volMacro.val[ins->std.volMacro.len]=m.val[j];
            lastVal=m.val[j];
            if (++ins->std.volMacro.len>=255) break;
          }
          if (ins->std.volMacro.len>=255) break;
        }
      }

      // frequency sequence
      lastVal=0;
      ins->amiga.initSample=-1;
      if (freqMacro<freqMacros.size()) {
        FCMacro& fm=freqMacros[freqMacro];
        for (int j=0; j<64; j++) {
          loopMapFreq[j]=ins->std.arpMacro.len;
          loopMapWave[j]=ins->std.waveMacro.len;
          if (fm.val[j]==0xe1) {
            break;
          } else if (fm.val[j]==0xe2 || fm.val[j]==0xe4) {
            if (++j>=64) break;
            unsigned char wave=fm.val[j];
            if (wave<10) { // sample
              if (ins->amiga.initSample==-1) {
                ins->amiga.initSample=wave;
                ins->amiga.useWave=false;
              }
            } else { // waveform
              ins->std.waveMacro.val[ins->std.waveMacro.len]=wave-10;
              ins->std.waveMacro.open=true;
              lastVal=wave;
              //if (++ins->std.arpMacro.len>=255) break;
            }
          } else if (fm.val[j]==0xe0) {
            if (++j>=64) break;
            ins->std.arpMacro.loop=loopMapFreq[fm.val[j]&63];
            ins->std.waveMacro.loop=loopMapWave[fm.val[j]&63];
            break;
          } else if (fm.val[j]==0xe3) {
            logV("unhandled vibrato!");
          } else if (fm.val[j]==0xe8) {
            logV("unhandled sustain!");
          } else if (fm.val[j]==0xe7) {
            if (++j>=64) break;
            fm=freqMacros[MIN(fm.val[j],freqMacros.size()-1)];
            j=0;
          } else if (fm.val[j]==0xe9) {
            logV("unhandled pack!");
          } else if (fm.val[j]==0xea) {
            logV("unhandled pitch!");
          } else {
            if (fm.val[j]>0x80) {
              ins->std.arpMacro.val[ins->std.arpMacro.len]=(fm.val[j]-0x80+24)^0x40000000;
            } else {
              ins->std.arpMacro.val[ins->std.arpMacro.len]=fm.val[j];
            }
            if (lastVal>=10) {
              ins->std.waveMacro.val[ins->std.waveMacro.len]=lastVal-10;
            }
            ins->std.arpMacro.open=true;
            if (++ins->std.arpMacro.len>=255) break;
            if (++ins->std.waveMacro.len>=255) break;
          }
        }
      }

      // waveform width
      if (lastVal>=10 && (unsigned int)(lastVal-10)<ds.wave.size()) {
        ins->amiga.waveLen=ds.wave[lastVal-10]->len-1;
      }

      // vibrato
      for (int j=0; j<=vibDelay; j++) {
        ins->std.pitchMacro.val[ins->std.pitchMacro.len]=0;
        if (++ins->std.pitchMacro.len>=255) break;
      }
      int vibPos=0;
      ins->std.pitchMacro.loop=ins->std.pitchMacro.len;
      do {
        vibPos+=vibSpeed;
        if (vibPos>vibDepth) vibPos=vibDepth;
        ins->std.pitchMacro.val[ins->std.pitchMacro.len]=vibPos*32;
        if (++ins->std.pitchMacro.len>=255) break;
      } while (vibPos<vibDepth);
      do {
        vibPos-=vibSpeed;
        if (vibPos<-vibDepth) vibPos=-vibDepth;
        ins->std.pitchMacro.val[ins->std.pitchMacro.len]=vibPos*32;
        if (++ins->std.pitchMacro.len>=255) break;
      } while (vibPos>-vibDepth);
      do {
        vibPos+=vibSpeed;
        if (vibPos>0) vibPos=0;
        ins->std.pitchMacro.val[ins->std.pitchMacro.len]=vibPos*32;
        if (++ins->std.pitchMacro.len>=255) break;
      } while (vibPos<0);

      ds.ins.push_back(ins);
    }
    ds.insLen=(int)ds.ins.size();

    // optimize
    ds.subsong[0]->optimizePatterns();
    ds.subsong[0]->rearrangePatterns();

    if (active) quitDispatch();
    BUSY_BEGIN_SOFT;
    saveLock.lock();
    song.unload();
    song=ds;
    changeSong(0);
    recalcChans();
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
    //logE("premature end of file!");
    lastError="incomplete file";
  } catch (InvalidHeaderException& e) {
    //logE("invalid header!");
    lastError="invalid header!";
  }
  return success;
}

