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
    void close(bool careAboutError=true);
    Pipe(): readFd(-1), writeFd(-1) {}
    Pipe(int pipeArr[2]): readFd(pipeArr[0]), writeFd(pipeArr[1]) {}
  };

private:
  std::vector<String> args;
  pid_t childPid=-1;
  Pipe stdinPipe, stdoutPipe, stderrPipe;

public:
  Subprocess(std::vector<String> args);
  ~Subprocess();

  // These functions enable piping, and return a file descriptor to an end of the created pipe.
  // On stdin, it's the writable end; on stdout and stderr, it's the readable end.
  // These should only be called before start(). They return -1 on error.
  int pipeStdin();
  int pipeStdout();
  int pipeStderr();

  void closeStdinPipe(bool careAboutError);

  // starts the subprocess.
  // returns whether it successfully started
  bool start();

  // waits for the process to finish and returns its exit code.
  // should only be called after start()
  int wait();

  // checks whether the subprocess is on running-mode
  bool isRunning() const;
};

#endif
