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

#include "engine.h"
#include "../ta-log.h"
#include "../utfutils.h"
#include "safeWriter.h"
#include "song.h"
#include <fmt/printf.h>
#include <array>
#include <vector>

constexpr int MASTER_CLOCK_PREC=(sizeof(void*)==8)?8:0;
constexpr int MASTER_CLOCK_MASK=(sizeof(void*)==8)?0xff:0;

static std::string ticksToTime(double rate, int ticks) {
  double timing = ticks / rate;

  return fmt::sprintf("%02d:%02d.%03d",
    (int) timing / 60,
    (int) timing % 60,
    (int) (timing * 1000) % 1000
  );
}

SafeWriter* DivEngine::saveSAPR(int sapScanlines) {
  int POKEY=-1;
  int IGNORED=0;

  // Locate system index.
  for (int i=0; i<song.systemLen; i++) {
    if (song.system[i] == DIV_SYSTEM_POKEY) {
        if (POKEY>=0) {
          IGNORED++;
          logD("Ignoring duplicate POKEY id %d",i);
          break;
        }
        POKEY=i;
        logD("POKEY detected as chip id %d",i);
        break;
    } else {
      IGNORED++;
      logD("Ignoring chip id %d, system id %d",i,(int)song.system[i]);
      break;
    }
  }
  if (POKEY<0) {
    logE("Could not find POKEY");
    return NULL;
  }
  if (IGNORED>0) {
    logW("SAP export ignoring %d unsupported system%c",IGNORED,IGNORED>1?'s':' ');
  }

  stop();
  repeatPattern=false;
  setOrder(0);
  BUSY_BEGIN_SOFT;

  bool palTiming = song.systemFlags[POKEY].getInt("clockSel",0) != 0;
  int scanlinesPerFrame = (palTiming?312:262);
  if (sapScanlines <= 0) {
    sapScanlines = scanlinesPerFrame;
  }
  double origRate = got.rate;
  //double sapRate = (palTiming?49.86:59.92) * scanlinesPerFrame / sapScanlines;
  double sapRate = (palTiming?50:60) * scanlinesPerFrame / sapScanlines;
  got.rate=sapRate;

  // Determine loop point.
  int loopOrder=0;
  int loopRow=0;
  int loopEnd=0;
  walkSong(loopOrder,loopRow,loopEnd);
  logI("loop point: %d %d",loopOrder,loopRow);
  warnings="";

  // Reset the playback state.
  curOrder=0;
  freelance=false;
  playing=false;
  extValuePresent=false;
  remainingLoops=-1;

  // Prepare to write song data.
  playSub(false);
  size_t tickCount=0;
  bool done=false;
  int fracWait=0; // accumulates fractional ticks
  disCont[POKEY].dispatch->toggleRegisterDump(true);

  std::vector<std::array<uint8_t, 9>> regs;
  std::array<uint8_t, 9> currRegs;

  while (!done) {
    if (nextTick() || !playing) {
      done=true;
      for (int i=0; i<song.systemLen; i++) {
        disCont[i].dispatch->getRegisterWrites().clear();
      }
      break;
    }
    // get register dumps
    std::vector<DivRegWrite>& writes=disCont[POKEY].dispatch->getRegisterWrites();
    if (writes.size() > 0) {
      logD("saprOps: found %d messages",writes.size());
      for (DivRegWrite& write: writes)
        if ((write.addr & 0xF) < 9)
          currRegs[write.addr & 0xF] = write.val;
      writes.clear();
    }

    // write wait
    tickCount++;
    int totalWait=cycles>>MASTER_CLOCK_PREC;
    fracWait+=cycles&MASTER_CLOCK_MASK;
    totalWait+=fracWait>>MASTER_CLOCK_PREC;
    fracWait&=MASTER_CLOCK_MASK;
    if (totalWait>0 && !done) {
      while (totalWait) {
        regs.push_back(currRegs);
        totalWait--;
        tickCount++;
      }
    }
  }
  // end of song

  // done - close out.
  got.rate=origRate;
  disCont[POKEY].dispatch->toggleRegisterDump(false);

  remainingLoops=-1;
  playing=false;
  freelance=false;
  extValuePresent=false;

  auto w = new SafeWriter;
  w->init(); 
  // Write SAP header: Author, name, timing, type.
  w->writeText(fmt::sprintf("SAP\r\nAUTHOR \"%s\"\r\nNAME \"%s\"\r\n%s\r\nTYPE R\r\n",
    song.author, song.name, palTiming ? "PAL" : "NTSC"
  ));
  if (sapScanlines != scanlinesPerFrame) {
    // Fastplay.
    w->writeText(fmt::sprintf("FASTPLAY %d\r\n", sapScanlines));
  }
  // Time.
  w->writeText(fmt::sprintf("TIME %s\r\n", ticksToTime(sapRate, tickCount)));
  w->writeText("\r\n");
  // Registers.
  for (auto atRegs : regs) {
    w->write(atRegs.data(), 9 * sizeof(uint8_t));
  }

  BUSY_END;
  return w;
}
