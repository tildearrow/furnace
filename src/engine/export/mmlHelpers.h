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

#include <string>
#include <map>
#include <cmath>
#include <algorithm>
#include <regex>
#include "../song.h"
#include "../safeWriter.h"   // for SafeWriter
#include "../wavetable.h"    // for DivWavetable

int _computeMmlOctave(int noteValue, int delta);
int _computeMmlTempo(DivSubSong* curSubSong, double divider);
std::string _generateOctShift(int diff, bool reversedNotation);
std::string _computeMmlOctString(int prevOctave, int noteValue, int delta, bool reversedNotation);

struct _MMLGBState {
  int prevVol[4];
  int currVol[4];
  int volTick[4];
  int prevPan[4];
  int currPan[4];
  int prevIns[4];
  int currIns[4];
  int currNote[4];
  int currOct[4];
  int prevOrder[4];
  int prevRow[4];
  int cmdTick[4];
  bool chanNameUsed[4];
  bool noteOn[4];
  int prevDuty[2];
  int currDuty[2];
  int dutyTick[2];
  int prevEnv[4];
  int currEnv[4];
  int prevWave;
  int currWave;
  int waveTick;
  int prevNoise;
  int currNoise;
  int noiseTick;
  int currTempo;
  int tempoTick;

  _MMLGBState();
};

extern const std::map<int, std::string> _mmlTickMap;

int _findLargestTick(int maxTick);
std::string _computeMmlPureTick(int ticks, int maxTicks, bool tiePrefix, bool tieNonFractionalTicks);
std::string _computeMmlTickLengthGB(_MMLGBState* state, int ticks, int maxTicks, int tick, int chan, bool tieNonFractionalTicks = true);
std::string _computeMmlNote(int noteValue);

// Noise table declarations
int _convertNoiseValue(int noteValue, bool useLegacyNoiseTable);

// Waveform writing
void _writeWaveData(SafeWriter* w, const std::function<int(int)>& valueFunc);
void _writeNormalizedGBWave(DivWavetable* wave, SafeWriter* w);
void _writeDefaultWave(SafeWriter* w);

// Command writing
void _writeCommonGBChannelState(_MMLGBState* state, int chan, std::string& result);
std::string _writeMMLGBCommands(_MMLGBState* state, int chan);
