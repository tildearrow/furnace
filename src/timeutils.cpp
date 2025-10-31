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

#include "timeutils.h"
#include <fmt/printf.h>

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

TimeMicros TimeMicros::fromString(String s) {
  return TimeMicros(0,0);
}
