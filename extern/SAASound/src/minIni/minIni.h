/*  minIni - Multi-Platform INI file parser, suitable for embedded systems
 *
 *  Copyright (c) CompuPhase, 2008-2017
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *  use this file except in compliance with the License. You may obtain a copy
 *  of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 *  License for the specific language governing permissions and limitations
 *  under the License.
 *
 *  Version: $Id: minIni.h 53 2015-01-18 13:35:11Z thiadmer.riemersma@gmail.com $
 */
#ifndef MININI_H
#define MININI_H

#include "minGlue.h"


#if (defined _UNICODE || defined __UNICODE__ || defined UNICODE) && !defined INI_ANSIONLY
#if !defined UNICODE   /* for Windows */
#define UNICODE
#endif
#if !defined _UNICODE  /* for C library */
#define _UNICODE
#endif
#endif

#if defined UNICODE
#include <tchar.h>
#define t_string std::wstring
#else
#define TCHAR char
#define t_string std::string
#define _tfopen fopen
#define _tremove remove
#define _trename rename
#endif

#if !defined __T
#define __T(s)    s
#endif

#if !defined _T
#define _T(s) __T(s)
#endif

#if defined INI_ANSIONLY
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#define mTCHAR     char
#define _mT(s)     s
#define mString    std::string
#define _mtcscat   strcat
#define _mtcschr   strchr
#define _mtcscmp   strcmp
#define _mtcscpy   strcpy
#define _mtcsicmp  stricmp
#define _mtcslen   strlen
#define _mtcsncmp  strncmp
#define _mtcsnicmp strnicmp
#define _mtcsrchr  strrchr
#define _mtcstol   strtol
#define _mtcstod   strtod
#define _mtotupper toupper
#define _mstprintf sprintf
#define _mtfgets   fgets
#define _mtfputs   fputs
#define _mtfopen   fopen
#define _mtremove  remove
#define _mtrename  rename
#else
#define mTCHAR     TCHAR
#define _mT(s)     __T(s)
#define mString    t_string
#define _mtcscat   _tcscat
#define _mtcschr   _tcschr
#define _mtcscmp   _tcscmp
#define _mtcscpy   _tcscpy
#define _mtcsicmp  _tcsicmp
#define _mtcslen   _tcslen
#define _mtcsncmp  _tcsncmp
#define _mtcsnicmp _tcsnicmp
#define _mtcsrchr  _tcsrchr
#define _mtcstol   _tcstol
#define _mtcstod   _tcstod
#define _mtotupper _totupper
#define _mstprintf _stprintf
#define _mtfgets   _tfgets
#define _mtfputs   _tfputs
#define _mtfopen   _tfopen
#define _mtremove  _tremove
#define _mtrename  _trename
#endif


#if !defined INI_BUFFERSIZE
  #define INI_BUFFERSIZE  512
#endif

#if defined __cplusplus
  extern "C" {
#endif

int   ini_getbool(const mTCHAR *Section, const mTCHAR *Key, int DefValue, const TCHAR *Filename);
long  ini_getl(const mTCHAR *Section, const mTCHAR *Key, long DefValue, const TCHAR *Filename);
int   ini_gets(const mTCHAR *Section, const mTCHAR *Key, const mTCHAR *DefValue, mTCHAR *Buffer, int BufferSize, const TCHAR *Filename);
int   ini_getsection(int idx, mTCHAR *Buffer, int BufferSize, const TCHAR *Filename);
int   ini_getkey(const mTCHAR *Section, int idx, mTCHAR *Buffer, int BufferSize, const TCHAR *Filename);

#if defined INI_REAL
INI_REAL ini_getf(const mTCHAR *Section, const mTCHAR *Key, INI_REAL DefValue, const TCHAR *Filename);
#endif

#if !defined INI_READONLY
int   ini_putl(const mTCHAR *Section, const mTCHAR *Key, long Value, const TCHAR *Filename);
int   ini_puts(const mTCHAR *Section, const mTCHAR *Key, const mTCHAR *Value, const TCHAR *Filename);
#if defined INI_REAL
int   ini_putf(const mTCHAR *Section, const mTCHAR *Key, INI_REAL Value, const TCHAR *Filename);
#endif
#endif /* INI_READONLY */

#if !defined INI_NOBROWSE
typedef int (*INI_CALLBACK)(const mTCHAR *Section, const mTCHAR *Key, const mTCHAR *Value, void *UserData);
int  ini_browse(INI_CALLBACK Callback, void *UserData, const TCHAR *Filename);
#endif /* INI_NOBROWSE */

#if defined __cplusplus
  }
#endif


#if defined __cplusplus

#if defined __WXWINDOWS__
	#include "wxMinIni.h"
#else
  #include <string>

  /* The C++ class in minIni.h was contributed by Steven Van Ingelgem. */
  class minIni
  {
  public:
    minIni(const t_string& filename) : iniFilename(filename)
      { }

    bool getbool(const mString& Section, const mString& Key, bool DefValue=false) const
      { return ini_getbool(Section.c_str(), Key.c_str(), int(DefValue), iniFilename.c_str()) != 0; }

    long getl(const mString& Section, const mString& Key, long DefValue=0) const
      { return ini_getl(Section.c_str(), Key.c_str(), DefValue, iniFilename.c_str()); }

    int geti(const mString& Section, const mString& Key, int DefValue=0) const
      { return static_cast<int>(this->getl(Section, Key, long(DefValue))); }

    mString gets(const mString& Section, const mString& Key, const mString& DefValue=_mT("")) const
      {
        mTCHAR buffer[INI_BUFFERSIZE];
        ini_gets(Section.c_str(), Key.c_str(), DefValue.c_str(), buffer, INI_BUFFERSIZE, iniFilename.c_str());
        return buffer;
      }

    mString getsection(int idx) const
      {
        mTCHAR buffer[INI_BUFFERSIZE];
        ini_getsection(idx, buffer, INI_BUFFERSIZE, iniFilename.c_str());
        return buffer;
      }

    mString getkey(const mString& Section, int idx) const
      {
        mTCHAR buffer[INI_BUFFERSIZE];
        ini_getkey(Section.c_str(), idx, buffer, INI_BUFFERSIZE, iniFilename.c_str());
        return buffer;
      }

#if defined INI_REAL
    INI_REAL getf(const mString& Section, const mString& Key, INI_REAL DefValue=0) const
      { return ini_getf(Section.c_str(), Key.c_str(), DefValue, iniFilename.c_str()); }
#endif

#if ! defined INI_READONLY
    bool put(const mString& Section, const mString& Key, long Value)
      { return ini_putl(Section.c_str(), Key.c_str(), Value, iniFilename.c_str()) != 0; }

    bool put(const mString& Section, const mString& Key, int Value)
      { return ini_putl(Section.c_str(), Key.c_str(), (long)Value, iniFilename.c_str()) != 0; }

    bool put(const mString& Section, const mString& Key, bool Value)
      { return ini_putl(Section.c_str(), Key.c_str(), (long)Value, iniFilename.c_str()) != 0; }

    bool put(const mString& Section, const mString& Key, const mString& Value)
      { return ini_puts(Section.c_str(), Key.c_str(), Value.c_str(), iniFilename.c_str()) != 0; }

    bool put(const mString& Section, const mString& Key, const mTCHAR* Value)
      { return ini_puts(Section.c_str(), Key.c_str(), Value, iniFilename.c_str()) != 0; }

#if defined INI_REAL
    bool put(const mString& Section, const mString& Key, INI_REAL Value)
      { return ini_putf(Section.c_str(), Key.c_str(), Value, iniFilename.c_str()) != 0; }
#endif

    bool del(const mString& Section, const mString& Key)
      { return ini_puts(Section.c_str(), Key.c_str(), 0, iniFilename.c_str()) != 0; }

    bool del(const mString& Section)
      { return ini_puts(Section.c_str(), 0, 0, iniFilename.c_str()) != 0; }
#endif

#if !defined INI_NOBROWSE
    bool browse(INI_CALLBACK Callback, void *UserData) const
      { return ini_browse(Callback, UserData, iniFilename.c_str()) != 0; }
#endif

  private:
      t_string iniFilename;
  };

#endif /* __WXWINDOWS__ */
#endif /* __cplusplus */

#endif /* MININI_H */
