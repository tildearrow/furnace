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

#include "gui.h"

#include "actionUtil.h"

void FurnaceGUI::startSelection(int xCoarse, int xFine, int y) {
  if (xCoarse!=selStart.xCoarse || xFine!=selStart.xFine || y!=selStart.y) {
    curNibble=false;
  }
  cursor.xCoarse=xCoarse;
  cursor.xFine=xFine;
  cursor.y=y;
  selStart.xCoarse=xCoarse;
  selStart.xFine=xFine;
  selStart.y=y;
  selEnd.xCoarse=xCoarse;
  selEnd.xFine=xFine;
  selEnd.y=y;
  selecting=true;
  e->setMidiBaseChan(cursor.xCoarse);
}

void FurnaceGUI::updateSelection(int xCoarse, int xFine, int y) {
  if (!selecting) return;
  selEnd.xCoarse=xCoarse;
  selEnd.xFine=xFine;
  selEnd.y=y;
}

void FurnaceGUI::finishSelection() {
  // swap points if needed
  if (selEnd.y<selStart.y) {
    selEnd.y^=selStart.y;
    selStart.y^=selEnd.y;
    selEnd.y^=selStart.y;
  }
  if (selEnd.xCoarse<selStart.xCoarse) {
    selEnd.xCoarse^=selStart.xCoarse;
    selStart.xCoarse^=selEnd.xCoarse;
    selEnd.xCoarse^=selStart.xCoarse;

    selEnd.xFine^=selStart.xFine;
    selStart.xFine^=selEnd.xFine;
    selEnd.xFine^=selStart.xFine;
  } else if (selEnd.xCoarse==selStart.xCoarse && selEnd.xFine<selStart.xFine) {
    selEnd.xFine^=selStart.xFine;
    selStart.xFine^=selEnd.xFine;
    selEnd.xFine^=selStart.xFine;
  }
  selecting=false;

  // boundary check
  int chanCount=e->getTotalChannelCount();

  if (selStart.xCoarse<0) selStart.xCoarse=0;
  if (selStart.xCoarse>=chanCount) selStart.xCoarse=chanCount-1;
  if (selStart.y<0) selStart.y=0;
  if (selStart.y>=e->song.patLen) selStart.y=e->song.patLen-1;
  if (selEnd.xCoarse<0) selEnd.xCoarse=0;
  if (selEnd.xCoarse>=chanCount) selEnd.xCoarse=chanCount-1;
  if (selEnd.y<0) selEnd.y=0;
  if (selEnd.y>=e->song.patLen) selEnd.y=e->song.patLen-1;
  if (cursor.xCoarse<0) cursor.xCoarse=0;
  if (cursor.xCoarse>=chanCount) cursor.xCoarse=chanCount-1;
  if (cursor.y<0) cursor.y=0;
  if (cursor.y>=e->song.patLen) cursor.y=e->song.patLen-1;

  if (e->song.chanCollapse[selEnd.xCoarse]) {
    selStart.xFine=0;
  }
  if (e->song.chanCollapse[selEnd.xCoarse]) {
    selEnd.xFine=2+e->song.pat[cursor.xCoarse].effectRows*2;
  }

  e->setMidiBaseChan(cursor.xCoarse);
}

void FurnaceGUI::moveCursor(int x, int y, bool select) {
  if (!select) {
    finishSelection();
  }

  DETERMINE_FIRST_LAST;
  
  curNibble=false;
  if (x!=0) {
    demandScrollX=true;
    if (x>0) {
      for (int i=0; i<x; i++) {
        if (++cursor.xFine>=(e->song.chanCollapse[cursor.xCoarse]?1:(3+e->song.pat[cursor.xCoarse].effectRows*2))) {
          cursor.xFine=0;
          if (++cursor.xCoarse>=lastChannel) {
            if (settings.wrapHorizontal!=0 && !select) {
              cursor.xCoarse=firstChannel;
              if (settings.wrapHorizontal==2) y++;
            } else {
              cursor.xCoarse=lastChannel-1;
              cursor.xFine=e->song.chanCollapse[cursor.xCoarse]?0:(2+e->song.pat[cursor.xCoarse].effectRows*2);
            }
          } else {
            while (!e->song.chanShow[cursor.xCoarse]) {
              cursor.xCoarse++;
              if (cursor.xCoarse>=e->getTotalChannelCount()) break;
            }
          }
        }
      }
    } else {
      for (int i=0; i<-x; i++) {
        if (--cursor.xFine<0) {
          if (--cursor.xCoarse<firstChannel) {
            if (settings.wrapHorizontal!=0 && !select) {
              cursor.xCoarse=lastChannel-1;
              cursor.xFine=2+e->song.pat[cursor.xCoarse].effectRows*2;
              if (settings.wrapHorizontal==2) y--;
            } else {
              cursor.xCoarse=firstChannel;
              cursor.xFine=0;
            }
          } else {
            while (!e->song.chanShow[cursor.xCoarse]) {
              cursor.xCoarse--;
              if (cursor.xCoarse<0) break;
            }
            if (e->song.chanCollapse[cursor.xCoarse]) {
              cursor.xFine=0;
            } else {
              cursor.xFine=2+e->song.pat[cursor.xCoarse].effectRows*2;
            }
          }
        }
      }
    }
  }
  if (y!=0) {
    if (y>0) {
      for (int i=0; i<y; i++) {
        cursor.y++;
        if (cursor.y>=e->song.patLen) {
          if (settings.wrapVertical!=0 && !select) {
            cursor.y=0;
            if (settings.wrapVertical==2) {
              if ((!e->isPlaying() || !followPattern) && curOrder<(e->song.ordersLen-1)) {
                setOrder(curOrder+1);
              } else {
                cursor.y=e->song.patLen-1;
              }
            }
          } else {
            cursor.y=e->song.patLen-1;
          }
        }
      }
    } else {
      for (int i=0; i<-y; i++) {
        cursor.y--;
        if (cursor.y<0) {
          if (settings.wrapVertical!=0 && !select) {
            cursor.y=e->song.patLen-1;
            if (settings.wrapVertical==2) {
              if ((!e->isPlaying() || !followPattern) && curOrder>0) {
                setOrder(curOrder-1);
              } else {
                cursor.y=0;
              }
            }
          } else {
            cursor.y=0;
          }
        }
      }
    }
  }
  if (!select) {
    selStart=cursor;
  }
  selEnd=cursor;
  updateScroll(cursor.y);
  e->setMidiBaseChan(cursor.xCoarse);
}

void FurnaceGUI::moveCursorPrevChannel(bool overflow) {
  finishSelection();
  curNibble=false;

  DETERMINE_FIRST_LAST;

  do {
    cursor.xCoarse--;
    if (cursor.xCoarse<0) break;
  } while (!e->song.chanShow[cursor.xCoarse]);
  if (cursor.xCoarse<firstChannel) {
    if (overflow) {
      cursor.xCoarse=lastChannel-1;
    } else {
      cursor.xCoarse=firstChannel;
    }
  }
  e->setMidiBaseChan(cursor.xCoarse);

  selStart=cursor;
  selEnd=cursor;
  demandScrollX=true;
}

void FurnaceGUI::moveCursorNextChannel(bool overflow) {
  finishSelection();
  curNibble=false;

  DETERMINE_FIRST_LAST;

  do {
    cursor.xCoarse++;
    if (cursor.xCoarse>=e->getTotalChannelCount()) break;
  } while (!e->song.chanShow[cursor.xCoarse]);
  if (cursor.xCoarse>=lastChannel) {
    if (overflow) {
      cursor.xCoarse=firstChannel;
    } else {
      cursor.xCoarse=lastChannel-1;
    }
  }
  e->setMidiBaseChan(cursor.xCoarse);

  selStart=cursor;
  selEnd=cursor;
  demandScrollX=true;
}

void FurnaceGUI::moveCursorTop(bool select) {
  finishSelection();
  curNibble=false;
  if (cursor.y==0) {
    DETERMINE_FIRST;
    cursor.xCoarse=firstChannel;
    cursor.xFine=0;
    demandScrollX=true;
  } else {
    cursor.y=0;
  }
  selStart=cursor;
  if (!select) {
    selEnd=cursor;
  }
  e->setMidiBaseChan(cursor.xCoarse);
  updateScroll(cursor.y);
}

void FurnaceGUI::moveCursorBottom(bool select) {
  finishSelection();
  curNibble=false;
  if (cursor.y==e->song.patLen-1) {
    DETERMINE_LAST;
    cursor.xCoarse=lastChannel-1;
    if (cursor.xCoarse<0) cursor.xCoarse=0;
    cursor.xFine=2+e->song.pat[cursor.xCoarse].effectRows*2;
    demandScrollX=true;
  } else {
    cursor.y=e->song.patLen-1;
  }
  if (!select) {
    selStart=cursor;
  }
  selEnd=cursor;
  e->setMidiBaseChan(cursor.xCoarse);
  updateScroll(cursor.y);
}

void FurnaceGUI::editAdvance() {
  finishSelection();
  cursor.y+=editStep;
  if (cursor.y>=e->song.patLen) cursor.y=e->song.patLen-1;
  selStart=cursor;
  selEnd=cursor;
  updateScroll(cursor.y);
}