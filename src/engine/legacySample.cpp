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

// from this point onwards, a mess.

#include "engine.h"
#include <fmt/printf.h>

bool DivEngine::convertLegacySampleMode() {
  logD("converting legacy sample mode...");
  int legacyInsInit=-1;
  
  // quit if we don't have space for a legacy sample instrument
  if (song.ins.size()>254) {
    logW("no space left on instrument list!");
    return false;
  }

  auto initSampleInsIfNeeded=[this,&legacyInsInit]() {
    if (legacyInsInit==-1) {
      legacyInsInit=(int)song.ins.size();
      for (size_t bank=0; bank<(11+song.sample.size())/12; bank++) {
        DivInstrument* ins=new DivInstrument;

        ins->type=DIV_INS_AMIGA;
        ins->amiga.useNoteMap=true;
        if (bank==0) {
          ins->name="Legacy Samples";
        } else {
          ins->name=fmt::sprintf("Legacy Samples (bank %d)",(int)bank);
        }
        for (int i=0; i<120; i++) {
          ins->amiga.noteMap[i].freq=48; // C-4
          ins->amiga.noteMap[i].map=12*bank+(i%12);
        }

        song.ins.push_back(ins);
        if (song.ins.size()>=256) break;
      }
      song.insLen=song.ins.size();
      checkAssetDir(song.insDir,song.ins.size());
    }
  };

  for (DivSubSong* h: song.subsong) {
    for (int i=0; i<chans; i++) {
      // 0: sample off
      // 1: legacy mode
      // 2: normal mode
      unsigned char sampleMode=0;
      unsigned char sampleBank=0;
      DivInstrumentType preferredInsType=DIV_INS_AMIGA;
      DivInstrumentType preferredInsType2=DIV_INS_AMIGA;
      bool noteOffDisablesSampleMode=false;
      bool hasLegacyToggle=false;

      switch (sysOfChan[i]) {
        case DIV_SYSTEM_NES:
        case DIV_SYSTEM_5E01:
          // NES PCM channel (on by default)
          if (dispatchChanOfChan[i]!=4) {
            continue;
          }
          sampleMode=1;
          break;
        case DIV_SYSTEM_MMC5:
          // MMC5 PCM channel
          if (dispatchChanOfChan[i]!=2) {
            continue;
          }
          sampleMode=1;
          break;
        case DIV_SYSTEM_YM2612:
          // YM2612 DAC channel
          if (dispatchChanOfChan[i]!=5) {
            continue;
          }
          hasLegacyToggle=true;
          break;
        case DIV_SYSTEM_YM2612_EXT:
        case DIV_SYSTEM_YM2612_CSM:
          // YM2612 DAC channel
          if (dispatchChanOfChan[i]!=8) {
            continue;
          }
          hasLegacyToggle=true;
          break;
        case DIV_SYSTEM_PCE:
        case DIV_SYSTEM_X1_010:
          // any channel can be DAC'd
          noteOffDisablesSampleMode=true;
          hasLegacyToggle=true;
          break;
        case DIV_SYSTEM_AY8910:
        case DIV_SYSTEM_AY8930:
          // any channel can be DAC'd
          break;
        case DIV_SYSTEM_YM2610:
        case DIV_SYSTEM_YM2610_FULL:
          // Neo Geo CD ADPCM channels
          if (dispatchChanOfChan[i]<7) {
            continue;
          }
          sampleMode=1;
          preferredInsType=DIV_INS_ADPCMA;
          preferredInsType2=DIV_INS_ADPCMB;
          break;
        case DIV_SYSTEM_YM2610_EXT:
        case DIV_SYSTEM_YM2610_FULL_EXT:
          // Neo Geo CD ADPCM channels
          if (dispatchChanOfChan[i]<10) {
            continue;
          }
          sampleMode=1;
          preferredInsType=DIV_INS_ADPCMA;
          preferredInsType2=DIV_INS_ADPCMB;
          break;
        case DIV_SYSTEM_YM2610_CSM:
          // Neo Geo CD ADPCM channels
          if (dispatchChanOfChan[i]<11) {
            continue;
          }
          sampleMode=1;
          preferredInsType=DIV_INS_ADPCMA;
          preferredInsType2=DIV_INS_ADPCMB;
          break;
        case DIV_SYSTEM_YM2610B:
          // ADPCM channels
          if (dispatchChanOfChan[i]<9) {
            continue;
          }
          sampleMode=1;
          preferredInsType=DIV_INS_ADPCMA;
          preferredInsType2=DIV_INS_ADPCMB;
          break;
        case DIV_SYSTEM_YM2610B_EXT:
          // ADPCM channels
          if (dispatchChanOfChan[i]<12) {
            continue;
          }
          sampleMode=1;
          preferredInsType=DIV_INS_ADPCMA;
          preferredInsType2=DIV_INS_ADPCMB;
          break;
        case DIV_SYSTEM_YM2610B_CSM:
          // ADPCM channels
          if (dispatchChanOfChan[i]<13) {
            continue;
          }
          sampleMode=1;
          preferredInsType=DIV_INS_ADPCMA;
          preferredInsType2=DIV_INS_ADPCMB;
          break;
        case DIV_SYSTEM_YM2608:
          // ADPCM channel
          if (dispatchChanOfChan[i]!=15) {
            continue;
          }
          sampleMode=1;
          preferredInsType=DIV_INS_ADPCMB;
          break;
        case DIV_SYSTEM_YM2608_EXT:
          // ADPCM channel
          if (dispatchChanOfChan[i]!=18) {
            continue;
          }
          sampleMode=1;
          preferredInsType=DIV_INS_ADPCMB;
          break;
        case DIV_SYSTEM_YM2608_CSM:
          // ADPCM channel
          if (dispatchChanOfChan[i]!=19) {
            continue;
          }
          sampleMode=1;
          preferredInsType=DIV_INS_ADPCMB;
          break;
        case DIV_SYSTEM_SEGAPCM:
        case DIV_SYSTEM_SEGAPCM_COMPAT:
          // all channels can play back samples
          sampleMode=1;
          preferredInsType=DIV_INS_SEGAPCM;
          break;
        case DIV_SYSTEM_MSM6258:
          sampleMode=1;
          preferredInsType=DIV_INS_MSM6258;
          break;
        case DIV_SYSTEM_MSM6295:
          sampleMode=1;
          preferredInsType=DIV_INS_MSM6295;
          break;
        case DIV_SYSTEM_Y8950:
          // Y8950 ADPCM
          if (dispatchChanOfChan[i]!=9) {
            continue;
          }
          sampleMode=1;
          break;
        case DIV_SYSTEM_Y8950_DRUMS:
          // Y8950 ADPCM
          if (dispatchChanOfChan[i]!=11) {
            continue;
          }
          sampleMode=1;
          break;
        case DIV_SYSTEM_SWAN:
          // PCM channel
          if (dispatchChanOfChan[i]!=1) {
            continue;
          }
          noteOffDisablesSampleMode=true;
          hasLegacyToggle=true;
          break;
        case DIV_SYSTEM_VRC6:
          // pulse DAC mode
          if (dispatchChanOfChan[i]>=2) {
            continue;
          }
          hasLegacyToggle=true;
          break;
        default: // not a chip with convertible channels
          continue;
      }

      logV("- channel %d",i);

      bool didThisPattern[DIV_MAX_PATTERNS];
      memset(didThisPattern,0,DIV_MAX_PATTERNS*sizeof(bool));

      for (int j=0; j<h->ordersLen; j++) {
        int patIndex=h->orders.ord[i][j];
        if (didThisPattern[patIndex]) continue;
        didThisPattern[patIndex]=true;
        DivPattern* p=h->pat[i].data[patIndex];
        if (p==NULL) continue;

        for (int k=0; k<h->patLen; k++) {
          // check for legacy mode toggle and sample bank changes
          for (int l=0; l<h->pat[i].effectCols; l++) {
            int fxVal=p->newData[k][DIV_PAT_FXVAL(l)];
            if (fxVal<0) fxVal=0;
            switch (p->newData[k][DIV_PAT_FX(l)]) {
              case 0x17: // set legacy sample mode
                if (hasLegacyToggle) {
                  if (fxVal==0) {
                    sampleMode=0;
                  } else {
                    sampleMode=1;
                  }
                  // clear effect
                  p->newData[k][DIV_PAT_FX(l)]=-1;
                  p->newData[k][DIV_PAT_FXVAL(l)]=-1;
                }
                break;
              case 0xeb: // set sample bank
                sampleBank=fxVal;
                // clear effect
                p->newData[k][DIV_PAT_FX(l)]=-1;
                p->newData[k][DIV_PAT_FXVAL(l)]=-1;
                logV("change bank to %d",sampleBank);
                break;
            }
          }

          // check for instrument changes
          if (p->newData[k][DIV_PAT_INS]!=-1) {
            DivInstrument* ins=getIns(p->newData[k][DIV_PAT_INS]);
            if (ins->type==DIV_INS_AMIGA || ins->amiga.useSample || ins->type==preferredInsType || ins->type==preferredInsType2) {
              sampleMode=2;
            }
          }

          if (p->newData[k][DIV_PAT_NOTE]!=-1 &&
              p->newData[k][DIV_PAT_NOTE]!=DIV_NOTE_OFF &&
              p->newData[k][DIV_PAT_NOTE]!=DIV_NOTE_REL &&
              p->newData[k][DIV_PAT_NOTE]!=DIV_MACRO_REL) {
            // we've got a note
            if (sampleMode==1) {
              initSampleInsIfNeeded();
              //p->newData[k][DIV_PAT_NOTE]=60+12*sampleBank+(p->newData[k][DIV_PAT_NOTE]%12);
              p->newData[k][DIV_PAT_INS]=MIN(0xff,legacyInsInit+sampleBank);
            }
          } else if (p->newData[k][DIV_PAT_NOTE]==DIV_NOTE_OFF && noteOffDisablesSampleMode) {
            sampleMode=0;
          }
        }
      }
    }
  }

  return (legacyInsInit!=-1);
}
