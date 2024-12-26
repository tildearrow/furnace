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
#include "subprocess.h"
#include "ta-log.h"

#define tryMakePipe(_arrVar) \
  int _arrVar[2]; \
  int status=pipe(_arrVar); \
  if (status==-1) { \
    return -1; \
  }

Subprocess::Subprocess(std::vector<String> args):
  args(args)
{}

Subprocess::~Subprocess() {
  logD("closing pipes!");
  const auto closePipe = [](Subprocess::Pipe& p){
    if (p.readFd!=-1)
      close(p.readFd);
    if (p.writeFd!=-1)
      close(p.writeFd);
  };
  closePipe(stdinPipe);
  closePipe(stdoutPipe);
  closePipe(stderrPipe);
}

int Subprocess::pipeStdin() {
  tryMakePipe(arr);
  stdinPipe=Subprocess::Pipe(arr);
  logD("made pipe, fds (%d,%d) ~ (%d,%d)\n",arr[0],arr[1],stdinPipe.writeFd,stdinPipe.readFd);
  return stdinPipe.writeFd;
}

int Subprocess::pipeStdout() {
  tryMakePipe(arr);
  stdinPipe=Subprocess::Pipe(arr);
  return stdinPipe.readFd;
}

int Subprocess::pipeStderr() {
  tryMakePipe(arr);
  stdinPipe=Subprocess::Pipe(arr);
  return stdinPipe.readFd;
}

bool Subprocess::start() {
  childPid=fork();

  if (childPid==-1 || args.size()==0) {
    return false;
  }

  if (childPid==0) {
    // child process

    // set the desired pipes
    if (stdinPipe.readFd!=-1) {
      dup2(stdinPipe.readFd,STDIN_FILENO);
    }
    if (stdoutPipe.writeFd!=-1) {
      dup2(stdoutPipe.writeFd,STDOUT_FILENO);
    }
    if (stderrPipe.writeFd!=-1) {
      dup2(stderrPipe.writeFd,STDERR_FILENO);
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
    return true;
  }
}

int Subprocess::wait() {
  // TODO
  childPid=-1;
  return -1;
}
