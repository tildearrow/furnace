/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2022 tildearrow and contributors
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

int logLevel=LOGLEVEL_INFO;

std::atomic<unsigned short> logPosition;

LogEntry logEntries[TA_LOG_SIZE];

static constexpr unsigned int TA_LOG_MASK=TA_LOG_SIZE-1;

int logV(const char* format, ...) {
  va_list va;
  int ret;
  if (logLevel<LOGLEVEL_TRACE) return 0;
#ifdef _WIN32
  printf("[trace] ");
#else
  printf("\x1b[1;37m[trace]\x1b[m ");
#endif
  va_start(va,format);
  ret=vprintf(format,va);
  va_end(va);
  fflush(stdout);
  return ret;
}

int logD(const char* format, ...) {
  va_list va;
  int ret;
  if (logLevel<LOGLEVEL_DEBUG) return 0;
#ifdef _WIN32
  printf("[debug] ");
#else
  printf("\x1b[1;34m[debug]\x1b[m ");
#endif
  va_start(va,format);
  ret=vprintf(format,va);
  va_end(va);
  fflush(stdout);
  return ret;
}

int logW(const char* format, ...) {
  va_list va;
  int ret;
  if (logLevel<LOGLEVEL_WARN) return 0;
#ifdef _WIN32
  printf("[warning] ");
#else
  printf("\x1b[1;33m[warning]\x1b[m ");
#endif
  va_start(va,format);
  ret=vprintf(format,va);
  va_end(va);
  return ret;
}

int logE(const char* format, ...) {
  va_list va;
  int ret;
  if (logLevel<LOGLEVEL_ERROR) return 0;
#ifdef _WIN32
  printf("[ERROR] ");
#else
  printf("\x1b[1;31m[ERROR]\x1b[m ");
#endif
  va_start(va,format);
  ret=vprintf(format,va);
  va_end(va);
  return ret;
}

int writeLog(int level, const char* msg, fmt::printf_args& args) {
  time_t thisMakesNoSense=time(NULL);
  int pos=logPosition;
  logPosition=(logPosition+1)&TA_LOG_MASK;

  logEntries[pos].text=fmt::vsprintf(msg,args);
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

  if (logLevel<level) return 0;
  switch (level) {
    case LOGLEVEL_ERROR:
      return fmt::printf("\x1b[1;31m[ERROR]\x1b[m %s\n",logEntries[pos].text);
    case LOGLEVEL_WARN:
      return fmt::printf("\x1b[1;33m[warning]\x1b[m %s\n",logEntries[pos].text);
    case LOGLEVEL_INFO:
      return fmt::printf("\x1b[1;32m[info]\x1b[m %s\n",logEntries[pos].text);
    case LOGLEVEL_DEBUG:
      return fmt::printf("\x1b[1;34m[debug]\x1b[m %s\n",logEntries[pos].text);
    case LOGLEVEL_TRACE:
      return fmt::printf("\x1b[1;37m[trace]\x1b[m %s\n",logEntries[pos].text);
  }
  return -1;
}

void initLog() {
  logPosition=0;
  for (int i=0; i<TA_LOG_SIZE; i++) {
    logEntries[i].text.reserve(128);
  }
}
