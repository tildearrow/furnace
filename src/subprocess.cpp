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

#include "subprocess.h"
#include "ta-log.h"

#ifndef _WIN32
#include <unistd.h>
#include <sys/wait.h>
#include <poll.h>
#include <signal.h>

#define TRY_MAKE_PIPE(_arrVar) \

/**
 * Try to make a pipe.
 *
 * @return whether it succeeded.
 */
static bool makePipe(Subprocess::Pipe *p) {
  int arr[2];
  if (::pipe(arr)<0) return false;

  *p=Subprocess::Pipe(arr);
  return true;
}

void Subprocess::Pipe::close(bool careAboutError) {
  const auto closeFd=[careAboutError](int& fd) {
    if (fd==-1) return;
    if (::close(fd)==-1 && careAboutError) {
      logE("Subprocess::Pipe::close: failed to close fd %d: %s", fd, strerror(errno));
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

int Subprocess::pipeStdin() {
  if (status!=SUBPROCESS_NOT_STARTED) return -1;

  if (!makePipe(&stdinPipe)) return -1;
  return stdinPipe.writeFd;
}

int Subprocess::pipeStdout() {
  if (status!=SUBPROCESS_NOT_STARTED) return -1;

  if (!makePipe(&stdoutPipe)) return -1;
  return stdoutPipe.readFd;
}

int Subprocess::pipeStderr() {
  if (status!=SUBPROCESS_NOT_STARTED) return -1;

  if (!makePipe(&stderrPipe)) return -1;
  return stderrPipe.readFd;
}

bool Subprocess::start() {
  if (status!=SUBPROCESS_NOT_STARTED) return false;

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
    // FIXME: I think this is not needed? Because it wasn't happening anyway and it still worked
    // const auto closeEnd=[](int& fd) {
    //   if (fd!=-1) return;
    //   close(fd);
    //   fd=-1;
    // };
    // closeEnd(stdinPipe.readFd);
    // closeEnd(stdinPipe.writeFd);
    // closeEnd(stdinPipe.writeFd);

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

void Subprocess::closeStdoutPipe(bool careAboutError) {
  stdoutPipe.close(careAboutError);
}

void Subprocess::closeStderrPipe(bool careAboutError) {
  stderrPipe.close(careAboutError);
}

bool Subprocess::waitStdinOrExit() {
  struct pollfd pf;
  pf.fd=stdinPipe.writeFd;
  pf.events=POLLOUT;

  while (true) {
    // I've lost my patience completely so let's just throw a poll call with a timeout here.
    int pollResult=poll(&pf,1,250);
    if (pollResult==-1) {
      logE("failed to use ppoll (%s)\n",strerror(errno));
      return false;
    }

    if (pollResult>0) return true;

    // TODO: refactor this, it's ugly! (getExitCodeNoWait() might work but it's not 100% the same thing)
    int status;
    int result=waitpid(childPid,&status,WNOHANG);
    if (result!=0) {
      if ((result==-1 && errno==ECHILD) || (result==0 && WIFEXITED(status))) {
        // pipe closed!
        return false;
      } else {
        logE("Subprocess::waitStdinOrExit: unknown waitpid result");
        return false;
      }
    }
  }
}
#endif
