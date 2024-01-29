/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2024 tildearrow and contributors
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

#include "stringsUtil.h"

const char* ssgEnvTypes[8]={
  "Down Down Down", "Down.", "Down Up Down Up", "Down UP", "Up Up Up", "Up.", "Up Down Up Down", "Up DOWN"
};

const char* fmParamNames[3][32]={
  {"Algorithm", "Feedback", "LFO > Freq", "LFO > Amp", "Attack", "Decay", "Decay 2", "Release", "Sustain", "Level", "EnvScale", "Multiplier", "Detune", "Detune 2", "SSG-EG", "AM", "AM Depth", "Vibrato Depth", "Sustained", "Sustained", "Level Scaling", "Sustain", "Vibrato", "Waveform", "Scale Rate", "OP2 Half Sine", "OP1 Half Sine", "EnvShift", "Reverb", "Fine", "LFO2 > Freq", "LFO2 > Amp"},
  {"ALG", "FB", "FMS/PMS", "AMS", "AR", "DR", "SR", "RR", "SL", "TL", "KS", "MULT", "DT", "DT2", "SSG-EG", "AM", "AMD", "FMD", "EGT", "EGT", "KSL", "SUS", "VIB", "WS", "KSR", "DC", "DM", "EGS", "REV", "Fine", "FMS/PMS2", "AMS2"},
  {"ALG", "FB", "FMS/PMS", "AMS", "AR", "DR", "D2R", "RR", "SL", "TL", "RS", "MULT", "DT", "DT2", "SSG-EG", "AM", "DAM", "DVB", "EGT", "EGS", "KSL", "SUS", "VIB", "WS", "KSR", "DC", "DM", "EGS", "REV", "Fine", "FMS/PMS2", "AMS2"}
};

const char* esfmParamLongNames[9]={
  "OP4 Noise Mode", "Envelope Delay", "Output Level", "Modulation Input Level", "Left Output", "Right Output", "Coarse Tune (semitones)", "Detune", "Fixed Frequency Mode"
};

const char* esfmParamNames[9]={
  "OP4 Noise Mode", "Env. Delay", "Output Level", "ModInput", "Left", "Right", "Tune", "Detune", "Fixed"
};

const char* esfmParamShortNames[9]={
  "NOI", "DL", "OL", "MI", "L", "R", "CT", "DT", "FIX"
};

const char* fmParamShortNames[3][32]={
  {"ALG", "FB", "FMS", "AMS", "A", "D", "D2", "R", "S", "TL", "RS", "ML", "DT", "DT2", "SSG", "AM", "DAM", "DVB", "SUS", "SUS", "KSL", "SUS", "VIB", "WS", "KSR", "DC", "DM", "EGS", "REV", "Fine", "FMS2", "AMS2"},
  {"ALG", "FB", "FMS", "AMS", "A", "D", "SR", "R", "S", "TL", "KS", "ML", "DT", "DT2", "SSG", "AM", "AMD", "FMD", "EGT", "EGT", "KSL", "SUS", "VIB", "WS", "KSR", "DC", "DM", "EGS", "REV", "Fine", "FMS2", "AMS2"},
  {"ALG", "FB", "FMS", "AMS", "A", "D", "D2", "R", "S", "TL", "RS", "ML", "DT", "DT2", "SSG", "AM", "DAM", "DVB", "EGT", "EGS", "KSL", "SUS", "VIB", "WS", "KSR", "DC", "DM", "EGS", "REV", "Fine", "FMS2", "AMS2"}
};

const char* opllVariants[4]={
  "OPLL",
  "YMF281",
  "YM2423",
  "VRC7"
};

const char* opllInsNames[4][17]={
  /* YM2413 */ {
    "User",
    "1. Violin",
    "2. Guitar",
    "3. Piano",
    "4. Flute",
    "5. Clarinet",
    "6. Oboe",
    "7. Trumpet",
    "8. Organ",
    "9. Horn",
    "10. Synth",
    "11. Harpsichord",
    "12. Vibraphone",
    "13. Synth Bass",
    "14. Acoustic Bass",
    "15. Electric Guitar",
    "Drums"
  },
  /* YMF281 */ {
    "User",
    "1. Electric String",
    "2. Bow wow",
    "3. Electric Guitar",
    "4. Organ",
    "5. Clarinet",
    "6. Saxophone",
    "7. Trumpet",
    "8. Street Organ",
    "9. Synth Brass",
    "10. Electric Piano",
    "11. Bass",
    "12. Vibraphone",
    "13. Chime",
    "14. Tom Tom II",
    "15. Noise",
    "Drums"
  },
  /* YM2423 */ {
    "User",
    "1. Strings",
    "2. Guitar",
    "3. Electric Guitar",
    "4. Electric Piano",
    "5. Flute",
    "6. Marimba",
    "7. Trumpet",
    "8. Harmonica",
    "9. Tuba",
    "10. Synth Brass",
    "11. Short Saw",
    "12. Vibraphone",
    "13. Electric Guitar 2",
    "14. Synth Bass",
    "15. Sitar",
    "Drums"
  },
  // stolen from FamiTracker
  /* VRC7 */ {
    "User",
    "1. Bell",
    "2. Guitar",
    "3. Piano",
    "4. Flute",
    "5. Clarinet",
    "6. Rattling Bell",
    "7. Trumpet",
    "8. Reed Organ",
    "9. Soft Bell",
    "10. Xylophone",
    "11. Vibraphone",
    "12. Brass",
    "13. Bass Guitar",
    "14. Synth",
    "15. Chorus",
    "Drums"
  }
};

const char* oplWaveforms[8]={
  "Sine", "Half Sine", "Absolute Sine", "Quarter Sine", "Squished Sine", "Squished AbsSine", "Square", "Derived Square"
};

const char* oplWaveformsStandard[8]={
  "Sine", "Half Sine", "Absolute Sine", "Pulse Sine", "Sine (Even Periods)", "AbsSine (Even Periods)", "Square", "Derived Square"
};

const char* opzWaveforms[8]={
  "Sine", "Triangle", "Cut Sine", "Cut Triangle", "Squished Sine", "Squished Triangle", "Squished AbsSine", "Squished AbsTriangle"
};

const char* oplDrumNames[4]={
  "Snare", "Tom", "Top", "HiHat"
};

const char* esfmNoiseModeNames[4]={
  "Normal", "Snare", "HiHat", "Top"
};

const char* esfmNoiseModeDescriptions[4]={
  "Noise disabled", "Square + noise", "Ringmod from OP3 + noise", "Ringmod from OP3 + double pitch ModInput\nWARNING - has emulation issues, subject to change"
};

const bool opIsOutput[8][4]={
  {false,false,false,true},
  {false,false,false,true},
  {false,false,false,true},
  {false,false,false,true},
  {false,true,false,true},
  {false,true,true,true},
  {false,true,true,true},
  {true,true,true,true}
};

const bool opIsOutputOPL[4][4]={
  {false,false,false,true},
  {true,false,false,true},
  {false,true,false,true},
  {true,false,true,true}
};

const char* fmOperatorBits[5]={
  "op1", "op2", "op3", "op4", NULL
};

const char* c64ShapeBits[5]={
  "triangle", "saw", "pulse", "noise", NULL
};

const char* ayShapeBits[4]={
  "tone", "noise", "envelope", NULL
};

const char* ayEnvBits[4]={
  "hold", "alternate", "direction", "enable"
};

const char* ssgEnvBits[5]={
  "0", "1", "2", "enabled", NULL
};

const char* saaEnvBits[9]={
  "mirror", "loop", "cut", "direction", "resolution", "fixed", "N/A","enabled", NULL
};

const char* snesModeBits[6]={
  "noise", "echo", "pitch mod", "invert right", "invert left", NULL
};

const char* filtModeBits[5]={
  "low", "band", "high", "ch3off", NULL
};

const char* c64TestGateBits[5]={
  "gate", "sync", "ring", "test", NULL
};

const char* pokeyCtlBits[9]={
  "15KHz", "filter 2+4", "filter 1+3", "16-bit 3+4", "16-bit 1+2", "high3", "high1", "poly9", NULL
};

const char* mikeyFeedbackBits[11] = {
  "0", "1", "2", "3", "4", "5", "7", "10", "11", "int", NULL
};

const char* msm5232ControlBits[7]={
  "16'", "8'", "4'", "2'", "sustain", NULL
};

const char* tedControlBits[3]={
  "square", "noise", NULL
};

const char* c219ControlBits[4]={
  "noise", "invert", "surround", NULL
};

const char* x1_010EnvBits[8]={
  "enable", "oneshot", "split L/R", "HinvR", "VinvR", "HinvL", "VinvL", NULL
};

/*const char* n163UpdateBits[8]={
  "now", "every waveform changed", NULL
};*/

const char* suControlBits[5]={
  "ring mod", "low pass", "high pass", "band pass", NULL
};

const char* es5506FilterModes[4]={
  "HP/K2, HP/K2", "HP/K2, LP/K1", "LP/K2, LP/K2", "LP/K2, LP/K1",
};

const char* panBits[5]={
  "right", "left", "rear right", "rear left", NULL
};

const char* powerNoiseControlBits[3]={
  "enable tap B", "AM with slope", NULL
};

const char* powerNoiseSlopeControlBits[7]={
  "invert B", "invert A", "reset B", "reset A", "clip B", "clip A", NULL
};

const char* oneBit[2]={
  "on", NULL
};

const char* es5506EnvelopeModes[3]={
  "k1 slowdown", "k2 slowdown", NULL
};

const char* es5506ControlModes[3]={
  "pause", "reverse", NULL
};

const int orderedOps[4]={
  0, 2, 1, 3
};

const char* singleWSEffects[7]={
  "None",
  "Invert",
  "Add",
  "Subtract",
  "Average",
  "Phase",
  "Chorus"
};

const char* dualWSEffects[9]={
  "None (dual)",
  "Wipe",
  "Fade",
  "Fade (ping-pong)",
  "Overlay",
  "Negative Overlay",
  "Slide",
  "Mix Chorus",
  "Phase Modulation"
};

const char* gbHWSeqCmdTypes[6]={
  "Envelope",
  "Sweep",
  "Wait",
  "Wait for Release",
  "Loop",
  "Loop until Release"
};

const char* suHWSeqCmdTypes[7]={
  "Volume Sweep",
  "Frequency Sweep",
  "Cutoff Sweep",
  "Wait",
  "Wait for Release",
  "Loop",
  "Loop until Release"
};

const char* snesGainModes[5]={
  "Direct",
  "Decrease (linear)",
  "Decrease (logarithmic)",
  "Increase (linear)",
  "Increase (bent line)"
};

const int detuneMap[2][8]={
  {-3, -2, -1, 0, 1, 2, 3, 4},
  { 7,  6,  5, 0, 1, 2, 3, 4}
};

const int detuneUnmap[2][11]={
  {0, 1, 2, 3, 4, 5, 6, 7, 0, 0, 0},
  {0, 0, 0, 3, 4, 5, 6, 7, 2, 1, 0}
};

const int kslMap[4]={
  0, 2, 1, 3
};

// do not change these!
// anything other than a checkbox will look ugly!
//
// if you really need to, and have a good rationale (and by good I mean a VERY
// good one), please tell me and we'll sort it out.
const char* macroAbsoluteMode="Fixed";
const char* macroRelativeMode="Relative";
const char* macroQSoundMode="QSound";
const char* macroDummyMode="Bug";