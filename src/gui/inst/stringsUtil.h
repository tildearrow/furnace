/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2023 tildearrow and contributors
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

#pragma once

extern const char* ssgEnvTypes[8];
extern const char* fmParamNames[3][32];
extern const char* esfmParamLongNames[9];
extern const char* esfmParamNames[9];
extern const char* esfmParamShortNames[9];
extern const char* fmParamShortNames[3][32];
extern const char* opllVariants[4];
extern const char* opllInsNames[4][17];
extern const char* oplWaveforms[8];
extern const char* oplWaveformsStandard[8];
extern const char* opzWaveforms[8];
extern const char* oplDrumNames[4];
extern const char* esfmNoiseModeNames[4];
extern const char* esfmNoiseModeDescriptions[4];
extern const bool opIsOutput[8][4];
extern const bool opIsOutputOPL[4][4];
extern const char* fmOperatorBits[5];
extern const char* c64ShapeBits[5];
extern const char* ayShapeBits[4];
extern const char* ayEnvBits[4];
extern const char* ssgEnvBits[5];
extern const char* saaEnvBits[9];
extern const char* snesModeBits[6];
extern const char* filtModeBits[5];
extern const char* c64TestGateBits[5];
extern const char* pokeyCtlBits[9];
extern const char* mikeyFeedbackBits[11];
extern const char* msm5232ControlBits[7];
extern const char* tedControlBits[3];
extern const char* c219ControlBits[4];
extern const char* x1_010EnvBits[8];
extern const char* suControlBits[5];
extern const char* es5506FilterModes[4];
extern const char* panBits[5];
extern const char* oneBit[2];
extern const char* es5506EnvelopeModes[3];
extern const char* es5506ControlModes[3];
extern const int orderedOps[4];
extern const char* singleWSEffects[7];
extern const char* dualWSEffects[9];
extern const char* gbHWSeqCmdTypes[6];
extern const char* suHWSeqCmdTypes[7];
extern const char* snesGainModes[5];
extern const int detuneMap[2][8];
extern const int detuneUnmap[2][11];
extern const int kslMap[4];
extern const char* macroAbsoluteMode;
extern const char* macroRelativeMode;
extern const char* macroQSoundMode;
extern const char* macroDummyMode;