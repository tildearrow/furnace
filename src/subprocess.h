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

#ifndef _SUBPROCESS_H
#define _SUBPROCESS_H

#include "ta-utils.h"

class Subprocess {
public:
  struct Pipe {
    int readFd;
    int writeFd;
    Pipe(): readFd(-1), writeFd(-1) {}
    Pipe(int pipeArr[2]): readFd(pipeArr[0]), writeFd(pipeArr[1]) {}
    ~Pipe();
  };

private:
  std::vector<String> args;
  bool isRunning=false;
  pid_t childPid=-1;
  Pipe stdinPipe, stdoutPipe, stderrPipe;

public:
  Subprocess(std::vector<String> args);

  // enables stdin piping, and returns a file descriptor to the writable end
  // should only be called before start()
  // returns -1 on error
  int pipeStdin();

  // enables stdout piping, and returns a file decriptor to the readable end
  // should only be called before start()
  // returns -1 on error
  int pipeStdout();

  // enables stderr piping, and returns a file decriptor to the readable end
  // should only be called before start()
  // returns -1 on error
  int pipeStderr();

  // starts the subprocess
  // returns whether it successfully started
  bool start();

  // waits for the process to finish and returns its exit code
  // should only be called after start()
  int wait();
};

#endif
