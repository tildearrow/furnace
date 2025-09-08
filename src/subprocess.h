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

#ifndef _WIN32 // TODO: windows impl
/**
 * Manages a child process and facilitates communication with it through pipes.
 */
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
    enum Status {
      SUBPROCESS_NOT_STARTED,
      SUBPROCESS_RUNNING,
      SUBPROCESS_FINISHED
    };

    std::vector<String> args;
    pid_t childPid=-1;
    int statusCode;
    Pipe stdinPipe, stdoutPipe, stderrPipe;
    Status status=SUBPROCESS_NOT_STARTED;

  public:
    Subprocess(std::vector<String> args);

    /**
     * Create a pipe for communication with the subprocess.
     *
     * Should only be called before `start()`.
     *
     * @return the writable end of the pipe, or -1 on error
     */
    int pipeStdin();

    /**
     * Same as `pipeStdin`, but for stdout.
     *
     * @return the readable end of the pipe, or -1 on error
     */
    int pipeStdout();

    /**
     * Same as `pipeStdin`, but for stderr.
     *
     * @return the readable end of the pipe, or -1 on error
     */
    int pipeStderr();

    void closeStdinPipe(bool careAboutError=true);
    void closeStdoutPipe(bool careAboutError=true);
    void closeStderrPipe(bool careAboutError=true);

    /*
     * Start the subprocess.
     *
     * @return whether it successfully started
     */
    bool start();

    /*
     * Wait for the stdin pipe (write end) to be writable, or for the subprocess
     * to exit.
     *
     * @return whether it got more data in the pipe (if false, it might mean the
     * child process exited, or that an error ocurred).
     */
    bool waitStdinOrExit();

    /**
     * Try to get the subprocess's exit code.
     *
     * For simpler argument passing, see `waitForExitCode` and `getExitCodeNoWait`.
     *
     * @param outCode the destination where the exit code should be written
     *
     * @param wait whether to wait for the subprocess to finish (if it hasn't
     * finished yet)
     *
     * @return whether it managed to get the exit code
     */
    bool getExitCode(int* outCode, bool wait);

    bool waitForExitCode(int* outCode);

    bool getExitCodeNoWait(int* outCode);
};
#endif

#endif
