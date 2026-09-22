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

#include "insEditCommon.h"
#include "../guiConst.h"

// do not change these!
// anything other than a checkbox will look ugly!
//
// if you really need to, and have a good rationale (and by good I mean a VERY
// good one), please tell me and we'll sort it out.
const char* macroAbsoluteMode="Fixed";
const char* macroRelativeMode="Relative";
const char* macroQSoundMode="QSound";
const char* macroDummyMode="Bug";

const char* panBits[5]={
  _N("right"),
  _N("left"),
  _N("rear right"),
  _N("rear left"),
  NULL
};

void addAALine(ImDrawList* dl, const ImVec2& p1, const ImVec2& p2, const ImU32 color, float thickness) {
  ImVec2 pt[2];
  pt[0]=p1;
  pt[1]=p2;
  dl->AddPolyline(pt,2,color,thickness,ImDrawFlags_None);
}

String macroHoverNote(int id, float val, void* u) {
  int* macroVal=(int*)u;
  if ((macroVal[id]&0xc0000000)==0x40000000 || (macroVal[id]&0xc0000000)==0x80000000) {
    if (val<-60 || val>=120) return "???";
    return fmt::sprintf("%d: %s",id,noteNames[(int)val+60]);
  }
  return fmt::sprintf("%d: %d",id,(int)val);
}

String macroLFOWaves(int id, float val, void* u) {
  const char* label="???";
  switch (((int)val)&3) {
    case 0:
      label=_("Saw");
      break;
    case 1:
      label=_("Square");
      break;
    case 2:
      label=_("Triangle");
      break;
    case 3:
      label=_("Random");
      break;
    default: break;
  }
  return fmt::sprintf("%d: %s",id,label);
}

// TODO: move to fm.cpp?

const char* fmParamNames[3][35]={
  {_N("Algorithm"), _N("Feedback"), _N("LFO > Freq"), _N("LFO > Amp"), _N("Attack"), _N("Decay"), _N("Decay 2"), _N("Release"), _N("Sustain"), _N("Level"), _N("EnvScale"), _N("Multiplier"), _N("Detune"), _N("Detune 2"), _N("SSG-EG"), _N("AM"), _N("AM Depth"), _N("Vibrato Depth"), _N("Sustained"), _N("Sustained"), _N("Level Scaling"), _N("Sustain"), _N("Vibrato"), _N("Waveform"), _N("Scale Rate"), _N("OP2 Half Sine"), _N("OP1 Half Sine"), _N("EnvShift"), _N("Reverb"), _N("Fine"), _N("LFO2 > Freq"), _N("LFO2 > Amp"), _N("Octave"), _N("TL Ramp"), _N("Tremolo Sensitivity")},
  {"ALG", "FB", "FMS/PMS", "AMS", "AR", "DR", "SR", "RR", "SL", "TL", "KS", "MULT", "DT", "DT2", "SSG-EG", "AM", "AMD", "FMD", "EGT", "EGT", "KSL", "SUS", "VIB", "WS", "KSR", "DC", "DM", "EGS", "REV", "Fine", "FMS/PMS2", "AMS2", "Block", "TL Ramp", "Tremolo Sensitivity"},
  {"ALG", "FB", "FMS/PMS", "AMS", "AR", "DR", "D2R", "RR", "SL", "TL", "RS", "MULT", "DT", "DT2", "SSG-EG", "AM", "DAM", "DVB", "EGT", "EGS", "KSL", "SUS", "VIB", "WS", "KSR", "DC", "DM", "EGS", "REV", "Fine", "FMS/PMS2", "AMS2", "Block", "TL Ramp", "Tremolo Sensitivity"}
};

const char* esfmParamLongNames[9]={
  _N("OP4 Noise Mode"),
  _N("Envelope Delay"),
  _N("Output Level"),
  _N("Modulation Input Level"),
  _N("Left Output"),
  _N("Right Output"),
  _N("Coarse Tune (semitones)"),
  _N("Detune"),
  _N("Fixed Frequency Mode")
};

const char* esfmParamNames[9]={
  _N("OP4 Noise Mode"),
  _N("Env. Delay"),
  _N("Output Level"),
  _N("ModInput"),
  _N("Left"),
  _N("Right"),
  _N("Tune"),
  _N("Detune"),
  _N("Fixed")
};

const char* esfmParamShortNames[9]={
  "NOI", "DL", "OL", "MI", "L", "R", "CT", "DT", "FIX"
};

const char* fmParamShortNames[3][35]={
  {"ALG", "FB", "FMS", "AMS", "A", "D", "D2", "R", "S", "TL", "RS", "ML", "DT", "DT2", "SSG", "AM", "DAM", "DVB", "SUS", "SUS", "KSL", "SUS", "VIB", "WS", "KSR", "DC", "DM", "EGS", "REV", "Fine", "FMS2", "AMS2", "Blk", "TLR", "TS"},
  {"ALG", "FB", "FMS", "AMS", "A", "D", "SR", "R", "S", "TL", "KS", "ML", "DT", "DT2", "SSG", "AM", "AMD", "FMD", "EGT", "EGT", "KSL", "SUS", "VIB", "WS", "KSR", "DC", "DM", "EGS", "REV", "Fine", "FMS2", "AMS2", "Blk", "TLR", "TS"},
  {"ALG", "FB", "FMS", "AMS", "A", "D", "D2", "R", "S", "TL", "RS", "ML", "DT", "DT2", "SSG", "AM", "DAM", "DVB", "EGT", "EGS", "KSL", "SUS", "VIB", "WS", "KSR", "DC", "DM", "EGS", "REV", "Fine", "FMS2", "AMS2", "Blk", "TLR", "TS"}
};

const char* fmOperatorBits[5]={
  "op1", "op2", "op3", "op4", NULL
};

const int orderedOps[4]={
  0, 2, 1, 3
};

const char* ssgEnvBits[5]={
  "0", "1", "2", _N("enabled"), NULL
};