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

#include <Cocoa/Cocoa.h>
#include "macstuff.h"

double getMacDPIScale(void* sysWin, unsigned char isUIKit) {
  NSScreen* screen=nil;
  if (sysWin!=NULL) {
    if (isUIKit) {
      return 1.0;
      /*
      UIWindow* win=(UIWindow*)sysWin;
      UIWindowScene* winScene=[win windowScene];
      if (winScene!=nil) {
        UIScreen* winScreen=[winScene screen];
        CGFloat ret=[winScreen scale];
        return (double)ret;
      }*/
    } else {
      NSWindow* win=(NSWindow*)sysWin;
      screen=[win screen];
    }
  }
  if (screen==nil) {
    screen=[NSScreen mainScreen];
  }
  if (screen==nil) {
    return 1.0;
  }
  CGFloat val=[screen backingScaleFactor];
  return (double)val;
}
