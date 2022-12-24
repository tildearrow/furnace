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

#ifndef _TA_LOG_H
#define _TA_LOG_H
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <atomic>
#include <string>
#include <fmt/printf.h>

#define LOGLEVEL_ERROR 0
#define LOGLEVEL_WARN 1
#define LOGLEVEL_INFO 2
#define LOGLEVEL_DEBUG 3
#define LOGLEVEL_TRACE 4

// this has to be a power of 2
#define TA_LOG_SIZE 2048

// this as well
#define TA_LOGFILE_BUF_SIZE 65536

extern int logLevel;

extern std::atomic<unsigned short> logPosition;

struct LogEntry {
  int loglevel;
  struct tm time;
  std::string text;
  bool ready;
  LogEntry():
    loglevel(0),
    ready(false) {
    memset(&time,0,sizeof(struct tm));
  }
};

int writeLog(int level, const char* msg, fmt::printf_args args);

extern LogEntry logEntries[TA_LOG_SIZE];

template<typename... T> int logV(const char* msg, const T&... args) {
  return writeLog(LOGLEVEL_TRACE,msg,fmt::make_printf_args(args...));
}

template<typename... T> int logD(const char* msg, const T&... args) {
  return writeLog(LOGLEVEL_DEBUG,msg,fmt::make_printf_args(args...));
}

template<typename... T> int logI(const char* msg, const T&... args) {
  return writeLog(LOGLEVEL_INFO,msg,fmt::make_printf_args(args...));
}

template<typename... T> int logW(const char* msg, const T&... args) {
  return writeLog(LOGLEVEL_WARN,msg,fmt::make_printf_args(args...));
}

template<typename... T> int logE(const char* msg, const T&... args) {
  return writeLog(LOGLEVEL_ERROR,msg,fmt::make_printf_args(args...));
}

void initLog();
bool startLogFile(const char* path);
bool finishLogFile();
#endif
