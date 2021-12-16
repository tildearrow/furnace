#ifndef _TA_UTILS_H
#define _TA_UTILS_H
#include <stdio.h>
#include <string.h>
#include <string>

typedef std::string String;

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#ifdef _WIN32
typedef std::wstring WString;
#endif

struct TAParam {
  String shortName;
  String name;
  String valName;
  String desc;
  bool value;
  bool (*func)(String);
  TAParam(String sn, String n, bool v, bool (*f)(String), String vn, String d):
    shortName(sn),
    name(n),
    valName(vn),
    desc(d),
    value(v),
    func(f) {}
};

#endif
