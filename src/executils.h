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

#ifndef _EXECUTILS_H
#define _EXECUTILS_H

#include "ta-utils.h"

struct TAExecConsole {
  FILE* in;
  FILE* out;
  FILE* err;
};

/**
 * execute a process.
 * the environment and current working directory are preserved.
 * @param path path to an executable.
 * @param args a list of arguments (argv[0] will be set to the path).
 * @param con a TAExecConsole. if this is provided, three files will be created for standard I/O.
 * @return the new process' ID on success, or -1 on failure.
 */
int taExec(String path, const std::vector<String>& args, TAExecConsole* con=NULL);

/**
 * wait for a child process to finish.
 * @param pid the process ID.
 * @return the exit code (could be -1 if this function fails).
 */
int taWaitProcess(int pid);

/**
 * this must be called before using taExec.
 * it sets up handlers for child processes.
 */
void taInstallExecHandler();

#endif
