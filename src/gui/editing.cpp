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

const char* FurnaceGUI::noteNameNormal(short note, short octave) {
  if (note==100) { // note cut
    return "OFF";
  } else if (note==101) { // note off and envelope release
    return "===";
  } else if (note==102) { // envelope release only
    return "REL";
  } else if (octave==0 && note==0) {
    return "...";
  }
  int seek=(note+(signed char)octave*12)+60;
  if (seek<0 || seek>=180) {
    return "???";
  }
  return noteNames[seek];
}

void FurnaceGUI::prepareUndo(ActionType action, UndoRegion region) {
  if (region.begin.ord==-1) {
    region.begin.ord=curOrder;
    region.end.ord=curOrder;
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
  bool shallWalk=false;
  UndoStep s;
  s.type=action;
  s.cursor=cursor;
  s.selStart=selStart;
  s.selEnd=selEnd;
  s.order=curOrder;
  s.oldOrdersLen=oldOrdersLen;
  s.newOrdersLen=e->curSubSong->ordersLen;
  s.nibble=curNibble;
  size_t subSong=e->getCurrentSubSong();

  if (region.begin.ord==-1) {
    region.begin.ord=curOrder;
    region.end.ord=curOrder;
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
              if (p->data[j][k]!=op->data[j][k]) {
                s.pat.push_back(UndoPatternData(subSong,i,e->curOrders->ord[i][h],j,k,op->data[j][k],p->data[j][k]));

                if (k>=4) {
                  if (op->data[j][k&(~1)]==0x0b ||
                      p->data[j][k&(~1)]==0x0b ||
                      op->data[j][k&(~1)]==0x0d ||
                      p->data[j][k&(~1)]==0x0d ||
                      op->data[j][k&(~1)]==0xff ||
                      p->data[j][k&(~1)]==0xff) {
                    shallWalk=true;
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
  if (shallWalk) {
    e->walkSong(loopOrder,loopRow,loopEnd);
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
    // find row position
    for (SelectionPoint i; i.xCoarse!=selStart.xCoarse || i.xFine!=selStart.xFine; selStartX++) {
      i.xFine++;
      if (i.xFine>=3+e->curPat[i.xCoarse].effectCols*2) {
        i.xFine=0;
        i.xCoarse++;
      }
    }
    for (SelectionPoint i; i.xCoarse!=selEnd.xCoarse || i.xFine!=selEnd.xFine; selEndX++) {
      i.xFine++;
      if (i.xFine>=3+e->curPat[i.xCoarse].effectCols*2) {
        i.xFine=0;
        i.xCoarse++;
      }
    }

    float aspect=float(selEndX-selStartX+1)/float(selEnd.y-selStart.y+1);
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

void FurnaceGUI::doDelete() {
  finishSelection();
  prepareUndo(GUI_UNDO_PATTERN_DELETE);
  curNibble=false;

  int iCoarse=selStart.xCoarse;
  int iFine=selStart.xFine;
  for (; iCoarse<=selEnd.xCoarse; iCoarse++) {
    if (!e->curSubSong->chanShow[iCoarse]) continue;
    DivPattern* pat=e->curPat[iCoarse].getPattern(e->curOrders->ord[iCoarse][curOrder],true);
    for (; iFine<3+e->curPat[iCoarse].effectCols*2 && (iCoarse<selEnd.xCoarse || iFine<=selEnd.xFine); iFine++) {
      maskOut(opMaskDelete,iFine);
      for (int j=selStart.y; j<=selEnd.y; j++) {
        if (iFine==0) {
          pat->data[j][iFine]=0;
          if (selStart.y==selEnd.y) pat->data[j][2]=-1;
        }
        pat->data[j][iFine+1]=(iFine<1)?0:-1;

        if (selStart.y==selEnd.y && iFine>2 && iFine&1 && settings.effectDeletionAltersValue) {
          pat->data[j][iFine+2]=-1;
        }
      }
    }
    iFine=0;
  }

  makeUndo(GUI_UNDO_PATTERN_DELETE);
}

void FurnaceGUI::doPullDelete() {
  finishSelection();
  prepareUndo(GUI_UNDO_PATTERN_PULL);
  curNibble=false;

  if (settings.pullDeleteBehavior) {
    if (--selStart.y<0) selStart.y=0;
    if (--selEnd.y<0) selEnd.y=0;
    if (--cursor.y<0) cursor.y=0;
    updateScroll(cursor.y);
  }

  SelectionPoint sStart=selStart;
  SelectionPoint sEnd=selEnd;

  if (selStart.xCoarse==selEnd.xCoarse && selStart.xFine==selEnd.xFine && selStart.y==selEnd.y && settings.pullDeleteRow) {
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
        if (j<e->curSubSong->patLen-1) {
          if (iFine==0) {
            pat->data[j][iFine]=pat->data[j+1][iFine];
          }
          pat->data[j][iFine+1]=pat->data[j+1][iFine+1];
        } else {
          if (iFine==0) {
            pat->data[j][iFine]=0;
          }
          pat->data[j][iFine+1]=(iFine<1)?0:-1;
        }
      }
    }
    iFine=0;
  }

  makeUndo(GUI_UNDO_PATTERN_PULL);
}

void FurnaceGUI::doInsert() {
  finishSelection();
  prepareUndo(GUI_UNDO_PATTERN_PUSH);
  curNibble=false;

  SelectionPoint sStart=selStart;
  SelectionPoint sEnd=selEnd;

  if (selStart.xCoarse==selEnd.xCoarse && selStart.xFine==selEnd.xFine && selStart.y==selEnd.y && settings.insertBehavior) {
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
          if (iFine==0) {
            pat->data[j][iFine]=0;
          }
          pat->data[j][iFine+1]=(iFine<1)?0:-1;
        } else {
          if (iFine==0) {
            pat->data[j][iFine]=pat->data[j-1][iFine];
          }
          pat->data[j][iFine+1]=pat->data[j-1][iFine+1];
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
    DivPattern* pat=e->curPat[iCoarse].getPattern(e->curOrders->ord[iCoarse][curOrder],true);
    for (; iFine<3+e->curPat[iCoarse].effectCols*2 && (iCoarse<selEnd.xCoarse || iFine<=selEnd.xFine); iFine++) {
      maskOut(mask,iFine);
      for (int j=selStart.y; j<=selEnd.y; j++) {
        if (iFine==0) {
          int origNote=pat->data[j][0];
          int origOctave=(signed char)pat->data[j][1];
          if (origNote!=0 && origNote!=100 && origNote!=101 && origNote!=102) {
            origNote+=amount;
            while (origNote>12) {
              origNote-=12;
              origOctave++;
            }
            while (origNote<1) {
              origNote+=12;
              origOctave--;
            }
            if (origOctave==9 && origNote>11) {
              origNote=11;
              origOctave=9;
            } 
            if (origOctave>9) {
              origNote=11;
              origOctave=9;
            }
            if (origOctave<-5) {
              origNote=1;
              origOctave=-5;
            }
            pat->data[j][0]=origNote;
            pat->data[j][1]=(unsigned char)origOctave;
          }
        } else {
          int top=255;
          if (iFine==1) {
            if (e->song.ins.empty()) continue;
            top=e->song.ins.size()-1;
          } else if (iFine==2) { // volume
            top=e->getMaxVolumeChan(iCoarse);
          }
          if (pat->data[j][iFine+1]==-1) continue;
          pat->data[j][iFine+1]=MIN(top,MAX(0,pat->data[j][iFine+1]+amount));
        }
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

  for (int j=sStart.y; j<=sEnd.y; j++) {
    int iCoarse=sStart.xCoarse;
    int iFine=sStart.xFine;
    if (iFine>3 && !(iFine&1)) {
      iFine--;
    }
    clipb+='\n';
    for (; iCoarse<=sEnd.xCoarse; iCoarse++) {
      if (!e->curSubSong->chanShow[iCoarse]) continue;
      DivPattern* pat=e->curPat[iCoarse].getPattern(e->curOrders->ord[iCoarse][curOrder],true);
      for (; iFine<3+e->curPat[iCoarse].effectCols*2 && (iCoarse<sEnd.xCoarse || iFine<=sEnd.xFine); iFine++) {
        if (iFine==0) {
          clipb+=noteNameNormal(pat->data[j][0],pat->data[j][1]);
          if (cut) {
            pat->data[j][0]=0;
            pat->data[j][1]=0;
          }
        } else {
          if (pat->data[j][iFine+1]==-1) {
            clipb+="..";
          } else {
            clipb+=fmt::sprintf("%.2X",pat->data[j][iFine+1]);
          }
          if (cut) {
            pat->data[j][iFine+1]=-1;
          }
        }
      }
      clipb+='|';
      iFine=0;
    }
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
  char note[4];
  for (size_t i=2; i<data.size() && j<e->curSubSong->patLen; i++) {
    size_t charPos=0;
    int iCoarse=cursor.xCoarse;
    int iFine=(startOff>2 && cursor.xFine>2)?(((cursor.xFine-1)&(~1))|1):startOff;

    String& line=data[i];

    while (charPos<line.size() && iCoarse<lastChannel) {
      DivPattern* pat=e->curPat[iCoarse].getPattern(e->curOrders->ord[iCoarse][curOrder],true);
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
          if (!(mode==GUI_PASTE_MODE_MIX_BG || mode==GUI_PASTE_MODE_INS_BG) || (pat->data[j][0]==0 && pat->data[j][1]==0)) {
            if (!decodeNote(note,pat->data[j][0],pat->data[j][1])) {
              invalidData=true;
              break;
            }
            if (mode==GUI_PASTE_MODE_INS_BG || mode==GUI_PASTE_MODE_INS_FG) pat->data[j][2]=arg;
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

        if (iFine==1) {
          if (!opMaskPaste.ins || mode==GUI_PASTE_MODE_INS_BG || mode==GUI_PASTE_MODE_INS_FG) {
            iFine++;
            continue;
          }
        } else if (iFine==2) {
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
            pat->data[j][iFine+1]=-1;
          }
        } else {
          unsigned int val=0;
          if (sscanf(note,"%2X",&val)!=1) {
            invalidData=true;
            break;
          }
          if (!(mode==GUI_PASTE_MODE_MIX_BG || mode==GUI_PASTE_MODE_INS_BG) || pat->data[j][iFine+1]==-1) {
            if (iFine<(3+e->curPat[iCoarse].effectCols*2)) pat->data[j][iFine+1]=val;
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
    if (mode==GUI_PASTE_MODE_OVERFLOW && j>=e->curSubSong->patLen && curOrder<e->curSubSong->ordersLen-1) {
      j=0;
      curOrder++;
    }

    if (mode==GUI_PASTE_MODE_FLOOD && i==data.size()-1) {
      i=1;
    }
  }

  if (readClipboard) {
    if (settings.cursorPastePos) {
      cursor.y=j;
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

// TODO: fix code style
void FurnaceGUI::doPasteMPT(PasteMode mode, int arg, bool readClipboard, String clipb, std::vector<String> data, int mptFormat, UndoRegion ur)
{
  DETERMINE_LAST;

  int j=cursor.y;
  char note[4];
  bool invalidData=false;

  memset(note,0,4);

  for(size_t i=1; i<data.size() && j<e->curSubSong->patLen; i++)
  {
    size_t charPos=1;
    int iCoarse=cursor.xCoarse;
    int iFine=0;

    String& line=data[i];

    while (charPos<line.size() && iCoarse<lastChannel)
    {
      DivPattern* pat=e->curPat[iCoarse].getPattern(e->curOrders->ord[iCoarse][curOrder],true);
      if (line[charPos]=='|' && charPos != 0) // MPT format starts every pattern line with '|'
      {
        iCoarse++;

        if (iCoarse<lastChannel) while (!e->curSubSong->chanShow[iCoarse])
        {
          iCoarse++;
          if (iCoarse>=lastChannel) break;
        }

        iFine=0;
        charPos++;
        continue;
      }
      if (iFine==0) // note
      {
        if (charPos>=line.size())
        {
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

        if (strcmp(note,"...")==0 || strcmp(note,"   ")==0)
        {
          // do nothing.
        } 
        
        else 
        {
          if (!(mode==GUI_PASTE_MODE_MIX_BG || mode==GUI_PASTE_MODE_INS_BG) || (pat->data[j][0]==0 && pat->data[j][1]==0)) 
          {
            if (!decodeNote(note,pat->data[j][0],pat->data[j][1]))
            {
              if(strcmp(note, "^^^") == 0)
              {
                pat->data[j][0]=100;
                pat->data[j][1]=0;
              }
              else if(strcmp(note, "~~~") == 0 || strcmp(note, "===") == 0)
              {
                pat->data[j][0]=101;
                pat->data[j][1]=0;
              }
              else
              {
                invalidData=true;
              }
              
              break;
            }
            else
            {
              pat->data[j][1]--; // MPT is one octave higher...
            }

            if (mode==GUI_PASTE_MODE_INS_BG || mode==GUI_PASTE_MODE_INS_FG) pat->data[j][2]=arg;
          }
        }
      } 
      else if (iFine==1) // instrument
      {
        if (charPos>=line.size())
        {
          invalidData=true;
          break;
        }
        note[0]=line[charPos++];
        if (charPos>=line.size())
        {
          invalidData=true;
          break;
        }
        note[1]=line[charPos++];
        note[2]=0;

        if (iFine==1)
        {
          if (!opMaskPaste.ins || mode==GUI_PASTE_MODE_INS_BG || mode==GUI_PASTE_MODE_INS_FG)
          {
            iFine++;
            continue;
          }
        }

        if (strcmp(note,"..")==0 || strcmp(note,"  ")==0)
        {
          if (!(mode==GUI_PASTE_MODE_MIX_BG || mode==GUI_PASTE_MODE_MIX_FG ||
                mode==GUI_PASTE_MODE_INS_BG || mode==GUI_PASTE_MODE_INS_FG))
          {
            pat->data[j][iFine+1]=-1;
          }
        } 
        else 
        {
          unsigned int val=0;
          if (sscanf(note,"%2d",&val)!=1) 
          {
            invalidData=true;
            break;
          }

          if (!(mode==GUI_PASTE_MODE_MIX_BG || mode==GUI_PASTE_MODE_INS_BG) || pat->data[j][iFine+1]==-1) 
          {
            pat->data[j][iFine+1]=val - 1;
          }
        }
      }
      else
      { // volume and effects
        if (charPos>=line.size())
        {
          invalidData=true;
          break;
        }
        note[0]=line[charPos++];
        if (charPos>=line.size())
        {
          invalidData=true;
          break;
        }
        note[1]=line[charPos++];
        if (charPos>=line.size())
        {
          invalidData=true;
          break;
        }
        note[2]=line[charPos++];
        note[3]=0;

        if (iFine==2)
        {
          if (!opMaskPaste.vol)
          {
            iFine++;
            continue;
          }
        } 
        
        else if ((iFine&1)==0) 
        {
          if (!opMaskPaste.effectVal) 
          {
            iFine++;
            continue;
          }
        } 
        else if ((iFine&1)==1) 
        {
          if (!opMaskPaste.effect) 
          {
            iFine++;
            continue;
          }
        }

        if (strcmp(note,"...")==0 || strcmp(note,"   ")==0)
        {
          if (!(mode==GUI_PASTE_MODE_MIX_BG || mode==GUI_PASTE_MODE_MIX_FG ||
                mode==GUI_PASTE_MODE_INS_BG || mode==GUI_PASTE_MODE_INS_FG))
          {
            pat->data[j][iFine+1]=-1;
          }
        } 
        else 
        {
          unsigned int val=0;
          unsigned char symbol = '\0';

          symbol = note[0];

          if(iFine == 2)
          {
            sscanf(&note[1],"%2d",&val);
          }
          else
          {
            sscanf(&note[1],"%2X",&val);
          }

          if (!(mode==GUI_PASTE_MODE_MIX_BG || mode==GUI_PASTE_MODE_INS_BG) || pat->data[j][iFine+1]==-1) 
          {
            // if (iFine<(3+e->curPat[iCoarse].effectCols*2)) pat->data[j][iFine+1]=val;
            if(iFine == 2) // volume
            {
              switch(symbol)
              {
                case 'v':
                {
                  pat->data[j][iFine+1]=val;
                  break;
                }
                
                default:
                  break;
              }
            }
            else // effect
            {
              unsigned int eff = 0;
              
              if(mptFormat == 0)
              {
                eff = convertEffectMPT_MOD(symbol, val); // up to 4 effects stored in one variable

                if(((eff & 0x0f00) >> 8) == 0x0C) // set volume
                {
                  pat->data[j][iFine]=eff & 0xff;
                }
              }

              if(mptFormat == 1)
              {
                eff = convertEffectMPT_S3M(symbol, val);
              }

              if(mptFormat == 2 || mptFormat == 3) // set volume
              {
                eff = convertEffectMPT_XM(symbol, val);
                
                if(((eff & 0x0f00) >> 8) == 0x0C)
                {
                  pat->data[j][iFine]=eff & 0xff;
                }
              }

              if(mptFormat == 4 || mptFormat == 5)
              {
                eff = convertEffectMPT_IT(symbol, val);
              }

              if(mptFormat == 6)
              {
                eff = convertEffectMPT_MPTM(symbol, val);
              }

              pat->data[j][iFine+1]=((eff & 0xff00) >> 8);
              pat->data[j][iFine+2]=(eff & 0xff);

              if(eff > 0xffff)
              {
                pat->data[j][iFine+3]=((eff & 0xff000000) >> 24);
                pat->data[j][iFine+4]=((eff & 0xff0000) >> 16);
              }
              
            }
          }
        }
      }

      iFine++;

      if(charPos >= line.size() - 1)
      {
        invalidData = false;
        break;
      }
    }
    
    if (invalidData)
    {
      logW(_("invalid clipboard data! failed at line %d char %d"),i,charPos);
      logW("%s",line.c_str());
      break;
    }

    j++;
    if (mode==GUI_PASTE_MODE_OVERFLOW && j>=e->curSubSong->patLen && curOrder<e->curSubSong->ordersLen-1)
    {
      j=0;
      curOrder++;
    }

    if (mode==GUI_PASTE_MODE_FLOOD && i==data.size()-1)
    {
      i=1;
    }
  }

  if (readClipboard) {
    if (settings.cursorPastePos) {
      cursor.y=j;
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
    int firstPattern=curOrder;
    int lastPattern=curOrder;
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
    if (!e->curSubSong->chanShow[iCoarse]) continue;
    DivPattern* pat=e->curPat[iCoarse].getPattern(e->curOrders->ord[iCoarse][curOrder],true);
    for (int j=selStart.y; j<=selEnd.y; j++) {
      if (pat->data[j][2]!=-1 || !((pat->data[j][0]==0 || pat->data[j][0]==100 || pat->data[j][0]==101 || pat->data[j][0]==102) && pat->data[j][1]==0)) {
        pat->data[j][2]=ins;
      }
    }
  }

  makeUndo(GUI_UNDO_PATTERN_CHANGE_INS);
}

void FurnaceGUI::doInterpolate() {
  finishSelection();
  prepareUndo(GUI_UNDO_PATTERN_INTERPOLATE);

  std::vector<std::pair<int,int>> points;
  int iCoarse=selStart.xCoarse;
  int iFine=selStart.xFine;
  for (; iCoarse<=selEnd.xCoarse; iCoarse++) {
    if (!e->curSubSong->chanShow[iCoarse]) continue;
    DivPattern* pat=e->curPat[iCoarse].getPattern(e->curOrders->ord[iCoarse][curOrder],true);
    for (; iFine<3+e->curPat[iCoarse].effectCols*2 && (iCoarse<selEnd.xCoarse || iFine<=selEnd.xFine); iFine++) {
      maskOut(opMaskInterpolate,iFine);
      points.clear();
      if (iFine!=0) {
        for (int j=selStart.y; j<=selEnd.y; j++) {
          if (pat->data[j][iFine+1]!=-1) {
            points.emplace(points.end(),j,pat->data[j][iFine+1]);
          }
        }

        if (points.size()>1) for (size_t j=0; j<points.size()-1; j++) {
          std::pair<int,int>& curPoint=points[j];
          std::pair<int,int>& nextPoint=points[j+1];
          double distance=nextPoint.first-curPoint.first;
          for (int k=0; k<(nextPoint.first-curPoint.first); k++) {
            pat->data[k+curPoint.first][iFine+1]=curPoint.second+((nextPoint.second-curPoint.second)*(double)k/distance);
          }
        }
      } else {
        for (int j=selStart.y; j<=selEnd.y; j++) {
          if (pat->data[j][0]!=0 || pat->data[j][1]!=0) {
            if (pat->data[j][0]!=100 && pat->data[j][0]!=101 && pat->data[j][0]!=102) {
              points.emplace(points.end(),j,pat->data[j][0]+(signed char)pat->data[j][1]*12);
            }
          }
        }

        if (points.size()>1) for (size_t j=0; j<points.size()-1; j++) {
          std::pair<int,int>& curPoint=points[j];
          std::pair<int,int>& nextPoint=points[j+1];
          double distance=nextPoint.first-curPoint.first;
          for (int k=0; k<(nextPoint.first-curPoint.first); k++) {
            int val=curPoint.second+((nextPoint.second-curPoint.second)*(double)k/distance);
            pat->data[k+curPoint.first][0]=val%12;
            pat->data[k+curPoint.first][1]=val/12;
            if (pat->data[k+curPoint.first][0]==0) {
              pat->data[k+curPoint.first][0]=12;
              pat->data[k+curPoint.first][1]--;
            }
            pat->data[k+curPoint.first][1]&=255;
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
  for (; iCoarse<=selEnd.xCoarse; iCoarse++) {
    if (!e->curSubSong->chanShow[iCoarse]) continue;
    DivPattern* pat=e->curPat[iCoarse].getPattern(e->curOrders->ord[iCoarse][curOrder],true);
    for (; iFine<3+e->curPat[iCoarse].effectCols*2 && (iCoarse<selEnd.xCoarse || iFine<=selEnd.xFine); iFine++) {
      maskOut(opMaskFade,iFine);
      if (iFine!=0) {
        int absoluteTop=255;
        if (iFine==1) {
          if (e->song.ins.empty()) continue;
          absoluteTop=e->song.ins.size()-1;
        } else if (iFine==2) { // volume
          absoluteTop=e->getMaxVolumeChan(iCoarse);
        }
        if (selEnd.y-selStart.y<1) continue;
        for (int j=selStart.y; j<=selEnd.y; j++) {
          double fraction=double(j-selStart.y)/double(selEnd.y-selStart.y);
          int value=p0+double(p1-p0)*fraction;
          if (mode) { // nibble
            value&=15;
            pat->data[j][iFine+1]=MIN(absoluteTop,value|(value<<4));
          } else { // byte
            pat->data[j][iFine+1]=MIN(absoluteTop,value);
          }
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
    DivPattern* pat=e->curPat[iCoarse].getPattern(e->curOrders->ord[iCoarse][curOrder],true);
    for (; iFine<3+e->curPat[iCoarse].effectCols*2 && (iCoarse<selEnd.xCoarse || iFine<=selEnd.xFine); iFine++) {
      maskOut(opMaskInvertVal,iFine);
      if (iFine!=0) {
        int top=255;
        if (iFine==1) {
          if (e->song.ins.empty()) continue;
          top=e->song.ins.size()-1;
        } else if (iFine==2) { // volume
          top=e->getMaxVolumeChan(iCoarse);
        }
        for (int j=selStart.y; j<=selEnd.y; j++) {
          if (pat->data[j][iFine+1]==-1) continue;
          pat->data[j][iFine+1]=top-pat->data[j][iFine+1];
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
    DivPattern* pat=e->curPat[iCoarse].getPattern(e->curOrders->ord[iCoarse][curOrder],true);
    for (; iFine<3+e->curPat[iCoarse].effectCols*2 && (iCoarse<selEnd.xCoarse || iFine<=selEnd.xFine); iFine++) {
      maskOut(opMaskScale,iFine);
      if (iFine!=0) {
        int absoluteTop=255;
        if (iFine==1) {
          if (e->song.ins.empty()) continue;
          absoluteTop=e->song.ins.size()-1;
        } else if (iFine==2) { // volume
          absoluteTop=e->getMaxVolumeChan(iCoarse);
        }
        for (int j=selStart.y; j<=selEnd.y; j++) {
          if (pat->data[j][iFine+1]==-1) continue;
          pat->data[j][iFine+1]=MIN(absoluteTop,(double)pat->data[j][iFine+1]*(top/100.0f));
        }
      }
    }
    iFine=0;
  }

  makeUndo(GUI_UNDO_PATTERN_SCALE);
}

void FurnaceGUI::doRandomize(int bottom, int top, bool mode) {
  finishSelection();
  prepareUndo(GUI_UNDO_PATTERN_RANDOMIZE);

  int iCoarse=selStart.xCoarse;
  int iFine=selStart.xFine;
  for (; iCoarse<=selEnd.xCoarse; iCoarse++) {
    if (!e->curSubSong->chanShow[iCoarse]) continue;
    DivPattern* pat=e->curPat[iCoarse].getPattern(e->curOrders->ord[iCoarse][curOrder],true);
    for (; iFine<3+e->curPat[iCoarse].effectCols*2 && (iCoarse<selEnd.xCoarse || iFine<=selEnd.xFine); iFine++) {
      maskOut(opMaskRandomize,iFine);
      if (iFine!=0) {
        int absoluteTop=255;
        if (iFine==1) {
          if (e->song.ins.empty()) continue;
          absoluteTop=e->song.ins.size()-1;
        } else if (iFine==2) { // volume
          absoluteTop=e->getMaxVolumeChan(iCoarse);
        }
        for (int j=selStart.y; j<=selEnd.y; j++) {
          int value=0;
          int value2=0;
          if (top-bottom<=0) {
            value=MIN(absoluteTop,bottom);
            value2=MIN(absoluteTop,bottom);
          } else {
            value=MIN(absoluteTop,bottom+(rand()%(top-bottom+1)));
            value2=MIN(absoluteTop,bottom+(rand()%(top-bottom+1)));
          }
          if (mode) {
            value&=15;
            value2&=15;
            pat->data[j][iFine+1]=value|(value2<<4);
          } else {
            pat->data[j][iFine+1]=value;
          }
        }
      }
    }
    iFine=0;
  }

  makeUndo(GUI_UNDO_PATTERN_RANDOMIZE);
}

void FurnaceGUI::doFlip() {
  finishSelection();
  prepareUndo(GUI_UNDO_PATTERN_FLIP);

  DivPattern patBuffer;
  int iCoarse=selStart.xCoarse;
  int iFine=selStart.xFine;
  for (; iCoarse<=selEnd.xCoarse; iCoarse++) {
    if (!e->curSubSong->chanShow[iCoarse]) continue;
    DivPattern* pat=e->curPat[iCoarse].getPattern(e->curOrders->ord[iCoarse][curOrder],true);
    for (; iFine<3+e->curPat[iCoarse].effectCols*2 && (iCoarse<selEnd.xCoarse || iFine<=selEnd.xFine); iFine++) {
      maskOut(opMaskFlip,iFine);
      for (int j=selStart.y; j<=selEnd.y; j++) {
        if (iFine==0) {
          patBuffer.data[j][0]=pat->data[j][0];
        }
        patBuffer.data[j][iFine+1]=pat->data[j][iFine+1];
      }
      for (int j=selStart.y; j<=selEnd.y; j++) {
        if (iFine==0) {
          pat->data[j][0]=patBuffer.data[selEnd.y-j+selStart.y][0];
        }
        pat->data[j][iFine+1]=patBuffer.data[selEnd.y-j+selStart.y][iFine+1];
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
        if (iFine==0) {
          patBuffer.data[j][0]=pat->data[j][0];
        }
        patBuffer.data[j][iFine+1]=pat->data[j][iFine+1];
      }
      for (int j=0; j<=sEnd.y-sStart.y; j++) {
        if (j*divider>=sEnd.y-sStart.y) {
          if (iFine==0) {
            pat->data[j+sStart.y][0]=0;
            pat->data[j+sStart.y][1]=0;
          } else {
            pat->data[j+sStart.y][iFine+1]=-1;
          }
        } else {
          if (iFine==0) {
            pat->data[j+sStart.y][0]=patBuffer.data[j*divider+sStart.y][0];
          }
          pat->data[j+sStart.y][iFine+1]=patBuffer.data[j*divider+sStart.y][iFine+1];

          if (iFine==0) {
            for (int k=1; k<divider; k++) {
              if ((j*divider+k)>=sEnd.y-sStart.y) break;
              if (!(pat->data[j+sStart.y][0]==0 && pat->data[j+sStart.y][1]==0)) break;
              pat->data[j+sStart.y][0]=patBuffer.data[j*divider+sStart.y+k][0];
              pat->data[j+sStart.y][1]=patBuffer.data[j*divider+sStart.y+k][1];
            }
          } else {
            for (int k=1; k<divider; k++) {
              if ((j*divider+k)>=sEnd.y-sStart.y) break;
              if (pat->data[j+sStart.y][iFine+1]!=-1) break;
              pat->data[j+sStart.y][iFine+1]=patBuffer.data[j*divider+sStart.y+k][iFine+1];
            }
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
        if (iFine==0) {
          patBuffer.data[j][0]=pat->data[j][0];
        }
        patBuffer.data[j][iFine+1]=pat->data[j][iFine+1];
      }
      for (int j=0; j<=(sEnd.y-sStart.y)*multiplier; j++) {
        if ((j+sStart.y)>=e->curSubSong->patLen) break;
        if ((j%multiplier)!=0) {
          if (iFine==0) {
            pat->data[j+sStart.y][0]=0;
            pat->data[j+sStart.y][1]=0;
          } else {
            pat->data[j+sStart.y][iFine+1]=-1;
          }
          continue;
        }
        if (iFine==0) {
          pat->data[j+sStart.y][0]=patBuffer.data[j/multiplier+sStart.y][0];
        }
        pat->data[j+sStart.y][iFine+1]=patBuffer.data[j/multiplier+sStart.y][iFine+1];
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
          if (l==0) {
            if (!(pat->data[k/divider][0]==0 && pat->data[k/divider][1]==0)) continue;
          } else {
            if (pat->data[k/divider][l+1]!=-1) continue;
          }

          if (l==0) {
            pat->data[k/divider][l]=patCopy.data[k][l];
          }
          pat->data[k/divider][l+1]=patCopy.data[k][l+1];

          if (l>3 && !(l&1)) { // scale effects as needed
            switch (pat->data[k/divider][l]) {
              case 0x0d:
                pat->data[k/divider][l+1]/=divider;
                break;
              case 0x0f:
                pat->data[k/divider][l+1]=CLAMP(pat->data[k/divider][l+1]*divider,1,255);
                break;
            }
          }
        }
      }

      // put undo
      for (int k=0; k<DIV_MAX_ROWS; k++) {
        for (int l=0; l<DIV_MAX_COLS; l++) {
          if (pat->data[k][l]!=patCopy.data[k][l]) {
            us.pat.push_back(UndoPatternData(subSong,i,j,k,l,patCopy.data[k][l],pat->data[k][l]));
          }
        }
      }
    }
  }
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
          if (l==0) {
            if (!(pat->data[k*multiplier][0]==0 && pat->data[k*multiplier][1]==0)) continue;
          } else {
            if (pat->data[k*multiplier][l+1]!=-1) continue;
          }

          if (l==0) {
            pat->data[k*multiplier][l]=patCopy.data[k][l];
          }
          pat->data[k*multiplier][l+1]=patCopy.data[k][l+1];

          if (l>3 && !(l&1)) { // scale effects as needed
            switch (pat->data[k*multiplier][l]) {
              case 0x0d:
                pat->data[k*multiplier][l+1]/=multiplier;
                break;
              case 0x0f:
                pat->data[k*multiplier][l+1]=CLAMP(pat->data[k*multiplier][l+1]/multiplier,1,255);
                break;
            }
          }
        }
      }

      // put undo
      for (int k=0; k<DIV_MAX_ROWS; k++) {
        for (int l=0; l<DIV_MAX_COLS; l++) {
          if (pat->data[k][l]!=patCopy.data[k][l]) {
            us.pat.push_back(UndoPatternData(subSong,i,j,k,l,patCopy.data[k][l],pat->data[k][l]));
          }
        }
      }
    }
  }
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
      if (!foundIns && pat->data[i][2] >= 0) {
        foundIns=true;
        curIns=pat->data[i][2];
      }

      // absorb most recent octave (i.e. set curOctave such that the "main row" (QWERTY) of
      // notes will result in an octave number equal to the previous note). make sure to
      // skip "special note values" like OFF/REL/=== and "none", since there won't be valid
      // octave values
      unsigned char note=pat->data[i][0];
      if (!foundOctave && note!=0 && note!=100 && note!=101 && note!=102) {
        foundOctave=true;

        // decode octave data (was signed cast to unsigned char)
        int octave=pat->data[i][1];
        if (octave>128) octave-=256;

        // @NOTE the special handling when note==12, which is really an octave above what's
        // stored in the octave data. without this handling, if you press Q, then
        // "ABSORB_INSTRUMENT", then Q again, you'd get a different octave!
        if (pat->data[i][0]==12) octave++;
        curOctave=CLAMP(octave-1, GUI_EDIT_OCTAVE_MIN, GUI_EDIT_OCTAVE_MAX);
      }
    }
  }

  // if no instrument has been set at this point, the only way to match it is to use "none"
  if (!foundIns) curIns=-1;

  logD("doAbsorbInstrument -- searched %d orders", curOrder-orderIdx);
}

void FurnaceGUI::doDrag() {
  int len=dragEnd.xCoarse-dragStart.xCoarse+1;

  if (len<1) return;
  
  prepareUndo(GUI_UNDO_PATTERN_DRAG);

  // copy and clear
  String c=doCopy(true,false,dragStart,dragEnd);

  logV(_("copy: %s"),c);

  // replace
  cursor=selStart;
  doPaste(GUI_PASTE_MODE_NORMAL,0,false,c);

  makeUndo(GUI_UNDO_PATTERN_DRAG);
}

void FurnaceGUI::moveSelected(int x, int y) {
  prepareUndo(GUI_UNDO_PATTERN_DRAG);

  // copy and clear
  String c=doCopy(true,false,selStart,selEnd);

  logV(_("copy: %s"),c);

  // replace
  selStart.xCoarse+=x;
  selEnd.xCoarse+=x;
  selStart.y+=y;
  selEnd.y+=y;
  cursor=selStart;
  doPaste(GUI_PASTE_MODE_NORMAL,0,false,c);

  makeUndo(GUI_UNDO_PATTERN_DRAG);
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
        p->data[i.row][i.col]=i.oldVal;
      }
      if (us.type!=GUI_UNDO_REPLACE) {
        if (!e->isPlaying() || !followPattern) {
          cursor=us.cursor;
          selStart=us.selStart;
          selEnd=us.selEnd;
          curNibble=us.nibble;
          updateScroll(cursor.y);
          setOrder(us.order);
        }
      }
      e->walkSong(loopOrder,loopRow,loopEnd);
      break;
  }

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
        p->data[i.row][i.col]=i.newVal;
      }
      if (us.type!=GUI_UNDO_REPLACE) {
        if (!e->isPlaying() || !followPattern) {
          cursor=us.cursor;
          selStart=us.selStart;
          selEnd=us.selEnd;
          curNibble=us.nibble;
          updateScroll(cursor.y);
          setOrder(us.order);
        }
      }
      e->walkSong(loopOrder,loopRow,loopEnd);
      break;
  }

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
