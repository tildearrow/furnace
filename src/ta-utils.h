#ifndef _TA_UTILS_H
#define _TA_UTILS_H
#include <stdio.h>
#include <string.h>
#include <string>

#ifdef _MSC_VER
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

typedef std::string String;

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

typedef std::wstring WString;

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
