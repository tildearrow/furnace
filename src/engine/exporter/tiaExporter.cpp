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

#include <fmt/printf.h>
#include "tiaExporter.h"

const int AUDC0 = 0x15;
const int AUDC1 = 0x16;
const int AUDF0 = 0x17;
const int AUDF1 = 0x18;
const int AUDV0 = 0x19;
const int AUDV1 = 0x1A;

struct TiaRegisters {

  unsigned short audc0;
  unsigned short audc1;
  unsigned short audf0;
  unsigned short audf1;
  unsigned short audv0;
  unsigned short audv1;

  bool write(const DivRegWrite& registerWrite);

};

bool TiaRegisters::write(const DivRegWrite& registerWrite) {
  switch (registerWrite.addr) {
    case AUDC0:
      audc0 = registerWrite.val & 0xff;
      return true;
    case AUDC1:
      audc1 = registerWrite.val & 0xff;
      return true;
    case AUDF0:
      audf0 = registerWrite.val & 0xff;
      return true;
    case AUDF1:
      audf1 = registerWrite.val & 0xff;
      return true;
    case AUDV0:
      audv0 = registerWrite.val & 0xff;
      return true;
    case AUDV1:
      audv1 = registerWrite.val & 0xff;
      return true;
  }
  return false;
}

struct PatternInfo {
  unsigned short subsong, chan, pat;
  PatternInfo(unsigned short s, unsigned short c, unsigned short p):
    subsong(s),
    chan(c),
    pat(p) {}
};

unsigned char R9TrackerBuilder::calculateFrequency(unsigned char shape, unsigned char note, unsigned char octave, int pitch) {
  int value = note + ((signed char)octave) * 12;
  int base = value << 8;
  if (base&0x80000000 && ((base&0x7fffffff)<32)) {
    return base&0x1f;
  }
  int bp=base+pitch;
  double mult=0.25*(e->song.tuning*0.0625)*pow(2.0,double(768+bp)/(256.0*12.0));
  if (mult<0.5) mult=0.5;
  switch (shape) {
    case 1: // buzzy
      return ceil(31400/(30.6*mult))-1;
      break;
    case 2: // low buzzy
      return ceil(31400/(480*mult))-1;
      break;
    case 3: // flangy
      return ceil(31400/(60*mult))-1;
      break;
    case 4: case 5: // square
      return ceil(31400/(4.05*mult))-1;
      break;
    case 6: case 7: case 8: case 9: case 10: // pure buzzy/reedy/noise
      return ceil(31400/(63*mult))-1;
      break;
    case 12: case 13: // low square
      return round(31400/(4.0*3*mult))-1;
      break;
    case 14: case 15: // low pure buzzy/reedy
      return ceil(31400/(3*63*mult))-1;
      break;
  }
  return 0;
}

SafeWriter* R9TrackerBuilder::buildROM(int sysIndex) {

  SafeWriter* w=new SafeWriter;
  w->init();
  w->writeText("# Data exported from Furnace to R9 (Tia) data track.\n");
  w->writeText(fmt::sprintf("# Song: %s\n", e->song.name));
  w->writeText(fmt::sprintf("# Author: %s\n", e->song.author));
  // TODO: never populated?
  //   w->writeText(fmt::sprintf("# Composer: %s\n", e->song.composer));
  //   w->writeText(fmt::sprintf("# Arranger: %s\n", e->song.arranger));
  //   w->writeText(fmt::sprintf("# Copyright: %s\n", e->song.copyright));
  //   w->writeText(fmt::sprintf("# Created Date: %s\n", e->song.createdDate));

  dumpCommands(w);

  return w;
}

void R9TrackerBuilder::dumpCommands(SafeWriter *w) {
  e->stop();
  e->setRepeatPattern(false);
  e->setOrder(0);

  // determine loop point
  int loopOrder=0;
  int loopRow=0;
  int loopEnd=0;
  e->walkSong(loopOrder, loopRow, loopEnd);

  // play the song ourselves
  e->play();

  for (int i=0; i<e->song.systemLen; i++) {
    e->getDispatch(i)->toggleRegisterDump(true);
  }

  int tick = 0;
  int lastWriteTick = 0;
  TiaRegisters currentState;
  memset(&currentState, 0, sizeof(currentState));
  bool done=false;
  while (!done) {
    if (e->tick() || !e->isPlaying()) {
      done=true;
      for (int i=0; i<e->song.systemLen; i++) {
        e->getDispatch(i)->getRegisterWrites().clear();
      }
      break;
    }
    tick++;
    // get register dumps
    for (int i=0; i<e->song.systemLen; i++) {
      bool isDirty = false;
      std::vector<DivRegWrite>& registerWrites=e->getDispatch(i)->getRegisterWrites();
      for (DivRegWrite& registerWrite: registerWrites) {
        isDirty |= currentState.write(registerWrite);
      }
      registerWrites.clear();

      if (isDirty) {
        // end last seq
        if (lastWriteTick > 0) {
          w->writeText(fmt::sprintf("  byte %d\n", tick - lastWriteTick));
        }
        // start next seq
        lastWriteTick = tick;
        w->writeText(fmt::sprintf("  byte %d,%d,%d\n", currentState.audc0, currentState.audf0, currentState.audv0));
        w->writeText(fmt::sprintf("  byte %d,%d,%d\n", currentState.audc1, currentState.audf1, currentState.audv1));      
      }
    }
  }

  // final seq
  w->writeText(fmt::sprintf("  byte %d\n", tick - lastWriteTick));
  w->writeText(fmt::sprintf("  byte %d,%d,%d\n", currentState.audc0, currentState.audf0, currentState.audv0));
  w->writeText(fmt::sprintf("  byte %d,%d,%d\n", currentState.audc1, currentState.audf1, currentState.audv1));      

  for (int i=0; i<e->song.systemLen; i++) {
    e->getDispatch(i)->toggleRegisterDump(false);
  }

}

void R9TrackerBuilder::writeTrackData(SafeWriter *w) {

  w->writeText("rft_InsAUDCxTable\n");
  for (int i = 0; i < e->song.insLen; i++) {
    DivInstrument* ins = e->getIns(i, DIV_INS_TIA);
    w->writeText(fmt::sprintf("rft_Ins%d_AUDCx ; Waveforms for %s", i, ins->name));
    for (int j = 0; j < 1; j++) { // TODO: multiple waveforms? ins->std.waveMacro.len; j++) {
      if (j % 8 == 0) {
        w->writeText("\n    byte ");
      } else {
        w->writeText(",");
      }
      w->writeText(fmt::sprintf("$%02x", ins->std.waveMacro.val[j]));
    }
    w->writeC('\n');
  }

  w->writeText("rft_InsAUDFxAUDVxTable\n");
  for (int i = 0; i < e->song.insLen; i++) {
    DivInstrument* ins = e->getIns(i, DIV_INS_TIA);
    w->writeText(fmt::sprintf("rft_Ins%d_AUDFxAUDVx ; FreqVol for %s", i, ins->name));
    for (int j = 0; j < ins->std.volMacro.len; j++) {
      if (j % 8 == 0) {
        w->writeText("\n    byte ");
      } else {
        w->writeText(",");
      }
      short freqVol = (ins->std.volMacro.val[j] << 4) + ins->std.pitchMacro.val[j];
      w->writeText(fmt::sprintf("$%02x", freqVol));
    }
    w->writeC('\n');
  }

  // borrow from fileops - pull patterns to write
  std::vector<PatternInfo> patterns;
  bool alreadyAdded[256];
  for (int i = 0; i < e->getChannelCount(DIV_SYSTEM_TIA); i++) {
    for (size_t j = 0; j < e->song.subsong.size(); j++) {
      DivSubSong* subs = e->song.subsong[j];
      memset(alreadyAdded, 0, 256*sizeof(bool));
      for (int k = 0; k < subs->ordersLen; k++) {
        if (alreadyAdded[subs->orders.ord[i][k]]) continue;
        patterns.push_back(PatternInfo(j, i, subs->orders.ord[i][k]));
        alreadyAdded[subs->orders.ord[i][k]] = true;
      }
    }
  }

  for (PatternInfo& i: patterns) {
    DivPattern* pat = e->song.subsong[i.subsong]->pat[i.chan].getPattern(i.pat, false);
    w->writeText(fmt::sprintf("# Subsong: %d Channel: %d Pattern: %d / %s\n", i.subsong, i.chan, i.pat, pat->name));
    w->writeText(fmt::sprintf("rft_subsong%d_chan%d_pat%d_start", i.subsong, i.chan, i.pat));
    for (int j=0; j<e->song.subsong[i.subsong]->patLen; j++) {
      if (j % 8 == 0) {
        w->writeText("\n    byte ");
      } else {
        w->writeText(",");
      }
      short note = pat->data[j][0];
      short octave = pat->data[j][1];
      short instrument = pat->data[j][2];
      //short volume = pat->data[j][3]; 
      // write instrument to bits 7..5, frequency to bits 0..4 
      short shape = e->getIns(instrument)->std.waveMacro.val[0];
      unsigned char freq = calculateFrequency(shape, note, octave, 0);
      char bits = (((0x07 & instrument) + 1) << 5) + (0xf & freq);
      w->writeText(fmt::sprintf("$%02x", bits));
    }
    w->writeC('\n');
  }
}
