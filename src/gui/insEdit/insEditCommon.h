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

#ifndef _INS_EDIT_COMMON_H
#define _INS_EDIT_COMMON_H
#include "../gui.h"

#define P(x) if (x) { \
  MARK_MODIFIED; \
  e->notifyInsChange(curIns); \
  updateFMPreview=true; \
}

#define PARAMETER MARK_MODIFIED; e->notifyInsChange(curIns); updateFMPreview=true;

#define MACRO_WAVE_COUNT MAX(1,e->song.waveLen-1)

void addAALine(ImDrawList* dl, const ImVec2& p1, const ImVec2& p2, const ImU32 color, float thickness=1.0f);

extern const char* panBits[5];

extern const char* macroAbsoluteMode;
extern const char* macroRelativeMode;
extern const char* macroQSoundMode;
extern const char* macroDummyMode;

String macroHoverNote(int id, float val, void* u);
String macroLFOWaves(int id, float val, void* u);

enum FMParams {
  FM_ALG=0,
  FM_FB=1,
  FM_FMS=2,
  FM_AMS=3,
  FM_AR=4,
  FM_DR=5,
  FM_D2R=6,
  FM_RR=7,
  FM_SL=8,
  FM_TL=9,
  FM_RS=10,
  FM_MULT=11,
  FM_DT=12,
  FM_DT2=13,
  FM_SSG=14,
  FM_AM=15,
  FM_DAM=16,
  FM_DVB=17,
  FM_EGT=18,
  FM_EGS=19,
  FM_KSL=20,
  FM_SUS=21,
  FM_VIB=22,
  FM_WS=23,
  FM_KSR=24,
  FM_DC=25,
  FM_DM=26,
  FM_EGSHIFT=27,
  FM_REV=28,
  FM_FINE=29,
  FM_FMS2=30,
  FM_AMS2=31,
  FM_BLOCK=32,
  FM_TLRAMP=33,
  FM_TS=34
};

enum ESFMParams {
  ESFM_NOISE=0,
  ESFM_DELAY=1,
  ESFM_OUTLVL=2,
  ESFM_MODIN=3,
  ESFM_LEFT=4,
  ESFM_RIGHT=5,
  ESFM_CT=6,
  ESFM_DT=7,
  ESFM_FIXED=8
};

extern const char* fmParamNames[3][35];
extern const char* fmParamShortNames[3][35];
extern const char* esfmParamLongNames[9];
extern const char* esfmParamNames[9];
extern const char* esfmParamShortNames[9];

extern const char* fmOperatorBits[5];
extern const int orderedOps[4];
extern const char* ssgEnvBits[5];

#define FM_NAME(x) _(fmParamNames[settings.fmNames][x])
#define FM_SHORT_NAME(x) fmParamShortNames[settings.fmNames][x]
#define ESFM_LONG_NAME(x) _(esfmParamLongNames[x])
#define ESFM_NAME(x) _(esfmParamNames[x])
#define ESFM_SHORT_NAME(x) (esfmParamShortNames[x])

#endif
