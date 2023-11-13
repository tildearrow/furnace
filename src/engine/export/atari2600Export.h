/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2022 tildearrow and contributors
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

#ifndef _ATARI2600_EXPORT_H
#define _ATARI2600_EXPORT_H

#include "../engine.h"
#include "registerDump.h"

class DivExportAtari2600 : public DivROMExport {
  
  void writeWaveformHeader(SafeWriter* w, const char* key);
  size_t writeTextGraphics(SafeWriter* w, const char* value);
  size_t writeNote(SafeWriter* w, const ChannelState& next, const char duration, const ChannelState& last);

  void writeTrackV0(
    DivEngine* e, 
    std::vector<String> *channelSequences,
    std::map<String, DumpSequence> &registerDumps,
    std::vector<DivROMExportOutput> &ret
  );

  void writeTrackV1(
    DivEngine* e, 
    std::map<uint64_t, String> &commonDumpSequences,
    std::map<uint64_t, unsigned int> &frequencyMap,
    std::map<String, String> &representativeMap,
    std::map<String, DumpSequence> &registerDumps,
    std::vector<DivROMExportOutput> &ret
  );

  void writeTrackV2(
    DivEngine* e, 
    std::map<uint64_t, String> &commonDumpSequences,
    std::map<String, String> &representativeMap,
    std::vector<String> *channelSequences,
    std::map<String, DumpSequence> &registerDumps,
    std::vector<DivROMExportOutput> &ret
  );

public:

  ~DivExportAtari2600() {}

  std::vector<DivROMExportOutput> go(DivEngine* e) override;

};

#endif // _ATARI2600_EXPORT_H