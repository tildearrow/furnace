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

#include "timeutils.h"
#include <fmt/printf.h>
#include <stdexcept>

String TimeMicros::toString(signed char prec, TATimeFormats hms) {
  String ret;
  int actualSeconds=seconds;
  int actualMicros=micros;
  bool negative=(seconds<0);
  if (seconds<0 && micros!=0) {
    actualSeconds++;
    actualMicros=1000000-micros;
  }
  if (negative) actualSeconds=-actualSeconds;

  // handle auto formats
  switch (hms) {
    case TA_TIME_FORMAT_AUTO:
      hms=TA_TIME_FORMAT_SECONDS;
      if (actualSeconds>60) hms=TA_TIME_FORMAT_MS;
      if (actualSeconds>3600) hms=TA_TIME_FORMAT_HMS;
      if (actualSeconds>86400) hms=TA_TIME_FORMAT_DAYS_HMS;
      break;
    case TA_TIME_FORMAT_AUTO_ZERO:
      hms=TA_TIME_FORMAT_SECONDS;
      if (actualSeconds>60) hms=TA_TIME_FORMAT_MS_ZERO;
      if (actualSeconds>3600) hms=TA_TIME_FORMAT_HMS_ZERO;
      if (actualSeconds>86400) hms=TA_TIME_FORMAT_DAYS_HMS_ZERO;
      break;
    case TA_TIME_FORMAT_AUTO_MS:
      hms=TA_TIME_FORMAT_MS;
      if (actualSeconds>3600) hms=TA_TIME_FORMAT_HMS;
      if (actualSeconds>86400) hms=TA_TIME_FORMAT_DAYS_HMS;
      break;
    case TA_TIME_FORMAT_AUTO_MS_ZERO:
      hms=TA_TIME_FORMAT_MS_ZERO;
      if (actualSeconds>3600) hms=TA_TIME_FORMAT_HMS_ZERO;
      if (actualSeconds>86400) hms=TA_TIME_FORMAT_DAYS_HMS_ZERO;
      break;
    default:
      break;
  }

  switch (hms) {
    case TA_TIME_FORMAT_SECONDS: { // seconds
      if (negative) {
        ret=fmt::sprintf("-%d",actualSeconds);
      } else {
        ret=fmt::sprintf("%d",actualSeconds);
      }
      break;
    }
    case TA_TIME_FORMAT_MS: { // m:ss
      int m=actualSeconds/60;
      int s=actualSeconds%60;
      
      if (negative) {
        ret=fmt::sprintf("-%d:%02d",m,s);
      } else {
        ret=fmt::sprintf("%d:%02d",m,s);
      }
      break;
    }
    case TA_TIME_FORMAT_HMS: { // h:mm:ss
      int h=actualSeconds/3600;
      int m=(actualSeconds/60)%60;
      int s=actualSeconds%60;

      if (negative) {
        ret=fmt::sprintf("-%d:%02d:%02d",h,m,s);
      } else {
        ret=fmt::sprintf("%d:%02d:%02d",h,m,s);
      }
      break;
    }
    case TA_TIME_FORMAT_MS_ZERO: { // mm:ss
      int m=actualSeconds/60;
      int s=actualSeconds%60;
      
      if (negative) {
        ret=fmt::sprintf("-%02d:%02d",m,s);
      } else {
        ret=fmt::sprintf("%02d:%02d",m,s);
      }
      break;
    }
    case TA_TIME_FORMAT_HMS_ZERO: { // hh:mm:ss
      int h=actualSeconds/3600;
      int m=(actualSeconds/60)%60;
      int s=actualSeconds%60;

      if (negative) {
        ret=fmt::sprintf("-%02d:%02d:%02d",h,m,s);
      } else {
        ret=fmt::sprintf("%02d:%02d:%02d",h,m,s);
      }
      break;
    }
    case TA_TIME_FORMAT_DAYS_HMS: { // ?y?m?d h:mm:ss
      int days=actualSeconds/86400;
      int years=days/365;
      days%=365;
      int months=days/30;
      days%=30;
      int h=(actualSeconds/3600)%24;
      int m=(actualSeconds/60)%60;
      int s=actualSeconds%60;

      if (negative) {
        ret=fmt::sprintf("-%dy%dm%dd %d:%02d:%02d",years,months,days,h,m,s);
      } else {
        ret=fmt::sprintf("%dy%dm%dd %d:%02d:%02d",years,months,days,h,m,s);
      }
      break;
    }
    case TA_TIME_FORMAT_DAYS_HMS_ZERO: { // ?y?m?d hh:mm:ss
      int days=actualSeconds/86400;
      int years=days/365;
      days%=365;
      int months=days/30;
      days%=30;
      int h=(actualSeconds/3600)%24;
      int m=(actualSeconds/60)%60;
      int s=actualSeconds%60;

      if (negative) {
        ret=fmt::sprintf("-%dy%dm%dd %02d:%02d:%02d",years,months,days,h,m,s);
      } else {
        ret=fmt::sprintf("%dy%dm%dd %02d:%02d:%02d",years,months,days,h,m,s);
      }
      break;
    }
    default:
      return "ERROR!";
  }

  if (prec<0) {
    // automatic precision
    prec=6;
    int precMicros=actualMicros;
    while ((precMicros%10)==0 && prec>1) {
      prec--;
      precMicros/=10;
    }
  }

  if (prec>0) {
    if (prec>6) prec=6;
    switch (prec) {
      case 1:
        ret+=fmt::sprintf(".%01d",actualMicros/100000);
        break;
      case 2:
        ret+=fmt::sprintf(".%02d",actualMicros/10000);
        break;
      case 3:
        ret+=fmt::sprintf(".%03d",actualMicros/1000);
        break;
      case 4:
        ret+=fmt::sprintf(".%04d",actualMicros/100);
        break;
      case 5:
        ret+=fmt::sprintf(".%05d",actualMicros/10);
        break;
      case 6:
        ret+=fmt::sprintf(".%06d",actualMicros);
        break;
    }
  }

  return ret;
}

TimeMicros TimeMicros::fromString(const String& s) {
  bool seenYears=false;
  bool seenMonths=false;
  bool seenDays=false;
  bool seenDecimal=false;
  bool needSomething=true;
  bool canNegative=true;
  bool isNegative=false;
  unsigned int element[4];
  unsigned int elementDigits[4];
  int elementCount=0;
  int curElement=0;
  int curElementDigits=0;

  int years=0;
  int months=0;
  int days=0;

  element[0]=0;
  element[1]=0;
  element[2]=0;
  element[3]=0;
  elementDigits[0]=0;
  elementDigits[1]=0;
  elementDigits[2]=0;
  elementDigits[3]=0;

  for (char i: s) {
    switch (i) {
      // numbers
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
        if (curElementDigits<9) {
          curElement*=10;
          curElement+=(i-'0');
          curElementDigits++;
          needSomething=false;
        } else {
          throw std::invalid_argument("value out of range");
        }
        break;

      // negative indicator
      case '-':
        if (!canNegative) {
          throw std::invalid_argument("negative symbol shall be at the beginning");
        }
        isNegative=true;
        break;

      // element separator
      case ':':
        if (needSomething) {
          throw std::invalid_argument("missing value");
        }
        if (seenDecimal) {
          throw std::invalid_argument("after decimal");
        }
        if (elementCount>=3) {
          throw std::invalid_argument("too many elements");
        }
        element[elementCount]=curElement;
        elementDigits[elementCount++]=curElementDigits;
        curElement=0;
        curElementDigits=0;
        break;

      // decimal separator
      case '.': case ',':
        if (needSomething && elementCount!=0) {
          throw std::invalid_argument("wrong decimal separator");
        }
        if (elementCount>=3) {
          throw std::invalid_argument("too many elements");
        }
        element[elementCount]=curElement;
        elementDigits[elementCount++]=curElementDigits;
        curElement=0;
        curElementDigits=0;
        needSomething=true;
        seenDecimal=true;
        break;

      // year/month/day marker
      case 'Y': case 'y':
        if (seenYears) {
          throw std::invalid_argument("already have years");
        }
        if (elementCount!=0) {
          throw std::invalid_argument("years must be before time");
        }
        years=curElement;
        curElement=0;
        curElementDigits=0;
        needSomething=true;
        seenYears=true;
        break;
      case 'M': case 'm':
        if (seenMonths) {
          throw std::invalid_argument("already have months");
        }
        if (elementCount!=0) {
          throw std::invalid_argument("months must be before time");
        }
        months=curElement;
        curElement=0;
        curElementDigits=0;
        needSomething=true;
        seenMonths=true;
        break;
      case 'D': case 'd':
        if (seenDays) {
          throw std::invalid_argument("already have days");
        }
        if (elementCount!=0) {
          throw std::invalid_argument("days must be before time");
        }
        days=curElement;
        curElement=0;
        curElementDigits=0;
        needSomething=true;
        seenDays=true;
        break;

      // ignore spaces
      case ' ':
        break;
      
      // fail if any other character is seen
      default:
        throw std::invalid_argument("invalid character");
        break;
    }
    canNegative=false;
  }

  // fail if you didn't provide an element
  if (needSomething) {
    throw std::invalid_argument("missing value at the end");
  }

  element[elementCount]=curElement;
  elementDigits[elementCount]=curElementDigits;

  // safety checks (yeah I know these are a bit off but whatever)
  if (years>68 || months>828 || (years*365+months*30+days)>24855) {
    throw std::invalid_argument("years/months/days out of range");
  }
  
  // now combine elements
  TimeMicros ret;
  ret.seconds=86400*(years*365+months*30+days);
  if (seenDecimal || elementCount==3) {
    if (elementDigits[elementCount]<6) {
      for (int i=elementDigits[elementCount]; i<6; i++) {
        element[elementCount]*=10;
      }
    } else if (elementDigits[elementCount]>6) {
      for (int i=elementDigits[elementCount]; i>6; i--) {
        element[elementCount]/=10;
      }
    }
    ret.micros=element[elementCount];
    
    elementCount--;
  }
  switch (elementCount) {
    case 0: // seconds only
      ret.seconds+=element[0];
      break;
    case 1: // minutes and seconds
      if (element[0]>=35791394) {
        throw std::invalid_argument("minutes out of range");
      }
      if (element[1]>=60) {
        throw std::invalid_argument("seconds out of range");
      }
      ret.seconds+=(element[0]*60)+element[1];
      break;
    case 2: // hours, minutes and seconds
      if (seenYears || seenMonths || seenDays) {
        if (element[0]>=24) {
          throw std::invalid_argument("hours out of range");
        }
      } else {
        if (element[0]>=596523) {
          throw std::invalid_argument("hours out of range");
        }
      }
      if (element[1]>=60) {
        throw std::invalid_argument("minutes out of range");
      }
      if (element[2]>=60) {
        throw std::invalid_argument("seconds out of range");
      }
      ret.seconds+=(element[0]*3600)+(element[1]*60)+element[2];
      break;
    default:
      throw std::invalid_argument("shouldn't happen");
      break;
  }

  if (isNegative) {
    ret.seconds=-ret.seconds;
    if (ret.micros!=0) {
      ret.micros=1000000-ret.micros;
      ret.seconds--;
    }
  }

  return ret;
}
