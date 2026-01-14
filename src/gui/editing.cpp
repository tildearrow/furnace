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

#include "gui.h"
#include "../ta-log.h"
#include "guiConst.h"
#include <fmt/printf.h>

#include "actionUtil.h"

static const char* modPlugFormatHeaders[]={
  "ModPlug Tracker MOD",
  "ModPlug Tracker S3M",
  "ModPlug Tracker  XM",
  "ModPlug Tracker XM",
  "ModPlug Tracker  IT",
  "ModPlug Tracker IT",
  "ModPlug Tracker MPT",
  NULL,
};

const char* FurnaceGUI::noteNameNormal(short note) {
  if (note==DIV_NOTE_OFF) { // note cut
    return "OFF";
  } else if (note==DIV_NOTE_REL) { // note off and envelope release
    return "===";
  } else if (note==DIV_MACRO_REL) { // envelope release only
    return "REL";
  } else if (note==-1) {
    return "...";
  } else if (note==DIV_NOTE_NULL_PAT) {
    return "BUG";
  }
  if (note<0 || note>=180) {
    return "???";
  }
  return noteNames[note];
}

void FurnaceGUI::prepareUndo(ActionType action, UndoRegion region) {
  undoCursor=cursor;
  undoSelStart=selStart;
  undoSelEnd=selEnd;
  undoOrder=curOrder;

  if (region.begin.ord==-1) {
    region.begin.ord=selStart.order;
    region.end.ord=selEnd.order;
    region.begin.x=0;
    region.end.x=e->getTotalChannelCount()-1;
    region.begin.y=0;
    region.end.y=e->curSubSong->patLen-1;
  } else {
    if (region.begin.ord<0) region.begin.ord=0;
    if (region.begin.ord>e->curSubSong->ordersLen) region.begin.ord=e->curSubSong->ordersLen;
    if (region.end.ord<0) region.end.ord=0;
    if (region.end.ord>e->curSubSong->ordersLen) region.end.ord=e->curSubSong->ordersLen;
    if (region.begin.x<0) region.begin.x=0;
    if (region.begin.x>=e->getTotalChannelCount()) region.begin.x=e->getTotalChannelCount()-1;
    if (region.end.x<0) region.end.x=0;
    if (region.end.x>=e->getTotalChannelCount()) region.end.x=e->getTotalChannelCount()-1;
    if (region.begin.y<0) region.begin.y=0;
    if (region.begin.y>=e->curSubSong->patLen) region.begin.y=e->curSubSong->patLen-1;
    if (region.end.y<0) region.end.y=0;
    if (region.end.y>=e->curSubSong->patLen) region.end.y=e->curSubSong->patLen-1;
  }

  switch (action) {
    case GUI_UNDO_CHANGE_ORDER:
      memcpy(&oldOrders,e->curOrders,sizeof(DivOrders));
      oldOrdersLen=e->curSubSong->ordersLen;
      break;
    case GUI_UNDO_PATTERN_EDIT:
    case GUI_UNDO_PATTERN_DELETE:
    case GUI_UNDO_PATTERN_PULL:
    case GUI_UNDO_PATTERN_PUSH:
    case GUI_UNDO_PATTERN_CUT:
    case GUI_UNDO_PATTERN_PASTE:
    case GUI_UNDO_PATTERN_CHANGE_INS:
    case GUI_UNDO_PATTERN_INTERPOLATE:
    case GUI_UNDO_PATTERN_FADE:
    case GUI_UNDO_PATTERN_SCALE:
    case GUI_UNDO_PATTERN_RANDOMIZE:
    case GUI_UNDO_PATTERN_INVERT_VAL:
    case GUI_UNDO_PATTERN_FLIP:
    case GUI_UNDO_PATTERN_COLLAPSE:
    case GUI_UNDO_PATTERN_EXPAND:
    case GUI_UNDO_PATTERN_DRAG:
      for (int h=region.begin.ord; h<=region.end.ord; h++) {
        for (int i=region.begin.x; i<=region.end.x; i++) {
          unsigned short id=h|(i<<8);
          DivPattern* p=NULL;

          auto it=oldPatMap.find(id);
          if (it==oldPatMap.end()) {
            p=oldPatMap[id]=new DivPattern;
            //logV("oldPatMap: allocating for %.4x",id);
          } else {
            p=it->second;
          }

          e->curPat[i].getPattern(e->curOrders->ord[i][h],false)->copyOn(p);
        }
      }
      break;
    case GUI_UNDO_PATTERN_COLLAPSE_SONG:
    case GUI_UNDO_PATTERN_EXPAND_SONG: // this is handled by doCollapseSong/doExpandSong
      break;
    case GUI_UNDO_REPLACE: // this is handled by doReplace()
      break;
  }
}

void FurnaceGUI::makeUndo(ActionType action, UndoRegion region) {
  bool doPush=false;
  UndoStep s;
  s.type=action;
  s.oldCursor=undoCursor;
  s.oldSelStart=undoSelStart;
  s.oldSelEnd=undoSelEnd;
  s.oldScroll=patScroll;
  s.oldOrder=undoOrder;
  s.newCursor=cursor;
  s.newSelStart=selStart;
  s.newSelEnd=selEnd;
  s.newScroll=(nextScroll>=0.0f)?nextScroll:patScroll;
  s.newOrder=curOrder;
  s.oldOrdersLen=oldOrdersLen;
  s.newOrdersLen=e->curSubSong->ordersLen;
  s.nibble=curNibble;
  size_t subSong=e->getCurrentSubSong();

  if (region.begin.ord==-1) {
    region.begin.ord=selStart.order;
    region.end.ord=selEnd.order;
    region.begin.x=0;
    region.end.x=e->getTotalChannelCount()-1;
    region.begin.y=0;
    region.end.y=e->curSubSong->patLen-1;
  } else {
    if (region.begin.ord<0) region.begin.ord=0;
    if (region.begin.ord>e->curSubSong->ordersLen) region.begin.ord=e->curSubSong->ordersLen;
    if (region.end.ord<0) region.end.ord=0;
    if (region.end.ord>e->curSubSong->ordersLen) region.end.ord=e->curSubSong->ordersLen;
    if (region.begin.x<0) region.begin.x=0;
    if (region.begin.x>=e->getTotalChannelCount()) region.begin.x=e->getTotalChannelCount()-1;
    if (region.end.x<0) region.end.x=0;
    if (region.end.x>=e->getTotalChannelCount()) region.end.x=e->getTotalChannelCount()-1;
    if (region.begin.y<0) region.begin.y=0;
    if (region.begin.y>=e->curSubSong->patLen) region.begin.y=e->curSubSong->patLen-1;
    if (region.end.y<0) region.end.y=0;
    if (region.end.y>=e->curSubSong->patLen) region.end.y=e->curSubSong->patLen-1;
  }

  switch (action) {
    case GUI_UNDO_CHANGE_ORDER:
      for (int i=0; i<DIV_MAX_CHANS; i++) {
        for (int j=0; j<128; j++) {
          if (oldOrders.ord[i][j]!=e->curOrders->ord[i][j]) {
            s.ord.push_back(UndoOrderData(subSong,i,j,oldOrders.ord[i][j],e->curOrders->ord[i][j]));
          }
        }
      }
      if (oldOrdersLen!=e->curSubSong->ordersLen) {
        doPush=true;
      }
      if (!s.ord.empty()) {
        doPush=true;
      }
      recalcTimestamps=true;
      break;
    case GUI_UNDO_PATTERN_EDIT:
    case GUI_UNDO_PATTERN_DELETE:
    case GUI_UNDO_PATTERN_PULL:
    case GUI_UNDO_PATTERN_PUSH:
    case GUI_UNDO_PATTERN_CUT:
    case GUI_UNDO_PATTERN_PASTE:
    case GUI_UNDO_PATTERN_CHANGE_INS:
    case GUI_UNDO_PATTERN_INTERPOLATE:
    case GUI_UNDO_PATTERN_FADE:
    case GUI_UNDO_PATTERN_SCALE:
    case GUI_UNDO_PATTERN_RANDOMIZE:
    case GUI_UNDO_PATTERN_INVERT_VAL:
    case GUI_UNDO_PATTERN_FLIP:
    case GUI_UNDO_PATTERN_COLLAPSE:
    case GUI_UNDO_PATTERN_EXPAND:
    case GUI_UNDO_PATTERN_DRAG:
      for (int h=region.begin.ord; h<=region.end.ord; h++) {
        for (int i=region.begin.x; i<=region.end.x; i++) {
          DivPattern* p=e->curPat[i].getPattern(e->curOrders->ord[i][h],false);
          DivPattern* op=NULL;
          unsigned short id=h|(i<<8);

          auto it=oldPatMap.find(id);
          if (it==oldPatMap.end()) {
            logW(_("no data in oldPatMap for channel %d!"),i);
            continue;
          } else {
            op=it->second;
          }

          int jBegin=0;
          int jEnd=e->curSubSong->patLen-1;

          if (h==region.begin.ord) jBegin=region.begin.y;
          if (h==region.end.ord) jEnd=region.end.y;

          for (int j=jBegin; j<=jEnd; j++) {
            for (int k=0; k<DIV_MAX_COLS; k++) {
              if (p->newData[j][k]!=op->newData[j][k]) {
                s.pat.push_back(UndoPatternData(subSong,i,e->curOrders->ord[i][h],j,k,op->newData[j][k],p->newData[j][k]));

                if (k>=DIV_PAT_FX(0)) {
                  int fxCol=(k&1)?k:(k-1);
                  if (op->newData[j][fxCol]==0x09 ||
                      op->newData[j][fxCol]==0x0b ||
                      op->newData[j][fxCol]==0x0d ||
                      op->newData[j][fxCol]==0x0f ||
                      op->newData[j][fxCol]==0xc0 ||
                      op->newData[j][fxCol]==0xc1 ||
                      op->newData[j][fxCol]==0xc2 ||
                      op->newData[j][fxCol]==0xc3 ||
                      op->newData[j][fxCol]==0xf0 ||
                      op->newData[j][fxCol]==0xff ||
                      p->newData[j][fxCol]==0x09 ||
                      p->newData[j][fxCol]==0x0b ||
                      p->newData[j][fxCol]==0x0d ||
                      p->newData[j][fxCol]==0x0f ||
                      p->newData[j][fxCol]==0xc0 ||
                      p->newData[j][fxCol]==0xc1 ||
                      p->newData[j][fxCol]==0xc2 ||
                      p->newData[j][fxCol]==0xc3 ||
                      p->newData[j][fxCol]==0xf0 ||
                      p->newData[j][fxCol]==0xff) {
                    logV("recalcTimestamps due to speed effect.");
                    recalcTimestamps=true;
                  }
                }

              }
            }
          }
        }
      }
      if (!s.pat.empty()) {
        doPush=true;
      }
      break;
    case GUI_UNDO_PATTERN_COLLAPSE_SONG:
    case GUI_UNDO_PATTERN_EXPAND_SONG: // this is handled by doCollapseSong/doExpandSong
      break;
    case GUI_UNDO_REPLACE: // this is handled by doReplace()
      break;
  }
  if (doPush) {
    MARK_MODIFIED;
    undoHist.push_back(s);
    redoHist.clear();
    if (undoHist.size()>settings.maxUndoSteps) undoHist.pop_front();
  }

  // garbage collection
  for (std::pair<unsigned short,DivPattern*> i: oldPatMap) {
    delete i.second;
  }
  oldPatMap.clear();
}

void FurnaceGUI::doSelectAll() {
  finishSelection();
  curNibble=false;
  if (selStart.xFine==0 && selEnd.xFine==2+e->curPat[selEnd.xCoarse].effectCols*2) {
    if (selStart.y==0 && selEnd.y==e->curSubSong->patLen-1) { // select entire pattern
      selStart.xCoarse=0;
      selStart.xFine=0;
      selEnd.xCoarse=e->getTotalChannelCount()-1;
      selEnd.xFine=2+e->curPat[selEnd.xCoarse].effectCols*2;
    } else { // select entire column
      selStart.y=0;
      selEnd.y=e->curSubSong->patLen-1;
    }
  } else {
    int selStartX=0;
    int selEndX=0;
    int chanCount=e->getTotalChannelCount();
    // find row position
    for (SelectionPoint i; i.xCoarse!=selStart.xCoarse || i.xFine!=selStart.xFine; selStartX++) {
      i.xFine++;
      if (i.xCoarse>=chanCount) {
        logE("xCoarse of selStart iterator went too far!");
        showError("you have found a bug. please report it now.");
        i.xCoarse=chanCount-1;
        break;
      }
      if (i.xFine>=3+e->curPat[i.xCoarse].effectCols*2) {
        i.xFine=0;
        i.xCoarse++;
      }
    }
    for (SelectionPoint i; i.xCoarse!=selEnd.xCoarse || i.xFine!=selEnd.xFine; selEndX++) {
      i.xFine++;
      if (i.xCoarse>=chanCount) {
        logE("xCoarse of selEnd iterator went too far!");
        showError("you have found a bug. please report it now.");
        i.xCoarse=chanCount-1;
        break;
      }
      if (i.xFine>=3+e->curPat[i.xCoarse].effectCols*2) {
        i.xFine=0;
        i.xCoarse++;
      }
    }

    float aspect=float(selEndX-selStartX+1)/float(selEnd.y-selStart.y+1);
    if (selStart.order!=selEnd.order) {
      // guarantee vertical aspect ratio
      aspect=0.0f;
    }
    if (aspect<=1.0f && !(selStart.y==0 && selEnd.y==e->curSubSong->patLen-1)) { // up-down
      selStart.y=0;
      selEnd.y=e->curSubSong->patLen-1;
    } else { // left-right
      selStart.xFine=0;
      selEnd.xFine=2+e->curPat[selEnd.xCoarse].effectCols*2;
    }
  }
}

#define maskOut(m,x) \
  if (x==0) { \
    if (!m.note) continue; \
  } else if (x==1) { \
    if (!m.ins) continue; \
  } else if (x==2) { \
    if (!m.vol) continue; \
  } else if (((x)&1)==0) { \
    if (!m.effectVal) continue; \
  } else if (((x)&1)==1) { \
    if (!m.effect) continue; \
  }

#define touch(_order,_y) \
  if (opTouched[(e->curOrders->ord[iCoarse][_order]<<8)|(_y)]) continue; \
  opTouched[(e->curOrders->ord[iCoarse][_order]<<8)|(_y)]=true;

#define resetTouches memset(opTouched,0,DIV_MAX_PATTERNS*DIV_MAX_ROWS);

void FurnaceGUI::doDelete() {
  finishSelection();
  prepareUndo(GUI_UNDO_PATTERN_DELETE);
  curNibble=false;

  int iCoarse=selStart.xCoarse;
  int iFine=selStart.xFine;
  for (; iCoarse<=selEnd.xCoarse; iCoarse++) {
    if (!e->curSubSong->chanShow[iCoarse]) continue;
    for (; iFine<3+e->curPat[iCoarse].effectCols*2 && (iCoarse<selEnd.xCoarse || iFine<=selEnd.xFine); iFine++) {
      int jOrder=selStart.order;
      int j=selStart.y;
      maskOut(opMaskDelete,iFine);
      resetTouches;
      for (; jOrder<=selEnd.order; jOrder++) {
        DivPattern* pat=e->curPat[iCoarse].getPattern(e->curOrders->ord[iCoarse][jOrder],true);
        for (; j<e->curSubSong->patLen && (j<=selEnd.y || jOrder<selEnd.order); j++) {
          touch(jOrder,j);
          if (iFine==0) {
            if (selStart.y==selEnd.y && selStart.order==selEnd.order) pat->newData[j][DIV_PAT_INS]=-1;
          }
          pat->newData[j][iFine]=-1;

          if (selStart.y==selEnd.y && selStart.order==selEnd.order && DIV_PAT_IS_EFFECT(iFine) && settings.effectDeletionAltersValue) {
            pat->newData[j][iFine+1]=-1;
          }
        }
        j=0;
      }
    }
    iFine=0;
  }

  makeUndo(GUI_UNDO_PATTERN_DELETE);
}

void FurnaceGUI::doPullDelete() {
  finishSelection();

  if (selStart.order!=selEnd.order) {
    showError(_("you can only pull delete within the same order."));
    return;
  }

  prepareUndo(GUI_UNDO_PATTERN_PULL);
  curNibble=false;

  if (settings.pullDeleteBehavior) {
    if (--selStart.y<0) {
      if (--selStart.order<0) {
        selStart.order=0;
        selStart.y=0;
      } else {
        selStart.y+=e->curSubSong->patLen;
      }
    }
    if (--selEnd.y<0) {
      if (--selEnd.order<0) {
        selEnd.order=0;
        selEnd.y=0;
      } else {
        selEnd.y+=e->curSubSong->patLen;
      }
    }
    if (--cursor.y<0) {
      if (--cursor.order<0) {
        cursor.order=0;
        cursor.y=0;
      } else {
        cursor.y+=e->curSubSong->patLen;
      }
    }
    updateScroll(cursor.y);
  }

  SelectionPoint sStart=selStart;
  SelectionPoint sEnd=selEnd;

  if (selStart.xCoarse==selEnd.xCoarse && selStart.xFine==selEnd.xFine && selStart.y==selEnd.y && selStart.order==selEnd.order && settings.pullDeleteRow) {
    sStart.xFine=0;
    sEnd.xFine=2+e->curPat[sEnd.xCoarse].effectCols*2;
  }

  int iCoarse=sStart.xCoarse;
  int iFine=sStart.xFine;
  for (; iCoarse<=sEnd.xCoarse; iCoarse++) {
    if (!e->curSubSong->chanShow[iCoarse]) continue;
    DivPattern* pat=e->curPat[iCoarse].getPattern(e->curOrders->ord[iCoarse][curOrder],true);
    for (; iFine<3+e->curPat[iCoarse].effectCols*2 && (iCoarse<sEnd.xCoarse || iFine<=sEnd.xFine); iFine++) {
      maskOut(opMaskPullDelete,iFine);
      for (int j=sStart.y; j<e->curSubSong->patLen; j++) {
        // TODO: we've got a problem here. this should pull from the next row if the selection spans
        //       more than one order.
        if (j<e->curSubSong->patLen-1) {
          pat->newData[j][iFine]=pat->newData[j+1][iFine];
        } else {
          pat->newData[j][iFine]=-1;
        }
      }
    }
    iFine=0;
  }

  makeUndo(GUI_UNDO_PATTERN_PULL);
}

void FurnaceGUI::doInsert() {
  finishSelection();

  if (selStart.order!=selEnd.order) {
    showError(_("you can only insert/push within the same order."));
    return;
  }

  prepareUndo(GUI_UNDO_PATTERN_PUSH);
  curNibble=false;

  SelectionPoint sStart=selStart;
  SelectionPoint sEnd=selEnd;

  if (selStart.xCoarse==selEnd.xCoarse && selStart.xFine==selEnd.xFine && selStart.y==selEnd.y && selStart.order==selEnd.order && settings.insertBehavior) {
    sStart.xFine=0;
    sEnd.xFine=2+e->curPat[sEnd.xCoarse].effectCols*2;
  }

  int iCoarse=sStart.xCoarse;
  int iFine=sStart.xFine;
  for (; iCoarse<=sEnd.xCoarse; iCoarse++) {
    if (!e->curSubSong->chanShow[iCoarse]) continue;
    DivPattern* pat=e->curPat[iCoarse].getPattern(e->curOrders->ord[iCoarse][curOrder],true);
    for (; iFine<3+e->curPat[iCoarse].effectCols*2 && (iCoarse<sEnd.xCoarse || iFine<=sEnd.xFine); iFine++) {
      maskOut(opMaskInsert,iFine);
      for (int j=e->curSubSong->patLen-1; j>=sStart.y; j--) {
        if (j==sStart.y) {
          pat->newData[j][iFine]=-1;
        } else {
          pat->newData[j][iFine]=pat->newData[j-1][iFine];
        }
      }
    }
    iFine=0;
  }

  makeUndo(GUI_UNDO_PATTERN_PUSH);
}

void FurnaceGUI::doTranspose(int amount, OperationMask& mask) {
  finishSelection();
  prepareUndo(GUI_UNDO_PATTERN_DELETE);
  curNibble=false;

  int iCoarse=selStart.xCoarse;
  int iFine=selStart.xFine;
  for (; iCoarse<=selEnd.xCoarse; iCoarse++) {
    if (!e->curSubSong->chanShow[iCoarse]) continue;
    for (; iFine<3+e->curPat[iCoarse].effectCols*2 && (iCoarse<selEnd.xCoarse || iFine<=selEnd.xFine); iFine++) {
      int jOrder=selStart.order;
      int j=selStart.y;
      maskOut(mask,iFine);
      resetTouches;
      for (; jOrder<=selEnd.order; jOrder++) {
        DivPattern* pat=e->curPat[iCoarse].getPattern(e->curOrders->ord[iCoarse][jOrder],true);
        for (; j<e->curSubSong->patLen && (j<=selEnd.y || jOrder<selEnd.order); j++) {
          touch(jOrder,j);
          int top=255;
          if (iFine==DIV_PAT_NOTE) {
            top=179;
            // don't transpose special notes
            if (pat->newData[j][iFine]==DIV_NOTE_OFF) continue;
            if (pat->newData[j][iFine]==DIV_NOTE_REL) continue;
            if (pat->newData[j][iFine]==DIV_MACRO_REL) continue;
            if (pat->newData[j][iFine]==DIV_NOTE_NULL_PAT) continue;
          } else if (iFine==DIV_PAT_INS) {
            if (e->song.ins.empty()) continue;
            top=e->song.ins.size()-1;
          } else if (iFine==DIV_PAT_VOL) { // volume
            top=e->getMaxVolumeChan(iCoarse);
          }
          if (pat->newData[j][iFine]==-1) continue;
          pat->newData[j][iFine]=MIN(top,MAX(0,pat->newData[j][iFine]+amount));
        }
        j=0;
      }
    }
    iFine=0;
  }

  makeUndo(GUI_UNDO_PATTERN_DELETE);
}

String FurnaceGUI::doCopy(bool cut, bool writeClipboard, const SelectionPoint& sStart, const SelectionPoint& sEnd) {
  if (writeClipboard) {
    finishSelection();
    if (cut) {
      curNibble=false;
      prepareUndo(GUI_UNDO_PATTERN_CUT);
    }
  }
  String clipb=fmt::sprintf("org.tildearrow.furnace - Pattern Data (%d)\n%d",DIV_ENGINE_VERSION,sStart.xFine);
  int jOrder=sStart.order;
  int j=sStart.y;

  for (; jOrder<=sEnd.order; jOrder++) {
    for (; j<e->curSubSong->patLen && (j<=sEnd.y || jOrder<sEnd.order); j++) {
      int iCoarse=sStart.xCoarse;
      int iFine=sStart.xFine;
      if (iFine>3 && !(iFine&1)) {
        iFine--;
      }
      clipb+='\n';
      for (; iCoarse<=sEnd.xCoarse; iCoarse++) {
        if (!e->curSubSong->chanShow[iCoarse]) continue;
        DivPattern* pat=e->curPat[iCoarse].getPattern(e->curOrders->ord[iCoarse][jOrder],true);
        for (; iFine<3+e->curPat[iCoarse].effectCols*2 && (iCoarse<sEnd.xCoarse || iFine<=sEnd.xFine); iFine++) {
          if (iFine==0) {
            clipb+=noteNameNormal(pat->newData[j][DIV_PAT_NOTE]);
            if (cut) {
              pat->newData[j][DIV_PAT_NOTE]=-1;
            }
          } else {
            if (pat->newData[j][iFine]==-1) {
              clipb+="..";
            } else {
              clipb+=fmt::sprintf("%.2X",pat->newData[j][iFine]);
            }
            if (cut) {
              pat->newData[j][iFine]=-1;
            }
          }
        }
        clipb+='|';
        iFine=0;
      }
    }
    j=0;
  }

  if (writeClipboard) {
    SDL_SetClipboardText(clipb.c_str());
    if (cut) {
      makeUndo(GUI_UNDO_PATTERN_CUT);
    }
    clipboard=clipb;
  }
  return clipb;
}

void FurnaceGUI::doPasteFurnace(PasteMode mode, int arg, bool readClipboard, String clipb, std::vector<String> data, int startOff, bool invalidData, UndoRegion ur)
{
  if (sscanf(data[1].c_str(),"%d",&startOff)!=1) return;
  if (startOff<0) return;

  DETERMINE_LAST;

  int j=cursor.y;
  int jOrder=cursor.order;
  char note[4];
  for (size_t i=2; i<data.size() && j<e->curSubSong->patLen; i++) {
    size_t charPos=0;
    int iCoarse=cursor.xCoarse;
    int iFine=(startOff>2 && cursor.xFine>2)?(((cursor.xFine-1)&(~1))|1):startOff;

    String& line=data[i];

    while (charPos<line.size() && iCoarse<lastChannel) {
      DivPattern* pat=e->curPat[iCoarse].getPattern(e->curOrders->ord[iCoarse][jOrder],true);
      if (line[charPos]=='|') {
        iCoarse++;
        if (iCoarse<lastChannel) while (!e->curSubSong->chanShow[iCoarse]) {
          iCoarse++;
          if (iCoarse>=lastChannel) break;
        }
        iFine=0;
        charPos++;
        continue;
      }
      if (iFine==0) {
        if (charPos>=line.size()) {
          invalidData=true;
          break;
        }
        note[0]=line[charPos++];
        if (charPos>=line.size()) {
          invalidData=true;
          break;
        }
        note[1]=line[charPos++];
        if (charPos>=line.size()) {
          invalidData=true;
          break;
        }
        note[2]=line[charPos++];
        note[3]=0;

        if (iFine==0 && !opMaskPaste.note) {
          iFine++;
          continue;
        }

        if ((mode==GUI_PASTE_MODE_MIX_BG || mode==GUI_PASTE_MODE_MIX_FG ||
             mode==GUI_PASTE_MODE_INS_BG || mode==GUI_PASTE_MODE_INS_FG) && strcmp(note,"...")==0) {
          // do nothing.
        } else {
          if (!(mode==GUI_PASTE_MODE_MIX_BG || mode==GUI_PASTE_MODE_INS_BG) || (pat->newData[j][DIV_PAT_NOTE]==-1)) {
            if (!decodeNote(note,pat->newData[j][DIV_PAT_NOTE])) {
              invalidData=true;
              break;
            }
            if (mode==GUI_PASTE_MODE_INS_BG || mode==GUI_PASTE_MODE_INS_FG) pat->newData[j][DIV_PAT_INS]=arg;
          }
        }
      } else {
        if (charPos>=line.size()) {
          invalidData=true;
          break;
        }
        note[0]=line[charPos++];
        if (charPos>=line.size()) {
          invalidData=true;
          break;
        }
        note[1]=line[charPos++];
        note[2]=0;

        if (iFine==DIV_PAT_INS) {
          if (!opMaskPaste.ins || mode==GUI_PASTE_MODE_INS_BG || mode==GUI_PASTE_MODE_INS_FG) {
            iFine++;
            continue;
          }
        } else if (iFine==DIV_PAT_VOL) {
          if (!opMaskPaste.vol) {
            iFine++;
            continue;
          }
        } else if ((iFine&1)==0) {
          if (!opMaskPaste.effectVal) {
            iFine++;
            continue;
          }
        } else if ((iFine&1)==1) {
          if (!opMaskPaste.effect) {
            iFine++;
            continue;
          }
        }

        if (strcmp(note,"..")==0) {
          if (!(mode==GUI_PASTE_MODE_MIX_BG || mode==GUI_PASTE_MODE_MIX_FG ||
                mode==GUI_PASTE_MODE_INS_BG || mode==GUI_PASTE_MODE_INS_FG)) {
            pat->newData[j][iFine]=-1;
          }
        } else {
          unsigned int val=0;
          if (sscanf(note,"%2X",&val)!=1) {
            invalidData=true;
            break;
          }
          if (!(mode==GUI_PASTE_MODE_MIX_BG || mode==GUI_PASTE_MODE_INS_BG) || pat->newData[j][iFine]==-1) {
            if (iFine<(3+e->curPat[iCoarse].effectCols*2)) pat->newData[j][iFine]=val;
          }
        }
      }
      iFine++;
    }
    
    if (invalidData) {
      logW(_("invalid clipboard data! failed at line %d char %d"),i,charPos);
      logW("%s",line.c_str());
      break;
    }
    j++;
    if (mode==GUI_PASTE_MODE_OVERFLOW && j>=e->curSubSong->patLen && jOrder<e->curSubSong->ordersLen-1) {
      j=0;
      jOrder++;
    }

    if (mode==GUI_PASTE_MODE_FLOOD && i==data.size()-1) {
      i=1;
    }
  }

  curOrder=jOrder;
  if (mode==GUI_PASTE_MODE_OVERFLOW && !e->isPlaying()) {
    setOrder(jOrder);
  }

  if (readClipboard) {
    if (settings.cursorPastePos) {
      makeCursorUndo();
      cursor.y=j;
      cursor.order=curOrder;
      if (cursor.y>=e->curSubSong->patLen) cursor.y=e->curSubSong->patLen-1;
      selStart=cursor;
      selEnd=cursor;
      updateScroll(cursor.y);
    }

    makeUndo(GUI_UNDO_PATTERN_PASTE,ur);
  }
}

unsigned int convertEffectMPT_MOD(unsigned char symbol, unsigned int val) {
  switch (symbol) {
    case '0':
      return (0x00<<8)|val;
      break;
    case '1':
      return (0x01<<8)|val;
      break;
    case '2':
      return (0x02<<8)|val;
      break;
    case '3':
      return (0x03<<8)|val;
      break;
    case '4':
      return (0x04<<8)|val;
      break;
    case '5':
      return (0x0a<<8)|val|(0x03<<24); // Axy+300
      break;
    case '6':
      return (0x0a<<8)|val|(0x04<<24); // Axy+400
      break;
    case '7':
      return (0x07<<8)|val;
      break;
    case '8':
      return (0x80<<8)|val;
      break;
    case '9':
      return (0x90<<8)|val;
      break;
    case 'A':
      return (0x0A<<8)|val;
      break;
    case 'B':
      return (0x0B<<8)|val;
      break;
    case 'C':
      return (0x0C<<8)|val; // interpreted as volume later
      break;
    case 'D': {
      unsigned char newParam=(val&0xf)+((val&0xff)>>4)*10; // hex to decimal, Protracker (and XM too!) lol
      return (0x0D<<8)|newParam;
      break;
    }
    case 'E': {
      switch (val>>4) {
        case 1:
          return (0xF1<<8)|(val&0xf);
          break;
        case 2:
          return (0xF2<<8)|(val&0xf);
          break;
        // glissando and vib shape not supported in Furnace
        case 5:
          return (0xF5<<8)|((val&0xf)<<4);
          break;
        // pattern loop not supported
        case 8:
          return (0x80<<8)|((val&0xf)<<4);
          break;
        case 9:
          return (0x0C<<8)|(val&0xf);
          break;
        case 0xA:
          return (0xF3<<8)|(val&0xf);
          break;
        case 0xB:
          return (0xF4<<8)|(val&0xf);
          break;
        case 0xC:
          return (0xFC<<8)|(val&0xf);
          break;
        case 0xD:
          return (0xFD<<8)|(val&0xf);
          break;
        default:
          break;
      }
      break;
    }
    case 'F':
      if (val<0x20) {
        return (0x0F<<8)|val;
      } else {
        return (0xF0<<8)|val;
      }
      break;
  }

  return 0;
}

unsigned int convertEffectMPT_S3M(unsigned char symbol, unsigned int val) {
  switch (symbol) {
    case 'A':
      return (0x09<<8)|val;
      break;
    case 'B':
      return (0x0B<<8)|val;
      break;
    case 'C':
      return (0x0D<<8)|val;
      break;
    case 'D':
      if ((val&0xf0)==0xf0) {
        return (0xF4<<8)|(val&0xf);
      } else if ((val&0xf)==0xf) {
        return (0xF3<<8)|((val&0xf0)>>4);
      } else {
        return (0x0A<<8)|val;
      }
      break;
    case 'E':
      if (val<0xe0) {
        return (0x02<<8)|val;
      } else if (val>=0xe0 && val<0xf0) {
        return (0xF2<<8)|(val&0xf);
      } else {
        return (0xF2<<8)|((val&0xf)>>1);
      }
      break;
    case 'F':
      if (val<0xe0) {
        return (0x01<<8)|val;
      } else if (val>=0xe0 && val<0xf0) {
        return (0xF1<<8)|(val&0xf);
      } else {
        return (0xF1<<8)|((val&0xf)>>1);
      }
      break;
    case 'G':
      return (0x03<<8)|val;
      break;
    case 'H':
      return (0x04<<8)|val;
      break;
    case 'J':
      return (0x00<<8)|val;
      break;
    case 'K':
      return (0x0a<<8)|val|(0x04<<24); // Axy+400
      break;
    case 'L':
      return (0x0a<<8)|val|(0x03<<24); // Axy+300
      break;
    case 'O':
      return (0x90<<8)|val;
      break;
    case 'Q':
      return (0xC0<<8)|(val&0xf);
      break;
    case 'R':
      return (0x07<<8)|(val&0xf);
      break;
    case 'S': {
      switch (val>>4) {
        case 2:
          return (0xE5<<8)|((val&0xf)<<4);
          break;
        case 8:
          return (0x80<<8)|((val&0xf)<<4);
          break;
        case 0xC:
          return (0xEC<<8)|(val&0xf);
          break;
        case 0xD:
          return (0xED<<8)|(val&0xf);
          break;
        default:
          break;
      }
      break;
    }
    case 'T':
      return (0xF0<<8)|(val&0xf);
      break;
    case 'U':
      return (0x04<<8)|MAX(1,((val&0xf0)>>6)<<4)|MAX(1,(val&0xf)>>2);
      break;
    case 'X':
      return (0x80<<8)|val;
      break;
    default:
      return 0;
      break;
  }

  return 0;
}

unsigned int convertEffectMPT_XM(unsigned char symbol, unsigned int val) {
  if (symbol=='K') {
    return (0xEC<<8)|val;
  }

  return convertEffectMPT_MOD(symbol,val); // for now
}

unsigned int convertEffectMPT_IT(unsigned char symbol, unsigned int val) {
  return convertEffectMPT_S3M(symbol,val); // for now
}

unsigned int convertEffectMPT_MPTM(unsigned char symbol, unsigned int val) {
  if (symbol==':') {
    return (0xED<<8)|((val&0xf0)>>4)|(0xEC<<24)|((((val&0xf0)>>4)+(val&0xf))<<16);
  }

  return convertEffectMPT_IT(symbol,val);
}

void FurnaceGUI::doPasteMPT(PasteMode mode, int arg, bool readClipboard, String clipb, std::vector<String> data, int mptFormat, UndoRegion ur) {
  DETERMINE_LAST;

  int j=cursor.y;
  int jOrder=cursor.order;
  char note[4];
  bool invalidData=false;

  memset(note,0,4);

  for (size_t i=1; i<data.size() && j<e->curSubSong->patLen; i++) {
    size_t charPos=1;
    int iCoarse=cursor.xCoarse;
    int iFine=0;
    String& line=data[i];
    while (charPos<line.size() && iCoarse<lastChannel) {
      DivPattern* pat=e->curPat[iCoarse].getPattern(e->curOrders->ord[iCoarse][jOrder],true);
      if (line[charPos]=='|' && charPos!=0) { // MPT format starts every pattern line with '|'
        iCoarse++;
        if (iCoarse<lastChannel) {
          while (!e->curSubSong->chanShow[iCoarse]) {
            iCoarse++;
            if (iCoarse>=lastChannel) break;
          }
        }
        iFine=0;
        charPos++;
        continue;
      }
      if (iFine==DIV_PAT_NOTE) { // note
        if (charPos>=line.size()) {
          invalidData=true;
          break;
        }
        note[0]=line[charPos++];
        if (charPos>=line.size()) {
          invalidData=true;
          break;
        }
        note[1]=line[charPos++];
        if (charPos>=line.size()) {
          invalidData=true;
          break;
        }
        note[2]=line[charPos++];
        note[3]=0;

        if (iFine==0 && !opMaskPaste.note) {
          iFine++;
          continue;
        }

        if (strcmp(note,"...")==0 || strcmp(note,"   ")==0) {
          // do nothing.
        } else {
          if (!(mode==GUI_PASTE_MODE_MIX_BG || mode==GUI_PASTE_MODE_INS_BG) || (pat->newData[j][DIV_PAT_NOTE]==-1)) {
            if (!decodeNote(note,pat->newData[j][DIV_PAT_NOTE])) {
              if (strcmp(note, "^^^")==0) {
                pat->newData[j][DIV_PAT_NOTE]=DIV_NOTE_OFF;
              } else if (strcmp(note, "~~~")==0 || strcmp(note,"===")==0) {
                pat->newData[j][DIV_PAT_NOTE]=DIV_NOTE_REL;
              } else {
                invalidData=true;
                break;
              }
            } else if (pat->newData[j][DIV_PAT_NOTE]<180) {
              // MPT is one octave higher...
              if (pat->newData[j][DIV_PAT_NOTE]<12) {
                pat->newData[j][DIV_PAT_NOTE]=0;
              } else {
                pat->newData[j][DIV_PAT_NOTE]-=12;
              }
            }
            if (mode==GUI_PASTE_MODE_INS_BG || mode==GUI_PASTE_MODE_INS_FG) pat->newData[j][DIV_PAT_INS]=arg;
          }
        }
      } else if (iFine==DIV_PAT_INS) { // instrument
        if (charPos>=line.size()) {
          invalidData=true;
          break;
        }
        note[0]=line[charPos++];
        if (charPos>=line.size()) {
          invalidData=true;
          break;
        }
        note[1]=line[charPos++];
        note[2]=0;

        if (iFine==DIV_PAT_INS) {
          if (!opMaskPaste.ins || mode==GUI_PASTE_MODE_INS_BG || mode==GUI_PASTE_MODE_INS_FG) {
            iFine++;
            continue;
          }
        }

        if (strcmp(note,"..")==0 || strcmp(note,"  ")==0) {
          if (!(mode==GUI_PASTE_MODE_MIX_BG || mode==GUI_PASTE_MODE_MIX_FG ||
                mode==GUI_PASTE_MODE_INS_BG || mode==GUI_PASTE_MODE_INS_FG)) {
            pat->newData[j][iFine]=-1;
          }
        } else {
          unsigned int val=0;
          if (sscanf(note,"%2d",&val)!=1) {
            invalidData=true;
            break;
          }

          if (!(mode==GUI_PASTE_MODE_MIX_BG || mode==GUI_PASTE_MODE_INS_BG) || pat->newData[j][iFine]==-1) {
            pat->newData[j][iFine]=val-1;
          }
        }
      } else { // volume and effects
        if (charPos>=line.size()) {
          invalidData=true;
          break;
        }
        note[0]=line[charPos++];
        if (charPos>=line.size()) {
          invalidData=true;
          break;
        }
        note[1]=line[charPos++];
        if (charPos>=line.size()) {
          invalidData=true;
          break;
        }
        note[2]=line[charPos++];
        note[3]=0;

        if (iFine==DIV_PAT_VOL) {
          if (!opMaskPaste.vol) {
            iFine++;
            continue;
          }
        } else if ((iFine&1)==0) { // FUCKING HELL WITH THE INDENTATION?!?!
          if (!opMaskPaste.effectVal) {
            iFine++;
            continue;
          }
        } else if ((iFine&1)==1) {
          if (!opMaskPaste.effect) {
            iFine++;
            continue;
          }
        }

        if (strcmp(note,"...")==0 || strcmp(note,"   ")==0) {
          if (!(mode==GUI_PASTE_MODE_MIX_BG || mode==GUI_PASTE_MODE_MIX_FG ||
                mode==GUI_PASTE_MODE_INS_BG || mode==GUI_PASTE_MODE_INS_FG)) {
            pat->newData[j][iFine]=-1;
          }
        } else {
          unsigned int val=0;
          unsigned char symbol='\0';

          symbol=note[0];

          if (iFine==DIV_PAT_VOL) {
            sscanf(&note[1],"%2d",&val);
          } else {
            sscanf(&note[1],"%2X",&val);
          }

          if (!(mode==GUI_PASTE_MODE_MIX_BG || mode==GUI_PASTE_MODE_INS_BG) || pat->newData[j][iFine]==-1) {
            // if (iFine<(3+e->curPat[iCoarse].effectCols*2)) pat->newData[j][iFine]=val;
            if (iFine==DIV_PAT_VOL) { // volume
              switch(symbol) {
                case 'v':
                {
                  pat->newData[j][iFine]=val;
                  break;
                }
                default:
                  break;
              }
            } else { // effect
              unsigned int eff=0;
              if (mptFormat==0) {
                eff=convertEffectMPT_MOD(symbol, val); // up to 4 effects stored in one variable
                if (((eff&0x0f00)>>8)==0x0C) { // set volume
                  pat->newData[j][iFine-1]=eff&0xff;
                }
              }

              if (mptFormat==1) {
                eff=convertEffectMPT_S3M(symbol, val);
              }

              if (mptFormat==2 || mptFormat==3) { // set volume
                eff=convertEffectMPT_XM(symbol, val);

                if (((eff&0x0f00)>>8)==0x0C)
                {
                  pat->newData[j][iFine-1]=eff&0xff;
                }
              }

              if (mptFormat==4|| mptFormat==5) {
                eff=convertEffectMPT_IT(symbol, val);
              }

              if (mptFormat==6) {
                eff=convertEffectMPT_MPTM(symbol, val);
              }

              pat->newData[j][iFine]=((eff&0xff00)>>8);
              pat->newData[j][iFine+1]=(eff&0xff);

              if (eff>0xffff) {
                pat->newData[j][iFine+2]=((eff&0xff000000)>>24);
                pat->newData[j][iFine+3]=((eff&0xff0000)>>16);
              }
            }
          }
        }
      }

      iFine++;

      if (charPos>=line.size()-1) {
        invalidData=false;
        break;
      }
    }

    if (invalidData) {
      logW(_("invalid clipboard data! failed at line %d char %d"),i,charPos);
      logW("%s",line.c_str());
      break;
    }

    j++;
    if (mode==GUI_PASTE_MODE_OVERFLOW && j>=e->curSubSong->patLen && jOrder<e->curSubSong->ordersLen-1) {
      j=0;
      jOrder++;
    }

    if (mode==GUI_PASTE_MODE_FLOOD && i==data.size()-1) {
      i=1;
    }
  }

  curOrder=jOrder;
  if (mode==GUI_PASTE_MODE_OVERFLOW && !e->isPlaying()) {
    setOrder(jOrder);
  }

  if (readClipboard) {
    if (settings.cursorPastePos) {
      makeCursorUndo();
      cursor.y=j;
      cursor.order=curOrder;
      if (cursor.y>=e->curSubSong->patLen) cursor.y=e->curSubSong->patLen-1;
      selStart=cursor;
      selEnd=cursor;
      updateScroll(cursor.y);
    }

    makeUndo(GUI_UNDO_PATTERN_PASTE,ur);
  }
}

void FurnaceGUI::doPaste(PasteMode mode, int arg, bool readClipboard, String clipb) {
  if (readClipboard) {
    finishSelection();
    char* clipText=SDL_GetClipboardText();
    if (clipText!=NULL) {
      if (clipText[0]) {
        clipboard=clipText;
      }
      SDL_free(clipText);
    }
    clipb=clipboard;
  }
  std::vector<String> data;
  String tempS;
  bool foundString=false;
  bool isFurnace=false;
  bool isModPlug=false;
  int mptFormat=0;
  for (char i: clipb) {
    if (i=='\r') continue;
    if (i=='\n') {
      data.push_back(tempS);
      tempS="";
      continue;
    }
    tempS+=i;
  }
  data.push_back(tempS);

  int startOff=-1;
  bool invalidData=false;
  if (data.size()<2) return;

  if (data[0].find("org.tildearrow.furnace - Pattern Data")==0) {
    foundString=true;
    isFurnace=true;
  } else {
    for (int i=0; modPlugFormatHeaders[i]; i++) {
      if (data[0].find(modPlugFormatHeaders[i])==0) {
        foundString=true;
        isModPlug=true;
        mptFormat=i;
        break;
      }
    }
  }

  if (!foundString) return;

  UndoRegion ur;
  if (mode==GUI_PASTE_MODE_OVERFLOW) {
    int rows=cursor.y;
    int firstPattern=cursor.order;
    int lastPattern=cursor.order;
    rows+=data.size();
    while (rows>=e->curSubSong->patLen) {
      lastPattern++;
      rows-=e->curSubSong->patLen;
    }

    ur=UndoRegion(firstPattern,0,0,lastPattern,e->getTotalChannelCount()-1,e->curSubSong->patLen-1);
  }

  if (readClipboard) {
    prepareUndo(GUI_UNDO_PATTERN_PASTE,ur);
  }

  if (isFurnace) {
    doPasteFurnace(mode,arg,readClipboard,clipb,data,startOff,invalidData,ur);
  } else if (isModPlug) {
    doPasteMPT(mode,arg,readClipboard,clipb,data,mptFormat,ur);
  }
}

void FurnaceGUI::doChangeIns(int ins) {
  finishSelection();
  prepareUndo(GUI_UNDO_PATTERN_CHANGE_INS);

  int iCoarse=selStart.xCoarse;
  for (; iCoarse<=selEnd.xCoarse; iCoarse++) {
    int jOrder=selStart.order;
    int j=selStart.y;
    if (!e->curSubSong->chanShow[iCoarse]) continue;
    resetTouches;
    for (; jOrder<=selEnd.order; jOrder++) {
      DivPattern* pat=e->curPat[iCoarse].getPattern(e->curOrders->ord[iCoarse][jOrder],true);
      for (; j<e->curSubSong->patLen && (j<=selEnd.y || jOrder<selEnd.order); j++) {
        touch(jOrder,j);
        if (pat->newData[j][DIV_PAT_INS]!=-1 || !(pat->newData[j][DIV_PAT_NOTE]==-1 || pat->newData[j][DIV_PAT_NOTE]==DIV_NOTE_OFF || pat->newData[j][DIV_PAT_NOTE]==DIV_NOTE_REL || pat->newData[j][DIV_PAT_NOTE]==DIV_MACRO_REL)) {
          pat->newData[j][DIV_PAT_INS]=ins;
        }
      }
      j=0;
    }
  }

  makeUndo(GUI_UNDO_PATTERN_CHANGE_INS);
}

void FurnaceGUI::doInterpolate() {
  finishSelection();
  prepareUndo(GUI_UNDO_PATTERN_INTERPOLATE);

  // first: fixed point, 8-bit order.row
  // second: value
  std::vector<std::pair<int,int>> points;
  int iCoarse=selStart.xCoarse;
  int iFine=selStart.xFine;
  for (; iCoarse<=selEnd.xCoarse; iCoarse++) {
    if (!e->curSubSong->chanShow[iCoarse]) continue;
    for (; iFine<3+e->curPat[iCoarse].effectCols*2 && (iCoarse<selEnd.xCoarse || iFine<=selEnd.xFine); iFine++) {
      maskOut(opMaskInterpolate,iFine);
      points.clear();
      resetTouches;
      if (iFine!=DIV_PAT_NOTE) {
        int jOrder=selStart.order;
        int j=selStart.y;
        for (; jOrder<=selEnd.order; jOrder++) {
          DivPattern* pat=e->curPat[iCoarse].getPattern(e->curOrders->ord[iCoarse][jOrder],true);
          for (; j<e->curSubSong->patLen && (j<=selEnd.y || jOrder<selEnd.order); j++) {
            touch(jOrder,j);
            if (pat->newData[j][iFine]!=-1) {
              points.emplace(points.end(),j|(jOrder<<8),pat->newData[j][iFine]);
            }
          }
          j=0;
        }

        if (points.size()>1) for (size_t j=0; j<points.size()-1; j++) {
          std::pair<int,int>& curPoint=points[j];
          std::pair<int,int>& nextPoint=points[j+1];
          int distance=(
            ((nextPoint.first&0xff)+((nextPoint.first>>8)*e->curSubSong->patLen))-
            ((curPoint.first&0xff)+((curPoint.first>>8)*e->curSubSong->patLen))
          );
          for (int k=0, k_p=curPoint.first; k<distance; k++) {
            DivPattern* pat=e->curPat[iCoarse].getPattern(e->curOrders->ord[iCoarse][(k_p>>8)&0xff],true);
            pat->newData[k_p&0xff][iFine]=curPoint.second+((nextPoint.second-curPoint.second)*(double)k/(double)distance);
            k_p++;
            if ((k_p&0xff)>=e->curSubSong->patLen) {
              k_p&=~0xff;
              k_p+=0x100;
            }
          }
        }
      } else {
        int jOrder=selStart.order;
        int j=selStart.y;
        for (; jOrder<=selEnd.order; jOrder++) {
          DivPattern* pat=e->curPat[iCoarse].getPattern(e->curOrders->ord[iCoarse][jOrder],true);
          for (; j<e->curSubSong->patLen && (j<=selEnd.y || jOrder<selEnd.order); j++) {
            touch(jOrder,j);
            if (pat->newData[j][DIV_PAT_NOTE]!=-1) {
              if (pat->newData[j][DIV_PAT_NOTE]!=DIV_NOTE_OFF && pat->newData[j][DIV_PAT_NOTE]!=DIV_NOTE_REL && pat->newData[j][DIV_PAT_NOTE]!=DIV_MACRO_REL) {
                points.emplace(points.end(),j|(jOrder<<8),pat->newData[j][DIV_PAT_NOTE]);
              }
            }
          }
          j=0;
        }

        if (points.size()>1) for (size_t j=0; j<points.size()-1; j++) {
          std::pair<int,int>& curPoint=points[j];
          std::pair<int,int>& nextPoint=points[j+1];
          int distance=(
            ((nextPoint.first&0xff)+((nextPoint.first>>8)*e->curSubSong->patLen))-
            ((curPoint.first&0xff)+((curPoint.first>>8)*e->curSubSong->patLen))
          );
          for (int k=0, k_p=curPoint.first; k<distance; k++) {
            DivPattern* pat=e->curPat[iCoarse].getPattern(e->curOrders->ord[iCoarse][(k_p>>8)&0xff],true);
            int val=curPoint.second+((nextPoint.second-curPoint.second)*(double)k/(double)distance);
            pat->newData[k_p&0xff][DIV_PAT_NOTE]=val;
            k_p++;
            if ((k_p&0xff)>=e->curSubSong->patLen) {
              k_p&=~0xff;
              k_p+=0x100;
            }
          }
        }
      }
    }
    iFine=0;
  }

  makeUndo(GUI_UNDO_PATTERN_INTERPOLATE);
}

void FurnaceGUI::doFade(int p0, int p1, bool mode) {
  finishSelection();
  prepareUndo(GUI_UNDO_PATTERN_FADE);

  int iCoarse=selStart.xCoarse;
  int iFine=selStart.xFine;
  int distance=(
    (selEnd.y+(selEnd.order*e->curSubSong->patLen))-
    (selStart.y+(selStart.order*e->curSubSong->patLen))
  );
  for (; iCoarse<=selEnd.xCoarse; iCoarse++) {
    if (!e->curSubSong->chanShow[iCoarse]) continue;
    for (; iFine<3+e->curPat[iCoarse].effectCols*2 && (iCoarse<selEnd.xCoarse || iFine<=selEnd.xFine); iFine++) {
      int jOrder=selStart.order;
      int j=selStart.y;
      int j_p=0;
      maskOut(opMaskFade,iFine);
      resetTouches;
      if (iFine!=DIV_PAT_NOTE) {
        int absoluteTop=255;
        if (iFine==DIV_PAT_INS) {
          if (e->song.ins.empty()) continue;
          absoluteTop=e->song.ins.size()-1;
        } else if (iFine==DIV_PAT_VOL) { // volume
          absoluteTop=e->getMaxVolumeChan(iCoarse);
        }
        if (distance<1) continue;
        for (; jOrder<=selEnd.order; jOrder++) {
          DivPattern* pat=e->curPat[iCoarse].getPattern(e->curOrders->ord[iCoarse][jOrder],true);
          for (; j<e->curSubSong->patLen && (j<=selEnd.y || jOrder<selEnd.order); j++) {
            double fraction=double(j_p)/double(distance);
            int value=p0+double(p1-p0)*fraction;
            if (mode) { // nibble
              value&=15;
              pat->newData[j][iFine]=MIN(absoluteTop,value|(value<<4));
            } else { // byte
              pat->newData[j][iFine]=MIN(absoluteTop,value);
            }
            j_p++;
          }
          j=0;
        }
      }
    }
    iFine=0;
  }

  makeUndo(GUI_UNDO_PATTERN_FADE);
}

void FurnaceGUI::doInvertValues() {
  finishSelection();
  prepareUndo(GUI_UNDO_PATTERN_INVERT_VAL);

  int iCoarse=selStart.xCoarse;
  int iFine=selStart.xFine;
  for (; iCoarse<=selEnd.xCoarse; iCoarse++) {
    if (!e->curSubSong->chanShow[iCoarse]) continue;
    for (; iFine<3+e->curPat[iCoarse].effectCols*2 && (iCoarse<selEnd.xCoarse || iFine<=selEnd.xFine); iFine++) {
      int jOrder=selStart.order;
      int j=selStart.y;
      maskOut(opMaskInvertVal,iFine);
      resetTouches;
      if (iFine!=DIV_PAT_NOTE) {
        int top=255;
        if (iFine==DIV_PAT_INS) {
          if (e->song.ins.empty()) continue;
          top=e->song.ins.size()-1;
        } else if (iFine==DIV_PAT_VOL) { // volume
          top=e->getMaxVolumeChan(iCoarse);
        }
        for (; jOrder<=selEnd.order; jOrder++) {
          DivPattern* pat=e->curPat[iCoarse].getPattern(e->curOrders->ord[iCoarse][jOrder],true);
          for (; j<e->curSubSong->patLen && (j<=selEnd.y || jOrder<selEnd.order); j++) {
            touch(jOrder,j);
            if (pat->newData[j][iFine]==-1) continue;
            pat->newData[j][iFine]=top-pat->newData[j][iFine];
          }
          j=0;
        }
      }
    }
    iFine=0;
  }

  makeUndo(GUI_UNDO_PATTERN_INVERT_VAL);
}

void FurnaceGUI::doScale(float top) {
  finishSelection();
  prepareUndo(GUI_UNDO_PATTERN_SCALE);

  int iCoarse=selStart.xCoarse;
  int iFine=selStart.xFine;
  for (; iCoarse<=selEnd.xCoarse; iCoarse++) {
    if (!e->curSubSong->chanShow[iCoarse]) continue;
    for (; iFine<3+e->curPat[iCoarse].effectCols*2 && (iCoarse<selEnd.xCoarse || iFine<=selEnd.xFine); iFine++) {
      int jOrder=selStart.order;
      int j=selStart.y;
      maskOut(opMaskScale,iFine);
      resetTouches;
      if (iFine!=DIV_PAT_NOTE) {
        int absoluteTop=255;
        if (iFine==DIV_PAT_INS) {
          if (e->song.ins.empty()) continue;
          absoluteTop=e->song.ins.size()-1;
        } else if (iFine==DIV_PAT_VOL) { // volume
          absoluteTop=e->getMaxVolumeChan(iCoarse);
        }
        for (; jOrder<=selEnd.order; jOrder++) {
          DivPattern* pat=e->curPat[iCoarse].getPattern(e->curOrders->ord[iCoarse][jOrder],true);
          for (; j<e->curSubSong->patLen && (j<=selEnd.y || jOrder<selEnd.order); j++) {
            touch(jOrder,j);
            if (pat->newData[j][iFine]==-1) continue;
            pat->newData[j][iFine]=MIN(absoluteTop,(double)pat->newData[j][iFine]*(top/100.0f));
          }
          j=0;
        }
      }
    }
    iFine=0;
  }

  makeUndo(GUI_UNDO_PATTERN_SCALE);
}

void FurnaceGUI::doRandomize(int bottom, int top, bool mode, bool eff, int effVal) {
  finishSelection();
  prepareUndo(GUI_UNDO_PATTERN_RANDOMIZE);

  int iCoarse=selStart.xCoarse;
  int iFine=selStart.xFine;
  for (; iCoarse<=selEnd.xCoarse; iCoarse++) {
    if (!e->curSubSong->chanShow[iCoarse]) continue;
    for (; iFine<3+e->curPat[iCoarse].effectCols*2 && (iCoarse<selEnd.xCoarse || iFine<=selEnd.xFine); iFine++) {
      int jOrder=selStart.order;
      int j=selStart.y;
      maskOut(opMaskRandomize,iFine);
      resetTouches;
      int absoluteTop=255;
      if (iFine==DIV_PAT_NOTE) {
        absoluteTop=179;
      } else if (iFine==DIV_PAT_INS) {
        if (e->song.ins.empty()) continue;
        absoluteTop=e->song.ins.size()-1;
      } else if (iFine==DIV_PAT_VOL) { // volume
        absoluteTop=e->getMaxVolumeChan(iCoarse);
      }
      for (; jOrder<=selEnd.order; jOrder++) {
        DivPattern* pat=e->curPat[iCoarse].getPattern(e->curOrders->ord[iCoarse][jOrder],true);
        for (; j<e->curSubSong->patLen && (j<=selEnd.y || jOrder<selEnd.order); j++) {
          int value=0;
          int value2=0;
          touch(jOrder,j);
          if (top-bottom<=0) {
            value=MIN(absoluteTop,bottom);
            value2=MIN(absoluteTop,bottom);
          } else {
            // HACK: MIN will call rand() twice....
            int randVal=rand();
            value=MIN(absoluteTop,bottom+(randVal%(top-bottom+1)));
            randVal=rand();
            value2=MIN(absoluteTop,bottom+(randVal%(top-bottom+1)));
          }
          if (mode) {
            value&=15;
            value2&=15;
            pat->newData[j][iFine]=value|(value2<<4);
          } else {
            pat->newData[j][iFine]=value;
          }
          if (eff && iFine>2 && (iFine&1)) {
            pat->newData[j][iFine]=effVal;
          }
        }
        j=0;
      }
    }
    iFine=0;
  }

  makeUndo(GUI_UNDO_PATTERN_RANDOMIZE);
}

struct PatBufferEntry {
  short data[DIV_MAX_COLS];
};

void FurnaceGUI::doFlip() {
  finishSelection();
  prepareUndo(GUI_UNDO_PATTERN_FLIP);

  std::vector<PatBufferEntry> patBuffer;
  int iCoarse=selStart.xCoarse;
  int iFine=selStart.xFine;
  for (; iCoarse<=selEnd.xCoarse; iCoarse++) {
    if (!e->curSubSong->chanShow[iCoarse]) continue;
    patBuffer.clear();

    int jOrder=selStart.order;
    int j=selStart.y;

    // collect pattern
    for (; jOrder<=selEnd.order; jOrder++) {
      DivPattern* pat=e->curPat[iCoarse].getPattern(e->curOrders->ord[iCoarse][jOrder],true);
      for (; j<e->curSubSong->patLen && (j<=selEnd.y || jOrder<selEnd.order); j++) {
        PatBufferEntry put;
        memcpy(put.data,pat->newData[j],DIV_MAX_COLS*sizeof(short));
        patBuffer.push_back(put);
      }
      j=0;
    }

    for (; iFine<3+e->curPat[iCoarse].effectCols*2 && (iCoarse<selEnd.xCoarse || iFine<=selEnd.xFine); iFine++) {
      maskOut(opMaskFlip,iFine);
      resetTouches;
      jOrder=selStart.order;
      j=selStart.y;
      int j_i=patBuffer.size();

      // insert flipped version
      for (; jOrder<=selEnd.order; jOrder++) {
        DivPattern* pat=e->curPat[iCoarse].getPattern(e->curOrders->ord[iCoarse][jOrder],true);
        for (; j<e->curSubSong->patLen && (j<=selEnd.y || jOrder<selEnd.order); j++) {
          j_i--;
          touch(jOrder,j);
          pat->newData[j][iFine]=patBuffer[j_i].data[iFine];
        }
        j=0;
      }
    }
    iFine=0;
  }

  makeUndo(GUI_UNDO_PATTERN_FLIP);
}

void FurnaceGUI::doCollapse(int divider, const SelectionPoint& sStart, const SelectionPoint& sEnd) {
  if (divider<2) return;
  if (e->curSubSong->patLen<divider) {
    showError(_("can't collapse any further!"));
    return;
  }
  if (sStart.order!=sEnd.order) {
    showError(_("can't collapse across orders."));
    return;
  }

  finishSelection();
  prepareUndo(GUI_UNDO_PATTERN_COLLAPSE);

  DivPattern patBuffer;
  int iCoarse=sStart.xCoarse;
  int iFine=sStart.xFine;
  for (; iCoarse<=sEnd.xCoarse; iCoarse++) {
    if (!e->curSubSong->chanShow[iCoarse]) continue;
    DivPattern* pat=e->curPat[iCoarse].getPattern(e->curOrders->ord[iCoarse][curOrder],true);
    for (; iFine<3+e->curPat[iCoarse].effectCols*2 && (iCoarse<sEnd.xCoarse || iFine<=sEnd.xFine); iFine++) {
      maskOut(opMaskCollapseExpand,iFine);
      for (int j=sStart.y; j<=sEnd.y; j++) {
        patBuffer.newData[j][iFine]=pat->newData[j][iFine];
      }
      for (int j=0; j<=sEnd.y-sStart.y; j++) {
        if (j*divider>=sEnd.y-sStart.y) {
          pat->newData[j+sStart.y][iFine]=-1;
        } else {
          pat->newData[j+sStart.y][iFine]=patBuffer.newData[j*divider+sStart.y][iFine];

          for (int k=1; k<divider; k++) {
            if ((j*divider+k)>=sEnd.y-sStart.y) break;
            if (pat->newData[j+sStart.y][iFine]!=-1) break;
            pat->newData[j+sStart.y][iFine]=patBuffer.newData[j*divider+sStart.y+k][iFine];
          }
        }
      }
    }
    iFine=0;
  }

  makeUndo(GUI_UNDO_PATTERN_COLLAPSE);
}

void FurnaceGUI::doExpand(int multiplier, const SelectionPoint& sStart, const SelectionPoint& sEnd) {
  if (multiplier<2) return;
  if (sStart.order!=sEnd.order) {
    showError(_("can't expand across orders."));
    return;
  }

  finishSelection();
  prepareUndo(GUI_UNDO_PATTERN_EXPAND);

  DivPattern patBuffer;
  int iCoarse=sStart.xCoarse;
  int iFine=sStart.xFine;
  for (; iCoarse<=sEnd.xCoarse; iCoarse++) {
    if (!e->curSubSong->chanShow[iCoarse]) continue;
    DivPattern* pat=e->curPat[iCoarse].getPattern(e->curOrders->ord[iCoarse][curOrder],true);
    for (; iFine<3+e->curPat[iCoarse].effectCols*2 && (iCoarse<sEnd.xCoarse || iFine<=sEnd.xFine); iFine++) {
      maskOut(opMaskCollapseExpand,iFine);
      for (int j=sStart.y; j<=sEnd.y; j++) {
        patBuffer.newData[j][iFine]=pat->newData[j][iFine];
      }
      for (int j=0; j<=(sEnd.y-sStart.y)*multiplier; j++) {
        if ((j+sStart.y)>=e->curSubSong->patLen) break;
        if ((j%multiplier)!=0) {
          pat->newData[j+sStart.y][iFine]=-1;
          continue;
        }
        pat->newData[j+sStart.y][iFine]=patBuffer.newData[j/multiplier+sStart.y][iFine];
      }
    }
    iFine=0;
  }

  makeUndo(GUI_UNDO_PATTERN_EXPAND);
}

void FurnaceGUI::doCollapseSong(int divider) {
  if (divider<2) return;
  if (e->curSubSong->patLen<divider) {
    showError(_("can't collapse any further!"));
    return;
  }
  finishSelection();

  UndoStep us;
  us.type=GUI_UNDO_PATTERN_COLLAPSE_SONG;

  DivPattern patCopy;

  size_t subSong=e->getCurrentSubSong();
  for (int i=0; i<e->getTotalChannelCount(); i++) {
    for (int j=0; j<DIV_MAX_PATTERNS; j++) {
      if (e->curPat[i].data[j]==NULL) continue;

      DivPattern* pat=e->curPat[i].getPattern(j,true);
      pat->copyOn(&patCopy);
      pat->clear();
      for (int k=0; k<DIV_MAX_ROWS; k++) {
        for (int l=0; l<DIV_MAX_COLS; l++) {
          if (pat->newData[k/divider][l]!=-1) continue;

          pat->newData[k/divider][l]=patCopy.newData[k][l];

          if (DIV_PAT_IS_EFFECT(l)) { // scale effects as needed
            switch (pat->newData[k/divider][l]) {
              case 0x0d:
                pat->newData[k/divider][l+1]/=divider;
                break;
              case 0x0f:
                pat->newData[k/divider][l+1]=CLAMP(pat->newData[k/divider][l+1]*divider,1,255);
                break;
            }
          }
        }
      }

      // put undo
      for (int k=0; k<DIV_MAX_ROWS; k++) {
        for (int l=0; l<DIV_MAX_COLS; l++) {
          if (pat->newData[k][l]!=patCopy.newData[k][l]) {
            us.pat.push_back(UndoPatternData(subSong,i,j,k,l,patCopy.newData[k][l],pat->newData[k][l]));
          }
        }
      }
    }
  }

  MARK_MODIFIED;

  // magic
  unsigned char* subSongInfoCopy=new unsigned char[1024];
  memcpy(subSongInfoCopy,e->curSubSong,1024);
  e->curSubSong->patLen/=divider;
  for (int i=0; i<e->curSubSong->speeds.len; i++) {
    e->curSubSong->speeds.val[i]=CLAMP(e->curSubSong->speeds.val[i]*divider,1,255);
  }
  unsigned char* newSubSongInfo=(unsigned char*)e->curSubSong;
  for (int i=0; i<1024; i++) {
    if (subSongInfoCopy[i]!=newSubSongInfo[i]) {
      us.other.push_back(UndoOtherData(GUI_UNDO_TARGET_SUBSONG,subSong,i,subSongInfoCopy[i],newSubSongInfo[i]));
    }
  }

  if (!us.pat.empty()) {
    undoHist.push_back(us);
    redoHist.clear();
    if (undoHist.size()>settings.maxUndoSteps) undoHist.pop_front();
  }
  recalcTimestamps=true;
  
  if (e->isPlaying()) e->play();
}

void FurnaceGUI::doExpandSong(int multiplier) {
  if (multiplier<2) return;
  if (e->curSubSong->patLen>(256/multiplier)) {
    showError(_("can't expand any further!"));
    return;
  }
  finishSelection();

  UndoStep us;
  us.type=GUI_UNDO_PATTERN_EXPAND_SONG;

  DivPattern patCopy;

  size_t subSong=e->getCurrentSubSong();
  for (int i=0; i<e->getTotalChannelCount(); i++) {
    for (int j=0; j<DIV_MAX_PATTERNS; j++) {
      if (e->curPat[i].data[j]==NULL) continue;

      DivPattern* pat=e->curPat[i].getPattern(j,true);
      pat->copyOn(&patCopy);
      pat->clear();
      for (int k=0; k<(256/multiplier); k++) {
        for (int l=0; l<DIV_MAX_COLS; l++) {
          if (pat->newData[k*multiplier][l]!=-1) continue;

          pat->newData[k*multiplier][l]=patCopy.newData[k][l];

          if (DIV_PAT_IS_EFFECT(l)) { // scale effects as needed
            switch (pat->newData[k*multiplier][l]) {
              case 0x0d:
                pat->newData[k*multiplier][l+1]/=multiplier;
                break;
              case 0x0f:
                pat->newData[k*multiplier][l+1]=CLAMP(pat->newData[k*multiplier][l+1]/multiplier,1,255);
                break;
            }
          }
        }
      }

      // put undo
      for (int k=0; k<DIV_MAX_ROWS; k++) {
        for (int l=0; l<DIV_MAX_COLS; l++) {
          if (pat->newData[k][l]!=patCopy.newData[k][l]) {
            us.pat.push_back(UndoPatternData(subSong,i,j,k,l,patCopy.newData[k][l],pat->newData[k][l]));
          }
        }
      }
    }
  }

  MARK_MODIFIED;

  // magic
  unsigned char* subSongInfoCopy=new unsigned char[1024];
  memcpy(subSongInfoCopy,e->curSubSong,1024);
  e->curSubSong->patLen*=multiplier;
  for (int i=0; i<e->curSubSong->speeds.len; i++) {
    e->curSubSong->speeds.val[i]=CLAMP(e->curSubSong->speeds.val[i]/multiplier,1,255);
  }
  unsigned char* newSubSongInfo=(unsigned char*)e->curSubSong;
  for (int i=0; i<1024; i++) {
    if (subSongInfoCopy[i]!=newSubSongInfo[i]) {
      us.other.push_back(UndoOtherData(GUI_UNDO_TARGET_SUBSONG,subSong,i,subSongInfoCopy[i],newSubSongInfo[i]));
    }
  }

  if (!us.pat.empty()) {
    undoHist.push_back(us);
    redoHist.clear();
    if (undoHist.size()>settings.maxUndoSteps) undoHist.pop_front();
  }
  recalcTimestamps=true;

  if (e->isPlaying()) e->play();
}

void FurnaceGUI::doAbsorbInstrument() {
  bool foundIns=false;
  bool foundOctave=false;
  auto foundAll = [&]() { return foundIns && foundOctave; };

  // search this order and all prior until we find all the data we need
  int orderIdx=curOrder;
  for (; orderIdx>=0 && !foundAll(); orderIdx--) {
    DivPattern* pat=e->curPat[cursor.xCoarse].getPattern(e->curOrders->ord[cursor.xCoarse][orderIdx],false);
    if (!pat) continue;

    // start on current row when searching current order, but start from end when searching
    // prior orders.
    int searchStartRow=orderIdx==curOrder ? cursor.y : e->curSubSong->patLen-1;
    for (int i=searchStartRow; i>=0 && !foundAll(); i--) {

      // absorb most recent instrument
      if (!foundIns && pat->newData[i][DIV_PAT_INS] >= 0) {
        foundIns=true;
        setCurIns(pat->newData[i][DIV_PAT_INS]);
      }

      // absorb most recent octave (i.e. set curOctave such that the "main row" (QWERTY) of
      // notes will result in an octave number equal to the previous note). make sure to
      // skip "special note values" like OFF/REL/=== and "none", since there won't be valid
      // octave values
      short note=pat->newData[i][DIV_PAT_NOTE];
      if (!foundOctave && note!=-1 && note!=DIV_NOTE_OFF && note!=DIV_NOTE_REL && note!=DIV_MACRO_REL) {
        foundOctave=true;

        // decode octave data
        int octave=(pat->newData[i][DIV_PAT_NOTE]-60)/12;

        curOctave=CLAMP(octave-1,GUI_EDIT_OCTAVE_MIN,GUI_EDIT_OCTAVE_MAX);
      }
    }
  }

  // if no instrument has been set at this point, the only way to match it is to use "none"
  if (!foundIns) setCurIns(-1);

  logD("doAbsorbInstrument -- searched %d orders", curOrder-orderIdx);
}

void FurnaceGUI::doDrag(bool copy) {
  int len=dragEnd.xCoarse-dragStart.xCoarse+1;
  int firstOrder=e->curSubSong->ordersLen;
  int lastOrder=0;

  if (dragStart.order<firstOrder) firstOrder=dragStart.order;
  if (dragEnd.order<firstOrder) firstOrder=dragEnd.order;
  if (selStart.order<firstOrder) firstOrder=selStart.order;
  if (dragStart.order>lastOrder) lastOrder=dragStart.order;
  if (dragEnd.order>lastOrder) lastOrder=dragEnd.order;
  if (selStart.order>lastOrder) lastOrder=selStart.order;

  logV("UR: %d - %d",firstOrder,lastOrder);

  if (len<1) return;
  
  prepareUndo(GUI_UNDO_PATTERN_DRAG,UndoRegion(firstOrder,0,0,lastOrder,e->getTotalChannelCount()-1,e->curSubSong->patLen-1));

  // copy and clear (if copy is false)
  String c=doCopy(!copy,false,dragStart,dragEnd);

  logV(_("copy: %s"),c);

  // replace
  cursor=selStart;
  doPaste(GUI_PASTE_MODE_OVERFLOW,0,false,c);
  updateScroll(cursor.y);

  makeUndo(GUI_UNDO_PATTERN_DRAG,UndoRegion(firstOrder,0,0,lastOrder,e->getTotalChannelCount()-1,e->curSubSong->patLen-1));
}

void FurnaceGUI::moveSelected(int x, int y) {
  SelectionPoint selStartOld, selEndOld, selStartNew, selEndNew;
  finishSelection();

  selStartOld=selStart;
  selEndOld=selEnd;

  // move selection
  DETERMINE_FIRST_LAST;

  bool outOfBounds=false;

  if (x>0) {
    for (int i=0; i<x; i++) {
      do {
        selStart.xCoarse++;
        if (selStart.xCoarse>=lastChannel) {
          outOfBounds=true;
          break;
        }
      } while (!e->curSubSong->chanShow[selStart.xCoarse]);

      do {
        selEnd.xCoarse++;
        if (selEnd.xCoarse>=lastChannel) {
          outOfBounds=true;
          break;
        }
      } while (!e->curSubSong->chanShow[selEnd.xCoarse]);

      if (outOfBounds) break;
    }
  } else if (x<0) {
    for (int i=0; i<-x; i++) {
      do {
        selStart.xCoarse--;
        if (selStart.xCoarse<firstChannel) {
          outOfBounds=true;
          break;
        }
      } while (!e->curSubSong->chanShow[selStart.xCoarse]);

      do {
        selEnd.xCoarse--;
        if (selEnd.xCoarse<firstChannel) {
          outOfBounds=true;
          break;
        }
      } while (!e->curSubSong->chanShow[selEnd.xCoarse]);

      if (outOfBounds) break;
    }
  }

  selStart.y+=y;
  selEnd.y+=y;

  while (selStart.y<0) {
    selStart.y+=e->curSubSong->patLen;
    selStart.order--;
  }
  while (selEnd.y<0) {
    selEnd.y+=e->curSubSong->patLen;
    selEnd.order--;
  }

  while (selStart.y>=e->curSubSong->patLen) {
    selStart.y-=e->curSubSong->patLen;
    selStart.order++;
  }
  while (selEnd.y>=e->curSubSong->patLen) {
    selEnd.y-=e->curSubSong->patLen;
    selEnd.order++;
  }

  if (selStart.order<0 || selStart.order>=e->curSubSong->ordersLen) outOfBounds=true;
  if (selEnd.order<0 || selEnd.order>=e->curSubSong->ordersLen) outOfBounds=true;

  selStartNew=selStart;
  selEndNew=selEnd;

  selStart=selStartOld;
  selEnd=selEndOld;

  if (outOfBounds) {
    return;
  }

  int firstOrder=e->curSubSong->ordersLen;
  int lastOrder=0;

  if (selStartNew.order<firstOrder) firstOrder=selStartNew.order;
  if (selEndNew.order<firstOrder) firstOrder=selEndNew.order;
  if (selStart.order<firstOrder) firstOrder=selStart.order;
  if (selStartNew.order>lastOrder) lastOrder=selStartNew.order;
  if (selEndNew.order>lastOrder) lastOrder=selEndNew.order;
  if (selStart.order>lastOrder) lastOrder=selStart.order;

  logV("UR: %d - %d",firstOrder,lastOrder);

  prepareUndo(GUI_UNDO_PATTERN_DRAG,UndoRegion(firstOrder,0,0,lastOrder,e->getTotalChannelCount()-1,e->curSubSong->patLen-1));

  // copy and clear
  String c=doCopy(true,false,selStart,selEnd);

  logV(_("copy: %s"),c);

  // move
  selStart=selStartNew;
  selEnd=selEndNew;

  // replace
  cursor=selStart;
  doPaste(GUI_PASTE_MODE_OVERFLOW,0,false,c);
  recalcTimestamps=true;

  makeUndo(GUI_UNDO_PATTERN_DRAG,UndoRegion(firstOrder,0,0,lastOrder,e->getTotalChannelCount()-1,e->curSubSong->patLen-1));
}

void FurnaceGUI::doUndo() {
  if (undoHist.empty()) return;
  UndoStep& us=undoHist.back();
  redoHist.push_back(us);
  MARK_MODIFIED;

  switch (us.type) {
    case GUI_UNDO_CHANGE_ORDER:
      e->curSubSong->ordersLen=us.oldOrdersLen;
      for (UndoOrderData& i: us.ord) {
        e->changeSongP(i.subSong);
        e->curOrders->ord[i.chan][i.ord]=i.oldVal;
      }
      break;
    case GUI_UNDO_PATTERN_EDIT:
    case GUI_UNDO_PATTERN_DELETE:
    case GUI_UNDO_PATTERN_PULL:
    case GUI_UNDO_PATTERN_PUSH:
    case GUI_UNDO_PATTERN_CUT:
    case GUI_UNDO_PATTERN_PASTE:
    case GUI_UNDO_PATTERN_CHANGE_INS:
    case GUI_UNDO_PATTERN_INTERPOLATE:
    case GUI_UNDO_PATTERN_FADE:
    case GUI_UNDO_PATTERN_SCALE:
    case GUI_UNDO_PATTERN_RANDOMIZE:
    case GUI_UNDO_PATTERN_INVERT_VAL:
    case GUI_UNDO_PATTERN_FLIP:
    case GUI_UNDO_PATTERN_COLLAPSE:
    case GUI_UNDO_PATTERN_EXPAND:
    case GUI_UNDO_PATTERN_COLLAPSE_SONG:
    case GUI_UNDO_PATTERN_EXPAND_SONG:
    case GUI_UNDO_PATTERN_DRAG:
    case GUI_UNDO_REPLACE:
      for (UndoPatternData& i: us.pat) {
        e->changeSongP(i.subSong);
        DivPattern* p=e->curPat[i.chan].getPattern(i.pat,true);
        p->newData[i.row][i.col]=i.oldVal;
      }
      if (us.type!=GUI_UNDO_REPLACE) {
        if (!e->isPlaying() || !followPattern) {
          cursor=us.oldCursor;
          selStart=us.oldSelStart;
          selEnd=us.oldSelEnd;
          curNibble=us.nibble;
          setOrder(us.oldOrder);
          if (us.oldScroll>=0.0f) {
            updateScrollRaw(us.oldScroll);
          }
        }
      }
      break;
  }

  recalcTimestamps=true;

  bool shallReplay=false;
  for (UndoOtherData& i: us.other) {
    switch (i.target) {
      case GUI_UNDO_TARGET_SONG:
        ((unsigned char*)(&e->song))[i.off]=i.oldVal;
        shallReplay=true;
        break;
      case GUI_UNDO_TARGET_SUBSONG:
        if (i.subtarget<0 || i.subtarget>=(int)e->song.subsong.size()) break;
        ((unsigned char*)(e->song.subsong[i.subtarget]))[i.off]=i.oldVal;
        shallReplay=true;
        break;
    }
  }
  if (shallReplay && e->isPlaying()) play();

  if (curOrder>=e->curSubSong->ordersLen) {
    curOrder=e->curSubSong->ordersLen-1;
    e->setOrder(curOrder);
  }

  undoHist.pop_back();
}

void FurnaceGUI::doRedo() {
  if (redoHist.empty()) return;
  UndoStep& us=redoHist.back();
  undoHist.push_back(us);
  MARK_MODIFIED;

  switch (us.type) {
    case GUI_UNDO_CHANGE_ORDER:
      e->curSubSong->ordersLen=us.newOrdersLen;
      for (UndoOrderData& i: us.ord) {
        e->changeSongP(i.subSong);
        e->curOrders->ord[i.chan][i.ord]=i.newVal;
      }
      break;
    case GUI_UNDO_PATTERN_EDIT:
    case GUI_UNDO_PATTERN_DELETE:
    case GUI_UNDO_PATTERN_PULL:
    case GUI_UNDO_PATTERN_PUSH:
    case GUI_UNDO_PATTERN_CUT:
    case GUI_UNDO_PATTERN_PASTE:
    case GUI_UNDO_PATTERN_CHANGE_INS:
    case GUI_UNDO_PATTERN_INTERPOLATE:
    case GUI_UNDO_PATTERN_FADE:
    case GUI_UNDO_PATTERN_SCALE:
    case GUI_UNDO_PATTERN_RANDOMIZE:
    case GUI_UNDO_PATTERN_INVERT_VAL:
    case GUI_UNDO_PATTERN_FLIP:
    case GUI_UNDO_PATTERN_COLLAPSE:
    case GUI_UNDO_PATTERN_EXPAND:
    case GUI_UNDO_PATTERN_DRAG:
    case GUI_UNDO_PATTERN_COLLAPSE_SONG:
    case GUI_UNDO_PATTERN_EXPAND_SONG:
    case GUI_UNDO_REPLACE:
      for (UndoPatternData& i: us.pat) {
        e->changeSongP(i.subSong);
        DivPattern* p=e->curPat[i.chan].getPattern(i.pat,true);
        p->newData[i.row][i.col]=i.newVal;
      }
      if (us.type!=GUI_UNDO_REPLACE) {
        if (!e->isPlaying() || !followPattern) {
          cursor=us.newCursor;
          selStart=us.newSelStart;
          selEnd=us.newSelEnd;
          curNibble=us.nibble;
          setOrder(us.newOrder);
          if (us.newScroll>=0.0f) {
            updateScrollRaw(us.newScroll);
          }
        }
      }
      break;
  }

  recalcTimestamps=true;

  bool shallReplay=false;
  for (UndoOtherData& i: us.other) {
    switch (i.target) {
      case GUI_UNDO_TARGET_SONG:
        ((unsigned char*)(&e->song))[i.off]=i.newVal;
        shallReplay=true;
        break;
      case GUI_UNDO_TARGET_SUBSONG:
        if (i.subtarget<0 || i.subtarget>=(int)e->song.subsong.size()) break;
        ((unsigned char*)(e->song.subsong[i.subtarget]))[i.off]=i.newVal;
        shallReplay=true;
        break;
    }
  }
  if (shallReplay && e->isPlaying()) play();

  if (curOrder>=e->curSubSong->ordersLen) {
    curOrder=e->curSubSong->ordersLen-1;
    e->setOrder(curOrder);
  }

  redoHist.pop_back();
}

CursorJumpPoint FurnaceGUI::getCurrentCursorJumpPoint() {
  return CursorJumpPoint(cursor, curOrder, e->getCurrentSubSong());
}

void FurnaceGUI::applyCursorJumpPoint(const CursorJumpPoint& spot) {
  cursor=spot.point;
  curOrder=MIN(e->curSubSong->ordersLen-1, spot.order);
  e->setOrder(curOrder);
  e->changeSongP(spot.subSong);
  if (!settings.cursorMoveNoScroll) {
    updateScroll(cursor.y);
  }
}

void FurnaceGUI::makeCursorUndo() {
  CursorJumpPoint spot = getCurrentCursorJumpPoint();
  if (!cursorUndoHist.empty() && spot == cursorUndoHist.back()) return;
  
  if (cursorUndoHist.size()>=settings.maxUndoSteps) cursorUndoHist.pop_front();
  cursorUndoHist.push_back(spot);

  // redo history no longer relevant, we've changed timeline
  cursorRedoHist.clear();
}

void FurnaceGUI::doCursorUndo() {
  if (cursorUndoHist.empty()) return;

  // allow returning to current spot
  if (cursorRedoHist.size()>=settings.maxUndoSteps) cursorRedoHist.pop_front();
  cursorRedoHist.push_back(getCurrentCursorJumpPoint());

  // apply spot
  applyCursorJumpPoint(cursorUndoHist.back());
  cursorUndoHist.pop_back();
}

void FurnaceGUI::doCursorRedo() {
if (cursorRedoHist.empty()) return;

  // allow returning to current spot
  if (cursorUndoHist.size()>=settings.maxUndoSteps) cursorUndoHist.pop_front();
  cursorUndoHist.push_back(getCurrentCursorJumpPoint());

  // apply spot
  applyCursorJumpPoint(cursorRedoHist.back());
  cursorRedoHist.pop_back();
}
