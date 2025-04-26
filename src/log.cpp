/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2025 tildearrow and contributors
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

#include "ta-log.h"
#include "fileutils.h"
#include <thread>
#include <condition_variable>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef IS_MOBILE
int logLevel=LOGLEVEL_TRACE;
#else
int logLevel=LOGLEVEL_TRACE; // until done
#endif

FILE* logOut;
FILE* logFile;
char* logFileBuf;
char* logFileWriteBuf;
unsigned int logFilePosI;
unsigned int logFilePosO;
std::thread* logFileThread;
std::mutex logFileLock;
std::mutex logFileLockI;
std::condition_variable logFileNotify;
std::atomic<bool> logFileAvail(false);

std::atomic<unsigned short> logPosition;

LogEntry logEntries[TA_LOG_SIZE];

static constexpr unsigned int TA_LOG_MASK=TA_LOG_SIZE-1;
static constexpr unsigned int TA_LOGFILE_BUF_MASK=TA_LOGFILE_BUF_SIZE-1;

const char* logTypes[5]={
  "ERROR",
  "warning",
  "info",
  "debug",
  "trace"
};

void appendLogBuf(const LogEntry& entry) {
  logFileLockI.lock();

  std::string toWrite=fmt::sprintf(
    "%02d:%02d:%02d [%s] %s\n",
    entry.time.tm_hour,
    entry.time.tm_min,
    entry.time.tm_sec,
    logTypes[entry.loglevel],
    entry.text
  );

  const char* msg=toWrite.c_str();
  size_t len=toWrite.size();

  int remaining=(logFilePosO-logFilePosI-1)&TA_LOGFILE_BUF_SIZE;

  if (len>=(unsigned int)remaining) {
    printf("line too long to fit in log buffer!\n");
    logFileLockI.unlock();
    return;
  }

  if ((logFilePosI+len)>=TA_LOGFILE_BUF_SIZE) {
    size_t firstWrite=TA_LOGFILE_BUF_SIZE-logFilePosI;
    memcpy(logFileBuf+logFilePosI,msg,firstWrite);
    memcpy(logFileBuf,msg+firstWrite,len-firstWrite);
  } else {
    memcpy(logFileBuf+logFilePosI,msg,len);
  }

  logFilePosI=(logFilePosI+len)&TA_LOGFILE_BUF_MASK;
  logFileLockI.unlock();
}

int writeLog(int level, const char* msg, fmt::printf_args args) {
  time_t thisMakesNoSense=time(NULL);
  int pos=(logPosition.fetch_add(1))&TA_LOG_MASK;

#if FMT_VERSION >= 100100
#ifdef _MSVC_LANG
#if _MSVC_LANG >= 201703L
  logEntries[pos].text.assign(fmt::vsprintf(std::basic_string_view(msg),args));
#else
  logEntries[pos].text.assign(fmt::vsprintf(fmt::basic_string_view<char>(msg),args));
#endif
#else
#if __cplusplus >= 201703L
  logEntries[pos].text.assign(fmt::vsprintf(std::basic_string_view(msg),args));
#else
  logEntries[pos].text.assign(fmt::vsprintf(fmt::basic_string_view<char>(msg),args));
#endif
#endif
#else
  logEntries[pos].text.assign(fmt::vsprintf(msg,args));
#endif
  // why do I have to pass a pointer
  // can't I just pass the time_t directly?!
#ifdef _WIN32
  struct tm* tempTM=localtime(&thisMakesNoSense);
  if (tempTM==NULL) {
    memset(&logEntries[pos].time,0,sizeof(struct tm));
  } else {
    memcpy(&logEntries[pos].time,tempTM,sizeof(struct tm));
  }
#else
  if (localtime_r(&thisMakesNoSense,&logEntries[pos].time)==NULL) {
    memset(&logEntries[pos].time,0,sizeof(struct tm));
  }
#endif
  logEntries[pos].loglevel=level;
  logEntries[pos].ready=true;

  // write to log file
  if (logFileAvail) {
    appendLogBuf(logEntries[pos]);
    logFileNotify.notify_one();
  }

  if (logLevel<level) return 0;
  switch (level) {
    case LOGLEVEL_ERROR:
      return fmt::fprintf(logOut,"\x1b[1;31m[ERROR]\x1b[m %s\n",logEntries[pos].text);
    case LOGLEVEL_WARN:
      return fmt::fprintf(logOut,"\x1b[1;33m[warning]\x1b[m %s\n",logEntries[pos].text);
    case LOGLEVEL_INFO:
      return fmt::fprintf(logOut,"\x1b[1;32m[info]\x1b[m %s\n",logEntries[pos].text);
    case LOGLEVEL_DEBUG:
      return fmt::fprintf(logOut,"\x1b[1;34m[debug]\x1b[m %s\n",logEntries[pos].text);
    case LOGLEVEL_TRACE:
      return fmt::fprintf(logOut,"\x1b[1;37m[trace]\x1b[m %s\n",logEntries[pos].text);
  }
  return -1;
}

void initLog(FILE* where) {
  logOut=where;

  // initialize coloring on Windows
#ifdef _WIN32
  HANDLE winout=GetStdHandle(STD_OUTPUT_HANDLE);
  int termprop=0;
  GetConsoleMode(winout,(LPDWORD)&termprop);
  termprop|=ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  SetConsoleMode(winout,termprop);
#endif

  // initialize log buffer
  logPosition=0;
  for (int i=0; i<TA_LOG_SIZE; i++) {
    logEntries[i].text.reserve(128);
  }

  // initialize log to file thread
  logFileAvail=false;
}

void changeLogOutput(FILE* where) {
  logOut=where;
}

void _logFileThread() {
  std::unique_lock<std::mutex> lock(logFileLock);
  while (true) {
    unsigned int logFilePosICopy=logFilePosI;
    if (logFilePosICopy!=logFilePosO) {
      // write
      if (logFilePosO>logFilePosICopy) {
        fwrite(logFileBuf+logFilePosO,1,TA_LOGFILE_BUF_SIZE-logFilePosO,logFile);
        logFilePosO=0;
      } else {
        fwrite(logFileBuf+logFilePosO,1,logFilePosICopy-logFilePosO,logFile);
        logFilePosO=logFilePosICopy&TA_LOGFILE_BUF_MASK;
      }
    } else {
      // wait
      fflush(logFile);
      if (!logFileAvail) break;
      logFileNotify.wait(lock);
    }
  }
}

bool startLogFile(const char* path) {
  if (logFileAvail) return true;

  // rotate log file if possible
  char oldPath[4096];
  char newPath[4096];

  if (fileExists(path)==1) {
    for (int i=4; i>=0; i--) {
      if (i>0) {
        snprintf(oldPath,4095,"%s.%d",path,i);
      } else {
        strncpy(oldPath,path,4095);
      }
      snprintf(newPath,4095,"%s.%d",path,i+1);

      if (i>=4) {
        deleteFile(oldPath);
      } else {
        moveFiles(oldPath,newPath);
      }
    }
  }
  
  // open log file
  if ((logFile=ps_fopen(path,"w+"))==NULL) {
    logFileAvail=false;
    logW("could not open log file! (%s)",strerror(errno));
    return false;
  }

  logFileBuf=new char[TA_LOGFILE_BUF_SIZE];
  logFileWriteBuf=new char[TA_LOGFILE_BUF_SIZE];
  logFilePosI=0;
  logFilePosO=0;
  logFileAvail=true;

  logFileThread=new std::thread(_logFileThread);
  return true;
}

bool finishLogFile() {
  if (!logFileAvail) return false;

  logFileAvail=false;

  // flush
  logFileLockI.lock();
  logFileNotify.notify_one();
  logFileThread->join();
  logFileLockI.unlock();

  fclose(logFile);
  return true;
}
