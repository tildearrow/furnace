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

#ifndef _FILEUTILS_H
#define _FILEUTILS_H
#include <stdio.h>

FILE* ps_fopen(const char* path, const char* mode);
bool moveFiles(const char* src, const char* dest);
bool copyFiles(const char* src, const char* dest);
bool deleteFile(const char* path);
// returns 1 if file exists, 0 if it doesn't and -1 on error.
int fileExists(const char* path);
bool dirExists(const char* what);
bool makeDir(const char* path);
int touchFile(const char* path);

// from this point onward, new file operations system.
// the idea is not to depend on POSIX files as they may not be available under
// certain circumstances, such as a sandbox or remote storage.

// this is currently an errno.
typedef int TAFileError;

/**
 * TAFile is an interface that abstracts file APIs.
 */
class TAFile {
  public:
    /**
     * read from file into memory.
     * @param ptr pointer to memory location.
     * @param count number of bytes to read.
     * @return number of bytes actually read. assume end of file if less than count.
     * @throws TAFileError if an error occurs.
     */
    virtual size_t read(void* ptr, size_t count);

    /**
     * write a block of memory to a file.
     * @param ptr pointer to memory location.
     * @param count number of bytes to write.
     * @return number of bytes actually written. assume end of file if less than count.
     * @throws TAFileError if an error occurs.
     */
    virtual size_t write(void* ptr, size_t count);

    /**
     * move the current file position.
     * @param off offset relative to whence.
     * @param whence any of the following:
     *   - SEEK_SET: from beginning of file
     *   - SEEK_CUR: from current position
     *   - SEEK_END: from end of file
     * @throws TAFileError if an error occurs.
     */
    virtual void seek(long off, int whence);

    /**
     * get the current file position.
     * @return the current position.
     * @throws TAFileError if an error occurs.
     */
    virtual size_t tell();

    /**
     * get whether we are at the end of this file.
     * @return end of file status.
     */
    virtual bool eof();

    /**
     * get the native handle for this file.
     * @return a native handle.
     */
    virtual void* getNativeHandle();

    /**
     * deconstructor - closes the file.
     */
    virtual ~TAFile();
};

enum TAFileModes {
  TA_FILE_READ=1,
  TA_FILE_WRITE=2,
  TA_FILE_BINARY=4,
  TA_FILE_PRESERVE=8,
  TA_FILE_APPEND=16
};

/**
 * open a file by URI.
 * @param path guess what this is.
 * @mode a combination of any TAFileModes.
 * @return a TAFile.
 * @throws TAFileError if an error occurs.
 */
TAFile* taOpenURI(const char* path, TAFileModes mode);

/**
 * open a file in the file system.
 * @param path guess what this is.
 * @mode a combination of any TAFileModes.
 * @return a TAFile.
 * @throws TAFileError if an error occurs.
 */
TAFile* taOpenFile(const char* path, TAFileModes mode);

#endif
