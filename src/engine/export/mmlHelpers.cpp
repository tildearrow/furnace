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

#include "mmlHelpers.h"

int _computeMmlOctave(int noteValue, int delta) {
  return (noteValue)/12+delta;
}

int _computeMmlTempo(DivSubSong* curSubSong, double divider) {
  return 150.0 / (curSubSong->timeBase + 1) * divider / 120.0;
}

std::string _generateOctShift(int diff, bool reversedNotation) {
  char up = reversedNotation ? '<' : '>';
  char down = reversedNotation ? '>' : '<';
  return std::string(std::abs(diff), diff < 0 ? down : up);
}

std::string _computeMmlOctString(int prevOctave, int noteValue, int delta, bool reversedNotation) {
  if (noteValue < 0) return "";

  int newOct = _computeMmlOctave(noteValue, delta);
  int octDiff = newOct - prevOctave;

  if (prevOctave < 0 || std::abs(octDiff) >= 3)
    return " o" + std::to_string(newOct) + " ";
  else if (octDiff == 0)
    return "";
  else
    return _generateOctShift(octDiff, reversedNotation);
}

_MMLGBState::_MMLGBState()
  : prevVol{-1, -1, -1, -1}, // uninitialized
    currVol{15, 15, 15, 15},
    volTick{-1, -1, -1, -1},
    prevPan{-2, -2, -2, -2},
    currPan{0, 0, 0, 0},
    prevIns{-1, -1, -1, -1},
    currIns{0, 0, 0, 0},
    currNote{-1, -1, -1, -1},
    currOct{-1, -1, -1, -1},
    prevOrder{-1, -1, -1, -1},
    prevRow{-1, -1, -1, -1},
    cmdTick{0, 0, 0, 0},
    chanNameUsed{false, false, false, false},
    noteOn{false, false, false, false},
    prevDuty{-1, -1},
    currDuty{0, 0},
    dutyTick{-1, -1},
    prevEnv{-16, -16, -16, -16},
    currEnv{-16, -16, -16, -16},
    prevWave(-1),
    currWave(0),
    waveTick(-1),
    prevNoise(-1),
    currNoise(0),
    noiseTick(-1),
    currTempo(-1),
    tempoTick(-1)
{
  // Constructor body empty
}

const std::map<int, std::string> _mmlTickMap = {
  // Base notes:
  {3, "64"}, {6, "32"}, {12, "16"}, {24, "8"}, {48, "4"}, {96, "2"}, {192, "1"}, // 2-base
  {4, "48"}, {8, "24"}, {16, "12"}, {32, "6"}, {64, "3"}, // 3-base
  // Single dotted notes:
  {9, "32."}, {18, "16."}, {36, "8."}, {72, "4."}, {144, "2."}, // 2-base
  // Double dotted notes:
  {21, "16.."}, {42, "8.."}, {84, "4.."}, {168, "2.."}, // 2-base
  {28, "12.."}, {56, "6.."}, {112, "3.."}, // 3-base
  // Triple dotted notes:
  {45, "8..."}, {90, "4..."}, {180, "2..."}, // 2-base
  {60, "6..."}, {120, "3..."}, // 3-base
  // Quadruple dotted notes:
  {93, "4...."}, {186, "2...."}, // 2-base
  {124, "3...."}, // 3-base
  // Quintuple dotted note:
  {189, "2....."} // 2-base
};

int _findLargestTick(int maxTick) {
  int result = 0;
  for (auto it = _mmlTickMap.begin(); it != _mmlTickMap.end(); ++it) {
    int tick = it->first;
    if (tick <= maxTick) result = tick;
    else break;
  }
  return result;
}

std::string _computeMmlPureTick(int ticks, int maxTicks, bool tiePrefix, bool tieNonFractionalTicks) {
  if (ticks <= 0) return "";

  std::string result = tiePrefix ? "^" : "";
  while (true) {
    int tickCount = std::min(maxTicks, ticks);
    if (_mmlTickMap.find(tickCount) == _mmlTickMap.end()) {
      if (tieNonFractionalTicks) {
        // Split tick count into ties of multiple fractional lengths.
        // Ex. a=160 becomes a2.^12
        int leftovers = tickCount;
        bool start = true;
        while (leftovers >= 3) {
          int diff = _findLargestTick(leftovers);
          result += (start ? "" : "^") + _mmlTickMap.at(diff);
          start = false;
          leftovers -= diff;
        }
        if (leftovers > 0 && leftovers < 3) 
          result += std::string(start ? "" : "^") + "=" + std::to_string(leftovers);
      } else {
        result += "=" + std::to_string(tickCount);
      }
    } else {
      result += _mmlTickMap.at(tickCount);
    }
    ticks -= maxTicks;
    if (ticks > 0) result += "^";
    else break;
  }
  return result;
}

std::string _computeMmlTickLengthGB(_MMLGBState* state, int ticks, int maxTicks, int tick, int chan, bool tieNonFractionalTicks) {
  int oldTick = tick - ticks;
  bool needsTUpdate = chan == 0 && state->tempoTick > oldTick && state->tempoTick <= tick;

  int ticksA = needsTUpdate ? state->tempoTick - oldTick : ticks;
  int ticksB = std::max(0, tick - state->tempoTick);

  if (needsTUpdate) {
    return _computeMmlPureTick(ticksA, maxTicks, false, tieNonFractionalTicks)
        + " t" + std::to_string(state->currTempo) + " "
        + _computeMmlPureTick(ticksB, maxTicks, true, tieNonFractionalTicks);
  } else {
    return _computeMmlPureTick(ticksA, maxTicks, false, tieNonFractionalTicks);
  }
}

std::string _computeMmlNote(int noteValue) {
  static const std::string notes[12] = {
    "c", "c+", "d", "d+", "e", "f", "f+", "g", "g+", "a", "a+", "b"
  };
  int mod = noteValue % 12;
  return (mod >= 0 && mod < 12) ? notes[mod] : "r";
}

void _writeCommonGBChannelState(_MMLGBState* state, int chan, std::string& result) {
  if (state->prevVol[chan] != state->currVol[chan]) {
    result += fmt::sprintf(" v%d ", state->currVol[chan]);
    state->prevVol[chan] = state->currVol[chan];
  }
  if (state->prevEnv[chan] != state->currEnv[chan]) {
    result += fmt::sprintf(" @ve%d ", state->currEnv[chan]);
    state->prevEnv[chan] = state->currEnv[chan];
  }
  if (state->prevPan[chan] != state->currPan[chan]) {
    result += fmt::sprintf(" y%d ", state->currPan[chan]);
    state->prevPan[chan] = state->currPan[chan];
  }
}

std::string _writeMMLGBCommands(_MMLGBState* state, int chan) {
  std::string result;

  if (chan == 0 || chan == 1) {
    // --- GB Pulse channels ---
    if (state->prevDuty[chan] != state->currDuty[chan]) {
      result += fmt::sprintf(" @wd%d ", state->currDuty[chan]);
      state->prevDuty[chan] = state->currDuty[chan];
    }
  } else if (chan == 2) {
    // --- GB Wave channel ---
    if (state->prevVol[chan] / 4 != state->currVol[chan] / 4) {
      result += fmt::sprintf(" v%d ", state->currVol[chan] / 4);
      state->prevVol[chan] = state->currVol[chan];
    }
    if (state->prevWave != state->currWave) {
      result += fmt::sprintf(" @wave%d ", state->currWave);
      state->prevWave = state->currWave;
    }
  } else if (chan == 3) {
    // --- GB Noise channel ---
    if (state->prevNoise != state->currNoise) {
      result += fmt::sprintf(" @ns%d ", state->currNoise);
      state->prevNoise = state->currNoise;
    }
  }

  // --- Shared state for channels 0–3 ---
  if (chan < 4) {
    _writeCommonGBChannelState(state, chan, result);
  }

  return result;
}

int _convertNoiseValue(int noteValue, bool useLegacyNoiseTable) {
    if (noteValue < 0) noteValue = 0;
    if (noteValue > 255) noteValue = 255;

    if (useLegacyNoiseTable) {
        // Legacy noise table logic:
        if (noteValue == 0) return 107;
        if (noteValue >= 1 && noteValue <= 47) {
            int r = (noteValue - 1) % 4 + 12;
            return r;
        }
        if (noteValue >= 48 && noteValue <= 59) {
            return noteValue + 34;
        }
        return 107; // fallback for values above 59
    } else {
        // Current noise table logic:
        if (noteValue == 0) return 107;
        if (noteValue >= 1 && noteValue <= 47) {
            int r = (noteValue - 1) % 4 + 12;
            return r;
        }
        if (noteValue >= 48 && noteValue <= 59) {
            return noteValue + 60;
        }
        if (noteValue >= 60 && noteValue <= 71) {
            return 108 + (noteValue - 60);
        }
        return 127; // fallback for values above 71
    }
}

void _writeWaveData(SafeWriter* w, const std::function<double(int)>& valueFunc) {
  for (int x = 0; x < 32; x++) {
    w->writeText(std::to_string(static_cast<int>(valueFunc(x))) + (x < 31 ? " " : ""));
  }
}

void _writeNormalizedGBWave(DivWavetable* wave, SafeWriter* w) {
  _writeWaveData(w, [=](int x) -> double {
    int sx = static_cast<int>(x * wave->len / 32.0);
    return static_cast<double>(wave->data[sx]) * 16.0 / (wave->max - wave->min + 1);
  });
}

void _writeDefaultWave(SafeWriter* w) {
  _writeWaveData(w, [](int x) -> double {
    return static_cast<double>(x) / 2;
  });
}
