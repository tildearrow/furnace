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
  bool loopMarkerWritten[4] = {
    false,
    false,
    false,
    false
  };

  // For progress estimation or song export
  bool songHasSongEndCommand = false;
  int songFadeoutSectionLength = 0;
  std::vector < int > songOrdersLengths;
  int songLength = 0;

  // Get tick position of loop point
  int loopTick = 0;

  findSongLength(
    loopOrder,
    loopRow,
    0.0, // No need for fadeout length
    songFadeoutSectionLength,
    songHasSongEndCommand,
    songOrdersLengths,
    songLength,
    loopTick
  );

  logI("loop tick: %d", loopTick);

  SafeWriter* w = new SafeWriter;
  w -> init();

  // Write header info with song and subsong metadata
  w -> writeText("; Furnace MML-GB Output\n;\n");
  w -> writeText("; Information:\n");
  w -> writeText(fmt::sprintf("; \tname: %s\n", song.name));
  w -> writeText(fmt::sprintf("; \tauthor: %s\n", song.author));
  w -> writeText(fmt::sprintf("; \tcategory: %s\n", song.category));
  w -> writeText(";\n");
  w -> writeText("; SubSongInformation:\n");
  w -> writeText(fmt::sprintf("; \tname: %s\n", curSubSong -> name));
  w -> writeText("\n");

  // Write WaveTable Macros, default or custom
  w -> writeText("; WaveTable Macros:\n");
  if (song.waveLen == 0) {
    w -> writeText("@wave0 = {");
    _writeDefaultWave(w);
    w -> writeText(" }\n");
  } else {
    for (int i = 0; i < song.waveLen; i++) {
      DivWavetable* wave = getWave(i);
      w -> writeText(fmt::sprintf("@wave%d = {", i));
      _writeNormalizedGBWave(wave, w);
      w -> writeText("}\n");
    }
  }
  w -> writeText("\n");
  w -> writeText("; Sequence data:\n");

  // Prepare for playback and MML generation
  playSub(false);
  bool done = false;
  int tick = 0;
  double curDivider = divider;
  cmdStreamEnabled = true;
  
  // Strings to accumulate MML output per channel
  String mmlChanStream[4] = {
    "",
    "",
    "",
    ""
  };
  String chanNames[4] = {
    "A",
    "B",
    "C",
    "D"
  };

  // Rows per measure from current subsong highlighting
  int rowsPerMeasure = curSubSong -> hilightB;

  // Calculate tempo for MML export based on current subsong and divider
  int tempo = _computeMmlTempo(curSubSong, curDivider);

  // Write initial tempo commands for all channels
  for (const String& cname: chanNames) {
    w -> writeText(fmt::sprintf("%s t%d\n", cname.c_str(), tempo));
  }
  w -> writeText("\n\n");

  // MML generation state
  _MMLGBState st {};
  st.currTempo = tempo;
  st.tempoTick = -1;
  std::fill(std::begin(st.cmdTick), std::end(st.cmdTick), 0);
  std::fill(std::begin(st.prevOrder), std::end(st.prevOrder), -1);
  std::fill(std::begin(st.prevRow), std::end(st.prevRow), -1);
  std::fill(std::begin(st.chanNameUsed), std::end(st.chanNameUsed), false);
  std::fill(std::begin(st.noteOn), std::end(st.noteOn), false);

  // Helper to update the MML stream of a channel given elapsed ticks
  auto updateMmlStream = [ & ](int chan, int ticksElapsed) {
    String& mmlStream = mmlChanStream[chan];
    const String& cname = chanNames[chan];
    if (ticksElapsed > 0) {
      if (!st.chanNameUsed[chan]) {
        mmlStream += cname + " ";
        st.chanNameUsed[chan] = true;
      }

      // Always compute octave and note for held note
      mmlStream += _writeMMLGBCommands(&st, chan);

      if (st.currNote[chan] < 0) {
        mmlStream += "r";
      } else {
        mmlStream += _computeMmlOctString(st.currOct[chan], st.currNote[chan], 0, false);
        mmlStream += _computeMmlNote(st.currNote[chan]);
      }

      mmlStream += _computeMmlTickLengthGB(&st, ticksElapsed, 192, tick, chan);
      st.cmdTick[chan] = tick;

      if (st.currNote[chan] >= 0)
        st.currOct[chan] = _computeMmlOctave(st.currNote[chan], 0);

      st.noteOn[chan] = true;
    }
  };

  // Main playback and MML generation loop
  while (!done) {
    // Check for loop point FIRST, before any command processing
    for (int chan = 0; chan < 4; chan++) {
      if (!loopMarkerWritten[chan] && curOrder == loopOrder && curRow == loopRow) {
        String& mmlStream = mmlChanStream[chan];
        const String& cname = chanNames[chan];

        // Always write any pending note content up to the loop point
        if (st.cmdTick[chan] < loopTick) {
          int durationToLoop = loopTick - st.cmdTick[chan];

          if (!st.chanNameUsed[chan]) {
            mmlStream += cname + " ";
            st.chanNameUsed[chan] = true;
          }

          mmlStream += _writeMMLGBCommands(&st, chan);

          if (st.currNote[chan] < 0) {
            mmlStream += "r";
          } else {
            mmlStream += _computeMmlOctString(st.currOct[chan], st.currNote[chan], 0, false);
            mmlStream += _computeMmlNote(st.currNote[chan]);
          }

          mmlStream += _computeMmlTickLengthGB(&st, durationToLoop, 192, loopTick, chan);

          if (st.currNote[chan] >= 0)
            st.currOct[chan] = _computeMmlOctave(st.currNote[chan], 0);

          // Important: update noteOn state after writing the note
          st.noteOn[chan] = (st.currNote[chan] >= 0);
        }

        // Update command tick to the loop point
        st.cmdTick[chan] = loopTick;

        // Add loop marker on its own line
        mmlStream += "\n" + cname + " L";

        // Write current channel state after loop marker
        String stateCommands = "";

        // Volume - wave channel has different range (0-3) vs others (0-15)
        if (chan == 2) {
          // Wave channel volume is 0-3, defer to 3 if invalid
          int waveVol = (st.currVol[chan] >= 0 && st.currVol[chan] <= 3) ? st.currVol[chan] : 3;
          stateCommands += fmt::sprintf(" v%d", waveVol);
        } else {
          // Other channels volume is 0-15, defer to 15 if invalid
          int vol = (st.currVol[chan] >= 0 && st.currVol[chan] <= 15) ? st.currVol[chan] : 15;
          stateCommands += fmt::sprintf(" v%d", vol);
        }

        // Octave: 0-9, defer to 4 if invalid
        int octave = (st.currOct[chan] >= 0 && st.currOct[chan] <= 9) ? st.currOct[chan] : 4;
        stateCommands += fmt::sprintf(" o%d", octave);

        // Volume envelope (for channels 0, 1, 3 - not wave channel 2): -7 to 7, defer to 0 if invalid
        if (chan != 2) {
          int envelope = (st.currEnv[chan] >= -7 && st.currEnv[chan] <= 7) ? st.currEnv[chan] : 0;
          if (envelope != 0) {
            stateCommands += fmt::sprintf(" @ve%d", envelope);
          }
        }

        // Duty cycle for pulse channels (0, 1): 0-3, defer to 0 if invalid
        if (chan < 2) {
          int duty = (st.currDuty[chan] >= 0 && st.currDuty[chan] <= 3) ? st.currDuty[chan] : 0;
          stateCommands += fmt::sprintf(" @wd%d", duty);
        }

        // Wave for wave channel (2): must be valid wave index
        if (chan == 2) {
          int wave = st.currWave;
          if (wave < 0 || wave >= song.waveLen) {
            wave = (song.waveLen > 0) ? 0 : 0; // Use first wave or 0 if no waves
          }
          stateCommands += fmt::sprintf(" @wave%d", wave);
        }

        // Noise mode for noise channel (3): 0 or 1, defer to 0 if invalid
        if (chan == 3) {
          int noise = (st.currNoise == 0 || st.currNoise == 1) ? st.currNoise : 0;
          stateCommands += fmt::sprintf(" @ns%d", noise);
        }

        // Panning: -1, 0, or 1, defer to 0 if invalid
        int panning = (st.currPan[chan] >= -1 && st.currPan[chan] <= 1) ? st.currPan[chan] : 0;
        stateCommands += fmt::sprintf(" y%d", panning);
        mmlStream += stateCommands + "\n";

        // Reset state for after loop
        st.chanNameUsed[chan] = false;
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
    for (DivCommand & cmd: cmdStream) {
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
        if (chan == 3) {
            if (cmd.value < 0x100) {
                st.currNote[chan] = _convertNoiseValue(cmd.value, useLegacyNoiseTable);
            }
        } else {
            st.currNote[chan] = cmd.value;
        }
        st.noteOn[chan] = false;

        DivInstrument* ins = getIns(st.currIns[chan], DivInstrumentType::DIV_INS_GB);

        if (chan != 2) {
          if (!ins -> gb.softEnv) {
            // Initialize envelope if applicable
            if ((ins -> gb.alwaysInit || st.prevIns[chan] != st.currIns[chan]) && tick != st.volTick[chan]) {
              st.currVol[chan] = ins -> gb.envVol;
            }
            st.currEnv[chan] = ((ins -> gb.envDir != 0) ? 1 : -1)* ins -> gb.envLen;
          }
          // Set duty cycle for pulse channels or noise channel duty
          if (chan < 2 && ins -> std.dutyMacro.len > 0) {
            st.currDuty[chan] = ins -> std.dutyMacro.val[0];
          } else if (chan == 3 && ins -> std.dutyMacro.len > 0) {
            st.currNoise = std::min(1, ins -> std.dutyMacro.val[0]);
          }
        } else if (chan == 2 && ins -> std.waveMacro.len > 0) {
          st.currWave = ins -> std.waveMacro.val[0];
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

  // Write leftover notes after finishing playback
  for (int chan = 0; chan < 4; chan++) {
    int ticksElapsed = tick - st.cmdTick[chan] - 1;
    if (ticksElapsed > 0) {
      if (!st.chanNameUsed[chan]) {
        mmlChanStream[chan] += chanNames[chan] + " ";
      }
      mmlChanStream[chan] += _writeMMLGBCommands(&st, chan) +
        (st.noteOn[chan] ? "" : _computeMmlOctString(st.currOct[chan], st.currNote[chan], 0, false)) +
        (st.noteOn[chan] ? "^" : (st.currNote[chan] < 0 ? "r" : _computeMmlNote(st.currNote[chan]))) +
        _computeMmlTickLengthGB(&st, ticksElapsed, 192, tick - 1, chan);
    }
  }

  // Clean up each channel's MML stream
  for (int chan = 0; chan < 4; chan++) {
    String& stream = mmlChanStream[chan];

    // 1. Remove trailing whitespace from each line
    {
      String cleaned;
      size_t start = 0;
      while (start < stream.size()) {
        size_t end = stream.find('\n', start);
        if (end == String::npos) end = stream.size();

        String line = stream.substr(start, end - start);

        size_t lastNonWS = line.find_last_not_of(" \t");
        if (lastNonWS != String::npos)
          line = line.substr(0, lastNonWS + 1);

        cleaned += line + '\n';
        start = end + 1;
      }
      stream = std::move(cleaned);
    }

    // 2. Compress multiple spaces/tabs into a single space (excluding newlines)
    {
      String result;
      bool inSpace = false;
      for (char c: stream) {
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
          if (stream[i - 1] != '\n')
            result += '\n';
        }
        result += stream[i];
      }
      stream = std::move(result);
    }
    
    // 4. Remove lines that only contain a channel letter (with optional spaces/tabs)
    {
      String cleaned;
      size_t start = 0;
      while (start < stream.size()) {
        size_t end = stream.find('\n', start);
        if (end == String::npos) end = stream.size();

        String line = stream.substr(start, end - start);

        // Trim line
        size_t first = line.find_first_not_of(" \t");
        size_t last = line.find_last_not_of(" \t");

        bool shouldRemove = false;
        if (first != String::npos) {
          line = line.substr(first, last - first + 1);
          // Remove if it's exactly one letter A-D or that letter followed by whitespace only
          if ((line.size() == 1 && line[0] >= 'A' && line[0] <= 'D') ||
              (line.size() > 1 && line[0] >= 'A' && line[0] <= 'D' &&
              line.substr(1).find_first_not_of(" \t") == String::npos)) {
            shouldRemove = true;
          }
        } else {
          // Entire line is whitespace
          shouldRemove = true;
        }

        if (!shouldRemove)
          cleaned += stream.substr(start, end - start) + '\n';

        start = end + 1;
      }
      stream = std::move(cleaned);
    }

    // 5. Remove trailing newlines and spaces
    {
      size_t end = stream.find_last_not_of(" \t\n");
      if (end != String::npos)
        stream = stream.substr(0, end + 1);
      else
        stream.clear();

      // 6. Prepend channel letter if missing
      if (!stream.empty() && !(stream[0] >= 'A' && stream[0] <= 'D' && stream[1] == ' ')) {
        stream = chanNames[chan] + " " + stream;
      }
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
  
  // Fix spacing inside wave braces @waveX = { ... }
  output = std::regex_replace(output, std::regex("\\{(\\S)"), "{ $1");
  output = std::regex_replace(output, std::regex("(\\S)\\}"), "$1 }");
  
  w -> writeText(output);

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
  w -> init();
  w -> writeText("; Dummy saveMMLSNESAMK output\n");
  return w;
}