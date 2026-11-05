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

#ifndef _FUR_CLI_H
#define _FUR_CLI_H


#include "../engine/engine.h"

#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <signal.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#endif

class FurnaceCLI {
  DivEngine* e;
  bool disableStatus;
  bool disableControls;

#ifdef _WIN32
  HANDLE winin;
  HANDLE winout;
#else
  struct sigaction intsa;
  struct termios termprop;
  struct termios termpropold;
#endif

  public:
    void noStatus();
    void noControls();
    void bindEngine(DivEngine* eng);
    bool loop();
    bool finish();
    bool init();
    FurnaceCLI();
};

#endif
