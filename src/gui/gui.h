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

#ifndef _FUR_GUI_H
#define _FUR_GUI_H

#include "../engine/engine.h"
#include "../engine/waveSynth.h"
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#include <SDL.h>
#include <deque>
#include <initializer_list>
#include <map>
#include <future>
#include <mutex>
#include <vector>

#include "fileDialog.h"

#define rightClickable if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) ImGui::SetKeyboardFocusHere(-1);
#define ctrlWheeling ((ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl)) && wheelY!=0)

#define handleUnimportant if (settings.insFocusesPattern && patternOpen) {nextWindow=GUI_WINDOW_PATTERN;}
#define unimportant(x) if (x) {handleUnimportant}

#define MARK_MODIFIED modified=true;
#define WAKE_UP drawHalt=16;

#define TOGGLE_COLOR(x) ((x)?uiColors[GUI_COLOR_TOGGLE_ON]:uiColors[GUI_COLOR_TOGGLE_OFF])

#define BIND_FOR(x) getKeyName(actionKeys[x],true).c_str()

// TODO:
// - add colors for FM envelope and waveform
// - maybe add "alternate" color for FM modulators/carriers (a bit difficult)
enum FurnaceGUIColors {
  GUI_COLOR_BACKGROUND=0,
  GUI_COLOR_FRAME_BACKGROUND,
  GUI_COLOR_MODAL_BACKDROP,
  GUI_COLOR_HEADER,
  GUI_COLOR_TEXT,
  GUI_COLOR_ACCENT_PRIMARY,
  GUI_COLOR_ACCENT_SECONDARY,
  GUI_COLOR_BORDER,
  GUI_COLOR_BORDER_SHADOW,
  GUI_COLOR_TOGGLE_OFF,
  GUI_COLOR_TOGGLE_ON,
  GUI_COLOR_EDITING,
  GUI_COLOR_SONG_LOOP,

  GUI_COLOR_FILE_DIR,
  GUI_COLOR_FILE_SONG_NATIVE,
  GUI_COLOR_FILE_SONG_IMPORT,
  GUI_COLOR_FILE_INSTR,
  GUI_COLOR_FILE_AUDIO,
  GUI_COLOR_FILE_WAVE,
  GUI_COLOR_FILE_VGM,
  GUI_COLOR_FILE_ZSM,
  GUI_COLOR_FILE_FONT,
  GUI_COLOR_FILE_OTHER,

  GUI_COLOR_OSC_BG1,
  GUI_COLOR_OSC_BG2,
  GUI_COLOR_OSC_BG3,
  GUI_COLOR_OSC_BG4,
  GUI_COLOR_OSC_BORDER,
  GUI_COLOR_OSC_WAVE,
  GUI_COLOR_OSC_WAVE_PEAK,
  GUI_COLOR_OSC_REF,
  GUI_COLOR_OSC_GUIDE,

  GUI_COLOR_VOLMETER_LOW,
  GUI_COLOR_VOLMETER_HIGH,
  GUI_COLOR_VOLMETER_PEAK,

  GUI_COLOR_ORDER_ROW_INDEX,
  GUI_COLOR_ORDER_ACTIVE,
  GUI_COLOR_ORDER_SIMILAR,
  GUI_COLOR_ORDER_INACTIVE,

  GUI_COLOR_FM_ALG_BG,
  GUI_COLOR_FM_ALG_LINE,
  GUI_COLOR_FM_MOD,
  GUI_COLOR_FM_PRIMARY_MOD,
  GUI_COLOR_FM_SECONDARY_MOD,
  GUI_COLOR_FM_BORDER_MOD,
  GUI_COLOR_FM_BORDER_SHADOW_MOD,
  GUI_COLOR_FM_CAR,
  GUI_COLOR_FM_PRIMARY_CAR,
  GUI_COLOR_FM_SECONDARY_CAR,
  GUI_COLOR_FM_BORDER_CAR,
  GUI_COLOR_FM_BORDER_SHADOW_CAR,

  GUI_COLOR_FM_ENVELOPE,
  GUI_COLOR_FM_ENVELOPE_SUS_GUIDE,
  GUI_COLOR_FM_ENVELOPE_RELEASE,
  GUI_COLOR_FM_SSG,
  GUI_COLOR_FM_WAVE,

  GUI_COLOR_MACRO_VOLUME,
  GUI_COLOR_MACRO_PITCH,
  GUI_COLOR_MACRO_OTHER,
  GUI_COLOR_MACRO_WAVE,

  GUI_COLOR_INSTR_STD,
  GUI_COLOR_INSTR_FM,
  GUI_COLOR_INSTR_GB,
  GUI_COLOR_INSTR_C64,
  GUI_COLOR_INSTR_AMIGA,
  GUI_COLOR_INSTR_PCE,
  GUI_COLOR_INSTR_AY,
  GUI_COLOR_INSTR_AY8930,
  GUI_COLOR_INSTR_TIA,
  GUI_COLOR_INSTR_SAA1099,
  GUI_COLOR_INSTR_VIC,
  GUI_COLOR_INSTR_PET,
  GUI_COLOR_INSTR_VRC6,
  GUI_COLOR_INSTR_OPLL,
  GUI_COLOR_INSTR_OPL,
  GUI_COLOR_INSTR_FDS,
  GUI_COLOR_INSTR_VBOY,
  GUI_COLOR_INSTR_N163,
  GUI_COLOR_INSTR_SCC,
  GUI_COLOR_INSTR_OPZ,
  GUI_COLOR_INSTR_POKEY,
  GUI_COLOR_INSTR_BEEPER,
  GUI_COLOR_INSTR_SWAN,
  GUI_COLOR_INSTR_MIKEY,
  GUI_COLOR_INSTR_VERA,
  GUI_COLOR_INSTR_X1_010,
  GUI_COLOR_INSTR_VRC6_SAW,
  GUI_COLOR_INSTR_ES5506,
  GUI_COLOR_INSTR_MULTIPCM,
  GUI_COLOR_INSTR_SNES,
  GUI_COLOR_INSTR_SU,
  GUI_COLOR_INSTR_NAMCO,
  GUI_COLOR_INSTR_UNKNOWN,

  GUI_COLOR_CHANNEL_FM,
  GUI_COLOR_CHANNEL_PULSE,
  GUI_COLOR_CHANNEL_NOISE,
  GUI_COLOR_CHANNEL_WAVE,
  GUI_COLOR_CHANNEL_PCM,
  GUI_COLOR_CHANNEL_OP,
  GUI_COLOR_CHANNEL_MUTED,

  GUI_COLOR_PATTERN_PLAY_HEAD,
  GUI_COLOR_PATTERN_CURSOR,
  GUI_COLOR_PATTERN_CURSOR_HOVER,
  GUI_COLOR_PATTERN_CURSOR_ACTIVE,
  GUI_COLOR_PATTERN_SELECTION,
  GUI_COLOR_PATTERN_SELECTION_HOVER,
  GUI_COLOR_PATTERN_SELECTION_ACTIVE,
  GUI_COLOR_PATTERN_HI_1,
  GUI_COLOR_PATTERN_HI_2,
  GUI_COLOR_PATTERN_ROW_INDEX,
  GUI_COLOR_PATTERN_ROW_INDEX_HI1,
  GUI_COLOR_PATTERN_ROW_INDEX_HI2,
  GUI_COLOR_PATTERN_ACTIVE,
  GUI_COLOR_PATTERN_INACTIVE,
  GUI_COLOR_PATTERN_ACTIVE_HI1,
  GUI_COLOR_PATTERN_INACTIVE_HI1,
  GUI_COLOR_PATTERN_ACTIVE_HI2,
  GUI_COLOR_PATTERN_INACTIVE_HI2,
  GUI_COLOR_PATTERN_INS,
  GUI_COLOR_PATTERN_INS_WARN,
  GUI_COLOR_PATTERN_INS_ERROR,
  GUI_COLOR_PATTERN_VOLUME_MAX,
  GUI_COLOR_PATTERN_VOLUME_HALF,
  GUI_COLOR_PATTERN_VOLUME_MIN,
  GUI_COLOR_PATTERN_EFFECT_INVALID,
  GUI_COLOR_PATTERN_EFFECT_PITCH,
  GUI_COLOR_PATTERN_EFFECT_VOLUME,
  GUI_COLOR_PATTERN_EFFECT_PANNING,
  GUI_COLOR_PATTERN_EFFECT_SONG,
  GUI_COLOR_PATTERN_EFFECT_TIME,
  GUI_COLOR_PATTERN_EFFECT_SPEED,
  GUI_COLOR_PATTERN_EFFECT_SYS_PRIMARY,
  GUI_COLOR_PATTERN_EFFECT_SYS_SECONDARY,
  GUI_COLOR_PATTERN_EFFECT_MISC,

  GUI_COLOR_PIANO_BACKGROUND,
  GUI_COLOR_PIANO_KEY_BOTTOM,
  GUI_COLOR_PIANO_KEY_TOP,
  GUI_COLOR_PIANO_KEY_BOTTOM_HIT,
  GUI_COLOR_PIANO_KEY_TOP_HIT,
  GUI_COLOR_PIANO_KEY_BOTTOM_ACTIVE,
  GUI_COLOR_PIANO_KEY_TOP_ACTIVE,

  GUI_COLOR_LOGLEVEL_ERROR,
  GUI_COLOR_LOGLEVEL_WARNING,
  GUI_COLOR_LOGLEVEL_INFO,
  GUI_COLOR_LOGLEVEL_DEBUG,
  GUI_COLOR_LOGLEVEL_TRACE,

  GUI_COLOR_EE_VALUE,
  GUI_COLOR_PLAYBACK_STAT,
  GUI_COLOR_MAX
};

enum FurnaceGUIWindows {
  GUI_WINDOW_NOTHING=0,
  GUI_WINDOW_EDIT_CONTROLS,
  GUI_WINDOW_SONG_INFO,
  GUI_WINDOW_ORDERS,
  GUI_WINDOW_INS_LIST,
  GUI_WINDOW_PATTERN,
  GUI_WINDOW_INS_EDIT,
  GUI_WINDOW_WAVE_LIST,
  GUI_WINDOW_WAVE_EDIT,
  GUI_WINDOW_SAMPLE_LIST,
  GUI_WINDOW_SAMPLE_EDIT,
  GUI_WINDOW_MIXER,
  GUI_WINDOW_ABOUT,
  GUI_WINDOW_SETTINGS,
  GUI_WINDOW_DEBUG,
  GUI_WINDOW_OSCILLOSCOPE,
  GUI_WINDOW_VOL_METER,
  GUI_WINDOW_STATS,
  GUI_WINDOW_COMPAT_FLAGS,
  GUI_WINDOW_PIANO,
  GUI_WINDOW_NOTES,
  GUI_WINDOW_CHANNELS,
  GUI_WINDOW_REGISTER_VIEW,
  GUI_WINDOW_LOG,
  GUI_WINDOW_EFFECT_LIST,
  GUI_WINDOW_CHAN_OSC,
  GUI_WINDOW_SUBSONGS
};

enum FurnaceGUIFileDialogs {
  GUI_FILE_OPEN,
  GUI_FILE_SAVE,
  GUI_FILE_SAVE_DMF_LEGACY,
  GUI_FILE_INS_OPEN,
  GUI_FILE_INS_OPEN_REPLACE,
  GUI_FILE_INS_SAVE,
  GUI_FILE_WAVE_OPEN,
  GUI_FILE_WAVE_SAVE,
  GUI_FILE_SAMPLE_OPEN,
  GUI_FILE_SAMPLE_SAVE,
  GUI_FILE_EXPORT_AUDIO_ONE,
  GUI_FILE_EXPORT_AUDIO_PER_SYS,
  GUI_FILE_EXPORT_AUDIO_PER_CHANNEL,
  GUI_FILE_EXPORT_VGM,
  GUI_FILE_EXPORT_ZSM,
  GUI_FILE_EXPORT_ROM,
  GUI_FILE_LOAD_MAIN_FONT,
  GUI_FILE_LOAD_PAT_FONT,
  GUI_FILE_IMPORT_COLORS,
  GUI_FILE_IMPORT_KEYBINDS,
  GUI_FILE_IMPORT_LAYOUT,
  GUI_FILE_EXPORT_COLORS,
  GUI_FILE_EXPORT_KEYBINDS,
  GUI_FILE_EXPORT_LAYOUT,
  GUI_FILE_YRW801_ROM_OPEN,
  GUI_FILE_TG100_ROM_OPEN,
  GUI_FILE_MU5_ROM_OPEN
};

enum FurnaceGUIWarnings {
  GUI_WARN_QUIT,
  GUI_WARN_NEW,
  GUI_WARN_OPEN,
  GUI_WARN_OPEN_BACKUP,
  GUI_WARN_OPEN_DROP,
  GUI_WARN_RESET_LAYOUT,
  GUI_WARN_RESET_COLORS,
  GUI_WARN_RESET_KEYBINDS,
  GUI_WARN_CLOSE_SETTINGS,
  GUI_WARN_CLEAR,
  GUI_WARN_SUBSONG_DEL,
  GUI_WARN_GENERIC
};

enum FurnaceGUIFMAlgs {
  FM_ALGS_4OP,
  FM_ALGS_2OP_OPL,
  FM_ALGS_4OP_OPL
};

enum FurnaceGUIActions {
  GUI_ACTION_GLOBAL_MIN=0,
  GUI_ACTION_OPEN,
  GUI_ACTION_OPEN_BACKUP,
  GUI_ACTION_SAVE,
  GUI_ACTION_SAVE_AS,
  GUI_ACTION_UNDO,
  GUI_ACTION_REDO,
  GUI_ACTION_PLAY_TOGGLE,
  GUI_ACTION_PLAY,
  GUI_ACTION_STOP,
  GUI_ACTION_PLAY_REPEAT,
  GUI_ACTION_PLAY_CURSOR,
  GUI_ACTION_STEP_ONE,
  GUI_ACTION_OCTAVE_UP,
  GUI_ACTION_OCTAVE_DOWN,
  GUI_ACTION_INS_UP,
  GUI_ACTION_INS_DOWN,
  GUI_ACTION_STEP_UP,
  GUI_ACTION_STEP_DOWN,
  GUI_ACTION_TOGGLE_EDIT,
  GUI_ACTION_METRONOME,
  GUI_ACTION_REPEAT_PATTERN,
  GUI_ACTION_FOLLOW_ORDERS,
  GUI_ACTION_FOLLOW_PATTERN,
  GUI_ACTION_FULLSCREEN,
  GUI_ACTION_TX81Z_REQUEST,
  GUI_ACTION_PANIC,

  GUI_ACTION_WINDOW_EDIT_CONTROLS,
  GUI_ACTION_WINDOW_ORDERS,
  GUI_ACTION_WINDOW_INS_LIST,
  GUI_ACTION_WINDOW_INS_EDIT,
  GUI_ACTION_WINDOW_SONG_INFO,
  GUI_ACTION_WINDOW_PATTERN,
  GUI_ACTION_WINDOW_WAVE_LIST,
  GUI_ACTION_WINDOW_WAVE_EDIT,
  GUI_ACTION_WINDOW_SAMPLE_LIST,
  GUI_ACTION_WINDOW_SAMPLE_EDIT,
  GUI_ACTION_WINDOW_ABOUT,
  GUI_ACTION_WINDOW_SETTINGS,
  GUI_ACTION_WINDOW_MIXER,
  GUI_ACTION_WINDOW_DEBUG,
  GUI_ACTION_WINDOW_OSCILLOSCOPE,
  GUI_ACTION_WINDOW_VOL_METER,
  GUI_ACTION_WINDOW_STATS,
  GUI_ACTION_WINDOW_COMPAT_FLAGS,
  GUI_ACTION_WINDOW_PIANO,
  GUI_ACTION_WINDOW_NOTES,
  GUI_ACTION_WINDOW_CHANNELS,
  GUI_ACTION_WINDOW_REGISTER_VIEW,
  GUI_ACTION_WINDOW_LOG,
  GUI_ACTION_WINDOW_EFFECT_LIST,
  GUI_ACTION_WINDOW_CHAN_OSC,
  GUI_ACTION_WINDOW_SUBSONGS,

  GUI_ACTION_COLLAPSE_WINDOW,
  GUI_ACTION_CLOSE_WINDOW,
  GUI_ACTION_GLOBAL_MAX,

  GUI_ACTION_PAT_MIN,
  GUI_ACTION_PAT_NOTE_UP,
  GUI_ACTION_PAT_NOTE_DOWN,
  GUI_ACTION_PAT_OCTAVE_UP,
  GUI_ACTION_PAT_OCTAVE_DOWN,
  GUI_ACTION_PAT_VALUE_UP,
  GUI_ACTION_PAT_VALUE_DOWN,
  GUI_ACTION_PAT_VALUE_UP_COARSE,
  GUI_ACTION_PAT_VALUE_DOWN_COARSE,
  GUI_ACTION_PAT_SELECT_ALL,
  GUI_ACTION_PAT_CUT,
  GUI_ACTION_PAT_COPY,
  GUI_ACTION_PAT_PASTE,
  GUI_ACTION_PAT_PASTE_MIX,
  GUI_ACTION_PAT_PASTE_MIX_BG,
  GUI_ACTION_PAT_PASTE_FLOOD,
  GUI_ACTION_PAT_PASTE_OVERFLOW,
  GUI_ACTION_PAT_CURSOR_UP,
  GUI_ACTION_PAT_CURSOR_DOWN,
  GUI_ACTION_PAT_CURSOR_LEFT,
  GUI_ACTION_PAT_CURSOR_RIGHT,
  GUI_ACTION_PAT_CURSOR_UP_ONE,
  GUI_ACTION_PAT_CURSOR_DOWN_ONE,
  GUI_ACTION_PAT_CURSOR_LEFT_CHANNEL,
  GUI_ACTION_PAT_CURSOR_RIGHT_CHANNEL,
  GUI_ACTION_PAT_CURSOR_NEXT_CHANNEL,
  GUI_ACTION_PAT_CURSOR_PREVIOUS_CHANNEL,
  GUI_ACTION_PAT_CURSOR_BEGIN,
  GUI_ACTION_PAT_CURSOR_END,
  GUI_ACTION_PAT_CURSOR_UP_COARSE,
  GUI_ACTION_PAT_CURSOR_DOWN_COARSE,
  GUI_ACTION_PAT_SELECTION_UP,
  GUI_ACTION_PAT_SELECTION_DOWN,
  GUI_ACTION_PAT_SELECTION_LEFT,
  GUI_ACTION_PAT_SELECTION_RIGHT,
  GUI_ACTION_PAT_SELECTION_UP_ONE,
  GUI_ACTION_PAT_SELECTION_DOWN_ONE,
  GUI_ACTION_PAT_SELECTION_BEGIN,
  GUI_ACTION_PAT_SELECTION_END,
  GUI_ACTION_PAT_SELECTION_UP_COARSE,
  GUI_ACTION_PAT_SELECTION_DOWN_COARSE,
  GUI_ACTION_PAT_DELETE,
  GUI_ACTION_PAT_PULL_DELETE,
  GUI_ACTION_PAT_INSERT,
  GUI_ACTION_PAT_MUTE_CURSOR,
  GUI_ACTION_PAT_SOLO_CURSOR,
  GUI_ACTION_PAT_UNMUTE_ALL,
  GUI_ACTION_PAT_NEXT_ORDER,
  GUI_ACTION_PAT_PREV_ORDER,
  GUI_ACTION_PAT_COLLAPSE,
  GUI_ACTION_PAT_INCREASE_COLUMNS,
  GUI_ACTION_PAT_DECREASE_COLUMNS,
  GUI_ACTION_PAT_INTERPOLATE,
  GUI_ACTION_PAT_FADE,
  GUI_ACTION_PAT_INVERT_VALUES,
  GUI_ACTION_PAT_FLIP_SELECTION,
  GUI_ACTION_PAT_COLLAPSE_ROWS,
  GUI_ACTION_PAT_EXPAND_ROWS,
  GUI_ACTION_PAT_COLLAPSE_PAT,
  GUI_ACTION_PAT_EXPAND_PAT,
  GUI_ACTION_PAT_COLLAPSE_SONG,
  GUI_ACTION_PAT_EXPAND_SONG,
  GUI_ACTION_PAT_LATCH,
  GUI_ACTION_PAT_MAX,

  GUI_ACTION_INS_LIST_MIN,
  GUI_ACTION_INS_LIST_ADD,
  GUI_ACTION_INS_LIST_DUPLICATE,
  GUI_ACTION_INS_LIST_OPEN,
  GUI_ACTION_INS_LIST_OPEN_REPLACE,
  GUI_ACTION_INS_LIST_SAVE,
  GUI_ACTION_INS_LIST_MOVE_UP,
  GUI_ACTION_INS_LIST_MOVE_DOWN,
  GUI_ACTION_INS_LIST_DELETE,
  GUI_ACTION_INS_LIST_EDIT,
  GUI_ACTION_INS_LIST_UP,
  GUI_ACTION_INS_LIST_DOWN,
  GUI_ACTION_INS_LIST_MAX,

  GUI_ACTION_WAVE_LIST_MIN,
  GUI_ACTION_WAVE_LIST_ADD,
  GUI_ACTION_WAVE_LIST_DUPLICATE,
  GUI_ACTION_WAVE_LIST_OPEN,
  GUI_ACTION_WAVE_LIST_SAVE,
  GUI_ACTION_WAVE_LIST_MOVE_UP,
  GUI_ACTION_WAVE_LIST_MOVE_DOWN,
  GUI_ACTION_WAVE_LIST_DELETE,
  GUI_ACTION_WAVE_LIST_EDIT,
  GUI_ACTION_WAVE_LIST_UP,
  GUI_ACTION_WAVE_LIST_DOWN,
  GUI_ACTION_WAVE_LIST_MAX,

  GUI_ACTION_SAMPLE_LIST_MIN,
  GUI_ACTION_SAMPLE_LIST_ADD,
  GUI_ACTION_SAMPLE_LIST_DUPLICATE,
  GUI_ACTION_SAMPLE_LIST_OPEN,
  GUI_ACTION_SAMPLE_LIST_SAVE,
  GUI_ACTION_SAMPLE_LIST_MOVE_UP,
  GUI_ACTION_SAMPLE_LIST_MOVE_DOWN,
  GUI_ACTION_SAMPLE_LIST_DELETE,
  GUI_ACTION_SAMPLE_LIST_EDIT,
  GUI_ACTION_SAMPLE_LIST_UP,
  GUI_ACTION_SAMPLE_LIST_DOWN,
  GUI_ACTION_SAMPLE_LIST_PREVIEW,
  GUI_ACTION_SAMPLE_LIST_STOP_PREVIEW,
  GUI_ACTION_SAMPLE_LIST_MAX,

  GUI_ACTION_SAMPLE_MIN,
  GUI_ACTION_SAMPLE_SELECT,
  GUI_ACTION_SAMPLE_DRAW,
  GUI_ACTION_SAMPLE_CUT,
  GUI_ACTION_SAMPLE_COPY,
  GUI_ACTION_SAMPLE_PASTE,
  GUI_ACTION_SAMPLE_PASTE_REPLACE,
  GUI_ACTION_SAMPLE_PASTE_MIX,
  GUI_ACTION_SAMPLE_SELECT_ALL,
  GUI_ACTION_SAMPLE_RESIZE,
  GUI_ACTION_SAMPLE_RESAMPLE,
  GUI_ACTION_SAMPLE_AMPLIFY,
  GUI_ACTION_SAMPLE_NORMALIZE,
  GUI_ACTION_SAMPLE_FADE_IN,
  GUI_ACTION_SAMPLE_FADE_OUT,
  GUI_ACTION_SAMPLE_SILENCE,
  GUI_ACTION_SAMPLE_INSERT,
  GUI_ACTION_SAMPLE_DELETE,
  GUI_ACTION_SAMPLE_TRIM,
  GUI_ACTION_SAMPLE_REVERSE,
  GUI_ACTION_SAMPLE_INVERT,
  GUI_ACTION_SAMPLE_SIGN,
  GUI_ACTION_SAMPLE_FILTER,
  GUI_ACTION_SAMPLE_PREVIEW,
  GUI_ACTION_SAMPLE_STOP_PREVIEW,
  GUI_ACTION_SAMPLE_ZOOM_IN,
  GUI_ACTION_SAMPLE_ZOOM_OUT,
  GUI_ACTION_SAMPLE_ZOOM_AUTO,
  GUI_ACTION_SAMPLE_MAKE_INS,
  GUI_ACTION_SAMPLE_MAX,

  GUI_ACTION_ORDERS_MIN,
  GUI_ACTION_ORDERS_UP,
  GUI_ACTION_ORDERS_DOWN,
  GUI_ACTION_ORDERS_LEFT,
  GUI_ACTION_ORDERS_RIGHT,
  GUI_ACTION_ORDERS_INCREASE,
  GUI_ACTION_ORDERS_DECREASE,
  GUI_ACTION_ORDERS_EDIT_MODE,
  GUI_ACTION_ORDERS_LINK,
  GUI_ACTION_ORDERS_ADD,
  GUI_ACTION_ORDERS_DUPLICATE,
  GUI_ACTION_ORDERS_DEEP_CLONE,
  GUI_ACTION_ORDERS_DUPLICATE_END,
  GUI_ACTION_ORDERS_DEEP_CLONE_END,
  GUI_ACTION_ORDERS_REMOVE,
  GUI_ACTION_ORDERS_MOVE_UP,
  GUI_ACTION_ORDERS_MOVE_DOWN,
  GUI_ACTION_ORDERS_REPLAY,
  GUI_ACTION_ORDERS_MAX,

  GUI_ACTION_MAX
};

enum PasteMode {
  GUI_PASTE_MODE_NORMAL=0,
  GUI_PASTE_MODE_MIX_FG,
  GUI_PASTE_MODE_MIX_BG,
  GUI_PASTE_MODE_FLOOD,
  GUI_PASTE_MODE_OVERFLOW
};

#define FURKMOD_CTRL (1U<<31)
#define FURKMOD_SHIFT (1<<29)
#define FURKMOD_META (1<<28)
#define FURKMOD_ALT (1<<27)
#define FURK_MASK 0x40ffffff

#ifdef __APPLE__
#define FURKMOD_CMD FURKMOD_META
#else
#define FURKMOD_CMD FURKMOD_CTRL
#endif

struct SelectionPoint {
  int xCoarse, xFine;
  int y;
  SelectionPoint():
    xCoarse(0), xFine(0), y(0) {}
};

enum ActionType {
  GUI_UNDO_CHANGE_ORDER,
  GUI_UNDO_PATTERN_EDIT,
  GUI_UNDO_PATTERN_DELETE,
  GUI_UNDO_PATTERN_PULL,
  GUI_UNDO_PATTERN_PUSH,
  GUI_UNDO_PATTERN_CUT,
  GUI_UNDO_PATTERN_PASTE,
  GUI_UNDO_PATTERN_CHANGE_INS,
  GUI_UNDO_PATTERN_INTERPOLATE,
  GUI_UNDO_PATTERN_FADE,
  GUI_UNDO_PATTERN_SCALE,
  GUI_UNDO_PATTERN_RANDOMIZE,
  GUI_UNDO_PATTERN_INVERT_VAL,
  GUI_UNDO_PATTERN_FLIP,
  GUI_UNDO_PATTERN_COLLAPSE,
  GUI_UNDO_PATTERN_EXPAND
};

struct UndoPatternData {
  int subSong, chan, pat, row, col;
  short oldVal, newVal;
  UndoPatternData(int s, int c, int p, int r, int co, short v1, short v2):
    subSong(s),
    chan(c),
    pat(p),
    row(r),
    col(co),
    oldVal(v1),
    newVal(v2) {}
};

struct UndoOrderData {
  int subSong, chan, ord;
  unsigned char oldVal, newVal;
  UndoOrderData(int s, int c, int o, unsigned char v1, unsigned char v2):
    subSong(s),
    chan(c),
    ord(o),
    oldVal(v1),
    newVal(v2) {}
};

struct UndoStep {
  ActionType type;
  SelectionPoint cursor, selStart, selEnd;
  int order;
  bool nibble;
  int oldOrdersLen, newOrdersLen;
  int oldPatLen, newPatLen;
  std::vector<UndoOrderData> ord;
  std::vector<UndoPatternData> pat;
};

// -1 = any
struct MIDIBind {
  int type, channel, data1, data2;
  int action;
  MIDIBind():
    type(0),
    channel(16),
    data1(128),
    data2(128),
    action(0) {}
};

struct MIDIMap {
  // access method: map[type][channel][data1][data2];
  // channel 16 = any
  // data1 128 = any
  // data2 128 = any
  int**** map;
  std::vector<MIDIBind> binds;

  bool noteInput, volInput, rawVolume, polyInput, directChannel, programChange, midiClock, midiTimeCode, yamahaFMResponse;
  // 0: disabled
  //
  // 1: C- C# D- D# E- F- F# G- G# A- A# B-
  // o1    1     3        6     8     A
  //    0     2     4  5     7     9     B
  //    C- C# D- D# E- F- F# G- G# A- A# B-
  // o2    D     F
  //    C     E
  //
  // 2: C- C# D- D# E- F- F# G- G# A- A# B-
  // o1    1     3        6     8     A
  //    0     2     4  5     7     9     B
  //    C- C# D- D# E- F- F# G- G# A- A# B-
  // o2    D     F        2     4     6
  //    C     E     0  1     3     5     7
  //
  // 3: C- C# D- D# E- F- F# G- G# A- A# B-
  // o1    A     B        C     D     E
  //    0     1     2  3     4     5     6
  //    C- C# D- D# E- F- F# G- G# A- A# B-
  // o2    F
  //    7     8     9
  //
  // 4: use dual CC for value input (nibble)
  // 5: use 14-bit CC for value input (MSB/LSB)
  // 6: use single CC for value input (may be imprecise)
  int valueInputStyle;
  int valueInputControlMSB; // on 4
  int valueInputControlLSB; // on 4
  int valueInputControlSingle;

  // 0: disabled
  // 1: use dual CC (nibble)
  // 2: use 14-bit CC (MSB/LSB)
  // 3: use single CC (may be imprecise)
  int valueInputSpecificStyle[18];
  int valueInputSpecificMSB[18];
  int valueInputSpecificLSB[18];
  int valueInputSpecificSingle[18];
  float volExp;

  int valueInputCurMSB, valueInputCurLSB, valueInputCurSingle;
  int valueInputCurMSBS[18];
  int valueInputCurLSBS[18];
  int valueInputCurSingleS[18];

  void compile();
  void deinit();
  int at(const TAMidiMessage& where);
  bool read(String path);
  bool write(String path);
  MIDIMap():
    map(NULL),
    noteInput(true),
    volInput(false),
    rawVolume(false),
    polyInput(false),
    directChannel(false),
    programChange(true),
    midiClock(false),
    midiTimeCode(false),
    yamahaFMResponse(false),
    valueInputStyle(1),
    valueInputControlMSB(0),
    valueInputControlLSB(0),
    valueInputControlSingle(0),
    volExp(1.0f),
    valueInputCurMSB(0),
    valueInputCurLSB(0),
    valueInputCurSingle(0) {
    memset(valueInputSpecificStyle,0,18*sizeof(int));
    memset(valueInputSpecificMSB,0,18*sizeof(int));
    memset(valueInputSpecificLSB,0,18*sizeof(int));
    memset(valueInputSpecificSingle,0,18*sizeof(int));

    memset(valueInputCurMSBS,0,18*sizeof(int));
    memset(valueInputCurLSBS,0,18*sizeof(int));
    memset(valueInputCurSingleS,0,18*sizeof(int));
  }
};

struct Particle {
  ImU32* colors;
  const char* type;
  ImVec2 pos, speed;
  float gravity, friction, life, lifeSpeed;
  bool update(float frameTime);
  Particle(ImU32* color, const char* ty, float x, float y, float sX, float sY, float g, float fr, float l, float lS):
    colors(color),
    type(ty),
    pos(x,y),
    speed(sX,sY),
    gravity(g),
    friction(fr),
    life(l),
    lifeSpeed(lS) {}
};

struct OperationMask {
  bool note, ins, vol, effect, effectVal;
  OperationMask():
    note(true),
    ins(true),
    vol(true),
    effect(true),
    effectVal(true) {}
};

struct TouchPoint {
  // an ID of -1 represents the mouse cursor.
  int id;
  float x, y, z;
  TouchPoint():
    id(-1),
    x(0.0f),
    y(0.0f),
    z(1.0f) {}
  TouchPoint(float xp, float yp):
    id(-1),
    x(xp),
    y(yp),
    z(1.0f) {}
  TouchPoint(int ident, float xp, float yp, float pressure=1.0f):
    id(ident),
    x(xp),
    y(yp),
    z(pressure) {}
};

struct FurnaceGUISysDef {
  const char* name;
  std::vector<int> definition;
  FurnaceGUISysDef(const char* n, std::initializer_list<int> def):
    name(n), definition(def) {
  }
};

struct FurnaceGUISysCategory {
  const char* name;
  const char* description;
  std::vector<FurnaceGUISysDef> systems;
  FurnaceGUISysCategory(const char* n, const char* d):
    name(n),
    description(d) {}
  FurnaceGUISysCategory():
    name(NULL),
    description(NULL) {}
};

struct FurnaceGUIMacroDesc {
  DivInstrumentMacro* macro;
  int min, max;
  float height;
  const char* displayName;
  const char** bitfieldBits;
  const char* modeName;
  ImVec4 color;
  unsigned int bitOffset;
  bool isBitfield, blockMode;
  String (*hoverFunc)(int,float);

  FurnaceGUIMacroDesc(const char* name, DivInstrumentMacro* m, int macroMin, int macroMax, float macroHeight, ImVec4 col=ImVec4(1.0f,1.0f,1.0f,1.0f), bool block=false, const char* mName=NULL, String (*hf)(int,float)=NULL, bool bitfield=false, const char** bfVal=NULL, unsigned int bitOff=0):
    macro(m),
    height(macroHeight),
    displayName(name),
    bitfieldBits(bfVal),
    modeName(mName),
    color(col),
    bitOffset(bitOff),
    isBitfield(bitfield),
    blockMode(block),
    hoverFunc(hf) {
    // MSVC -> hell
    this->min=macroMin;
    this->max=macroMax;
  }
};

class FurnaceGUI {
  DivEngine* e;

  SDL_Window* sdlWin;
  SDL_Renderer* sdlRend;

  SDL_Texture* sampleTex;
  int sampleTexW, sampleTexH;
  bool updateSampleTex;

  String workingDir, fileName, clipboard, warnString, errorString, lastError, curFileName, nextFile;
  String workingDirSong, workingDirIns, workingDirWave, workingDirSample, workingDirAudioExport, workingDirVGMExport, workingDirZSMExport, workingDirFont, workingDirColors, workingDirKeybinds, workingDirLayout, workingDirROM;
  String mmlString[32];
  String mmlStringW;

  bool quit, warnQuit, willCommit, edit, modified, displayError, displayExporting, vgmExportLoop, zsmExportLoop, wantCaptureKeyboard, oldWantCaptureKeyboard, displayMacroMenu;
  bool displayNew, fullScreen, preserveChanPos, wantScrollList;
  bool willExport[32];
  int vgmExportVersion;
  int drawHalt;
  int zsmExportTickRate;
  int macroPointSize;

  ImGuiWindowFlags globalWinFlags;

  FurnaceGUIFileDialogs curFileDialog;
  FurnaceGUIWarnings warnAction;
  FurnaceGUIWarnings postWarnAction;

  FurnaceGUIFileDialog* fileDialog;

  int scrW, scrH;

  double dpiScale;

  double aboutScroll, aboutSin;
  float aboutHue;

  std::atomic<double> backupTimer;
  std::future<bool> backupTask;
  std::mutex backupLock;
  String backupPath;

  std::mutex midiLock;
  std::queue<TAMidiMessage> midiQueue;
  MIDIMap midiMap;
  int learning;

  ImFont* mainFont;
  ImFont* iconFont;
  ImFont* patFont;
  ImFont* bigFont;
  ImWchar* fontRange;
  ImVec4 uiColors[GUI_COLOR_MAX];
  ImVec4 volColors[128];
  ImU32 pitchGrad[256];
  ImU32 volGrad[256];
  ImU32 noteGrad[256];
  ImU32 panGrad[256];
  ImU32 insGrad[256];
  ImU32 sysCmd1Grad[256];
  ImU32 sysCmd2Grad[256];

  struct Settings {
    int mainFontSize, patFontSize, iconSize;
    int audioEngine;
    int audioQuality;
    int arcadeCore;
    int ym2612Core;
    int snCore;
    int saaCore;
    int nesCore;
    int fdsCore;
    String yrw801Path;
    String tg100Path;
    String mu5Path;
    int mainFont;
    int patFont;
    int audioRate;
    int audioBufSize;
    int patRowsBase;
    int orderRowsBase;
    int soloAction;
    int pullDeleteBehavior;
    int wrapHorizontal;
    int wrapVertical;
    int macroView;
    int fmNames;
    int allowEditDocking;
    int chipNames;
    int overflowHighlight;
    int partyTime;
    int germanNotation;
    int stepOnDelete;
    int scrollStep;
    int sysSeparators;
    int forceMono;
    int controlLayout;
    int restartOnFlagChange;
    int statusDisplay;
    float dpiScale;
    int viewPrevPattern;
    int guiColorsBase;
    int guiColorsShading;
    int avoidRaisingPattern;
    int insFocusesPattern;
    int stepOnInsert;
    // TODO flags
    int unifiedDataView;
    int sysFileDialog;
    // end
    int roundedWindows;
    int roundedButtons;
    int roundedMenus;
    int loadJapanese;
    int loadChinese;
    int fmLayout;
    int sampleLayout;
    int waveLayout;
    int susPosition;
    int effectCursorDir;
    int cursorPastePos;
    int titleBarInfo;
    int titleBarSys;
    int frameBorders;
    int effectDeletionAltersValue;
    int oscRoundedCorners;
    int oscTakesEntireWindow;
    int oscBorder;
    int oscEscapesBoundary;
    int separateFMColors;
    int insEditColorize;
    int metroVol;
    int pushNibble;
    int scrollChangesOrder;
    int oplStandardWaveNames;
    int cursorMoveNoScroll;
    int lowLatency;
    int notePreviewBehavior;
    int powerSave;
    int absorbInsInput;
    int eventDelay;
    int moveWindowTitle;
    int hiddenSystems;
    int horizontalDataView;
    int noMultiSystem;
    int oldMacroVSlider;
    int displayAllInsTypes;
    int noteCellSpacing;
    int insCellSpacing;
    int volCellSpacing;
    int effectCellSpacing;
    int effectValCellSpacing;
    unsigned int maxUndoSteps;
    String mainFontPath;
    String patFontPath;
    String audioDevice;
    String midiInDevice;
    String midiOutDevice;
    std::vector<int> initialSys;

    Settings():
      mainFontSize(18),
      patFontSize(18),
      iconSize(16),
      audioEngine(DIV_AUDIO_SDL),
      audioQuality(0),
      arcadeCore(0),
      ym2612Core(0),
      snCore(0),
      saaCore(1),
      nesCore(0),
      fdsCore(0),
      yrw801Path(""),
      tg100Path(""),
      mu5Path(""),
      mainFont(0),
      patFont(0),
      audioRate(44100),
      audioBufSize(1024),
      patRowsBase(0),
      orderRowsBase(1),
      soloAction(0),
      pullDeleteBehavior(1),
      wrapHorizontal(0),
      wrapVertical(0),
      macroView(0),
      fmNames(0),
      allowEditDocking(0),
      chipNames(0),
      overflowHighlight(0),
      partyTime(0),
      germanNotation(0),
      stepOnDelete(0),
      scrollStep(0),
      sysSeparators(1),
      forceMono(0),
      controlLayout(3),
      restartOnFlagChange(1),
      statusDisplay(0),
      dpiScale(0.0f),
      viewPrevPattern(1),
      guiColorsBase(0),
      guiColorsShading(0),
      avoidRaisingPattern(0),
      insFocusesPattern(1),
      stepOnInsert(0),
      unifiedDataView(0),
      sysFileDialog(1),
      roundedWindows(1),
      roundedButtons(1),
      roundedMenus(0),
      loadJapanese(0),
      loadChinese(0),
      fmLayout(0),
      sampleLayout(0),
      waveLayout(0),
      susPosition(0),
      effectCursorDir(1),
      cursorPastePos(1),
      titleBarInfo(1),
      titleBarSys(1),
      frameBorders(0),
      effectDeletionAltersValue(1),
      oscRoundedCorners(1),
      oscTakesEntireWindow(0),
      oscBorder(1),
      oscEscapesBoundary(0),
      separateFMColors(0),
      insEditColorize(0),
      metroVol(100),
      pushNibble(0),
      scrollChangesOrder(0),
      oplStandardWaveNames(0),
      cursorMoveNoScroll(0),
      lowLatency(0),
      notePreviewBehavior(1),
      powerSave(1),
      absorbInsInput(0),
      eventDelay(0),
      moveWindowTitle(1),
      hiddenSystems(0),
      horizontalDataView(0),
      noMultiSystem(0),
      oldMacroVSlider(0),
      displayAllInsTypes(0),
      noteCellSpacing(0),
      insCellSpacing(0),
      volCellSpacing(0),
      effectCellSpacing(0),
      effectValCellSpacing(0),
      maxUndoSteps(100),
      mainFontPath(""),
      patFontPath(""),
      audioDevice(""),
      midiInDevice(""),
      midiOutDevice("") {}
  } settings;

  char finalLayoutPath[4096];

  DivInstrument* prevInsData;

  int curIns, curWave, curSample, curOctave, curOrder, prevIns, oldRow, oldOrder, oldOrder1, editStep, exportLoops, soloChan, soloTimeout, orderEditMode, orderCursor;
  int loopOrder, loopRow, loopEnd, isClipping, extraChannelButtons, patNameTarget, newSongCategory, latchTarget;
  int wheelX, wheelY;

  bool editControlsOpen, ordersOpen, insListOpen, songInfoOpen, patternOpen, insEditOpen;
  bool waveListOpen, waveEditOpen, sampleListOpen, sampleEditOpen, aboutOpen, settingsOpen;
  bool mixerOpen, debugOpen, inspectorOpen, oscOpen, volMeterOpen, statsOpen, compatFlagsOpen;
  bool pianoOpen, notesOpen, channelsOpen, regViewOpen, logOpen, effectListOpen, chanOscOpen;
  bool subSongsOpen;

  /* there ought to be a better way...
  bool editControlsDocked, ordersDocked, insListDocked, songInfoDocked, patternDocked, insEditDocked;
  bool waveListDocked, waveEditDocked, sampleListDocked, sampleEditDocked, aboutDocked, settingsDocked;
  bool mixerDocked, debugDocked, inspectorDocked, oscDocked, volMeterDocked, statsDocked, compatFlagsDocked;
  bool pianoDocked, notesDocked, channelsDocked, regViewDocked, logDocked, effectListDocked, chanOscDocked;
  bool subSongsDocked;
  */

  SelectionPoint selStart, selEnd, cursor;
  bool selecting, selectingFull, curNibble, orderNibble, followOrders, followPattern, changeAllOrders, mobileUI;
  bool collapseWindow, demandScrollX, fancyPattern, wantPatName, firstFrame, tempoView, waveHex, lockLayout, editOptsVisible, latchNibble, nonLatchNibble;
  FurnaceGUIWindows curWindow, nextWindow, curWindowLast;
  float peak[2];
  float patChanX[DIV_MAX_CHANS+1];
  float patChanSlideY[DIV_MAX_CHANS+1];
  const int* nextDesc;

  OperationMask opMaskDelete, opMaskPullDelete, opMaskInsert, opMaskPaste, opMaskTransposeNote, opMaskTransposeValue;
  OperationMask opMaskInterpolate, opMaskFade, opMaskInvertVal, opMaskScale;
  OperationMask opMaskRandomize, opMaskFlip, opMaskCollapseExpand;
  short latchNote, latchIns, latchVol, latchEffect, latchEffectVal;

  DivWaveSynth wavePreview;
  int wavePreviewLen, wavePreviewHeight;
  bool wavePreviewInit;

  // bit 31: ctrl
  // bit 30: reserved for SDL scancode mask
  // bit 29: shift
  // bit 28: meta (win)
  // bit 27: alt
  // bit 24-26: reserved
  int actionKeys[GUI_ACTION_MAX];

  std::map<int,int> actionMapGlobal;
  std::map<int,int> actionMapPat;
  std::map<int,int> actionMapOrders;
  std::map<int,int> actionMapSample;
  std::map<int,int> actionMapInsList;
  std::map<int,int> actionMapWaveList;
  std::map<int,int> actionMapSampleList;

  std::vector<DivRegWrite> pgProgram;
  int pgSys, pgAddr, pgVal;

  struct ActiveNote {
    int chan;
    int note;
    ActiveNote(int c, int n):
      chan(c),
      note(n) {}
  };
  std::vector<ActiveNote> activeNotes;
  std::vector<DivCommand> cmdStream;
  std::vector<Particle> particles;
  std::vector<DivInstrument*> pendingIns;

  std::vector<FurnaceGUISysCategory> sysCategories;

  bool wavePreviewOn;
  SDL_Scancode wavePreviewKey;
  int wavePreviewNote;

  bool samplePreviewOn;
  SDL_Scancode samplePreviewKey;
  int samplePreviewNote;

  // SDL_Scancode,int
  std::map<int,int> noteKeys;
  // SDL_Keycode,int
  std::map<int,int> valueKeys;

  // currently active touch points
  std::vector<TouchPoint> activePoints;
  // one frame points
  std::vector<TouchPoint> pressedPoints;
  std::vector<TouchPoint> releasedPoints;

  int arpMacroScroll;
  int pitchMacroScroll;

  ImVec2 macroDragStart;
  ImVec2 macroDragAreaSize;
  unsigned char* macroDragCTarget;
  int* macroDragTarget;
  int macroDragLen;
  int macroDragMin, macroDragMax;
  int macroDragLastX, macroDragLastY;
  int macroDragBitOff;
  int macroDragScroll;
  bool macroDragBitMode;
  bool macroDragInitialValueSet;
  bool macroDragInitialValue;
  bool macroDragChar;
  bool macroDragLineMode;
  bool macroDragMouseMoved;
  ImVec2 macroDragLineInitial;
  ImVec2 macroDragLineInitialV;
  bool macroDragActive;
  FurnaceGUIMacroDesc lastMacroDesc;
  int macroOffX, macroOffY;
  float macroScaleX, macroScaleY;

  ImVec2 macroLoopDragStart;
  ImVec2 macroLoopDragAreaSize;
  signed char* macroLoopDragTarget;
  int macroLoopDragLen;
  bool macroLoopDragActive;

  ImVec2 waveDragStart;
  ImVec2 waveDragAreaSize;
  int* waveDragTarget;
  int waveDragLen;
  int waveDragMin, waveDragMax;
  bool waveDragActive;

  int bindSetTarget, bindSetPrevValue;
  bool bindSetActive, bindSetPending;

  float nextScroll, nextAddScroll;

  int layoutTimeBegin, layoutTimeEnd, layoutTimeDelta;
  int renderTimeBegin, renderTimeEnd, renderTimeDelta;
  int eventTimeBegin, eventTimeEnd, eventTimeDelta;

  int chanToMove;

  ImVec2 patWindowPos, patWindowSize;

  // pattern view specific
  ImVec2 fourChars, threeChars, twoChars;
  ImVec2 noteCellSize, insCellSize, volCellSize, effectCellSize, effectValCellSize;
  SelectionPoint sel1, sel2;
  int dummyRows, demandX;
  int transposeAmount, randomizeMin, randomizeMax, fadeMin, fadeMax;
  float scaleMax;
  bool fadeMode, randomMode, haveHitBounds, pendingStepUpdate;

  int oldOrdersLen;
  DivOrders oldOrders;
  DivPattern* oldPat[DIV_MAX_CHANS];
  std::deque<UndoStep> undoHist;
  std::deque<UndoStep> redoHist;

  // sample editor specific
  double sampleZoom;
  double prevSampleZoom;
  int samplePos;
  int resizeSize, silenceSize;
  double resampleTarget;
  int resampleStrat;
  float amplifyVol;
  int sampleSelStart, sampleSelEnd;
  bool sampleDragActive, sampleDragMode, sampleDrag16, sampleZoomAuto;
  void* sampleDragTarget;
  ImVec2 sampleDragStart;
  ImVec2 sampleDragAreaSize;
  unsigned int sampleDragLen;
  float sampleFilterL, sampleFilterB, sampleFilterH, sampleFilterRes, sampleFilterCutStart, sampleFilterCutEnd;
  unsigned char sampleFilterPower;
  short* sampleClipboard;
  size_t sampleClipboardLen;
  bool openSampleResizeOpt, openSampleResampleOpt, openSampleAmplifyOpt, openSampleSilenceOpt, openSampleFilterOpt;

  // oscilloscope
  int oscTotal;
  float oscValues[512];
  float oscZoom;
  bool oscZoomSlider;

  // per-channel oscilloscope
  int chanOscCols;
  float chanOscWindowSize;
  bool chanOscWaveCorr;
  float chanOscLP0[DIV_MAX_CHANS];
  float chanOscLP1[DIV_MAX_CHANS];
  unsigned short lastNeedlePos[DIV_MAX_CHANS];
  unsigned short lastCorrPos[DIV_MAX_CHANS];

  // visualizer
  float keyHit[DIV_MAX_CHANS];
  int lastIns[DIV_MAX_CHANS];

  // log window
  bool followLog;

  // piano
  int pianoOctaves, pianoOctavesEdit;
  bool pianoOptions, pianoSharePosition, pianoOptionsSet;
  float pianoKeyHit[180];
  bool pianoKeyPressed[180];
  int pianoOffset, pianoOffsetEdit;
  int pianoView, pianoInputPadMode;

  // TX81Z
  bool hasACED;
  unsigned char acedData[23];

  void drawSSGEnv(unsigned char type, const ImVec2& size);
  void drawWaveform(unsigned char type, bool opz, const ImVec2& size);
  void drawAlgorithm(unsigned char alg, FurnaceGUIFMAlgs algType, const ImVec2& size);
  void drawFMEnv(unsigned char tl, unsigned char ar, unsigned char dr, unsigned char d2r, unsigned char rr, unsigned char sl, unsigned char sus, unsigned char egt, unsigned char algOrGlobalSus, float maxTl, float maxArDr, const ImVec2& size, unsigned short instType);
  void drawGBEnv(unsigned char vol, unsigned char len, unsigned char sLen, bool dir, const ImVec2& size);
  void drawSysConf(int chan, DivSystem type, unsigned int& flags, bool modifyOnChange);

  // these ones offer ctrl-wheel fine value changes.
  bool CWSliderScalar(const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format=NULL, ImGuiSliderFlags flags=0);
  bool CWVSliderScalar(const char* label, const ImVec2& size, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format=NULL, ImGuiSliderFlags flags=0);
  bool CWSliderInt(const char* label, int* v, int v_min, int v_max, const char* format="%d", ImGuiSliderFlags flags=0);
  bool CWSliderFloat(const char* label, float* v, float v_min, float v_max, const char* format="%.3f", ImGuiSliderFlags flags=0);
  bool CWVSliderInt(const char* label, const ImVec2& size, int* v, int v_min, int v_max, const char* format="%d", ImGuiSliderFlags flags=0);

  void updateWindowTitle();
  void prepareLayout();

  void readOsc();

  void pushAccentColors(const ImVec4& one, const ImVec4& two, const ImVec4& border, const ImVec4& borderShadow);
  void popAccentColors();

  float calcBPM(int s1, int s2, float hz, int vN, int vD);

  void patternRow(int i, bool isPlaying, float lineHeight, int chans, int ord, const DivPattern** patCache, bool inhibitSel);

  void drawMacros(std::vector<FurnaceGUIMacroDesc>& macros);

  void actualWaveList();
  void actualSampleList();

  void toggleMobileUI(bool enable, bool force=false);

  void drawMobileControls();
  void drawEditControls();
  void drawSongInfo();
  void drawOrders();
  void drawPattern();
  void drawInsList();
  void drawInsEdit();
  void drawWaveList();
  void drawWaveEdit();
  void drawSampleList();
  void drawSampleEdit();
  void drawMixer();
  void drawOsc();
  void drawChanOsc();
  void drawVolMeter();
  void drawStats();
  void drawCompatFlags();
  void drawPiano();
  void drawNotes();
  void drawChannels();
  void drawRegView();
  void drawAbout();
  void drawSettings();
  void drawDebug();
  void drawNewSong();
  void drawLog();
  void drawEffectList();
  void drawSubSongs();

  void parseKeybinds();
  void promptKey(int which);
  void doAction(int what);

  bool importColors(String path);
  bool exportColors(String path);
  bool importKeybinds(String path);
  bool exportKeybinds(String path);
  bool importLayout(String path);
  bool exportLayout(String path);

  void resetColors();
  void resetKeybinds();

  void syncSettings();
  void commitSettings();
  void processDrags(int dragX, int dragY);
  void processPoint(SDL_Event& ev);

  void startSelection(int xCoarse, int xFine, int y, bool fullRow=false);
  void updateSelection(int xCoarse, int xFine, int y, bool fullRow=false);
  void finishSelection();

  void moveCursor(int x, int y, bool select);
  void moveCursorPrevChannel(bool overflow);
  void moveCursorNextChannel(bool overflow);
  void moveCursorTop(bool select);
  void moveCursorBottom(bool select);
  void editAdvance();
  void prepareUndo(ActionType action);
  void makeUndo(ActionType action);
  void doSelectAll();
  void doDelete();
  void doPullDelete();
  void doInsert();
  void doTranspose(int amount, OperationMask& mask);
  void doCopy(bool cut);
  void doPaste(PasteMode mode=GUI_PASTE_MODE_NORMAL);
  void doChangeIns(int ins);
  void doInterpolate();
  void doFade(int p0, int p1, bool mode);
  void doInvertValues();
  void doScale(float top);
  void doRandomize(int bottom, int top, bool mode);
  void doFlip();
  void doCollapse(int divider);
  void doExpand(int multiplier);
  void doUndo();
  void doRedo();
  void editOptions(bool topMenu);
  void noteInput(int num, int key, int vol=-1);
  void valueInput(int num, bool direct=false, int target=-1);

  void doUndoSample();
  void doRedoSample();

  void play(int row=0);
  void setOrder(unsigned char order, bool forced=false);
  void stop();

  void previewNote(int refChan, int note, bool autoNote=false);
  void stopPreviewNote(SDL_Scancode scancode, bool autoNote=false);

  void keyDown(SDL_Event& ev);
  void keyUp(SDL_Event& ev);

  void openFileDialog(FurnaceGUIFileDialogs type);
  int save(String path, int dmfVersion);
  int load(String path);
  void exportAudio(String path, DivAudioExportModes mode);

  bool parseSysEx(unsigned char* data, size_t len);

  void applyUISettings(bool updateFonts=true);
  void initSystemPresets();

  void encodeMMLStr(String& target, int* macro, int macroLen, int macroLoop, int macroRel, bool hex=false);
  void decodeMMLStr(String& source, int* macro, unsigned char& macroLen, signed char& macroLoop, int macroMin, int macroMax, signed char& macroRel);
  void decodeMMLStrW(String& source, int* macro, int& macroLen, int macroMax, bool hex=false);

  String encodeKeyMap(std::map<int,int>& map);
  void decodeKeyMap(std::map<int,int>& map, String source);

  const char* getSystemName(DivSystem which);

  public:
    void showWarning(String what, FurnaceGUIWarnings type);
    void showError(String what);
    const char* noteName(short note, short octave);
    bool decodeNote(const char* what, short& note, short& octave);
    void bindEngine(DivEngine* eng);
    void updateScroll(int amount);
    void addScroll(int amount);
    void setFileName(String name);
    void runBackupThread();
    int processEvent(SDL_Event* ev);
    bool loop();
    bool finish();
    bool init();
    FurnaceGUI();
};

#endif
