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

// guiConst: constants used in the GUI like arrays, strings and other stuff

enum FurnaceGUIChanTypes {
  // the first five match DivChanTypes, do not change order!

  CHANNEL_TYPE_FM,
  CHANNEL_TYPE_PULSE,
  CHANNEL_TYPE_NOISE,
  CHANNEL_TYPE_WAVETABLE,
  CHANNEL_TYPE_SAMPLE, 

  CHANNEL_TYPE_SQUARE,
  CHANNEL_TYPE_TRIANGLE,
  CHANNEL_TYPE_SAW,
  CHANNEL_TYPE_OPERATOR,
  CHANNEL_TYPE_DRUMS,
  CHANNEL_TYPE_SLOPE,
  CHANNEL_TYPE_WAVE,
  CHANNEL_TYPE_PSG,
  CHANNEL_TYPE_OTHER,

  CHANNEL_TYPE_MAX
};

struct FurnaceGUIActionDef {
  const char* name;
  const char* friendlyName;
  std::vector<int> defaultBind;
  bool isNotABind() const { return defaultBind.size()==1 && defaultBind[0]==-1; }
  FurnaceGUIActionDef(const char* n, const char* fn, int db) :
    name(n), friendlyName(fn) {
    if (db!=0) defaultBind.push_back(db);
  }
  FurnaceGUIActionDef(const char* n, const char* fn, int db, int db2):
    name(n), friendlyName(fn) {
    if (db!=0) defaultBind.push_back(db);
    if (db2!=0) defaultBind.push_back(db);
  }
};

struct FurnaceGUIColorDef {
  const char* name;
  const char* friendlyName;
  unsigned int defaultColor;
  FurnaceGUIColorDef(const char* n, const char* fn, unsigned int dc):
    name(n), friendlyName(fn), defaultColor(dc) {}
};

extern const int opOrder[4];
extern const float topKeyStarts[5];
extern const int topKeyNotes[5];
extern const int bottomKeyNotes[7];
extern const bool isTopKey[12];
extern const char* noteNames[180];
extern const char* noteNamesG[180];
extern const char* noteNamesF[180];
extern const char* noteNamesGF[180];
extern const char* pitchLabel[11];
extern const char* insTypes[][3];
extern const char* sampleLoopModes[];
extern const char* sampleDepths[];
extern const char* resampleStrats[];
extern const char* chipCategoryNames[];
extern const int availableSystems[];
extern const int chipsFM[];
extern const int chipsSquare[];
extern const int chipsWavetable[];
extern const int chipsSpecial[];
extern const int chipsSample[];
extern const int* chipCategories[];
extern const FurnaceGUIActionDef guiActions[];
extern const FurnaceGUIColorDef guiColors[];
extern const int altValues[24];
extern const int vgmVersions[7];
extern const FurnaceGUIColors fxColors[256];
extern const char* fxColorsNames[10];
extern const char* chanNames[];
extern unsigned char chanNamesHierarchy[];
extern const char* triggerStates[3];
