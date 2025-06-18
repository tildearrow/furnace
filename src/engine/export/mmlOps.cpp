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
  bool loopMarkerWritten[4] = {false, false, false, false};

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
  String mmlChanStream[4] = {"", "", "", ""};
  String chanNames[4] = {"A", "B", "C", "D"};

  // Rows per measure from current subsong highlighting
  int rowsPerMeasure = curSubSong->hilightB;

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
    const auto& cname = chanNames[chan]; // Use chanNames array instead of local variable
    if (ticksElapsed > 0) {
        // Always ensure we have a channel name at the start of a line
        if (!st.chanNameUsed[chan]) {
            mmlStream += cname + " ";
            st.chanNameUsed[chan] = true;
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
    // Check for loop point FIRST, before any command processing
    for (int chan = 0; chan < 4; chan++) {
        if (!loopMarkerWritten[chan] && curOrder == loopOrder && curRow == loopRow) {
            String& mmlStream = mmlChanStream[chan];
            const String& cname = chanNames[chan]; // Now this is properly accessible
            
            // Finish any pending note/rest before loop marker
            int ticksElapsed = tick - st.cmdTick[chan];
            if (ticksElapsed > 0) {
                updateMmlStream(chan, ticksElapsed);
            }
            
            // Add loop marker on its own line
            mmlStream += "\n" + cname + " L\n";
            st.chanNameUsed[chan] = false; // Reset so next content gets channel name
            loopMarkerWritten[chan] = true;
        }
    }

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
      String& mmlStream = mmlChanStream[chan];

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
      int safeRowsPerMeasure = rowsPerMeasure < 1 ? 1 : rowsPerMeasure;

      if ((curRow - 1) / safeRowsPerMeasure != (st.prevRow[chan] - 1) / safeRowsPerMeasure) {
        // Force a new line and reset channel name usage
        mmlStream += "\n";
        st.chanNameUsed[chan] = false; // This will force the channel name on the next content
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

  // Clean up each channel's MML stream
  for (int chan = 0; chan < 4; chan++) {
      String& stream = mmlChanStream[chan];

      // 1. Remove trailing whitespace from each line
      {
          std::stringstream in(stream);
          String line, cleaned;
          while (std::getline(in, line)) {
              size_t end = line.find_last_not_of(" \t");
              if (end != std::string::npos)
                  line = line.substr(0, end + 1);
              cleaned += line + "\n";
          }
          stream = std::move(cleaned);
      }

      // 2. Compress multiple spaces/tabs into a single space (excluding newlines)
      {
          String result;
          bool inSpace = false;
          for (char c : stream) {
              if (c == ' ' || c == '\t') {
                  if (!inSpace) {
                      result += ' ';
                      inSpace = true;
                  }
              } else {
                  result += c;
                  inSpace = false;
              }
          }
          stream = std::move(result);
      }

      // 3. Ensure channel letters start new lines (except at start)
      {
          String result;
          for (size_t i = 0; i < stream.size(); ++i) {
              if (i > 0 && stream[i] >= 'A' && stream[i] <= 'D' && stream[i + 1] == ' ') {
                  if (stream[i - 1] != '\n') result += '\n';
              }
              result += stream[i];
          }
          stream = std::move(result);
      }

      // 4. Remove lines that only contain a channel letter
      {
          std::stringstream in(stream);
          String line, cleaned;
          while (std::getline(in, line)) {
              String trimmed = line;
              size_t start = trimmed.find_first_not_of(" \t");
              size_t end = trimmed.find_last_not_of(" \t");
              if (start != std::string::npos)
                  trimmed = trimmed.substr(start, end - start + 1);
              if (!(trimmed.size() == 1 && trimmed[0] >= 'A' && trimmed[0] <= 'D'))
                  cleaned += line + "\n";
          }
          stream = std::move(cleaned);
      }

      // 5. Remove trailing newlines and spaces
      {
          size_t end = stream.find_last_not_of(" \t\n");
          if (end != std::string::npos)
              stream = stream.substr(0, end + 1);
          else
              stream.clear();
      }

      // 6. Prepend channel letter if missing
      if (!stream.empty() && !(stream[0] >= 'A' && stream[0] <= 'D' && stream[1] == ' ')) {
          stream = chanNames[chan] + " " + stream;
      }
  }


  // Combine all channels - only add newlines between non-empty channels
  String output = "";
  for (int chan = 0; chan < 4; chan++) {
      if (!mmlChanStream[chan].empty()) {
          if (!output.empty()) {
              output += "\n\n";
          }
          output += mmlChanStream[chan];
      }
  }

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
