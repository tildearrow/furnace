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

#include "executils.h"

static bool execHandlerReady=false;

#ifdef _WIN32
#else
#include <sys/wait.h>
#include <unistd.h>
struct sigaction chldsa;
#endif

struct TAProcessInfo {
  char** argv;
};

std::map<int,TAProcessInfo> procInfoMap;

#ifdef _WIN32
int taExec(String path, const std::vector<String>& args, TAExecConsole* con) {
  return -1;
}

void taInstallExecHandler() {

}
#else

#define CRAP \
  for (int i=0; newArgv[i]; i++) { \
    delete[] newArgv[i]; \
  } \
  delete[] newArgv;

int taExec(String path, const std::vector<String>& args, TAExecConsole* con) {
  // prepare argv (one additional entry for argv[0] and another for NULL)
  char** newArgv=new char*[args.size()+2];

  // argv[0]
  newArgv[0]=new char[path.size()+1];
  memcpy(newArgv[0],path.c_str(),path.size()+1);

  // arguments
  for (size_t i=0; i<args.size(); i++) {
    newArgv[i+1]=new char[args[i].size()+1];
    memcpy(newArgv[i+1],args[i].c_str(),args[i].size()+1);
  }

  // end of argument list
  newArgv[args.size()+1]=NULL;

  TAProcessInfo newInfo;
  int inPipe[2];
  int outPipe[2];
  int errPipe[2];
  newInfo.argv=newArgv;

  // create pipe if we have a console
  if (con!=NULL) {
    if (pipe(inPipe)<0) {
      CRAP
      return -1;
    }
    if (pipe(outPipe)<0) {
      CRAP
      close(inPipe[0]);
      close(inPipe[1]);
      return -1;
    }
    if (pipe(errPipe)<0) {
      CRAP
      close(inPipe[0]);
      close(inPipe[1]);
      close(outPipe[0]);
      close(outPipe[1]);
      return -1;
    }

    // copy one side of the pipe
    con->in=fdopen(inPipe[1],"wb");
    con->out=fdopen(outPipe[0],"rb");
    con->err=fdopen(errPipe[0],"rb");
  }

  int pid=fork();
  if (pid==-1) {
    CRAP
    if (con!=NULL) {
      close(inPipe[0]);
      close(inPipe[1]);
      close(outPipe[0]);
      close(outPipe[1]);
      close(errPipe[0]);
      close(errPipe[1]);
    }
  } else if (pid==0) { // child
    // bind I/O if we should
    if (con!=NULL) {
      dup2(inPipe[0],STDIN_FILENO);
      dup2(outPipe[1],STDOUT_FILENO);
      dup2(errPipe[1],STDERR_FILENO);

      close(inPipe[0]);
      close(inPipe[1]);
      close(outPipe[0]);
      close(outPipe[1]);
      close(errPipe[0]);
      close(errPipe[1]);
    }
    execve(path.c_str(),newArgv,environ);
    // if we're here, execve has failed.
    exit(1);
  } else { // parent
    procInfoMap[pid]=newInfo;

    // throw away the other side of the pipes
    if (con!=NULL) {
      close(inPipe[0]);
      close(outPipe[1]);
      close(errPipe[1]);
    }
  }
  return pid;
}

int taWaitProcess(int pid) {
  int ret=0;
  if (waitpid(pid,&ret,0)<0) return -1;
  return ret;
}

static void handleChild(int) {
  // TODO: this...
}

void taInstallExecHandler() {
  sigemptyset(&chldsa.sa_mask);
  chldsa.sa_flags=0;
  chldsa.sa_handler=handleChild;
  sigaction(SIGCHLD,&chldsa,NULL);
  execHandlerReady=true;
}
#endif
