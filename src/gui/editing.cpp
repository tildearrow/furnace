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
#include "../ta-log.h"
#include "guiConst.h"
#include <fmt/printf.h>

#include "actionUtil.h"

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

void FurnaceGUI::prepareUndo(ActionType action) {
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
      for (int i=0; i<e->getTotalChannelCount(); i++) {
        e->curPat[i].getPattern(e->curOrders->ord[i][curOrder],false)->copyOn(oldPat[i]);
      }
      break;
    case GUI_UNDO_REPLACE: // this is handled by doReplace()
      break;
  }
}

void FurnaceGUI::makeUndo(ActionType action) {
  bool doPush=false;
  UndoStep s;
  s.type=action;
  s.cursor=cursor;
  s.selStart=selStart;
  s.selEnd=selEnd;
  s.order=curOrder;
  s.nibble=curNibble;
  size_t subSong=e->getCurrentSubSong();
  switch (action) {
    case GUI_UNDO_CHANGE_ORDER:
      for (int i=0; i<DIV_MAX_CHANS; i++) {
        for (int j=0; j<128; j++) {
          if (oldOrders.ord[i][j]!=e->curOrders->ord[i][j]) {
            s.ord.push_back(UndoOrderData(subSong,i,j,oldOrders.ord[i][j],e->curOrders->ord[i][j]));
          }
        }
      }
      s.oldOrdersLen=oldOrdersLen;
      s.newOrdersLen=e->curSubSong->ordersLen;
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
      for (int i=0; i<e->getTotalChannelCount(); i++) {
        DivPattern* p=e->curPat[i].getPattern(e->curOrders->ord[i][curOrder],false);
        for (int j=0; j<e->curSubSong->patLen; j++) {
          for (int k=0; k<DIV_MAX_COLS; k++) {
            if (p->data[j][k]!=oldPat[i]->data[j][k]) {
              s.pat.push_back(UndoPatternData(subSong,i,e->curOrders->ord[i][curOrder],j,k,oldPat[i]->data[j][k],p->data[j][k]));
            }
          }
        }
      }
      if (!s.pat.empty()) {
        doPush=true;
      }
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

  int iCoarse=selStart.xCoarse;
  int iFine=selStart.xFine;
  for (; iCoarse<=selEnd.xCoarse; iCoarse++) {
    if (!e->curSubSong->chanShow[iCoarse]) continue;
    DivPattern* pat=e->curPat[iCoarse].getPattern(e->curOrders->ord[iCoarse][curOrder],true);
    for (; iFine<3+e->curPat[iCoarse].effectCols*2 && (iCoarse<selEnd.xCoarse || iFine<=selEnd.xFine); iFine++) {
      maskOut(opMaskPullDelete,iFine);
      for (int j=selStart.y; j<e->curSubSong->patLen; j++) {
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

  int iCoarse=selStart.xCoarse;
  int iFine=selStart.xFine;
  for (; iCoarse<=selEnd.xCoarse; iCoarse++) {
    if (!e->curSubSong->chanShow[iCoarse]) continue;
    DivPattern* pat=e->curPat[iCoarse].getPattern(e->curOrders->ord[iCoarse][curOrder],true);
    for (; iFine<3+e->curPat[iCoarse].effectCols*2 && (iCoarse<selEnd.xCoarse || iFine<=selEnd.xFine); iFine++) {
      maskOut(opMaskInsert,iFine);
      for (int j=e->curSubSong->patLen-1; j>=selStart.y; j--) {
        if (j==selStart.y) {
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

void FurnaceGUI::doCopy(bool cut) {
  finishSelection();
  if (cut) {
    curNibble=false;
    prepareUndo(GUI_UNDO_PATTERN_CUT);
  }
  clipboard=fmt::sprintf("org.tildearrow.furnace - Pattern Data (%d)\n%d",DIV_ENGINE_VERSION,selStart.xFine);

  for (int j=selStart.y; j<=selEnd.y; j++) {
    int iCoarse=selStart.xCoarse;
    int iFine=selStart.xFine;
    if (iFine>3 && !(iFine&1)) {
      iFine--;
    }
    clipboard+='\n';
    for (; iCoarse<=selEnd.xCoarse; iCoarse++) {
      if (!e->curSubSong->chanShow[iCoarse]) continue;
      DivPattern* pat=e->curPat[iCoarse].getPattern(e->curOrders->ord[iCoarse][curOrder],true);
      for (; iFine<3+e->curPat[iCoarse].effectCols*2 && (iCoarse<selEnd.xCoarse || iFine<=selEnd.xFine); iFine++) {
        if (iFine==0) {
          clipboard+=noteNameNormal(pat->data[j][0],pat->data[j][1]);
          if (cut) {
            pat->data[j][0]=0;
            pat->data[j][1]=0;
          }
        } else {
          if (pat->data[j][iFine+1]==-1) {
            clipboard+="..";
          } else {
            clipboard+=fmt::sprintf("%.2X",pat->data[j][iFine+1]);
          }
          if (cut) {
            pat->data[j][iFine+1]=-1;
          }
        }
      }
      clipboard+='|';
      iFine=0;
    }
  }
  SDL_SetClipboardText(clipboard.c_str());

  if (cut) {
    makeUndo(GUI_UNDO_PATTERN_CUT);
  }
}

void FurnaceGUI::doPaste(PasteMode mode, int arg) {
  finishSelection();
  prepareUndo(GUI_UNDO_PATTERN_PASTE);
  char* clipText=SDL_GetClipboardText();
  if (clipText!=NULL) {
    if (clipText[0]) {
      clipboard=clipText;
    }
    SDL_free(clipText);
  }
  std::vector<String> data;
  String tempS;
  for (char i: clipboard) {
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
  if (data[0].find("org.tildearrow.furnace - Pattern Data")!=0) return;
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
      logW("invalid clipboard data! failed at line %d char %d",i,charPos);
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
  if (settings.cursorPastePos) {
    cursor.y=j;
    if (cursor.y>=e->curSubSong->patLen) cursor.y=e->curSubSong->patLen-1;
    selStart=cursor;
    selEnd=cursor;
    updateScroll(cursor.y);
  }

  makeUndo(GUI_UNDO_PATTERN_PASTE);
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

void FurnaceGUI::doCollapse(int divider) {
  finishSelection();
  prepareUndo(GUI_UNDO_PATTERN_COLLAPSE);

  DivPattern patBuffer;
  int iCoarse=selStart.xCoarse;
  int iFine=selStart.xFine;
  for (; iCoarse<=selEnd.xCoarse; iCoarse++) {
    if (!e->curSubSong->chanShow[iCoarse]) continue;
    DivPattern* pat=e->curPat[iCoarse].getPattern(e->curOrders->ord[iCoarse][curOrder],true);
    for (; iFine<3+e->curPat[iCoarse].effectCols*2 && (iCoarse<selEnd.xCoarse || iFine<=selEnd.xFine); iFine++) {
      maskOut(opMaskCollapseExpand,iFine);
      for (int j=selStart.y; j<=selEnd.y; j++) {
        if (iFine==0) {
          patBuffer.data[j][0]=pat->data[j][0];
        }
        patBuffer.data[j][iFine+1]=pat->data[j][iFine+1];
      }
      for (int j=0; j<=selEnd.y-selStart.y; j++) {
        if (j*divider>=selEnd.y-selStart.y) {
          if (iFine==0) {
            pat->data[j+selStart.y][0]=0;
            pat->data[j+selStart.y][1]=0;
          } else {
            pat->data[j+selStart.y][iFine+1]=-1;
          }
        } else {
          if (iFine==0) {
            pat->data[j+selStart.y][0]=patBuffer.data[j*divider+selStart.y][0];
          }
          pat->data[j+selStart.y][iFine+1]=patBuffer.data[j*divider+selStart.y][iFine+1];

          if (iFine==0) {
            for (int k=1; k<divider; k++) {
              if ((j*divider+k)>=selEnd.y-selStart.y) break;
              if (!(pat->data[j+selStart.y][0]==0 && pat->data[j+selStart.y][1]==0)) break;
              pat->data[j+selStart.y][0]=patBuffer.data[j*divider+selStart.y+k][0];
              pat->data[j+selStart.y][1]=patBuffer.data[j*divider+selStart.y+k][1];
            }
          } else {
            for (int k=1; k<divider; k++) {
              if ((j*divider+k)>=selEnd.y-selStart.y) break;
              if (pat->data[j+selStart.y][iFine+1]!=-1) break;
              pat->data[j+selStart.y][iFine+1]=patBuffer.data[j*divider+selStart.y+k][iFine+1];
            }
          }
        }
      }
    }
    iFine=0;
  }

  makeUndo(GUI_UNDO_PATTERN_COLLAPSE);
}

void FurnaceGUI::doExpand(int multiplier) {
  if (multiplier<1) return;

  finishSelection();
  prepareUndo(GUI_UNDO_PATTERN_EXPAND);

  DivPattern patBuffer;
  int iCoarse=selStart.xCoarse;
  int iFine=selStart.xFine;
  for (; iCoarse<=selEnd.xCoarse; iCoarse++) {
    if (!e->curSubSong->chanShow[iCoarse]) continue;
    DivPattern* pat=e->curPat[iCoarse].getPattern(e->curOrders->ord[iCoarse][curOrder],true);
    for (; iFine<3+e->curPat[iCoarse].effectCols*2 && (iCoarse<selEnd.xCoarse || iFine<=selEnd.xFine); iFine++) {
      maskOut(opMaskCollapseExpand,iFine);
      for (int j=selStart.y; j<=selEnd.y; j++) {
        if (iFine==0) {
          patBuffer.data[j][0]=pat->data[j][0];
        }
        patBuffer.data[j][iFine+1]=pat->data[j][iFine+1];
      }
      for (int j=0; j<=(selEnd.y-selStart.y)*multiplier; j++) {
        if ((j+selStart.y)>=e->curSubSong->patLen) break;
        if ((j%multiplier)!=0) {
          if (iFine==0) {
            pat->data[j+selStart.y][0]=0;
            pat->data[j+selStart.y][1]=0;
          } else {
            pat->data[j+selStart.y][iFine+1]=-1;
          }
          continue;
        }
        if (iFine==0) {
          pat->data[j+selStart.y][0]=patBuffer.data[j/multiplier+selStart.y][0];
        }
        pat->data[j+selStart.y][iFine+1]=patBuffer.data[j/multiplier+selStart.y][iFine+1];
      }
    }
    iFine=0;
  }

  makeUndo(GUI_UNDO_PATTERN_EXPAND);
}

void FurnaceGUI::doDrag() {
  DivPattern* patBuffer=NULL;
  int len=dragEnd.xCoarse-dragStart.xCoarse+1;

  DETERMINE_FIRST_LAST;

  if (len<1) return;
  
  patBuffer=new DivPattern[len];
  prepareUndo(GUI_UNDO_PATTERN_DRAG);

  // copy and clear
  {
    int iCoarse=dragStart.xCoarse;
    int iFine=dragStart.xFine;
    int iCoarseP=0;
    for (; iCoarse<=dragEnd.xCoarse; iCoarse++) {
      if (!e->curSubSong->chanShow[iCoarse]) continue;
      DivPattern* pat=e->curPat[iCoarse].getPattern(e->curOrders->ord[iCoarse][curOrder],true);
      for (; iFine<3+e->curPat[iCoarse].effectCols*2 && (iCoarse<dragEnd.xCoarse || iFine<=dragEnd.xFine); iFine++) {
        int row=0;
        for (int j=dragStart.y; j<=dragEnd.y; j++) {
          if (iFine==0) {
            patBuffer[iCoarseP].data[row][iFine]=pat->data[j][iFine];
            pat->data[j][iFine]=0;
            if (dragStart.y==dragEnd.y) pat->data[j][2]=-1;
          }
          patBuffer[iCoarseP].data[row][iFine+1]=pat->data[j][iFine+1];
          pat->data[j][iFine+1]=(iFine<1)?0:-1;

          if (dragStart.y==dragEnd.y && iFine>2 && iFine&1 && settings.effectDeletionAltersValue) {
            pat->data[j][iFine+2]=-1;
          }
          row++;
        }
      }
      iFine=0;
      iCoarseP++;
    }
  }

  // replace
  {
    int iCoarse=selStart.xCoarse;
    int iFine=selStart.xFine;
    int iCoarseP=0;
    for (; iCoarse<=selEnd.xCoarse && iCoarseP<len; iCoarse++) {
      if (iCoarse<firstChannel || iCoarse>lastChannel) continue;
      if (!e->curSubSong->chanShow[iCoarse]) continue;
      DivPattern* pat=e->curPat[iCoarse].getPattern(e->curOrders->ord[iCoarse][curOrder],true);
      for (; iFine<3+e->curPat[iCoarse].effectCols*2 && (iCoarse<selEnd.xCoarse || iFine<=selEnd.xFine); iFine++) {
        int row=-1;
        for (int j=selStart.y; j<=selEnd.y; j++) {
          row++;
          if (j<0 || j>=e->curSubSong->patLen) continue;
          if (iFine==0) {
            pat->data[j][iFine]=patBuffer[iCoarseP].data[row][iFine];
          }
          pat->data[j][iFine+1]=patBuffer[iCoarseP].data[row][iFine+1];
        }
      }
      iFine=0;
      iCoarseP++;
    }
  }

  delete[] patBuffer;
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
      break;
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

      break;
  }

  redoHist.pop_back();
}
