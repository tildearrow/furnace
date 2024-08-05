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
#include "commandPalette.h"
#include "../ta-log.h"
#include <fmt/printf.h>
#include <imgui.h>

#include "actionUtil.h"
#include "sampleUtil.h"

const unsigned char avRequest[15]={
  0xf0, 0x43, 0x20, 0x7e, 0x4c, 0x4d, 0x20, 0x20, 0x38, 0x39, 0x37, 0x36, 0x41, 0x45, 0xf7
};


void FurnaceGUI::doAction(int what) {
  switch (what) {
    case GUI_ACTION_NEW:
      if (modified) {
        showWarning(_("Unsaved changes! Save changes before creating a new song?"),GUI_WARN_NEW);
      } else {
        displayNew=true;
      }
      break;
    case GUI_ACTION_OPEN:
      if (modified) {
        showWarning(_("Unsaved changes! Save changes before opening another file?"),GUI_WARN_OPEN);
      } else {
        openFileDialog(GUI_FILE_OPEN);
      }
      break;
    case GUI_ACTION_OPEN_BACKUP:
      if (modified) {
        showWarning(_("Unsaved changes! Save changes before opening backup?"),GUI_WARN_OPEN_BACKUP);
      } else {
        openFileDialog(GUI_FILE_OPEN_BACKUP);
      }
      break;
    case GUI_ACTION_SAVE:
      if (curFileName=="" || curFileName==backupPath || e->song.version>=0xff00) {
        openFileDialog(GUI_FILE_SAVE);
      } else {
        if (save(curFileName,e->song.isDMF?e->song.version:0)>0) {
          showError(fmt::sprintf(_("Error while saving file! (%s)"),lastError));
        }
      }
      break;
    case GUI_ACTION_SAVE_AS:
      openFileDialog(GUI_FILE_SAVE);
      break;
    case GUI_ACTION_EXPORT:
      curExportType=GUI_EXPORT_NONE;
      displayExport=true;
      break;
    case GUI_ACTION_UNDO:
      if (curWindow==GUI_WINDOW_SAMPLE_EDIT) {
        doUndoSample();
      } else if (curWindow==GUI_WINDOW_INS_EDIT) {
        doUndoInstrument();
      } else {
        doUndo();
      }
      break;
    case GUI_ACTION_REDO:
      if (curWindow==GUI_WINDOW_SAMPLE_EDIT) {
        doRedoSample();
      } else if (curWindow==GUI_WINDOW_INS_EDIT) {
        doRedoInstrument();
      } else {
        doRedo();
      }
      break;
    case GUI_ACTION_QUIT:
      requestQuit();
      break;
    case GUI_ACTION_PLAY_TOGGLE:
      if (e->isPlaying() && !e->isStepping()) {
        stop();
      } else {
        play();
      }
      break;
    case GUI_ACTION_PLAY:
      play();
      break;
    case GUI_ACTION_STOP:
      stop();
      break;
    case GUI_ACTION_PLAY_START:
      e->setOrder(0);
      if (!e->isPlaying()) {
        play();
      }
      e->setRepeatPattern(false);
      break;
    case GUI_ACTION_PLAY_REPEAT:
      play();
      e->setRepeatPattern(true);
      break;
    case GUI_ACTION_PLAY_CURSOR:
      if (e->isPlaying() && !e->isStepping()) {
        stop();
      } else {
        play(cursor.y);
      }
      break;
    case GUI_ACTION_STEP_ONE:
      e->stepOne(cursor.y);
      pendingStepUpdate=1;
      break;
    case GUI_ACTION_OCTAVE_UP:
      if (++curOctave>7) {
        curOctave=7;
      } else {
        e->autoNoteOffAll();
        failedNoteOn=false;
      }
      break;
    case GUI_ACTION_OCTAVE_DOWN:
      if (--curOctave<-5) {
        curOctave=-5;
      } else {
        e->autoNoteOffAll();
        failedNoteOn=false;
      }
      break;
    case GUI_ACTION_INS_UP:
      if (--curIns<-1) {
        curIns=-1;
      }
      wavePreviewInit=true;
      wantScrollListIns=true;
      updateFMPreview=true;
      break;
    case GUI_ACTION_INS_DOWN:
      if (++curIns>=(int)e->song.ins.size()) {
        curIns=((int)e->song.ins.size())-1;
      }
      wavePreviewInit=true;
      wantScrollListIns=true;
      updateFMPreview=true;
      break;
    case GUI_ACTION_STEP_UP:
      if (++editStep>64) editStep=64;
      break;
    case GUI_ACTION_STEP_DOWN:
      if (--editStep<0) editStep=0;
      break;
    case GUI_ACTION_TOGGLE_EDIT:
      edit=!edit;
      break;
    case GUI_ACTION_METRONOME:
      e->setMetronome(!e->getMetronome());
      break;
    case GUI_ACTION_REPEAT_PATTERN:
      e->setRepeatPattern(!e->getRepeatPattern());
      break;
    case GUI_ACTION_FOLLOW_ORDERS:
      followOrders=!followOrders;
      break;
    case GUI_ACTION_FOLLOW_PATTERN:
      followPattern=!followPattern;
      break;
    case GUI_ACTION_FULLSCREEN:
      fullScreen=!fullScreen;
      SDL_SetWindowFullscreen(sdlWin,fullScreen?(SDL_WINDOW_FULLSCREEN|SDL_WINDOW_FULLSCREEN_DESKTOP):0);
      break;
    case GUI_ACTION_TX81Z_REQUEST: {
      TAMidiMessage msg;
      msg.type=TA_MIDI_SYSEX;
      msg.sysExData.reset(new unsigned char[15],std::default_delete<unsigned char[]>());
      msg.sysExLen=15;
      memcpy(msg.sysExData.get(),avRequest,15);
      if (!e->sendMidiMessage(msg)) {
        showError(_("Error while sending request (MIDI output not configured?)"));
      }
      break;
    }
    case GUI_ACTION_PANIC:
      e->syncReset();
      break;
    case GUI_ACTION_CLEAR:
      showWarning(_("Select an option: (cannot be undone!)"),GUI_WARN_CLEAR);
      break;
    case GUI_ACTION_COMMAND_PALETTE:
      displayPalette=true;
      curPaletteType=CMDPAL_TYPE_MAIN;
      break;
    case GUI_ACTION_CMDPAL_RECENT:
      displayPalette=true;
      curPaletteType=CMDPAL_TYPE_RECENT;
      break;
    case GUI_ACTION_CMDPAL_INSTRUMENTS:
      displayPalette=true;
      curPaletteType=CMDPAL_TYPE_INSTRUMENTS;
      break;
    case GUI_ACTION_CMDPAL_SAMPLES:
      displayPalette=true;
      curPaletteType=CMDPAL_TYPE_SAMPLES;
      break;
    case GUI_ACTION_CMDPAL_INSTRUMENT_CHANGE:
      displayPalette=true;
      curPaletteType=CMDPAL_TYPE_INSTRUMENT_CHANGE;
      break;
    case GUI_ACTION_CMDPAL_ADD_CHIP:
      displayPalette=true;
      curPaletteType=CMDPAL_TYPE_ADD_CHIP;
      break;
    case GUI_ACTION_WINDOW_EDIT_CONTROLS:
      nextWindow=GUI_WINDOW_EDIT_CONTROLS;
      break;
    case GUI_ACTION_WINDOW_ORDERS:
      nextWindow=GUI_WINDOW_ORDERS;
      break;
    case GUI_ACTION_WINDOW_INS_LIST:
      nextWindow=GUI_WINDOW_INS_LIST;
      break;
    case GUI_ACTION_WINDOW_INS_EDIT:
      nextWindow=GUI_WINDOW_INS_EDIT;
      break;
    case GUI_ACTION_WINDOW_SONG_INFO:
      nextWindow=GUI_WINDOW_SONG_INFO;
      break;
    case GUI_ACTION_WINDOW_SPEED:
      nextWindow=GUI_WINDOW_SPEED;
      break;
    case GUI_ACTION_WINDOW_PATTERN:
      nextWindow=GUI_WINDOW_PATTERN;
      break;
    case GUI_ACTION_WINDOW_WAVE_LIST:
      nextWindow=GUI_WINDOW_WAVE_LIST;
      break;
    case GUI_ACTION_WINDOW_WAVE_EDIT:
      nextWindow=GUI_WINDOW_WAVE_EDIT;
      break;
    case GUI_ACTION_WINDOW_SAMPLE_LIST:
      nextWindow=GUI_WINDOW_SAMPLE_LIST;
      break;
    case GUI_ACTION_WINDOW_SAMPLE_EDIT:
      nextWindow=GUI_WINDOW_SAMPLE_EDIT;
      break;
    case GUI_ACTION_WINDOW_ABOUT:
      nextWindow=GUI_WINDOW_ABOUT;
      break;
    case GUI_ACTION_WINDOW_SETTINGS:
      nextWindow=GUI_WINDOW_SETTINGS;
      break;
    case GUI_ACTION_WINDOW_MIXER:
      nextWindow=GUI_WINDOW_MIXER;
      break;
    case GUI_ACTION_WINDOW_DEBUG:
      nextWindow=GUI_WINDOW_DEBUG;
      break;
    case GUI_ACTION_WINDOW_OSCILLOSCOPE:
      nextWindow=GUI_WINDOW_OSCILLOSCOPE;
      break;
    case GUI_ACTION_WINDOW_VOL_METER:
      nextWindow=GUI_WINDOW_VOL_METER;
      break;
    case GUI_ACTION_WINDOW_STATS:
      nextWindow=GUI_WINDOW_STATS;
      break;
    case GUI_ACTION_WINDOW_COMPAT_FLAGS:
      nextWindow=GUI_WINDOW_COMPAT_FLAGS;
      break;
    case GUI_ACTION_WINDOW_PIANO:
      nextWindow=GUI_WINDOW_PIANO;
      break;
    case GUI_ACTION_WINDOW_NOTES:
      nextWindow=GUI_WINDOW_NOTES;
      break;
    case GUI_ACTION_WINDOW_CHANNELS:
      nextWindow=GUI_WINDOW_CHANNELS;
      break;
    case GUI_ACTION_WINDOW_PAT_MANAGER:
      nextWindow=GUI_WINDOW_PAT_MANAGER;
      break;
    case GUI_ACTION_WINDOW_SYS_MANAGER:
      nextWindow=GUI_WINDOW_SYS_MANAGER;
      break;
    case GUI_ACTION_WINDOW_REGISTER_VIEW:
      nextWindow=GUI_WINDOW_REGISTER_VIEW;
      break;
    case GUI_ACTION_WINDOW_LOG:
      nextWindow=GUI_WINDOW_LOG;
      break;
    case GUI_ACTION_WINDOW_EFFECT_LIST:
      nextWindow=GUI_WINDOW_EFFECT_LIST;
      break;
    case GUI_ACTION_WINDOW_CHAN_OSC:
      nextWindow=GUI_WINDOW_CHAN_OSC;
      break;
    case GUI_ACTION_WINDOW_FIND:
      nextWindow=GUI_WINDOW_FIND;
      break;
    case GUI_ACTION_WINDOW_GROOVES:
      nextWindow=GUI_WINDOW_GROOVES;
      break;
    case GUI_ACTION_WINDOW_XY_OSC:
      nextWindow=GUI_WINDOW_XY_OSC;
      break;
    case GUI_ACTION_WINDOW_MEMORY:
      nextWindow=GUI_WINDOW_MEMORY;
      break;
    case GUI_ACTION_WINDOW_CS_PLAYER:
      nextWindow=GUI_WINDOW_CS_PLAYER;
      break;
    case GUI_ACTION_WINDOW_USER_PRESETS:
      nextWindow=GUI_WINDOW_USER_PRESETS;
      break;
    
    case GUI_ACTION_COLLAPSE_WINDOW:
      collapseWindow=true;
      break;
    case GUI_ACTION_CLOSE_WINDOW:
      switch (curWindow) {
        case GUI_WINDOW_EDIT_CONTROLS:
          editControlsOpen=false;
          break;
        case GUI_WINDOW_SONG_INFO:
          songInfoOpen=false;
          break;
        case GUI_WINDOW_SPEED:
          speedOpen=false;
          break;
        case GUI_WINDOW_ORDERS:
          ordersOpen=false;
          break;
        case GUI_WINDOW_INS_LIST:
          insListOpen=false;
          break;
        case GUI_WINDOW_PATTERN:
          patternOpen=false;
          break;
        case GUI_WINDOW_INS_EDIT:
          insEditOpen=false;
          break;
        case GUI_WINDOW_WAVE_LIST:
          waveListOpen=false;
          break;
        case GUI_WINDOW_WAVE_EDIT:
          waveEditOpen=false;
          break;
        case GUI_WINDOW_SAMPLE_LIST:
          sampleListOpen=false;
          break;
        case GUI_WINDOW_SAMPLE_EDIT:
          sampleEditOpen=false;
          break;
        case GUI_WINDOW_MIXER:
          mixerOpen=false;
          break;
        case GUI_WINDOW_ABOUT:
          aboutOpen=false;
          break;
        case GUI_WINDOW_SETTINGS:
          settingsOpen=false;
          break;
        case GUI_WINDOW_DEBUG:
          debugOpen=false;
          break;
        case GUI_WINDOW_OSCILLOSCOPE:
          oscOpen=false;
          break;
        case GUI_WINDOW_VOL_METER:
          volMeterOpen=false;
          break;
        case GUI_WINDOW_STATS:
          statsOpen=false;
          break;
        case GUI_WINDOW_COMPAT_FLAGS:
          compatFlagsOpen=false;
          break;
        case GUI_WINDOW_PIANO:
          pianoOpen=false;
          break;
        case GUI_WINDOW_NOTES:
          notesOpen=false;
          break;
        case GUI_WINDOW_CHANNELS:
          channelsOpen=false;
          break;
        case GUI_WINDOW_PAT_MANAGER:
          patManagerOpen=false;
          break;
        case GUI_WINDOW_SYS_MANAGER:
          sysManagerOpen=false;
          break;
        case GUI_WINDOW_REGISTER_VIEW:
          regViewOpen=false;
          break;
        case GUI_WINDOW_LOG:
          logOpen=false;
          break;
        case GUI_WINDOW_EFFECT_LIST:
          effectListOpen=false;
          break;
        case GUI_WINDOW_CHAN_OSC:
          chanOscOpen=false;
          break;
        case GUI_WINDOW_FIND:
          findOpen=false;
          break;
        case GUI_WINDOW_GROOVES:
          groovesOpen=false;
          break;
        case GUI_WINDOW_XY_OSC:
          xyOscOpen=false;
          break;
        case GUI_WINDOW_MEMORY:
          memoryOpen=false;
          break;
        case GUI_WINDOW_CS_PLAYER:
          csPlayerOpen=false;
          break;
        case GUI_WINDOW_USER_PRESETS:
          userPresetsOpen=false;
        default:
          break;
      }
      curWindow=GUI_WINDOW_NOTHING;
      break;

    case GUI_ACTION_PAT_NOTE_UP:
      doTranspose(1,opMaskTransposeNote);
      break;
    case GUI_ACTION_PAT_NOTE_DOWN:
      doTranspose(-1,opMaskTransposeNote);
      break;
    case GUI_ACTION_PAT_OCTAVE_UP:
      doTranspose(12,opMaskTransposeNote);
      break;
    case GUI_ACTION_PAT_OCTAVE_DOWN:
      doTranspose(-12,opMaskTransposeNote);
      break;
    case GUI_ACTION_PAT_VALUE_UP:
      doTranspose(1,opMaskTransposeValue);
      break;
    case GUI_ACTION_PAT_VALUE_DOWN:
      doTranspose(-1,opMaskTransposeValue);
      break;
    case GUI_ACTION_PAT_VALUE_UP_COARSE:
      doTranspose(16,opMaskTransposeValue);
      break;
    case GUI_ACTION_PAT_VALUE_DOWN_COARSE:
      doTranspose(-16,opMaskTransposeValue);
      break;
    case GUI_ACTION_PAT_SELECT_ALL:
      doSelectAll();
      break;
    case GUI_ACTION_PAT_CUT:
      doCopy(true,true,selStart,selEnd);
      break;
    case GUI_ACTION_PAT_COPY:
      doCopy(false,true,selStart,selEnd);
      break;
    case GUI_ACTION_PAT_PASTE:
      doPaste();
      break;
    case GUI_ACTION_PAT_PASTE_MIX:
      doPaste(GUI_PASTE_MODE_MIX_FG);
      break;
    case GUI_ACTION_PAT_PASTE_MIX_BG:
      doPaste(GUI_PASTE_MODE_MIX_BG);
      break;
    case GUI_ACTION_PAT_PASTE_FLOOD:
      doPaste(GUI_PASTE_MODE_FLOOD);
      break;
    case GUI_ACTION_PAT_PASTE_OVERFLOW:
      doPaste(GUI_PASTE_MODE_OVERFLOW);
      break;
    case GUI_ACTION_PAT_CURSOR_UP:
      moveCursor(0,-MAX(1,settings.scrollStep?editStep:1),false);
      break;
    case GUI_ACTION_PAT_CURSOR_DOWN:
      moveCursor(0,MAX(1,settings.scrollStep?editStep:1),false);
      break;
    case GUI_ACTION_PAT_CURSOR_LEFT:
      moveCursor(-1,0,false);
      break;
    case GUI_ACTION_PAT_CURSOR_RIGHT:
      moveCursor(1,0,false);
      break;
    case GUI_ACTION_PAT_CURSOR_UP_ONE:
      moveCursor(0,-1,false);
      break;
    case GUI_ACTION_PAT_CURSOR_DOWN_ONE:
      moveCursor(0,1,false);
      break;
    case GUI_ACTION_PAT_CURSOR_LEFT_CHANNEL:
      moveCursorPrevChannel(false);
      break;
    case GUI_ACTION_PAT_CURSOR_RIGHT_CHANNEL:
      moveCursorNextChannel(false);
      break;
    case GUI_ACTION_PAT_CURSOR_NEXT_CHANNEL:
      moveCursorNextChannel(true);
      break;
    case GUI_ACTION_PAT_CURSOR_PREVIOUS_CHANNEL:
      moveCursorPrevChannel(true);
      break;
    case GUI_ACTION_PAT_CURSOR_BEGIN:
      moveCursorTop(false);
      break;
    case GUI_ACTION_PAT_CURSOR_END:
      moveCursorBottom(false);
      break;
    case GUI_ACTION_PAT_CURSOR_UP_COARSE:
      moveCursor(0,-editStepCoarse,false);
      break;
    case GUI_ACTION_PAT_CURSOR_DOWN_COARSE:
      moveCursor(0,editStepCoarse,false);
      break;
    case GUI_ACTION_PAT_SELECTION_UP:
      moveCursor(0,-MAX(1,settings.scrollStep?editStep:1),true);
      break;
    case GUI_ACTION_PAT_SELECTION_DOWN:
      moveCursor(0,MAX(1,settings.scrollStep?editStep:1),true);
      break;
    case GUI_ACTION_PAT_SELECTION_LEFT:
      moveCursor(-1,0,true);
      break;
    case GUI_ACTION_PAT_SELECTION_RIGHT:
      moveCursor(1,0,true);
      break;
    case GUI_ACTION_PAT_SELECTION_UP_ONE:
      moveCursor(0,-1,true);
      break;
    case GUI_ACTION_PAT_SELECTION_DOWN_ONE:
      moveCursor(0,1,true);
      break;
    case GUI_ACTION_PAT_SELECTION_BEGIN:
      moveCursorTop(true);
      break;
    case GUI_ACTION_PAT_SELECTION_END:
      moveCursorBottom(true);
      break;
    case GUI_ACTION_PAT_SELECTION_UP_COARSE:
      moveCursor(0,-editStepCoarse,true);
      break;
    case GUI_ACTION_PAT_SELECTION_DOWN_COARSE:
      moveCursor(0,editStepCoarse,true);
      break;
    case GUI_ACTION_PAT_MOVE_UP:
      moveSelected(0,-1);
      break;
    case GUI_ACTION_PAT_MOVE_DOWN:
      moveSelected(0,1);
      break;
    case GUI_ACTION_PAT_MOVE_LEFT_CHANNEL:
      moveSelected(-1,0);
      break;
    case GUI_ACTION_PAT_MOVE_RIGHT_CHANNEL:
      moveSelected(1,0);
      break;
    case GUI_ACTION_PAT_DELETE:
      doDelete();
      if (settings.stepOnDelete) {
        moveCursor(0,editStep,false);
      }
      break;
    case GUI_ACTION_PAT_PULL_DELETE:
      doPullDelete();
      break;
    case GUI_ACTION_PAT_INSERT:
      doInsert();
      if (settings.stepOnInsert) {
        moveCursor(0,editStep,false);
      }
      break;
    case GUI_ACTION_PAT_MUTE_CURSOR:
      if (cursor.xCoarse<0 || cursor.xCoarse>=e->getTotalChannelCount()) break;
      e->toggleMute(cursor.xCoarse);
      break;
    case GUI_ACTION_PAT_SOLO_CURSOR:
      if (cursor.xCoarse<0 || cursor.xCoarse>=e->getTotalChannelCount()) break;
      e->toggleSolo(cursor.xCoarse);
      break;
    case GUI_ACTION_PAT_UNMUTE_ALL:
      e->unmuteAll();
      break;
    case GUI_ACTION_PAT_NEXT_ORDER:
      if (curOrder<e->curSubSong->ordersLen-1) {
        setOrder(curOrder+1);
      }
      break;
    case GUI_ACTION_PAT_PREV_ORDER:
      if (curOrder>0) {
        setOrder(curOrder-1);
      }
      break;
    case GUI_ACTION_PAT_COLLAPSE:
      if (cursor.xCoarse<0 || cursor.xCoarse>=e->getTotalChannelCount()) break;
      if (e->curSubSong->chanCollapse[cursor.xCoarse]==0) {
        e->curSubSong->chanCollapse[cursor.xCoarse]=3;
      } else if (e->curSubSong->chanCollapse[cursor.xCoarse]>0) {
        e->curSubSong->chanCollapse[cursor.xCoarse]--;
      }
      break;
    case GUI_ACTION_PAT_INCREASE_COLUMNS:
      if (cursor.xCoarse<0 || cursor.xCoarse>=e->getTotalChannelCount()) break;
      e->curPat[cursor.xCoarse].effectCols++;
              if (e->curPat[cursor.xCoarse].effectCols>DIV_MAX_EFFECTS) e->curPat[cursor.xCoarse].effectCols=DIV_MAX_EFFECTS;
      break;
    case GUI_ACTION_PAT_DECREASE_COLUMNS:
      if (cursor.xCoarse<0 || cursor.xCoarse>=e->getTotalChannelCount()) break;
      e->curPat[cursor.xCoarse].effectCols--;
      if (e->curPat[cursor.xCoarse].effectCols<1) e->curPat[cursor.xCoarse].effectCols=1;
      break;
    case GUI_ACTION_PAT_INTERPOLATE:
      doInterpolate();
      break;
    case GUI_ACTION_PAT_INVERT_VALUES:
      doInvertValues();
      break;
    case GUI_ACTION_PAT_FLIP_SELECTION:
      doFlip();
      break;
    case GUI_ACTION_PAT_COLLAPSE_ROWS:
      doCollapse(collapseAmount,selStart,selEnd);
      break;
    case GUI_ACTION_PAT_EXPAND_ROWS:
      doExpand(collapseAmount,selStart,selEnd);
      break;
    case GUI_ACTION_PAT_COLLAPSE_PAT: {
      SelectionPoint selEndPat;
      selEndPat.xCoarse=e->getTotalChannelCount()-1;
      selEndPat.xFine=2+e->curPat[selEndPat.xCoarse].effectCols*2;
      selEndPat.y=e->curSubSong->patLen-1;
      doCollapse(collapseAmount,SelectionPoint(0,0,0),selEndPat);
      break;
    }
    case GUI_ACTION_PAT_EXPAND_PAT: {
      SelectionPoint selEndPat;
      selEndPat.xCoarse=e->getTotalChannelCount()-1;
      selEndPat.xFine=2+e->curPat[selEndPat.xCoarse].effectCols*2;
      selEndPat.y=e->curSubSong->patLen-1;
      doExpand(collapseAmount,SelectionPoint(0,0,0),selEndPat);
      break;
    }
    case GUI_ACTION_PAT_COLLAPSE_SONG:
      doCollapseSong(collapseAmount);
      break;
    case GUI_ACTION_PAT_EXPAND_SONG:
      doExpandSong(collapseAmount);
      break;
    case GUI_ACTION_PAT_LATCH: {
      DivPattern* pat=e->curPat[cursor.xCoarse].getPattern(e->curOrders->ord[cursor.xCoarse][curOrder],true);
      latchIns=pat->data[cursor.y][2];
      latchVol=pat->data[cursor.y][3];
      latchEffect=pat->data[cursor.y][4];
      latchEffectVal=pat->data[cursor.y][5];
      latchTarget=0;
      latchNibble=false;
      break;
    }
    case GUI_ACTION_PAT_CLEAR_LATCH:
      latchIns=-2;
      latchVol=-1;
      latchEffect=-1;
      latchEffectVal=-1;
      latchTarget=0;
      latchNibble=false;
      break;
    case GUI_ACTION_PAT_ABSORB_INSTRUMENT: {
      DivPattern* pat=e->curPat[cursor.xCoarse].getPattern(e->curOrders->ord[cursor.xCoarse][curOrder],false);
      if (!pat) break;
      for (int i=cursor.y; i>=0; i--) {
        if (pat->data[i][2] >= 0) {
          curIns=pat->data[i][2];
          break;
        }
      }
      break;
    }

    case GUI_ACTION_INS_LIST_ADD:
      if (settings.insTypeMenu) {
        makeInsTypeList=e->getPossibleInsTypes();
        if (makeInsTypeList.size()>1) {
          displayInsTypeList=true;
          displayInsTypeListMakeInsSample=-1;
          break;
        }
      }
      curIns=e->addInstrument(cursor.xCoarse);
      if (curIns==-1) {
        showError(_("too many instruments!"));
      } else {
        if (settings.blankIns) {
          e->song.ins[curIns]->fm.fb=0;
          for (int i=0; i<4; i++) {
            e->song.ins[curIns]->fm.op[i]=DivInstrumentFM::Operator();
            e->song.ins[curIns]->fm.op[i].ar=31;
            e->song.ins[curIns]->fm.op[i].dr=31;
            e->song.ins[curIns]->fm.op[i].rr=15;
            e->song.ins[curIns]->fm.op[i].tl=127;
            e->song.ins[curIns]->fm.op[i].dt=3;

            e->song.ins[curIns]->esfm.op[i].ct=0;
            e->song.ins[curIns]->esfm.op[i].dt=0;
            e->song.ins[curIns]->esfm.op[i].modIn=0;
            e->song.ins[curIns]->esfm.op[i].outLvl=0;
          }
        }
        wantScrollListIns=true;
        MARK_MODIFIED;
        wavePreviewInit=true;
        updateFMPreview=true;
      }
      break;
    case GUI_ACTION_INS_LIST_DUPLICATE:
      if (curIns>=0 && curIns<(int)e->song.ins.size()) {
        int prevIns=curIns;
        curIns=e->addInstrument(cursor.xCoarse);
        if (curIns==-1) {
          showError(_("too many instruments!"));
        } else {
          (*e->song.ins[curIns])=(*e->song.ins[prevIns]);
          wantScrollListIns=true;
          MARK_MODIFIED;
          wavePreviewInit=true;
          updateFMPreview=true;
        }
      }
      break;
    case GUI_ACTION_INS_LIST_OPEN:
      openFileDialog(GUI_FILE_INS_OPEN);
      break;
    case GUI_ACTION_INS_LIST_OPEN_REPLACE:
      openFileDialog(GUI_FILE_INS_OPEN_REPLACE);
      break;
    case GUI_ACTION_INS_LIST_SAVE:
      if (curIns>=0 && curIns<(int)e->song.ins.size()) openFileDialog(GUI_FILE_INS_SAVE);
      break;
    case GUI_ACTION_INS_LIST_SAVE_DMP:
      if (curIns>=0 && curIns<(int)e->song.ins.size()) openFileDialog(GUI_FILE_INS_SAVE_DMP);
      break;
    case GUI_ACTION_INS_LIST_MOVE_UP:
      if (e->moveInsUp(curIns)) {
        curIns--;
        wantScrollListIns=true;
        MARK_MODIFIED;
      }
      break;
    case GUI_ACTION_INS_LIST_MOVE_DOWN:
      if (e->moveInsDown(curIns)) {
        curIns++;
        wantScrollListIns=true;
        MARK_MODIFIED;
      }
      break;
    case GUI_ACTION_INS_LIST_DELETE:
      if (curIns>=0 && curIns<(int)e->song.ins.size()) {
        e->delInstrument(curIns);
        wantScrollListIns=true;
        MARK_MODIFIED;
        if (curIns>=(int)e->song.ins.size()) {
          curIns--;
        }
      }
      break;
    case GUI_ACTION_INS_LIST_EDIT:
      insEditOpen=true;
      break;
    case GUI_ACTION_INS_LIST_UP:
      if (--curIns<0) curIns=0;
      wantScrollListIns=true;
      wavePreviewInit=true;
      updateFMPreview=true;
      break;
    case GUI_ACTION_INS_LIST_DOWN:
      if (++curIns>=(int)e->song.ins.size()) curIns=((int)e->song.ins.size())-1;
      wantScrollListIns=true;
      wavePreviewInit=true;
      updateFMPreview=true;
      break;
    case GUI_ACTION_INS_LIST_DIR_VIEW:
      insListDir=!insListDir;
      break;
    case GUI_ACTION_INS_LIST_SAVE_ALL:
      if (e->song.ins.empty()) {
        showError(_("this song doesn't have any instruments."));
      } else {
        openFileDialog(GUI_FILE_INS_SAVE_ALL);
      }
      break;

    case GUI_ACTION_WAVE_LIST_ADD: {
      std::vector<DivSystem> alreadyDone;
      waveSizeList.clear();
      for (int i=0; i<e->song.systemLen; i++) {
        bool skip=false;
        for (DivSystem j: alreadyDone) {
          if (e->song.system[i]==j) {
            skip=true;
            break;
          }
        }
        if (skip) continue;
        const DivSysDef* sysDef=e->getSystemDef(e->song.system[i]);
        alreadyDone.push_back(e->song.system[i]);
        if (sysDef==NULL) continue;

        if (sysDef->waveHeight==0) continue;
        if (sysDef->waveWidth==0) {
          // add three preset sizes
          waveSizeList.push_back(FurnaceGUIWaveSizeEntry(32,sysDef->waveHeight,sysDef->name));
          waveSizeList.push_back(FurnaceGUIWaveSizeEntry(64,sysDef->waveHeight,sysDef->name));
          waveSizeList.push_back(FurnaceGUIWaveSizeEntry(128,sysDef->waveHeight,sysDef->name));
        } else {
          waveSizeList.push_back(FurnaceGUIWaveSizeEntry(sysDef->waveWidth,sysDef->waveHeight,sysDef->name));
        }
      }

      int finalWidth=32;
      int finalHeight=32;
      if (waveSizeList.size()==1) {
        finalWidth=waveSizeList[0].width;
        finalHeight=waveSizeList[0].height;
      } else if (waveSizeList.size()>1) {
        displayWaveSizeList=true;
        break;
      }

      curWave=e->addWave();
      if (curWave==-1) {
        showError(_("too many wavetables!"));
      } else {
        wantScrollListWave=true;
        e->song.wave[curWave]->len=finalWidth;
        e->song.wave[curWave]->max=finalHeight-1;
        for (int j=0; j<finalWidth; j++) {
          e->song.wave[curWave]->data[j]=(j*finalHeight)/finalWidth;
        }
        MARK_MODIFIED;
        RESET_WAVE_MACRO_ZOOM;
      }
      break;
    }
    case GUI_ACTION_WAVE_LIST_DUPLICATE:
      if (curWave>=0 && curWave<(int)e->song.wave.size()) {
        int prevWave=curWave;
        curWave=e->addWave();
        if (curWave==-1) {
          showError(_("too many wavetables!"));
        } else {
          (*e->song.wave[curWave])=(*e->song.wave[prevWave]);
          wantScrollListWave=true;
          MARK_MODIFIED;
          RESET_WAVE_MACRO_ZOOM;
        }
      }
      break;
    case GUI_ACTION_WAVE_LIST_OPEN:
      openFileDialog(GUI_FILE_WAVE_OPEN);
      break;
    case GUI_ACTION_WAVE_LIST_OPEN_REPLACE:
      openFileDialog(GUI_FILE_WAVE_OPEN_REPLACE);
      break;
    case GUI_ACTION_WAVE_LIST_SAVE:
      if (curWave>=0 && curWave<(int)e->song.wave.size()) openFileDialog(GUI_FILE_WAVE_SAVE);
      break;
    case GUI_ACTION_WAVE_LIST_SAVE_DMW:
      if (curWave>=0 && curWave<(int)e->song.wave.size()) openFileDialog(GUI_FILE_WAVE_SAVE_DMW);
      break;
    case GUI_ACTION_WAVE_LIST_SAVE_RAW:
      if (curWave>=0 && curWave<(int)e->song.wave.size()) openFileDialog(GUI_FILE_WAVE_SAVE_RAW);
      break;
    case GUI_ACTION_WAVE_LIST_MOVE_UP:
      if (e->moveWaveUp(curWave)) {
        curWave--;
        wantScrollListWave=true;
        MARK_MODIFIED;
      }
      break;
    case GUI_ACTION_WAVE_LIST_MOVE_DOWN:
      if (e->moveWaveDown(curWave)) {
        curWave++;
        wantScrollListWave=true;
        MARK_MODIFIED;
      }
      break;
    case GUI_ACTION_WAVE_LIST_DELETE:
      if (curWave>=0 && curWave<(int)e->song.wave.size()) {
        e->delWave(curWave);
        MARK_MODIFIED;
        wantScrollListWave=true;
        if (curWave>=(int)e->song.wave.size()) {
          curWave--;
        }
      }
      break;
    case GUI_ACTION_WAVE_LIST_EDIT:
      waveEditOpen=true;
      break;
    case GUI_ACTION_WAVE_LIST_UP:
      if (--curWave<0) curWave=0;
      wantScrollListWave=true;
      break;
    case GUI_ACTION_WAVE_LIST_DOWN:
      if (++curWave>=(int)e->song.wave.size()) curWave=((int)e->song.wave.size())-1;
      wantScrollListWave=true;
      break;
    case GUI_ACTION_WAVE_LIST_DIR_VIEW:
      waveListDir=!waveListDir;
      break;
    case GUI_ACTION_WAVE_LIST_SAVE_ALL:
      if (e->song.wave.empty()) {
        showError(_("this song doesn't have any wavetables."));
      } else {
        openFileDialog(GUI_FILE_WAVE_SAVE_ALL);
      }
      break;

    case GUI_ACTION_SAMPLE_LIST_ADD:
      curSample=e->addSample();
      if (curSample==-1) {
        showError(_("too many samples!"));
      } else {
        wantScrollListSample=true;
        MARK_MODIFIED;
      }
      updateSampleTex=true;
      break;
    case GUI_ACTION_SAMPLE_LIST_DUPLICATE:
      if (curSample>=0 && curSample<(int)e->song.sample.size()) {
        DivSample* prevSample=e->getSample(curSample);
        curSample=e->addSample();
        if (curSample==-1) {
          showError(_("too many samples!"));
        } else {
          e->lockEngine([this,prevSample]() {
            DivSample* sample=e->getSample(curSample);
            if (sample!=NULL) {
              sample->rate=prevSample->rate;
              sample->centerRate=prevSample->centerRate;
              sample->name=prevSample->name;
              sample->loopStart=prevSample->loopStart;
              sample->loopEnd=prevSample->loopEnd;
              sample->loop=prevSample->loop;
              sample->loopMode=prevSample->loopMode;
              sample->brrEmphasis=prevSample->brrEmphasis;
              sample->brrNoFilter=prevSample->brrNoFilter;
              sample->dither=prevSample->dither;
              sample->depth=prevSample->depth;
              if (sample->init(prevSample->samples)) {
                if (prevSample->getCurBuf()!=NULL) {
                  memcpy(sample->getCurBuf(),prevSample->getCurBuf(),prevSample->getCurBufLen());
                }
              }
            }
            e->renderSamples();
          });
          wantScrollListSample=true;
          MARK_MODIFIED;
        }
        updateSampleTex=true;
      }
      break;
    case GUI_ACTION_SAMPLE_LIST_OPEN:
      openFileDialog(GUI_FILE_SAMPLE_OPEN);
      break;
    case GUI_ACTION_SAMPLE_LIST_OPEN_REPLACE:
      openFileDialog(GUI_FILE_SAMPLE_OPEN_REPLACE);
      break;
    case GUI_ACTION_SAMPLE_LIST_OPEN_RAW:
      openFileDialog(GUI_FILE_SAMPLE_OPEN_RAW);
      break;
    case GUI_ACTION_SAMPLE_LIST_OPEN_REPLACE_RAW:
      openFileDialog(GUI_FILE_SAMPLE_OPEN_REPLACE_RAW);
      break;
    case GUI_ACTION_SAMPLE_LIST_SAVE:
      if (curSample>=0 && curSample<(int)e->song.sample.size()) openFileDialog(GUI_FILE_SAMPLE_SAVE);
      break;
    case GUI_ACTION_SAMPLE_LIST_SAVE_RAW:
      if (curSample>=0 && curSample<(int)e->song.sample.size()) openFileDialog(GUI_FILE_SAMPLE_SAVE_RAW);
      break;
    case GUI_ACTION_SAMPLE_LIST_MOVE_UP:
      if (e->moveSampleUp(curSample)) {
        curSample--;
        wantScrollListSample=true;
        updateSampleTex=true;
        MARK_MODIFIED;
      }
      break;
    case GUI_ACTION_SAMPLE_LIST_MOVE_DOWN:
      if (e->moveSampleDown(curSample)) {
        curSample++;
        wantScrollListSample=true;
        updateSampleTex=true;
        MARK_MODIFIED;
      }
      break;
    case GUI_ACTION_SAMPLE_LIST_DELETE:
      e->delSample(curSample);
      wantScrollListSample=true;
      MARK_MODIFIED;
      if (curSample>=(int)e->song.sample.size()) {
        curSample--;
      }
      updateSampleTex=true;
      break;
    case GUI_ACTION_SAMPLE_LIST_EDIT:
      sampleEditOpen=true;
      break;
    case GUI_ACTION_SAMPLE_LIST_UP:
      if (--curSample<0) curSample=0;
      wantScrollListSample=true;
      updateSampleTex=true;
      break;
    case GUI_ACTION_SAMPLE_LIST_DOWN:
      if (++curSample>=(int)e->song.sample.size()) curSample=((int)e->song.sample.size())-1;
      wantScrollListSample=true;
      updateSampleTex=true;
      break;
    case GUI_ACTION_SAMPLE_LIST_PREVIEW:
      e->previewSample(curSample);
      break;
    case GUI_ACTION_SAMPLE_LIST_STOP_PREVIEW:
      e->stopSamplePreview();
      break;
    case GUI_ACTION_SAMPLE_LIST_DIR_VIEW:
      sampleListDir=!sampleListDir;
      break;
    case GUI_ACTION_SAMPLE_LIST_MAKE_MAP: {
      // determine instrument type
      std::vector<DivInstrumentType> tempTypeList=e->getPossibleInsTypes();
      makeInsTypeList.clear();

      for (DivInstrumentType& i: tempTypeList) {
        if (i==DIV_INS_PCE ||
            i==DIV_INS_MSM6258 ||
            i==DIV_INS_MSM6295 ||
            i==DIV_INS_ADPCMA ||
            i==DIV_INS_ADPCMB ||
            i==DIV_INS_SEGAPCM ||
            i==DIV_INS_QSOUND ||
            i==DIV_INS_YMZ280B ||
            i==DIV_INS_RF5C68 ||
            i==DIV_INS_MULTIPCM ||
            i==DIV_INS_MIKEY ||
            i==DIV_INS_X1_010 ||
            i==DIV_INS_SWAN ||
            i==DIV_INS_AY ||
            i==DIV_INS_AY8930 ||
            i==DIV_INS_VRC6 ||
            i==DIV_INS_SU ||
            i==DIV_INS_SNES ||
            i==DIV_INS_ES5506 ||
            i==DIV_INS_K007232 ||
            i==DIV_INS_GA20 ||
            i==DIV_INS_K053260 ||
            i==DIV_INS_C140 ||
            i==DIV_INS_C219 ||
            i==DIV_INS_NDS) {
          makeInsTypeList.push_back(i);
        }
      }

      if (makeInsTypeList.empty()) {
        makeInsTypeList.push_back(DIV_INS_AMIGA);
      }

      displayInsTypeList=true;
      displayInsTypeListMakeInsSample=-2;
      break;
    }
    case GUI_ACTION_SAMPLE_LIST_SAVE_ALL:
      if (e->song.sample.empty()) {
        showError(_("this song doesn't have any samples."));
      } else {
        openFileDialog(GUI_FILE_SAMPLE_SAVE_ALL);
      }
      break;

    case GUI_ACTION_SAMPLE_SELECT:
      if (curSample<0 || curSample>=(int)e->song.sample.size()) break;
      sampleDragMode=false;
      break;
    case GUI_ACTION_SAMPLE_DRAW:
      if (curSample<0 || curSample>=(int)e->song.sample.size()) break;
      sampleDragMode=true;
      break;
    case GUI_ACTION_SAMPLE_CUT: {
      if (curSample<0 || curSample>=(int)e->song.sample.size()) break;
      DivSample* sample=e->song.sample[curSample];
      if (sample->depth!=DIV_SAMPLE_DEPTH_8BIT && sample->depth!=DIV_SAMPLE_DEPTH_16BIT) break;
      SAMPLE_OP_BEGIN;

      if (end-start<1) break;

      sample->prepareUndo(true);

      if (sampleClipboard!=NULL) {
        delete[] sampleClipboard;
      }
      sampleClipboard=new short[end-start];
      sampleClipboardLen=end-start;
      memcpy(sampleClipboard,&(sample->data16[start]),sizeof(short)*(end-start));

      e->lockEngine([this,sample,start,end]() {
        sample->strip(start,end);
        updateSampleTex=true;

        e->renderSamples(curSample);
      });
      sampleSelStart=-1;
      sampleSelEnd=-1;
      MARK_MODIFIED;

      break;
    }
    case GUI_ACTION_SAMPLE_COPY: {
      if (curSample<0 || curSample>=(int)e->song.sample.size()) break;
      DivSample* sample=e->song.sample[curSample];
      SAMPLE_OP_BEGIN;

      if (end-start<1) break;

      if (sampleClipboard!=NULL) {
        delete[] sampleClipboard;
      }
      sampleClipboard=new short[end-start];
      sampleClipboardLen=end-start;
      memcpy(sampleClipboard,&(sample->data16[start]),sizeof(short)*(end-start));
      break;
    }
    case GUI_ACTION_SAMPLE_PASTE: {
      if (curSample<0 || curSample>=(int)e->song.sample.size()) break;
      if (sampleClipboard==NULL || sampleClipboardLen<1) break;
      DivSample* sample=e->song.sample[curSample];
      if (sample->depth!=DIV_SAMPLE_DEPTH_8BIT && sample->depth!=DIV_SAMPLE_DEPTH_16BIT) break;
      sample->prepareUndo(true);
      int pos=(sampleSelStart==-1 || sampleSelStart==sampleSelEnd)?sample->samples:sampleSelStart;
      if (pos>=(int)sample->samples) pos=sample->samples-1;
      if (pos<0) pos=0;
      logV("paste position: %d",pos);

      e->lockEngine([this,sample,pos]() {
        if (!sample->insert(pos,sampleClipboardLen)) {
          showError(_("couldn't paste! make sure your sample is 8 or 16-bit."));
        } else {
          if (sample->depth==DIV_SAMPLE_DEPTH_8BIT) {
            for (size_t i=0; i<sampleClipboardLen; i++) {
              sample->data8[pos+i]=sampleClipboard[i]>>8;
            }
          } else {
            memcpy(&(sample->data16[pos]),sampleClipboard,sizeof(short)*sampleClipboardLen);
          }
        }
        e->renderSamples(curSample);
      });
      sampleSelStart=pos;
      sampleSelEnd=pos+sampleClipboardLen;
      updateSampleTex=true;
      MARK_MODIFIED;
      break;
    }
    case GUI_ACTION_SAMPLE_PASTE_REPLACE: {
      if (curSample<0 || curSample>=(int)e->song.sample.size()) break;
      if (sampleClipboard==NULL || sampleClipboardLen<1) break;
      DivSample* sample=e->song.sample[curSample];
      if (sample->depth!=DIV_SAMPLE_DEPTH_8BIT && sample->depth!=DIV_SAMPLE_DEPTH_16BIT) break;
      sample->prepareUndo(true);
      int pos=(sampleSelStart==-1 || sampleSelStart==sampleSelEnd)?0:sampleSelStart;
      if (pos>=(int)sample->samples) pos=sample->samples-1;
      if (pos<0) pos=0;

      e->lockEngine([this,sample,pos]() {
        if (sample->depth==DIV_SAMPLE_DEPTH_8BIT) {
          for (size_t i=0; i<sampleClipboardLen; i++) {
            if (pos+i>=sample->samples) break;
            sample->data8[pos+i]=sampleClipboard[i]>>8;
          }
        } else {
          for (size_t i=0; i<sampleClipboardLen; i++) {
            if (pos+i>=sample->samples) break;
            sample->data16[pos+i]=sampleClipboard[i];
          }
        }
        e->renderSamples(curSample);
      });
      sampleSelStart=pos;
      sampleSelEnd=pos+sampleClipboardLen;
      if (sampleSelEnd>(int)sample->samples) sampleSelEnd=sample->samples;
      updateSampleTex=true;
      MARK_MODIFIED;
      break;
    }
    case GUI_ACTION_SAMPLE_PASTE_MIX: {
      if (curSample<0 || curSample>=(int)e->song.sample.size()) break;
      if (sampleClipboard==NULL || sampleClipboardLen<1) break;
      DivSample* sample=e->song.sample[curSample];
      if (sample->depth!=DIV_SAMPLE_DEPTH_8BIT && sample->depth!=DIV_SAMPLE_DEPTH_16BIT) break;
      sample->prepareUndo(true);
      int pos=(sampleSelStart==-1 || sampleSelStart==sampleSelEnd)?0:sampleSelStart;
      if (pos>=(int)sample->samples) pos=sample->samples-1;
      if (pos<0) pos=0;

      e->lockEngine([this,sample,pos]() {
        if (sample->depth==DIV_SAMPLE_DEPTH_8BIT) {
          for (size_t i=0; i<sampleClipboardLen; i++) {
            if (pos+i>=sample->samples) break;
            int val=sample->data8[pos+i]+(sampleClipboard[i]>>8);
            if (val>127) val=127;
            if (val<-128) val=-128;
            sample->data8[pos+i]=val;
          }
        } else {
          for (size_t i=0; i<sampleClipboardLen; i++) {
            if (pos+i>=sample->samples) break;
            int val=sample->data16[pos+i]+sampleClipboard[i];
            if (val>32767) val=32767;
            if (val<-32768) val=-32768;
            sample->data16[pos+i]=val;
          }
        }
        e->renderSamples(curSample);
      });
      sampleSelStart=pos;
      sampleSelEnd=pos+sampleClipboardLen;
      if (sampleSelEnd>(int)sample->samples) sampleSelEnd=sample->samples;
      updateSampleTex=true;
      MARK_MODIFIED;
      break;
    }
    case GUI_ACTION_SAMPLE_SELECT_ALL: {
      if (curSample<0 || curSample>=(int)e->song.sample.size()) break;
      DivSample* sample=e->song.sample[curSample];
      sampleDragActive=false;
      sampleSelStart=0;
      sampleSelEnd=sample->samples;
      break;
    }
    case GUI_ACTION_SAMPLE_RESIZE:
      if (curSample<0 || curSample>=(int)e->song.sample.size()) break;
      openSampleResizeOpt=true;
      break;
    case GUI_ACTION_SAMPLE_RESAMPLE:
      if (curSample<0 || curSample>=(int)e->song.sample.size()) break;
      openSampleResampleOpt=true;
      break;
    case GUI_ACTION_SAMPLE_AMPLIFY:
      if (curSample<0 || curSample>=(int)e->song.sample.size()) break;
      openSampleAmplifyOpt=true;
      break;
    case GUI_ACTION_SAMPLE_NORMALIZE: {
      if (curSample<0 || curSample>=(int)e->song.sample.size()) break;
      DivSample* sample=e->song.sample[curSample];
      if (sample->depth!=DIV_SAMPLE_DEPTH_8BIT && sample->depth!=DIV_SAMPLE_DEPTH_16BIT) break;
      sample->prepareUndo(true);
      e->lockEngine([this,sample]() {
        SAMPLE_OP_BEGIN;
        float maxVal=0.0f;

        if (sample->depth==DIV_SAMPLE_DEPTH_16BIT) {
          for (unsigned int i=start; i<end; i++) {
            float val=fabs((float)sample->data16[i]/32767.0f);
            if (val>maxVal) maxVal=val;
          }
          if (maxVal>1.0f) maxVal=1.0f;
          if (maxVal>0.0f) {
            float vol=1.0f/maxVal;
            for (unsigned int i=start; i<end; i++) {
              float val=sample->data16[i]*vol;
              if (val<-32768) val=-32768;
              if (val>32767) val=32767;
              sample->data16[i]=val;
            }
          }
        } else if (sample->depth==DIV_SAMPLE_DEPTH_8BIT) {
          for (unsigned int i=start; i<end; i++) {
            float val=fabs((float)sample->data8[i]/127.0f);
            if (val>maxVal) maxVal=val;
          }
          if (maxVal>1.0f) maxVal=1.0f;
          if (maxVal>0.0f) {
            float vol=1.0f/maxVal;
            for (unsigned int i=start; i<end; i++) {
              float val=sample->data8[i]*vol;
              if (val<-128) val=-128;
              if (val>127) val=127;
              sample->data8[i]=val;
            }
          }
        }

        updateSampleTex=true;

        e->renderSamples(curSample);
      });
      MARK_MODIFIED;
      break;
    }
    case GUI_ACTION_SAMPLE_FADE_IN: {
      if (curSample<0 || curSample>=(int)e->song.sample.size()) break;
      DivSample* sample=e->song.sample[curSample];
      if (sample->depth!=DIV_SAMPLE_DEPTH_8BIT && sample->depth!=DIV_SAMPLE_DEPTH_16BIT) break;
      sample->prepareUndo(true);
      e->lockEngine([this,sample]() {
        SAMPLE_OP_BEGIN;

        if (sample->depth==DIV_SAMPLE_DEPTH_16BIT) {
          for (unsigned int i=start; i<end; i++) {
            float val=sample->data16[i]*float(i-start)/float(end-start);
            if (val<-32768) val=-32768;
            if (val>32767) val=32767;
            sample->data16[i]=val;
          }
        } else if (sample->depth==DIV_SAMPLE_DEPTH_8BIT) {
          for (unsigned int i=start; i<end; i++) {
            float val=sample->data8[i]*float(i-start)/float(end-start);
            if (val<-128) val=-128;
            if (val>127) val=127;
            sample->data8[i]=val;
          }
        }

        updateSampleTex=true;

        e->renderSamples(curSample);
      });
      MARK_MODIFIED;
      break;
    }
    case GUI_ACTION_SAMPLE_FADE_OUT: {
      if (curSample<0 || curSample>=(int)e->song.sample.size()) break;
      DivSample* sample=e->song.sample[curSample];
      if (sample->depth!=DIV_SAMPLE_DEPTH_8BIT && sample->depth!=DIV_SAMPLE_DEPTH_16BIT) break;
      sample->prepareUndo(true);
      e->lockEngine([this,sample]() {
        SAMPLE_OP_BEGIN;

        if (sample->depth==DIV_SAMPLE_DEPTH_16BIT) {
          for (unsigned int i=start; i<end; i++) {
            float val=sample->data16[i]*float(end-i)/float(end-start);
            if (val<-32768) val=-32768;
            if (val>32767) val=32767;
            sample->data16[i]=val;
          }
        } else if (sample->depth==DIV_SAMPLE_DEPTH_8BIT) {
          for (unsigned int i=start; i<end; i++) {
            float val=sample->data8[i]*float(end-i)/float(end-start);
            if (val<-128) val=-128;
            if (val>127) val=127;
            sample->data8[i]=val;
          }
        }

        updateSampleTex=true;

        e->renderSamples(curSample);
      });
      MARK_MODIFIED;
      break;
    }
    case GUI_ACTION_SAMPLE_INSERT:
      if (curSample<0 || curSample>=(int)e->song.sample.size()) break;
      openSampleSilenceOpt=true;
      break;
    case GUI_ACTION_SAMPLE_SILENCE: {
      if (curSample<0 || curSample>=(int)e->song.sample.size()) break;
      DivSample* sample=e->song.sample[curSample];
      if (sample->depth!=DIV_SAMPLE_DEPTH_8BIT && sample->depth!=DIV_SAMPLE_DEPTH_16BIT) break;
      sample->prepareUndo(true);
      e->lockEngine([this,sample]() {
        SAMPLE_OP_BEGIN;

        if (sample->depth==DIV_SAMPLE_DEPTH_16BIT) {
          for (unsigned int i=start; i<end; i++) {
            sample->data16[i]=0;
          }
        } else if (sample->depth==DIV_SAMPLE_DEPTH_8BIT) {
          for (unsigned int i=start; i<end; i++) {
            sample->data8[i]=0;
          }
        }

        updateSampleTex=true;

        e->renderSamples(curSample);
      });
      MARK_MODIFIED;
      break;
    }
    case GUI_ACTION_SAMPLE_DELETE: {
      if (curSample<0 || curSample>=(int)e->song.sample.size()) break;
      DivSample* sample=e->song.sample[curSample];
      if (sample->depth!=DIV_SAMPLE_DEPTH_8BIT && sample->depth!=DIV_SAMPLE_DEPTH_16BIT) break;
      sample->prepareUndo(true);
      e->lockEngine([this,sample]() {
        SAMPLE_OP_BEGIN;

        sample->strip(start,end);
        updateSampleTex=true;

        e->renderSamples(curSample);
      });
      sampleSelStart=-1;
      sampleSelEnd=-1;
      MARK_MODIFIED;
      break;
    }
    case GUI_ACTION_SAMPLE_TRIM: {
      if (curSample<0 || curSample>=(int)e->song.sample.size()) break;
      DivSample* sample=e->song.sample[curSample];
      if (sample->depth!=DIV_SAMPLE_DEPTH_8BIT && sample->depth!=DIV_SAMPLE_DEPTH_16BIT) break;
      sample->prepareUndo(true);
      e->lockEngine([this,sample]() {
        SAMPLE_OP_BEGIN;

        sample->trim(start,end);
        updateSampleTex=true;

        e->renderSamples(curSample);
      });
      sampleSelStart=-1;
      sampleSelEnd=-1;
      MARK_MODIFIED;
      break;
    }
    case GUI_ACTION_SAMPLE_REVERSE: {
      if (curSample<0 || curSample>=(int)e->song.sample.size()) break;
      DivSample* sample=e->song.sample[curSample];
      if (sample->depth!=DIV_SAMPLE_DEPTH_8BIT && sample->depth!=DIV_SAMPLE_DEPTH_16BIT) break;
      sample->prepareUndo(true);
      e->lockEngine([this,sample]() {
        SAMPLE_OP_BEGIN;

        if (sample->depth==DIV_SAMPLE_DEPTH_16BIT) {
          for (unsigned int i=start; i<end; i++) {
            unsigned int ri=end-i-1+start;
            if (ri<=i) break;
            sample->data16[i]^=sample->data16[ri];
            sample->data16[ri]^=sample->data16[i];
            sample->data16[i]^=sample->data16[ri];
          }
        } else if (sample->depth==DIV_SAMPLE_DEPTH_8BIT) {
          for (unsigned int i=start; i<end; i++) {
            unsigned int ri=end-i-1+start;
            if (ri<=i) break;
            sample->data8[i]^=sample->data8[ri];
            sample->data8[ri]^=sample->data8[i];
            sample->data8[i]^=sample->data8[ri];
          }
        }

        updateSampleTex=true;

        e->renderSamples(curSample);
      });
      MARK_MODIFIED;
      break;
    }
    case GUI_ACTION_SAMPLE_INVERT: {
      if (curSample<0 || curSample>=(int)e->song.sample.size()) break;
      DivSample* sample=e->song.sample[curSample];
      if (sample->depth!=DIV_SAMPLE_DEPTH_8BIT && sample->depth!=DIV_SAMPLE_DEPTH_16BIT) break;
      sample->prepareUndo(true);
      e->lockEngine([this,sample]() {
        SAMPLE_OP_BEGIN;

        if (sample->depth==DIV_SAMPLE_DEPTH_16BIT) {
          for (unsigned int i=start; i<end; i++) {
            sample->data16[i]=-sample->data16[i];
            if (sample->data16[i]==-32768) sample->data16[i]=32767;
          }
        } else if (sample->depth==DIV_SAMPLE_DEPTH_8BIT) {
          for (unsigned int i=start; i<end; i++) {
            sample->data8[i]=-sample->data8[i];
            if (sample->data8[i]==-128) sample->data8[i]=127;
          }
        }

        updateSampleTex=true;

        e->renderSamples(curSample);
      });
      MARK_MODIFIED;
      break;
    }
    case GUI_ACTION_SAMPLE_SIGN: {
      if (curSample<0 || curSample>=(int)e->song.sample.size()) break;
      DivSample* sample=e->song.sample[curSample];
      if (sample->depth!=DIV_SAMPLE_DEPTH_8BIT && sample->depth!=DIV_SAMPLE_DEPTH_16BIT) break;
      sample->prepareUndo(true);
      e->lockEngine([this,sample]() {
        SAMPLE_OP_BEGIN;

        if (sample->depth==DIV_SAMPLE_DEPTH_16BIT) {
          for (unsigned int i=start; i<end; i++) {
            sample->data16[i]^=0x8000;
          }
        } else if (sample->depth==DIV_SAMPLE_DEPTH_8BIT) {
          for (unsigned int i=start; i<end; i++) {
            sample->data8[i]^=0x80;
          }
        }

        updateSampleTex=true;

        e->renderSamples(curSample);
      });
      MARK_MODIFIED;
      break;
    }
    case GUI_ACTION_SAMPLE_CROSSFADE_LOOP:
      if (curSample<0 || curSample>=(int)e->song.sample.size()) break;
      openSampleCrossFadeOpt=true;
      break;
    case GUI_ACTION_SAMPLE_FILTER:
      if (curSample<0 || curSample>=(int)e->song.sample.size()) break;
      openSampleFilterOpt=true;
      break;
    case GUI_ACTION_SAMPLE_PREVIEW:
      if (curSample<0 || curSample>=(int)e->song.sample.size()) break;
      e->previewSample(curSample);
      break;
    case GUI_ACTION_SAMPLE_STOP_PREVIEW:
      if (curSample<0 || curSample>=(int)e->song.sample.size()) break;
      e->stopSamplePreview();
      break;
    case GUI_ACTION_SAMPLE_ZOOM_IN: {
      if (curSample<0 || curSample>=(int)e->song.sample.size()) break;
      double zoomPercent=100.0/sampleZoom;
      zoomPercent+=10.0;
      if (zoomPercent>10000.0) zoomPercent=10000.0;
      if (zoomPercent<1.0) zoomPercent=1.0;
      sampleZoom=100.0/zoomPercent;
      if (sampleZoom<0.01) sampleZoom=0.01;
      sampleZoomAuto=false;
      updateSampleTex=true;
      break;
    }
    case GUI_ACTION_SAMPLE_ZOOM_OUT: {
      if (curSample<0 || curSample>=(int)e->song.sample.size()) break;
      double zoomPercent=100.0/sampleZoom;
      zoomPercent-=10.0;
      if (zoomPercent>10000.0) zoomPercent=10000.0;
      if (zoomPercent<1.0) zoomPercent=1.0;
      sampleZoom=100.0/zoomPercent;
      if (sampleZoom<0.01) sampleZoom=0.01;
      sampleZoomAuto=false;
      updateSampleTex=true;
      break;
    }
    case GUI_ACTION_SAMPLE_ZOOM_AUTO:
      if (curSample<0 || curSample>=(int)e->song.sample.size()) break;
      if (sampleZoomAuto) {
        sampleZoom=1.0;
        sampleZoomAuto=false;
        updateSampleTex=true;
      } else {
        sampleZoomAuto=true;
        updateSampleTex=true;
      }
      break;
    case GUI_ACTION_SAMPLE_MAKE_INS: {
      if (curSample<0 || curSample>=(int)e->song.sample.size()) break;
      // determine instrument type
      std::vector<DivInstrumentType> tempTypeList=e->getPossibleInsTypes();
      makeInsTypeList.clear();

      for (DivInstrumentType& i: tempTypeList) {
        if (i==DIV_INS_PCE ||
            i==DIV_INS_MSM6258 ||
            i==DIV_INS_MSM6295 ||
            i==DIV_INS_ADPCMA ||
            i==DIV_INS_ADPCMB ||
            i==DIV_INS_SEGAPCM ||
            i==DIV_INS_QSOUND ||
            i==DIV_INS_YMZ280B ||
            i==DIV_INS_RF5C68 ||
            i==DIV_INS_MULTIPCM ||
            i==DIV_INS_MIKEY ||
            i==DIV_INS_X1_010 ||
            i==DIV_INS_SWAN ||
            i==DIV_INS_AY ||
            i==DIV_INS_AY8930 ||
            i==DIV_INS_VRC6 ||
            i==DIV_INS_SU ||
            i==DIV_INS_SNES ||
            i==DIV_INS_ES5506 ||
            i==DIV_INS_K007232 ||
            i==DIV_INS_GA20 ||
            i==DIV_INS_K053260 ||
            i==DIV_INS_C140 ||
            i==DIV_INS_C219 ||
            i==DIV_INS_NDS ||
            i==DIV_INS_GBA_DMA ||
            i==DIV_INS_GBA_MINMOD) {
          makeInsTypeList.push_back(i);
        }
      }

      if (makeInsTypeList.size()>1) {
        displayInsTypeList=true;
        displayInsTypeListMakeInsSample=curSample;
        break;
      }

      DivInstrumentType insType=DIV_INS_AMIGA;
      if (!makeInsTypeList.empty()) {
        insType=makeInsTypeList[0];
      }

      DivSample* sample=e->song.sample[curSample];
      curIns=e->addInstrument(cursor.xCoarse);
      if (curIns==-1) {
        showError(_("too many instruments!"));
      } else {
        e->song.ins[curIns]->type=insType;
        e->song.ins[curIns]->name=sample->name;
        e->song.ins[curIns]->amiga.initSample=curSample;
        if (insType!=DIV_INS_AMIGA) e->song.ins[curIns]->amiga.useSample=true;
        nextWindow=GUI_WINDOW_INS_EDIT;
        MARK_MODIFIED;
        wavePreviewInit=true;
        updateFMPreview=true;
      }
      break;
    }
    case GUI_ACTION_SAMPLE_SET_LOOP: {
      if (curSample<0 || curSample>=(int)e->song.sample.size()) break;
      DivSample* sample=e->song.sample[curSample];
      sample->prepareUndo(true);
      e->lockEngine([this,sample]() {
        SAMPLE_OP_BEGIN;

        sample->loopStart=start;
        sample->loopEnd=end;
        sample->loop=true;
        updateSampleTex=true;

        e->renderSamples(curSample);
      });
      MARK_MODIFIED;
      break;
    }
    case GUI_ACTION_SAMPLE_CREATE_WAVE: {
      if (curSample<0 || curSample>=(int)e->song.sample.size()) break;
      DivSample* sample=e->song.sample[curSample];
      SAMPLE_OP_BEGIN;
      if (end-start<1) {
        showError(_("select at least one sample!"));
      } else if (end-start>256) {
        showError(_("maximum size is 256 samples!"));
      } else {
        curWave=e->addWave();
        if (curWave==-1) {
          showError(_("too many wavetables!"));
        } else {
          DivWavetable* wave=e->song.wave[curWave];
          wave->min=0;
          wave->max=255;
          wave->len=end-start;
          for (unsigned int i=start; i<end; i++) {
            wave->data[i-start]=(((unsigned short)sample->data16[i]&0xff00)>>8)^0x80;
          }
          nextWindow=GUI_WINDOW_WAVE_EDIT;
          MARK_MODIFIED;
          RESET_WAVE_MACRO_ZOOM;
        }
      }
      break;
    }

    case GUI_ACTION_ORDERS_UP:
      if (curOrder>0) {
        setOrder(curOrder-1);
      }
      break;
    case GUI_ACTION_ORDERS_DOWN:
      if (curOrder<e->curSubSong->ordersLen-1) {
        setOrder(curOrder+1);
      }
      break;
    case GUI_ACTION_ORDERS_LEFT: {
      DETERMINE_FIRST;

      do {
        orderCursor--;
        if (orderCursor<firstChannel) {
          orderCursor=firstChannel;
          break;
        }
      } while (!e->curSubSong->chanShow[orderCursor]);
      break;
    }
    case GUI_ACTION_ORDERS_RIGHT: {
      DETERMINE_LAST;

      do {
        orderCursor++;
        if (orderCursor>=lastChannel) {
          orderCursor=lastChannel-1;
          break;
        }
      } while (!e->curSubSong->chanShow[orderCursor]);
      break;
    }
    case GUI_ACTION_ORDERS_INCREASE: {
      if (orderCursor<0 || orderCursor>=e->getTotalChannelCount()) break;
      if (e->curOrders->ord[orderCursor][curOrder]<0xff) {
        e->curOrders->ord[orderCursor][curOrder]++;
      }
      break;
    }
    case GUI_ACTION_ORDERS_DECREASE: {
      if (orderCursor<0 || orderCursor>=e->getTotalChannelCount()) break;
      if (e->curOrders->ord[orderCursor][curOrder]>0) {
        e->curOrders->ord[orderCursor][curOrder]--;
      }
      break;
    }
    case GUI_ACTION_ORDERS_EDIT_MODE:
      orderEditMode++;
      if (orderEditMode>3) orderEditMode=0;
      break;
    case GUI_ACTION_ORDERS_LINK:
      changeAllOrders=!changeAllOrders;
      break;
    case GUI_ACTION_ORDERS_ADD:
      prepareUndo(GUI_UNDO_CHANGE_ORDER);
      e->addOrder(curOrder,false,false);
      curOrder=e->getOrder();
      makeUndo(GUI_UNDO_CHANGE_ORDER);
      break;
    case GUI_ACTION_ORDERS_DUPLICATE:
      prepareUndo(GUI_UNDO_CHANGE_ORDER);
      e->addOrder(curOrder,true,false);
      makeUndo(GUI_UNDO_CHANGE_ORDER);
      break;
    case GUI_ACTION_ORDERS_DEEP_CLONE:
      prepareUndo(GUI_UNDO_CHANGE_ORDER);
      e->deepCloneOrder(curOrder,false);
      makeUndo(GUI_UNDO_CHANGE_ORDER);
      if (!e->getWarnings().empty()) {
        showWarning(e->getWarnings(),GUI_WARN_GENERIC);
      }
      break;
    case GUI_ACTION_ORDERS_DUPLICATE_END:
      prepareUndo(GUI_UNDO_CHANGE_ORDER);
      e->addOrder(curOrder,true,true);
      makeUndo(GUI_UNDO_CHANGE_ORDER);
      break;
    case GUI_ACTION_ORDERS_DEEP_CLONE_END:
      prepareUndo(GUI_UNDO_CHANGE_ORDER);
      e->deepCloneOrder(curOrder,true);
      makeUndo(GUI_UNDO_CHANGE_ORDER);
      if (!e->getWarnings().empty()) {
        showWarning(e->getWarnings(),GUI_WARN_GENERIC);
      }
      break;
    case GUI_ACTION_ORDERS_REMOVE:
      prepareUndo(GUI_UNDO_CHANGE_ORDER);
      e->deleteOrder(curOrder);
      if (curOrder>=e->curSubSong->ordersLen) {
        curOrder=e->curSubSong->ordersLen-1;
        e->setOrder(curOrder);
      }
      makeUndo(GUI_UNDO_CHANGE_ORDER);
      break;
    case GUI_ACTION_ORDERS_MOVE_UP:
      prepareUndo(GUI_UNDO_CHANGE_ORDER);
      e->moveOrderUp(curOrder);
      if (settings.cursorFollowsOrder) {
        e->setOrder(curOrder);
      }
      makeUndo(GUI_UNDO_CHANGE_ORDER);
      break;
    case GUI_ACTION_ORDERS_MOVE_DOWN:
      prepareUndo(GUI_UNDO_CHANGE_ORDER);
      e->moveOrderDown(curOrder);
      if (settings.cursorFollowsOrder) {
        e->setOrder(curOrder);
      }
      makeUndo(GUI_UNDO_CHANGE_ORDER);
      break;
    case GUI_ACTION_ORDERS_REPLAY:
      setOrder(curOrder);
      break;
  }
}
