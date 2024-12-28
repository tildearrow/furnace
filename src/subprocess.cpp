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

#include <unistd.h>
#include <sys/wait.h>
#include "subprocess.h"
#include "ta-log.h"

#define TRY_MAKE_PIPE(_arrVar) \
  int _arrVar[2]; \
  int status=pipe(_arrVar); \
  if (status==-1) { \
    return -1; \
  }

void Subprocess::Pipe::close(bool careAboutError) {
  const auto closeFd=[careAboutError](int& fd) {
    if (fd==-1) return;
    if (::close(fd)==-1 && careAboutError) {
      logE("(Subprocess::Pipe::close) failed to close fd %d: %s", fd, strerror(errno));
      return;
    }
    fd=-1;
  };
  closeFd(readFd);
  closeFd(writeFd);
}

Subprocess::Subprocess(std::vector<String> args):
  args(args)
{}

Subprocess::~Subprocess() {
  stdinPipe.close();
  stdoutPipe.close();
  stderrPipe.close();
}

int Subprocess::pipeStdin() {
  if (status!=SUBPROCESS_NOT_STARTED) return -1;

  TRY_MAKE_PIPE(arr);
  stdinPipe=Subprocess::Pipe(arr);
  return stdinPipe.writeFd;
}

int Subprocess::pipeStdout() {
  if (status!=SUBPROCESS_NOT_STARTED) return -1;

  TRY_MAKE_PIPE(arr);
  stdinPipe=Subprocess::Pipe(arr);
  return stdinPipe.readFd;
}

int Subprocess::pipeStderr() {
  if (status!=SUBPROCESS_NOT_STARTED) return -1;

  TRY_MAKE_PIPE(arr);
  stdinPipe=Subprocess::Pipe(arr);
  return stdinPipe.readFd;
}

bool Subprocess::start() {
  if (status!=SUBPROCESS_NOT_STARTED) return -1;

  childPid=fork();
  if (childPid==-1 || args.size()==0) {
    return false;
  }

  if (childPid==0) {
    // child process

    // connect the desired pipes to the main file descriptors
    // and then close the original copies (to avoid stalling)
    if (stdinPipe.readFd!=-1) {
      dup2(stdinPipe.readFd,STDIN_FILENO);
      stdinPipe.close();
    }
    if (stdoutPipe.writeFd!=-1) {
      dup2(stdoutPipe.writeFd,STDOUT_FILENO);
      stdoutPipe.close();
    }
    if (stderrPipe.writeFd!=-1) {
      dup2(stderrPipe.writeFd,STDERR_FILENO);
      stderrPipe.close();
    }

    // could not find a guaranteed way to cast a vector<String> to a char*[] so let's just create our own array and copy stuff to it
    std::vector<char*> v;
    for (int i=0; i<(int)args.size(); i++) {
      char *mem=(char*)malloc(sizeof(char)*(args[i].size()+1));
      memcpy(mem,args[i].c_str(),args[i].size()+1);
      v.push_back(mem);
    }
    v.push_back(nullptr);

    execvp(v[0],v.data());
    exit(127); // reachable if the execution fails - no need to log, just exit with a bad value and the parent process should be able to detect
  } else {
    // parent process

    // attempt to close the ends not used by the parent process
    // (to avoid stalling)
    const auto closeEnd=[](int& fd) {
      if (fd!=-1) return;
      close(fd);
      fd=-1;
    };
    closeEnd(stdinPipe.readFd);
    closeEnd(stdinPipe.writeFd);
    closeEnd(stdinPipe.writeFd);

    return true;
  }
}

bool Subprocess::getExitCode(int *outCode, bool wait) {
  if (status==SUBPROCESS_NOT_STARTED) {
    return false;
  } else if (status==SUBPROCESS_FINISHED) {
    *outCode=statusCode;
    return true;
  }

  int status;
  int flags=WUNTRACED|WCONTINUED;
  if (!wait) flags|=WNOHANG;
  pid_t result=waitpid(childPid,&status,flags);

  if (result==-1) {
    logE("failed to check status of child %lld: %s\n",childPid,strerror(errno));
    return false;
  }

  if (result==0) return false;
  if (!WIFEXITED(status)) return false;

  // at this point, the process has finished
  status=SUBPROCESS_FINISHED;
  childPid=-1;
  statusCode=WEXITSTATUS(status);
  *outCode=statusCode;
  return true;
}

bool Subprocess::waitForExitCode(int *outCode) {
  return getExitCode(outCode,true);
}

bool Subprocess::getExitCodeNoWait(int *outCode) {
  return getExitCode(outCode,false);
}

void Subprocess::closeStdinPipe(bool careAboutError) {
  stdinPipe.close(careAboutError);
}
