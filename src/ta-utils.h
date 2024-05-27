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

#ifndef _TA_UTILS_H
#define _TA_UTILS_H
#include <stdio.h>
#include <string.h>
#include "pch.h"

#ifdef _MSC_VER
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

#ifdef _WIN32
#define DIR_SEPARATOR '\\'
#define DIR_SEPARATOR_STR "\\"
#else
#define DIR_SEPARATOR '/'
#define DIR_SEPARATOR_STR "/"
#endif

typedef std::string String;

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define CLAMP(x,xMin,xMax) (MIN(MAX((x),(xMin)),(xMax)))

#ifdef HAVE_LOCALE
#include <libintl.h>
#define _(_str) gettext(_str)
#else
#define _(_str) _str
#endif

#define _GN(_strS,_strP,_cond) (((_cond)==1)?(_strS):(_strP))
#define _N(_str) _str

typedef std::wstring WString;

enum TAParamResult {
  TA_PARAM_ERROR=0,
  TA_PARAM_SUCCESS,
  TA_PARAM_QUIT
};

struct TAParam {
  String shortName;
  String name;
  String valName;
  String desc;
  bool value;
  TAParamResult (*func)(String);
  TAParam(const String& sn, const String& n, bool v, TAParamResult (*f)(String), const String& vn, const String& d):
    shortName(sn),
    name(n),
    valName(vn),
    desc(d),
    value(v),
    func(f) {}
};

void reportError(String what);

#endif
