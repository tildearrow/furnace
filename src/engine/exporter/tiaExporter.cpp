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

#include "tiaExporter.h"

SafeWriter* TiaTrackerROMBuilder::buildROM(DivEngine & e, int sysIndex) {
  e.stop();
  e.setOrder(0);
//  BUSY_BEGIN_SOFT;
//   // determine loop point
//   int loopOrder=0;
//   int loopRow=0;
//   int loopEnd=0;
//   walkSong(loopOrder,loopRow,loopEnd);
//   logI("loop point: %d %d",loopOrder,loopRow);

  SafeWriter* w=new SafeWriter;
  w->init();

  w->writeText("TIATracker data");
//   // write header
//   if (binary) {
//     w->write("FCS",4);
//   } else {

//     w->writeText("[Information]\n");
//     w->writeText(fmt::sprintf("name: %s\n",song.name));
//     w->writeText(fmt::sprintf("author: %s\n",song.author));
//     w->writeText(fmt::sprintf("category: %s\n",song.category));
//     w->writeText(fmt::sprintf("system: %s\n",song.systemName));

//     w->writeText("\n");

//     w->writeText("[SubSongInformation]\n");
//     w->writeText(fmt::sprintf("name: %s\n",curSubSong->name));
//     w->writeText(fmt::sprintf("tickRate: %f\n",curSubSong->hz));

//     w->writeText("\n");

//     w->writeText("[SysDefinition]\n");
//     // TODO

//     w->writeText("\n");
//   }

//   // play the song ourselves
//   bool done=false;
//   playSub(false);
  
//   if (!binary) {
//     w->writeText("[Stream]\n");
//   }
//   int tick=0;
//   bool oldCmdStreamEnabled=cmdStreamEnabled;
//   cmdStreamEnabled=true;
//   double curDivider=divider;
//   int lastTick=0;
//   while (!done) {
//     if (nextTick(false,true) || !playing) {
//       done=true;
//     }
//     // get command stream
//     bool wroteTick=false;
//     if (curDivider!=divider) {
//       curDivider=divider;
//       WRITE_TICK;
//       if (binary) {
//         w->writeC(0xfb);
//         w->writeI((int)(curDivider*65536));
//       } else {
//         w->writeText(fmt::sprintf(">> SET_RATE %f\n",curDivider));
//       }
//     }
//     for (DivCommand& i: cmdStream) {
//       switch (i.cmd) {
//         // strip away hinted/useless commands
//         case DIV_ALWAYS_SET_VOLUME:
//           break;
//         case DIV_CMD_GET_VOLUME:
//           break;
//         case DIV_CMD_VOLUME:
//           break;
//         case DIV_CMD_NOTE_PORTA:
//           break;
//         case DIV_CMD_LEGATO:
//           break;
//         case DIV_CMD_PITCH:
//           break;
//         case DIV_CMD_PRE_NOTE:
//           break;
//         default:
//           WRITE_TICK;
//           if (binary) {
//             w->writeC(i.chan);
//             writePackedCommandValues(w,i);
//           } else {
//             w->writeText(fmt::sprintf("  %d: %s %d %d\n",i.chan,cmdName[i.cmd],i.value,i.value2));
//           }
//           break;
//       }
//     }
//     cmdStream.clear();
//     tick++;
//   }
//   cmdStreamEnabled=oldCmdStreamEnabled;

//   if (binary) {
//     w->writeC(0xff);
//   } else {
//     if (!playing) {
//       w->writeText(">> END\n");
//     } else {
//       w->writeText(">> LOOP 0\n");
//     }
//   }

//   remainingLoops=-1;
//   playing=false;
//   freelance=false;
//   extValuePresent=false;
//   BUSY_END;

  return w;
}
    