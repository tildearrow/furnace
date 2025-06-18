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

#include "../fileOps/fileOpsCommon.h"
#include "mmlHelpers.h"

SafeWriter* DivEngine::saveMMLGB(bool useLegacyNoiseTable) {
  stop();
  repeatPattern = false;
  shallStop = false;
  setOrder(0);

  BUSY_BEGIN_SOFT;

  // Determine loop point in the song (order, row, loopEnd)
  int loopOrder = 0, loopRow = 0, loopEnd = 0;
  walkSong(loopOrder, loopRow, loopEnd);
  logI("loop point: %d %d", loopOrder, loopRow);

  // Unused but kept for potential future frequency analysis / statistics
  // int cmdPopularity[256] = {};
  // int delayPopularity[256] = {};
  // int sortedCmdPopularity[16] = {};
  // int sortedDelayPopularity[16] = {};
  // unsigned char sortedCmd[16] = {};
  // unsigned char sortedDelay[16] = {};
  // SafeWriter* chanStream[DIV_MAX_CHANS] = {};
  // unsigned int chanStreamOff[DIV_MAX_CHANS] = {};

  SafeWriter* w = new SafeWriter;
  w->init();

  // Write header info with song and subsong metadata
  w->writeText("; Furnace MML-GB Output\n;\n");
  w->writeText("; Information:\n");
  w->writeText(fmt::sprintf("; \tname: %s\n", song.name));
  w->writeText(fmt::sprintf("; \tauthor: %s\n", song.author));
  w->writeText(fmt::sprintf("; \tcategory: %s\n", song.category));
  w->writeText(";\n");
  w->writeText("; SubSongInformation:\n");
  w->writeText(fmt::sprintf("; \tname: %s\n", curSubSong->name));
  w->writeText("\n");

  // Write WaveTable Macros, default or custom
  w->writeText("; WaveTable Macros:\n");
  if (song.waveLen == 0) {
    w->writeText("@wave0 = {");
    _writeDefaultWave(w);
    w->writeText("}\n");
  } else {
    for (int i = 0; i < song.waveLen; i++) {
      auto wave = getWave(i);
      w->writeText(fmt::sprintf("@wave%d = {", i));
      _writeNormalizedGBWave(wave, w);
      w->writeText("}\n");
    }
  }
  w->writeText("\n");
  w->writeText("; Sequence data:\n");

  // Prepare for playback and MML generation
  playSub(false);
  bool done = false;
  int tick = 0;
  bool oldCmdStreamEnabled = cmdStreamEnabled;
  cmdStreamEnabled = true;
  double curDivider = divider;

  // Strings to accumulate MML output per channel
  std::string mmlChanStream[4] = {"", "", "", ""};
  std::string chanNames[4] = {"A", "B", "C", "D"};

  // Rows per measure from current subsong highlighting
  int rowsPerMeasure = curSubSong->hilightB;
  // Unused but kept if pattern length needed later
  // int rowsPerPattern = curSubSong->patLen;

  // Calculate tempo for MML export based on current subsong and divider
  int tempo = _computeMmlTempo(curSubSong, curDivider);

  // Write initial tempo commands for all channels
  for (const auto& cname : chanNames) {
    w->writeText(fmt::sprintf("%s t%d\n", cname.c_str(), tempo));
  }
  w->writeText("\n\n");

  // MML generation state
  _MMLGBState st{};
  st.currTempo = tempo;
  st.tempoTick = -1;
  std::fill(std::begin(st.cmdTick), std::end(st.cmdTick), 0);
  std::fill(std::begin(st.prevOrder), std::end(st.prevOrder), -1);
  std::fill(std::begin(st.prevRow), std::end(st.prevRow), -1);
  std::fill(std::begin(st.chanNameUsed), std::end(st.chanNameUsed), false);
  std::fill(std::begin(st.noteOn), std::end(st.noteOn), false);

  // Helper to update the MML stream of a channel given elapsed ticks
  auto updateMmlStream = [&](int chan, int ticksElapsed) {
    auto& mmlStream = mmlChanStream[chan];
    auto& cname = chanNames[chan];
    if (ticksElapsed > 0) {
      if (!st.chanNameUsed[chan]) {
        mmlStream += cname + " ";
      }
      mmlStream += _writeMMLGBCommands(&st, chan)
                 + (st.noteOn[chan] ? "" : _computeMmlOctString(st.currOct[chan], st.currNote[chan], 0, false))
                 + (st.noteOn[chan] ? "^" : (st.currNote[chan] < 0 ? "r" : _computeMmlNote(st.currNote[chan])))
                 + _computeMmlTickLengthGB(&st, ticksElapsed, 192, tick, chan);

      st.cmdTick[chan] = tick;

      if (st.currNote[chan] >= 0) {
        st.currOct[chan] = _computeMmlOctave(st.currNote[chan], 0);
      }
      if (!st.noteOn[chan]) st.noteOn[chan] = true;
    }
  };

  // Main playback and MML generation loop
  while (!done) {
    if (nextTick(false, true) || !playing) {
      done = true;
    }

    bool wroteTick[DIV_MAX_CHANS] = {};
    memset(wroteTick, 0, sizeof(wroteTick));

    // Check for tempo changes due to divider adjustments
    if (curDivider != divider) {
      curDivider = divider;
      int newTempo = _computeMmlTempo(curSubSong, curDivider);
      if (newTempo != st.currTempo) {
        st.currTempo = newTempo;
        st.tempoTick = tick;
      }
    }

    // Process all commands in the current tick's command stream
    for (DivCommand& cmd : cmdStream) {
      // Skip commands not relevant for MML export
      switch (cmd.cmd) {
        case DIV_CMD_GET_VOLUME:
        case DIV_CMD_VOLUME:
        case DIV_CMD_NOTE_PORTA:
        case DIV_CMD_LEGATO:
        case DIV_CMD_PITCH:
        case DIV_CMD_PRE_NOTE:
          continue;
        default:
          break;
      }

      int chan = cmd.chan % 4;
      std::string& mmlStream = mmlChanStream[chan];
      const std::string& cname = chanNames[chan];

      int ticksElapsed = tick - st.cmdTick[chan];
      updateMmlStream(chan, ticksElapsed);

      // Handle different command types for state update
      switch (cmd.cmd) {
        case DIV_CMD_NOTE_ON: {
          int noteVal = (chan == 3) ? _convertNoiseValue(cmd.value, useLegacyNoiseTable) : cmd.value;
          st.currNote[chan] = noteVal;
          st.noteOn[chan] = false;

          auto ins = getIns(st.currIns[chan], DivInstrumentType::DIV_INS_GB);

          if (chan != 2) {
            if (!ins->gb.softEnv) {
              // Initialize envelope if applicable
              if ((ins->gb.alwaysInit || st.prevIns[chan] != st.currIns[chan]) && tick != st.volTick[chan]) {
                st.currVol[chan] = ins->gb.envVol;
              }
              st.currEnv[chan] = ((ins->gb.envDir != 0) ? 1 : -1) * ins->gb.envLen;
            }
            // Set duty cycle for pulse channels or noise channel duty
            if (chan < 2 && ins->std.dutyMacro.len > 0) {
              st.currDuty[chan] = ins->std.dutyMacro.val[0];
            } else if (chan == 3 && ins->std.dutyMacro.len > 0) {
              st.currNoise = std::min(1, ins->std.dutyMacro.val[0]);
            }
          } else if (chan == 2 && ins->std.waveMacro.len > 0) {
            st.currWave = ins->std.waveMacro.val[0];
            if (st.currWave >= song.waveLen) st.currWave = song.waveLen - 1;
          }

          st.prevIns[chan] = st.currIns[chan];
          break;
        }
        case DIV_CMD_NOTE_OFF:
        case DIV_CMD_NOTE_OFF_ENV:
          st.currNote[chan] = -1;
          st.noteOn[chan] = false;
          break;
        case DIV_CMD_INSTRUMENT:
          st.currIns[chan] = cmd.value;
          break;
        case DIV_CMD_HINT_VOLUME:
          if (chan == 2) st.noteOn[chan] = false;
          st.currVol[chan] = cmd.value;
          st.volTick[chan] = tick;
          break;
        case DIV_CMD_PANNING: {
          int l = cmd.value;
          int r = cmd.value2;
          if ((l == 0) == (r == 0)) st.currPan[chan] = 0;
          else if (l == 0) st.currPan[chan] = 1;
          else st.currPan[chan] = -1;
          break;
        }
        case DIV_CMD_STD_NOISE_MODE:
          if (chan < 2) {
            st.currDuty[chan] = cmd.value;
            st.dutyTick[chan] = tick;
          } else if (chan == 3) {
            st.currNoise = cmd.value;
            st.noiseTick = tick;
            st.noteOn[chan] = false;
          }
          break;
        case DIV_CMD_WAVE:
          if (chan == 2) {
            st.currWave = cmd.value;
            if (st.currWave >= song.waveLen) st.currWave = song.waveLen - 1;
            st.waveTick = tick;
            st.noteOn[chan] = false;
          }
          break;
        default:
          break;
      }

      // Handle pattern and measure boundaries for formatting new lines
      if (rowsPerMeasure > 0) {
        if ((curRow - 1) / rowsPerMeasure != (st.prevRow[chan] - 1) / rowsPerMeasure) {
          mmlStream += "\n" + cname + " ";
          st.chanNameUsed[chan] = true;
        }
      }

      st.prevRow[chan] = curRow;
      st.prevOrder[chan] = curOrder;
    }
    cmdStream.clear();
    tick++;
  }

  cmdStreamEnabled = oldCmdStreamEnabled;

  // Write leftover notes after finishing playback
  for (int chan = 0; chan < 4; chan++) {
    int ticksElapsed = tick - st.cmdTick[chan] - 1;
    if (ticksElapsed > 0) {
      if (!st.chanNameUsed[chan]) {
        mmlChanStream[chan] += chanNames[chan] + " ";
      }
      mmlChanStream[chan] += _writeMMLGBCommands(&st, chan)
                         + (st.noteOn[chan] ? "" : _computeMmlOctString(st.currOct[chan], st.currNote[chan], 0, false))
                         + (st.noteOn[chan] ? "^" : (st.currNote[chan] < 0 ? "r" : _computeMmlNote(st.currNote[chan])))
                         + _computeMmlTickLengthGB(&st, ticksElapsed, 192, tick - 1, chan);
    }
  }

  // Cleanup MML streams: compress multiple spaces into one
  for (auto& stream : mmlChanStream) {
    stream = std::regex_replace(stream, std::regex("[ ]{2,}"), " ");
  }

  // Concatenate all channels with spacing and remove lines with only channel letters
  std::string output = std::regex_replace(
    mmlChanStream[0] + "\n\n\n" + mmlChanStream[1] + "\n\n\n" +
    mmlChanStream[2] + "\n\n\n" + mmlChanStream[3] + "\n",
    std::regex("[A-D][ ]*\\n"), ""
  );
  output = std::regex_replace(output, std::regex("[\\n]{4,}"), "\n\n\n");

  w->writeText(output);

  // Reset flags after export
  remainingLoops = -1;
  playing = false;
  freelance = false;
  extValuePresent = false;

  BUSY_END;

  return w;
}

SafeWriter* DivEngine::saveMMLSNESAMK(int amkVersion) {
    SafeWriter* w = new SafeWriter;
    w->init();
    w->writeText("; Dummy saveMMLSNESAMK output\n");
    return w;
}
