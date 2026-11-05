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

#include "cli.h"
#include "../ta-log.h"

bool cliQuit=false;

#ifndef _WIN32
static void handleTerm(int) {
  cliQuit=true;
}
#endif

void FurnaceCLI::noStatus() {
  disableStatus=true;
}

void FurnaceCLI::noControls() {
  disableControls=true;
}

void FurnaceCLI::bindEngine(DivEngine* eng) {
  e=eng;
}

bool FurnaceCLI::loop() {
  if (disableControls) {
    while (!cliQuit) {
#ifdef _WIN32
      Sleep(1000);
#else
      pause();
#endif
    }
    return true;
  }

  bool escape=false;
  bool escapeSecondStage=false;
  while (!cliQuit) {
#ifdef _WIN32
    int c;
    c=fgetc(stdin);
    if (c==EOF) break;
#else
    unsigned char c;
    if (read(STDIN_FILENO,&c,1)<=0) continue;
#endif
    if (escape) {
      if (escapeSecondStage) {
        switch (c) {
          case 'C': // right
            e->setOrder(e->getOrder()+1);
            escape=false;
            escapeSecondStage=false;
            break;
          case 'D': // left
            e->setOrder(e->getOrder()-1);
            escape=false;
            escapeSecondStage=false;
            break;
          default:
            escape=false;
            escapeSecondStage=false;
            break;
        }
      } else {
        switch (c) {
          case '[': case 'O':
            escapeSecondStage=true;
            break;
          default:
            escape=false;
            break;
        }
      }
    } else {
      switch (c) {
        case 0x1b: // <ESC>
          escape=true;
          break;
        case 'h': // left
          e->setOrder(e->getOrder()-1);
          break;
        case 'l': // right
          e->setOrder(e->getOrder()+1);
          break;
        case ' ':
          if (e->isHalted()) {
            e->resume();
          } else {
            e->halt();
          }
          break;
      }
    }
  }
  printf("\n");
  return true;
}

bool FurnaceCLI::finish() {
  if (disableControls) return true;
#ifdef _WIN32
#else
  if (tcsetattr(0,TCSAFLUSH,&termpropold)!=0) {
    logE("could not set console attributes!");
    logE("you may have to run `reset` on your terminal.");
    return false;
  }
#endif
  return true;
}

// blatantly copied from tildearrow/tfmxplay
bool FurnaceCLI::init() {
#ifdef _WIN32
  if (disableControls) return true;
  
  winin=GetStdHandle(STD_INPUT_HANDLE);
  winout=GetStdHandle(STD_OUTPUT_HANDLE);
  int termprop=0;
  int termpropi=0;
  GetConsoleMode(winout,(LPDWORD)&termprop);
  GetConsoleMode(winin,(LPDWORD)&termpropi);
  termprop|=ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  termpropi&=~ENABLE_LINE_INPUT;
  SetConsoleMode(winout,termprop);
  SetConsoleMode(winin,termpropi);
#else
  sigemptyset(&intsa.sa_mask);
  intsa.sa_flags=0;
  intsa.sa_handler=handleTerm;
  sigaction(SIGINT,&intsa,NULL);

  if (disableControls) return true;

  if (tcgetattr(0,&termprop)!=0) {
    logE("could not get console attributes!");
    return false;
  }
  memcpy(&termpropold,&termprop,sizeof(struct termios));
  termprop.c_lflag&=~ECHO;
  termprop.c_lflag&=~ICANON;
  if (tcsetattr(0,TCSAFLUSH,&termprop)!=0) {
    logE("could not set console attributes!");
    return false;
  }
#endif
  return true;
}

FurnaceCLI::FurnaceCLI():
  e(NULL) {
}
